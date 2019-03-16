/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLSliceLinkLogic.h,v $
  Date:      $Date$
  Version:   $Revision$

=========================================================================auto=*/

///  vtkMRMLSliceLinkLogic - slicer logic class for linked slice manipulation
///
/// This class manages the logic associated with linking the controls
/// of multiple slice and slice composite nodes. It listens to the
/// MRML scene for new slice and slice composite nodes and observes
/// these nodes for ModifiedEvents. When notified of a ModifiedEvent
/// on a slice or slice composite node, this logic class will
/// propagate state to other slice and slice composite nodes. A
/// critical component of the design is that slice and slice composite
/// nodes "know" when they are be changed interactively verses when
/// their state is being updated programmatically.

#ifndef __vtkMRMLSliceLinkLogic_h
#define __vtkMRMLSliceLinkLogic_h

// MRMLLogic includes
#include "vtkMRMLAbstractLogic.h"

// STD includes
#include <vector>

class vtkMRMLSliceNode;
class vtkMRMLSliceCompositeNode;

class VTK_MRML_LOGIC_EXPORT vtkMRMLSliceLinkLogic : public vtkMRMLAbstractLogic
{
public:

  /// The Usual VTK class functions
  static vtkMRMLSliceLinkLogic *New();
  vtkTypeMacro(vtkMRMLSliceLinkLogic,vtkMRMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:

  vtkMRMLSliceLinkLogic();
  ~vtkMRMLSliceLinkLogic() override;

  // On a change in scene, we need to manage the observations.
  void SetMRMLSceneInternal(vtkMRMLScene * newScene) override;

  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;
  void OnMRMLNodeModified(vtkMRMLNode* node) override;
  void OnMRMLSceneStartBatchProcess() override;
  void OnMRMLSceneEndBatchProcess() override;
  void OnMRMLSceneStartImport() override;
  void OnMRMLSceneEndImport() override;
  void OnMRMLSceneStartRestore() override;
  void OnMRMLSceneEndRestore() override;

  // Used internally to control whether we are in the process of
  // broadcasting events. PIMPL it?
  void BroadcastingEventsOn();
  void BroadcastingEventsOff();
  int GetBroadcastingEvents();

  /// Broadcast a slice node to other slice nodes.
  void BroadcastSliceNodeEvent(vtkMRMLSliceNode *sliceNode);

  /// Broadcast a slice composite node to other slice composite nodes
  void BroadcastSliceCompositeNodeEvent(vtkMRMLSliceCompositeNode *compositeNode);

  /// Returns true if orientation of the slices match. Slice position and scaling is ignored.
  bool IsOrientationMatching(vtkMRMLSliceNode *sliceNode1, vtkMRMLSliceNode *sliceNode2, double comparisonTolerance = 0.001);

private:

  vtkMRMLSliceLinkLogic(const vtkMRMLSliceLinkLogic&) = delete;
  void operator=(const vtkMRMLSliceLinkLogic&) = delete;

  vtkMRMLSliceCompositeNode* GetCompositeNode(vtkMRMLSliceNode*);
  void BroadcastLastRotation(vtkMRMLSliceNode*, vtkMRMLSliceNode*);
  void UpdateSliceNodeInteractionStatus(vtkMRMLSliceNode*);

  // Counter on nested requests for Broadcasting events. Counter is
  // used as scene restores and scene view restores issue several
  // nested events and we want to block from the first Start to the
  // last End event (StartBatchProcess, StartImport, StartRestore).
  int BroadcastingEvents;

  struct SliceNodeInfos
    {
    SliceNodeInfos(int interacting) : Interacting(interacting) {}
    double LastNormal[3];
    int Interacting;
    };

  typedef std::map<std::string, SliceNodeInfos> SliceNodeStatusMap;
  SliceNodeStatusMap SliceNodeInteractionStatus;

};

#endif
