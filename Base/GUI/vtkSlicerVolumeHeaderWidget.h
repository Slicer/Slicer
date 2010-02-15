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


#ifndef __vtkSlicerVolumeHeaderWidget_h
#define __vtkSlicerVolumeHeaderWidget_h

#include "vtkSlicerWidget.h"

#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWEntry.h"

#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLStorageNode.h"

class vtkKWPushButtonWithLabel;
class vtkKWCheckButtonWithLabel;
class vtkKWLabel;
class vtkKWListBoxWithScrollbars;
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerVolumeHeaderWidget : public vtkSlicerWidget
{
  
public:
  static vtkSlicerVolumeHeaderWidget* New();
  vtkTypeRevisionMacro(vtkSlicerVolumeHeaderWidget,vtkSlicerWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  void SetVolumeNode ( vtkMRMLVolumeNode *node );
  
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
  /// add observers on widgets in the class
  virtual void AddWidgetObservers ( );

  virtual void UpdateWidgetFromMRML();

  /// 
  /// get current volume node
  vtkMRMLVolumeNode* GetVolumeNode ();
  
  /// 
  /// get current volume storage node
  vtkMRMLStorageNode* GetVolumeStorageNode ();

  /// 
  /// Add a Node Selector Widget
  vtkSetMacro(AddNodeSelectorWidget,int);
  vtkGetMacro(AddNodeSelectorWidget,int);
  vtkBooleanMacro(AddNodeSelectorWidget,int);

protected:
  vtkSlicerVolumeHeaderWidget();
  virtual ~vtkSlicerVolumeHeaderWidget();

  /// 
  /// Create the widget.
  virtual void CreateWidget();

  int AddNodeSelectorWidget;

  vtkMRMLVolumeNode *VolumeNode;

  vtkSlicerNodeSelectorWidget *VolumeSelectorWidget;

  vtkKWEntryWithLabel *DimensionEntry0;
  vtkKWEntry          *DimensionEntry1;
  vtkKWEntry          *DimensionEntry2;

  vtkKWEntryWithLabel *SpacingEntry0;
  vtkKWEntry          *SpacingEntry1;
  vtkKWEntry          *SpacingEntry2;

  vtkKWEntryWithLabel *OriginEntry0;
  vtkKWEntry          *OriginEntry1;
  vtkKWEntry          *OriginEntry2;

  vtkKWPushButtonWithLabel *CenterButton;

  vtkKWEntryWithLabel *ScanOrderEntry;

  vtkKWEntryWithLabel *NumScalarsEntry;

  vtkKWEntryWithLabel *ScalarTypeEntry;
  
  vtkKWEntryWithLabel *FileNameEntry;

  vtkKWCheckButtonWithLabel *LabelMapCheckButton;

  vtkKWLabel *WinLevelPresetsLabel;
  vtkKWListBoxWithScrollbars *WinLevelPresetsListBox;

private:

  /// internal flag to prevent widget updates from triggering 
  /// a feedback loop
  int UpdatingFromMRML;

  vtkSlicerVolumeHeaderWidget(const vtkSlicerVolumeHeaderWidget&); /// Not implemented
  void operator=(const vtkSlicerVolumeHeaderWidget&); /// Not Implemented
};

#endif

