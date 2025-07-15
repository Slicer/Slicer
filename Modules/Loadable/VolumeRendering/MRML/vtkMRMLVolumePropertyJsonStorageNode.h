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

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported in part through NIH grant R01 HL153166.

==============================================================================*/
///  vtkMRMLVolumePropertyJsonStorageNode - MRML node for volume property storage as json
///
/// Storage nodes has methods to read/write transforms to/from disk

#ifndef __vtkMRMLVolumePropertyJsonStorageNode_h
#define __vtkMRMLVolumePropertyJsonStorageNode_h

// VolumeRendering includes
#include "vtkSlicerVolumeRenderingModuleMRMLExport.h"

// MRML includes
#include "vtkMRMLStorageNode.h"

class vtkContourValues;
class vtkDataObject;
class vtkImageData;
class vtkMRMLJsonElement;
class vtkMRMLJsonWriter;
class vtkMRMLVolumePropertyNode;
class vtkPiecewiseFunction;
class vtkVolumeProperty;

class VTK_SLICER_VOLUMERENDERING_MODULE_MRML_EXPORT vtkMRMLVolumePropertyJsonStorageNode : public vtkMRMLStorageNode
{
public:
  static vtkMRMLVolumePropertyJsonStorageNode* New();
  vtkTypeMacro(vtkMRMLVolumePropertyJsonStorageNode, vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Get node XML tag name (like Storage, Transform)
  const char* GetNodeTagName() override { return "VolumePropertyJsonStorage"; }

  /// Return true if the node can be read in
  bool CanReadInReferenceNode(vtkMRMLNode* refNode) override;

  int GetNumberOfVolumePropertiesInFile(const char* filePath);
  vtkMRMLVolumePropertyNode* AddNewVolumePropertyNodeFromFile(const char* filePath, const char* nodeName = nullptr, int vpIndex = 0);

protected:
  vtkMRMLVolumePropertyJsonStorageNode();
  ~vtkMRMLVolumePropertyJsonStorageNode() override;
  vtkMRMLVolumePropertyJsonStorageNode(const vtkMRMLVolumePropertyJsonStorageNode&);
  void operator=(const vtkMRMLVolumePropertyJsonStorageNode&);

  /// Initialize all the supported read file types
  void InitializeSupportedReadFileTypes() override;

  /// Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;

  /// Read data and set it in the referenced node
  int ReadDataInternal(vtkMRMLNode* refNode) override;

  /// Write data from a  referenced node
  int WriteDataInternal(vtkMRMLNode* refNode) override;

  static std::string GetInterpolationTypeAsString(int interpolationType);
  static int GetInterpolationTypeFromString(const std::string& interpolationTypeString);

  bool WriteVolumePropertyNode(vtkMRMLJsonWriter* writer, vtkMRMLVolumePropertyNode* volumePropertyNode);
  bool WriteVolumeProperty(vtkMRMLJsonWriter* writer, vtkVolumeProperty* volumeProperty, int numberOfComponents = 1);
  bool WriteContourValues(vtkMRMLJsonWriter* writer, const char* name, vtkContourValues* contourValues);
  bool WriteTransferFunction(vtkMRMLJsonWriter* writer, const char* name, vtkObject* transferFunction);

  bool ReadVolumePropertyNode(vtkMRMLVolumePropertyNode* vpNode, vtkMRMLJsonElement* volumePropertyElement);
  bool ReadTransferFunction(vtkObject* transferFunction, vtkMRMLJsonElement* transferFunctionElement);
};

#endif
