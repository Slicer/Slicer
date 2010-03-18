/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

// .NAME vtkIGTToolSelectorLogic - slicer logic class for Locator module 
// .SECTION Description
// This class manages the logic associated with tracking device for
// IGT. 


#ifndef __vtkIGTToolSelectorLogic_h
#define __vtkIGTToolSelectorLogic_h

#include "vtkIGTToolSelectorWin32Header.h"

#include "vtkSlicerBaseLogic.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerApplication.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLSliceNode.h"

class vtkIGTLConnector;
class vtkAppendPolyData;

class VTK_IGTToolSelector_EXPORT vtkIGTToolSelectorLogic : public vtkSlicerModuleLogic 
{
 public:
  //BTX
  enum {  // Events
    //LocatorUpdateEvent      = 50000,
    StatusUpdateEvent       = 50001,
  };
  //ETX

 public:
  
  static vtkIGTToolSelectorLogic *New();
  
  vtkTypeRevisionMacro(vtkIGTToolSelectorLogic,vtkObject);
  void PrintSelf(ostream&, vtkIndent);

  //-----------------------------------------------------------------------------
  // NEW LOGIC
  //-----------------------------------------------------------------------------
  // define the transform object coming from OpenIGTLink.
  int SetToolTransformNode(const char* nodeID);

  void BuildToolPolyData(int tool);
  void BuildDefaultToolPolyData();
  void BuildNeedlePolyData();
  void BuildPointerPolyData();

  int EnableTool(int tool, int sw);

  void SetShowAxes(int sw);
  void SetShowProjection(int sw);
  void SetShowToolTip(int sw);

  void SetProjectionLength(double val);
  void SetProjectionDiameter(double val);
  void SetToolTipDiameter(double val);

  vtkMRMLModelNode* SetVisibilityOfTool(const char* nodeName, int tool, int v);
  vtkMRMLModelNode* AddToolModel(const char* nodeName);

 protected:
  
  vtkIGTToolSelectorLogic();
  ~vtkIGTToolSelectorLogic();

  void operator=(const vtkIGTToolSelectorLogic&);
  vtkIGTToolSelectorLogic(const vtkIGTToolSelectorLogic&);

  static void DataCallback(vtkObject*, unsigned long, void *, void *);
  void UpdateAll();

  vtkCallbackCommand *DataCallbackCommand;

 private:

  //-----------------------------------------------------------------------------
  // Local Variables.
  //-----------------------------------------------------------------------------
  //BTX
  std::string ToolTransformNodeID; // this is the real-time updated transform node.
  vtkSmartPointer<vtkAppendPolyData> ToolPolyData;
  vtkSmartPointer<vtkAppendPolyData> ToolTipPolyData;
  vtkSmartPointer<vtkAppendPolyData> ToolProjPolyData;
  int ToolType;
  int EnableToolFlag;
  int ShowAxesFlag;
  int CurrentShowAxesFlag;
  int ShowToolTipFlag;
  int CurrentShowTipFlag;
  int ShowProjectionFlag;
  int CurrentShowProjectionFlag;

  int NewParameters; //projection length changed or sphere diameters changed.

  // projection variables.
  double projectionLength;
  double projectionDiameter;
  double toolTipDiameter;

  // enumerate the different types of tools.
  enum
  {
    OPENIGT_DEFAULT,
    NEEDLE,
    POINTER
  };
  //ETX
};

#endif


  
