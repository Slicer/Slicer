/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFiducialListDisplayNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLFiducialListDisplayNode.h"
#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkMRMLFiducialListDisplayNode* vtkMRMLFiducialListDisplayNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFiducialListDisplayNode");
  if(ret)
    {
    return (vtkMRMLFiducialListDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLFiducialListDisplayNode;
}

//-----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLFiducialListDisplayNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFiducialListDisplayNode");
  if(ret)
    {
    return (vtkMRMLFiducialListDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLFiducialListDisplayNode;
}


//----------------------------------------------------------------------------
vtkMRMLFiducialListDisplayNode::vtkMRMLFiducialListDisplayNode()
{

  // Strings


  // Numbers
  this->Opacity = 1.0;
  this->Visibility = 1;
  
  this->Ambient = 0;
  this->Diffuse = 1.0;
  this->Specular = 0;
  this->Power = 1;
  
  // Arrays
  this->Color[0] = 0.5;
  this->Color[1] = 0.5;
  this->Color[2] = 0.5;
}

//----------------------------------------------------------------------------
vtkMRMLFiducialListDisplayNode::~vtkMRMLFiducialListDisplayNode()
{

}

//----------------------------------------------------------------------------
void vtkMRMLFiducialListDisplayNode::WriteXML(ostream& of, int nIndent)
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
}

//----------------------------------------------------------------------------
void vtkMRMLFiducialListDisplayNode::ReadXMLAttributes(const char** atts)
{

  vtkMRMLNode::ReadXMLAttributes(atts);

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
      std::stringstream ss;
      ss << attValue;
      ss >> Visibility;
      }
    }  
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLFiducialListDisplayNode::Copy(vtkMRMLNode *anode)
{
  vtkMRMLNode::Copy(anode);
  vtkMRMLFiducialListDisplayNode *node = (vtkMRMLFiducialListDisplayNode *) anode;

  // Strings

  this->SetColor(node->Color);

  // Vectors

  
  // Numbers
  this->SetOpacity(node->Opacity);
  this->SetAmbient(node->Ambient);
  this->SetDiffuse(node->Diffuse);
  this->SetSpecular(node->Specular);
  this->SetPower(node->Power);
  this->SetVisibility(node->Visibility);
}

//----------------------------------------------------------------------------
void vtkMRMLFiducialListDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  int idx;
  
  vtkMRMLNode::PrintSelf(os,indent);

  os << indent << "Color:             " << this->Color[0] << ", " << this->Color[1] << ", " << this->Color[2] << "\n";
  os << indent << "Opacity:           " << this->Opacity << "\n";
  os << indent << "Ambient:           " << this->Ambient << "\n";
  os << indent << "Diffuse:           " << this->Diffuse << "\n";
  os << indent << "Specular:          " << this->Specular << "\n";
  os << indent << "Power:             " << this->Power << "\n";
  os << indent << "Visibility:        " << this->Visibility << "\n";

}


//---------------------------------------------------------------------------
void vtkMRMLFiducialListDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
/*
  if (this->TextureImageData == vtkImageData::SafeDownCast(caller) &&
    event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent, NULL);
    }
*/
  return;
}
