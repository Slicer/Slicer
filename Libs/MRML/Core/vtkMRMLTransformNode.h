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

#include "vtkMRMLStorableNode.h"

class vtkCollection;
class vtkAbstractTransform;
class vtkGeneralTransform;
class vtkMatrix4x4;

/// \brief MRML node for representing a transformation
/// between this node space and a parent node space.
///
/// General Transformation between this node space and a parent node space.
/// A vtkMRMLTransformableNode::TransformModifiedEvent is called if the transforms
/// are changed. ModifiedEvent is called if either transforms or other properties
/// of the object are changed.
class VTK_MRML_EXPORT vtkMRMLTransformNode : public vtkMRMLStorableNode
{
public:
  static vtkMRMLTransformNode *New();
  vtkTypeMacro(vtkMRMLTransformNode,vtkMRMLStorableNode);
  void PrintSelf(ostream& os, vtkIndent indent);

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
  virtual const char* GetNodeTagName() {return "Transform";};

  /// 
  /// Finds the storage node and read the data
  virtual void UpdateScene(vtkMRMLScene *scene)
    {
     Superclass::UpdateScene(scene);
    };

  /// 
  /// 1 if transfrom is linear, 0 otherwise
  virtual int IsLinear() { return 0; }

  ///
  /// Transform of this node to parent
  virtual vtkAbstractTransform* GetTransformToParent();

  ///
  /// Transform of this node from parent
  virtual vtkAbstractTransform* GetTransformFromParent();

  /// 
  /// 1 if all the transforms to the top are linear, 0 otherwise
  int  IsTransformToWorldLinear() ;

  /// 
  /// 1 if all the transforms between nodes are linear, 0 otherwise
  int  IsTransformToNodeLinear(vtkMRMLTransformNode* node);

  /// 
  /// Get concatenated transforms to the top
  void GetTransformToWorld(vtkGeneralTransform* transformToWorld);

  /// 
  /// Get concatenated transforms from the top
  void GetTransformFromWorld(vtkGeneralTransform* transformToWorld);

  ///
  /// Get concatenated transforms between nodes
  void GetTransformToNode(vtkMRMLTransformNode* node,
                          vtkGeneralTransform* transformToNode);

  /// 
  /// Get concatenated transforms to the top.
  /// This method and probably needs to be moved down a level in the
  /// hierarchy because this node cannot satisfy the call.
  /// Must be overridden in linear transform node classses.
  /// Returns 0 if the transform is not linear (cannot be described by a matrix).
  virtual int GetMatrixTransformToWorld(vtkMatrix4x4* transformToWorld);

  /// 
  /// Get concatenated transforms between nodes
  /// This method and probably needs to be moved down a level in the
  /// hierarchy because this node cannot satisfy the call.
  /// Must be overridden in linear transform node classses.
  /// Returns 0 if the transform is not linear (cannot be described by a matrix).
  virtual int GetMatrixTransformToNode(vtkMRMLTransformNode* node, 
                                       vtkMatrix4x4* transformToNode);
  /// 
  /// Returns 1 if this node is one of the node's descendents
  int IsTransformNodeMyParent(vtkMRMLTransformNode* node);

  /// 
  /// Returns 1 if the node is one of the this node's descendents
  int IsTransformNodeMyChild(vtkMRMLTransformNode* node);

  /// Reimplemented from vtkMRMLTransformableNode
  virtual bool CanApplyNonLinearTransforms()const;
  /// Reimplemented from vtkMRMLTransformableNode
  virtual void ApplyTransform(vtkAbstractTransform* transform);

  /// 
  /// Create default storage node or NULL if does not have one
  virtual vtkMRMLStorageNode* CreateDefaultStorageNode();

  /// Get/Set for ReadWriteAsTransformToParent
  /// Typically only a non-inverse transform can be written to file,
  /// so this flag should be set to read/write the forward transform.
  /// The flag is set automatically when a transform is set by SetAndObserveTransformToParent
  /// or SetAndObserveTransformFromParent and also inverted if the transform is inverted,
  /// so most of the cases it should be not necessary to manually change this flag.
  /// In the future it could be useful to be able to read/write both the toParent
  /// and fromParent transforms (if they are both specified).
  /// Note: When a transform is opened using AddData, the stored transform is assumed
  /// to be FromParent, as this is the ITK convention for transform storage in files.
  vtkGetMacro(ReadWriteAsTransformToParent, int);
  vtkSetMacro(ReadWriteAsTransformToParent, int);
  vtkBooleanMacro(ReadWriteAsTransformToParent, int);

  ///
  /// Start modifying the transform in the node.
  /// Disable vtkMRMLTransformableNode::TransformModifiedEvent events.
  /// Returns the previous state of DisableTransformModifiedEvent flag
  /// that should be passed to EndTransformModify() method
  virtual int StartTransformModify()
    {
    int disabledTransformModify = this->GetDisableTransformModifiedEvent();
    this->DisableTransformModifiedEventOn();
    return disabledTransformModify;
    };

