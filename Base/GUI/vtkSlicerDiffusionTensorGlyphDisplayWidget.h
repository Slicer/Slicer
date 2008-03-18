/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerNodeSelectorWidget.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

// .NAME vtkSlicerDiffusionTensorGlyphDisplayWidget - GUI for setting parameters of a 
// vtkMRMLDiffusionTensorGlyphDisplayNode
// .SECTION Description
//


#ifndef __vtkSlicerDiffusionTensorGlyphDisplayWidget_h
#define __vtkSlicerDiffusionTensorGlyphDisplayWidget_h

#include "vtkSlicerWidget.h"

#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWScaleWithLabel.h"
#include "vtkKWMenuButtonWithLabel.h"

#include "vtkMRMLDiffusionTensorDisplayPropertiesNode.h"


class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerDiffusionTensorGlyphDisplayWidget : public vtkSlicerWidget
{
  
public:
  static vtkSlicerDiffusionTensorGlyphDisplayWidget* New();
  vtkTypeRevisionMacro(vtkSlicerDiffusionTensorGlyphDisplayWidget,vtkSlicerWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set DiffusionTensorDisplayPropertiesNode currently active in this GUI.
  // Internally this method sets the DiffusionTensorDisplayPropertiesNodeID 
  // and sets up observers.
  void SetDiffusionTensorDisplayPropertiesNode ( vtkMRMLDiffusionTensorDisplayPropertiesNode *node );

  // Description:
  // Get MRML DiffusionTensorDisplayPropertiesNodeID.
  vtkGetStringMacro ( DiffusionTensorDisplayPropertiesNodeID );
  
  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
  

 protected:
  vtkSlicerDiffusionTensorGlyphDisplayWidget();
  virtual ~vtkSlicerDiffusionTensorGlyphDisplayWidget();

  // Description:
  // add observers on display node
  virtual void AddMRMLObservers ( );

  // Description:
  // remove observers on display node
  virtual void RemoveMRMLObservers ( );

  // Description:
  // adds observers on widgets in the class
  virtual void AddWidgetObservers ( );

  // Description:
  // removes observers on widgets in the class
  virtual void RemoveWidgetObservers ( );

  // Description:
  // Set MRML DiffusionTensorDisplayPropertiesNodeID.
  vtkSetStringMacro ( DiffusionTensorDisplayPropertiesNodeID );

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
  // ID in the MRML scene of the current display properties node
  char* DiffusionTensorDisplayPropertiesNodeID;

  // Description:
  // All of the widgets used in this widget
  vtkKWMenuButtonWithLabel  *GlyphGeometryMenu;

  //vtkKWCheckButtonWithLabel *VisibilityButton;
  //vtkKWCheckButtonWithLabel *ScalarVisibilityButton;
  //vtkKWSurfaceMaterialPropertyWidget *SurfaceMaterialPropertyWidget;
  //vtkKWChangeColorButton *ChangeColorButton;

  // advanced frame
  vtkKWScaleWithLabel  *GlyphScale;
  vtkKWScaleWithLabel  *GlyphResolutionScale;

  // lines frame
  vtkKWMenuButtonWithLabel  *LineGlyphEigenvectorMenu;

  // tubes frame
  vtkKWMenuButtonWithLabel  *TubeGlyphEigenvectorMenu;
  vtkKWScaleWithLabel  *TubeNumberOfSidesScale;

  // ellipsoids frame

  // superquadrics frame

  //BTX
  std::map <std::string, int> GlyphGeometryMap;
  std::map <std::string, int> GlyphEigenvectorMap;
  //ETX

private:


  vtkSlicerDiffusionTensorGlyphDisplayWidget(const vtkSlicerDiffusionTensorGlyphDisplayWidget&); // Not implemented
  void operator=(const vtkSlicerDiffusionTensorGlyphDisplayWidget&); // Not Implemented
};

#endif

