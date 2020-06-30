/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLTransformNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/

#ifndef __vtkMRMLTransformNode_h
#define __vtkMRMLTransformNode_h

#include "vtkMRMLDisplayableNode.h"

class vtkCollection;
class vtkAbstractTransform;
class vtkGeneralTransform;
class vtkMatrix4x4;
class vtkTransform;

/// \brief MRML node for representing a transformation
/// between this node space and a parent node space.
///
/// General Transformation between this node space and a parent node space.
/// A vtkMRMLTransformableNode::TransformModifiedEvent is called if the transforms
/// are changed. ModifiedEvent is called if either transforms or other properties
/// of the object are changed.
class VTK_MRML_EXPORT vtkMRMLTransformNode : public vtkMRMLDisplayableNode
{
public:
  static vtkMRMLTransformNode *New();
  vtkTypeMacro(vtkMRMLTransformNode,vtkMRMLDisplayableNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLTransformNode);

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "Transform";};

  ///
  /// Finds the storage node and read the data
  void UpdateScene(vtkMRMLScene *scene) override
    {
     Superclass::UpdateScene(scene);
    };

  ///
  /// Returns 1 if transform is a non-composite linear tansform, 0 otherwise (if composite transform or non-linear transform)
  virtual int IsLinear();

  ///
  /// Returns 1 if the transform is a composite transform (a transform that may contain multiple simple transforms)
  virtual int IsComposite();

  ///
  /// Transform of this node to parent
  virtual vtkAbstractTransform* GetTransformToParent();

  ///
  /// Get the vtkMatrix4x4 transform of this node to parent node
  /// Returns 0 if the transform is not linear or there is an error.
  virtual int GetMatrixTransformToParent(vtkMatrix4x4* matrix);

  ///
  /// Get the vtkMatrix4x4 transform of this node from parent node
  /// Returns 0 if the transform is not linear or there is an error.
  virtual int GetMatrixTransformFromParent(vtkMatrix4x4* matrix);

  ///
  /// Set a new matrix transform of this node to parent node.
  /// \deprecated Use GetMatrixTransformToParent(vtkMatrix4x4*) instead.
  /// The method returns a cached copy of the transform, so modification
  /// of the matrix does not alter the transform node.
  virtual vtkMatrix4x4* GetMatrixTransformToParent();

  ///
  /// Set a new matrix transform of this node from parent node.
  /// \deprecated Use GetMatrixTransformFromParent(vtkMatrix4x4*) instead.
  /// The method returns a cached copy of the transform, so modification
  /// of the matrix does not alter the transform node.
  virtual vtkMatrix4x4* GetMatrixTransformFromParent();

  ///
  /// Get a human-readable description of the transform
  virtual const char* GetTransformToParentInfo();

  ///
  /// Transform of this node from parent
  virtual vtkAbstractTransform* GetTransformFromParent();

  ///
  /// Get a human-readable description of the transform
  virtual const char* GetTransformFromParentInfo();

  ///
  /// 1 if all the transforms to the top are linear, 0 otherwise
  int  IsTransformToWorldLinear();

  ///
  /// 1 if all the transforms between nodes are linear, 0 otherwise
  int  IsTransformToNodeLinear(vtkMRMLTransformNode* node);

  ///
  /// Get concatenated transforms to world.
  /// \sa GetTransformBetweenNodes
  void GetTransformToWorld(vtkGeneralTransform* transformToWorld);

  ///
  /// Get concatenated transforms from world.
  /// \sa GetTransformBetweenNodes
  void GetTransformFromWorld(vtkGeneralTransform* transformToWorld);

  ///
  /// Get concatenated transforms to the specified node.
  /// \sa GetTransformBetweenNodes
  void GetTransformToNode(vtkMRMLTransformNode* node,
                          vtkGeneralTransform* transformToNode);

  ///
  /// Get concatenated transforms from the specified node.
  /// \sa GetTransformBetweenNodes
  void GetTransformFromNode(vtkMRMLTransformNode* node,
                          vtkGeneralTransform* transformFromNode);

  ///
  /// Get concatenated transforms from source to target node
  /// Source and target nodes are allowed to be nullptr, which means that transform is the world transform.
  static void GetTransformBetweenNodes(vtkMRMLTransformNode* sourceNode,
    vtkMRMLTransformNode* targetNode, vtkGeneralTransform* transformSourceToTarget);

  ///
  /// Get concatenated transforms to world.
  /// Returns 0 if the transform is not linear (cannot be described by a matrix).
  /// \sa GetMatrixTransformBetweenNodes
  virtual int GetMatrixTransformToWorld(vtkMatrix4x4* transformToWorld);

  ///
  /// Get concatenated transforms from world.
  /// Returns 0 if the transform is not linear (cannot be described by a matrix).
  /// \sa GetMatrixTransformBetweenNodes
  virtual int GetMatrixTransformFromWorld(vtkMatrix4x4* transformFromWorld);

  ///
  /// Get concatenated transforms to the specified node.
  /// Returns 0 if the transform is not linear (cannot be described by a matrix).
  /// \sa GetMatrixTransformBetweenNodes
  virtual int GetMatrixTransformToNode(vtkMRMLTransformNode* node,
                                       vtkMatrix4x4* transformToNode);

  ///
  /// Get concatenated transforms from the specified node.
  /// Returns 0 if the transform is not linear (cannot be described by a matrix).
  /// \sa GetMatrixTransformBetweenNodes
  virtual int GetMatrixTransformFromNode(vtkMRMLTransformNode* node,
                                       vtkMatrix4x4* transformFromNode);

  ///
  /// Get concatenated transforms from source to target node
  /// Source and target nodes are allowed to be nullptr, which means that transform is the world transform.
  /// Returns 0 if the transform is not linear (cannot be described by a matrix).
  static int GetMatrixTransformBetweenNodes(vtkMRMLTransformNode* sourceNode,
    vtkMRMLTransformNode* targetNode, vtkMatrix4x4* transformSourceToTarget);

  ///
  /// Returns 1 if this node is one of the node's descendents
  /// nullptr designates the world transform node and so always returns with 1.
  int IsTransformNodeMyParent(vtkMRMLTransformNode* node);

  ///
  /// Returns 1 if the node is one of the this node's descendents
  int IsTransformNodeMyChild(vtkMRMLTransformNode* node);

  ///
  /// Get the first common parent of he current and the target transform node
  /// If there are no common parents then nullptr is returned
  /// \param targetNode The transform for which the current transform will be determined
  vtkMRMLTransformNode* GetFirstCommonParent(vtkMRMLTransformNode* targetNode);

  ///
  /// Set a new matrix transform of this node to parent node.
  /// Invokes a TransformModified event (does not invoke Modified).
  /// The method only allows setting or updating an empty or simple
  /// linear transform.
  /// Returns nonzero if the transform is successfully updated,
  /// returns 0 if the transform is non-linear or composite and therefore
  /// cannot be updated.
  /// To overwrite a non-linear or composite transform first remove
  /// the old transform by calling SetAndObserveTransformToParent(nullptr)
  /// and then set the matrix.
  /// \sa SetMatrixTransformFromParent
  virtual int SetMatrixTransformToParent(vtkMatrix4x4 *matrix);

  ///
  /// Set a new matrix transform of this node from parent node.
  /// \sa SetMatrixTransformToParent
  virtual int SetMatrixTransformFromParent(vtkMatrix4x4 *matrix);

  ///
  /// Applies a transformation matrix to the current matrix.
  /// Difference compared to ApplyTransform(): if the current
  /// transform is a linear then instead of creating a composite
  /// transform, the current and applicable transform matrices
  /// are multiplied and stored in a simple (non-composite)
  /// linear transform.
  /// If the current transformation was non-linear then the method
  /// is equivalent to ApplyTransform (creates a composite transform).
  void ApplyTransformMatrix(vtkMatrix4x4* transformMatrix) override;

  ///
  /// Set a new matrix transform of this node to parent node.
  /// Returns 0 if the current transform is not linear.
  /// \deprecated Use SetMatrixTransformToParent instead.
  virtual int SetAndObserveMatrixTransformToParent(vtkMatrix4x4 *matrix);

  ///
  /// Set a new matrix transform of this node from parent node.
  /// Returns 0 if the current transform is not linear.
  /// \deprecated Use SetMatrixTransformToParent instead.
  virtual int SetAndObserveMatrixTransformFromParent(vtkMatrix4x4 *matrix);

  /// Reimplemented from vtkMRMLTransformableNode
  bool CanApplyNonLinearTransforms()const override;
  /// Reimplemented from vtkMRMLTransformableNode
  void ApplyTransform(vtkAbstractTransform* transform) override;

  /// Split a composite transform to its components. The components are inserted to the scene between this transform and its parent.
  /// A composite transform can be created by hardening different types of transforms on each other.
  /// Return non-zero on success.
  virtual int Split();

  ///
  /// Create default storage node or nullptr if does not have one
  vtkMRMLStorageNode* CreateDefaultStorageNode() override;

  /// Creates the most appropriate storage node class for storing a sequence of these nodes.
  vtkMRMLStorageNode* CreateDefaultSequenceStorageNode() override;

  ///
  /// Create and observe default display node
  void CreateDefaultDisplayNodes() override;

  /// Creates the most appropriate display node class for storing a sequence of these nodes.
  void CreateDefaultSequenceDisplayNodes() override;

  /// Get/Set for ReadAsTransformToParent
  /// Indicates that the transform in the storage node has to be interpreted as
  /// a transformToParent (as opposed to the default ITK-style transformFromParent).
  /// Since writing of ITK inverse transform is implemented, this option is not needed anymore
  /// and kept only for backward compatibility: when a transform is read from an old scene file
  /// that has ReadWriteAsTransformToParent="1" then the transform is interpreted as such.
  /// Transforms are now always written as transformFromParent.
  vtkGetMacro(ReadAsTransformToParent, int);
  vtkSetMacro(ReadAsTransformToParent, int);
  vtkBooleanMacro(ReadAsTransformToParent, int);

  ///
  /// Indicates that the transform inside the object is modified.
  /// Typical usage would be to disable transform modified events, call a series of operations that change transforms
  /// and then re-enable transform modified events to invoke any pending notifications.
  virtual void TransformModified()
    {
    this->InvokeCustomModifiedEvent(vtkMRMLTransformableNode::TransformModifiedEvent);
    }

  bool GetModifiedSinceRead() override;

  ///
  /// Retrieves the transform as the specified transform class.
  /// If modifiableOnly is set to true then nullptr will be returned for transforms that cannot be modified (e.g., because it is computed from its inverse).
  /// Returns nullptr if the transform is not a kind of transform that was requested.
  /// Example usage: vtkOrientedBSplineTransform* bsplineTransform=vtkOrientedBSplineTransform::SafeDownCast(GetTransformToParentAs("vtkOrientedBSplineTransform"));
  vtkAbstractTransform* GetTransformToParentAs(const char* transformType, bool logErrorIfFails = true, bool modifiableOnly = false);

  ///
  /// Retrieves the transform as the specified transform class.
  /// If modifiableOnly is set to true then nullptr will be returned for transforms that cannot be modified (e.g., because it is computed from its inverse).
  /// Returns nullptr if the transform is not a kind of transform that was requested.
  /// Example usage: vtkOrientedBSplineTransform* bsplineTransform=vtkOrientedBSplineTransform::SafeDownCast(GetTransformFromParentAs("vtkOrientedBSplineTransform"));
  vtkAbstractTransform* GetTransformFromParentAs(const char* transformType, bool logErrorIfFails = true, bool modifiableOnly = false);

  /// Set and observe a new transform of this node to parent node.
  /// Each time the transform is modified,
  /// vtkMRMLTransformableNode::TransformModifiedEvent is fired.
  /// ModifiedEvent() and TransformModifiedEvent() are fired after the transform
  /// is set.
  void SetAndObserveTransformToParent(vtkAbstractTransform *transform);

  /// Set and observe a new transform of this node from parent node.
  /// Each time the transform is modified,
  /// vtkMRMLTransformableNode::TransformModifiedEvent is fired.
  /// ModifiedEvent() and TransformModifiedEvent() are fired after the transform
  /// is set.
  void SetAndObserveTransformFromParent(vtkAbstractTransform *transform);

  /// alternative method to propagate events generated in Transform nodes
  void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ ) override;

  ///
  /// Creates a shallow copy of an input composite transform (that can contain a complex hierarchy of transforms)
  /// into a flat list of transforms. This is useful for simplifying serialization for copying and writing to file.
  static void FlattenGeneralTransform(vtkCollection* outputTransformList, vtkAbstractTransform* inputTransform);

  ///
  /// Return true if the two transforms are equal. A transform object is considered to be the same if it is
  /// made up of the same flattened list of transforms.
  static bool AreTransformsEqual(vtkAbstractTransform* transform1, vtkAbstractTransform* transform2);

  ///
  /// Utility function that determines if a transform is linear. It looks into composite transforms and only returns
  /// with true if all the transform components are linear.
  /// If concatenatedLinearTransform is specified and the transform is linear then it returns the concatenated linear
  /// transformation matrix.
  static bool IsGeneralTransformLinear(vtkAbstractTransform* inputTransform, vtkTransform* concatenatedLinearTransform=nullptr);

  ///
  /// Utility function that determines if a transform is computed from its inverse.
  /// It may be important to know if a transform is computed from its inverse because then
  /// it cannot not be modified (if modification is needed then he source transform has to be updated).
  static bool IsAbstractTransformComputedFromInverse(vtkAbstractTransform* abstractTransform);

  ///
  /// Some transforms have DeepCopy method that actually only creates a shallow copy
  /// (such as vtkGeneralTransform and vtkGridTransform). This method creates a true deep copy of a transform.
  /// Returns nonzero on success.
  static int DeepCopyTransform(vtkAbstractTransform* dst, vtkAbstractTransform* src);

  ///
  /// Invert the transform.
  /// Internally it does not perform any actual computation just switches ToParent and FromParent.
  void Inverse();

  /// Update the node's name to reflect that the node content is inverted.
  /// Inversion is implemented by adding/removing " (-)" suffix.
  virtual void InverseName();

  /// Get the latest modification time of the stored transform
  vtkMTimeType GetTransformToWorldMTime();

  /// Get a human-readable description of the transformation
  /// The returned string is stored in a shared buffer therefore the text has to be copied. This is a
  /// static-style function (the contents of the owner transform node is not used), but the returned
  /// string buffer needs to be owned by an object.
  /// \param inputTransform The transform for which information is obtained
  const char* GetTransformInfo(vtkAbstractTransform* inputTransform);

  /// Reference role name from the transform node to the moving volume or fiducial node that participated in registration.
  static const char* GetMovingNodeReferenceRole() { return "spatialRegistrationMoving"; };
  /// Reference role name from the transform node to the moving volume or fiducial node that participated in registration
  static const char* GetFixedNodeReferenceRole() { return "spatialRegistrationFixed"; };

