/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLNRRDStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.8 $

=========================================================================auto=*/
// .NAME vtkMRMLNRRDStorageNode - MRML node for representing a volume storage
// .SECTION Description
// vtkMRMLNRRDStorageNode nodes describe the archetybe based volume storage
// node that allows to read/write volume data from/to file using generic ITK mechanism.

#ifndef __vtkMRMLNRRDStorageNode_h
#define __vtkMRMLNRRDStorageNode_h

#include "vtkMRML.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLStorageNode.h"
class vtkNRRDReader;
class vtkDoubleArray;

class vtkImageData;

class VTK_MRML_EXPORT vtkMRMLNRRDStorageNode : public vtkMRMLStorageNode
{
  public:
  static vtkMRMLNRRDStorageNode *New();
  vtkTypeMacro(vtkMRMLNRRDStorageNode,vtkMRMLStorageNode);
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
  virtual const char* GetNodeTagName()  {return "NRRDStorage";};

  // Description:
  // Center image on read
  vtkGetMacro(CenterImage, int);
  vtkSetMacro(CenterImage, int);

  // Description:
  // Access the nrrd header fields to create a diffusion gradient table
  int ParseDiffusionInformation(vtkNRRDReader *reader,vtkDoubleArray *grad,vtkDoubleArray *bvalues);

protected:


  vtkMRMLNRRDStorageNode();
  ~vtkMRMLNRRDStorageNode();
  vtkMRMLNRRDStorageNode(const vtkMRMLNRRDStorageNode&);
  void operator=(const vtkMRMLNRRDStorageNode&);

  int CenterImage;

};

#endif

