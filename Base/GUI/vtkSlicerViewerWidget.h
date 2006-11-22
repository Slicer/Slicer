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

#include "vtkKWRenderWidget.h"
#include "vtkKWFrame.h"
#include "vtkMRMLClipModelsNode.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLCameraNode.h"

class vtkMRMLModelNode;
class vtkMRMLModelDisplayNode;
class vtkMRMLFiducialListNode;
class vtkMRMLFiducialListDisplayNode;
class vtkMRMLFiducial;
class vtkPolyData;
class vtkActor;
class vtkFollower;
class vtkImplicitBoolean;
class vtkPlane;

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
  // return the current model actor corresponding to a give MRML ID
  vtkActor *GetActorByID (const char *id);

  // Description:
  // return the current node ID corresponding to a given vtkActor
  const char *GetIDByActor (vtkActor *actor);
  
  // Description:
  // return the current actor corresponding to a give MRML ID and fiducial index
  vtkActor *GetFiducialActorByID(const char *id, int index);

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
  virtual void PackWidget ( );
  virtual void UnpackWidget ( );
  virtual void GridWidget ( int row, int col );
  virtual void UngridWidget ( );
  
  // Description:
  // Updates Actors based on models in the scene
  void UpdateFromMRML();

protected:
  vtkSlicerViewerWidget();
  virtual ~vtkSlicerViewerWidget();

  // Description:
  // Create the widget.
  virtual void CreateWidget();
  
  void UpdateCameraNode();

  vtkKWRenderWidget *MainViewer;
  vtkKWFrame *ViewerFrame;
  int RenderPending;

  void RemoveModelProps();
  void RemoveFiducialProps();

  void RemoveModelObservers();
  void RemoveFiducialObservers();

  void UpdateFiducialsFromMRML();
  void UpdateModelsFromMRML();
  void UpdateModel(vtkMRMLModelNode *model);
  void UpdateModelPolyData(vtkMRMLModelNode *model);

  void CreateClipSlices();
  int UpdateClipSlicesFormMRML();

  void SetModelDisplayProperty(vtkMRMLModelNode *model,  vtkActor *actor);
  void SetFiducialDisplayProperty(vtkMRMLFiducialListNode *flist, int n,
                                  vtkActor *actor, vtkFollower *textActor);

  //BTX
  std::map<const char *, vtkActor *> DisplayedModels;
  std::map<const char *, vtkActor *> DisplayedFiducials;
  std::map<const char *, vtkFollower *> DisplayedTextFiducials;

  std::map<const char *, int> DisplayedModelsClipState;

  std::string GetFiducialActorID (const char *id, int index);
  std::string GetFiducialNodeID (const char *actorid, int &index);

  //ETX

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

  bool SceneClosing;

private:
  
  vtkSlicerViewerWidget(const vtkSlicerViewerWidget&); // Not implemented
  void operator=(const vtkSlicerViewerWidget&); // Not Implemented
};

#endif

