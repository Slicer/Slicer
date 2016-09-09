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
vtkCxxSetReferenceStringMacro(vtkMRMLDisplayNode, ActiveScalarName);

//----------------------------------------------------------------------------
vtkMRMLDisplayNode::vtkMRMLDisplayNode()
{
  this->HideFromEditors = 1;

  this->Opacity = 1.0;
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
  this->EdgeVisibility = 0;
  this->Clipping = 0;
  this->SliceIntersectionVisibility = 0;
  this->SliceIntersectionThickness = 1;
  this->FrontfaceCulling = 0;
  this->BackfaceCulling = 1;
  this->ScalarVisibility = 0;
  this->VectorVisibility = 0;
  this->TensorVisibility = 0;
  this->InterpolateTexture = 0;
  this->ScalarRangeFlag = this->UseColorNodeScalarRange;
  this->AutoScalarRange = 1;

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

  this->TextureImageDataConnection = NULL;
  this->ColorNodeID = NULL;
  this->ColorNode = NULL;

  this->ActiveScalarName = NULL;
  this->ActiveAttributeLocation = vtkAssignAttribute::POINT_DATA;

  // add observer to process visualization pipeline
  vtkEventBroker::GetInstance()->AddObservation(
    this, vtkCommand::ModifiedEvent, this, this->MRMLCallbackCommand );
}

