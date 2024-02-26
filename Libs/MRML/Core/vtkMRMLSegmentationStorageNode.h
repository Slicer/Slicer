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
# include <itkImageRegionIteratorWithIndex.h>
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
/// Only the source representation of the segmentation is stored on disk.
///
/// If source representation is labelmap then it is stored as a NRRD image file (.seg.nrrd file).
/// Upon saving, segments are automatically collapsed to as few 3D volumes as possible.
/// If no segments overlap, then the segmentation will be saved as a 3D volume.
/// If segments overlap (same voxel position is included in multiple segments) then a 4D volume is saved.
///
/// If source representation is polygonal mesh, such as closed
/// surface, then the segmentation is stored as a VTK multiblock data set
/// (.vtm file accompanied by a number of .vtk polydata files).
///
/// Detailed specification of the segmentation file format is available at:
/// https://slicer.readthedocs.io/en/latest/developer_guide/modules/segmentations.html
///

class VTK_MRML_EXPORT vtkMRMLSegmentationStorageNode : public vtkMRMLStorageNode
{
#ifdef SUPPORT_4D_SPATIAL_NRRD
  // Although internally binary labelmap representations can be of unsigned char, unsigned short
  // or short types, the output file is always unsigned char
  // TODO: This is a limitation for now
  typedef itk::Image<unsigned char, 4> BinaryLabelmap4DImageType;
  typedef itk::ImageRegionIteratorWithIndex<BinaryLabelmap4DImageType> BinaryLabelmap4DIteratorType;
#endif

public:
  static vtkMRMLSegmentationStorageNode* New();
  vtkTypeMacro(vtkMRMLSegmentationStorageNode, vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  /// Read node attributes from XML file
  void ReadXMLAttributes(const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy the node's attributes to this object
  void Copy(vtkMRMLNode* node) override;

  /// Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override { return "SegmentationStorage"; }

  /// Return a default file extension for writing
  /// File write extension is determined dynamically
  /// from source representation type.
  const char* GetDefaultWriteFileExtension() override;

  /// Return true if the reference node can be read in
  bool CanReadInReferenceNode(vtkMRMLNode* refNode) override;

  /// Reset supported write file types. Called when source representation is changed
  void ResetSupportedWriteFileTypes();

  /// Controls if segmentation labelmap representation is written using minimum necessary extent
  /// or the extent of reference image.
  /// If false (default): the segmentation will be saved using the same extent as the reference image.
  /// If true: the segmentation will be saved using the effective extent. This saves space (especially when
  /// saving the segmentation uncompressed), but makes voxel coordinates of the segmentation different from
  /// voxel coordinates of the reference image, which may cause problems in software that assume voxel coordinate system
  /// of the reference image is the same as the segmentation's.
  vtkSetMacro(CropToMinimumExtent, bool);
  vtkGetMacro(CropToMinimumExtent, bool);
  vtkBooleanMacro(CropToMinimumExtent, bool);

protected:
  /// Initialize all the supported read file types
  void InitializeSupportedReadFileTypes() override;

  /// Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;

  /// Get data node that is associated with this storage node
  vtkMRMLSegmentationNode* GetAssociatedDataNode();

  /// Write data from a referenced node
  int WriteDataInternal(vtkMRMLNode* refNode) override;

  /// Write binary labelmap representation to file
  virtual int WriteBinaryLabelmapRepresentation(vtkMRMLSegmentationNode* segmentationNode, std::string path);

  /// Write a poly data representation to file
  virtual int WritePolyDataRepresentation(vtkMRMLSegmentationNode* segmentationNode, std::string path);

  /// Read data and set it in the referenced node
  int ReadDataInternal(vtkMRMLNode* refNode) override;

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

  /// Get the metadata string for the segment and key from the dictionary
  static bool GetSegmentMetaDataFromDicitionary(std::string& headerValue,
                                                itk::MetaDataDictionary dictionary,
                                                int segmentIndex,
                                                std::string keyName);

  /// Get the metadata string for the segmentation key from the dictionary
  static bool GetSegmentationMetaDataFromDicitionary(std::string& headerValue,
                                                     itk::MetaDataDictionary dictionary,
                                                     std::string keyName);

  static std::string GetSegmentMetaDataKey(int segmentIndex, const std::string& keyName);

  static std::string GetSegmentationMetaDataKey(const std::string& keyName);

  static std::string GetSegmentTagsAsString(vtkSegment* segment);
  static void SetSegmentTagsFromString(vtkSegment* segment, std::string tagsValue);

  static std::string GetImageExtentAsString(vtkOrientedImageData* image);
  static std::string GetImageExtentAsString(int extent[6]);
  static void GetImageExtentFromString(int extent[6], std::string extentValue);

  static std::string GetSegmentColorAsString(vtkMRMLSegmentationNode* segmentationNode, const std::string& segmentId);
  static void GetSegmentColorFromString(double color[3], std::string colorString);

protected:
  bool CropToMinimumExtent{ false };

protected:
  vtkMRMLSegmentationStorageNode();
  ~vtkMRMLSegmentationStorageNode() override;

private:
  vtkMRMLSegmentationStorageNode(const vtkMRMLSegmentationStorageNode&) = delete;
  void operator=(const vtkMRMLSegmentationStorageNode&) = delete;
};

#endif
