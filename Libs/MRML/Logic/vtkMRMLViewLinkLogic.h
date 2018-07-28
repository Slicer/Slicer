/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLSliceLinkLogic.h,v $
  Date:      $Date$
  Version:   $Revision$

=========================================================================auto=*/

///  vtkMRMLViewLinkLogic - slicer logic class for linked view manipulation
///
/// This class manages the logic associated with linking the controls
/// of multiple view and camera nodes. It listens to the
/// MRML scene for new view and camera nodes and observes
/// these nodes for ModifiedEvents. When notified of a ModifiedEvent
/// on a view or camera node, this logic class will
/// propagate state to other view and camera nodes. A
/// critical component of the design is that view and camera
/// nodes "know" when they are be changed interactively verses when
/// their state is being updated programmatically.

#ifndef __vtkMRMLViewLinkLogic_h
#define __vtkMRMLViewLinkLogic_h

// MRMLLogic includes
#include "vtkMRMLAbstractLogic.h"

// STD includes
#include <vector>

class vtkMRMLViewNode;
class vtkMRMLCameraNode;

class VTK_MRML_LOGIC_EXPORT vtkMRMLViewLinkLogic : public vtkMRMLAbstractLogic
{
public:

  /// The Usual VTK class functions
  static vtkMRMLViewLinkLogic *New();
  vtkTypeMacro(vtkMRMLViewLinkLogic,vtkMRMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

protected:

  vtkMRMLViewLinkLogic();
  virtual ~vtkMRMLViewLinkLogic();

  // On a change in scene, we need to manage the observations.
  virtual void SetMRMLSceneInternal(vtkMRMLScene * newScene) VTK_OVERRIDE;

  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node) VTK_OVERRIDE;
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) VTK_OVERRIDE;
  virtual void OnMRMLNodeModified(vtkMRMLNode* node) VTK_OVERRIDE;

  /// Broadcast a view node to other view nodes.
  void BroadcastViewNodeEvent(vtkMRMLViewNode *viewNode);

  /// Broadcast a camera node to other camera nodes
  void BroadcastCameraNodeEvent(vtkMRMLCameraNode *cameraNode);

private:

  vtkMRMLViewLinkLogic(const vtkMRMLViewLinkLogic&);
  void operator=(const vtkMRMLViewLinkLogic&);

};

#endif
