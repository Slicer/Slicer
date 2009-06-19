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
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLCurveAnalysisNode");
  if(ret)
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
  this->SourceData = NULL;
  this->FittedData = NULL;
  this->OutputParameters.clear();
  this->InputParameters.clear();
  this->InitialOptimParameters.clear();
  this->InputData.clear();
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
void vtkMRMLCurveAnalysisNode::SetInputData(const char* name, vtkDoubleArray* data)
{
  std::cerr << "void vtkMRMLCurveAnalysisNode::SetInputData(const char* name, vtkDoubleArray* data)" << std::endl;
  std::cerr << name << std::endl;
  this->InputData[name] = data;
}


//----------------------------------------------------------------------------
vtkDoubleArray* vtkMRMLCurveAnalysisNode::GetInputData(const char* name)
{
  ArrayMapType::iterator iter;
  iter = this->InputData.find(name);
  if (iter != this->InputData.end())
    {
    return iter->second;
    }
  else
    {
    return NULL;
    }
}

//----------------------------------------------------------------------------
vtkStringArray* vtkMRMLCurveAnalysisNode::GetInputDataNameArray()
{
  ArrayMapType::iterator iter;
  vtkStringArray* array = vtkStringArray::New();

  for (iter = this->InputData.begin(); iter != this->InputData.end(); iter ++)
    {
    array->InsertNextValue(iter->first.c_str());
    }
  return array;
}


//----------------------------------------------------------------------------
void vtkMRMLCurveAnalysisNode::ClearInputData()
{
  this->InputData.clear();
}


//----------------------------------------------------------------------------
void vtkMRMLCurveAnalysisNode::SetInitialOptimParameter(const char* name, double value)
{
  this->InitialOptimParameters[name] = value;
}


//----------------------------------------------------------------------------
double vtkMRMLCurveAnalysisNode::GetInitialOptimParameter(const char* name)
{
  ValueMapType::iterator iter;
  iter = this->InitialOptimParameters.find(name);
  if (iter != this->InitialOptimParameters.end())
    {
    return iter->second;
    }
  else
    {
    return 0.0;
    }
}


//----------------------------------------------------------------------------
vtkStringArray* vtkMRMLCurveAnalysisNode::GetInitialOptimParameterNameArray()
{
  ValueMapType::iterator iter;
  vtkStringArray* array = vtkStringArray::New();

  for (iter = this->InitialOptimParameters.begin(); iter != this->InitialOptimParameters.end(); iter ++)
    {
    array->InsertNextValue(iter->first.c_str());
    }

  return array;
}


//----------------------------------------------------------------------------
void vtkMRMLCurveAnalysisNode::ClearInitialOptimParameters()
{
  this->InitialOptimParameters.clear();
}


//----------------------------------------------------------------------------
void vtkMRMLCurveAnalysisNode::SetInputParameter(const char *name, const double value)
{
  this->InputParameters[name] = value;
}


//----------------------------------------------------------------------------
double vtkMRMLCurveAnalysisNode::GetInputParameter(const char* name)
{
  ValueMapType::iterator iter;
  iter = this->InputParameters.find(name);
  if (iter != this->InputParameters.end())
    {
    return this->InputParameters[name]; 
    }
  else
    {
    return 0.0;
    }
}


//----------------------------------------------------------------------------
vtkStringArray* vtkMRMLCurveAnalysisNode::GetInputParameterNameArray()
{
  ValueMapType::iterator iter;
  vtkStringArray* array = vtkStringArray::New();
  
  for (iter = this->InputParameters.begin(); iter != this->InputParameters.end(); iter ++)
    {
    array->InsertNextValue(iter->first.c_str());
    }

  return array;
}


//----------------------------------------------------------------------------
void vtkMRMLCurveAnalysisNode::ClearInputParameters()
{
  this->InputParameters.clear();
}



//----------------------------------------------------------------------------
void vtkMRMLCurveAnalysisNode::SetParameter(const char *name, const double value)
{
  this->OutputParameters[name] = value;
}


//----------------------------------------------------------------------------
double vtkMRMLCurveAnalysisNode::GetParameter(const char* name)
{
  ValueMapType::iterator iter;
  iter = this->OutputParameters.find(name);
  if (iter != this->OutputParameters.end())
    {
    return this->OutputParameters[name]; 
    }
  else
    {
    return 0.0;
    }
}


//----------------------------------------------------------------------------
vtkStringArray* vtkMRMLCurveAnalysisNode::GetParameterNameArray()
{
  ValueMapType::iterator iter;
  vtkStringArray* array = vtkStringArray::New();

  for (iter = this->OutputParameters.begin(); iter != this->OutputParameters.end(); iter ++)
    {
    array->InsertNextValue(iter->first.c_str());
    }

  return array;
}



//----------------------------------------------------------------------------
void vtkMRMLCurveAnalysisNode::ClearParameters()
{
  this->OutputParameters.clear();
}