  ///
  /// End modifying the transform in the node.
  /// Enable vtkMRMLTransformableNode::TransformModifiedEvent events if the
  /// previous state of DisableTransformModifiedEvent flag is 0.
  /// Return the number of pending events (even if
  /// InvokePendingTransformModifiedEvent is not called).
  virtual int EndTransformModify(int previousDisableTransformModifiedEventState)
    {
    this->SetDisableTransformModifiedEvent(previousDisableTransformModifiedEventState);
    if (!previousDisableTransformModifiedEventState)
      {
      return this->InvokePendingTransformModifiedEvent();
      }
    return this->TransformModifiedEventPending;
    };

  ///
  /// Turn on/off generating InvokeEvent for transformation changes
  vtkGetMacro(DisableTransformModifiedEvent, int);
  void SetDisableTransformModifiedEvent(int onOff)
    {
    this->DisableTransformModifiedEvent = onOff;
    }
  void DisableTransformModifiedEventOn()
    {
    this->SetDisableTransformModifiedEvent(1);
    }
  void DisableTransformModifiedEventOff()
    {
    this->SetDisableTransformModifiedEvent(0);
    }

  /// Count of pending modified events
  vtkGetMacro(TransformModifiedEventPending, int);

  ///
  /// Indicates that the transform inside the object is modified.
  /// Typical usage would be to disable transform modified events, call a series of operations that change transforms
  /// and then re-enable transform modified events to invoke any pending notifications.
  virtual void TransformModified()
    {
    if (!this->GetDisableTransformModifiedEvent())
      {
      this->InvokeEvent(vtkMRMLTransformableNode::TransformModifiedEvent, NULL);
      }
    else
      {
      ++this->TransformModifiedEventPending;
      }
    }

  ///
  /// Invokes any transform modified events that are 'pending', meaning they were generated
  /// while the DisableTransformModifiedEvent flag was nonzero.
  /// Returns the old flag state.
  virtual int InvokePendingTransformModifiedEvent ()
    {
    if ( this->TransformModifiedEventPending )
      {
      int oldModifiedEventPending = this->TransformModifiedEventPending;
      this->TransformModifiedEventPending = 0;
      this->InvokeEvent(vtkMRMLTransformableNode::TransformModifiedEvent, NULL);
      return oldModifiedEventPending;
      }
    return this->TransformModifiedEventPending;
    }

  virtual bool GetModifiedSinceRead();

  ///
  /// Retrieves the transform as the specified transform class.
  /// Returns NULL if the transform is not a kind of transform that was requested.
  /// Example usage: vtkWarpTransform* warpTransform=vtkWarpTransform::SafeDownCast(GetTransformToParentAs("vtkWarpTransform"));
  vtkAbstractTransform* GetTransformToParentAs(const char* transformType, bool logErrorIfFails=true);

  ///
  /// Retrieves the transform as the specified transform class.
  /// Returns NULL if the transform is not a kind of transform that was requested.
  /// Example usage: vtkWarpTransform* warpTransform=vtkWarpTransform::SafeDownCast(GetTransformFromParentAs("vtkWarpTransform"));
  vtkAbstractTransform* GetTransformFromParentAs(const char* transformType, bool logErrorIfFails=true);

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
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ );

  ///
  /// Creates a shallow copy of an input composite transform (that can contain a complex hierarchy of transforms)
  /// into a flat list of transforms. This is useful for simplifying serialization for copying and writing to file.
  static void FlattenGeneralTransform(vtkCollection* outputTransformList, vtkAbstractTransform* inputTransform);

  ///
  /// Some transforms have DeepCopy method that actually only creates a shallow copy
  /// (such as vtkGeneralTransform and vtkGridTransform). This method creates a true deep copy of a transform.
  /// Returns nonzero on success.
  static int DeepCopyTransform(vtkAbstractTransform* dst, vtkAbstractTransform* src);

  ///
  /// Invert the transform.
  /// Internally it does not perform any actual computation just switches ToParent and FromParent.
  void Inverse();

  ///
  /// Sets the ReadWriteAsTransformToParent flag automatically
  /// The flag is set so that the ReadWriteAsTransformToParent points to the forward transform
  void SetReadWriteAsTransformToParentAuto();

protected:
  vtkMRMLTransformNode();
  ~vtkMRMLTransformNode();
  vtkMRMLTransformNode(const vtkMRMLTransformNode&);
  void operator=(const vtkMRMLTransformNode&);

  ///
  /// Retrieves a simple transform from a generic transform
  /// If the generic transform is composed of multiple transform or contains a different
  /// transform type then it returns NULL.
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

  int ReadWriteAsTransformToParent;

  int DisableTransformModifiedEvent;
  int TransformModifiedEventPending;
};

#endif
