/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLVolumeArchetypeStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.8 $

=========================================================================auto=*/

#ifndef __vtkMRMLVolumeArchetypeStorageNode_h
#define __vtkMRMLVolumeArchetypeStorageNode_h

#include "vtkMRMLStorageNode.h"

class vtkImageData;
class vtkITKArchetypeImageSeriesReader;
class vtkMRMLVolumeNode;

/// \brief MRML node for representing a volume storage.
///
/// vtkMRMLVolumeArchetypeStorageNode nodes describe the archetybe based volume storage
/// node that allows to read/write volume data from/to file using generic ITK mechanism.
class VTK_MRML_EXPORT vtkMRMLVolumeArchetypeStorageNode : public vtkMRMLStorageNode
{
public:
  static vtkMRMLVolumeArchetypeStorageNode *New();
  vtkTypeMacro(vtkMRMLVolumeArchetypeStorageNode,vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Do a temp write to update the file list in this storage node with all
  /// file names that are written when write out the ref node
  /// If move is 1, return the directory that contains the written files and
  /// only the written files, for use in a move instead of a double
  /// write. Otherwise return an empty string.
  std::string UpdateFileList(vtkMRMLNode *refNode, int move = 0);

  ///
  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  ///
  /// Copy the node's attributes to this object
  void Copy(vtkMRMLNode *node) override;

  ///
  /// Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override {return "VolumeArchetypeStorage";}

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

  /// Return true if the reference node is supported by the storage node
  bool CanReadInReferenceNode(vtkMRMLNode* refNode) override;
  bool CanWriteFromReferenceNode(vtkMRMLNode* refNode) override;

  ///
  /// Configure the storage node for data exchange. This is an
  /// opportunity to optimize the storage node's settings, for
  /// instance to turn off compression.
  void ConfigureForDataExchange() override;

  ///
  /// Provide a uniform way to populate the volume nodes's itk
  /// metadatadictionary from the reader.  Since itk::MetaDataDictionary
  /// is not exposed in python, this method allows it to be set indirectly
  /// using only wrapped types.
  static void SetMetaDataDictionaryFromReader(vtkMRMLVolumeNode*, vtkITKArchetypeImageSeriesReader*);

protected:
  vtkMRMLVolumeArchetypeStorageNode();
  ~vtkMRMLVolumeArchetypeStorageNode() override;
  vtkMRMLVolumeArchetypeStorageNode(const vtkMRMLVolumeArchetypeStorageNode&);
  void operator=(const vtkMRMLVolumeArchetypeStorageNode&);

  /// Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;

  vtkITKArchetypeImageSeriesReader* InstantiateVectorVolumeReader(const std::string &fullName);

  void ConvertSpatialVectorVoxelsBetweenRasLps(vtkImageData* imageData);

  /// Read data and set it in the referenced node
  int ReadDataInternal(vtkMRMLNode *refNode) override;

  /// Write data from a referenced node
  int WriteDataInternal(vtkMRMLNode *refNode) override;

  int CenterImage;
  int SingleFile;
  int UseOrientationFromFile;

};

#endif
