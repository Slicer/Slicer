/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLTransformNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/
///  vtkMRMLTransformNode - MRML node for representing a transformation
/// between this node space and a parent node space
/// 
/// General Transformation between this node space and a parent node space

#ifndef __vtkMRMLTransformNode_h
#define __vtkMRMLTransformNode_h

#include "vtkMRMLStorableNode.h"

class vtkGeneralTransform;
class vtkMatrix4x4;

class VTK_MRML_EXPORT vtkMRMLTransformNode : public vtkMRMLStorableNode
{
public:
  vtkTypeMacro(vtkMRMLTransformNode,vtkMRMLStorableNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance() = 0;

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
  virtual const char* GetNodeTagName() = 0;

  /// 
  /// Finds the storage node and read the data
  virtual void UpdateScene(vtkMRMLScene *scene){
     Superclass::UpdateScene(scene);
  };

  /// 
  /// 1 if transfrom is linear, 0 otherwise
  virtual int IsLinear() = 0;

  /// 
  /// vtkGeneral transform of this node
  virtual vtkGeneralTransform* GetTransformToParent() {
     return this->TransformToParent; };

  /// 
  /// 1 if all the transforms to the top are linear, 0 otherwise
  int  IsTransformToWorldLinear() ;

  /// 
  /// 1 if all the transforms bwetween nodes  are linear, 0 otherwise
  int  IsTransformToNodeLinear(vtkMRMLTransformNode* node);

  /// 
  /// Get concatinated transforms to the top
  void GetTransformToWorld(vtkGeneralTransform* transformToWorld);

  /// 
  /// Get concatinated transforms  bwetween nodes  
  void GetTransformToNode(vtkMRMLTransformNode* node, 
                          vtkGeneralTransform* transformToNode);

  /// 
  /// Get concatinated transforms to the top
  virtual int GetMatrixTransformToWorld(vtkMatrix4x4* transformToWorld) = 0;

  /// 
  /// Get concatinated transforms  bwetween nodes  
  virtual int GetMatrixTransformToNode(vtkMRMLTransformNode* node, 
                                       vtkMatrix4x4* transformToNode) = 0;
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

protected:
  vtkMRMLTransformNode();
  ~vtkMRMLTransformNode();
  vtkMRMLTransformNode(const vtkMRMLTransformNode&);
  void operator=(const vtkMRMLTransformNode&);

  vtkGeneralTransform* TransformToParent;

};

#endif

