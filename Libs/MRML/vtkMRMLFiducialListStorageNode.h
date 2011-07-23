/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLFiducialListStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.8 $

=========================================================================auto=*/
///  vtkMRMLFiducialListStorageNode - MRML node for representing a volume storage
/// 
/// vtkMRMLFiducialListStorageNode nodes describe the fiducial storage
/// node that allows to read/write point data from/to file.

#ifndef __vtkMRMLFiducialListStorageNode_h
#define __vtkMRMLFiducialListStorageNode_h

#include "vtkMRMLStorageNode.h"

class VTK_MRML_EXPORT vtkMRMLFiducialListStorageNode : public vtkMRMLStorageNode
{
  public:
  static vtkMRMLFiducialListStorageNode *New();
  vtkTypeMacro(vtkMRMLFiducialListStorageNode,vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  /// 
  /// Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

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

 /// Description:
  /// Set dependencies between this node and the parent node
  /// when parsing XML file
  virtual void ProcessParentNode(vtkMRMLNode *parentNode);

  /// 
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// 
  /// Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName()  {return "FiducialListStorage";};

  /// 
  /// Check to see if this storage node can handle the file type in the input
  /// string. If input string is null, check URI, then check FileName. 
  /// Subclasses should implement this method.
  virtual int SupportedFileType(const char *fileName);

  /// 
  /// Initialize all the supported write file types
  virtual void InitializeSupportedWriteFileTypes();

  /// 
  /// Return a default file extension for writing
  virtual const char* GetDefaultWriteFileExtension()
    {
    return "fcsv";
    };

  ///
  /// Get/Set the storage node version
  vtkGetMacro(Version, int);
  vtkSetMacro(Version, int);

protected:


  vtkMRMLFiducialListStorageNode();
  ~vtkMRMLFiducialListStorageNode();
  vtkMRMLFiducialListStorageNode(const vtkMRMLFiducialListStorageNode&);
  void operator=(const vtkMRMLFiducialListStorageNode&);

  ///
  /// the storage node version
  // version 1 has the old glyph numbering (pre svn 12553), starting at 0
  // version 2 has the new glyph numbering, starting at 1
  int Version;
};

#endif



