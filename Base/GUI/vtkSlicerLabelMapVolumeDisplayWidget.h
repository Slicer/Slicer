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


#ifndef __vtkSlicerLabelMapVolumeDisplayWidget_h
#define __vtkSlicerLabelMapVolumeDisplayWidget_h

#include "vtkSlicerVolumeDisplayWidget.h"

#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkKWWindowLevelThresholdEditor.h"
#include "vtkKWCheckButton.h"

#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLVolumeDisplayNode.h"


class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerLabelMapVolumeDisplayWidget : public vtkSlicerVolumeDisplayWidget
{
  
public:
  static vtkSlicerLabelMapVolumeDisplayWidget* New();
  vtkTypeRevisionMacro(vtkSlicerLabelMapVolumeDisplayWidget,vtkSlicerVolumeDisplayWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

 // Description:
  // add observers on widgets in the class
  virtual void AddWidgetObservers ( );

  // Description:
  // removes observers on widgets in the class
  virtual void RemoveWidgetObservers ( );

 // Description:
  // update the widget upon MRML change
  virtual void UpdateWidgetFromMRML();
  

protected:
  vtkSlicerLabelMapVolumeDisplayWidget();
  virtual ~vtkSlicerLabelMapVolumeDisplayWidget();

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  vtkSlicerNodeSelectorWidget* ColorSelectorWidget;

  int UpdatingMRML;
  int UpdatingWidget;

private:


  vtkSlicerLabelMapVolumeDisplayWidget(const vtkSlicerLabelMapVolumeDisplayWidget&); // Not implemented
  void operator=(const vtkSlicerLabelMapVolumeDisplayWidget&); // Not Implemented
};

#endif

