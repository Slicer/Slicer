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

#include "vtkSlicerWidget.h"

#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWScaleWithLabel.h"
#include "vtkKWSurfaceMaterialPropertyWidget.h"
#include "vtkKWChangeColorButton.h"

#include "vtkMRMLFiberBundleNode.h"
#include "vtkMRMLFiberBundleDisplayNode.h"


class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerFiberBundleDisplayWidget : public vtkSlicerWidget
{
  
public:
  static vtkSlicerFiberBundleDisplayWidget* New();
  vtkTypeRevisionMacro(vtkSlicerFiberBundleDisplayWidget,vtkSlicerWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Getting setting  MRML FiberBundleNodeID.
  vtkGetStringMacro ( FiberBundleNodeID );
  vtkSetStringMacro ( FiberBundleNodeID );
  
  void SetFiberBundleNode ( vtkMRMLFiberBundleNode *node );

  // Description:
  // Getting setting and observing MRML FiberBundleDisplayNodeID.
  vtkGetStringMacro ( FiberBundleDisplayNodeID );
  vtkSetStringMacro ( FiberBundleDisplayNodeID );
  
  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  // Description:
  // removes observers on widgets in the class
  virtual void RemoveWidgetObservers ( );

  // Description:
  // add observers on display node
  virtual void AddMRMLObservers ( );

  // Description:
  // remove observers on display node
  virtual void RemoveMRMLObservers ( );
  
 protected:
  vtkSlicerFiberBundleDisplayWidget();
  virtual ~vtkSlicerFiberBundleDisplayWidget();

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  // Description:
  // Update the widget's values to correspond to the MRML display node.
  void UpdateWidget();

  // Description:
  // Update the node's values to correspond to the widget
  void UpdateMRML();
  
  char* FiberBundleNodeID;
  char* FiberBundleDisplayNodeID;
  
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

