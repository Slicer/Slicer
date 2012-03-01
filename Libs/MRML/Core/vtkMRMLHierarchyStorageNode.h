/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLHierarchyStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.8 $

=========================================================================auto=*/
// .NAME vtkMRMLHierarchyStorageNode - MRML node for representing a no-op hierarchy storage
// .SECTION Description
// vtkMRMLHierarchyStorageNode nodes describe the storage that is a place
// holder for hierarchy nodes that don't need to write anything to file,
// subclasses should reimplement ReadData and WriteData

#ifndef __vtkMRMLHierarchyStorageNode_h
#define __vtkMRMLHierarchyStorageNode_h

#include "vtkMRMLStorageNode.h"

class VTK_MRML_EXPORT vtkMRMLHierarchyStorageNode : public vtkMRMLStorageNode
{
  public:
  static vtkMRMLHierarchyStorageNode *New();
  vtkTypeMacro(vtkMRMLHierarchyStorageNode,vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

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
  // Set dependencies between this node and the parent node
  // when parsing XML file
  virtual void ProcessParentNode(vtkMRMLNode *parentNode);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName()  {return "HierarchyStorage";};

  // Description:
  // Check to see if this storage node can handle the file type in the input
  // string. If input string is null, check URI, then check FileName. 
  // Subclasses should implement this method.
  virtual int SupportedFileType(const char *fileName);

  // Description:
  // Initialize all the supported write file types
  virtual void InitializeSupportedWriteFileTypes();

  // Description:
  // Return a default file extension for writting
  virtual const char* GetDefaultWriteFileExtension()
    {
    return "txt";
    };

protected:


  vtkMRMLHierarchyStorageNode();
  ~vtkMRMLHierarchyStorageNode();
  vtkMRMLHierarchyStorageNode(const vtkMRMLHierarchyStorageNode&);
  void operator=(const vtkMRMLHierarchyStorageNode&);

};

#endif



