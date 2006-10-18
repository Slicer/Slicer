/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLQueryAtlasNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkMRMLQueryAtlasNode_h
#define __vtkMRMLQueryAtlasNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLStorageNode.h"

#include "vtkMatrix4x4.h"
#include "vtkTransform.h"
#include "vtkImageData.h"

#include "vtkQueryAtlas.h"

class vtkImageData;

class VTK_QUERYATLAS_EXPORT vtkMRMLQueryAtlasNode : public vtkMRMLNode
{
  public:
  static vtkMRMLQueryAtlasNode *New();
  vtkTypeMacro(vtkMRMLQueryAtlasNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "QueryAtlasParameters";};

protected:
  vtkMRMLQueryAtlasNode();
  ~vtkMRMLQueryAtlasNode();
  vtkMRMLQueryAtlasNode(const vtkMRMLQueryAtlasNode&);
  void operator=(const vtkMRMLQueryAtlasNode&);

};

#endif

