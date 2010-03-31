/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerNodeSelectorWidget.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

///  vtkSlicerNodeSelectorWidget - menu to select volumes from current mrml scene
/// 
/// Inherits most behavior from kw widget, but is specialized to observe
/// the current mrml scene and update the entries of the pop up menu to correspond
/// to the currently available volumes.  This widget also has a notion of the current selection
/// that can be observed or set externally
//


#ifndef __vtkMeasurementsAngleWidget_h
#define __vtkMeasurementsAngleWidget_h

#include "vtkMeasurementsWin32Header.h"

#include "vtkSlicerWidget.h"

  
class vtkSlicerNodeSelectorWidget;
class vtkKWCheckButtonWithLabel;
class vtkKWScaleWithLabel;
class vtkKWChangeColorButton;
class vtkKWMenuButton;
class vtkKWMenuButtonWithLabel;
class vtkKWLabel;
class vtkKWEntry;
class vtkKWEntryWithLabel;
class vtkMRMLMeasurementsAngleNode;
class vtkSlicerViewerWidget;
class vtkMeasurementsAngleWidgetClass;
class VTK_MEASUREMENTS_EXPORT vtkMeasurementsAngleWidget : public vtkSlicerWidget
{
  
public:
  static vtkMeasurementsAngleWidget* New();
  vtkTypeRevisionMacro(vtkMeasurementsAngleWidget,vtkSlicerWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// access methods
  vtkGetObjectMacro(VisibilityButton, vtkKWCheckButtonWithLabel);
  vtkGetObjectMacro (AllVisibilityMenuButton, vtkKWMenuButton);
  vtkGetObjectMacro (AnnotationFormatMenuButton, vtkKWMenuButtonWithLabel);

  /// 
  /// Getting the mrml angle node id
  vtkGetStringMacro(AngleNodeID);
  
  /// 
  /// Set the fid list id, and update the widgets
  void SetAngleNodeID(char *id);
    
  /// 
  /// alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  /// 
  /// alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  /// 
  /// adds observes on widgets in the class
  virtual void AddWidgetObservers();
  

  /// 
  /// removes observers on widgets in the class
  virtual void RemoveWidgetObservers ( );

  /// 
  /// add observers on node
  virtual void AddMRMLObservers ( );

  /// 
  /// remove observers on node
  virtual void RemoveMRMLObservers ( );

  /// 
  /// Get/set the viewer widget so can add a the angle widget to it
  vtkGetObjectMacro(ViewerWidget, vtkSlicerViewerWidget);
  virtual void SetViewerWidget(vtkSlicerViewerWidget *viewerWidget);
//BTX
  /// 
  /// encapsulated 3d widgets for each angle node
  std::map<std::string, vtkMeasurementsAngleWidgetClass *> AngleWidgets;
//ETX
  /// 
  /// get a distance widget by angle node id
  vtkMeasurementsAngleWidgetClass *GetAngleWidget(const char * nodeID);

  /// 
  /// set up a new distance widget for this node
  void AddAngleWidget(vtkMRMLMeasurementsAngleNode *angleNode);
  /// 
  /// remove distance widget for this node
  void RemoveAngleWidget(vtkMRMLMeasurementsAngleNode *angleNode);
  /// 
  /// check scene to make sure that have a widget for each angle node, and no extra widgets...
  void Update3DWidgetsFromMRML();

protected:
  vtkMeasurementsAngleWidget();
  virtual ~vtkMeasurementsAngleWidget();

  /// 
  /// Create the widget.
  virtual void CreateWidget();

  /// 
  /// update the widget GUI from the settings in the passed in angleNode
  void UpdateWidget(vtkMRMLMeasurementsAngleNode *angleNode);
  /// DescriptioN:
  /// update the 3d distance widget from the settings in teh passed in
  /// angleNode
  void Update3DWidget(vtkMRMLMeasurementsAngleNode *angleNode);

  /// 
  /// Set visibility on all angle nodes in the scene to the passed in visibility flag
  void ModifyAllAngleVisibility (int visibilityState);

  /// 
  /// the id of the mrml node currently displayed in the widget
  char* AngleNodeID;
  /// 
  /// to select a measurements angle node from the scene, and create new ones
  vtkSlicerNodeSelectorWidget* AngleSelectorWidget;

  /// 
  /// visibility check buttons
  vtkKWCheckButtonWithLabel *VisibilityButton;
  vtkKWCheckButtonWithLabel *Ray1VisibilityButton;
  vtkKWCheckButtonWithLabel *Ray2VisibilityButton;
  vtkKWCheckButtonWithLabel *ArcVisibilityButton;

  /// 
  /// Change the colour of the end points
  vtkKWChangeColorButton *PointColourButton;
  /// 
  /// Change the colour of the line
  vtkKWChangeColorButton *LineColourButton;
  /// 
  /// Change the colour of the label text
  vtkKWChangeColorButton *TextColourButton;
  
  /// 
  /// point position entry
  vtkKWLabel *Position1Label;
  vtkKWEntry *Position1XEntry;
  vtkKWEntry *Position1YEntry;
  vtkKWEntry *Position1ZEntry;
  vtkKWLabel *Position2Label;
  vtkKWEntry *Position2XEntry;
  vtkKWEntry *Position2YEntry;
  vtkKWEntry *Position2ZEntry;
  vtkKWLabel *PositionCenterLabel;
  vtkKWEntry *PositionCenterXEntry;
  vtkKWEntry *PositionCenterYEntry;
  vtkKWEntry *PositionCenterZEntry;


  /// 
  /// distance annotation option entries
  vtkKWEntryWithLabel *LabelFormatEntry;
  vtkKWEntryWithLabel *LabelScaleEntry;
  vtkKWCheckButtonWithLabel *LabelVisibilityButton;

  /// 
  /// number of subdivisions on the line
  vtkKWEntryWithLabel *ResolutionEntry;

  /// 
  /// Angle GUI elements to select models upon which to constrain the angle end points
  vtkSlicerNodeSelectorWidget* AngleModel1SelectorWidget;
  vtkSlicerNodeSelectorWidget* AngleModel2SelectorWidget;
  vtkSlicerNodeSelectorWidget* AngleModelCenterSelectorWidget;

  /// 
  /// menu button whose menu exposes options for
  /// setting visibility of all angle nodes
  vtkKWMenuButton *AllVisibilityMenuButton;

  ///
  /// menu button to select different default annotation options
  vtkKWMenuButtonWithLabel *AnnotationFormatMenuButton;
  
  /// 
  /// flag set to 1 when updating a 3d widget (todo: set it to the index of the
  /// angle node / 3d widget once have more than one)
  int Updating3DWidget;

  /// 
  /// pointer to the viewer widget so can add props, request renders 
  vtkSlicerViewerWidget *ViewerWidget;
  
  ///
  /// Sets Slicer's interaction mode to "pick and manipulate"
  /// when a widget is being manipulated, and
  void UpdateInteractionModeAtStartInteraction();
  ///
  /// Resets Slicer's interaction mode back to original
  /// interaction mode when manipulation is finished.
  void UpdateInteractionModeAtEndInteraction();
  


private:


  vtkMeasurementsAngleWidget(const vtkMeasurementsAngleWidget&); /// Not implemented
  void operator=(const vtkMeasurementsAngleWidget&); /// Not Implemented
};

#endif

