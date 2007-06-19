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


#ifndef __vtkSlicerRecordSnapshotWidget_h
#define __vtkSlicerRecordSnapshotWidget_h

#include "vtkSlicerWidget.h"
#include "vtkSlicerNodeSelectorWidget.h"

#include "vtkKWPushButton.h"

#include "vtkMRMLSceneSnapshotNode.h"
#include "vtkMRMLSnapshotClipNode.h"

class vtkKWSimpleEntryDialog;

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerRecordSnapshotWidget : public vtkSlicerWidget
{
  
public:
  static vtkSlicerRecordSnapshotWidget* New();
  vtkTypeRevisionMacro(vtkSlicerRecordSnapshotWidget,vtkSlicerWidget);
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

  virtual void AddMRMLObservers();
  virtual void RemoveMRMLObservers();

protected:
  vtkSlicerRecordSnapshotWidget();
  virtual ~vtkSlicerRecordSnapshotWidget();

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  vtkKWPushButton* StartRecordButton;
  vtkKWPushButton* StopRecordButton;
  vtkKWPushButton* ReplayButton;

  vtkKWPushButton* SaveClipButton;
  vtkSlicerNodeSelectorWidget* ClipSelectorWidget;
  vtkKWSimpleEntryDialog *NameDialog;

  //BTX
  std::vector <vtkMRMLSceneSnapshotNode *> Sanpshots;
  //ETX
private:


  vtkSlicerRecordSnapshotWidget(const vtkSlicerRecordSnapshotWidget&); // Not implemented
  void operator=(const vtkSlicerRecordSnapshotWidget&); // Not Implemented
};

#endif

