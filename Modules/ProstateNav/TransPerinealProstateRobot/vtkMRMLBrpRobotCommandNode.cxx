/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLBrpRobotCommandNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkIGTLToMRMLBrpRobotCommand.h"

#include "vtkMRMLBrpRobotCommandNode.h"
#include "vtkMRMLDiffusionTensorDisplayPropertiesNode.h"

//------------------------------------------------------------------------------
vtkMRMLBrpRobotCommandNode* vtkMRMLBrpRobotCommandNode::New()
{
  vtkMRMLBrpRobotCommandNode* ret;

  // First try to create the object from the vtkObjectFactory
  vtkObject* r = vtkObjectFactory::CreateInstance("vtkMRMLBrpRobotCommandNode");
  if(r)
    {
    ret =  (vtkMRMLBrpRobotCommandNode*)r;
    }
  else
    {
    // If the factory was unable to create the object, then create it here.
    ret =  new vtkMRMLBrpRobotCommandNode;
    }
  
  return ret;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLBrpRobotCommandNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLBrpRobotCommandNode");
  if(ret)
    {
    return (vtkMRMLBrpRobotCommandNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLBrpRobotCommandNode;
}


//----------------------------------------------------------------------------
vtkMRMLBrpRobotCommandNode::vtkMRMLBrpRobotCommandNode()
{
  this->ZFrameTransformNodeID = "";

  this->StepToCommandMap["SetUp"]="START_UP";
  this->StepToCommandMap["ZFrameCalibration"]="CALIBRATION";
  this->StepToCommandMap["PointTargeting"]="TARGETING";
  this->StepToCommandMap["PointVerification"]="MANUAL";
  this->StepToCommandMap["TransperinealProstateRobotManualControl"]="PLANNING";
}


//----------------------------------------------------------------------------
void vtkMRMLBrpRobotCommandNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);

}


//----------------------------------------------------------------------------
void vtkMRMLBrpRobotCommandNode::SetZFrameTransformNodeID(const char* name)
{
  this->ZFrameTransformNodeID = name;
}


//----------------------------------------------------------------------------
void vtkMRMLBrpRobotCommandNode::SetTargetTransformNodeID(const char* name)
{
  this->TargetTransformNodeID = name;
}


//----------------------------------------------------------------------------
void vtkMRMLBrpRobotCommandNode::PushOutgoingCommand(const char* name)
{
  //this->OutCommandQueue.push(std::string(name));
  this->OutCommand = name;
}


//----------------------------------------------------------------------------
const char* vtkMRMLBrpRobotCommandNode::PopOutgoingCommand()
{
  //std::string ret = this->OutCommandQueue.front();
  //this->OutCommandQueue.pop();

  //return ret.c_str();
  return this->OutCommand.c_str();
}


//----------------------------------------------------------------------------
void vtkMRMLBrpRobotCommandNode::PushIncomingCommand(const char* name)
{
  //this->InCommandQueue.push(std::string(name));
  this->InCommand = name;
}


//----------------------------------------------------------------------------
const char* vtkMRMLBrpRobotCommandNode::PopIncomingCommand()
{
  //std::string ret = this->InCommandQueue.front();
  //this->InCommandQueue.pop();
  //
  //return ret.c_str();
  return this->InCommand.c_str();
}


//----------------------------------------------------------------------------
int vtkMRMLBrpRobotCommandNode::SwitchStep(const char* step)
{

  std::map<std::string,std::string>::const_iterator stepToCommand;

  stepToCommand = this->StepToCommandMap.find( step );
  if (stepToCommand==this->StepToCommandMap.end())
  {
    vtkErrorMacro("Unknown step: "<<step);
    return 0;
  }

  this->PushOutgoingCommand(stepToCommand->second.c_str());
  this->Modified();
    
  return 1;
}
