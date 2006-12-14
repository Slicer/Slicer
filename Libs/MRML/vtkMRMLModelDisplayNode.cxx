/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLModelDisplayNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkMRMLModelDisplayNode* vtkMRMLModelDisplayNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLModelDisplayNode");
  if(ret)
    {
    return (vtkMRMLModelDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLModelDisplayNode;
}

//-----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLModelDisplayNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLModelDisplayNode");
  if(ret)
    {
    return (vtkMRMLModelDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLModelDisplayNode;
}


//----------------------------------------------------------------------------
vtkMRMLModelDisplayNode::vtkMRMLModelDisplayNode()
{

  // Strings
  this->Color[0] = 0.5;
  this->Color[1] = 0.5;
  this->Color[2] = 0.5;

  // Numbers
  this->Opacity = 1.0;
  this->Visibility = 1;
  this->Clipping = 0;
  this->BackfaceCulling = 1;
  this->ScalarVisibility = 0;
  this->VectorVisibility = 0;
  this->TensorVisibility = 0;
  
  this->Ambient = 0;
  this->Diffuse = 1.0;
  this->Specular = 0;
  this->Power = 1;
  
  // Arrays
  this->ScalarRange[0] = 0;
  this->ScalarRange[1] = 100;

  this->TextureImageData = NULL;
  this->ColorNodeID = NULL;
  this->ColorNode = NULL;

}

//----------------------------------------------------------------------------
vtkMRMLModelDisplayNode::~vtkMRMLModelDisplayNode()
{
  this->SetAndObserveTextureImageData(NULL);
  this->SetAndObserveColorNodeID( NULL);

}

//----------------------------------------------------------------------------
void vtkMRMLModelDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  if (this->Color)
    {
    of << indent << " color=\"" << this->Color[0] << " "
      << this->Color[1] << " "
      << this->Color[2] << "\"";
    }

  of << indent << " ambient=\"" << this->Ambient << "\"";

  of << indent << " diffuse=\"" << this->Diffuse << "\"";

  of << indent << " specular=\"" << this->Specular << "\"";

  of << indent << " power=\"" << this->Power << "\"";

  of << indent << " opacity=\"" << this->Opacity << "\"";

  of << indent << " visibility=\"" << (this->Visibility ? "true" : "false") << "\"";

  of << indent << " clipping=\"" << (this->Clipping ? "true" : "false") << "\"";

  of << indent << " backfaceCulling=\"" << (this->BackfaceCulling ? "true" : "false") << "\"";

  of << indent << " scalarVisibility=\"" << (this->ScalarVisibility ? "true" : "false") << "\"";

  of << indent << " vectorVisibility=\"" << (this->VectorVisibility ? "true" : "false") << "\"";

  of << indent << " tensorVisibility=\"" << (this->TensorVisibility ? "true" : "false") << "\"";

  of << indent << " scalarRange=\"" << this->ScalarRange[0] << " "
     << this->ScalarRange[1] << "\"";

  if (this->ColorNodeID != NULL) 
    {
    of << indent << "colorNodeRef=\"" << this->ColorNodeID << "\" ";
    }

}

//----------------------------------------------------------------------------
void vtkMRMLModelDisplayNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  if (!strcmp(oldID, this->ColorNodeID))
    {
    this->SetColorNodeID(newID);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLModelDisplayNode::ReadXMLAttributes(const char** atts)
{

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
    else if (!strcmp(attName, "colorNodeRef")) 
      {
      this->SetColorNodeID(attValue);
      this->Scene->AddReferencedNodeID(this->ColorNodeID, this);
      }

    }  
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLModelDisplayNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLModelDisplayNode *node = (vtkMRMLModelDisplayNode *) anode;

  // Strings

  this->SetColor(node->Color);

  // Vectors
  this->SetScalarRange(node->ScalarRange);
  
  // Numbers
  this->SetOpacity(node->Opacity);
  this->SetAmbient(node->Ambient);
  this->SetDiffuse(node->Diffuse);
  this->SetSpecular(node->Specular);
  this->SetPower(node->Power);
  this->SetVisibility(node->Visibility);
  this->SetScalarVisibility(node->ScalarVisibility);
  this->SetBackfaceCulling(node->BackfaceCulling);
  this->SetClipping(node->Clipping);
  this->SetAndObserveTextureImageData(node->TextureImageData);
  this->SetColorNodeID(node->ColorNodeID);

}

//----------------------------------------------------------------------------
void vtkMRMLModelDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  int idx;
  
  Superclass::PrintSelf(os,indent);

  os << indent << "Color:             " << this->Color << "\n";
  os << indent << "Opacity:           " << this->Opacity << "\n";
  os << indent << "Ambient:           " << this->Ambient << "\n";
  os << indent << "Diffuse:           " << this->Diffuse << "\n";
  os << indent << "Specular:          " << this->Specular << "\n";
  os << indent << "Power:             " << this->Power << "\n";
  os << indent << "Visibility:        " << this->Visibility << "\n";
  os << indent << "ScalarVisibility:  " << this->ScalarVisibility << "\n";
  os << indent << "BackfaceCulling:   " << this->BackfaceCulling << "\n";
  os << indent << "Clipping:          " << this->Clipping << "\n";

  os << "ScalarRange:\n";
  for (idx = 0; idx < 2; ++idx)
    {
    os << indent << ", " << this->ScalarRange[idx];
    }
  os << indent << "ColorNodeID: " <<
    (this->ColorNodeID ? this->ColorNodeID : "(none)") << "\n";

}

