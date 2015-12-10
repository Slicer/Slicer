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
#include "vtkMRMLNode.h"
class vtkMRMLTransformNode;

// VTK includes
class vtkAbstractTransform;
class vtkMatrix4x4;

/// \brief MRML node for representing a node with a tranform.
///
/// A superclass for other nodes that can have a transform to parent node
/// like volume, model and transformation nodes.
class VTK_MRML_EXPORT vtkMRMLTransformableNode : public vtkMRMLNode
{
public:
  vtkTypeMacro(vtkMRMLTransformableNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance() = 0;

  ///
  /// Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  ///
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  ///
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() = 0;

  ///
  /// Set a reference to transform node
  void SetAndObserveTransformNodeID(const char *transformNodeID);

  ///
  /// Associated transform MRML node
  vtkMRMLTransformNode* GetParentTransformNode();

  ///
  /// alternative method to propagate events generated in Transform nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                  unsigned long /*event*/,
                                  void * /*callData*/ );

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

  /// Apply the observed transform to the input point.
  /// \sa TransformPointFromWorld, SetAndObserveTransformNodeID
  virtual void TransformPointToWorld(const double in[4], double out[4]);

  /// Apply the invert of the observed transform to the input point.
  /// \sa TransformPointToWorld, SetAndObserveTransformNodeID
  virtual void TransformPointFromWorld(const double in[4], double out[4]);

  /// Get referenced transform node id
  const char *GetTransformNodeID();

protected:
  vtkMRMLTransformableNode();
  ~vtkMRMLTransformableNode();
  vtkMRMLTransformableNode(const vtkMRMLTransformableNode&);
  void operator=(const vtkMRMLTransformableNode&);

  static const char* TransformNodeReferenceRole;
  static const char* TransformNodeReferenceMRMLAttributeName;

  virtual const char* GetTransformNodeReferenceRole();
  virtual const char* GetTransformNodeReferenceMRMLAttributeName();

  ///
  /// Called when a node reference ID is added (list size increased).
  virtual void OnNodeReferenceAdded(vtkMRMLNodeReference *reference)
  {
    Superclass::OnNodeReferenceAdded(reference);
    if (std::string(reference->GetReferenceRole()) == this->TransformNodeReferenceRole)
      {
      this->InvokeCustomModifiedEvent(vtkMRMLTransformableNode::TransformModifiedEvent, reference->GetReferencedNode());
      }
  }

  ///
  /// Called when a node reference ID is modified.
  virtual void OnNodeReferenceModified(vtkMRMLNodeReference *reference)
  {
    Superclass::OnNodeReferenceModified(reference);
    if (std::string(reference->GetReferenceRole()) == this->TransformNodeReferenceRole)
    {
      this->InvokeCustomModifiedEvent(vtkMRMLTransformableNode::TransformModifiedEvent, reference->GetReferencedNode());
    }
  }

  ///
  /// Called after a node reference ID is removed (list size decreased).
  virtual void OnNodeReferenceRemoved(vtkMRMLNodeReference *reference)
  {
    Superclass::OnNodeReferenceRemoved(reference);
    if (std::string(reference->GetReferenceRole()) == this->TransformNodeReferenceRole)
    {
      this->InvokeCustomModifiedEvent(vtkMRMLTransformableNode::TransformModifiedEvent, reference->GetReferencedNode());
    }
  }


private:
  char* TransformNodeIDInternal;
  vtkSetStringMacro(TransformNodeIDInternal);
  vtkGetStringMacro(TransformNodeIDInternal);

};

#endif
