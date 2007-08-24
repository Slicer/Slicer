/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVectorVolumeDisplayNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLVectorVolumeDisplayNode.h"
#include "vtkMRMLScene.h"


//------------------------------------------------------------------------------
vtkMRMLVectorVolumeDisplayNode* vtkMRMLVectorVolumeDisplayNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLVectorVolumeDisplayNode");
  if(ret)
    {
    return (vtkMRMLVectorVolumeDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLVectorVolumeDisplayNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLVectorVolumeDisplayNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLVectorVolumeDisplayNode");
  if(ret)
    {
    return (vtkMRMLVectorVolumeDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLVectorVolumeDisplayNode;
}

//----------------------------------------------------------------------------
vtkMRMLVectorVolumeDisplayNode::vtkMRMLVectorVolumeDisplayNode()
{
 this->VisualizationMode = this->visModeScalar;
 this->ScalarMode = this->scalarModeMagnitude;
 this->GlyphMode = this->glyphModeLines;
}

//----------------------------------------------------------------------------
vtkMRMLVectorVolumeDisplayNode::~vtkMRMLVectorVolumeDisplayNode()
{

}

//----------------------------------------------------------------------------
void vtkMRMLVectorVolumeDisplayNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  std::stringstream ss;

  ss.clear();
  ss << this->ScalarMode;
  of << indent << " scalarMode=\"" << ss.str() << "\"";

  ss.clear();
  ss << this->GlyphMode;
  of << indent << " glyphMode=\"" << ss.str() << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLVectorVolumeDisplayNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "scalarMode"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->ScalarMode;
      }
    else if (!strcmp(attName, "glyphMode")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->GlyphMode;
      }
    }        
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLVectorVolumeDisplayNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLVectorVolumeDisplayNode *node = (vtkMRMLVectorVolumeDisplayNode *) anode;

  this->SetScalarMode(node->ScalarMode);
  this->SetGlyphMode(node->GlyphMode);
}

//----------------------------------------------------------------------------
void vtkMRMLVectorVolumeDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);

  os << indent << "Scalar Mode:   " << this->ScalarMode << "\n";
  os << indent << "Glyph Mode:    " << this->GlyphMode << "\n";
}



