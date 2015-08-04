/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLCameraNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/

#ifndef __vtkMRMLCameraNode_h
#define __vtkMRMLCameraNode_h

// MRML includes
#include "vtkMRMLTransformableNode.h"

// VTK includes
class vtkCamera;
class vtkMatrix4x4;
class vtkRenderer;

/// \brief MRML node to represent camera node
///
/// Camera node uses vtkCamera to store the state of the camera
class VTK_MRML_EXPORT vtkMRMLCameraNode : public vtkMRMLTransformableNode
{
public:
  static vtkMRMLCameraNode *New();
  vtkTypeMacro(vtkMRMLCameraNode,vtkMRMLTransformableNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();

  ///
  /// Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  ///
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);


  ///
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  ///
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "Camera";};

  ///
  /// Set the camera active tag, i.e. the tag for which object (view) this
  /// camera is active.
  const char* GetActiveTag();
  virtual void SetActiveTag(const char *);

  ///
  /// vtkCamera
  vtkGetObjectMacro(Camera, vtkCamera);

  ///
  /// Set camera ParallelProjection flag
  void SetParallelProjection(int parallelProjection);

  ///
  /// Set camera ParallelProjection flag
  int GetParallelProjection();

  ///
  /// Set camera Parallel Scale
  void SetParallelScale(double scale);

  ///
  /// Set camera Parallel Scale
  double GetParallelScale();

  ///
  /// Set the camera view angle
  /// \sa GetViewAngle(), vtkCamera::SetViewAngle(), SetParallelScale(),
  /// SetParallelProjection()
  void SetViewAngle(double viewAngle);

  ///
  /// Get the camera view angle
  /// \sa SetViewAngle(), vtkCamera::GetViewAngle(), GetParallelScale(),
  /// GetParallelProjection()
  double GetViewAngle();

  ///
  /// Set the position of the camera in world coordinates.
  /// \sa GetPosition(), SetFocalPoint(), SetViewUp()
  void SetPosition(double position[3]);
  inline void SetPosition(double x, double y, double z);

  ///
  /// Get the position of the camera in world coordinates.
  /// \sa SetPosition(), GetFocalPoint(), GetViewUp()
  double *GetPosition();
  void GetPosition(double position[3]);

  ///
  /// Set the focal point of the camera in world coordinates.
  /// It is also the point around which the camera rotates around.
  /// \sa GetFocalPoint(), SetPosition(), SetViewUp()
  void SetFocalPoint(double focalPoint[3]);
  inline void SetFocalPoint(double x, double y, double z);

  ///
  /// Get the focal point of the camera in world coordinates.
  /// \sa SetFocalPoint(), GetPosition(), GetViewUp()
  double *GetFocalPoint();
  void GetFocalPoint(double focalPoint[3]);

  ///
  /// Set camera Up vector
  /// \sa GetViewUp(), SetPosition(), SetFocalPoint()
  void SetViewUp(double viewUp[3]);
  inline void SetViewUp(double vx, double vy, double vz);

  ///
  /// Get camera Up vector
  /// \sa SetViewUp(), GetPosition(), GetFocalPoint()
  double *GetViewUp();
  void GetViewUp(double viewUp[3]);

  ///
  /// alternative method to propagate events generated in Camera nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ );

  /// This is the transform that was last applied
  /// to the position, focal point, and up vector
  /// (for any new transforms, the incremental difference
  /// is calculated and applied to the parameters)
  vtkGetObjectMacro(AppliedTransform, vtkMatrix4x4);
  virtual void SetAppliedTransform(vtkMatrix4x4* appliedTransform);

  ///
  /// Events
  enum
  {
    ActiveTagModifiedEvent = 30000
  };

  /// Mark the active tag node as references.
  virtual void SetSceneReferences();

  ///
  /// Updates this node if it depends on other nodes
  /// when the node is deleted in the scene
  virtual void UpdateReferences();

  ///
  /// Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  /// Reset the clipping range just based on its position and focal point
  void ResetClippingRange();

  enum Direction{
    Right = 0,
    Left = 1,
    Anterior = 2,
    Posterior = 3,
    Superior = 4,
    Inferior = 5
  };

  enum RASAxis{
    R = 0,
    A = 1,
    S = 2,
  };

  enum ScreenAxis{
    X = 0,
    Y = 1,
    Z = 2
  };

  /// Moves the camera toward a position.
  /// Keeps the same distance to the focal point.
  void RotateTo(Direction position);

  /// Utility function that rotates of 15 degrees around an
  /// axis. Call RotateAround 6 times to make a right angle
  void RotateAround(RASAxis axis, bool clockWise);

  /// 15 degrees by default
  void RotateAround(RASAxis axis, double angle = 15.);

  /// Translate the camera and focal point of a 6th of the screen width.
  /// Call TranslateAround 6 times to not see what was on screen before.
  void TranslateAlong(ScreenAxis axis, bool positive);

  /// Reset the camera
  /// If resetRotation is true, the camera rotates to the closest direction
  /// If resetTranslation is true, the focal point is moved to the center
  /// of the renderer props not changing the rotation.
  /// If resetDistance is true, the camera to moved to make sure the view
  /// contains the renderer bounds.
  using vtkMRMLNode::Reset;
  void Reset(bool resetRotation,
             bool resetTranslation = true,
             bool resetDistance = true,
             vtkRenderer* renderer = 0);
protected:
  vtkMRMLCameraNode();
  ~vtkMRMLCameraNode();
  vtkMRMLCameraNode(const vtkMRMLCameraNode&);
  void operator=(const vtkMRMLCameraNode&);


  void SetCamera(vtkCamera* camera);
  void SetAndObserveCamera(vtkCamera *camera);
  vtkCamera *Camera;

  vtkMRMLCameraNode* FindActiveTagInScene(const char *tag);

  void SetInternalActiveTag(const char* id);
  char *InternalActiveTag;

  vtkMatrix4x4 *AppliedTransform;
};

//---------------------------------------------------------------------------
void vtkMRMLCameraNode::SetPosition(double x, double y, double z)
{
  double pos[3] = {x, y, z};
  this->SetPosition(pos);
}

//---------------------------------------------------------------------------
void vtkMRMLCameraNode::SetFocalPoint(double x, double y, double z)
{
  double pos[3] = {x, y, z};
  this->SetFocalPoint(pos);
}

//---------------------------------------------------------------------------
void vtkMRMLCameraNode::SetViewUp(double vx, double vy, double vz)
{
  double viewUp[3] = {vx, vy, vz};
  this->SetViewUp(viewUp);
}

#endif
