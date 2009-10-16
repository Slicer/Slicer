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

  // access methods
  vtkGetObjectMacro(VisibilityButton, vtkKWCheckButtonWithLabel);
  vtkGetObjectMacro (AllVisibilityMenuButton, vtkKWMenuButton);

  // Description:
  // Getting the mrml angle node id
  vtkGetStringMacro(AngleNodeID);
  
  // Description:
  // Set the fid list id, and update the widgets
  void SetAngleNodeID(char *id);
    
  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  // Description:
  // adds observes on widgets in the class
  virtual void AddWidgetObservers();
  

  // Description:
  // removes observers on widgets in the class
  virtual void RemoveWidgetObservers ( );

  // Description:
  // add observers on node
  virtual void AddMRMLObservers ( );

  // Description:
  // remove observers on node
  virtual void RemoveMRMLObservers ( );

  // Description:
  // Get/set the viewer widget so can add a the angle widget to it
  vtkGetObjectMacro(ViewerWidget, vtkSlicerViewerWidget);
  virtual void SetViewerWidget(vtkSlicerViewerWidget *viewerWidget);
//BTX
  // Description:
  // encapsulated 3d widgets for each angle node
  std::map<std::string, vtkMeasurementsAngleWidgetClass *> AngleWidgets;
//ETX
  // Description:
  // get a distance widget by angle node id
  vtkMeasurementsAngleWidgetClass *GetAngleWidget(const char * nodeID);

  // Description:
  // set up a new distance widget for this node
  void AddAngleWidget(vtkMRMLMeasurementsAngleNode *angleNode);
  // Description:
  // remove distance widget for this node
  void RemoveAngleWidget(vtkMRMLMeasurementsAngleNode *angleNode);
  // Description:
  // check scene to make sure that have a widget for each angle node, and no extra widgets...
  void Update3DWidgetsFromMRML();

protected:
  vtkMeasurementsAngleWidget();
  virtual ~vtkMeasurementsAngleWidget();

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  // Description:
  // update the widget GUI from the settings in the passed in angleNode
  void UpdateWidget(vtkMRMLMeasurementsAngleNode *angleNode);
  // DescriptioN:
  // update the 3d distance widget from the settings in teh passed in
  // angleNode
  void Update3DWidget(vtkMRMLMeasurementsAngleNode *angleNode);

  // Description:
  // Set visibility on all angle nodes in the scene to the passed in visibility flag
  void ModifyAllAngleVisibility (int visibilityState);

  // Description:
  // the id of the mrml node currently displayed in the widget
  char* AngleNodeID;
  // Description:
  // to select a measurements angle node from the scene, and create new ones
  vtkSlicerNodeSelectorWidget* AngleSelectorWidget;

  // Description:
  // visibility check buttons
  vtkKWCheckButtonWithLabel *VisibilityButton;
  vtkKWCheckButtonWithLabel *Ray1VisibilityButton;
  vtkKWCheckButtonWithLabel *Ray2VisibilityButton;
  vtkKWCheckButtonWithLabel *ArcVisibilityButton;

  // Description:
  // Change the colour of the end points
  vtkKWChangeColorButton *PointColourButton;
  // Description:
  // Change the colour of the line
  vtkKWChangeColorButton *LineColourButton;
  // Description:
  // Change the colour of the label text
  vtkKWChangeColorButton *TextColourButton;
  
  // Description:
  // point position entry
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


  // Description:
  // distance annotation option entries
  vtkKWEntryWithLabel *LabelFormatEntry;
  vtkKWEntryWithLabel *LabelScaleEntry;
  vtkKWCheckButtonWithLabel *LabelVisibilityButton;

  // Description:
  // number of subdivisions on the line
  vtkKWEntryWithLabel *ResolutionEntry;

  // Description:
  // Angle GUI elements to select models upon which to constrain the angle end points
  vtkSlicerNodeSelectorWidget* AngleModel1SelectorWidget;
  vtkSlicerNodeSelectorWidget* AngleModel2SelectorWidget;
  vtkSlicerNodeSelectorWidget* AngleModelCenterSelectorWidget;

  // Description:
  // menu button whose menu exposes options for
  // setting visibility of all angle nodes
  vtkKWMenuButton *AllVisibilityMenuButton;

  // Description:
  // flag set to 1 when updating a 3d widget (todo: set it to the index of the
  // angle node / 3d widget once have more than one)
  int Updating3DWidget;

  // Description:
  // pointer to the viewer widget so can add props, request renders 
  vtkSlicerViewerWidget *ViewerWidget;
  
private:


  vtkMeasurementsAngleWidget(const vtkMeasurementsAngleWidget&); // Not implemented
  void operator=(const vtkMeasurementsAngleWidget&); // Not Implemented
};

#endif

