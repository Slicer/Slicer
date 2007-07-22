/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLVolumeArchetypeStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.8 $

=========================================================================auto=*/
// .NAME vtkMRMLVolumeArchetypeStorageNode - MRML node for representing a volume storage
// .SECTION Description
// vtkMRMLVolumeArchetypeStorageNode nodes describe the archetybe based volume storage
// node that allows to read/write volume data from/to file using generic ITK mechanism.

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
  virtual const char* GetNodeTagName()  {return "VolumeArchetypeStorage";};

  // Description:
  // Center image on read
  vtkGetMacro(CenterImage, int);
  vtkSetMacro(CenterImage, int);

protected:


  vtkMRMLVolumeArchetypeStorageNode();
  ~vtkMRMLVolumeArchetypeStorageNode();
  vtkMRMLVolumeArchetypeStorageNode(const vtkMRMLVolumeArchetypeStorageNode&);
  void operator=(const vtkMRMLVolumeArchetypeStorageNode&);

  int CenterImage;

};

#endif

