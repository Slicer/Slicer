/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Simon Drouin, Brigham and Women's
  Hospital, Boston, MA.

==============================================================================*/
///  vtkMRMLShaderPropertyStorageNode - MRML node for transform storage on disk
///
/// Storage nodes has methods to read/write transforms to/from disk

#ifndef __vtkMRMLShaderPropertyStorageNode_h
#define __vtkMRMLShaderPropertyStorageNode_h

// VolumeRendering includes
#include "vtkSlicerVolumeRenderingModuleMRMLExport.h"

// MRML includes
#include "vtkMRMLStorageNode.h"

class vtkImageData;

class VTK_SLICER_VOLUMERENDERING_MODULE_MRML_EXPORT vtkMRMLShaderPropertyStorageNode
  : public vtkMRMLStorageNode
{
public:
  static vtkMRMLShaderPropertyStorageNode *New();
  vtkTypeMacro(vtkMRMLShaderPropertyStorageNode,vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

  virtual vtkMRMLNode* CreateNodeInstance() VTK_OVERRIDE;

  ///
  /// Get node XML tag name (like Storage, Transform)
  virtual const char* GetNodeTagName() VTK_OVERRIDE {return "ShaderPropertyStorage";}

  /// Return true if the node can be read in
  virtual bool CanReadInReferenceNode(vtkMRMLNode *refNode) VTK_OVERRIDE;

protected:
  vtkMRMLShaderPropertyStorageNode();
  ~vtkMRMLShaderPropertyStorageNode() VTK_OVERRIDE;
  vtkMRMLShaderPropertyStorageNode(const vtkMRMLShaderPropertyStorageNode&);
  void operator=(const vtkMRMLShaderPropertyStorageNode&);

  /// Initialize all the supported read file types
  virtual void InitializeSupportedReadFileTypes() VTK_OVERRIDE;

  /// Initialize all the supported write file types
  virtual void InitializeSupportedWriteFileTypes() VTK_OVERRIDE;

  /// Read data and set it in the referenced node
  virtual int ReadDataInternal(vtkMRMLNode *refNode) VTK_OVERRIDE;

  /// Write data from a  referenced node
  virtual int WriteDataInternal(vtkMRMLNode *refNode) VTK_OVERRIDE;

};

#endif
