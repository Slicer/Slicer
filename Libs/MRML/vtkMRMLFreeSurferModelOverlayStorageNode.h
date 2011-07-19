/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLFreeSurferModelOverlayStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
///  vtkMRMLFreeSurferModelOverlayStorageNode - MRML node for model storage on disk
/// 
/// Storage nodes has methods to read/write vtkPolyData to/from disk

#ifndef __vtkMRMLFreeSurferModelOverlayStorageNode_h
#define __vtkMRMLFreeSurferModelOverlayStorageNode_h

#include "vtkMRMLModelStorageNode.h"

class VTK_MRML_EXPORT vtkMRMLFreeSurferModelOverlayStorageNode : public vtkMRMLModelStorageNode
{
  public:
  static vtkMRMLFreeSurferModelOverlayStorageNode *New();
  vtkTypeMacro(vtkMRMLFreeSurferModelOverlayStorageNode,vtkMRMLModelStorageNode);
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
  /// Copy data from a  referenced node's filename to new location.
  /// NOTE: use this instead of Write Data in the Remote IO Pipeline
  /// until FreeSurferModel Writers are available.
  virtual int CopyData(vtkMRMLNode *refNode, const char *newFileName);

  /// 
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// 
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// 
  /// Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName()  {return "FreeSurferModelOverlayStorage";};
  
//BTX
  /// 
  /// Add a known overlay file extension
  void AddFileExtension(std::string ext);
  /// 
  /// returns true if on the list, false otherwise
  bool IsKnownFileExtension(std::string ext);
//ETX

  /// 
  /// return the number of known file extensions
  int GetNumberOfKnownFileExtensions();
  
  /// 
  /// return the indexed file extension, null if i out of range
  const char *GetKnownFileExtension(int i);

  /// 
  /// can I load files with extensions like this filename has?
  int SupportedFileType(const char *fileName);

  /// 
  /// Initialize all the supported write file types
  virtual void InitializeSupportedWriteFileTypes();

protected:
  vtkMRMLFreeSurferModelOverlayStorageNode();
  ~vtkMRMLFreeSurferModelOverlayStorageNode();
  vtkMRMLFreeSurferModelOverlayStorageNode(const vtkMRMLFreeSurferModelOverlayStorageNode&);
  void operator=(const vtkMRMLFreeSurferModelOverlayStorageNode&);

  /// 
  /// a list of valid overlay file extensions
  //BTX
  std::vector< std::string > KnownFileExtensions; 
  //ETX
};

#endif



