/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
// .NAME vtkMRMLStorageNode - a supercalss for other storage nodes
// .SECTION Description
// a supercalss for other storage nodes like volume and model

#ifndef __vtkMRMLStorageNode_h
#define __vtkMRMLStorageNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"


class VTK_MRML_EXPORT vtkMRMLStorageNode : public vtkMRMLNode
{
  public:
  static vtkMRMLStorageNode *New(){return NULL;};
  vtkTypeMacro(vtkMRMLStorageNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance() = 0;

  // Description:
  // Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);
  
  // Description:
  // Read data and set it in the referenced node
  // NOTE: Subclasses should implement this method
  virtual int ReadData(vtkMRMLNode *refNode) = 0;

  // Description:
  // Write data from a  referenced node
  // NOTE: Subclasses should implement this method
  virtual int WriteData(vtkMRMLNode *refNode) = 0;

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName() = 0;

  // Description:
  // A file name or one name in a series
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Use compression on write
  vtkGetMacro(UseCompression, int);
  vtkSetMacro(UseCompression, int);

  // Description:
  // Propagate Progress Event generated in ReadData
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

protected:
  vtkMRMLStorageNode();
  ~vtkMRMLStorageNode();
  vtkMRMLStorageNode(const vtkMRMLStorageNode&);
  void operator=(const vtkMRMLStorageNode&);

  char *FileName;
  int UseCompression;

};

#endif

