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


#ifndef __vtkSlicerDiffusionTensorVolumeDisplayWidget_h
#define __vtkSlicerDiffusionTensorVolumeDisplayWidget_h

#include "vtkVolumes.h"
#include "vtkSlicerVolumeDisplayWidget.h"

#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkKWWindowLevelThresholdEditor.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWMenuButtonWithSpinButtonsWithLabel.h"
#include "vtkKWCheckButton.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLVolumeDisplayNode.h"

class vtkSlicerDiffusionTensorVolumeGlyphDisplayWidget;

class vtkDiffusionTensorMathematics;
class vtkImageExtractComponents;
class vtkAssignAttribute;

class VTK_VOLUMES_EXPORT vtkSlicerDiffusionTensorVolumeDisplayWidget : public vtkSlicerVolumeDisplayWidget
{
  
public:
  static vtkSlicerDiffusionTensorVolumeDisplayWidget* New();
  vtkTypeRevisionMacro(vtkSlicerDiffusionTensorVolumeDisplayWidget,vtkSlicerVolumeDisplayWidget);
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
  virtual void RemoveWidgetObservers();

  virtual void UpdateWidgetFromMRML();

protected:
  vtkSlicerDiffusionTensorVolumeDisplayWidget();
  virtual ~vtkSlicerDiffusionTensorVolumeDisplayWidget();

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  vtkKWMenuButtonWithSpinButtonsWithLabel* ScalarModeMenu;
  vtkSlicerModuleCollapsibleFrame* ScalarOptionsFrame; 
  vtkSlicerNodeSelectorWidget* ColorSelectorWidget;
  vtkKWWindowLevelThresholdEditor* WindowLevelThresholdEditor;
  vtkKWCheckButton* InterpolateButton;

  vtkSlicerDiffusionTensorVolumeGlyphDisplayWidget *GlyphDisplayWidget;
  vtkDiffusionTensorMathematics *DTIMathematics;
  vtkImageExtractComponents *ExtractComponent;
  
  vtkAssignAttribute* AssignAttributeTensorsFromScalars;
  vtkAssignAttribute* AssignAttributeScalarsFromTensors;

  int UpdatingMRML;
  int UpdatingWidget;

  //BTX
  std::map <std::string, int> ScalarModeMap;
  std::map <std::string, int> GlyphModeMap;
  //ETX
private:

  vtkSlicerDiffusionTensorVolumeDisplayWidget(const vtkSlicerDiffusionTensorVolumeDisplayWidget&); // Not implemented
  void operator=(const vtkSlicerDiffusionTensorVolumeDisplayWidget&); // Not Implemented
};

#endif

