/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLNonlinearTransformNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/

#ifndef __vtkMRMLNonlinearTransformNode_h
#define __vtkMRMLNonlinearTransformNode_h

#include "vtkMRMLTransformNode.h"

class vtkWarpTransform;
class vtkMRMLStorageNode;

/// \brief MRML node for representing a nonlinear transformation to the parent node.
///
/// MRML node for representing a nonlinear transformation to the parent
/// node in the form of a vtkWarpTransform.
class VTK_MRML_EXPORT vtkMRMLNonlinearTransformNode : public vtkMRMLTransformNode
{
  public:
  static vtkMRMLNonlinearTransformNode *New();
  vtkTypeMacro(vtkMRMLNonlinearTransformNode,vtkMRMLTransformNode);
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
  virtual const char* GetNodeTagName() {return "NonlinearTransform";};

  /// 
  /// 1 if transfrom is linear, 0 otherwise
  virtual int IsLinear() {return 0;};

  /// 
  /// vtkGeneral transform of this node to paren node
  virtual vtkGeneralTransform* GetTransformToParent();

  /// 
  /// vtkGeneral transform of this node from paren node
  virtual vtkGeneralTransform* GetTransformFromParent();

  ///
  /// vtkWarpTransform of this node to paren node
  virtual vtkWarpTransform* GetWarpTransformToParent()
    {
    return 0;
    }

  ///
  /// vtkWarpTransform of this node from paren node
  virtual vtkWarpTransform* GetWarpTransformFromParent()
    {
    return 0;
    }

  ///
  /// Set and observe a new  transform of this node to parent node.
  /// Each time the matrix is modified,
  /// vtkMRMLTransformableNode::TransformModifiedEvent is fired.
  /// ModifiedEvent() and TransformModifiedEvent() are fired after the transform
  /// is set.
  void SetAndObserveWarpTransformToParent(vtkWarpTransform *warp, bool updateTrasformFromParent);

  ///
  /// Set and observe a new  transform of this node from parent node.
  /// Each time the matrix is modified,
  /// vtkMRMLTransformableNode::TransformModifiedEvent is fired.
  /// ModifiedEvent() and TransformModifiedEvent() are fired after the transform
  /// is set.
  void SetAndObserveWarpTransformFromParent(vtkWarpTransform *warp, bool updateTrasformToParent);


  /// 
  /// Get concatinated transforms to the top. This method is from
  /// the superclass and probably needs to be moved down a level in the
  /// hierarchy because this node cannot satisfy the call.
  virtual int  GetMatrixTransformToWorld(vtkMatrix4x4* transformToWorld);
  
  /// 
  /// Get concatinated transforms between nodes. This method is from
  /// the superclass and probably needs to be moved down a level in the
  /// hierarchy because this node cannot satisfy the call.
  virtual int  GetMatrixTransformToNode(vtkMRMLTransformNode* node, 
                                        vtkMatrix4x4* transformToNode);

  /// 
  /// alternative method to propagate events generated in Transform nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );
  /// 
  /// Create default storage node or NULL if does not have one
  virtual vtkMRMLStorageNode* CreateDefaultStorageNode()
    {
    return Superclass::CreateDefaultStorageNode();
    };

  ///
  /// Deep copy input transform to this node transform to parent
  virtual void DeepCopyTransformToParent(vtkWarpTransform * vtkNotUsed(warp)) {}

  ///
  /// Deep copy input transform to this node transform from parent
  virtual void DeepCopyTransformFromParent(vtkWarpTransform * vtkNotUsed(warp)) {}

protected:
  vtkMRMLNonlinearTransformNode();
  ~vtkMRMLNonlinearTransformNode();
  vtkMRMLNonlinearTransformNode(const vtkMRMLNonlinearTransformNode&);
  void operator=(const vtkMRMLNonlinearTransformNode&);

  virtual void SetWarpTransformToParent(vtkWarpTransform *);
  virtual void SetWarpTransformFromParent(vtkWarpTransform *);

  vtkWarpTransform* WarpTransformToParent;
  vtkWarpTransform* WarpTransformFromParent;

  int InSetAndObserveWarpTransformToParent;
  int InSetAndObserveWarpTransformFromParent;

};

#endif
