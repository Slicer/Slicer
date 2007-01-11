/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerNodeSelectorWidget.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

// .NAME vtkSlicerFiberBundleDisplayWidget - GUI for setting parameters of a 
// vtkMRMLFiberBundleDisplayNode
// .SECTION Description
//


#ifndef __vtkSlicerFiberBundleDisplayWidget_h
#define __vtkSlicerFiberBundleDisplayWidget_h

#include "vtkSlicerTractographyDisplayWin32Header.h"

#include "vtkSlicerWidget.h"

#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWScaleWithLabel.h"
#include "vtkKWSurfaceMaterialPropertyWidget.h"
#include "vtkKWChangeColorButton.h"

#include "vtkMRMLFiberBundleNode.h"
#include "vtkMRMLFiberBundleDisplayNode.h"


class VTK_SLICERTRACTOGRAPHYDISPLAY_EXPORT vtkSlicerFiberBundleDisplayWidget : public vtkSlicerWidget
{
  
public:
  static vtkSlicerFiberBundleDisplayWidget* New();
  vtkTypeRevisionMacro(vtkSlicerFiberBundleDisplayWidget,vtkSlicerWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set FiberBundleDisplayNode currently active in this GUI.
  // Internally this method sets the FiberBundleNodeID and FiberBundleDisplayNodeID,
  // and sets up observers.
  void SetFiberBundleNode ( vtkMRMLFiberBundleNode *node );

  // Description:
  // Get MRML FiberBundleNodeID.
  vtkGetStringMacro ( FiberBundleNodeID );
  
  // Description:
  // Get MRML FiberBundleDisplayNodeID.
  vtkGetStringMacro ( FiberBundleDisplayNodeID );

  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
  

 protected:
  vtkSlicerFiberBundleDisplayWidget();
  virtual ~vtkSlicerFiberBundleDisplayWidget();

  // Description:
  // add observers on display node
  virtual void AddMRMLObservers ( );

  // Description:
  // remove observers on display node
  virtual void RemoveMRMLObservers ( );
  
  // Description:
  // removes observers on widgets in the class
  virtual void RemoveWidgetObservers ( );

  // Description:
  // Set MRML FiberBundleDisplayNodeID.
  vtkSetStringMacro ( FiberBundleDisplayNodeID );
  
  // Description:
  // Set MRML FiberBundleNodeID.
  vtkSetStringMacro ( FiberBundleNodeID );

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  // Description:
  // Update the widget's values to correspond to the MRML display node.
  void UpdateWidget();

  // Description:
  // Update the display node's values to correspond to the widget
  void UpdateMRML();
  
  // Description:
  // ID in the MRML scene of the current fiber bundle node
  char* FiberBundleNodeID;

  // Description:
  // ID in the MRML scene of the current fiber bundle node's display node
  char* FiberBundleDisplayNodeID;
  
  // Description:
  // All of the widgets used in this widget
  vtkSlicerNodeSelectorWidget* FiberBundleSelectorWidget;
  vtkKWCheckButtonWithLabel *VisibilityButton;
  vtkKWCheckButtonWithLabel *ScalarVisibilityButton;
  vtkSlicerNodeSelectorWidget* ColorSelectorWidget;
  vtkKWCheckButtonWithLabel *ClippingButton;
  vtkKWScaleWithLabel  *OpacityScale;
  vtkKWSurfaceMaterialPropertyWidget *SurfaceMaterialPropertyWidget;
  vtkKWChangeColorButton *ChangeColorButton;

private:


  vtkSlicerFiberBundleDisplayWidget(const vtkSlicerFiberBundleDisplayWidget&); // Not implemented
  void operator=(const vtkSlicerFiberBundleDisplayWidget&); // Not Implemented
};

#endif

