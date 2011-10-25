/*=========================================================================

  Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   SlicerViewerWidget
  Module:    $HeadURL: http://svn.slicer.org/Slicer4/trunk/Base/GUI/vtkMRMLModelDisplayableManager.h $
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

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractThreeDViewDisplayableManager.h"

// MRMLLogic includes
#include "vtkMRMLModelHierarchyLogic.h"

// MRML includes
#include "vtkMRMLClipModelsNode.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLCameraNode.h"
#include "vtkMRMLViewNode.h"

// VTK includes
#include "vtkRenderWindow.h"

#include "vtkMRMLDisplayableManagerWin32Header.h"

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
class vtkBoundingBox;

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLModelDisplayableManager :
    public vtkMRMLAbstractThreeDViewDisplayableManager
{
public:
  static vtkMRMLModelDisplayableManager* New();
  vtkTypeRevisionMacro(vtkMRMLModelDisplayableManager,vtkMRMLAbstractThreeDViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// 
  /// Get/Set the ClipModels Node
  vtkMRMLClipModelsNode* GetClipModelsNode();
  void SetClipModelsNode(vtkMRMLClipModelsNode *snode);

  /// 
  /// Return the current model actor corresponding to a give MRML ID
  vtkProp3D *GetActorByID(const char *id);

  /// 
  /// Return the current node ID corresponding to a given vtkProp3D
  const char *GetIDByActor(vtkProp3D *actor);

  /// 
  /// Get world point picker
  vtkWorldPointPicker* GetWorldPointPicker();
  
  /// 
  /// Get property picker
  vtkPropPicker* GetPropPicker();

  /// 
  /// Get cell picker
  vtkCellPicker* GetCellPicker();

  /// 
  /// Get point picker
  vtkPointPicker* GetPointPicker();
  
  /// Convert an x/y location to a mrml node, 3d RAS point, point id, cell id,
  /// as appropriate depending what's found under the xy.
  int Pick(int x, int y);

  /// 
  /// Get the name of the picked node, returns empty string if no pick
  const char *GetPickedNodeName();
  
  /// 
  /// Get/Set the picked RAS point, returns 0,0,0 if no pick
  double* GetPickedRAS();
  void SetPickedRAS(double* newPickedRAS);
  
  /// Get/Set the picked cell id, returns -1 if no pick
  vtkIdType GetPickedCellID();
  void SetPickedCellID(vtkIdType newCellID);

  /// 
  /// Get/Set the picked point id, returns -1 if no pick
  vtkIdType GetPickedPointID();
  void SetPickedPointID(vtkIdType newPointID);
  
  /// 
  /// Get/Set vtkMRMLModelHierarchyLogic
  vtkMRMLModelHierarchyLogic* GetModelHierarchyLogic();
  void SetModelHierarchyLogic(vtkMRMLModelHierarchyLogic* newModelHierarchyLogic);

  void SetClipPlaneFromMatrix(vtkMatrix4x4 *sliceMatrix, 
                             int planeDirection,
                             vtkPlane *plane);

  /// Return true if the node can be represented as a model
  bool IsModelDisplayable(vtkMRMLDisplayableNode* node)const;
  /// Return true if the display node is a model
  bool IsModelDisplayable(vtkMRMLDisplayNode* node)const;
protected:

  vtkMRMLModelDisplayableManager();
  virtual ~vtkMRMLModelDisplayableManager();

  virtual void AdditionnalInitializeStep();

  virtual void OnMRMLSceneAboutToBeClosedEvent();
  virtual void OnMRMLSceneClosedEvent();
  virtual void OnMRMLSceneImportedEvent();
  virtual void OnMRMLSceneRestoredEvent();
  virtual void OnMRMLSceneNodeAddedEvent(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemovedEvent(vtkMRMLNode* node);

  virtual void ProcessMRMLNodesEvents(vtkObject *caller, unsigned long event, void *callData);

  ///
  /// Returns true if something visible in modelNode has changed and would
  /// require a refresh.
  bool OnMRMLDisplayableModelNodeModifiedEvent(vtkMRMLDisplayableNode * modelNode);

  ///
  /// Updates Actors based on models in the scene
  void UpdateFromMRML();

  virtual void RemoveMRMLObservers();

  //BTX
  friend class vtkThreeDViewInteractorStyle; // Access to RequestRender();
  //ETX

  void RemoveModelProps();
  void RemoveModelObservers(int clearCache);
  void RemoveDisplayable(vtkMRMLDisplayableNode* model);
  void RemoveDisplayableNodeObservers(vtkMRMLDisplayableNode *model);

  void UpdateModelsFromMRML();
  void UpdateModel(vtkMRMLDisplayableNode *model);
  void UpdateModelPolyData(vtkMRMLDisplayableNode *model);
  void UpdateModifiedModel(vtkMRMLDisplayableNode *model);

  void SetModelDisplayProperty(vtkMRMLDisplayableNode *model);
  int GetDisplayedModelsVisibility(vtkMRMLDisplayNode *model);

  /// Returns not null if modified
  int UpdateClipSlicesFromMRML();
  vtkClipPolyData* CreateTransformedClipper(vtkMRMLDisplayableNode *model);

  void AddHierarchyObservers();
  void RemoveHierarchyObservers(int clearCache);

  void CheckModelHierarchies();
  void UpdateModelHierarchies();
  void UpdateModelHierarchyVisibility(vtkMRMLModelHierarchyNode* mhnode, int visibility );
  void UpdateModelHierarchyDisplay(vtkMRMLDisplayableNode *model);

  vtkMRMLDisplayNode*  GetHierarchyDisplayNode(vtkMRMLDisplayableNode *model);

  /// if any of the parent nodes have display nodes with the visibility flag
  /// set to false, return false, else, true. If no hierarchy, return true
  bool GetHierarchyVisibility(vtkMRMLDisplayableNode *model);
  
  //BTX
  void RemoveDispalyedID(std::string &id);
  //ETX
  
private:
  
  vtkMRMLModelDisplayableManager(const vtkMRMLModelDisplayableManager&); // Not implemented
  void operator=(const vtkMRMLModelDisplayableManager&);                 // Not Implemented

  //BTX
  class vtkInternal;
  vtkInternal* Internal;
  //ETX

};

#endif

