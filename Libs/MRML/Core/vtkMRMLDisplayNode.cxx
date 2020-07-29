/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLDisplayNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/

// MRML includes
#include "vtkEventBroker.h"
#include "vtkMRMLColorNode.h"
#include "vtkMRMLDisplayNode.h"
#include "vtkMRMLDisplayableNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkAssignAttribute.h>
#include <vtkCallbackCommand.h>
#include <vtkCollection.h>
#include <vtkImageData.h>
#include <vtksys/SystemTools.hxx>

// STD includes
#include <algorithm>
#include <sstream>

//----------------------------------------------------------------------------
vtkCxxSetReferenceStringMacro(vtkMRMLDisplayNode, ColorNodeID);

//----------------------------------------------------------------------------
vtkMRMLDisplayNode::vtkMRMLDisplayNode()
{
  this->HideFromEditors = 1;

  this->Opacity = 1.0;
  this->SliceIntersectionOpacity = 1.0;
  this->Ambient = 0.0;
  this->Diffuse = 1.0;
  this->Specular = 0;
  this->Power = 1;
  this->SelectedAmbient = 0.4;
  this->SelectedSpecular = 0.5;

  this->PointSize = 1.0;
  this->LineWidth = 1.0;
  this->Representation = vtkMRMLDisplayNode::SurfaceRepresentation;
  this->Lighting = 1;
  this->Interpolation = vtkMRMLDisplayNode::GouraudInterpolation;
  this->Shading = 1;

  this->Visibility = 1;
  this->Visibility2D = 0;
  this->Visibility3D = 1;
  this->EdgeVisibility = 0;
  this->Clipping = 0;
  this->SliceIntersectionThickness = 1;
  this->FrontfaceCulling = 0;
  this->BackfaceCulling = 1;
  this->ScalarVisibility = 0;
  this->VectorVisibility = 0;
  this->TensorVisibility = 0;
  this->InterpolateTexture = 0;
  this->ScalarRangeFlag = vtkMRMLDisplayNode::UseDataScalarRange;
  this->FolderDisplayOverrideAllowed = true;

  // Arrays
  this->ScalarRange[0] = 0;
  this->ScalarRange[1] = 100;

  // Set default color to yellow to have some contrast compared to grayscale images
  this->Color[0] = 0.9;
  this->Color[1] = 0.9;
  this->Color[2] = 0.3;
  this->EdgeColor[0] = 0.0;
  this->EdgeColor[1] = 0.0;
  this->EdgeColor[2] = 0.0;
  this->SelectedColor[0] = 1.0;
  this->SelectedColor[1] = 0.0;
  this->SelectedColor[2] = 0.0;
  this->SelectedAmbient = 0.4;
  this->SelectedSpecular = 0.5;

  this->TextureImageDataConnection = nullptr;
  this->ColorNodeID = nullptr;
  this->ColorNode = nullptr;

  this->ActiveScalarName = nullptr;
  this->ActiveAttributeLocation = vtkAssignAttribute::POINT_DATA;

  // add observer to process visualization pipeline
  vtkEventBroker::GetInstance()->AddObservation(
    this, vtkCommand::ModifiedEvent, this, this->MRMLCallbackCommand );
}

