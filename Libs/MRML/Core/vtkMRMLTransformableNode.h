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

  /// Returns true if the transformable node can apply non linear transforms
  /// \sa ApplyTransformMatrix, ApplyTransform
  virtual bool CanApplyNonLinearTransforms()const;

  /// Concatenate a matrix to the current transform matrix.
  /// \sa SetAndObserveTransformNodeID, ApplyTransform,
  /// CanApplyNonLinearTransforms
  virtual void ApplyTransformMatrix(vtkMatrix4x4* transformMatrix);

  /// Concatenate a transform to the current transform matrix.
  /// \sa SetAndObserveTransformNodeID, ApplyMatrix,
  /// CanApplyNonLinearTransforms
  virtual void ApplyTransform(vtkAbstractTransform* transform);

  /// Apply the observed transform to the input point.
  /// \sa TransformPointFromWorld, SetAndObserveTransformNodeID
  virtual void TransformPointToWorld(const double in[4], double out[4]);

  /// Apply the invert of the observed transform to the input point.
  /// \sa TransformPointToWorld, SetAndObserveTransformNodeID
  virtual void TransformPointFromWorld(const double in[4], double out[4]);

  /// Get referenced transform node id
  const char *GetTransformNodeID();

  static const std::string TRANSFORM_NODE_REFERENCE_ROLE;
  static const std::string TRANSFORM_NODE_REFERENCE_MRML_ATTRIBUTE_NAME;

protected:
  vtkMRMLTransformableNode();
  ~vtkMRMLTransformableNode();
  vtkMRMLTransformableNode(const vtkMRMLTransformableNode&);
  void operator=(const vtkMRMLTransformableNode&);

  std::string TransformNodeID;

};

#endif
