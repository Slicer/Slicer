/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLTransPerinealProstateRobotNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkMRMLTransPerinealProstateTemplateNode_h
#define __vtkMRMLTransPerinealProstateTemplateNode_h

#include "vtkOpenIGTLinkIFWin32Header.h"
#include "vtkMRML.h"
#include "vtkMRMLRobotNode.h"
#include "vtkMRMLStorageNode.h"

#include "vtkCylinderSource.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkTransform.h"
#include "vtkAppendPolyData.h"

#include "vtkObject.h"
#include "vtkProstateNavWin32Header.h" 

#include "vtkMRMLIGTLConnectorNode.h"

#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLModelNode.h"

class vtkTransform;
class vtkIGTLToMRMLCoordinate;
class vtkSlicerApplication;

class VTK_PROSTATENAV_EXPORT vtkMRMLTransPerinealProstateTemplateNode : public vtkMRMLRobotNode
{

 public:

  //----------------------------------------------------------------
  // Standard methods for MRML nodes
  //----------------------------------------------------------------

  static vtkMRMLTransPerinealProstateTemplateNode *New();
  vtkTypeMacro(vtkMRMLTransPerinealProstateTemplateNode,vtkMRMLRobotNode);  

  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLTransPerinealProstateTemplateNode* CreateNodeInstance();

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
  // Update the stored reference to another node in the scene
  void UpdateReferenceID(const char *oldID, const char *newID);

  // Description:
  // Updates this node if it depends on other nodes 
  // when the node is deleted in the scene
  void UpdateReferences();

  // Description:
  // Updates other nodes in the scene depending on this node
  // or updates this node if it depends on other nodes
  virtual void UpdateScene(vtkMRMLScene *);

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName()
    {return "TransPerinealProstateTemplate";};

  // method to propagate events generated in mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  //----------------------------------------------------------------
  // Commands
  //----------------------------------------------------------------

  // Description:
  // Initialize the robot
  virtual int Init(vtkSlicerApplication* app);

  vtkGetStringMacro(ScannerConnectorNodeID);
  vtkMRMLIGTLConnectorNode* GetScannerConnectorNode();
  void SetAndObserveScannerConnectorNodeID(const char *nodeID);

  vtkGetStringMacro(WorkspaceModelNodeID);
  vtkMRMLModelNode* GetWorkspaceModelNode();
  void SetAndObserveWorkspaceModelNodeID(const char *nodeID);

  vtkGetStringMacro(ZFrameModelNodeID);
  vtkMRMLModelNode* GetZFrameModelNode();
  void SetAndObserveZFrameModelNodeID(const char *nodeID);

  vtkGetStringMacro(ZFrameTransformNodeID);
  vtkMRMLLinearTransformNode* GetZFrameTransformNode();
  void SetAndObserveZFrameTransformNodeID(const char *nodeID);

  vtkGetStringMacro(TemplateModelNodeID);
  vtkMRMLModelNode* GetTemplateModelNode();
  void SetAndObserveTemplateModelNodeID(const char *nodeID);

  //vtkGetStringMacro(TemplateTransformNodeID);
  //vtkMRMLLinearTransformNode* GetTemplateTransformNode();
  //void SetAndObserveTemplateTransformNodeID(const char *nodeID);

  vtkGetStringMacro(ActiveNeedleModelNodeID);
  vtkMRMLModelNode* GetActiveNeedleModelNode();
  void SetAndObserveActiveNeedleModelNodeID(const char *nodeID);

  vtkGetStringMacro(ActiveNeedleTransformNodeID);
  vtkMRMLLinearTransformNode* GetActiveNeedleTransformNode();
  void SetAndObserveActiveNeedleTransformNodeID(const char *nodeID);

  vtkGetStringMacro(ScreenMessage);

  virtual int  MoveTo(const char *transformNodeId);
  
  virtual void SwitchStep(const char *stepName);

  virtual int OnTimer();

  virtual bool FindTargetingParams(vtkProstateNavTargetDescriptor *targetDesc);
  virtual std::string GetTargetInfoText(vtkProstateNavTargetDescriptor *targetDesc);

  virtual const char* GetCalibrationObjectModelId() { return GetZFrameModelNodeID(); };
  virtual const char* GetCalibrationObjectTransformId() { return GetZFrameTransformNodeID(); };
  virtual const char* GetWorkspaceObjectModelId() { return GetWorkspaceModelNodeID(); };

  virtual int PerformRegistration(vtkMRMLScalarVolumeNode* volumeNode);
  virtual int PerformRegistration(vtkMRMLScalarVolumeNode* volumeNode, int param1, int param2);

  virtual const char* GetWorkflowStepsString()
    {return "SetUpTemplate ZFrameCalibration TemplateTargeting PointVerification"; };

  //----------------------------------------------------------------------------
  // Funcitons to obtain information about needle guiding template
  
  // Description:
  // Get position and orientation of grid hole (i, j)
  int GetHoleTransform(int i, int j, vtkMatrix4x4* matrix); 

