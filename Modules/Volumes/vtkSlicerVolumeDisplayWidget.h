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

#include "vtkVolumes.h"
#include "vtkSlicerWidget.h"

#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkKWWindowLevelThresholdEditor.h"
#include "vtkKWCheckButton.h"

#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLVolumeDisplayNode.h"


class VTK_VOLUMES_EXPORT vtkSlicerVolumeDisplayWidget : public vtkSlicerWidget
{
  
public:
  static vtkSlicerVolumeDisplayWidget* New();
  vtkTypeRevisionMacro(vtkSlicerVolumeDisplayWidget,vtkSlicerWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // get current volume node
  vtkGetObjectMacro(VolumeNode, vtkMRMLVolumeNode);

  // Description:
  // Keep track of changes in the volume node
  void SetVolumeNode ( vtkMRMLVolumeNode *node )
  {
    vtkSetAndObserveMRMLNodeMacro( this->VolumeNode, node);
  }
  
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
  // add observers on mrml scene
  virtual void AddMRMLObservers ( );

  // Description:
  // removes observers on mrml scene
  virtual void RemoveMRMLObservers ( );

  virtual void UpdateWidgetFromMRML();

  // Description:
  // This method releases references and removes observers.
  virtual void TearDownWidget ( );
  
  // Description:
  // get current volume display node
  vtkMRMLVolumeDisplayNode* GetVolumeDisplayNode ();

protected:
  vtkSlicerVolumeDisplayWidget();
  virtual ~vtkSlicerVolumeDisplayWidget();

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  vtkMRMLVolumeNode *VolumeNode;
 
private:

  vtkSlicerVolumeDisplayWidget(const vtkSlicerVolumeDisplayWidget&); // Not implemented
  void operator=(const vtkSlicerVolumeDisplayWidget&); // Not Implemented
};

#endif

