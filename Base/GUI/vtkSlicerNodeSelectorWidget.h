/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerNodeSelectorWidget.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

// .NAME vtkSlicerNodeSelectorWidget - menu to select volumes from current mrml scene
// .SECTION Description
// Inherits most behavior from kw widget, but is specialized to observe
// the current mrml scene and update the entries of the pop up menu to correspond
// to the currently available volumes.  This widget also has a notion of the current selection
// that can be observed or set externally
//


#ifndef __vtkSlicerNodeSelectorWidget_h
#define __vtkSlicerNodeSelectorWidget_h

#include "vtkCallbackCommand.h"

#include "vtkKWMenuButtonWithSpinButtonsWithLabel.h"
#include "vtkKWMenuButtonWithSpinButtons.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"

#include "vtkSlicerBaseGUI.h"

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerNodeSelectorWidget : public vtkKWMenuButtonWithSpinButtonsWithLabel
{
public:
  static vtkSlicerNodeSelectorWidget* New();
  vtkTypeRevisionMacro(vtkSlicerNodeSelectorWidget,vtkKWMenuButtonWithSpinButtonsWithLabel);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // this is the scene to observe
  vtkGetObjectMacro(MRMLScene, vtkMRMLScene);
  void SetMRMLScene(vtkMRMLScene *MRMLScene);

  // Description:
  // Set class name of this node to select
  void SetNodeClass(char *className) {
    NodeClasses.clear();
    NodeClasses.push_back(std::string(className));
  };

  // Description:
  // Add class name of this node to select
  void AddNodeClass(char *className) {
    NodeClasses.push_back(std::string(className));
  };

  // Description:
  // Get a n-th class name of this node to select
  const char* GetNodeClass(int ind) {
    return NodeClasses[ind].c_str();
  };

  // Description:
  // Get a number of class names to select
  int GetNumberOfNodeClasses() {
    return NodeClasses.size();
  };

  // Description:
  // Specifies whether new node creation is enabled
  vtkBooleanMacro(NewNodeEnabled, int);
  vtkGetMacro(NewNodeEnabled, int);
  vtkSetMacro(NewNodeEnabled, int);
  
  // Description
  // Get selected node
  vtkMRMLNode *GetSelected();

  // Description
  // Set selected node
  void SetSelected(vtkMRMLNode *node);

  // Description
  // Get selection to new node of n-th class name
  void SetSelectedNew(const char *className);

//BTX
  enum
    {
      NodeSelectedEvent = 10000,
    };
//ETX

  // Description:
  // reflect the state of the mrml scene in the menu
  void UpdateMenu();

  void ProcessNewNodeCommand(char *className);
  void ProcessCommand(char *slectedId);

protected:
  vtkSlicerNodeSelectorWidget();
  ~vtkSlicerNodeSelectorWidget();

//BTX
  std::vector<std::string> NodeClasses;

  std::string SelectedID;
//ETX
  
  int NewNodeEnabled;
  
  vtkMRMLScene       *MRMLScene;

private:

  int NewNodeCount;

  vtkCallbackCommand *MRMLCallbackCommand;

  vtkSlicerNodeSelectorWidget(const vtkSlicerNodeSelectorWidget&); // Not implemented
  void operator=(const vtkSlicerNodeSelectorWidget&); // Not Implemented
};

#endif

