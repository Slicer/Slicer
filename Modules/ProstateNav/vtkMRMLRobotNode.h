/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLCurveAnalysisNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkMRMLRobotNode_h
#define __vtkMRMLRobotNode_h

#include "vtkOpenIGTLinkIFWin32Header.h"
#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLStorageNode.h"

#include "vtkObject.h"
#include "vtkProstateNavWin32Header.h" 

#include "vtkMRMLLinearTransformNode.h"

class vtkMRMLModelNode;
class vtkMRMLScalarVolumeNode;
class vtkSlicerApplication;
class vtkProstateNavTargetDescriptor;

class VTK_PROSTATENAV_EXPORT vtkMRMLRobotNode : public vtkMRMLNode
{

 public:

  //----------------------------------------------------------------
  // Constants Definitions
  //----------------------------------------------------------------

  // Events
  enum {
    ChangeStatusEvent     = 200907,
    ChangeTargetEvent     = 200908,
    RobotMovedEvent       = 200910
  };

  enum STATUS_ID {
    StatusOff=0,
    StatusOk,
    StatusWarning,
    StatusError
  };

  struct StatusDescriptor
  {
    std::string text;
    STATUS_ID indicator; // this determines the background color
  };



 public:

  //----------------------------------------------------------------
  // Standard methods for MRML nodes
  //----------------------------------------------------------------

  static vtkMRMLRobotNode *New();
  vtkTypeMacro(vtkMRMLRobotNode,vtkMRMLNode);
  
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Updates other nodes in the scene depending on this node
  // or updates this node if it depends on other nodes
  virtual void UpdateScene(vtkMRMLScene *);

// Description:
  // Update the stored reference to another node in the scene
  void UpdateReferenceID(const char *oldID, const char *newID);

  // Description:
  // Updates this node if it depends on other nodes 
  // when the node is deleted in the scene
  void UpdateReferences();

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName()
    {return "Robot";};

  virtual const char* GetWorkflowStepsString()
    {return "PointTargeting PointVerification"; };

  // method to propagate events generated in mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  // Description:
  // Get/Set robot target (vtkMRMLLinearTransformNode)
  vtkGetStringMacro(TargetTransformNodeID);
  vtkMRMLTransformNode* GetTargetTransformNode();
  void SetAndObserveTargetTransformNodeID(const char *transformNodeID);

  virtual int Init(vtkSlicerApplication* app);

  virtual int  MoveTo(const char *transformNodeId) { return 0; };

  virtual void SwitchStep(const char *stepName) {};

  virtual int OnTimer() {return 1; };

  virtual bool FindTargetingParams(vtkProstateNavTargetDescriptor *targetDesc) { return false; };
  virtual bool ShowRobotAtTarget(vtkProstateNavTargetDescriptor *targetDesc) { return false; };
  virtual std::string GetTargetInfoText(vtkProstateNavTargetDescriptor *targetDesc) { return ""; };

  // Description:
  // Sets the transform to a coordinate system that is aligned with the robot base.
  // This coordinate system is determined during the calibration and it is not changed after that.
  virtual bool GetRobotBaseTransform(vtkMatrix4x4* transform) { return false; };

  virtual int PerformRegistration(vtkMRMLScalarVolumeNode* volumeNode) { return 0; };

  // The following method is defined tentatively to pass registration parameter.
  virtual int PerformRegistration(vtkMRMLScalarVolumeNode* volumeNode, int param1, int param2) { return 0; };


  // Description:
  // Get calibration object (Z frame, fiducials, etc.) model and transform
  virtual const char* GetCalibrationObjectModelId() {return ""; };
  virtual const char* GetCalibrationObjectTransformId() { return ""; };

  // Description:
  // Get workspace object model (representing range of motion of the device)
  virtual const char* GetWorkspaceObjectModelId() {return ""; };

  // Description:
  // Get robot model (representing the robot with the needle guide, etc.)
  virtual const char* GetRobotModelId() {return ""; };

  int GetStatusDescriptorCount();
  
  // Description:
  // returns 0 if failed
  int GetStatusDescriptor(unsigned int index, std::string &text, STATUS_ID &indicator);

 protected:
  //----------------------------------------------------------------
  // Constructor and destructor
  //----------------------------------------------------------------
  
  vtkMRMLRobotNode();
  ~vtkMRMLRobotNode();
  vtkMRMLRobotNode(const vtkMRMLRobotNode&);
  void operator=(const vtkMRMLRobotNode&);

  vtkSetReferenceStringMacro(TargetTransformNodeID);
  char *TargetTransformNodeID;
  vtkMRMLTransformNode* TargetTransformNode;

  std::vector<StatusDescriptor> StatusDescriptors;

 protected:
  //
  
  
};

#endif

