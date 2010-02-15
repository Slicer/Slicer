/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All
  Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerApplicationGUI.cxx,v $
  Date:      $Date$
  Version:   $Revision$
a
=========================================================================auto=*/

///  vtkSlicerContextMenuHelper - helper code to manage node editing context menus for the MRMLTreeWidget and the NodeSelectorWidget
/// 
/// This class combines common context menu functions in a single class
/// for easier management.  This class handles populating the menu and 
/// handles the callbacks.  A scene and a node are instance variables
/// to handle callbacks.
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

  /// 
  /// Populate a context menu for a given node
  virtual void PopulateMenu();

  /// 
  /// Callbacks triggered by the menu.  String argument is a node id
  /// to operate on.
  //
  virtual void DeleteNodeCallback();
  virtual void RenameNodeCallback();
  virtual void ToggleVisibilityCallback();
  virtual void RenameApplyCallback();
  virtual void RenameApplyCallback(char * vtkNotUsed( name ) ) {this->RenameApplyCallback();};
  virtual void EditCallback();

  /// 
  /// Show/Hide mini dialog to rename node
  //
  void PopUpRenameEntry();
  void HideRenameEntry();

  /// 
  /// MRML scene to operate on (not observed)
  vtkSetObjectMacro(MRMLScene, vtkMRMLScene);
  vtkGetObjectMacro(MRMLScene, vtkMRMLScene);

  /// 
  /// MRML node to operate on (not observed)
  vtkSetObjectMacro(MRMLNode, vtkMRMLNode);
  vtkGetObjectMacro(MRMLNode, vtkMRMLNode);

  /// 
  /// menu to operate on (used for processing callbacks)
  vtkSetObjectMacro(ContextMenu, vtkKWMenu);
  vtkGetObjectMacro(ContextMenu, vtkKWMenu);

  /// 
  /// Update menu item state based on MRMLNode state (null or not null)
  void UpdateMenuState();

  protected:
    
  vtkSlicerContextMenuHelper();
  ~vtkSlicerContextMenuHelper();

  private:
  vtkSlicerContextMenuHelper(const vtkSlicerContextMenuHelper&); /// Not implemented
  void operator =(const vtkSlicerContextMenuHelper&); /// Not implemented

  vtkMRMLScene *MRMLScene;
  vtkMRMLNode *MRMLNode;
  vtkKWMenu *ContextMenu;
  vtkKWTopLevel *RenameTopLevel;
  vtkKWEntryWithLabel *RenameEntry;
  vtkKWPushButton *RenameApply;
  vtkKWPushButton *RenameCancel;
  /// 
  /// holders for menu item indices
  int DeleteItem;
  int RenameItem;
  int EditItem;
};

#endif
