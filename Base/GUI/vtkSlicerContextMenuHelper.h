/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All
  Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerApplicationGUI.cxx,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $
a
=========================================================================auto=*/

// .NAME vtkSlicerContextMenuHelper - helper code to manage node editing context menus for the MRMLTreeWidget and the NodeSelectorWidget
// .SECTION Description
// This class combines common context menu functions in a single class
// for easier management.  This class handles populating the menu and 
// handles the callbacks.  A scene and a node are instance variables
// to handle callbacks.
//

#ifndef __vtkSlicerContextMenuHelper_h
#define __vtkSlicerContextMenuHelper_h

#include "vtkObject.h"
#include "vtkSlicerBaseGUIWin32Header.h" 


#include "vtkMRMLScene.h" 
#include "vtkMRMLNode.h" 

#include "vtkKWTopLevel.h"
#include "vtkKWMenu.h"

#include "vtkKWEntryWithLabel.h"
class vtkKWPushButton;


class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerContextMenuHelper : public vtkObject
{
  public:
  static vtkSlicerContextMenuHelper* New();
  vtkTypeRevisionMacro(vtkSlicerContextMenuHelper, vtkObject);

  // Description:
  // Populate a context menu for a given node
  virtual void PopulateMenu();

  // Description:
  // Callbacks triggered by the menu.  String argument is a node id
  // to operate on.
  //
  virtual void DeleteNodeCallback();
  virtual void ToggleVisibilityCallback();
  virtual void RenameCallback();
  virtual void RenameApplyCallback();
  virtual void EditCallback();

  // Description:
  // Show/Hide mini dialog to rename node
  //
  void PopUpRenameEntry();
  void HideRenameEntry();

  // Description:
  // MRML scene to operate on (not observed)
  vtkSetObjectMacro(MRMLScene, vtkMRMLScene);
  vtkGetObjectMacro(MRMLScene, vtkMRMLScene);

  // Description:
  // MRML node to operate on (not observed)
  vtkSetObjectMacro(MRMLNode, vtkMRMLNode);
  vtkGetObjectMacro(MRMLNode, vtkMRMLNode);

  // Description:
  // menu to operate on (used for processing callbacks)
  vtkSetObjectMacro(ContextMenu, vtkKWMenu);
  vtkGetObjectMacro(ContextMenu, vtkKWMenu);

  protected:
    
  vtkSlicerContextMenuHelper();
  ~vtkSlicerContextMenuHelper();

  private:
  vtkSlicerContextMenuHelper(const vtkSlicerContextMenuHelper&); // Not implemented
  void operator =(const vtkSlicerContextMenuHelper&); // Not implemented

  vtkMRMLScene *MRMLScene;
  vtkMRMLNode *MRMLNode;
  vtkKWMenu *ContextMenu;
  vtkKWTopLevel *RenameTopLevel;
  vtkKWEntryWithLabel *RenameEntry;
  vtkKWPushButton *RenameApply;
  vtkKWPushButton *RenameCancel;
};

#endif
