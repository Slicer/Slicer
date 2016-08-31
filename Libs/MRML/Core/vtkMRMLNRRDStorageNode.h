/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLNRRDStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.8 $

=========================================================================auto=*/

#ifndef __vtkMRMLNRRDStorageNode_h
#define __vtkMRMLNRRDStorageNode_h

#include "vtkMRMLStorageNode.h"
class vtkDoubleArray;
class vtkNRRDReader;

/// \brief MRML node for representing a volume storage.
///
/// vtkMRMLNRRDStorageNode nodes describe the archetybe based volume storage
/// node that allows to read/write volume data from/to file using generic ITK mechanism.
class VTK_MRML_EXPORT vtkMRMLNRRDStorageNode : public vtkMRMLStorageNode
{
  public:
  static vtkMRMLNRRDStorageNode *New();
  vtkTypeMacro(vtkMRMLNRRDStorageNode,vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  ///
  /// Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  ///
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  ///
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  ///
  /// Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName()  {return "NRRDStorage";};

  ///
  /// Center image on read
  vtkGetMacro(CenterImage, int);
  vtkSetMacro(CenterImage, int);

  ///
  /// Access the nrrd header fields to create a diffusion gradient table
  int ParseDiffusionInformation(vtkNRRDReader *reader,vtkDoubleArray *grad,vtkDoubleArray *bvalues);

  /// Return true if the node can be read in.
  virtual bool CanReadInReferenceNode(vtkMRMLNode *refNode);

  ///
  /// Configure the storage node for data exchange. This is an
  /// opportunity to optimize the storage node's settings, for
  /// instance to turn off compression.
  virtual void ConfigureForDataExchange();

protected:
  vtkMRMLNRRDStorageNode();
  ~vtkMRMLNRRDStorageNode();
  vtkMRMLNRRDStorageNode(const vtkMRMLNRRDStorageNode&);
  void operator=(const vtkMRMLNRRDStorageNode&);

  /// Initialize all the supported write file types
  virtual void InitializeSupportedReadFileTypes();

  /// Initialize all the supported write file types
  virtual void InitializeSupportedWriteFileTypes();

  /// Read data and set it in the referenced node
  virtual int ReadDataInternal(vtkMRMLNode *refNode);

  /// Write data from a  referenced node
  virtual int WriteDataInternal(vtkMRMLNode *refNode);

  int CenterImage;

};

#endif
