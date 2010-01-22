/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLNonlinearTransformNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/
///  vtkMRMLNonlinearTransformNode - MRML node for representing a nonlinear transformation to the parent node
/// 
/// MRML node for representing a nonlinear transformation to the parent
/// node in the form of a vtkWarpTransform 

#ifndef __vtkMRMLNonlinearTransformNode_h
#define __vtkMRMLNonlinearTransformNode_h

#include "vtkMRMLTransformNode.h"

class vtkWarpTransform;
class vtkMRMLStorageNode;

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
  /// vtkWarpTransform of this node to paren node
  vtkGetObjectMacro(WarpTransformToParent, vtkWarpTransform); 
  void SetAndObserveWarpTransformToParent(vtkWarpTransform *warp);

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

  
protected:
  vtkMRMLNonlinearTransformNode();
  ~vtkMRMLNonlinearTransformNode();
  vtkMRMLNonlinearTransformNode(const vtkMRMLNonlinearTransformNode&);
  void operator=(const vtkMRMLNonlinearTransformNode&);

  virtual void SetWarpTransformToParent(vtkWarpTransform *);
  vtkWarpTransform* WarpTransformToParent;

};

#endif

