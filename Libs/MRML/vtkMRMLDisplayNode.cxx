/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLDisplayNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLDisplayNode.h"
#include "vtkMRMLDisplayableNode.h"
#include "vtkMRMLScene.h"




//----------------------------------------------------------------------------
vtkMRMLDisplayNode::vtkMRMLDisplayNode()
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
  this->AutoScalarRange = 1;

  // Arrays
  this->ScalarRange[0] = 0;
  this->ScalarRange[1] = 100;

  this->TextureImageData = NULL;
  this->ColorNodeID = NULL;
  this->ColorNode = NULL;

  this->ActiveScalarName = NULL;

  // add observer to process visualization pipeline
  vtkEventBroker::GetInstance()->AddObservation( 
    this, vtkCommand::ModifiedEvent, this, this->MRMLCallbackCommand );

}

//----------------------------------------------------------------------------
vtkMRMLDisplayNode::~vtkMRMLDisplayNode()
{
  this->SetAndObserveTextureImageData(NULL);
  this->SetAndObserveColorNodeID( NULL);

}

//----------------------------------------------------------------------------
void vtkMRMLDisplayNode::WriteXML(ostream& of, int nIndent)
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

  of << indent << " autoScalarRange=\"" << (this->AutoScalarRange ? "true" : "false") << "\"";

  of << indent << " scalarRange=\"" << this->ScalarRange[0] << " "
     << this->ScalarRange[1] << "\"";

  if (this->ColorNodeID != NULL) 
    {
    of << indent << " colorNodeRef=\"" << this->ColorNodeID << "\"";
    }

  if (this->ActiveScalarName != NULL)
    {
    of << indent << " activeScalarName=\"" << this->ActiveScalarName << "\"";
    }

  of << " ";
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  if (this->ColorNodeID && !strcmp(oldID, this->ColorNodeID))
    {
    this->SetColorNodeID(newID);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayNode::ReadXMLAttributes(const char** atts)
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
    else if (!strcmp(attName, "colorNodeRef")) 
      {
      this->SetColorNodeID(attValue);
      //this->Scene->AddReferencedNodeID(this->ColorNodeID, this);
      }
    else if (!strcmp(attName, "activeScalarName"))
      {
      this->SetActiveScalarName(attValue);
      }

    }  
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLDisplayNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLDisplayNode *node = (vtkMRMLDisplayNode *) anode;

  this->DisableModifiedEventOn();

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
  this->SetVectorVisibility(node->VectorVisibility);
  this->SetTensorVisibility(node->TensorVisibility);
  this->SetAutoScalarRange(node->AutoScalarRange);
  this->SetBackfaceCulling(node->BackfaceCulling);
  this->SetClipping(node->Clipping);
  this->SetAndObserveTextureImageData(node->TextureImageData);
  this->SetColorNodeID(node->ColorNodeID);
  this->SetActiveScalarName(node->ActiveScalarName);

  this->DisableModifiedEventOff();
  this->InvokePendingModifiedEvent();

}

//----------------------------------------------------------------------------
void vtkMRMLDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
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
  os << indent << "VectorVisibility:  " << this->VectorVisibility << "\n";
  os << indent << "TensorVisibility:  " << this->TensorVisibility << "\n";
  os << indent << "AutoScalarRange:   " << this->AutoScalarRange << "\n";
  os << indent << "BackfaceCulling:   " << this->BackfaceCulling << "\n";
  os << indent << "Clipping:          " << this->Clipping << "\n";

  os << "ScalarRange:\n";
  for (idx = 0; idx < 2; ++idx)
    {
    os << indent << ", " << this->ScalarRange[idx];
    }
  os << endl;
  os << indent << "ColorNodeID: " <<
    (this->ColorNodeID ? this->ColorNodeID : "(none)") << "\n";

  os << indent<< "ActiveScalarName: " <<
    (this->ActiveScalarName ? this->ActiveScalarName : "(none)") << "\n";
}

