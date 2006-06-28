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


#ifndef __vtkSlicerMRMLTreeWidget_h
#define __vtkSlicerMRMLTreeWidget_h

#include "vtkSlicerWidget.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLTransformableNode.h"

class vtkKWMenu;
class vtkKWTreeWithScrollbars;
class vtkKWEntryWithLabel;
class vtkKWLabelWithLabel;

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerMRMLTreeWidget : public vtkSlicerWidget
{
  
public:
  static vtkSlicerMRMLTreeWidget* New();
  vtkTypeRevisionMacro(vtkSlicerMRMLTreeWidget,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  // Description:
  // removes observers on widgets in the class
  virtual void RemoveWidgetObservers ( );

  vtkGetObjectMacro(CutNode, vtkMRMLTransformableNode);
  vtkSetObjectMacro(CutNode, vtkMRMLTransformableNode);
  
  // Description::
  // Callbacks
  virtual void DeleteNodeCallback(const char *id);

  virtual void CutNodeCallback(const char *id);

  virtual void PasteNodeCallback(const char *id);

protected:
  vtkSlicerMRMLTreeWidget();
  ~vtkSlicerMRMLTreeWidget();

  // Description:
  // Create the widget.
  virtual void CreateWidget();
  
  void UpdateTreeFromMRML();
  void AddNodeToTree(vtkMRMLNode *node);
  void UpdateNodeInspector(vtkMRMLNode *node);
  vtkMRMLNode* GetSelectedNodeInTree();

  vtkMRMLTransformableNode* CutNode;

private:
  
  vtkKWTreeWithScrollbars *TreeWidget;
  vtkKWMenu *ContextMenu;

  vtkKWLabelWithLabel *NodeID;
  vtkKWEntryWithLabel *NodeName;

  vtkSlicerMRMLTreeWidget(const vtkSlicerMRMLTreeWidget&); // Not implemented
  void operator=(const vtkSlicerMRMLTreeWidget&); // Not Implemented
};

#endif

