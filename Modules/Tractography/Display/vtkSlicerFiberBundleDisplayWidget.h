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

#include "vtkSlicerTractographyDisplay.h"

#include "vtkSlicerWidget.h"

#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkSlicerDiffusionTensorGlyphDisplayWidget.h"

#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWScaleWithLabel.h"
#include "vtkKWChangeColorButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWFrameWithLabel.h"

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
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  void SetTractVisibility(int visibility);

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
  // Create the widget.
  virtual void CreateWidget();

  // Description:
  // Update the widget's values to correspond to the MRML display node.
  void UpdateWidget();

  // Description:
  // Update the display node's values to correspond to the widget
  void UpdateMRML();
  
  bool SyncSceneNodes();
    
  // Description:
  // All of the widgets used in this widget
  vtkSlicerNodeSelectorWidget* FiberBundleSelectorWidget;
  vtkSlicerNodeSelectorWidget* ColorSelectorWidget;
  vtkKWCheckButtonWithLabel *ClippingButton;
  vtkKWScaleWithLabel  *OpacityScale;
  vtkKWChangeColorButton *ChangeColorButton;

  vtkKWCheckButtonWithLabel *VisibilityButton;

  vtkSlicerDiffusionTensorGlyphDisplayWidget *GlyphDisplayWidget;

  int UpdatingMRML;
  int UpdatingWidget;
  
  vtkMRMLFiberBundleNode* FiberBundleNode;
  
  vtkMRMLFiberBundleDisplayNode* FiberBundleLineDisplayNode;
  vtkMRMLFiberBundleDisplayNode* FiberBundleTubeDisplayNode;
  vtkMRMLFiberBundleDisplayNode* FiberBundleGlyphDisplayNode;

  vtkMRMLFiberBundleDisplayNode* GetCurrentDisplayNode();
  vtkMRMLDiffusionTensorDisplayPropertiesNode* GetCurrentDiffusionTensorDisplayPropertyNode();

  vtkKWMenuButtonWithLabel *GeometryMenu;
  vtkKWFrameWithLabel *DisplayFrame;
  vtkKWMenuButtonWithLabel  *GeometryColorMenu;

  
//BTX
  std::string CurrentGeometry;
  std::map <std::string, int> GeometryColorMap;
//ETX
  
private:


  vtkSlicerFiberBundleDisplayWidget(const vtkSlicerFiberBundleDisplayWidget&); // Not implemented
  void operator=(const vtkSlicerFiberBundleDisplayWidget&); // Not Implemented
};

#endif

