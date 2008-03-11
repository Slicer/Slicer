/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLUnstructuredGridStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
// .NAME vtkMRMLUnstructuredGridStorageNode - MRML node for fiberBundle storage on disk.
// .SECTION Description
// The storage node has methods to read/write vtkPolyData to/from disk.

#ifndef __vtkMRMLUnstructuredGridStorageNode_h
#define __vtkMRMLUnstructuredGridStorageNode_h

#include "vtkMRML.h"
#include "vtkMRMLModelStorageNode.h"
#include "vtkMRMLUnstructuredGridNode.h" 

class VTK_MRML_EXPORT vtkMRMLUnstructuredGridStorageNode : public vtkMRMLStorageNode
{
  public:
  static vtkMRMLUnstructuredGridStorageNode *New();
  vtkTypeMacro(vtkMRMLUnstructuredGridStorageNode,vtkMRMLStorageNode);
  //void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName()  {return "UnstructuredGridStorage";};

 // Description:
  // Read data and set it in the referenced node
  // NOTE: Subclasses should implement this method
  virtual int ReadData(vtkMRMLNode *refNode);

  // Description:
  // Write data from a  referenced node
  // NOTE: Subclasses should implement this method
  virtual int WriteData(vtkMRMLNode *refNode);

  virtual void ProcessParentNode(vtkMRMLNode *parentNode);

  // Description:
  // Check to see if this storage node can handle the file type in the input
  // string. If input string is null, check URI, then check FileName. 
  // Subclasses should implement this method.
  virtual int SupportedFileType(const char *fileName);
  
protected:
  vtkMRMLUnstructuredGridStorageNode(){};
  ~vtkMRMLUnstructuredGridStorageNode(){};
  vtkMRMLUnstructuredGridStorageNode(const vtkMRMLUnstructuredGridStorageNode&);
  void operator=(const vtkMRMLUnstructuredGridStorageNode&);

};

#endif