//----------------------------------------------------------------------------
vtkMRMLDisplayableNode* vtkMRMLDisplayNode::GetDisplayableNode()
{
    int numNodes = this->Scene->GetNumberOfNodesByClass("vtkMRMLDisplayableNode");
    for (int i=0; i<numNodes; i++)
      {
      vtkMRMLDisplayableNode *model = vtkMRMLDisplayableNode::SafeDownCast(this->Scene->GetNthNodeByClass(i, "vtkMRMLDisplayableNode"));
      int ndnodes = model->GetNumberOfDisplayNodes();
      for (int k=0; k<ndnodes; k++)
        {
        const char *id = model->GetNthDisplayNodeID(k);
        if (id && !strcmp(id, this->GetID()))
          {
          return model;
          }
        }
      }
    return NULL;
}


//----------------------------------------------------------------------------
void vtkMRMLDisplayNode::SetAndObserveTextureImageData(vtkImageData *ImageData)
{
  if (this->TextureImageData != NULL)
    {
    vtkEventBroker::GetInstance()->RemoveObservations( 
      this->TextureImageData, vtkCommand::ModifiedEvent, this, this->MRMLCallbackCommand );
    }

  this->SetTextureImageData(ImageData);
  if (this->TextureImageData != NULL)
    {
    vtkEventBroker::GetInstance()->AddObservation( 
      this->TextureImageData, vtkCommand::ModifiedEvent, this, this->MRMLCallbackCommand );
    }
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

  if (this->ColorNodeID != NULL && this->Scene->GetNodeByID(this->ColorNodeID) == NULL)
    {
    this->SetAndObserveColorNodeID(NULL);
    }
}

//----------------------------------------------------------------------------
vtkMRMLColorNode* vtkMRMLDisplayNode::GetColorNode()
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
void vtkMRMLDisplayNode::SetAndObserveColorNodeID(const char *colorNodeID)
{
  vtkSetAndObserveMRMLObjectMacro(this->ColorNode, NULL);

  this->SetColorNodeID(colorNodeID);

  vtkMRMLNode *cnode = this->GetColorNode();

  vtkSetAndObserveMRMLObjectMacro(this->ColorNode, cnode);
}

void vtkMRMLDisplayNode::SetAndObserveColorNodeID(const std::string& colorNodeID)
{
  this->SetAndObserveColorNodeID( colorNodeID.c_str() );
}

//---------------------------------------------------------------------------
void vtkMRMLDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  if (this->TextureImageData != NULL && this->TextureImageData == vtkImageData::SafeDownCast(caller) &&
    event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent, NULL);
    }

  vtkMRMLColorNode *cnode = this->GetColorNode();
  if (cnode != NULL &&
      cnode == vtkMRMLColorNode::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent, NULL);
    }
  if (event ==  vtkCommand::ModifiedEvent)
    {
    this->UpdatePolyDataPipeline();
    this->UpdateImageDataPipeline();
    this->Modified();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLDisplayNode::SetActiveScalarName(const char *scalarName)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting ActiveScalarName to " << (scalarName ? scalarName : "(null)"));
  
  if (this->ActiveScalarName == NULL && scalarName == NULL)
    {
    return;
    }
  if (this->ActiveScalarName && scalarName && (!strcmp(this->ActiveScalarName,scalarName)))
    {
    return;
    }
  if (this->ActiveScalarName)
    {
    delete [] this->ActiveScalarName;
    }

  if (scalarName)
    {
    size_t n = strlen(scalarName) + 1;
    char *cp1 = new char[n];
    const char *cp2 = (scalarName);
    this->ActiveScalarName = cp1;
    do { *cp1++ = *cp2++; } while ( --n );
    }
  else
    {
    this->ActiveScalarName = NULL;
    this->Modified();
    return;
    }

  // is it an empty string?
  if (strcmp(scalarName,"") == 0)
    {
    vtkDebugMacro("SetActiveScalarName: scalar name is an emtpy string, not setting the color node on display node " << this->GetID());
    return;
    }

  // calls to SetAndObserveColorNodeID will set up the color table for
  // displaying these scalars
  
  this->Modified();
}
