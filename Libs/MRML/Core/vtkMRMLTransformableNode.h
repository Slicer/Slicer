/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLTransformableNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/

#ifndef __vtkMRMLTransformableNode_h
#define __vtkMRMLTransformableNode_h

// MRML includes
#include "vtkMRMLStorableNode.h"
#include "vtkVector.h"
class vtkMRMLTransformNode;

// VTK includes
class vtkAbstractTransform;
class vtkMatrix4x4;

/// \brief MRML node for representing a node with a transform.
///
/// A superclass for other nodes that can have a transform to parent node
/// like volume, model and transformation nodes.
class VTK_MRML_EXPORT vtkMRMLTransformableNode : public vtkMRMLStorableNode
{
public:
  vtkTypeMacro(vtkMRMLTransformableNode,vtkMRMLStorableNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override = 0;

  ///
  /// Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override = 0;

  ///
  /// Set a reference to transform node
  /// Returns true on success.
  /// The method will fail if a child transform of a transform node is attempted
  /// to be set as parent to prevent circular reference.
  /// If current node or new parent transform node is not added to the scene yet
  /// then circular reference is not checked and it is the developer's responsibility
  /// no child transform is set as parent.
  bool SetAndObserveTransformNodeID(const char *transformNodeID);

  ///
  /// Associated transform MRML node
  vtkMRMLTransformNode* GetParentTransformNode();

  ///
  /// alternative method to propagate events generated in Transform nodes
  void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                  unsigned long /*event*/,
                                  void * /*callData*/ ) override;

  /// TransformModifiedEvent is send when the parent transform is modidied
  enum
    {
      TransformModifiedEvent = 15000
    };

  /// Returns true if the transformable node can apply non-linear transforms.
  /// A transformable node is always expected to apply linear transforms.
  /// \sa ApplyTransformMatrix, ApplyTransform
  virtual bool CanApplyNonLinearTransforms()const;

  /// Convenience function to allow transforming a node by specifying a
  /// transformation matrix.
  /// \sa ApplyTransformMatrix, ApplyTransform
  virtual void ApplyTransformMatrix(vtkMatrix4x4* transformMatrix);

  /// Transforms the node with the provided non-linear transform.
  /// \sa SetAndObserveTransformNodeID, ApplyTransformMatrix, CanApplyNonLinearTransforms
  virtual void ApplyTransform(vtkAbstractTransform* transform);

  /// Utility function to convert a point position in the node's coordinate system to world coordinate system.
  /// \sa TransformPointFromWorld, SetAndObserveTransformNodeID
  virtual void TransformPointToWorld(const double inLocal[3], double outWorld[3]);

  /// Utility function to convert a point position in the node's coordinate system to world coordinate system.
  /// \sa TransformPointToWorld, SetAndObserveTransformNodeID
  virtual void TransformPointToWorld(const vtkVector3d &inLocal, vtkVector3d &outWorld);

  /// Utility function to convert a point position in world coordinate system to markup node's coordinate system
  /// \sa TransformPointToWorld, SetAndObserveTransformNodeID
  virtual void TransformPointFromWorld(const double inWorld[3], double outLocal[3]);

  /// Utility function to convert a point position in world coordinate system to markup node's coordinate system
  /// \sa TransformPointToWorld, SetAndObserveTransformNodeID
  virtual void TransformPointFromWorld(const vtkVector3d &inWorld, vtkVector3d &outLocal);

  /// Get referenced transform node id
  const char *GetTransformNodeID();

  /// Apply the associated transform to the transformable node. Return true
  /// on success, false otherwise.
  bool HardenTransform();

protected:
  vtkMRMLTransformableNode();
  ~vtkMRMLTransformableNode() override;
  vtkMRMLTransformableNode(const vtkMRMLTransformableNode&);
  void operator=(const vtkMRMLTransformableNode&);

  static const char* TransformNodeReferenceRole;
  static const char* TransformNodeReferenceMRMLAttributeName;

  virtual const char* GetTransformNodeReferenceRole();
  virtual const char* GetTransformNodeReferenceMRMLAttributeName();

  ///
  /// Called when a node reference ID is added (list size increased).
  void OnNodeReferenceAdded(vtkMRMLNodeReference *reference) override;

  ///
  /// Called when a node reference ID is modified.
  void OnNodeReferenceModified(vtkMRMLNodeReference *reference) override;

  ///
  /// Called after a node reference ID is removed (list size decreased).
  void OnNodeReferenceRemoved(vtkMRMLNodeReference *reference) override;

  /// Called when transform node reference added/modified/removed
  virtual void OnTransformNodeReferenceChanged(vtkMRMLTransformNode* transformNode);

private:
  char* TransformNodeIDInternal;
  vtkSetStringMacro(TransformNodeIDInternal);
  vtkGetStringMacro(TransformNodeIDInternal);

};

#endif
