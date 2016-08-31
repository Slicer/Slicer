/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Adam Rankin and Csaba Pinter, PerkLab, Queen's
  University and was supported through the Applied Cancer Research Unit program of Cancer
  Care Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#ifndef __vtkMRMLSegmentationStorageNode_h
#define __vtkMRMLSegmentationStorageNode_h

// Temporarily keep support for reading nrrd files that are saved as 4D spatial image.
// The current way of writing volume is 3 spatial dimension and a list.
#define SUPPORT_4D_SPATIAL_NRRD

// MRML includes
#include "vtkMRMLStorageNode.h"

#ifdef SUPPORT_4D_SPATIAL_NRRD
  // ITK includes
  #include <itkImageRegionIteratorWithIndex.h>
#endif

class vtkMRMLSegmentationNode;
class vtkMatrix4x4;
class vtkPolyData;
class vtkOrientedImageData;
class vtkSegmentation;
class vtkMRMLSegmentationNode;
class vtkSegment;
class vtkInformationStringKey;
class vtkInformationIntegerVectorKey;

/// \brief MRML node for segmentation storage on disk.
///
/// Storage nodes has methods to read/write segmentations to/from disk.
class VTK_MRML_EXPORT vtkMRMLSegmentationStorageNode : public vtkMRMLStorageNode
{
#ifdef SUPPORT_4D_SPATIAL_NRRD
  // Although internally binary labelmap representations can be of unsigned char, unsigned short
  // or short types, the output file is always unsigned char
  //TODO: This is a limitation for now
  typedef itk::Image<unsigned char, 4> BinaryLabelmap4DImageType;
  typedef itk::ImageRegionIteratorWithIndex<BinaryLabelmap4DImageType> BinaryLabelmap4DIteratorType;
#endif

public:
  static vtkMRMLSegmentationStorageNode *New();
  vtkTypeMacro(vtkMRMLSegmentationStorageNode, vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  /// Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName()  {return "SegmentationStorage";};

  /// Return a default file extension for writing
  /// File write extension is determined dynamically
  /// from master representation type.
  virtual const char* GetDefaultWriteFileExtension();

  /// Return true if the reference node can be read in
  virtual bool CanReadInReferenceNode(vtkMRMLNode *refNode);

  /// Reset supported write file types. Called when master representation is changed
  void ResetSupportedWriteFileTypes();

protected:
  /// Initialize all the supported read file types
  virtual void InitializeSupportedReadFileTypes();

  /// Initialize all the supported write file types
  virtual void InitializeSupportedWriteFileTypes();

  /// Get data node that is associated with this storage node
  vtkMRMLSegmentationNode* GetAssociatedDataNode();

  /// Write data from a referenced node
  virtual int WriteDataInternal(vtkMRMLNode *refNode);

  /// Write binary labelmap representation to file
  virtual int WriteBinaryLabelmapRepresentation(vtkMRMLSegmentationNode* segmentationNode, std::string path);

  /// Write a poly data representation to file
  virtual int WritePolyDataRepresentation(vtkMRMLSegmentationNode* segmentationNode, std::string path);

  /// Read data and set it in the referenced node
  virtual int ReadDataInternal(vtkMRMLNode *refNode);

  /// Read binary labelmap representation from nrrd file (3D spatial + list)
  virtual int ReadBinaryLabelmapRepresentation(vtkMRMLSegmentationNode* segmentationNode, std::string path);

#ifdef SUPPORT_4D_SPATIAL_NRRD
  /// Read binary labelmap representation from 4D spatial nrrd file - obsolete
  virtual int ReadBinaryLabelmapRepresentation4DSpatial(vtkMRMLSegmentationNode* segmentationNode, std::string path);
#endif

  /// Read a poly data representation to file
  virtual int ReadPolyDataRepresentation(vtkMRMLSegmentationNode* segmentationNode, std::string path);

  /// Add all files corresponding to poly data representation to the storage node
  /// (multiblock dataset writes segments to individual files in a separate folder)
  void AddPolyDataFileNames(std::string path, vtkSegmentation* segmentation);

  /// Serialize contained representation names in a string
  std::string SerializeContainedRepresentationNames(vtkSegmentation* segmentation);

  /// Create representations based on serialized representation names string
  void CreateRepresentationsBySerializedNames(vtkSegmentation* segmentation, std::string representationNames);

  static std::string GetSegmentMetaDataKey(int segmentIndex, const std::string& keyName);

  static std::string GetSegmentationMetaDataKey(const std::string& keyName);

  static std::string GetSegmentTagsAsString(vtkSegment* segment);
  static void SetSegmentTagsFromString(vtkSegment* segment, std::string tagsValue);

  static std::string GetImageExtentAsString(vtkOrientedImageData* image);
  static std::string GetImageExtentAsString(int extent[6]);
  static void GetImageExtentFromString(int extent[6], std::string extentValue);

  static std::string GetSegmentDefaultColorAsString(vtkMRMLSegmentationNode* segmentationNode, const std::string& segmentId);
  static void GetSegmentDefaultColorFromString(double defaultColor[3], std::string defaultColorValue);

protected:
  vtkMRMLSegmentationStorageNode();
  ~vtkMRMLSegmentationStorageNode();

private:
  vtkMRMLSegmentationStorageNode(const vtkMRMLSegmentationStorageNode&);  /// Not implemented.
  void operator=(const vtkMRMLSegmentationStorageNode&);  /// Not implemented.
};

#endif
