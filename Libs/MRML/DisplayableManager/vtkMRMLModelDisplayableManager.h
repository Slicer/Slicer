/*=========================================================================

  Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   SlicerViewerWidget
  Module:    $HeadURL: http://svn.slicer.org/Slicer4/trunk/Base/GUI/vtkMRMLModelDisplayableManager.h $
  Date:      $Date: 2010-05-12 08:34:19 -0400 (Wed, 12 May 2010) $
  Version:   $Revision: 13332 $

==========================================================================*/

#ifndef __vtkMRMLModelDisplayableManager_h
#define __vtkMRMLModelDisplayableManager_h

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractThreeDViewDisplayableManager.h"
#include "vtkMRMLDisplayableManagerExport.h"

// MRMLLogic includes
#include "vtkMRMLModelHierarchyLogic.h"

// MRML includes
#include <vtkMRMLModelNode.h>
class vtkMRMLClipModelsNode;
class vtkMRMLDisplayNode;
class vtkMRMLDisplayableNode;
class vtkMRMLModelHierarchyLogic;
class vtkMRMLModelHierarchyNode;
class vtkMRMLSelectionNode;
class vtkMRMLTransformNode;

// VTK includes
#include "vtkRenderWindow.h"
class vtkActor;
class vtkActorText;
class vtkAlgorithm;
class vtkBoundingBox;
class vtkCellArray;
class vtkCellPicker;
class vtkClipPolyData;
class vtkFollower;
class vtkImplicitBoolean;
class vtkLookupTable;
class vtkMatrix4x4;
class vtkPMatrix4x4;
class vtkPlane;
class vtkPlane;
class vtkPointPicker;
class vtkPolyData;
class vtkProp3D;
class vtkPropPicker;
class vtkWorldPointPicker;

/// \brief Manage display nodes with polydata in 3D views.
///
/// Any display node in the scene that contains a valid output polydata is
/// represented into the view renderer using configured synchronized vtkActors
/// and vtkMappers.
/// Note that the display nodes must be of type vtkMRMLModelDisplayNode
/// (to have an output polydata) but the displayable nodes don't necessarily
/// have to be of type vtkMRMLModelNode.
class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLModelDisplayableManager
  : public vtkMRMLAbstractThreeDViewDisplayableManager
{
public:
  static vtkMRMLModelDisplayableManager* New();
  vtkTypeMacro(vtkMRMLModelDisplayableManager,vtkMRMLAbstractThreeDViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

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

  /// Convert an x/y location to a MRML node, 3D RAS point, point ID, cell ID,
  /// as appropriate depending what's found under the xy.
  int Pick(int x, int y);

  /// Convert a RAS location to a MRML node, point ID, cell ID,
  /// as appropriate depending what's found under the position.
  int Pick3D(double ras[3]) override;

  /// Get/Set tolerance for Pick() method.
  /// it will call vtkCellPicker.Get/SetTolerance()
  double GetPickTolerance();
  void SetPickTolerance(double tolerance);

  ///
  /// Get the MRML ID of the picked node, returns empty string if no pick
  const char* GetPickedNodeID() override;

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

  void SetClipPlaneFromMatrix(vtkMatrix4x4 *sliceMatrix,
                             int planeDirection,
                             vtkPlane *plane);

  /// Return true if the node can be represented as a model
  bool IsModelDisplayable(vtkMRMLDisplayableNode* node)const;
  /// Return true if the display node is a model
  bool IsModelDisplayable(vtkMRMLDisplayNode* node)const;

  /// Helper function for copying lookup tables
  /// It handles special types of lookup tables and fixes
  /// error in vtkLoookupTable copy.
  static vtkLookupTable* CreateLookupTableCopy(vtkLookupTable* source);

  /// Helper function for determining what type of scalar is active.
  /// \return True if attribute location in display node is vtkAssignAttribute::CELL_DATA
  ///   or active cell scalar name in the model node is vtkDataSetAttributes::SCALARS.
  ///   False otherwise.
  static bool IsCellScalarsActive(vtkMRMLDisplayNode* displayNode, vtkMRMLModelNode* model = nullptr);

protected:

  vtkMRMLModelDisplayableManager();
  ~vtkMRMLModelDisplayableManager() override;

  void AdditionalInitializeStep() override;
  int ActiveInteractionModes() override;

  void UnobserveMRMLScene() override;

  void OnMRMLSceneStartClose() override;
  void OnMRMLSceneEndClose() override;
  void UpdateFromMRMLScene() override;
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;

  void OnInteractorStyleEvent(int eventId) override;
  void ProcessMRMLNodesEvents(vtkObject *caller, unsigned long event, void *callData) override;

  /// Returns true if something visible in modelNode has changed and would
  /// require a refresh.
  bool OnMRMLDisplayableModelNodeModifiedEvent(vtkMRMLDisplayableNode * modelNode);

  /// Updates Actors based on models in the scene
  void UpdateFromMRML() override;

  void RemoveMRMLObservers() override;

  friend class vtkMRMLThreeDViewInteractorStyle; // Access to RequestRender();

  void RemoveModelProps();
  void RemoveModelObservers(int clearCache);
  void RemoveDisplayable(vtkMRMLDisplayableNode* model);
  void RemoveDisplayableNodeObservers(vtkMRMLDisplayableNode *model);

  void UpdateModelsFromMRML();
  void UpdateModel(vtkMRMLDisplayableNode *model);
  void UpdateModelMesh(vtkMRMLDisplayableNode *model);
  void UpdateModifiedModel(vtkMRMLDisplayableNode *model);

  void SetModelDisplayProperty(vtkMRMLDisplayableNode *model);
  int GetDisplayedModelsVisibility(vtkMRMLDisplayNode *model);

  const char* GetActiveScalarName(vtkMRMLDisplayNode* displayNode,
                                  vtkMRMLModelNode* model = nullptr);

  /// Returns not null if modified
  int UpdateClipSlicesFromMRML();
  vtkAlgorithm *CreateTransformedClipper(vtkMRMLTransformNode *tnode,
                                         vtkMRMLModelNode::MeshTypeHint type);

  void AddHierarchyObservers();
  void RemoveHierarchyObservers(int clearCache);

  void CheckModelHierarchies();
  void UpdateModelHierarchies();
  void UpdateModelHierarchyVisibility(vtkMRMLModelHierarchyNode* mhnode, int visibility );
  void UpdateModelHierarchyDisplay(vtkMRMLDisplayableNode *model);

  vtkMRMLDisplayNode*  GetHierarchyDisplayNode(vtkMRMLDisplayableNode *model);

  void RemoveDispalyedID(std::string &id);

private:

  vtkMRMLModelDisplayableManager(const vtkMRMLModelDisplayableManager&) = delete;
  void operator=(const vtkMRMLModelDisplayableManager&) = delete;

  class vtkInternal;
  vtkInternal* Internal;
};

#endif
