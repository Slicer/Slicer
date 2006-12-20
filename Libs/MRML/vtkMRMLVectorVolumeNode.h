/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLVectorVolumeNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/
// .NAME vtkMRMLVectorVolumeNode - MRML node for representing a vector volume (image stack).
// .SECTION Description
// Volume with vector pixel type.

#ifndef __vtkMRMLVectorVolumeNode_h
#define __vtkMRMLVectorVolumeNode_h


#include "vtkMRMLTensorVolumeNode.h"

class vtkImageData;

class VTK_MRML_EXPORT vtkMRMLVectorVolumeNode : public vtkMRMLTensorVolumeNode
{
  public:
  static vtkMRMLVectorVolumeNode *New();
  vtkTypeMacro(vtkMRMLVectorVolumeNode,vtkMRMLTensorVolumeNode);
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
  virtual const char* GetNodeTagName() {return "VectorVolume";};

protected:
  vtkMRMLVectorVolumeNode();
  ~vtkMRMLVectorVolumeNode();
  vtkMRMLVectorVolumeNode(const vtkMRMLVectorVolumeNode&);
  void operator=(const vtkMRMLVectorVolumeNode&);

};

#endif


 