//----------------------------------------------------------------------------
void vtkMRMLModelDisplayNode::SetAndObserveTextureImageData(vtkImageData *ImageData)
{
  if (this->TextureImageData != NULL)
    {
    this->TextureImageData->RemoveObservers ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
    }

  this->SetTextureImageData(ImageData);
  if (this->TextureImageData != NULL)
    {
    this->TextureImageData->AddObserver ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
    }
}


//-----------------------------------------------------------
void vtkMRMLModelDisplayNode::UpdateScene(vtkMRMLScene *scene)
{
   Superclass::UpdateScene(scene);

   this->SetAndObserveColorNodeID(this->GetColorNodeID());
}

//-----------------------------------------------------------
void vtkMRMLModelDisplayNode::UpdateReferences()
{
   Superclass::UpdateReferences();

  if (this->ColorNodeID != NULL && this->Scene->GetNodeByID(this->ColorNodeID) == NULL)
    {
    this->SetAndObserveColorNodeID(NULL);
    }
}

//----------------------------------------------------------------------------
vtkMRMLColorNode* vtkMRMLModelDisplayNode::GetColorNode()
{
  vtkMRMLColorNode* node = NULL;
  if (this->GetScene() && this->GetColorNodeID() )
    {
    vtkMRMLNode* cnode = this->GetScene()->GetNodeByID(this->ColorNodeID);
    node = vtkMRMLColorNode::SafeDownCast(cnode);
    }
  return node;
}

//----------------------------------------------------------------------------
void vtkMRMLModelDisplayNode::SetAndObserveColorNodeID(const char *colorNodeID)
{
  vtkSetAndObserveMRMLObjectMacro(this->ColorNode, NULL);

  this->SetColorNodeID(colorNodeID);

  vtkMRMLColorNode *cnode = this->GetColorNode();

  vtkSetAndObserveMRMLObjectMacro(this->ColorNode, cnode);

}

//---------------------------------------------------------------------------
void vtkMRMLModelDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  if (this->TextureImageData == vtkImageData::SafeDownCast(caller) &&
    event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent, NULL);
    return;
    }

  vtkMRMLColorNode *cnode = this->GetColorNode();
  if (cnode != NULL && cnode == vtkMRMLColorNode::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent, NULL);
    }
  return;
}
