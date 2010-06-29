/*=========================================================================

  Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   SlicerViewerWidget
  Module:    $HeadURL: http://svn.slicer.org/Slicer3/trunk/Base/GUI/vtkMRMLModelDisplayableManager.h $
  Date:      $Date: 2010-05-12 08:34:19 -0400 (Wed, 12 May 2010) $
  Version:   $Revision: 13332 $

==========================================================================*/

///  vtkSlicerNodeSelectorWidget - menu to select volumes from current mrml scene
/// 
/// Inherits most behavior from kw widget, but is specialized to observe
/// the current mrml scene and update the entries of the pop up menu to correspond
/// to the currently available volumes.  This widget also has a notion of the current selection
/// that can be observed or set externally
//


#ifndef __vtkMRMLModelDisplayableManager_h
#define __vtkMRMLModelDisplayableManager_h

#include "vtkMRMLDisplayableManagerWin32Header.h"

#include "vtkMRMLAbstractDisplayableManager.h"

#include "vtkRenderWindow.h"

#include "vtkMRMLModelHierarchyLogic.h"


#include "vtkMRMLClipModelsNode.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLCameraNode.h"
#include "vtkMRMLViewNode.h"

//#include "vtkBoundingBox.h"

class vtkMRMLDisplayableNode;
class vtkMRMLDisplayNode;
class vtkMRMLModelHierarchyNode;
class vtkMRMLModelHierarchyLogic;
class vtkPolyData;
class vtkCellArray;
class vtkProp3D;
class vtkActor;
class vtkActorText;
class vtkFollower;
class vtkImplicitBoolean;
class vtkPlane;
class vtkWorldPointPicker;
class vtkPropPicker;
class vtkCellPicker;
class vtkPointPicker;
class vtkPlane;
class vtkClipPolyData;
class vtkPMatrix4x4;
class vtkDisplayableManagerBoxWidget2;
class vtkDisplayableManagerBoxRepresentation;
class vtkBoundingBox;

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLModelDisplayableManager : public vtkMRMLAbstractDisplayableManager
{

public:
  static vtkMRMLModelDisplayableManager* New();
  vtkTypeRevisionMacro(vtkMRMLModelDisplayableManager,vtkMRMLAbstractDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);

  
  //virtual void Register(vtkObjectBase *o) { Superclass::Register(o); };
  //virtual void UnRegister(vtkObjectBase *o) { Superclass::UnRegister(o); };


  /// 
  /// Get/Set the Clip Nodes
  vtkGetObjectMacro ( ClipModelsNode, vtkMRMLClipModelsNode );
  void SetClipModelsNode (vtkMRMLClipModelsNode *snode)
    {
    vtkSetAndObserveMRMLNodeMacro( this->ClipModelsNode, snode );
    }

  /// 
  /// alternative method to propagate events generated in GUI to logic / mrml
  /// Here, it calls RequestRender in response to Expose and Configure events
  virtual void ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  /// 
  /// alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  /// 
  /// removes observers on widgets in the class
  virtual void RemoveWidgetObservers ( );

  /// 
  /// removes observers on widgets in the class
  virtual void RemoveMRMLObservers ( );

  /// 
  /// Get/Set the CameraNode
  vtkGetObjectMacro(CameraNode, vtkMRMLCameraNode);
  void SetAndObserveCameraNode (vtkMRMLCameraNode *snode)
    {
    vtkSetAndObserveMRMLNodeMacro( this->CameraNode, snode );
    };

  /// Description:
  /// Get/Set the ViewNode
  vtkGetObjectMacro(ViewNode, vtkMRMLViewNode);
  void SetAndObserveViewNode (vtkMRMLViewNode *snode)
    {
    vtkSetAndObserveMRMLNodeMacro( this->ViewNode, snode );
    };

  /// 
  /// return the current model actor corresponding to a give MRML ID
  vtkProp3D *GetActorByID (const char *id);

  /// 
  /// return the current node ID corresponding to a given vtkProp3D
  const char *GetIDByActor (vtkProp3D *actor);
  
  ///  
  /// Post a request for a render -- won't be done until the system is
  /// idle, and then only once....
  void RequestRender();

  ///  
  /// Actually do a render (don't wait for idle)
  //  Avoid using this method - use RequestRender to automatically compress
  //  multiple renders for better interactive performance
  void Render();

  /// 
  /// Show and hide widget

  void ColorAxisLabelActors ( double r, double g, double b);
  
  /// 
  /// Updates Actors based on models in the scene
  void UpdateFromMRML();

  /// 
  /// picks a world point
  vtkGetObjectMacro(WorldPointPicker, vtkWorldPointPicker);
  
  /// 
  /// picks a property in the scene
  vtkGetObjectMacro(PropPicker, vtkPropPicker);
  /// 
  /// picks a cell
  vtkGetObjectMacro(CellPicker, vtkCellPicker);
  /// 
  /// picks a point
  vtkGetObjectMacro(PointPicker, vtkPointPicker);
  
  /// 
  /// Convert an x/y location to a mrml node, 3d RAS point, point id, cell id,
  /// as appropriate depending what's found under the xy. Called by
  /// vtkSlicerViewerInteractorStyle PickEvent consumers. Returns 1 on
  /// successful pick.
  int Pick(int x, int y);

  /// 
  /// Get the name of the picked node, returns empty string if no pick
  const char *GetPickedNodeName()
  {    
    return this->PickedNodeName.c_str();
  }
  /// 
  /// Get/Set the picked RAS point, returns 0,0,0 if no pick
  vtkGetVectorMacro( PickedRAS, double, 3);
  vtkSetVectorMacro( PickedRAS, double, 3);
  /// 
  /// Get/Set the picked cell id, returns -1 if no pick
  vtkGetMacro( PickedCellID, vtkIdType);
  vtkSetMacro( PickedCellID, vtkIdType);
  /// 
  /// Get/Set the picked point id, returns -1 if no pick
  vtkGetMacro( PickedPointID, vtkIdType);
  vtkSetMacro( PickedPointID, vtkIdType);
  
  /// 
  /// get/set vtkMRMLModelHierarchyLogic
  vtkGetObjectMacro( ModelHierarchyLogic, vtkMRMLModelHierarchyLogic );
  vtkSetObjectMacro( ModelHierarchyLogic, vtkMRMLModelHierarchyLogic );

  vtkGetObjectMacro ( BoxWidget, vtkDisplayableManagerBoxWidget2 );
  vtkGetObjectMacro ( BoxWidgetRepresentation, vtkDisplayableManagerBoxRepresentation );

  void SetBoxWidgetInteractor();
  
  /// 
  /// Get/Set the enable renderer
  vtkGetMacro( EnableRender, int);
  vtkSetMacro( EnableRender, int);

  /// 
  /// Events
  //BTX
  enum
  {
    ActiveCameraChangedEvent   = 30000
  };
  //ETX

  void SetClipPlaneFromMatrix(vtkMatrix4x4 *sliceMatrix, 
                             int planeDirection,
                             vtkPlane *plane);
  ///
  /// get at the bounding box, to allow the default placement of widgets
  vtkGetObjectMacro(BoxAxisBoundingBox, vtkBoundingBox);

  int IsCreated()
  {
    return this->Created;
  }

  /// 
  /// Create the widget.
  virtual void Create();
  
protected:
  vtkMRMLModelDisplayableManager();
  virtual ~vtkMRMLModelDisplayableManager();


  int Created;

  
  void UpdateCameraNode();
  void UpdateViewNode();

  int RenderPending;
  int UpdateFromMRMLRequested;

  void RemoveModelProps();

  void RemoveModelObservers(int clearCache);
  void RemoveModelObservers( vtkMRMLDisplayableNode *model);

  void UpdateModelsFromMRML();
  void UpdateModel(vtkMRMLDisplayableNode *model);
  void UpdateModelPolyData(vtkMRMLDisplayableNode *model);
  void UpdateModifiedModel(vtkMRMLDisplayableNode *model);

  void CreateClipSlices();

  void CreateAxis();
  void AddAxisActors();
  void UpdateAxis();

  int UpdateClipSlicesFromMRML();

  void CheckModelHierarchies();
  void AddHierarchiyObservers();
  void RemoveHierarchyObservers(int clearCache);

  void UpdateModelHierarchies() {
    this->CheckModelHierarchies();
    this->AddHierarchiyObservers();
    };
  void UpdateModelHierarchyVisibility(vtkMRMLModelHierarchyNode* mhnode, int visibility );
  void UpdateModelHierarchyDisplay(vtkMRMLDisplayableNode *model);

  void SetModelDisplayProperty(vtkMRMLDisplayableNode *model);

  int GetDisplayedModelsVisibility(vtkMRMLDisplayNode *model);

  void RemoveDisplayable(vtkMRMLDisplayableNode* model);

  void AddCameraObservers();
  void RemoveCameraObservers();

  vtkMRMLDisplayNode*  GetHierarchyDisplayNode(vtkMRMLDisplayableNode *model);

  vtkClipPolyData* CreateTransformedClipper(vtkMRMLDisplayableNode *model);
  
  //BTX

  std::vector< vtkMRMLDisplayNode* > GetDisplayNode(vtkMRMLDisplayableNode *model);
  void RemoveDispalyedID(std::string &id);

  std::map<std::string, vtkProp3D *> DisplayedActors;
  std::map<std::string, vtkMRMLDisplayNode *> DisplayedNodes;
  std::map<std::string, int> DisplayedClipState;
  std::map<std::string, int> DisplayedVisibility;
  std::map<std::string, vtkMRMLDisplayableNode *> DisplayableNodes;

  std::vector<vtkFollower *> AxisLabelActors;

  std::map<std::string, int>  RegisteredModelHierarchies;

  //ETX

  vtkActor       *BoxAxisActor;
  vtkBoundingBox *BoxAxisBoundingBox;

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
  
  bool ModelHierarchiesPresent;

  vtkMRMLModelHierarchyLogic *ModelHierarchyLogic;

  vtkMRMLCameraNode *CameraNode;
  int CameraNodeWasCreated;

  vtkMRMLViewNode *ViewNode;

  bool SceneClosing;

  vtkWorldPointPicker *WorldPointPicker;
  vtkPropPicker *PropPicker;
  vtkCellPicker *CellPicker;
  vtkPointPicker *PointPicker;
  
  /// 
  /// information about a pick event
  //BTX
  std::string PickedNodeName;
  //ETX
  double PickedRAS[3];
  vtkIdType PickedCellID;
  vtkIdType PickedPointID;

  /// 
  /// Reset all the pick vars
  void ResetPick();
  
  vtkDisplayableManagerBoxWidget2         *BoxWidget;
  vtkDisplayableManagerBoxRepresentation  *BoxWidgetRepresentation;

  int EnableRender;

  int UpdatingAxis;

  int IsRendering;
  
private:
  
  vtkMRMLModelDisplayableManager(const vtkMRMLModelDisplayableManager&); /// Not implemented
  void operator=(const vtkMRMLModelDisplayableManager&); /// Not Implemented
};

#endif