//----------------------------------------------------------------------------
vtkMRMLDisplayNode::~vtkMRMLDisplayNode()
{
  this->SetTextureImageDataConnection(nullptr);
  this->SetActiveScalarName(nullptr);
  this->SetAndObserveColorNodeID( nullptr);
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);

  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLVectorMacro(color, Color, double, 3);
  vtkMRMLWriteXMLVectorMacro(edgeColor, EdgeColor, double, 3);
  vtkMRMLWriteXMLVectorMacro(selectedColor, SelectedColor, double, 3);
  vtkMRMLWriteXMLFloatMacro(selectedAmbient, SelectedAmbient);
  vtkMRMLWriteXMLFloatMacro(ambient, Ambient);
  vtkMRMLWriteXMLFloatMacro(diffuse, Diffuse);
  vtkMRMLWriteXMLFloatMacro(selectedSpecular, SelectedSpecular);
  vtkMRMLWriteXMLFloatMacro(specular, Specular);
  vtkMRMLWriteXMLFloatMacro(power, Power);
  vtkMRMLWriteXMLFloatMacro(opacity, Opacity);
  vtkMRMLWriteXMLFloatMacro(sliceIntersectionOpacity, SliceIntersectionOpacity);
  vtkMRMLWriteXMLFloatMacro(pointSize, PointSize);
  vtkMRMLWriteXMLFloatMacro(lineWidth, LineWidth);
  vtkMRMLWriteXMLIntMacro(representation, Representation);
  vtkMRMLWriteXMLBooleanMacro(lighting, Lighting);
  vtkMRMLWriteXMLIntMacro(interpolation, Interpolation);
  vtkMRMLWriteXMLBooleanMacro(shading, Shading);
  vtkMRMLWriteXMLBooleanMacro(visibility, Visibility);
  vtkMRMLWriteXMLBooleanMacro(visibility2D, Visibility2D);
  vtkMRMLWriteXMLBooleanMacro(visibility3D, Visibility3D);
  vtkMRMLWriteXMLBooleanMacro(edgeVisibility, EdgeVisibility);
  vtkMRMLWriteXMLBooleanMacro(clipping, Clipping);
  vtkMRMLWriteXMLIntMacro(sliceIntersectionThickness, SliceIntersectionThickness);
  vtkMRMLWriteXMLBooleanMacro(frontfaceCulling, FrontfaceCulling);
  vtkMRMLWriteXMLBooleanMacro(backfaceCulling, BackfaceCulling);
  vtkMRMLWriteXMLBooleanMacro(scalarVisibility, ScalarVisibility);
  vtkMRMLWriteXMLBooleanMacro(vectorVisibility, VectorVisibility);
  vtkMRMLWriteXMLBooleanMacro(tensorVisibility, TensorVisibility);
  vtkMRMLWriteXMLBooleanMacro(interpolateTexture, InterpolateTexture);
  vtkMRMLWriteXMLStringMacro(scalarRangeFlag, ScalarRangeFlagAsString);
  vtkMRMLWriteXMLVectorMacro(scalarRange, ScalarRange, double, 2);
  vtkMRMLWriteXMLStringMacro(colorNodeID, ColorNodeID);
  vtkMRMLWriteXMLStringMacro(activeScalarName, ActiveScalarName);
  vtkMRMLWriteXMLStringMacro(activeAttributeLocation, ActiveAttributeLocationAsString);
  vtkMRMLWriteXMLStdStringVectorMacro(viewNodeRef, ViewNodeIDs, std::vector);
  vtkMRMLWriteXMLBooleanMacro(folderDisplayOverrideAllowed, FolderDisplayOverrideAllowed);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayNode::ReadXMLAttributes(const char** atts)
{
  MRMLNodeModifyBlocker blocker(this);

  Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLVectorMacro(color, Color, double, 3);
  vtkMRMLReadXMLVectorMacro(edgeColor, EdgeColor, double, 3);
  vtkMRMLReadXMLVectorMacro(selectedColor, SelectedColor, double, 3);
  vtkMRMLReadXMLFloatMacro(selectedAmbient, SelectedAmbient);
  vtkMRMLReadXMLFloatMacro(ambient, Ambient);
  vtkMRMLReadXMLFloatMacro(diffuse, Diffuse);
  vtkMRMLReadXMLFloatMacro(selectedSpecular, SelectedSpecular);
  vtkMRMLReadXMLFloatMacro(specular, Specular);
  vtkMRMLReadXMLFloatMacro(power, Power);
  vtkMRMLReadXMLFloatMacro(opacity, Opacity);
  vtkMRMLReadXMLFloatMacro(sliceIntersectionOpacity, SliceIntersectionOpacity);
  vtkMRMLReadXMLFloatMacro(pointSize, PointSize);
  vtkMRMLReadXMLFloatMacro(lineWidth, LineWidth);
  vtkMRMLReadXMLIntMacro(representation, Representation);
  vtkMRMLReadXMLBooleanMacro(lighting, Lighting);
  vtkMRMLReadXMLIntMacro(interpolation, Interpolation);
  vtkMRMLReadXMLBooleanMacro(shading, Shading);
  vtkMRMLReadXMLBooleanMacro(visibility, Visibility);
  vtkMRMLReadXMLBooleanMacro(visibility2D, Visibility2D);
  vtkMRMLReadXMLBooleanMacro(sliceIntersectionVisibility, Visibility2D); // For backwards compatibility
  vtkMRMLReadXMLBooleanMacro(visibility3D, Visibility3D);
  vtkMRMLReadXMLBooleanMacro(edgeVisibility, EdgeVisibility);
  vtkMRMLReadXMLBooleanMacro(clipping, Clipping);
  vtkMRMLReadXMLIntMacro(sliceIntersectionThickness, SliceIntersectionThickness);
  vtkMRMLReadXMLBooleanMacro(frontfaceCulling, FrontfaceCulling);
  vtkMRMLReadXMLBooleanMacro(backfaceCulling, BackfaceCulling);
  vtkMRMLReadXMLBooleanMacro(scalarVisibility, ScalarVisibility);
  vtkMRMLReadXMLBooleanMacro(vectorVisibility, VectorVisibility);
  vtkMRMLReadXMLBooleanMacro(tensorVisibility, TensorVisibility);
  vtkMRMLReadXMLBooleanMacro(interpolateTexture, InterpolateTexture);
  vtkMRMLReadXMLStringMacro(scalarRangeFlag, ScalarRangeFlagFromString);
  vtkMRMLReadXMLVectorMacro(scalarRange, ScalarRange, double, 2);
  vtkMRMLReadXMLStringMacro(colorNodeID, ColorNodeID);
  vtkMRMLReadXMLStringMacro(colorNodeRef, ColorNodeID);
  vtkMRMLReadXMLStringMacro(activeScalarName, ActiveScalarName);
  vtkMRMLReadXMLStringMacro(activeAttributeLocation, ActiveAttributeLocationFromString);
  vtkMRMLReadXMLBooleanMacro(folderDisplayOverrideAllowed, FolderDisplayOverrideAllowed);
  if (!strcmp(xmlReadAttName, "autoScalarRange"))
    {
    if (!strcmp(xmlReadAttName,"true"))
      {
      this->SetScalarRangeFlag(vtkMRMLDisplayNode::UseDataScalarRange);
      }
    else
      {
      this->SetScalarRangeFlag(vtkMRMLDisplayNode::UseManualScalarRange);
      }
    }
  if (!strcmp(xmlReadAttValue, "viewNodeRef"))
    {
    std::stringstream ss(xmlReadAttValue);
    while (!ss.eof())
      {
      std::string id;
      ss >> id;
      this->AddViewNodeID(id.c_str());
      }
    }
  vtkMRMLReadXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLDisplayNode *node = vtkMRMLDisplayNode::SafeDownCast(anode);
  if (!node)
    {
    return;
    }

  // Strings
  this->SetColor(node->Color);
  this->SetSelectedColor(node->SelectedColor);

  // Vectors
  this->SetScalarRange(node->ScalarRange);

  // Numbers
  this->SetSelectedAmbient(node->SelectedAmbient);
  this->SetSelectedSpecular(node->SelectedSpecular);
  this->SetOpacity(node->Opacity);
  this->SetSliceIntersectionOpacity(node->SliceIntersectionOpacity);
  this->SetAmbient(node->Ambient);
  this->SetDiffuse(node->Diffuse);
  this->SetSpecular(node->Specular);
  this->SetPower(node->Power);
  this->SetVisibility(node->Visibility);
  this->SetVisibility2D(node->Visibility2D);
  this->SetVisibility3D(node->Visibility3D);
  this->SetScalarVisibility(node->ScalarVisibility);
  this->SetVectorVisibility(node->VectorVisibility);
  this->SetTensorVisibility(node->TensorVisibility);
  this->SetInterpolateTexture(node->InterpolateTexture);
  this->SetScalarRangeFlag(node->ScalarRangeFlag);
  this->SetBackfaceCulling(node->BackfaceCulling);
  this->SetClipping(node->Clipping);
  this->SetVisibility2D(node->GetSliceIntersectionVisibility());
  this->SetSliceIntersectionThickness(node->SliceIntersectionThickness);
  this->SetTextureImageDataConnection(node->TextureImageDataConnection);
  this->SetAndObserveColorNodeID(node->ColorNodeID);
  this->SetActiveScalarName(node->ActiveScalarName);
  this->SetFolderDisplayOverrideAllowed(node->FolderDisplayOverrideAllowed);
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintVectorMacro(Color, double, 3);
  vtkMRMLPrintVectorMacro(EdgeColor, double, 3);
  vtkMRMLPrintVectorMacro(SelectedColor, double, 3);
  vtkMRMLPrintFloatMacro(SelectedAmbient);
  vtkMRMLPrintFloatMacro(Ambient);
  vtkMRMLPrintFloatMacro(Diffuse);
  vtkMRMLPrintFloatMacro(SelectedSpecular);
  vtkMRMLPrintFloatMacro(Specular);
  vtkMRMLPrintFloatMacro(Power);
  vtkMRMLPrintFloatMacro(Opacity);
  vtkMRMLPrintFloatMacro(SliceIntersectionOpacity);
  vtkMRMLPrintFloatMacro(PointSize);
  vtkMRMLPrintFloatMacro(LineWidth);
  vtkMRMLPrintIntMacro(Representation);
  vtkMRMLPrintBooleanMacro(Lighting);
  vtkMRMLPrintIntMacro(Interpolation);
  vtkMRMLPrintBooleanMacro(Shading);
  vtkMRMLPrintBooleanMacro(Visibility);
  vtkMRMLPrintBooleanMacro(Visibility2D);
  vtkMRMLPrintBooleanMacro(Visibility3D);
  vtkMRMLPrintBooleanMacro(EdgeVisibility);
  vtkMRMLPrintBooleanMacro(Clipping);
  vtkMRMLPrintIntMacro(SliceIntersectionThickness);
  vtkMRMLPrintBooleanMacro(FrontfaceCulling);
  vtkMRMLPrintBooleanMacro(BackfaceCulling);
  vtkMRMLPrintBooleanMacro(ScalarVisibility);
  vtkMRMLPrintBooleanMacro(VectorVisibility);
  vtkMRMLPrintBooleanMacro(TensorVisibility);
  vtkMRMLPrintBooleanMacro(InterpolateTexture);
  vtkMRMLPrintStringMacro(ScalarRangeFlagAsString);
  vtkMRMLPrintVectorMacro(ScalarRange, double, 2);
  vtkMRMLPrintStringMacro(ColorNodeID);
  vtkMRMLPrintStringMacro(ActiveScalarName);
  vtkMRMLPrintStringMacro(ActiveAttributeLocationAsString);
  vtkMRMLPrintStdStringVectorMacro(ViewNodeIDs, std::vector);
  vtkMRMLPrintBooleanMacro(FolderDisplayOverrideAllowed);
  vtkMRMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayNode::SetSceneReferences()
{
  this->Superclass::SetSceneReferences();
  if (this->Scene == nullptr)
    {
    return;
    }
  this->Scene->AddReferencedNodeID(this->ColorNodeID, this);
  for (unsigned int i=0; i<this->ViewNodeIDs.size(); i++)
    {
    this->Scene->AddReferencedNodeID(this->ViewNodeIDs[i].c_str(), this);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  this->Superclass::UpdateReferenceID(oldID, newID);
  if (this->ColorNodeID && !strcmp(oldID, this->ColorNodeID))
    {
    this->SetAndObserveColorNodeID(newID);
    }
  else
    {
    bool modified = false;
    for (unsigned int i=0; i<this->ViewNodeIDs.size(); i++)
      {
      if ( std::string(oldID) == this->ViewNodeIDs[i])
        {
        this->ViewNodeIDs[i] =  std::string(newID);
        modified = true;
        }
      }
    if (modified)
      {
      this->Modified();
      }
    }
}

//----------------------------------------------------------------------------
vtkMRMLDisplayableNode* vtkMRMLDisplayNode::GetDisplayableNode()
{
  if (this->Scene == nullptr)
    {
    return nullptr;
    }
  // It is an expensive operation to determine the displayable node
  // (need to iterate through the scene), so the last found value
  // is cached. If it is still valid then we use it.
  if (this->LastFoundDisplayableNode != nullptr)
    {
    if (this->LastFoundDisplayableNode->GetScene() == this->Scene
      && this->LastFoundDisplayableNode->HasDisplayNodeID(this->GetID()))
      {
      return this->LastFoundDisplayableNode;
      }
    }
  vtkMRMLNode* node = nullptr;
  vtkCollectionSimpleIterator it;
  vtkCollection* sceneNodes = this->Scene->GetNodes();
  for (sceneNodes->InitTraversal(it);
       (node = vtkMRMLNode::SafeDownCast(sceneNodes->GetNextItemAsObject(it))) ;)
    {
    vtkMRMLDisplayableNode* displayableNode =
      vtkMRMLDisplayableNode::SafeDownCast(node);
    if (displayableNode && displayableNode->HasDisplayNodeID(this->GetID()))
      {
      this->LastFoundDisplayableNode = displayableNode;
      return displayableNode;
      }
    }
  this->LastFoundDisplayableNode = nullptr;
  return nullptr;
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayNode
::SetTextureImageDataConnection(vtkAlgorithmOutput* newTextureImageDataConnection)
{
   if (newTextureImageDataConnection == this->TextureImageDataConnection)
    {
    return;
    }

  vtkAlgorithm* oldTextureImageDataAlgorithm = this->TextureImageDataConnection ?
    this->TextureImageDataConnection->GetProducer() : nullptr;

  this->TextureImageDataConnection = newTextureImageDataConnection;

  vtkAlgorithm* textureImageDataAlgorithm = this->TextureImageDataConnection ?
    this->TextureImageDataConnection->GetProducer() : nullptr;

  if (textureImageDataAlgorithm != nullptr)
    {
    vtkEventBroker::GetInstance()->AddObservation(
      this->TextureImageDataConnection, vtkCommand::ModifiedEvent,
      this, this->MRMLCallbackCommand );
    textureImageDataAlgorithm->Register(this);
    }

  if (oldTextureImageDataAlgorithm != nullptr)
    {
    vtkEventBroker::GetInstance()->RemoveObservations(
      this->TextureImageDataConnection, vtkCommand::ModifiedEvent,
      this, this->MRMLCallbackCommand );
    oldTextureImageDataAlgorithm->UnRegister(this);
    }
  this->Modified();
}

//-----------------------------------------------------------
void vtkMRMLDisplayNode::UpdateScene(vtkMRMLScene *scene)
{
   Superclass::UpdateScene(scene);

   this->SetAndObserveColorNodeID(this->GetColorNodeID());
}

//-----------------------------------------------------------
void vtkMRMLDisplayNode::UpdateReferences()
{
  Superclass::UpdateReferences();

  if (this->Scene == nullptr)
    {
    return;
    }

  if (this->ColorNodeID != nullptr && this->Scene->GetNodeByID(this->ColorNodeID) == nullptr)
    {
    this->SetAndObserveColorNodeID(nullptr);
    }

  std::vector< std::string > viewNodeIDs;

  for (unsigned int i=0; i < this->ViewNodeIDs.size(); i++)
    {
    if (this->Scene->GetNodeByID(this->ViewNodeIDs[i]) != nullptr)
      {
      viewNodeIDs.push_back(this->ViewNodeIDs[i]);
      }
    }
  if (this->ViewNodeIDs.size() != viewNodeIDs.size())
    {
    // The following line is probably incorrect
    // (should be this->ViewNodeIDs = viewNodeIDs;)
    viewNodeIDs = this->ViewNodeIDs;
    this->Modified();
    }
}

//----------------------------------------------------------------------------
vtkMRMLColorNode* vtkMRMLDisplayNode::GetColorNode()
{
  if (this->ColorNode || !this->ColorNodeID)
    {
    return this->ColorNode;
    }
  vtkMRMLColorNode* cnode = nullptr;
  if (this->GetScene())
    {
    cnode = vtkMRMLColorNode::SafeDownCast(
      this->GetScene()->GetNodeByID(this->ColorNodeID));
    }
  vtkSetAndObserveMRMLObjectMacro(this->ColorNode, cnode);
  return cnode;
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayNode::SetAndObserveColorNodeID(const char *colorNodeID)
{
  vtkMRMLColorNode* cnode = nullptr;
  if (this->GetScene() && colorNodeID)
    {
    cnode = vtkMRMLColorNode::SafeDownCast(
      this->GetScene()->GetNodeByID(colorNodeID));
    }
  if (this->ColorNode != cnode)
    {
    vtkSetAndObserveMRMLObjectMacro(this->ColorNode, cnode);
    }
  this->SetColorNodeInternal(cnode);
  this->SetColorNodeID(colorNodeID);
  if (this->Scene)
    {
    this->Scene->AddReferencedNodeID(this->ColorNodeID, this);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayNode::SetAndObserveColorNodeID(const std::string& colorNodeID)
{
  this->SetAndObserveColorNodeID( colorNodeID.c_str() );
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayNode::SetColorNodeInternal(vtkMRMLColorNode* vtkNotUsed(newColorNode))
{
}

//---------------------------------------------------------------------------
void vtkMRMLDisplayNode::ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData)
{
  this->Superclass::ProcessMRMLEvents(caller, event, callData);

  if (this->TextureImageDataConnection != nullptr &&
      this->TextureImageDataConnection == vtkAlgorithmOutput::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent, nullptr);
    }

  vtkMRMLColorNode* cnode = vtkMRMLColorNode::SafeDownCast(caller);
  if (cnode != nullptr &&
      this->ColorNodeID != nullptr && cnode->GetID() != nullptr &&
      strcmp(this->ColorNodeID, cnode->GetID()) == 0 &&
      event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent, nullptr);
    }
}

//-------------------------------------------------------
void vtkMRMLDisplayNode::AddViewNodeID(const char* viewNodeID)
{
  if (viewNodeID == nullptr)
    {
    return;
    }

  if (this->IsViewNodeIDPresent(viewNodeID))
    {
    return; // already exists, do nothing
    }

  this->ViewNodeIDs.emplace_back(viewNodeID);
  if (this->Scene)
    {
    this->Scene->AddReferencedNodeID(viewNodeID, this);
    }

  this->Modified();
}

//-------------------------------------------------------
void vtkMRMLDisplayNode::RemoveViewNodeID(char* viewNodeID)
{
  if (viewNodeID == nullptr)
    {
    return;
    }
  std::vector< std::string > viewNodeIDs;
  for(unsigned int i=0; i<this->ViewNodeIDs.size(); i++)
    {
    if (std::string(viewNodeID) != this->ViewNodeIDs[i])
      {
      viewNodeIDs.push_back(this->ViewNodeIDs[i]);
      }
    }
  if (viewNodeIDs.size() != this->ViewNodeIDs.size() && this->Scene!=nullptr)
    {
    this->Scene->RemoveReferencedNodeID(viewNodeID, this);
    this->ViewNodeIDs = viewNodeIDs;
    this->Modified();
    }
}

//-------------------------------------------------------
void vtkMRMLDisplayNode::RemoveAllViewNodeIDs()
{
  if (this->Scene!=nullptr)
    {
    for(unsigned int i=0; i<this->ViewNodeIDs.size(); i++)
      {
      this->Scene->RemoveReferencedNodeID(ViewNodeIDs[i].c_str(), this);
      }
    }
  this->ViewNodeIDs.clear();
  this->Modified();
}

//-------------------------------------------------------
const char* vtkMRMLDisplayNode::GetNthViewNodeID(unsigned int index)
{
  if (index >= ViewNodeIDs.size())
    {
    vtkErrorMacro("vtkMRMLDisplayNode::GetNthViewNodeID() index " << index << " outside the range 0-" << ViewNodeIDs.size()-1 );
    return nullptr;
    }
  return ViewNodeIDs[index].c_str();
}

//-------------------------------------------------------
bool vtkMRMLDisplayNode::IsViewNodeIDPresent(const char* viewNodeID)const
{
  if (viewNodeID == nullptr)
    {
    return false;
    }
  std::string value(viewNodeID);
  std::vector< std::string >::const_iterator it =
    std::find(this->ViewNodeIDs.begin(), this->ViewNodeIDs.end(), value);
  return it != this->ViewNodeIDs.end();
}

//-------------------------------------------------------
bool vtkMRMLDisplayNode::IsDisplayableInView(const char* viewNodeID)const
{
  return this->GetNumberOfViewNodeIDs() == 0 || this->IsViewNodeIDPresent(viewNodeID);
}

//-------------------------------------------------------
void vtkMRMLDisplayNode::SetDisplayableOnlyInView(const char *viewNodeID)
{
  if (viewNodeID == nullptr)
    {
    return;
    }

  int disabledModify = this->StartModify();
  this->RemoveAllViewNodeIDs();
  this->AddViewNodeID(viewNodeID);
  this->EndModify(disabledModify);
}

//-------------------------------------------------------
void vtkMRMLDisplayNode
::SetViewNodeIDs(const std::vector<std::string>& viewNodeIDs)
{
  std::vector<std::string>::const_iterator sourceIt =
    viewNodeIDs.begin();
  std::vector<std::string>::iterator destIt =
    this->ViewNodeIDs.begin();
  bool different = false;
  for (; sourceIt != viewNodeIDs.end(); ++sourceIt, ++destIt)
    {
    if (destIt == this->ViewNodeIDs.end())
      {
      different = true;
      }
    else if (*sourceIt != *destIt)
      {
      different = true;
      destIt = this->ViewNodeIDs.erase(destIt);
      }
    if (different)
      {
      destIt = this->ViewNodeIDs.insert(destIt, *sourceIt);
      }
    }
  if (destIt != this->ViewNodeIDs.end())
    {
    different = true;
    this->ViewNodeIDs.erase(destIt, this->ViewNodeIDs.end());
    }
  if (different)
    {
    this->Modified();
    }
}

//-------------------------------------------------------
bool vtkMRMLDisplayNode::GetVisibility(const char* viewNodeID)
{
  bool res = this->GetVisibility() != 0;
  res = res && this->IsDisplayableInView(viewNodeID);
  return res;
}

//-------------------------------------------------------
void vtkMRMLDisplayNode::SetAutoScalarRange(int b)
{
  if (b)
    {
    this->AutoScalarRangeOn();
    }
  else
    {
    this->AutoScalarRangeOff();
    }
}

//-------------------------------------------------------
int vtkMRMLDisplayNode::GetAutoScalarRange()
{
  return (this->GetScalarRangeFlag() == vtkMRMLDisplayNode::UseDataScalarRange);
}

//-------------------------------------------------------
void vtkMRMLDisplayNode::AutoScalarRangeOn()
{
  this->SetScalarRangeFlag(vtkMRMLDisplayNode::UseDataScalarRange);
}

//-------------------------------------------------------
void vtkMRMLDisplayNode::AutoScalarRangeOff()
{
  this->SetScalarRangeFlag(vtkMRMLDisplayNode::UseManualScalarRange);
}

//-------------------------------------------------------
const char* vtkMRMLDisplayNode::GetScalarRangeFlagTypeAsString(int flag)
{
  switch (flag)
    {
    case UseManualScalarRange: return "UseManual";
    case UseDataScalarRange: return "UseData";
    case UseColorNodeScalarRange: return "UseColorNode";
    case UseDataTypeScalarRange: return "UseDataType";
    case UseDirectMapping: return "UseDirectMapping";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkMRMLDisplayNode::GetScalarRangeFlagTypeFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int i = 0; i < vtkMRMLDisplayNode::NUM_SCALAR_RANGE_FLAGS; i++)
    {
    if (strcmp(name, vtkMRMLDisplayNode::GetScalarRangeFlagTypeAsString(i)) == 0)
      {
      // found a matching name
      return i;
      }
    }
  // name not found
  return -1;
}

//-----------------------------------------------------------
const char* vtkMRMLDisplayNode::GetScalarRangeFlagAsString()
{
  return this->GetScalarRangeFlagTypeAsString(this->ScalarRangeFlag);
}

//-----------------------------------------------------------
void vtkMRMLDisplayNode::SetScalarRangeFlagFromString(const char* str)
{
  int scalarRangeFlag = this->GetScalarRangeFlagTypeFromString(str);
  if (scalarRangeFlag < 0)
    {
    // invalid value, maybe legacy scene
    scalarRangeFlag = atoi(str);
    }
  this->SetScalarRangeFlag(scalarRangeFlag);
}

//-----------------------------------------------------------
const char* vtkMRMLDisplayNode::GetAttributeLocationAsString(int id)
{
  switch (id)
    {
    case vtkAssignAttribute::POINT_DATA: return "point";
    case vtkAssignAttribute::CELL_DATA: return "cell";
    case vtkAssignAttribute::VERTEX_DATA: return "vertex";
    case vtkAssignAttribute::EDGE_DATA: return "edge";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkMRMLDisplayNode::GetAttributeLocationFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int i = 0; i < vtkAssignAttribute::NUM_ATTRIBUTE_LOCS; i++)
    {
    if (strcmp(name, vtkMRMLDisplayNode::GetAttributeLocationAsString(i)) == 0)
      {
      // found a matching name
      return i;
      }
    }
  // name not found
  return -1;
}

//-----------------------------------------------------------
const char* vtkMRMLDisplayNode::GetActiveAttributeLocationAsString()
{
  return vtkMRMLDisplayNode::GetAttributeLocationAsString(this->GetActiveAttributeLocation());
}

//-----------------------------------------------------------
void vtkMRMLDisplayNode::SetActiveAttributeLocationFromString(const char* str)
{
  int id = this->GetAttributeLocationFromString(str);
  if (id<0)
    {
    vtkWarningMacro("Invalid activeAttributeLocation: " << (str ? str : "(none)"));
    }
  else
    {
    this->SetActiveAttributeLocation(id);
    }
}

//-----------------------------------------------------------
void vtkMRMLDisplayNode::SetSliceIntersectionVisibility(int on)
{
  vtkWarningMacro("SetSliceIntersectionVisibility method is deprecated, please use SetVisibility2D instead");
  this->SetVisibility2D(on);
}

//-----------------------------------------------------------
int vtkMRMLDisplayNode::GetSliceIntersectionVisibility()
{
  vtkDebugMacro("GetSliceIntersectionVisibility method is deprecated, please use GetVisibility2D instead");
  return this->GetVisibility2D();
}

//-----------------------------------------------------------
void vtkMRMLDisplayNode::SliceIntersectionVisibilityOn()
{
  vtkWarningMacro("SliceIntersectionVisibilityOn method is deprecated, please use Visibility2DOn instead");
  this->Visibility2DOn();
}

//-----------------------------------------------------------
void vtkMRMLDisplayNode::SliceIntersectionVisibilityOff()
{
  vtkWarningMacro("SliceIntersectionVisibilityOff method is deprecated, please use Visibility2DOff instead");
  this->Visibility2DOff();
}
