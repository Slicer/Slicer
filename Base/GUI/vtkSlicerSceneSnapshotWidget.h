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


#ifndef __vtkSlicerSceneSnapshotWidget_h
#define __vtkSlicerSceneSnapshotWidget_h

#include "vtkSlicerWidget.h"

#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkSlicerFoundationIcons.h"
#include "vtkKWPushButton.h"

#include "vtkMRMLSceneSnapshotNode.h"

#include "vtkCallbackCommand.h"

class vtkKWSimpleEntryDialog;

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerSceneSnapshotWidget : public vtkSlicerWidget
{
  
public:
  static vtkSlicerSceneSnapshotWidget* New();
  vtkTypeRevisionMacro(vtkSlicerSceneSnapshotWidget,vtkSlicerWidget);
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
  /// removes observers on node deleted events 
  virtual void AddMRMLObservers();
  virtual void RemoveMRMLObservers();

  vtkGetObjectMacro ( Icons, vtkSlicerFoundationIcons );

protected:
  vtkSlicerSceneSnapshotWidget();
  virtual ~vtkSlicerSceneSnapshotWidget();

  /// 
  /// Create the widget.
  virtual void CreateWidget();
  vtkCallbackCommand *MRMLExtraCallbackCommand;

  //BTX
  static void MRMLExtraCallback(  vtkObject *__caller,
                                unsigned long eid, void *__clientData, void *callData );    
  //ETX

  
  vtkSlicerNodeSelectorWidget* SnapshotSelectorWidget;
  vtkKWPushButton* CreateSnapshotButton;
  vtkKWPushButton *DeleteSnapshotButton;
  vtkKWPushButton* RestoreSceneButton;
  vtkKWSimpleEntryDialog *NameDialog;
  vtkSlicerFoundationIcons *Icons;
  
private:


  vtkSlicerSceneSnapshotWidget(const vtkSlicerSceneSnapshotWidget&); /// Not implemented
  void operator=(const vtkSlicerSceneSnapshotWidget&); /// Not Implemented
};

#endif

