/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLFiberBundleStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
// .NAME vtkMRMLFiberBundleStorageNode - MRML node for fiberBundle storage on disk
// .SECTION Description
// Storage nodes has methods to read/write vtkPolyData to/from disk

#ifndef __vtkMRMLFiberBundleStorageNode_h
#define __vtkMRMLFiberBundleStorageNode_h

#include "vtkMRML.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLFiberBundleNode.h" 

class vtkImageData;

class VTK_MRML_EXPORT vtkMRMLFiberBundleStorageNode : public vtkMRMLStorageNode
{
  public:
  static vtkMRMLFiberBundleStorageNode *New();
  vtkTypeMacro(vtkMRMLFiberBundleStorageNode,vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

   // Description:
  // Set dependencies between this node and the parent node
  // when parsing XML file
  virtual void ProcessParentNode(vtkMRMLNode *parentNode);
  
  // Description:
  // Read data and set it in the referenced node
  // NOTE: Subclasses should implement this method
  virtual int ReadData(vtkMRMLNode *refNode);

  // Description:
  // Write data from a  referenced node
  // NOTE: Subclasses should implement this method
  virtual int WriteData(vtkMRMLNode *refNode);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName()  {return "FiberBundleStorage";};


protected:
  vtkMRMLFiberBundleStorageNode();
  ~vtkMRMLFiberBundleStorageNode();
  vtkMRMLFiberBundleStorageNode(const vtkMRMLFiberBundleStorageNode&);
  void operator=(const vtkMRMLFiberBundleStorageNode&);

};

#endif

