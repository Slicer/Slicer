/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLVolumeArchetypeStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.8 $

=========================================================================auto=*/
// .NAME vtkMRMLVolumeArchetypeStorageNode - MRML node for representing a volume (image stack).
// .SECTION Description
// Storage nodes describe data sets that can be thought of as stacks of 2D 
// images that form a 3D volume.  Storage nodes describe where the images 
// are stored on disk, how to render the data (window and level), and how 
// to read the files.  This information is extracted from the image 
// headers (if they exist) at the time the MRML file is generated.  
// Consequently, MRML files isolate MRML browsers from understanding how 
// to read the myriad of file formats for medical data. 

#ifndef __vtkMRMLVolumeArchetypeStorageNode_h
#define __vtkMRMLVolumeArchetypeStorageNode_h

#include "vtkMRML.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLStorageNode.h"


class vtkImageData;

class VTK_MRML_EXPORT vtkMRMLVolumeArchetypeStorageNode : public vtkMRMLStorageNode
{
  public:
  static vtkMRMLVolumeArchetypeStorageNode *New();
  vtkTypeMacro(vtkMRMLVolumeArchetypeStorageNode,vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

   // Description:
  // Read data and set it in the referenced node
  // NOTE: Subclasses should implement this method
  virtual void ReadData(vtkMRMLNode *refNode);

  // Description:
  // Write data from a  referenced node
  // NOTE: Subclasses should implement this method
  virtual void WriteData(vtkMRMLNode *refNode);

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
  virtual const char* GetNodeTagName()  {return "VolumeArchetypeStorage";};

  // Description:
  // A file name or one name in a series
  vtkGetStringMacro(FileArchetype);
  vtkSetStringMacro(FileArchetype);

protected:


  vtkMRMLVolumeArchetypeStorageNode();
  ~vtkMRMLVolumeArchetypeStorageNode();
  vtkMRMLVolumeArchetypeStorageNode(const vtkMRMLVolumeArchetypeStorageNode&);
  void operator=(const vtkMRMLVolumeArchetypeStorageNode&);

  char *FileArchetype;

};

#endif

