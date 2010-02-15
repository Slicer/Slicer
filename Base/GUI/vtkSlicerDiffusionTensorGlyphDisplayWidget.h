/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerNodeSelectorWidget.h,v $
  Date:      $Date$
  Version:   $Revision$

=========================================================================auto=*/

///  vtkSlicerDiffusionTensorGlyphDisplayWidget - GUI for setting parameters of a 
/// vtkMRMLDiffusionTensorGlyphDisplayNode
/// 
//


#ifndef __vtkSlicerDiffusionTensorGlyphDisplayWidget_h
#define __vtkSlicerDiffusionTensorGlyphDisplayWidget_h

#include "vtkSlicerWidget.h"

#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWScaleWithLabel.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWFrameWithLabel.h"

#include "vtkMRMLDiffusionTensorDisplayPropertiesNode.h"


class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerDiffusionTensorGlyphDisplayWidget : public vtkSlicerWidget
{
  
public:
  static vtkSlicerDiffusionTensorGlyphDisplayWidget* New();
  vtkTypeRevisionMacro(vtkSlicerDiffusionTensorGlyphDisplayWidget,vtkSlicerWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// 
  /// Set DiffusionTensorDisplayPropertiesNode currently active in this GUI.
  /// Internally this method sets the DiffusionTensorDisplayPropertiesNodeID 
  /// and sets up observers.
  void SetDiffusionTensorDisplayPropertiesNode ( vtkMRMLDiffusionTensorDisplayPropertiesNode *node );

  /// 
  /// Get MRML DiffusionTensorDisplayPropertiesNodeID.
  vtkGetStringMacro ( DiffusionTensorDisplayPropertiesNodeID );
  
  /// 
  /// alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  /// 
  /// alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  void SetGlyphResolution(int value);

  vtkGetObjectMacro(Frame, vtkKWFrameWithLabel);

  void Collapse() 
  {
    this->Frame->CollapseFrame();
  };
 void Expend() 
  {
    this->Frame->ExpandFrame ();
  };


 protected:
  vtkSlicerDiffusionTensorGlyphDisplayWidget();
  virtual ~vtkSlicerDiffusionTensorGlyphDisplayWidget();

  /// 
  /// add observers on display node
  virtual void AddMRMLObservers ( );

  /// 
  /// remove observers on display node
  virtual void RemoveMRMLObservers ( );

  /// 
  /// adds observers on widgets in the class
  virtual void AddWidgetObservers ( );

  /// 
  /// removes observers on widgets in the class
  virtual void RemoveWidgetObservers ( );

  /// 
  /// Set MRML DiffusionTensorDisplayPropertiesNodeID.
  vtkSetStringMacro ( DiffusionTensorDisplayPropertiesNodeID );

  /// 
  /// Create the widget.
  virtual void CreateWidget();

  /// 
  /// Update the widget's values to correspond to the MRML display node.
  void UpdateWidget();

  /// 
  /// Update the display node's values to correspond to the widget
  void UpdateMRML();
  
  /// 
  /// ID in the MRML scene of the current display properties node
  char* DiffusionTensorDisplayPropertiesNodeID;

  /// 
  /// All of the widgets used in this widget
  vtkKWMenuButtonWithLabel  *GlyphGeometryMenu;

  vtkKWFrameWithLabel *Frame;

  //vtkKWCheckButtonWithLabel *VisibilityButton;
  //vtkKWCheckButtonWithLabel *ScalarVisibilityButton;
  //vtkKWSurfaceMaterialPropertyWidget *SurfaceMaterialPropertyWidget;
  //vtkKWChangeColorButton *ChangeColorButton;

  /// advanced frame
  vtkKWScaleWithLabel  *GlyphScale;
  vtkKWScaleWithLabel  *GlyphResolutionScale;

  /// lines frame
  vtkKWMenuButtonWithLabel  *LineGlyphEigenvectorMenu;

  /// tubes frame
  vtkKWMenuButtonWithLabel  *TubeGlyphEigenvectorMenu;
  vtkKWScaleWithLabel  *TubeNumberOfSidesScale;
  vtkKWScaleWithLabel  *TubeRadiusScale;

  /// ellipsoids frame

  /// superquadrics frame

  //BTX
  std::map <std::string, int> GlyphGeometryMap;
  std::map <std::string, int> GlyphEigenvectorMap;
  //ETX

private:


  vtkSlicerDiffusionTensorGlyphDisplayWidget(const vtkSlicerDiffusionTensorGlyphDisplayWidget&); /// Not implemented
  void operator=(const vtkSlicerDiffusionTensorGlyphDisplayWidget&); /// Not Implemented
};

#endif

