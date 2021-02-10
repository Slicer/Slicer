/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/

/// Markups Module MRML storage nodes
///
/// vtkMRMLMarkupsROIJsonStorageNode - MRML node for storing markups in JSON file
///

#ifndef __vtkMRMLMarkupsROIJsonStorageNode_h
#define __vtkMRMLMarkupsROIJsonStorageNode_h

// Markups includes
#include "vtkSlicerMarkupsModuleMRMLExport.h"
#include "vtkMRMLMarkupsJsonStorageNode.h"

class vtkMRMLMarkupsNode;
class vtkMRMLMarkupsDisplayNode;

/// \ingroup Slicer_QtModules_Markups
class VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkMRMLMarkupsROIJsonStorageNode : public vtkMRMLMarkupsJsonStorageNode
{
public:
  static vtkMRMLMarkupsROIJsonStorageNode* New();
  vtkTypeMacro(vtkMRMLMarkupsROIJsonStorageNode, vtkMRMLMarkupsJsonStorageNode);

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override { return "MarkupsROIJsonStorage"; };

  bool CanReadInReferenceNode(vtkMRMLNode* refNode) override;

protected:
  vtkMRMLMarkupsROIJsonStorageNode();
  ~vtkMRMLMarkupsROIJsonStorageNode() override;
  vtkMRMLMarkupsROIJsonStorageNode(const vtkMRMLMarkupsROIJsonStorageNode&);
  void operator=(const vtkMRMLMarkupsROIJsonStorageNode&);

  class vtkInternalROI;
  friend class vtkInternalROI;
};

#endif
