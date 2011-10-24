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
  vtkTypeRevisionMacro(vtkMRMLSliceLinkLogic,vtkMRMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:

  vtkMRMLSliceLinkLogic();
  virtual ~vtkMRMLSliceLinkLogic();

  // On a change in scene, we need to manage the observations.
  virtual void SetMRMLSceneInternal(vtkMRMLScene * newScene);

  virtual void OnMRMLSceneNodeAddedEvent(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemovedEvent(vtkMRMLNode* node);
  virtual void OnMRMLNodeModified(vtkMRMLNode* node);

  // Used internally to control whether we are in the process of
  // broadcasting events. PIMPL it?
  vtkSetMacro(BroadcastingEvents, int);
  vtkGetMacro(BroadcastingEvents, int);
  vtkBooleanMacro(BroadcastingEvents, int);
 
  /// Broadcast a slice node to other slice nodes. 
  void BroadcastSliceNodeEvent(vtkMRMLSliceNode *sliceNode);

  /// Broadcast a slice composite node to other slice composite nodes
  void BroadcastSliceCompositeNodeEvent(vtkMRMLSliceCompositeNode *compositeNode);

private:

  vtkMRMLSliceLinkLogic(const vtkMRMLSliceLinkLogic&);
  void operator=(const vtkMRMLSliceLinkLogic&);

  int BroadcastingEvents;
};

#endif
