/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLGridTransformNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/

#ifndef __vtkMRMLGridTransformNode_h
#define __vtkMRMLGridTransformNode_h

#include "vtkMRMLTransformNode.h"

/// \brief MRML node for representing a nonlinear transformation to the parent node using a grid transform.
///
/// MRML node for representing a nonlinear transformation to the parent
/// node in the form of a vtkOrientedGridTransform.
class VTK_MRML_EXPORT vtkMRMLGridTransformNode : public vtkMRMLTransformNode
{
public:
  static vtkMRMLGridTransformNode *New();
  vtkTypeMacro(vtkMRMLGridTransformNode,vtkMRMLTransformNode);
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
  virtual const char* GetNodeTagName() {return "GridTransform";};

protected:
  vtkMRMLGridTransformNode();
  ~vtkMRMLGridTransformNode();
  vtkMRMLGridTransformNode(const vtkMRMLGridTransformNode&);
  void operator=(const vtkMRMLGridTransformNode&);
};

#endif
