/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLTransRectalProstateRobotNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkMRMLTransRectalRobotNode_h
#define __vtkMRMLTransRectalRobotNode_h

#include "vtkOpenIGTLinkIFWin32Header.h"
#include "vtkMRML.h"
#include "vtkMRMLRobotNode.h"
#include "vtkMRMLStorageNode.h"
#include "vtkSmartPointer.h"
#include "vtkLineSource.h"

#include "vtkObject.h"
#include "vtkProstateNavWin32Header.h" 
#include "vtkTransRectalFiducialCalibrationAlgo.h"

class vtkPolyData;

class VTK_PROSTATENAV_EXPORT vtkMRMLTransRectalProstateRobotNode : public vtkMRMLRobotNode
{
 public:

  //----------------------------------------------------------------
  // Standard methods for MRML nodes
  //----------------------------------------------------------------

  static vtkMRMLTransRectalProstateRobotNode *New();
  vtkTypeMacro(vtkMRMLTransRectalProstateRobotNode,vtkMRMLRobotNode);
  
  // Description:
  // Initialize the robot
  virtual int Init(vtkSlicerApplication* app);

  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLTransRectalProstateRobotNode* CreateNodeInstance();

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
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName()
    {return "TransRectalProstateRobot";};

  virtual const char* GetWorkflowStepsString()
    {return "FiducialCalibration PointTargetingWithoutOrientation PointVerification"; };

  // method to propagate events generated in mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  virtual bool FindTargetingParams(vtkProstateNavTargetDescriptor *targetDesc);
  virtual bool ShowRobotAtTarget(vtkProstateNavTargetDescriptor *targetDesc);
  //BTX
  virtual std::string GetTargetInfoText(vtkProstateNavTargetDescriptor *targetDesc);
  //ETX

  // Description:
  void GetCalibrationMarker(unsigned int markerNr, double &r, double &a, double &s, bool &valid);
  void SetCalibrationMarker(unsigned int markerNr, double markerRAS[3]);
  void RemoveAllCalibrationMarkers();

  void ResetCalibrationData();
  const TRProstateBiopsyCalibrationData& GetCalibrationData() { return this->CalibrationData; }
  void SetCalibrationData(const TRProstateBiopsyCalibrationData& calibData);

  //BTX
  bool SegmentRegisterMarkers(vtkMRMLScalarVolumeNode *calibVol, double thresh[4], double fidDimsMm[3], double radiusMm, bool bUseRadius, double initialAngle, std::string &resultDetails, bool enableAutomaticCenterpointAdjustment);
  //ETX
 
 virtual const char* GetRobotModelId() {return GetRobotModelNodeID(); };
 virtual bool GetRobotBaseTransform(vtkMatrix4x4* transform);

 void SetModelAxesVisible(bool visible);
 vtkGetMacro(ModelAxesVisible, bool);

 protected:
  //----------------------------------------------------------------
  // Constructor and destroctor
  //----------------------------------------------------------------
  
  vtkMRMLTransRectalProstateRobotNode();
  ~vtkMRMLTransRectalProstateRobotNode();
  vtkMRMLTransRectalProstateRobotNode(const vtkMRMLTransRectalProstateRobotNode&);
  void operator=(const vtkMRMLTransRectalProstateRobotNode&);  

  vtkGetStringMacro(RobotModelNodeID);
  vtkMRMLModelNode* GetRobotModelNode();
  void SetAndObserveRobotModelNodeID(const char *nodeID);

  const char* AddRobotModel(const char* nodeName); 

 protected:

  void UpdateModel();
  void UpdateModelAxes();
  void UpdateModelProbe();
  void UpdateModelMarkers();
  void UpdateModelNeedle(vtkProstateNavTargetDescriptor *targetDesc);


  TRProstateBiopsyCalibrationData CalibrationData;

  //BTX
  vtkSmartPointer<vtkTransRectalFiducialCalibrationAlgo> CalibrationAlgo;  
  //ETX

  double CalibrationMarkerPositions[CALIB_MARKER_COUNT][3];
  bool CalibrationMarkerValid[CALIB_MARKER_COUNT];

  // Robot model components
  vtkPolyData* ModelMarkers[CALIB_MARKER_COUNT];
  vtkPolyData* ModelAxes;
  vtkPolyData* ModelProbe;
  vtkPolyData* ModelNeedle;

  bool ModelAxesVisible;

  vtkSetReferenceStringMacro(RobotModelNodeID);
  char *RobotModelNodeID;
  vtkMRMLModelNode* RobotModelNode;
  
};

#endif

