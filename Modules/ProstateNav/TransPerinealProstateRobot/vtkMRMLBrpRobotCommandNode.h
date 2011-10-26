/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLBrpRobotCommandNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/
// .NAME vtkMRMLBrpRobotCommandNode - MRML node to represent BRP Prostate Robot
// .SECTION Description
//

#ifndef __vtkMRMLBrpRobotCommandNode_h
#define __vtkMRMLBrpRobotCommandNode_h


#include "vtkProstateNavWin32Header.h"

#include "vtkPolyData.h" 

#include "vtkMRML.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"

#include <string>
#include <queue>

class vtkCallbackCommand;
class vtkSphereSource;

class VTK_PROSTATENAV_EXPORT vtkMRMLBrpRobotCommandNode : public vtkMRMLNode
{
 public:
  static vtkMRMLBrpRobotCommandNode *New();
  vtkTypeMacro(vtkMRMLBrpRobotCommandNode,vtkMRMLNode);

  void PrintSelf(ostream& os, vtkIndent indent);
  
  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "BrpRobotCommand";};

 protected:
  vtkMRMLBrpRobotCommandNode();
  ~vtkMRMLBrpRobotCommandNode(){};
  vtkMRMLBrpRobotCommandNode(const vtkMRMLBrpRobotCommandNode&);
  void operator=(const vtkMRMLBrpRobotCommandNode&);
  

 public:
  void SetZFrameTransformNodeID(const char* name);
  const char* GetZFrameTransformNodeID() { return this->ZFrameTransformNodeID.c_str(); };
  void SetTargetTransformNodeID(const char* name);
  const char* GetTargetTransformNodeID() { return this->TargetTransformNodeID.c_str(); };

  void PushOutgoingCommand(const char* name);
  const char* PopOutgoingCommand();
  void PushIncomingCommand(const char* name);
  const char* PopIncomingCommand();

  int SwitchStep(const char* step);

 protected:
  std::string ZFrameTransformNodeID;
  std::string TargetTransformNodeID;

  std::queue<std::string> InCommandQueue;
  std::queue<std::string> OutCommandQueue;
  std::string InCommand;
  std::string OutCommand;

  std::map<std::string,std::string> StepToCommandMap;


};

#endif
