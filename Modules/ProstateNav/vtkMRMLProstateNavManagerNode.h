/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLCurveAnalysisNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkMRMLProstateNavManagerNode_h
#define __vtkMRMLProstateNavManagerNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLStorageNode.h"

#include "vtkObject.h"
#include "vtkProstateNavWin32Header.h" 

#include "vtkMRMLFiducialListNode.h"
#include "vtkMRMLIGTLConnectorNode.h"

class vtkProstateNavStep;

class VTK_PROSTATENAV_EXPORT vtkMRMLProstateNavManagerNode : public vtkMRMLNode
{

 public:

  //----------------------------------------------------------------
  // Constants Definitions
  //----------------------------------------------------------------

  //BTX
  // Events
  enum {
    ConnectedEvent        = 118944,
    DisconnectedEvent     = 118945,
    ActivatedEvent        = 118946,
    DeactivatedEvent      = 118947,
    ReceiveEvent          = 118948,
    NewDeviceEvent        = 118949,
  };
  //ETX

 public:

  //----------------------------------------------------------------
  // Get and Set Macros
  //----------------------------------------------------------------
  vtkGetObjectMacro ( TargetPlanList, vtkMRMLFiducialListNode );
  vtkGetObjectMacro ( TargetCompletedList, vtkMRMLFiducialListNode );

  vtkGetObjectMacro ( RobotConnector, vtkMRMLIGTLConnectorNode );
  vtkGetObjectMacro ( ScannerConnector, vtkMRMLIGTLConnectorNode );

  //----------------------------------------------------------------
  // Standard methods for MRML nodes
  //----------------------------------------------------------------

  static vtkMRMLProstateNavManagerNode *New();
  vtkTypeMacro(vtkMRMLProstateNavManagerNode,vtkMRMLNode);
  
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
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName()
    {return "ProstateNavManager";};


  // Description:
  // Method to propagate events generated in mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  //----------------------------------------------------------------
  // Workflow wizard step management
  //----------------------------------------------------------------

  // Description:
  // Get number of wizard steps
  int GetNumberOfSteps();

  // Description:
  // Get number of wizard steps
  const char* GetStepName(int i);

  // Description:
  // Get page by vtkProstateNavStep* pointer
  vtkProstateNavStep* GetStepPage(int i);

  // Description:
  // Add a new step. Please note that the transition matrix is resized
  // each time the new step is added by AddNewStep() fucntion.
  // The matrix should  be defined after all steps are added to the
  // manager class.
  void AddNewStep(const char* name, vtkProstateNavStep* page);
  
  // Description:
  // Clear the step
  void ClearSteps();

  // Description:
  // Switch step. Returns 0 if it is not allowed.
  int SwitchStep(int i);

  // Description:
  // Get current step.
  int GetCurrentStep();

  // Description:
  // Get previous step.
  int GetPreviousStep();


  //----------------------------------------------------------------
  // Phase transitions
  //----------------------------------------------------------------
  
  // Description:
  // Fill the transition matrix with 1 to allow all step transitions.
  void AllowAllTransitions();

  // Description:
  // Fill the transition matrix with 0 to forbid all step transitions.
  void ForbidAllTransitions();

  // Description:
  // Set phase transition by 2-D int array.
  // The format of 'matrix' argument should be matrix[step_from][step_to].
  int SetStepTransitionMatrix(const int** matrix);
  
  // Description:
  // Allow trasition from 'step_from' to 'step_to'.
  int SetAllowTransition(int step_from, int step_to);
  
  // Description:
  // Forbid trasition from 'step_from' to 'step_to'.
  int SetForbidTransition(int step_from, int step_to);

  // Description:
  // Check if the step can transtion from 'step_from' to 'step_to'.
  // Returns 0, if forbidden, 1 if allowed, -1 if not defined.
  int IsTransitionable(int step_from, int step_to);

  // Description:
  // Check if the step can transtion from current step to 'step_to'.
  // Returns 0, if forbidden, 1 if allowed, -1 if not defined.
  int IsTransitionable(int step_to);

  //----------------------------------------------------------------
  // Target Management
  //----------------------------------------------------------------
  
  // Description:
  // Set and start observing target plan list
  void SetAndObserveTargetPlanList(vtkMRMLFiducialListNode* ptr);

  // Description:
  // Set and start observing completed target list
  void SetAndObserveTargetCompletedList(vtkMRMLFiducialListNode* ptr);

  //----------------------------------------------------------------
  // Connectors
  //----------------------------------------------------------------

  // Description:
  // Set and start observing OpenIGTLink connector for robot
  void SetAndObserveRobotConnector(vtkMRMLIGTLConnectorNode* ptr);

  // Description:
  // Set and start observing OpenIGTLink connector for scanner
  void SetAndObserveScannerConnector(vtkMRMLIGTLConnectorNode* ptr);


 protected:
  //----------------------------------------------------------------
  // Constructor and destroctor
  //----------------------------------------------------------------
  
  vtkMRMLProstateNavManagerNode();
  ~vtkMRMLProstateNavManagerNode();
  vtkMRMLProstateNavManagerNode(const vtkMRMLProstateNavManagerNode&);
  void operator=(const vtkMRMLProstateNavManagerNode&);


 protected:
  //----------------------------------------------------------------
  // Data
  //----------------------------------------------------------------

  // List of wizard pages
  //BTX
  typedef struct {
    std::string         name;
    vtkProstateNavStep* page;
  } StepInfoType;
  std::vector<StepInfoType>         StepList;

  std::vector< std::vector<int> >   StepTransitionMatrix;
  //ETX
  
  int CurrentStep;
  int PreviousStep;
  
  vtkMRMLFiducialListNode* TargetPlanList;
  vtkMRMLFiducialListNode* TargetCompletedList;

  vtkMRMLIGTLConnectorNode* RobotConnector;
  vtkMRMLIGTLConnectorNode* ScannerConnector;
  
};

#endif

