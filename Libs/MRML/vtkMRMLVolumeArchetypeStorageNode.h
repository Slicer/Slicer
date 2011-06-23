/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLVolumeArchetypeStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.8 $

=========================================================================auto=*/
///  vtkMRMLVolumeArchetypeStorageNode - MRML node for representing a volume storage
/// 
/// vtkMRMLVolumeArchetypeStorageNode nodes describe the archetybe based volume storage
/// node that allows to read/write volume data from/to file using generic ITK mechanism.

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

  /// 
  /// Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

   /// 
  /// Read data and set it in the referenced node
  /// NOTE: Subclasses should implement this method
  virtual int ReadData(vtkMRMLNode *refNode);

  /// 
  /// Do a temp write to update the file list in this storage node with all
  /// file names that are written when write out the ref node
  /// If move is 1, return the directory that contains the written files and
  /// only the written files, for use in a move instead of a double
  /// write. Otherwise return an empty string.
//BTX
  std::string UpdateFileList(vtkMRMLNode *refNode, int move = 0);
//ETX
  
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
  virtual const char* GetNodeTagName()  {return "VolumeArchetypeStorage";};

  /// 
  /// Center image on read
  vtkGetMacro(CenterImage, int);
  vtkSetMacro(CenterImage, int);

  /// 
  /// whether to read single file or the whole series
  vtkGetMacro(SingleFile, int);
  vtkSetMacro(SingleFile, int);

  /// 
  /// Whether to use orientation from file
  vtkSetMacro(UseOrientationFromFile, int);
  vtkGetMacro(UseOrientationFromFile, int);

  /// 
  /// Check to see if this storage node can handle the file type in the input
  /// string. If input string is null, check URI, then check FileName. 
  /// Subclasses should implement this method.
  virtual int SupportedFileType(const char *fileName);

  /// 
  /// Initialize all the supported write file types
  virtual void InitializeSupportedWriteFileTypes();

  /// 
  /// Return a defualt file extension for writting
  virtual const char* GetDefaultWriteFileExtension()
    {
    return "nrrd";
    };

protected:


  vtkMRMLVolumeArchetypeStorageNode();
  ~vtkMRMLVolumeArchetypeStorageNode();
  vtkMRMLVolumeArchetypeStorageNode(const vtkMRMLVolumeArchetypeStorageNode&);
  void operator=(const vtkMRMLVolumeArchetypeStorageNode&);

  int CenterImage;
  int SingleFile;
  int UseOrientationFromFile;

};

#endif



