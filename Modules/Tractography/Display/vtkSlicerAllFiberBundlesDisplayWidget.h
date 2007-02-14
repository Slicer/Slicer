/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerNodeSelectorWidget.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

// .NAME vtkSlicerAllFiberBundlesDisplayWidget - GUI for setting parameters of  
// all vtkMRMLFiberBundleDisplayNodes.  This is the row of shortcut buttons
// that affect all fiber bundles at once (all visible, all tubes, all FA, etc.)
// .SECTION Description
//


#ifndef __vtkSlicerAllFiberBundlesDisplayWidget_h
#define __vtkSlicerAllFiberBundlesDisplayWidget_h

#include "vtkSlicerTractographyDisplay.h"

#include "vtkSlicerWidget.h"

#include "vtkKWSurfaceMaterialPropertyWidget.h"
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWRadioButtonSet.h"
#include "vtkKWRadioButton.h"
#include "vtkKWMenuButton.h"

#include "vtkMRMLFiberBundleNode.h"
#include "vtkMRMLFiberBundleDisplayNode.h"
#include "vtkSlicerDiffusionTensorIcons.h"


class VTK_SLICERTRACTOGRAPHYDISPLAY_EXPORT vtkSlicerAllFiberBundlesDisplayWidget : public vtkSlicerWidget
{
  
public:
  static vtkSlicerAllFiberBundlesDisplayWidget* New();
  vtkTypeRevisionMacro(vtkSlicerAllFiberBundlesDisplayWidget,vtkSlicerWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData );

  // List of most common types of coloring to enable in shortcut buttons
  //BTX
  enum
  {
    ColorModeSolid = 0,
    ColorModeFA = 1,
    ColorModeCL = 2,
    ColorModeTrace = 3
  };
  //ETX
 
 protected:
  vtkSlicerAllFiberBundlesDisplayWidget();
  virtual ~vtkSlicerAllFiberBundlesDisplayWidget();

  // Description:
  // removes observers on widgets in the class
  virtual void RemoveWidgetObservers ( );

  // Description:
  // adds observers on widgets in the class
  virtual void AddWidgetObservers ( );

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
  // All of the widgets used in this widget
  vtkKWCheckButtonWithLabel *LineVisibilityButton;
  vtkKWCheckButtonWithLabel *TubeVisibilityButton;
  vtkKWCheckButtonWithLabel *GlyphVisibilityButton;

  vtkKWSurfaceMaterialPropertyWidget *SurfaceMaterialPropertyWidget;

  vtkKWPushButton *ToggleLineVisibilityButton;
  vtkKWPushButton *ToggleTubeVisibilityButton;
  vtkKWPushButton *ToggleGlyphVisibilityButton;

  vtkKWMenuButton *VisibilityButton;

  vtkKWRadioButtonSet *ColorModeRadioButtons;

   // Description:
  // Contains icons
  vtkSlicerDiffusionTensorIcons *DiffusionTensorIcons;
  
private:

  int ColorMode;

  vtkSlicerAllFiberBundlesDisplayWidget(const vtkSlicerAllFiberBundlesDisplayWidget&); // Not implemented
  void operator=(const vtkSlicerAllFiberBundlesDisplayWidget&); // Not Implemented
};

#endif

