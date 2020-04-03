/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLBSplineTransformNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/

#ifndef __vtkMRMLBSplineTransformNode_h
#define __vtkMRMLBSplineTransformNode_h

#include "vtkMRMLTransformNode.h"

class vtkMRMLStorageNode;

/// \brief MRML node for representing a nonlinear transformation to the parent node
/// using a bspline transform.
///
/// MRML node for representing a nonlinear transformation to the parent
/// node in the form of a vtkBSplineDeformableTransform.
class VTK_MRML_EXPORT vtkMRMLBSplineTransformNode : public vtkMRMLTransformNode
{
  public:
  static vtkMRMLBSplineTransformNode *New();
  vtkTypeMacro(vtkMRMLBSplineTransformNode,vtkMRMLTransformNode);
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
  vtkMRMLCopyContentDefaultMacro(vtkMRMLBSplineTransformNode);

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "BSplineTransform";};

  ///
  /// Create default storage node or nullptr if does not have one
  vtkMRMLStorageNode* CreateDefaultStorageNode() override
    {
    return Superclass::CreateDefaultStorageNode();
    };

protected:
  vtkMRMLBSplineTransformNode();
  ~vtkMRMLBSplineTransformNode() override;
  vtkMRMLBSplineTransformNode(const vtkMRMLBSplineTransformNode&);
  void operator=(const vtkMRMLBSplineTransformNode&);

};

#endif

