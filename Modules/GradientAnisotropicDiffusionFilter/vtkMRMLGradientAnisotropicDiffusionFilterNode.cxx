/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLGradientAnisotropicDiffusionFilterNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <ostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkMRMLGradientAnisotropicDiffusionFilterNode.h"
#include "vtkMRMLScene.h"


//------------------------------------------------------------------------------
vtkMRMLGradientAnisotropicDiffusionFilterNode* vtkMRMLGradientAnisotropicDiffusionFilterNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLGradientAnisotropicDiffusionFilterNode");
  if(ret)
    {
      return (vtkMRMLGradientAnisotropicDiffusionFilterNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLGradientAnisotropicDiffusionFilterNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLGradientAnisotropicDiffusionFilterNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLGradientAnisotropicDiffusionFilterNode");
  if(ret)
    {
      return (vtkMRMLGradientAnisotropicDiffusionFilterNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLGradientAnisotropicDiffusionFilterNode;
}

//----------------------------------------------------------------------------
vtkMRMLGradientAnisotropicDiffusionFilterNode::vtkMRMLGradientAnisotropicDiffusionFilterNode()
{
   this->Conductance = 1.0;
   this->NumberOfIterations = 1;
   this->TimeStep = 0.1;
}

//----------------------------------------------------------------------------
vtkMRMLGradientAnisotropicDiffusionFilterNode::~vtkMRMLGradientAnisotropicDiffusionFilterNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLGradientAnisotropicDiffusionFilterNode::WriteXML(ostream& of, int nIndent)
{
Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  {
    std::stringstream ss;
    ss << this->Conductance;
    of << indent << "Conductance='" << ss.str() << "' ";
  }
  {
    std::stringstream ss;
    ss << this->NumberOfIterations;
    of << indent << "NumberOfIterations='" << ss.str() << "' ";
  }
  {
    std::stringstream ss;
    ss << this->TimeStep;
    of << indent << "TimeStep='" << ss.str() << "' ";
  }
}

//----------------------------------------------------------------------------
void vtkMRMLGradientAnisotropicDiffusionFilterNode::ReadXMLAttributes(const char** atts)
{

  vtkMRMLNode::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "Conductance")) {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Conductance;
    }
    else if (!strcmp(attName, "NumberOfIterations")) {
      std::stringstream ss;
      ss << attValue;
      ss >> this->NumberOfIterations;
    }
    else if (!strcmp(attName, "TimeStep")) {
      std::stringstream ss;
      ss << attValue;
      ss >> this->TimeStep;
    }
  }
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLGradientAnisotropicDiffusionFilterNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLGradientAnisotropicDiffusionFilterNode *node = (vtkMRMLGradientAnisotropicDiffusionFilterNode *) anode;

  this->SetConductance(node->Conductance);
  this->SetNumberOfIterations(node->NumberOfIterations);
  this->SetTimeStep(node->TimeStep);
}

//----------------------------------------------------------------------------
void vtkMRMLGradientAnisotropicDiffusionFilterNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  vtkMRMLNode::PrintSelf(os,indent);

  os << indent << "Conductance:   " << this->Conductance << "\n";
  os << indent << "NumberOfIterations:   " << this->NumberOfIterations << "\n";
  os << indent << "TimeStep:   " << this->TimeStep << "\n";
}

