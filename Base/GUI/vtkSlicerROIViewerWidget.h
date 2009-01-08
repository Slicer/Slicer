/*=auto=========================================================================

  Portions (c) Copyright 2006 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerROIViewerWidget.h,v $
  Date:      $Date: $
  Version:   $Revision: $

=========================================================================auto=*/

// .NAME vtkSlicerROIViewerWidget - 
// .SECTION Description
// Inherits most behavior from kw widget, but is specialized to observe
// the current mrml scene and update the ROI 3d graphics to correspond
// to the currently available lists. 
//

#ifndef __vtkSlicerROIViewerWidget_h
#define __vtkSlicerROIViewerWidget_h

#include "vtkSlicerWidget.h"
#include "vtkKWRenderWidget.h"

#include "vtkSlicerBoxWidget2.h"
#include "vtkSlicerBoxRepresentation.h"
#include "vtkSlicerViewerWidget.h"

class vtkMRMLROINode;
class vtkKWRenderWidget;
class vtkTransform;
class vtkCollection;
class vtkBoxWidgetCallback;

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerROIViewerWidget : public vtkSlicerWidget
{
public:
  static vtkSlicerROIViewerWidget* New();
  vtkTypeRevisionMacro(vtkSlicerROIViewerWidget,vtkSlicerWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  //virtual void ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData );

  // Description:
  // add mrml scene observers 
  virtual void AddMRMLObservers();
  
  // Description:
  // removes observers on widgets in the class
  virtual void RemoveMRMLObservers ( );

  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  
  // Description:
  // Set/Get the main viewer, called by vtkSlicerApplicationGUI
  void SetMainViewerWidget(vtkSlicerViewerWidget *viewer)
    {
    this->MainViewerWidget = viewer;
    };

  vtkGetObjectMacro(MainViewerWidget, vtkSlicerViewerWidget);
  
  // Description:
  // Updates all roi's based on mrml nodes
  void UpdateFromMRML();

  // Description:
  // Updates an roi's based on mrml nodes
  void UpdateROIFromMRML(vtkMRMLROINode *roi);

  // Description:
  // Updates an roi's based on transform of the ROI to World
  void UpdateROITransform(vtkMRMLROINode *roi);

  // Description: 
  // Post a request for a render -- won't be done until the system is
  // idle, and then only once....
  void RequestRender();

  // Description: 
  // Actually do a render (don't wait for idle)
  void Render();


  // Description:
  // return the current box widget corresponding to a given MRML ID
  vtkSlicerBoxWidget2 *GetBoxWidgetByID (const char *id);

  // Description:
  // return the current box widget callback corresponding to a given MRML ID

  vtkBoxWidgetCallback *GetBoxWidgetCallbackByID (const char *id);

  // Description:
  // Remove all the box widgets from the scene and memory
  void RemoveBoxWidgets();

  // Description:
  // Remove the box widget associated with this id
  void RemoveBoxWidget(const char *pointID);
  
  // Description:
  // Remove the box widget
  void RemoveBoxWidget(vtkSlicerBoxWidget2* boxWidget);
  
  void SetProcessingWidgetEvent(int flag)
  {
    this->ProcessingWidgetEvent = flag;
  };

protected:
  vtkSlicerROIViewerWidget();
  virtual ~vtkSlicerROIViewerWidget();

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  // Description:
  // Add observers for this MRML node
  void AddMRMLROIObservers(vtkMRMLROINode *roi);

  // Description:
  // Remove observers for all MRML nodes
  void RemoveMRMLROIObservers();

  // Description:
  // Remove observers for this MRML node
  void RemoveMRMLROINodeObservers(vtkMRMLROINode *roi);
  
  
  // Description:
  // Removes observers that this widget placed on the ROI node in the
  // mrml tree
  void RemoveROIObservers();

  // Description:
  // Goes through the MRML scene and for each roi
  // node, calls UpdateROIFromMRML. Once done, requests a render.
  void UpdateROIsFromMRML();

  
  //BTX
  std::map<std::string, vtkSlicerBoxWidget2*> DisplayedBoxWidgets; 
  std::map<std::string, vtkBoxWidgetCallback*> DisplayedBoxWidgetCallbacks; 
  //ETX
  
  // Description:
  // Flag set to 1 when processing mrml events
  int ProcessingMRMLEvent;
  int ProcessingWidgetEvent;
  

  // Description:
  // A pointer back to the main viewer, so that can render when update
  vtkSlicerViewerWidget *MainViewerWidget;
 
    
private:
  
  vtkSlicerROIViewerWidget(const vtkSlicerROIViewerWidget&); // Not implemented
  void operator=(const vtkSlicerROIViewerWidget&); // Not Implemented
};

#endif
