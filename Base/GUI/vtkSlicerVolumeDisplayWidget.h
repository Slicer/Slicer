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


#ifndef __vtkSlicerVolumeDisplayWidget_h
#define __vtkSlicerVolumeDisplayWidget_h

#include "vtkSlicerWidget.h"

#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkKWWindowLevelThresholdEditor.h"

#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLVolumeDisplayNode.h"


class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerVolumeDisplayWidget : public vtkSlicerWidget
{
  
public:
  static vtkSlicerVolumeDisplayWidget* New();
  vtkTypeRevisionMacro(vtkSlicerVolumeDisplayWidget,vtkSlicerWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Getting setting  MRML VolumeNodeID.
  vtkGetStringMacro ( VolumeNodeID );
  vtkSetStringMacro ( VolumeNodeID );
  
  void SetVolumeNode ( vtkMRMLVolumeNode *node );

  // Description:
  // Getting setting and observing MRML VolumeDisplayNodeID.
  vtkGetStringMacro ( VolumeDisplayNodeID );
  vtkSetStringMacro ( VolumeDisplayNodeID );
  
  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  // Description:
  // removes observers on widgets in the class
  virtual void RemoveWidgetObservers ( );
  
 protected:
  vtkSlicerVolumeDisplayWidget();
  virtual ~vtkSlicerVolumeDisplayWidget();

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  char* VolumeNodeID;
  char* VolumeDisplayNodeID;
  
  vtkSlicerNodeSelectorWidget* VolumeSelectorWidget;
  vtkSlicerNodeSelectorWidget* ColorSelectorWidget;
  vtkKWWindowLevelThresholdEditor* WindowLevelThresholdEditor;
  
private:


  vtkSlicerVolumeDisplayWidget(const vtkSlicerVolumeDisplayWidget&); // Not implemented
  void operator=(const vtkSlicerVolumeDisplayWidget&); // Not Implemented
};

#endif

