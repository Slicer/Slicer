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


#ifndef __vtkMeasurementsRulerWidget_h
#define __vtkMeasurementsRulerWidget_h

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
class vtkLineWidget2;
class vtkPointHandleRepresentation3D;
class vtkLineRepresentation;
class vtkPolygonalSurfacePointPlacer;
class vtkMRMLMeasurementsRulerNode;
class vtkSlicerViewerWidget;
class VTK_MEASUREMENTS_EXPORT vtkMeasurementsRulerWidget : public vtkSlicerWidget
{
  
public:
  static vtkMeasurementsRulerWidget* New();
  vtkTypeRevisionMacro(vtkMeasurementsRulerWidget,vtkSlicerWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // access methods
  vtkGetObjectMacro(VisibilityButton, vtkKWCheckButtonWithLabel);
  vtkGetObjectMacro(DistanceWidget, vtkLineWidget2);
  vtkGetObjectMacro(DistanceRepresentation, vtkLineRepresentation);

  // Description:
  // Getting the mrml ruler node id
  vtkGetStringMacro(RulerNodeID);
  
  // Description:
  // Set the fid list id, and update the widgets
  void SetRulerNodeID(char *id);
    
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
  // Get/set the viewer widget so can add a the ruler widget to it
  vtkGetObjectMacro(ViewerWidget, vtkSlicerViewerWidget);
  virtual void SetViewerWidget(vtkSlicerViewerWidget *viewerWidget);

 protected:
  vtkMeasurementsRulerWidget();
  virtual ~vtkMeasurementsRulerWidget();

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  // Description:
  // update the widget GUI from the settings in the passed in rulerNode
  void UpdateWidget(vtkMRMLMeasurementsRulerNode *rulerNode);
  // DescriptioN:
  // update the 3d distance widget from the settings in teh passed in
  // rulerNode
  void Update3DWidget(vtkMRMLMeasurementsRulerNode *rulerNode);

  // Description:
  // update the mrml node selected in the node selector to match the gui elements
  void UpdateMRMLFromGUI();
  // Description:
  // update the mrml node passed in, or the selected in the node selector if
  // null, to match the 3d widget
  void UpdateMRMLFromWidget(vtkMRMLMeasurementsRulerNode *rulerNode = NULL);
  
  // Description:
  // the id of the mrml node currently displayed in the widget
  char* RulerNodeID;
  // Description:
  // to select a measurements ruler node from the scene, and create new ones
  vtkSlicerNodeSelectorWidget* RulerSelectorWidget;

  // Description:
  // visibility check button
  vtkKWCheckButtonWithLabel *VisibilityButton;
  // Description:
  // Change the colour of the end points
  vtkKWChangeColorButton *PointColourButton;
  // Description:
  // Change the colour of the line
  vtkKWChangeColorButton *LineColourButton;


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


  // Description:
  // distance annotation option entries
  vtkKWEntryWithLabel *DistanceAnnotationFormatEntry;
  vtkKWEntryWithLabel *DistanceAnnotationScaleEntry;
  vtkKWCheckButtonWithLabel *DistanceAnnotationVisibilityButton;

  // Description:
  // number of subdivisions on the line
  vtkKWEntryWithLabel *ResolutionEntry;
  
  // Description:
  // the ruler widget
  vtkPointHandleRepresentation3D *DistanceHandleRepresentation;
  vtkLineRepresentation *DistanceRepresentation;
  vtkLineWidget2 *DistanceWidget;
  vtkPolygonalSurfacePointPlacer *RulerModel1PointPlacer;
  vtkPolygonalSurfacePointPlacer *RulerModel2PointPlacer;

  // Description:
  // Ruler GUI elements to select models upon which to constrain the ruler end points
  vtkSlicerNodeSelectorWidget* RulerModel1SelectorWidget;
  vtkSlicerNodeSelectorWidget* RulerModel2SelectorWidget;

  // Description:
  // Set this when processing an event
  int ProcessingMRMLEvent;
  int ProcessingWidgetEvent;

  // Description:
  // flag to set when updating the mrml node
  int UpdatingMRML;
  // Description:
  // flag set to 1 when updating the widget GUI
  int UpdatingWidget;
  // Description:
  // flag set to 1 when updating a 3d widget (todo: set it to the index of the
  // ruler node / 3d widget once have more than one)
  int Updating3DWidget;

  // Description:
  // pointer to the viewer widget so can add props, request renders 
  vtkSlicerViewerWidget *ViewerWidget;
  
private:


  vtkMeasurementsRulerWidget(const vtkMeasurementsRulerWidget&); // Not implemented
  void operator=(const vtkMeasurementsRulerWidget&); // Not Implemented
};

#endif

