/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLFiberBundleStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
// .NAME vtkMRMLFiberBundleStorageNode - MRML node for fiberBundle storage on disk.
// .SECTION Description
// The storage node has methods to read/write vtkPolyData to/from disk.

#ifndef __vtkMRMLFiberBundleStorageNode_h
#define __vtkMRMLFiberBundleStorageNode_h

#include "vtkMRML.h"
#include "vtkMRMLModelStorageNode.h"
#include "vtkMRMLFiberBundleNode.h" 

class VTK_MRML_EXPORT vtkMRMLFiberBundleStorageNode : public vtkMRMLModelStorageNode
{
  public:
  static vtkMRMLFiberBundleStorageNode *New();
  vtkTypeMacro(vtkMRMLFiberBundleStorageNode,vtkMRMLModelStorageNode);
  //void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName()  {return "FiberBundleStorage";};

  // Description:
  // Check to see if this storage node can handle the file type in the input
  // string. If input string is null, check URI, then check FileName. 
  // Subclasses should implement this method.
  virtual int SupportedFileType(const char *fileName);

protected:
  vtkMRMLFiberBundleStorageNode(){};
  ~vtkMRMLFiberBundleStorageNode(){};
  vtkMRMLFiberBundleStorageNode(const vtkMRMLFiberBundleStorageNode&);
  void operator=(const vtkMRMLFiberBundleStorageNode&);

};

#endif