  // Description:
  // Get needle tip position and orientation, when grid hole (i, j) is used.
  int GetNeedleTransform(int i, int j, double length, vtkMatrix4x4* matrix);
  
  // Description:
  // Find the best hole to target the designated target position
  int FindHole(double targetX, double targetY, double targetZ,
               int& nearest_i, int& nearest_j, double& nearest_depth,
               double& errorX, double& errorY, double& errorZ);



 protected:

  vtkMRMLTransPerinealProstateTemplateNode();
  ~vtkMRMLTransPerinealProstateTemplateNode();
  vtkMRMLTransPerinealProstateTemplateNode(const vtkMRMLTransPerinealProstateTemplateNode&);
  void operator=(const vtkMRMLTransPerinealProstateTemplateNode&);

  ///////////

  int  SendZFrame();

  vtkGetMacro ( Connection,              bool );
  vtkGetMacro ( ScannerWorkPhase,         int );

  const char* AddWorkspaceModel(const char* nodeName);
  const char* AddZFrameModel(const char* nodeName);
  const char* AddTemplateModel(const char* nodeName);
  const char* AddNeedleModel(const char* nodeName, double length, double diameter);

  // NOTE: Since we couldn't update ScannerStatusLabelDisp and RobotStatusLabelDisp
  // directly from ProcessMRMLEvent(), we added following flags to update those GUI
  // widgets in the timer handler.
  // if flag == 0, the widget does not need to be updated()
  // if flag == 1, the connector has connected to the target
  // if flag == 2, the connector has disconnected from the target
  int ScannerConnectedFlag;


private:

  // Node references

  vtkSetReferenceStringMacro(ScannerConnectorNodeID);
  char *ScannerConnectorNodeID;
  vtkMRMLIGTLConnectorNode* ScannerConnectorNode;

  vtkSetReferenceStringMacro(ZFrameModelNodeID);
  char *ZFrameModelNodeID;
  vtkMRMLModelNode* ZFrameModelNode;

  vtkSetReferenceStringMacro(ZFrameTransformNodeID); 
  char *ZFrameTransformNodeID;
  vtkMRMLLinearTransformNode* ZFrameTransformNode;  

  vtkSetReferenceStringMacro(TemplateModelNodeID);
  char *TemplateModelNodeID;
  vtkMRMLModelNode* TemplateModelNode;

  //vtkSetReferenceStringMacro(TemplateTransformNodeID); 
  //char *TemplateTransformNodeID;
  //vtkMRMLLinearTransformNode* TemplateTransformNode;  

  vtkSetReferenceStringMacro(WorkspaceModelNodeID);
  char *WorkspaceModelNodeID;
  vtkMRMLModelNode* WorkspaceModelNode;

  vtkSetReferenceStringMacro(ActiveNeedleModelNodeID);
  char *ActiveNeedleModelNodeID;
  vtkMRMLModelNode* ActiveNeedleModelNode;

  vtkSetReferenceStringMacro(ActiveNeedleTransformNodeID); 
  char *ActiveNeedleTransformNodeID;
  vtkMRMLLinearTransformNode* ActiveNeedleTransformNode;

  // Screen message
  vtkSetReferenceStringMacro(ScreenMessage);
  char *ScreenMessage;

  // Other member variables

  vtkIGTLToMRMLCoordinate* CoordinateConverter;

  bool  Connection;  
  int   ScannerWorkPhase;

  // Template for needle guidance
  double TemplateGridPitch[2];   // Template pitch in mm (x and y in the figure bellow)
  double TemplateNumGrids[2];    // Number of holes in each direction (m and n in the figure bellow)
  double TemplateOffset[3];      // Offset of the first hole ((0,0) in the figure bellow)
                                 // from the Z-frame center (mm) (in the Z-frame coordinate system)

  //
  // Figure: needle guiding template parameters
  //
  //   Template (holes are indicated as 'o')
  //
  //   +---------------------------------------+
  //   |         x                             |
  //   |     |<----->|                         |
  //   |  -- o       o       o   ....    o     |
  //   |  ^ (0,0)   (0,1)   (0,2)       (0,n-1)|
  //   | y|                                    |
  //   |  v                                    |
  //   |  -- o       o       o   ....    o     |
  //   |    (1,0)   (1,1)   (1,2)       (1,n-1)|
  //   |                                       |
  //   |                                       |
  //   |     o       o       o   ....    o     |
  //   |    (2,0)   (2,1)   (2,2)       (2,n-1)|
  //   |     .       .       .   .       .     |
  //   |     .       .       .     .     .     |
  //   |     .       .       .       .   .     |
  //   |                                       |
  //   |     o       o       o   ....    o     |
  //   |    (m-1,0) (m-1,1) (m-1,2)   (m-1,n-1)|
  //   |                                       |
  //   +---------------------------------------+ 
  //
  // NOTE: index (i, j) starts from (0, 0) for coding. When it displayed
  //  for clinicians, (i+1, j+1) is used. 
  //
  
};

#endif

