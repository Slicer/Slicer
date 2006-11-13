/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLLinearTransformNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/
// .NAME vtkMRMLLinearTransformNode - MRML node for representing 
// a linear transformation to the parent node
// .SECTION Description
// MRML node for representing 
// a linear transformation to the parent node in the form vtkMatrix4x4
// MatrixTransformToParent

#ifndef __vtkMRMLLinearTransformNode_h
#define __vtkMRMLLinearTransformNode_h

#include "vtkMRMLTransformNode.h"

class VTK_MRML_EXPORT vtkMRMLLinearTransformNode : public vtkMRMLTransformNode
{
  public:
  static vtkMRMLLinearTransformNode *New();
  vtkTypeMacro(vtkMRMLLinearTransformNode,vtkMRMLTransformNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "LinearTransform";};

  // Description:
  // 1 if transfrom is linear, 0 otherwise
  virtual int IsLinear() {return 1;};

  // Description:
  // vtkGeneral transform of this node to paren node
  virtual vtkGeneralTransform* GetTransformToParent();

  // Description:
  // vtkMatrix4x4 transform of this node to paren node
  vtkGetObjectMacro(MatrixTransformToParent, vtkMatrix4x4); 
  void SetAndObserveMatrixTransformToParent(vtkMatrix4x4 *matrix);

  // Description:
  // Get concatinated transforms to the top
  virtual int  GetMatrixTransformToWorld(vtkMatrix4x4* transformToWorld);
  
  // Description:
  // Get concatinated transforms  bwetween nodes  
  virtual int  GetMatrixTransformToNode(vtkMRMLTransformNode* node, 
                                        vtkMatrix4x4* transformToNode);

  // Description:
  // alternative method to propagate events generated in Transform nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );
  
protected:
  vtkMRMLLinearTransformNode();
  ~vtkMRMLLinearTransformNode();
  vtkMRMLLinearTransformNode(const vtkMRMLLinearTransformNode&);
  void operator=(const vtkMRMLLinearTransformNode&);

  vtkSetObjectMacro(MatrixTransformToParent, vtkMatrix4x4); 
  vtkMatrix4x4* MatrixTransformToParent;

};

#endif

