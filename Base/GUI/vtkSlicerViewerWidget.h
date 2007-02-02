/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerViewerWidget.h,v $
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


#ifndef __vtkSlicerViewerWidget_h
#define __vtkSlicerViewerWidget_h

#include "vtkSlicerWidget.h"
#include "vtkSlicerApplicationLogic.h"

#include "vtkKWRenderWidget.h"
#include "vtkKWFrame.h"
#include "vtkMRMLClipModelsNode.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLCameraNode.h"
#include "vtkMRMLViewNode.h"

class vtkMRMLModelNode;
class vtkMRMLModelDisplayNode;
class vtkPolyData;
class vtkCellArray;
class vtkActor;
class vtkActorText;
class vtkFollower;
class vtkImplicitBoolean;
class vtkPlane;
class vtkWorldPointPicker;
class vtkPropPicker;
class vtkCellPicker;
class vtkPointPicker;
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerViewerWidget : public vtkSlicerWidget
{
  
public:
  static vtkSlicerViewerWidget* New();
  vtkTypeRevisionMacro(vtkSlicerViewerWidget,vtkSlicerWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get/Set the Clip Nodes
  vtkGetObjectMacro ( ClipModelsNode, vtkMRMLClipModelsNode );
  void SetClipModelsNode (vtkMRMLClipModelsNode *snode)
    {
    vtkSetAndObserveMRMLNodeMacro( this->ClipModelsNode, snode );
    }

  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  // Description:
  // removes observers on widgets in the class
  virtual void RemoveWidgetObservers ( );

  // Description:
  // removes observers on widgets in the class
  virtual void RemoveMRMLObservers ( );

  vtkGetObjectMacro (ApplicationLogic, vtkSlicerApplicationLogic );
  vtkSetObjectMacro (ApplicationLogic, vtkSlicerApplicationLogic );

  vtkGetObjectMacro(MainViewer, vtkKWRenderWidget);
  vtkSetObjectMacro(MainViewer, vtkKWRenderWidget);
  vtkGetObjectMacro (ViewerFrame, vtkKWFrame );

  // Description:
  // Get/Set the CamerNode
  vtkGetObjectMacro(CameraNode, vtkMRMLCameraNode);
  void SetAndObserveCameraNode (vtkMRMLCameraNode *snode)
    {
    vtkSetAndObserveMRMLNodeMacro( this->CameraNode, snode );
    };

 // Description:
  // Get/Set the CamerNode
  vtkGetObjectMacro(ViewNode, vtkMRMLViewNode);
  void SetAndObserveViewNode (vtkMRMLViewNode *snode)
    {
    vtkSetAndObserveMRMLNodeMacro( this->ViewNode, snode );
    };

  // Description:
  // return the current model actor corresponding to a give MRML ID
  vtkActor *GetActorByID (const char *id);

  // Description:
  // return the current node ID corresponding to a given vtkActor
  const char *GetIDByActor (vtkActor *actor);
  
  // Description: 
  // Post a request for a render -- won't be done until the system is
  // idle, and then only once....
  void RequestRender();

  // Description: 
  // Actually do a render (don't wait for idle)
  void Render();

  // Description: 
  // Used to track the fact that there is a idle task pending requesting a render
  vtkSetMacro (RenderPending, int);
  vtkGetMacro (RenderPending, int);

  // Description:
  // Show and hide widget
  virtual void PackWidget ( vtkKWFrame *f );
  virtual void UnpackWidget ( );
  virtual void GridWidget ( vtkKWFrame *f, int row, int col );
  virtual void UngridWidget ( );
  void ColorAxisLabelActors ( double r, double g, double b);
  
  // Description:
  // Updates Actors based on models in the scene
  void UpdateFromMRML();

  // Description:
  // Pick event
  //BTX
  enum
  {
    PickEvent,
  };
  //ETX

  // Description:
  // picks a world point
  vtkGetObjectMacro(WorldPointPicker, vtkWorldPointPicker);
  
  // Description:
  // picks a property in the scene
  vtkGetObjectMacro(PropPicker, vtkPropPicker);
  // Description:
  // picks a cell
  vtkGetObjectMacro(CellPicker, vtkCellPicker);
  // Description:
  // picks a point
  vtkGetObjectMacro(PointPicker, vtkPointPicker);
  
  // Description:
  // Convert an x/y location to a Pick event, called by the
  // vtkSlicerViewerInteractorStyle. 
  void Pick(int x, int y);

  // Description:
  // Get the name of the picked node, returns empty string if no pick
  const char *GetPickedNodeName()
  {    
    return this->PickedNodeName.c_str();
  }
  // Description:
  // Get/Set the picked RAS point, returns 0,0,0 if no pick
  vtkGetVectorMacro( PickedRAS, double, 3);
  vtkSetVectorMacro( PickedRAS, double, 3);
  // Description:
  // Get/Set the picked cell id, returns -1 if no pick
  vtkGetMacro( PickedCellID, vtkIdType);
  vtkSetMacro( PickedCellID, vtkIdType);
  // Description:
  // Get/Set the picked point id, returns -1 if no pick
  vtkGetMacro( PickedPointID, vtkIdType);
  vtkSetMacro( PickedPointID, vtkIdType);
protected:
  vtkSlicerViewerWidget();
  virtual ~vtkSlicerViewerWidget();

  // Description:
  // Create the widget.
  virtual void CreateWidget();
  
  void UpdateCameraNode();
  void UpdateViewNode();

  vtkSlicerApplicationLogic *ApplicationLogic;
  vtkKWRenderWidget *MainViewer;
  vtkKWFrame *ViewerFrame;
  int RenderPending;

  void RemoveModelProps();

  void RemoveModelObservers();

  void UpdateModelsFromMRML();
  void UpdateModel(vtkMRMLModelNode *model);
  void UpdateModelPolyData(vtkMRMLModelNode *model);

  void CreateClipSlices();

  void CreateAxis();
  void AddAxisActors();
  void UpdateAxis();

  int UpdateClipSlicesFormMRML();

  void SetModelDisplayProperty(vtkMRMLModelNode *model,  vtkActor *actor);

  //BTX
  std::map<const char *, vtkActor *> DisplayedModels;

  std::map<const char *, int> DisplayedModelsClipState;

  std::vector<vtkFollower *> AxisLabelActors;
  //ETX
  vtkActor *BoxAxisActor;

  int ProcessingMRMLEvent;

  vtkMRMLClipModelsNode *ClipModelsNode;

  vtkMRMLSliceNode *RedSliceNode;
  vtkMRMLSliceNode *GreenSliceNode;
  vtkMRMLSliceNode *YellowSliceNode;

  vtkImplicitBoolean *SlicePlanes;
  vtkPlane *RedSlicePlane;
  vtkPlane *GreenSlicePlane;
  vtkPlane *YellowSlicePlane;

  int ClipType;
  int RedSliceClipState;
  int YellowSliceClipState;
  int GreenSliceClipState;

  bool ClippingOn;
  
  vtkMRMLCameraNode *CameraNode;
  vtkMRMLViewNode *ViewNode;

  bool SceneClosing;

  vtkWorldPointPicker *WorldPointPicker;
  vtkPropPicker *PropPicker;
  vtkCellPicker *CellPicker;
  vtkPointPicker *PointPicker;
  
  // Description:
  // information about a pick event
  //BTX
  std::string PickedNodeName;
  //ETX
  double PickedRAS[3];
  vtkIdType PickedCellID;
  vtkIdType PickedPointID;

  // Description:
  // Reset all the pick vars
  void ResetPick();
  
private:
  
  vtkSlicerViewerWidget(const vtkSlicerViewerWidget&); // Not implemented
  void operator=(const vtkSlicerViewerWidget&); // Not Implemented
};

#endif

