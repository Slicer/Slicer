/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerNodeSelectorWidget.h,v $
  Date:      $Date$
  Version:   $Revision$

=========================================================================auto=*/

///  vtkSlicerNodeSelectorWidget - menu to select volumes from current mrml scene
/// 
/// Inherits most behavior from kw widget, but is specialized to observe
/// the current mrml scene and update the entries of the pop up menu to correspond
/// to the currently available volumes.  This widget also has a notion of the current selection
/// that can be observed or set externally
//


#ifndef __vtkSlicerModelHierarchyWidget_h
#define __vtkSlicerModelHierarchyWidget_h

#include "vtkSlicerWidget.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLModelHierarchyNode.h"
#include "vtkMRMLModelDisplayNode.h"

#include "vtkSlicerModelHierarchyLogic.h"

#include <vtksys/stl/string>

class vtkKWMenu;
class vtkKWTreeWithScrollbars;
class vtkKWSimpleEntryDialog;
class vtkSlicerNodeSelectorWidget;
class vtkSlicerModelDisplayWidget;

class vtkSlicerModelHierarchyLogic;

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerModelHierarchyWidget : public vtkSlicerWidget
{
  
public:
  static vtkSlicerModelHierarchyWidget* New();
  vtkTypeRevisionMacro(vtkSlicerModelHierarchyWidget,vtkSlicerWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  /// 
  /// alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  /// 
  /// alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  /// 
  /// removes observers on widgets in the class
  virtual void RemoveWidgetObservers ( );
  
  /// 
  /// select node in the tree and selector
  virtual void SelectNode(vtkMRMLNode *node);

  /// :
  /// Callbacks
  virtual void ModelVisibilityCallback(const char *id);
  virtual void HierarchyVisibilityCallback(const char *id);
  virtual void AllVisibilityCallback(int visibility);
  virtual void ColorCallback(const char *id);
  virtual void InsertHierarchyNodeCallback(const char *id);
  virtual void DeleteNodeCallback(const char *id);
  virtual void RenameNodeCallback(const char *id);
  virtual void SelectNodeCallback(const char *id);
  virtual void SelectReparentCallback(const char *id);
  virtual void ReparentCallback(const char *id);
  virtual void OpenHierarchyCommand(const char *id);
  virtual void CloseHierarchyCommand(const char *id);
  virtual void NodeParentChangedCallback (const char *node, const char *new_parent, const char*);
  virtual void ProcessRightClick(const char *id);
  virtual void SearchNodeCallback();
  
  /// 
  /// get/set vtkSlicerModelHierarchyLogic
  vtkGetObjectMacro( ModelHierarchyLogic, vtkSlicerModelHierarchyLogic );
  vtkSetObjectMacro( ModelHierarchyLogic, vtkSlicerModelHierarchyLogic );

  /// 
  /// get at the node selector so can use it to assign scalars
  vtkGetObjectMacro( ModelDisplaySelectorWidget, vtkSlicerNodeSelectorWidget);
    
  void UpdateTreeFromMRML();

//BTX
  enum
    {
      SelectedEvent = 711100,
   };
//ETX
protected:
  vtkSlicerModelHierarchyWidget();
  virtual ~vtkSlicerModelHierarchyWidget();

  /// 
  /// Create the widget.
  virtual void CreateWidget();

  void AddNodeToTree(vtkMRMLNode *node);
  vtkMRMLNode* GetSelectedNodeInTree();

  int IsLeafSelected(const char* leaf);
  void SetSelectesLeaves();

  int UpdatingTree;
  
  //BTX
  std::vector<vtksys_stl::string> SelectedLeaves;
  std::vector<vtksys_stl::string> SelectedForReparenting;
  //ETX

private:
  
  vtkKWTreeWithScrollbars *TreeWidget;
  vtkKWMenu *ContextMenu;

  vtkKWSimpleEntryDialog *NameDialog;

  vtkMRMLModelDisplayNode* ModelDisplayNode;

  vtkSlicerNodeSelectorWidget *ModelDisplaySelectorWidget;

  vtkSlicerModelDisplayWidget *ModelDisplayWidget;

  vtkSlicerModelHierarchyLogic *ModelHierarchyLogic;

  vtkSlicerModelHierarchyWidget(const vtkSlicerModelHierarchyWidget&); /// Not implemented
  void operator=(const vtkSlicerModelHierarchyWidget&); /// Not Implemented
};

#endif

