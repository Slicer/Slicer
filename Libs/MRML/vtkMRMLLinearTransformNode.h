/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLLinearTransformNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/
// .NAME vtkMRMLLinearTransformNode - MRML node for representing a volume (image stack).
// .SECTION Description
// Volume nodes describe data sets that can be thought of as stacks of 2D 
// images that form a 3D volume.  Volume nodes describe where the images 
// are stored on disk, how to render the data (window and level), and how 
// to read the files.  This information is extracted from the image 
// headers (if they exist) at the time the MRML file is generated.  
// Consequently, MRML files isolate MRML browsers from understanding how 
// to read the myriad of file formats for medical data. 

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
  virtual const char* GetNodeTagName() {return "LinearTransform";};

  // Description:
  // 1 if transfrom is linear, 0 otherwise
  virtual int IsLinear() {return 1;};

  // Description:
  // vtkGeneral transform of this node
  virtual vtkGeneralTransform* GetTransformToParent();

  // Description:
  // vtkMatrix4x4 transform of this node
  vtkGetObjectMacro(MatrixTransformToParent, vtkMatrix4x4); 
  vtkSetObjectMacro(MatrixTransformToParent, vtkMatrix4x4); 

  // Description:
  // Get concatinated transforms to the top
  virtual int  GetMatrixTransformToWorld(vtkMatrix4x4* transformToWorld);
  
  // Description:
  // Get concatinated transforms  bwetween nodes  
  virtual int  GetMatrixTransformToNode(vtkMRMLTransformNode* node, 
                                        vtkMatrix4x4* transformToNode);
  
protected:
  vtkMRMLLinearTransformNode();
  ~vtkMRMLLinearTransformNode();
  vtkMRMLLinearTransformNode(const vtkMRMLLinearTransformNode&);
  void operator=(const vtkMRMLLinearTransformNode&);

  vtkMatrix4x4* MatrixTransformToParent;

};

#endif