//----------------------------------------------------------------------------
vtkMRMLDisplayNode::~vtkMRMLDisplayNode()
{
  this->SetTextureImageDataConnection(NULL);
  this->SetActiveScalarName(NULL);
  this->SetAndObserveColorNodeID( NULL);
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << " color=\"" << this->Color[0] << " "
    << this->Color[1] << " "
    << this->Color[2] << "\"";

  of << indent << " edgeColor=\"" << this->EdgeColor[0] << " "
    << this->EdgeColor[1] << " " << this->EdgeColor[2] << "\"";
  of << indent << " selectedColor=\"" << this->SelectedColor[0] << " "
    << this->SelectedColor[1] << " "
    << this->SelectedColor[2] << "\"";

  of << indent << " selectedAmbient=\"" << this->SelectedAmbient << "\"";

  of << indent << " ambient=\"" << this->Ambient << "\"";

  of << indent << " diffuse=\"" << this->Diffuse << "\"";

  of << indent << " selectedSpecular=\"" << this->SelectedSpecular << "\"";

  of << indent << " specular=\"" << this->Specular << "\"";

  of << indent << " power=\"" << this->Power << "\"";

  of << indent << " opacity=\"" << this->Opacity << "\"";

  of << indent << " pointSize=\"" << this->PointSize << "\"";
  of << indent << " lineWidth=\"" << this->LineWidth << "\"";
  of << indent << " representation=\"" << this->Representation << "\"";
  of << indent << " lighting=\"" << (this->Lighting? "true" : "false") << "\"";
  of << indent << " interpolation=\"" << this->Interpolation << "\"";
  of << indent << " shading=\"" << (this->Shading? "true" : "false") << "\"";

  of << indent << " visibility=\"" << (this->Visibility ? "true" : "false") << "\"";
  of << indent << " edgeVisibility=\"" << (this->EdgeVisibility? "true" : "false") << "\"";
  of << indent << " clipping=\"" << (this->Clipping ? "true" : "false") << "\"";

  of << indent << " sliceIntersectionVisibility=\"" << (this->SliceIntersectionVisibility ? "true" : "false") << "\"";

  of << indent << " sliceIntersectionThickness=\"" << this->SliceIntersectionThickness << "\"";

  of << indent << " frontfaceCulling=\"" << (this->FrontfaceCulling ? "true" : "false") << "\"";
  of << indent << " backfaceCulling=\"" << (this->BackfaceCulling ? "true" : "false") << "\"";

  of << indent << " scalarVisibility=\"" << (this->ScalarVisibility ? "true" : "false") << "\"";

  of << indent << " vectorVisibility=\"" << (this->VectorVisibility ? "true" : "false") << "\"";

  of << indent << " tensorVisibility=\"" << (this->TensorVisibility ? "true" : "false") << "\"";

  of << indent << " interpolateTexture=\"" << (this->InterpolateTexture ? "true" : "false") << "\"";

  of << indent << " scalarRangeFlag=\"" << this->ScalarRangeFlag << "\"";

  of << indent << " autoScalarRange=\"" << (this->AutoScalarRange ? "true" : "false") << "\"";

  of << indent << " scalarRange=\"" << this->ScalarRange[0] << " "
     << this->ScalarRange[1] << "\"";

  if (this->ColorNodeID != NULL)
    {
    of << indent << " colorNodeID=\"" << this->ColorNodeID << "\"";
    }

  if (this->ActiveScalarName != NULL)
    {
    of << indent << " activeScalarName=\"" << this->ActiveScalarName << "\"";
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
    of << indent << " viewNodeRef=\"" << ss.str().c_str() << "\"";
    }

  of << " ";
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayNode::SetSceneReferences()
{
  this->Superclass::SetSceneReferences();
  if (this->Scene == NULL)
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
  while (*atts != NULL)
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
        this->SliceIntersectionVisibility = 1;
        }
      else
        {
        this->SliceIntersectionVisibility = 0;
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
      this->SetScalarRangeFlag(atoi(attValue));
      }
    else if (!strcmp(attName, "autoScalarRange"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->AutoScalarRange = 1;
        }
      else
        {
        this->AutoScalarRange = 0;
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
  this->SetAmbient(node->Ambient);
  this->SetDiffuse(node->Diffuse);
  this->SetSpecular(node->Specular);
  this->SetPower(node->Power);
  this->SetVisibility(node->Visibility);
  this->SetScalarVisibility(node->ScalarVisibility);
  this->SetVectorVisibility(node->VectorVisibility);
  this->SetTensorVisibility(node->TensorVisibility);
  this->SetInterpolateTexture(node->InterpolateTexture);
  this->SetScalarRangeFlag(node->ScalarRangeFlag);
  this->SetAutoScalarRange(node->AutoScalarRange);
  this->SetBackfaceCulling(node->BackfaceCulling);
  this->SetClipping(node->Clipping);
  this->SetSliceIntersectionVisibility(node->SliceIntersectionVisibility);
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

  os << indent << "Color:             " << this->Color << "\n";
  os << indent << "SelectedColor:     " << this->SelectedColor << "\n";
  os << indent << "SelectedAmbient:   " << this->SelectedAmbient << "\n";
  os << indent << "SelectedSpecular:  " << this->SelectedSpecular << "\n";
  os << indent << "Opacity:           " << this->Opacity << "\n";
  os << indent << "Ambient:           " << this->Ambient << "\n";
  os << indent << "Diffuse:           " << this->Diffuse << "\n";
  os << indent << "Specular:          " << this->Specular << "\n";
  os << indent << "Power:             " << this->Power << "\n";
  os << indent << "Visibility:        " << this->Visibility << "\n";
  os << indent << "ScalarVisibility:  " << this->ScalarVisibility << "\n";
  os << indent << "VectorVisibility:  " << this->VectorVisibility << "\n";
  os << indent << "TensorVisibility:  " << this->TensorVisibility << "\n";
  os << indent << "InterpolateTexture:" << this->InterpolateTexture << "\n";
  os << indent << "ScalarRangeFlag:   " << this->ScalarRangeFlag << "\n";
  os << indent << "AutoScalarRange:   " << this->AutoScalarRange << "\n";
  os << indent << "BackfaceCulling:   " << this->BackfaceCulling << "\n";
  os << indent << "Clipping:          " << this->Clipping << "\n";
  os << indent << "SliceIntersectionVisibility: " << this->SliceIntersectionVisibility << "\n";
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
  if (this->Scene == NULL)
    {
    return NULL;
    }
  // It is an expensive operation to determine the displayable node
  // (need to iterate through the scene), so the last found value
  // is cached. If it is still valid then we use it.
  if (this->LastFoundDisplayableNode != NULL)
    {
    if (this->LastFoundDisplayableNode->GetScene() == this->Scene
      && this->LastFoundDisplayableNode->HasDisplayNodeID(this->GetID()))
      {
      return this->LastFoundDisplayableNode;
      }
    }
  vtkMRMLNode* node = NULL;
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
  this->LastFoundDisplayableNode = NULL;
  return NULL;
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
    this->TextureImageDataConnection->GetProducer() : 0;

  this->TextureImageDataConnection = newTextureImageDataConnection;

  vtkAlgorithm* textureImageDataAlgorithm = this->TextureImageDataConnection ?
    this->TextureImageDataConnection->GetProducer() : 0;

  if (textureImageDataAlgorithm != NULL)
    {
    vtkEventBroker::GetInstance()->AddObservation(
      this->TextureImageDataConnection, vtkCommand::ModifiedEvent,
      this, this->MRMLCallbackCommand );
    textureImageDataAlgorithm->Register(this);
    }

  if (oldTextureImageDataAlgorithm != NULL)
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

  if (this->Scene == NULL)
    {
    return;
    }

  if (this->ColorNodeID != NULL && this->Scene->GetNodeByID(this->ColorNodeID) == NULL)
    {
    this->SetAndObserveColorNodeID(NULL);
    }

  std::vector< std::string > viewNodeIDs;

  for (unsigned int i=0; i < this->ViewNodeIDs.size(); i++)
    {
    if (this->Scene->GetNodeByID(this->ViewNodeIDs[i]) != NULL)
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
  vtkMRMLColorNode* cnode = NULL;
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
  vtkMRMLColorNode* cnode = NULL;
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

  if (this->TextureImageDataConnection != NULL &&
      this->TextureImageDataConnection == vtkAlgorithmOutput::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent, NULL);
    }

  vtkMRMLColorNode* cnode = vtkMRMLColorNode::SafeDownCast(caller);
  if (cnode != NULL &&
      this->ColorNodeID != NULL &&
      strcmp(this->ColorNodeID, cnode->GetID()) == 0 &&
      event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent, NULL);
    }
}

//-------------------------------------------------------
void vtkMRMLDisplayNode::AddViewNodeID(const char* viewNodeID)
{
  if (viewNodeID == 0)
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
  if (viewNodeID == NULL)
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
  if (viewNodeIDs.size() != this->ViewNodeIDs.size() && this->Scene!=NULL)
    {
    this->Scene->RemoveReferencedNodeID(viewNodeID, this);
    this->ViewNodeIDs = viewNodeIDs;
    this->Modified();
    }
}

//-------------------------------------------------------
void vtkMRMLDisplayNode::RemoveAllViewNodeIDs()
{
  if (this->Scene!=NULL)
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
    return NULL;
    }
  return ViewNodeIDs[index].c_str();
}

//-------------------------------------------------------
bool vtkMRMLDisplayNode::IsViewNodeIDPresent(const char* viewNodeID)const
{
  if (viewNodeID == 0)
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
  if (viewNodeID == 0)
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
