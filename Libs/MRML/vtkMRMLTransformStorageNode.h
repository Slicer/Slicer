/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLTransformStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
///  vtkMRMLTransformStorageNode - MRML node for transform storage on disk
/// 
/// Storage nodes has methods to read/write transforms to/from disk

#ifndef __vtkMRMLTransformStorageNode_h
#define __vtkMRMLTransformStorageNode_h

#include "vtkMRML.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLTransformNode.h" 

class vtkImageData;

class VTK_MRML_EXPORT vtkMRMLTransformStorageNode : public vtkMRMLStorageNode
{
  public:
  static vtkMRMLTransformStorageNode *New();
  vtkTypeMacro(vtkMRMLTransformStorageNode,vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  /// 
  /// Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

   /// 
  /// Set dependencies between this node and the parent node
  /// when parsing XML file
  virtual void ProcessParentNode(vtkMRMLNode *parentNode);
  
  /// 
  /// Read data and set it in the referenced node
  /// NOTE: Subclasses should implement this method
  virtual int ReadData(vtkMRMLNode *refNode);

  /// 
  /// Write data from a  referenced node
  /// NOTE: Subclasses should implement this method
  virtual int WriteData(vtkMRMLNode *refNode);

  /// 
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// 
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// 
  /// Get node XML tag name (like Storage, Transform)
  virtual const char* GetNodeTagName()  {return "TransformStorage";};

  /// 
  /// Check to see if this storage node can handle the file type in the input
  /// string. If input string is null, check URI, then check FileName. 
  /// Subclasses should implement this method.
  virtual int SupportedFileType(const char *fileName);

  /// 
  /// Initialize all the supported write file types
  virtual void InitializeSupportedWriteFileTypes();

  /// 
  /// Return a default file extension for writting
  virtual const char* GetDefaultWriteFileExtension()
    {
    return "tfm";
    };

protected:
  vtkMRMLTransformStorageNode();
  ~vtkMRMLTransformStorageNode();
  vtkMRMLTransformStorageNode(const vtkMRMLTransformStorageNode&);
  void operator=(const vtkMRMLTransformStorageNode&);

};

#endif



