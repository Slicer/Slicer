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

==============================================================================*/

#pragma once

#include "vtkSlicerLayerDMModuleMRMLDisplayableManagerExport.h"

#include "vtkMRMLLayerDMPipeline.h"

// VTK includes
#include <vtkObject.h>

class vtkMRMLAbstractViewNode;
class vtkMRMLNode;

/// \brief Interface responsible for creating new pipelines given input pairs of viewNode and node.
///
/// \sa vtkMRMLLayerDMPipelineCallbackCreator
/// \sa vtkMRMLLayerDMPipelineFactory::AddPipelineCreator
class VTK_SLICER_LAYERDM_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLLayerDMPipelineCreator : public vtkObject
{
public:
  static vtkMRMLLayerDMPipelineCreator* New();
  vtkTypeMacro(vtkMRMLLayerDMPipelineCreator, vtkObject);

  virtual vtkSmartPointer<vtkMRMLLayerDMPipeline> CreatePipeline(vtkMRMLAbstractViewNode* viewNode, vtkMRMLNode* node) const;

  /// @{
  /// \brief Get/Set the priority of creation of the pipeline creator.
  /// Pipelines with higher priority will be allowed to handle creation first (allowing to supplant existing creators).
  vtkGetMacro(Priority, int);
  vtkSetMacro(Priority, int);
  /// @}
protected:
  vtkMRMLLayerDMPipelineCreator() = default;
  ~vtkMRMLLayerDMPipelineCreator() override = default;

private:
  int Priority = 0;
};
