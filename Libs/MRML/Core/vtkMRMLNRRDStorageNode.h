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
class vtkTeemNRRDReader;

/// \brief MRML node for representing a volume storage.
///
/// vtkMRMLNRRDStorageNode nodes describe the archetype based volume storage
/// node that allows to read/write volume data from/to file using generic ITK mechanism.
class VTK_MRML_EXPORT vtkMRMLNRRDStorageNode : public vtkMRMLStorageNode
{
public:
  static vtkMRMLNRRDStorageNode* New();
  vtkTypeMacro(vtkMRMLNRRDStorageNode, vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Read node attributes from XML file
  void ReadXMLAttributes(const char** atts) override;

  ///
  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  ///
  /// Copy the node's attributes to this object
  void Copy(vtkMRMLNode* node) override;

  ///
  /// Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override { return "NRRDStorage"; }

  ///
  /// Center image on read
  vtkGetMacro(CenterImage, int);
  vtkSetMacro(CenterImage, int);

  ///
  /// Only read files that contain diffusion weighted MRI (DWI) metadata (modality: DWMRI or DWMRI_... fields).
  /// Disabled by default. It is enabled when the application looks for a reader for a new file and attempts
  /// to read it as a vector volume, because vtkMRMLVolumeArchetypeStorageNode reads other vector volumes correctly
  /// (including spatial vector, color, and list voxel types). It must remain disabled for reading scenes that
  /// store vector volumes using this storage node.
  vtkGetMacro(ReadOnlyDiffusionWeightedVolumes, bool);
  vtkSetMacro(ReadOnlyDiffusionWeightedVolumes, bool);
  vtkBooleanMacro(ReadOnlyDiffusionWeightedVolumes, bool);

  ///
  /// Returns true if the NRRD file header (read by the reader) contains diffusion weighted MRI metadata.
  static bool HasDiffusionWeightedMetadata(vtkTeemNRRDReader* reader);

  ///
  /// Access the nrrd header fields to create a diffusion gradient table
  int ParseDiffusionInformation(vtkTeemNRRDReader* reader, vtkDoubleArray* grad, vtkDoubleArray* bvalues);

  /// Return true if the node can be read in.
  bool CanReadInReferenceNode(vtkMRMLNode* refNode) override;

  ///
  /// Configure the storage node for data exchange. This is an
  /// opportunity to optimize the storage node's settings, for
  /// instance to turn off compression.
  void ConfigureForDataExchange() override;

  /// Compression parameter corresponding to minimum compression (fast)
  std::string GetCompressionParameterFastest() { return "gzip_fastest"; };

  /// Compression parameter corresponding to normal compression
  std::string GetCompressionParameterNormal() { return "gzip_normal"; };
  /// Compression parameter corresponding to maximum compression (slow)
  std::string GetCompressionParameterMinimumSize() { return "gzip_minimum_size"; };

protected:
  vtkMRMLNRRDStorageNode();
  ~vtkMRMLNRRDStorageNode() override;
  vtkMRMLNRRDStorageNode(const vtkMRMLNRRDStorageNode&);
  void operator=(const vtkMRMLNRRDStorageNode&);

  /// Initialize all the supported write file types
  void InitializeSupportedReadFileTypes() override;

  /// Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;

  /// Read data and set it in the referenced node
  int ReadDataInternal(vtkMRMLNode* refNode) override;

  /// Write data from a  referenced node
  int WriteDataInternal(vtkMRMLNode* refNode) override;

  /// Convert compression parameter string to gzip compression level
  int GetGzipCompressionLevelFromCompressionParameter(std::string parameter);

  int CenterImage;
  bool ReadOnlyDiffusionWeightedVolumes{ false };
};

#endif