protected:
  vtkMRMLTransformNode();
  ~vtkMRMLTransformNode() override;
  vtkMRMLTransformNode(const vtkMRMLTransformNode&);
  void operator=(const vtkMRMLTransformNode&);

  ///
  /// Retrieves a simple transform from a generic transform
  /// If the generic transform is composed of multiple transform or contains a different
  /// transform type then it returns nullptr.
  virtual vtkAbstractTransform* GetAbstractTransformAs(vtkAbstractTransform* inputTransform, const char* transformClassName, bool logErrorIfFails);

  ///
  /// Sets and observes a transform and deletes the inverse (so that the inverse will be computed automatically)
  virtual void SetAndObserveTransform(vtkAbstractTransform** originalTransformPtr, vtkAbstractTransform** inverseTransformPtr, vtkAbstractTransform *transform);

  ///
  /// These transforms store the transforms that were set externally.
  /// We use the capability of generic transforms for concatenating and inverting the same
  /// abstract transform in multiple generic transforms, therefore they are automatically updated.
  /// We keep the two separate member variables because in the future we may allow setting of a
  /// custom inverse transform (that is NOT computed automatically from each other, for example
  /// some Demons registration can provide both the forward and inverse transforms as output).
  vtkAbstractTransform* TransformToParent;
  vtkAbstractTransform* TransformFromParent;

  int ReadAsTransformToParent;

  // Temporary buffers used for returning transform info as char*
  std::string TransformInfo;

  /// These variables are only for supporting the deprecated
  /// GetMatrixTransformToParent and GetMatrixFromParent methods
  vtkMatrix4x4* CachedMatrixTransformToParent;
  vtkMatrix4x4* CachedMatrixTransformFromParent;
};

#endif
