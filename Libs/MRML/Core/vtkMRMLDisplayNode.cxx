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

  // Arrays
  this->ScalarRange[0] = 0;
  this->ScalarRange[1] = 100;

  this->Color[0] = 0.5;
  this->Color[1] = 0.5;
  this->Color[2] = 0.5;
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

  of << " color=\"" << this->Color[0] << " "
    << this->Color[1] << " "
    << this->Color[2] << "\"";

  of << " edgeColor=\"" << this->EdgeColor[0] << " "
    << this->EdgeColor[1] << " " << this->EdgeColor[2] << "\"";
  of << " selectedColor=\"" << this->SelectedColor[0] << " "
    << this->SelectedColor[1] << " "
    << this->SelectedColor[2] << "\"";

  of << " selectedAmbient=\"" << this->SelectedAmbient << "\"";

  of << " ambient=\"" << this->Ambient << "\"";

  of << " diffuse=\"" << this->Diffuse << "\"";

  of << " selectedSpecular=\"" << this->SelectedSpecular << "\"";

  of << " specular=\"" << this->Specular << "\"";

  of << " power=\"" << this->Power << "\"";

  of << " opacity=\"" << this->Opacity << "\"";
  of << " sliceIntersectionOpacity=\"" << this->SliceIntersectionOpacity << "\"";

  of << " pointSize=\"" << this->PointSize << "\"";
  of << " lineWidth=\"" << this->LineWidth << "\"";
  of << " representation=\"" << this->Representation << "\"";
  of << " lighting=\"" << (this->Lighting? "true" : "false") << "\"";
  of << " interpolation=\"" << this->Interpolation << "\"";
  of << " shading=\"" << (this->Shading? "true" : "false") << "\"";

  of << " visibility=\"" << (this->Visibility ? "true" : "false") << "\"";
  of << " visibility2D=\"" << (this->Visibility2D ? "true" : "false") << "\"";
  of << " visibility3D=\"" << (this->Visibility3D ? "true" : "false") << "\"";
  of << " edgeVisibility=\"" << (this->EdgeVisibility? "true" : "false") << "\"";
  of << " clipping=\"" << (this->Clipping ? "true" : "false") << "\"";

  of << " sliceIntersectionThickness=\"" << this->SliceIntersectionThickness << "\"";

  of << " frontfaceCulling=\"" << (this->FrontfaceCulling ? "true" : "false") << "\"";
  of << " backfaceCulling=\"" << (this->BackfaceCulling ? "true" : "false") << "\"";

  of << " scalarVisibility=\"" << (this->ScalarVisibility ? "true" : "false") << "\"";

  of << " vectorVisibility=\"" << (this->VectorVisibility ? "true" : "false") << "\"";

  of << " tensorVisibility=\"" << (this->TensorVisibility ? "true" : "false") << "\"";

  of << " interpolateTexture=\"" << (this->InterpolateTexture ? "true" : "false") << "\"";

  of << " scalarRangeFlag=\"" << this->GetScalarRangeFlagTypeAsString(this->ScalarRangeFlag) << "\"";

  of << " scalarRange=\"" << this->ScalarRange[0] << " "
     << this->ScalarRange[1] << "\"";

  if (this->ColorNodeID != nullptr)
    {
    of << " colorNodeID=\"" << this->ColorNodeID << "\"";
    }

  if (this->ActiveScalarName != nullptr)
    {
    of << " activeScalarName=\"" << this->ActiveScalarName << "\"";
    of << " activeAttributeLocation=\"" << vtkMRMLDisplayNode::GetAttributeLocationAsString(
        this->GetActiveAttributeLocation()) << "\"";
    }

  std::stringstream ss;
  unsigned int n;
  for (n=0; n < this->ViewNodeIDs.size(); n++)
    {
    ss << this->ViewNodeIDs[n];
    if (n < ViewNodeIDs.size()-1)
      {
      ss << " ";
      }
    }
  if (this->ViewNodeIDs.size() > 0)
    {
    of << " viewNodeRef=\"" << ss.str().c_str() << "\"";
    }

  of << " ";
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
void vtkMRMLDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "color"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> Color[0];
      ss >> Color[1];
      ss >> Color[2];
      }
    else if (!strcmp(attName, "edgeColor"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->EdgeColor[0];
      ss >> this->EdgeColor[1];
      ss >> this->EdgeColor[2];
      }
    else if (!strcmp(attName, "selectedColor"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> SelectedColor[0];
      ss >> SelectedColor[1];
      ss >> SelectedColor[2];
      }
    else if (!strcmp(attName, "selectedAmbient"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> SelectedAmbient;
      }
    else if (!strcmp(attName, "selectedSpecular"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> SelectedSpecular;
      }
    else if (!strcmp(attName, "scalarRange"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> ScalarRange[0];
      ss >> ScalarRange[1];
      }
    else if (!strcmp(attName, "ambient"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> Ambient;
      }
    else if (!strcmp(attName, "diffuse"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> Diffuse;
      }
    else if (!strcmp(attName, "specular"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> Specular;
      }
    else if (!strcmp(attName, "power"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> Power;
      }
    else if (!strcmp(attName, "opacity"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> Opacity;
      }
    else if (!strcmp(attName, "sliceIntersectionOpacity"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> SliceIntersectionOpacity;
      }
    else if (!strcmp(attName, "pointSize"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->PointSize;
      }
    else if (!strcmp(attName, "lineWidth"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->LineWidth;
      }
    else if (!strcmp(attName, "representation"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Representation;
      }
    else if (!strcmp(attName, "lighting"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->Lighting = 1;
        }
      else
        {
        this->Lighting = 0;
        }
      }
    else if (!strcmp(attName, "interpolation"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Interpolation;
      }
    else if (!strcmp(attName, "shading"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->Shading = 1;
        }
      else
        {
        this->Shading = 0;
        }
      }
    else if (!strcmp(attName, "visibility"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->Visibility = 1;
        }
      else
        {
        this->Visibility = 0;
        }
      }
    else if (!strcmp(attName, "visibility2D"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->Visibility2D = 1;
        }
      else
        {
        this->Visibility2D = 0;
        }
      }
    else if (!strcmp(attName, "visibility3D"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->Visibility3D = 1;
        }
      else
        {
        this->Visibility3D = 0;
        }
      }
    else if (!strcmp(attName, "edgeVisibility"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->EdgeVisibility = 1;
        }
      else
        {
        this->EdgeVisibility = 0;
        }
      }
    else if (!strcmp(attName, "clipping"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->Clipping = 1;
        }
      else
        {
        this->Clipping = 0;
        }
     }
    else if (!strcmp(attName, "sliceIntersectionVisibility"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->Visibility2D = 1;
        }
      else
        {
        this->Visibility2D = 0;
        }
      }
    else if (!strcmp(attName, "sliceIntersectionThickness"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->SliceIntersectionThickness;
      }
    else if (!strcmp(attName, "frontfaceCulling"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->FrontfaceCulling = 1;
        }
      else
        {
        this->FrontfaceCulling = 0;
        }
      }
    else if (!strcmp(attName, "backfaceCulling"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->BackfaceCulling = 1;
        }
      else
        {
        this->BackfaceCulling = 0;
        }
      }
    else if (!strcmp(attName, "scalarVisibility"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->ScalarVisibility = 1;
        }
      else
        {
        this->ScalarVisibility = 0;
        }
      }
    else if (!strcmp(attName, "vectorVisibility"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->VectorVisibility = 1;
        }
      else
        {
        this->VectorVisibility = 0;
        }
      }
    else if (!strcmp(attName, "tensorVisibility"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->TensorVisibility = 1;
        }
      else
        {
        this->TensorVisibility = 0;
        }
      }
    else if (!strcmp(attName, "interpolateTexture"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->InterpolateTexture = 1;
        }
      else
        {
        this->InterpolateTexture = 0;
        }
      }
    else if (!strcmp(attName, "scalarRangeFlag"))
      {
      int scalarRangeFlag = this->GetScalarRangeFlagTypeFromString(attValue);
      if (scalarRangeFlag < 0)
        {
        // invalid value, maybe legacy scene
        scalarRangeFlag = atoi(attValue);
        }
      this->SetScalarRangeFlag(scalarRangeFlag);
      }
    else if (!strcmp(attName, "autoScalarRange"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->SetScalarRangeFlag(vtkMRMLDisplayNode::UseDataScalarRange);
        }
      else
        {
        this->SetScalarRangeFlag(vtkMRMLDisplayNode::UseManualScalarRange);
        }
      }
    else if (!strcmp(attName, "colorNodeID") ||
             !strcmp(attName, "colorNodeRef"))
      {
      this->SetAndObserveColorNodeID(attValue);
      }
    else if (!strcmp(attName, "activeScalarName"))
      {
      this->SetActiveScalarName(attValue);
      }
    else if (!strcmp(attName, "activeAttributeLocation"))
      {
      int id = this->GetAttributeLocationFromString(attValue);
      if (id<0)
        {
        vtkWarningMacro("Invalid activeAttributeLocation: " << (attValue ? attValue : "(none)"));
        }
      else
        {
        this->SetActiveAttributeLocation(id);
        }
      }
    else if (!strcmp(attName, "viewNodeRef"))
      {
      std::stringstream ss(attValue);
      while (!ss.eof())
        {
        std::string id;
        ss >> id;
        this->AddViewNodeID(id.c_str());
        }

      //this->Scene->AddReferencedNodeID(this->DisplayNodeID, this);
      }
    }
    this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLDisplayNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLDisplayNode *node = (vtkMRMLDisplayNode *) anode;

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

  const int ndnodes = node->GetNumberOfViewNodeIDs();
  for (int i=0; i<ndnodes; i++)
    {
    this->AddViewNodeID(node->ViewNodeIDs[i].c_str());
    }

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  int idx;

  Superclass::PrintSelf(os,indent);

  os << indent << "Color:             ";
  for (idx = 0; idx < 3; ++idx)
    {
    os << this->Color[idx] << ((idx == 2) ? "\n" : ", ");
    }
  os << indent << "SelectedColor:     ";
  for (idx = 0; idx < 3; ++idx)
    {
    os << this->SelectedColor[idx] << ((idx == 2) ? "\n" : ", ");
    }
  os << indent << "EdgeColor:         ";
  for (idx = 0; idx < 3; ++idx)
    {
    os << this->EdgeColor[idx] << ((idx == 2) ? "\n" : ", ");
    }
  os << indent << "SelectedAmbient:   " << this->SelectedAmbient << "\n";
  os << indent << "SelectedSpecular:  " << this->SelectedSpecular << "\n";
  os << indent << "Opacity:           " << this->Opacity << "\n";
  os << indent << "SliceIntersectionOpacity:           " << this->SliceIntersectionOpacity << "\n";
  os << indent << "Ambient:           " << this->Ambient << "\n";
  os << indent << "Diffuse:           " << this->Diffuse << "\n";
  os << indent << "Specular:          " << this->Specular << "\n";
  os << indent << "Power:             " << this->Power << "\n";
  os << indent << "Visibility:        " << this->Visibility << "\n";
  os << indent << "Visibility2D:      " << this->Visibility2D << "\n";
  os << indent << "Visibility3D:      " << this->Visibility3D << "\n";
  os << indent << "ScalarVisibility:  " << this->ScalarVisibility << "\n";
  os << indent << "VectorVisibility:  " << this->VectorVisibility << "\n";
  os << indent << "TensorVisibility:  " << this->TensorVisibility << "\n";
  os << indent << "InterpolateTexture:" << this->InterpolateTexture << "\n";
  os << indent << "ScalarRangeFlag:   " << this->GetScalarRangeFlagTypeAsString(this->ScalarRangeFlag) << "\n";
  os << indent << "BackfaceCulling:   " << this->BackfaceCulling << "\n";
  os << indent << "Clipping:          " << this->Clipping << "\n";
  os << indent << "SliceIntersectionThickness: " << this->SliceIntersectionThickness << "\n";

  os << indent << "ScalarRange:       ";
  for (idx = 0; idx < 2; ++idx)
    {
    os << this->ScalarRange[idx] << ((idx == 0) ? ", " : "\n");
    }
  os << indent << "ColorNodeID: " <<
    (this->ColorNodeID ? this->ColorNodeID : "(none)") << "\n";

  os << indent<< "ActiveScalarName: " <<
    (this->ActiveScalarName ? this->ActiveScalarName : "(none)") << "\n";

  for (unsigned int i=0; i<this->ViewNodeIDs.size(); i++)
    {
    os << indent << "ViewNodeIDs[" << i << "]: " <<
      this->ViewNodeIDs[i] << "\n";
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
void vtkMRMLDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
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

  this->ViewNodeIDs.push_back(std::string(viewNodeID));
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
  return this->GetNumberOfViewNodeIDs() == 0
    || this->IsViewNodeIDPresent(viewNodeID);
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
  if(b)
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
const char* vtkMRMLDisplayNode
::GetScalarRangeFlagTypeAsString(int flag)
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
void vtkMRMLDisplayNode::SetSliceIntersectionVisibility(int on)
{
  vtkWarningMacro("Deprecated, please use SetVisibility2D instead");
  this->SetVisibility2D(on);
}

//-----------------------------------------------------------
int vtkMRMLDisplayNode::GetSliceIntersectionVisibility()
{
  vtkDebugMacro("Deprecated, please use GetVisibility2D instead");
  return this->GetVisibility2D();
}

//-----------------------------------------------------------
void vtkMRMLDisplayNode::SliceIntersectionVisibilityOn()
{
  vtkWarningMacro("Deprecated, please use Visibility2DOn instead");
  this->Visibility2DOn();
}

//-----------------------------------------------------------
void vtkMRMLDisplayNode::SliceIntersectionVisibilityOff()
{
  vtkWarningMacro("Deprecated, please use Visibility2DOff instead");
  this->Visibility2DOff();
}
