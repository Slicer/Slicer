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
#include <iostream>
#include <sstream>
#include <map>

#include "vtkObjectFactory.h"

#include "vtkMRMLCurveAnalysisNode.h"
#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkMRMLCurveAnalysisNode* vtkMRMLCurveAnalysisNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLCurveAnalysisNode"); if(ret)
    {
      return (vtkMRMLCurveAnalysisNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLCurveAnalysisNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLCurveAnalysisNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLCurveAnalysisNode");
  if(ret)
    {
      return (vtkMRMLCurveAnalysisNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLCurveAnalysisNode;
}

//----------------------------------------------------------------------------
vtkMRMLCurveAnalysisNode::vtkMRMLCurveAnalysisNode()
{
  this->HideFromEditors = true;

  this->TargetCurve = NULL;
  this->FittedCurve = NULL;
  this->OutputValues.clear();
  this->Constants.clear();
  this->InitialParameters.clear();
  this->InputArrays.clear();
}

//----------------------------------------------------------------------------
vtkMRMLCurveAnalysisNode::~vtkMRMLCurveAnalysisNode()
{
}


//----------------------------------------------------------------------------
void vtkMRMLCurveAnalysisNode::WriteXML(ostream& of, int nIndent)
{
  // Start by having the superclass write its information
  Superclass::WriteXML(of, nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLCurveAnalysisNode::ReadXMLAttributes(const char** atts)
{
  vtkMRMLNode::ReadXMLAttributes(atts);
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLCurveAnalysisNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLCurveAnalysisNode *node = (vtkMRMLCurveAnalysisNode *) anode;
}


//----------------------------------------------------------------------------
void vtkMRMLCurveAnalysisNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLNode::PrintSelf(os,indent);
}


//----------------------------------------------------------------------------
void vtkMRMLCurveAnalysisNode::SetInputArray(const char* name, vtkDoubleArray* data)
{
  std::cerr << "void vtkMRMLCurveAnalysisNode::SetInputArray(const char* name, vtkDoubleArray* data)" << std::endl;
  std::cerr << name << std::endl;
  this->InputArrays[name] = data;
}


//----------------------------------------------------------------------------
vtkDoubleArray* vtkMRMLCurveAnalysisNode::GetInputArray(const char* name)
{
  ArrayMapType::iterator iter;
  iter = this->InputArrays.find(name);
  if (iter != this->InputArrays.end())
    {
    return iter->second;
    }
  else
    {
    return NULL;
    }
}

//----------------------------------------------------------------------------
vtkStringArray* vtkMRMLCurveAnalysisNode::GetInputArrayNameArray()
{
  ArrayMapType::iterator iter;
  vtkStringArray* array = vtkStringArray::New();

  for (iter = this->InputArrays.begin(); iter != this->InputArrays.end(); iter ++)
    {
    array->InsertNextValue(iter->first.c_str());
    }
  return array;
}


//----------------------------------------------------------------------------
void vtkMRMLCurveAnalysisNode::ClearInputArrays()
{
  this->InputArrays.clear();
}


//----------------------------------------------------------------------------
void vtkMRMLCurveAnalysisNode::SetInitialParameter(const char* name, double value)
{
  this->InitialParameters[name] = value;
}


//----------------------------------------------------------------------------
double vtkMRMLCurveAnalysisNode::GetInitialParameter(const char* name)
{
  ValueMapType::iterator iter;
  iter = this->InitialParameters.find(name);
  if (iter != this->InitialParameters.end())
    {
    return iter->second;
    }
  else
    {
    return 0.0;
    }
}


//----------------------------------------------------------------------------
vtkStringArray* vtkMRMLCurveAnalysisNode::GetInitialParameterNameArray()
{
  ValueMapType::iterator iter;
  vtkStringArray* array = vtkStringArray::New();

  for (iter = this->InitialParameters.begin(); iter != this->InitialParameters.end(); iter ++)
    {
    array->InsertNextValue(iter->first.c_str());
    }

  return array;
}


//----------------------------------------------------------------------------
void vtkMRMLCurveAnalysisNode::ClearInitialParameters()
{
  this->InitialParameters.clear();
}


//----------------------------------------------------------------------------
void vtkMRMLCurveAnalysisNode::SetConstant(const char *name, const double value)
{
  this->Constants[name] = value;
}


//----------------------------------------------------------------------------
double vtkMRMLCurveAnalysisNode::GetConstant(const char* name)
{
  ValueMapType::iterator iter;
  iter = this->Constants.find(name);
  if (iter != this->Constants.end())
    {
    return this->Constants[name]; 
    }
  else
    {
    return 0.0;
    }
}


//----------------------------------------------------------------------------
vtkStringArray* vtkMRMLCurveAnalysisNode::GetConstantNameArray()
{
  ValueMapType::iterator iter;
  vtkStringArray* array = vtkStringArray::New();
  
  for (iter = this->Constants.begin(); iter != this->Constants.end(); iter ++)
    {
    array->InsertNextValue(iter->first.c_str());
    }

  return array;
}


//----------------------------------------------------------------------------
void vtkMRMLCurveAnalysisNode::ClearConstants()
{
  this->Constants.clear();
}



//----------------------------------------------------------------------------
void vtkMRMLCurveAnalysisNode::SetOutputValue(const char *name, const double value)
{
  this->OutputValues[name] = value;
}


//----------------------------------------------------------------------------
double vtkMRMLCurveAnalysisNode::GetOutputValue(const char* name)
{
  ValueMapType::iterator iter;
  iter = this->OutputValues.find(name);
  if (iter != this->OutputValues.end())
    {
    return this->OutputValues[name]; 
    }
  else
    {
    return 0.0;
    }
}


//----------------------------------------------------------------------------
vtkStringArray* vtkMRMLCurveAnalysisNode::GetOutputValueNameArray()
{
  ValueMapType::iterator iter;
  vtkStringArray* array = vtkStringArray::New();

  for (iter = this->OutputValues.begin(); iter != this->OutputValues.end(); iter ++)
    {
    array->InsertNextValue(iter->first.c_str());
    }

  return array;
}



//----------------------------------------------------------------------------
void vtkMRMLCurveAnalysisNode::ClearOutputValues()
{
  this->OutputValues.clear();
}



