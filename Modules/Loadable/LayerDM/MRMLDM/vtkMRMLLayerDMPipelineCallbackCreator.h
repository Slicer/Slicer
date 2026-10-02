/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware SAS

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Thibault Pelletier, Kitware SAS,
  and was partially funded by ANR grants ANR-22-CE45-0034 and ANR-18-RHUS-005.

==============================================================================*/

#ifndef __vtkMRMLLayerDMPipelineCallbackCreator_h
#define __vtkMRMLLayerDMPipelineCallbackCreator_h

#include "vtkSlicerLayerDMModuleMRMLDisplayableManagerExport.h"

#include "vtkMRMLLayerDMPipelineCreator.h"

// STL includes
#include <functional>

/// \brief Lambda pipeline creator.
/// Delegates to its internal lambda when CreatePipeline is called.
///
/// \sa vtkMRMLLayerDMPipelineScriptedCreator
/// \sa vtkMRMLLayerDMPipelineFactory::AddPipelineCreator
class VTK_SLICER_LAYERDM_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLLayerDMPipelineCallbackCreator : public vtkMRMLLayerDMPipelineCreator
{
public:
  static vtkMRMLLayerDMPipelineCallbackCreator* New();
  vtkTypeMacro(vtkMRMLLayerDMPipelineCallbackCreator, vtkMRMLLayerDMPipelineCreator);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkSmartPointer<vtkMRMLLayerDMPipeline> CreatePipeline(vtkMRMLAbstractViewNode* viewNode, vtkMRMLNode* node) const override;
  void SetCallback(const std::function<vtkSmartPointer<vtkMRMLLayerDMPipeline>(vtkMRMLAbstractViewNode*, vtkMRMLNode*)>& callback);

protected:
  vtkMRMLLayerDMPipelineCallbackCreator();
  ~vtkMRMLLayerDMPipelineCallbackCreator() override = default;

private:
  std::function<vtkSmartPointer<vtkMRMLLayerDMPipeline>(vtkMRMLAbstractViewNode*, vtkMRMLNode*)> Callback;
};

#endif
