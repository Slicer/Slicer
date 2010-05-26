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
class vtkKWScaleWithLabel;
class vtkKWChangeColorButton;
class vtkKWMenuButton;
class vtkKWMenuButtonWithLabel;
class vtkKWLabel;
class vtkKWEntry;
class vtkKWPushButton;
class vtkKWEntryWithLabel;
class vtkMRMLMeasurementsAngleNode;
class vtkSlicerViewerWidget;
class vtkMeasurementsAngleWidgetClass;
class vtkCamera;
class VTK_MEASUREMENTS_EXPORT vtkMeasurementsAngleWidget : public vtkSlicerWidget
{
  
public:
  static vtkMeasurementsAngleWidget* New();
  vtkTypeRevisionMacro(vtkMeasurementsAngleWidget,vtkSlicerWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// access methods
  vtkGetObjectMacro(VisibilityButton, vtkKWPushButton);
  vtkGetObjectMacro(Ray1VisibilityButton, vtkKWPushButton);
  vtkGetObjectMacro(Ray2VisibilityButton, vtkKWPushButton);
  vtkGetObjectMacro(ArcVisibilityButton, vtkKWPushButton);
  vtkGetObjectMacro(LabelVisibilityButton, vtkKWPushButton);
  vtkGetObjectMacro(AllVisibilityMenuButton, vtkKWMenuButton);
  vtkGetObjectMacro(AnnotationFormatMenuButton, vtkKWMenuButtonWithLabel);
  vtkGetObjectMacro(RemoveAllAnglesButton, vtkKWPushButton);
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

  ///
  /// Update the camera
  void UpdateCamera();

  ///
  /// get the currently active camera
  vtkCamera *GetActiveCamera();
  
  ///
  /// Update the interactors on all the angle widgets
  void UpdateAngleWidgetInteractors();
//BTX
  /// 
  /// encapsulated 3d widgets for each angle node
  std::map<std::string, vtkMeasurementsAngleWidgetClass *> AngleWidgets;
//ETX
  /// 
  /// get a angle widget by angle node id
  vtkMeasurementsAngleWidgetClass *GetAngleWidget(const char * nodeID);

  /// 
  /// set up a new angle widget for this node
  void AddAngleWidget(vtkMRMLMeasurementsAngleNode *angleNode);
  /// 
  /// remove angle widget for this node
  void RemoveAngleWidget(vtkMRMLMeasurementsAngleNode *angleNode);
  ///
  /// remove all angle widgets
  void RemoveAngleWidgets();

  ///
  /// update the visibility of the 3d widget associated with the passed node.
  /// if hte update clicks flag is true, fool the angle widget into thinking
  /// it's been placed, but it's false by default.
  void Update3DWidgetVisibility(vtkMRMLMeasurementsAngleNode *angleNode, bool updateClicks = false);

  /// 
  /// check scene to make sure that have a widget for each angle node, and no extra widgets...
  void Update3DWidgetsFromMRML();
  
  /// 
  /// Update the angle label with the value from the angle widget
  void UpdateAngleLabel(vtkMRMLMeasurementsAngleNode *angleNode);

  ///
  /// Update the lock state for all angle widgets. If lockFlag is 0, turn
  /// process events off, if lock flag is 1, turn process events on.
  void ModifyAllLock(int lockFlag);
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
  /// update the 3d angle widget from the settings in teh passed in
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
  vtkKWPushButton *VisibilityButton;
  vtkKWPushButton *Ray1VisibilityButton;
  vtkKWPushButton *Ray2VisibilityButton;
  vtkKWPushButton *ArcVisibilityButton;

  /// 
  /// Change the colour of the end points
  vtkKWChangeColorButton *PointColourButton;
  vtkKWChangeColorButton *Point2ColourButton;
  vtkKWChangeColorButton *PointCentreColourButton;
  /// 
  /// Change the colour of the line
  vtkKWChangeColorButton *LineColourButton;
  ///
  /// change the colour of the arc
  vtkKWChangeColorButton *ArcColourButton;
  /// 
  /// Change the colour of the label text
  vtkKWChangeColorButton *TextColourButton;

  ///
  /// angle label
  vtkKWLabel *AngleLabel;

  /// 
  /// point position entry
  vtkKWEntry *Position1XEntry;
  vtkKWEntry *Position1YEntry;
  vtkKWEntry *Position1ZEntry;
  vtkKWEntry *Position2XEntry;
  vtkKWEntry *Position2YEntry;
  vtkKWEntry *Position2ZEntry;
  vtkKWEntry *PositionCenterXEntry;
  vtkKWEntry *PositionCenterYEntry;
  vtkKWEntry *PositionCenterZEntry;

  ///
  /// remove all angle widgets
  vtkKWPushButton *RemoveAllAnglesButton;

  /// 
  /// angle annotation option entries
  vtkKWEntryWithLabel *LabelFormatEntry;
  vtkKWEntryWithLabel *LabelScaleEntry;
  vtkKWPushButton *LabelVisibilityButton;

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

  ///
  /// Resets the gui to neutral, when scene closes or nodes are all gone
  void ResetGUI();


private:


  vtkMeasurementsAngleWidget(const vtkMeasurementsAngleWidget&); /// Not implemented
  void operator=(const vtkMeasurementsAngleWidget&); /// Not Implemented
};

#endif

