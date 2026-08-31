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

#include "vtkMRMLLayerDMLayerManager.h"

// Layer DM includes
#include "vtkMRMLLayerDMObjectEventObserver.h"
#include "vtkMRMLLayerDMPipeline.h"

// VTK includes
#include <vtkBoundingBox.h>
#include <vtkCamera.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>

// STD includes
#include <algorithm>

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLLayerDMLayerManager);

//-----------------------------------------------------------------------------
bool vtkMRMLLayerDMLayerManager::AddPipelineLayers(vtkMRMLLayerDMPipeline* pipeline)
{
  if (!pipeline)
  {
    return false;
  }

  for (const auto order : pipeline->GetRenderOrders())
  {
    auto key = std::make_tuple(order, GetCameraId(pipeline->GetCustomCamera(order)));
    if (!this->ContainsLayerKey(key))
    {
      this->PipelineLayers[key] = {};
    }
    this->PipelineLayers[key].emplace(pipeline);
  }
  return true;
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMLayerManager::AddPipeline(vtkMRMLLayerDMPipeline* pipeline)
{
  if (!pipeline)
  {
    return;
  }

  this->Observer->UpdateObservation(nullptr, pipeline, vtkMRMLLayerDMPipeline::RenderGroupingModified);
  this->AddPipelineLayers(pipeline);
  this->UpdateLayers();
}

//-----------------------------------------------------------------------------
int vtkMRMLLayerDMLayerManager::GetNumberOfDistinctLayers() const
{
  return static_cast<int>(this->PipelineLayers.size());
}

//-----------------------------------------------------------------------------
int vtkMRMLLayerDMLayerManager::GetNumberOfManagedLayers() const
{
  return this->GetNumberOfDistinctLayers() - 1;
}

//-----------------------------------------------------------------------------
int vtkMRMLLayerDMLayerManager::GetNumberOfRenderers() const
{
  return static_cast<int>(this->Renderers.size());
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMLayerManager::RemovePipelineLayers(vtkMRMLLayerDMPipeline* pipeline)
{
  for (auto& [key, pipelines] : this->PipelineLayers)
  {
    pipelines.erase(pipeline);
  }
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMLayerManager::RemovePipeline(vtkMRMLLayerDMPipeline* pipeline)
{
  if (!pipeline)
  {
    return;
  }

  this->Observer->UpdateObservation(pipeline, nullptr);
  this->RemovePipelineRenderer(pipeline);
  this->RemovePipelineLayers(pipeline);
  this->UpdateLayers();
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMLayerManager::ResetCameraClippingRange() const
{
  // Reset first renderer clipping range
  if (const auto defaultRenderer = this->GetDefaultRenderer())
  {
    defaultRenderer->ResetCameraClippingRange();
  }

  // Reset the managed renderers grouped by common cameras
  for (const auto& pair : this->CameraRendererMap)
  {
    this->ResetRenderersCameraClippingRange(pair.second, this->ComputeRenderersVisibleBounds(pair.second));
  }
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMLayerManager::SetRenderWindow(vtkRenderWindow* renderWindow)
{
  if (this->RenderWindow == renderWindow)
  {
    return;
  }

  this->RemoveAllLayers();
  this->RenderWindow = renderWindow;
  this->UpdateLayers();
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMLayerManager::SetDefaultCamera(const vtkSmartPointer<vtkCamera>& camera)
{
  if (this->DefaultCamera == camera)
  {
    return;
  }

  this->DefaultCamera = camera;
  this->UpdateLayers();
}

//-----------------------------------------------------------------------------
vtkMRMLLayerDMLayerManager::vtkMRMLLayerDMLayerManager()
  : EmptyPipeline(vtkSmartPointer<vtkMRMLLayerDMPipeline>::New())
  , Observer(vtkSmartPointer<vtkMRMLLayerDMObjectEventObserver>::New())
{
  this->Observer->SetUpdateCallback(
    [this](vtkObject* obj)
    {
      if (auto pipeline = vtkMRMLLayerDMPipeline::SafeDownCast(obj))
      {
        this->RemovePipelineLayers(pipeline);
        this->AddPipelineLayers(pipeline);
        this->UpdateLayers();
      }
    });

  // A pipeline destroyed without being removed first would otherwise be left in the layer sets, where its weak
  // pointer would null itself in place and break their ordering.
  this->Observer->SetDeleteCallback(
    [this](vtkObject* obj)
    {
      if (auto pipeline = vtkMRMLLayerDMPipeline::SafeDownCast(obj))
      {
        this->RemovePipeline(pipeline);
      }
    });
  this->AddPipeline(this->EmptyPipeline);
}

//-----------------------------------------------------------------------------
vtkMRMLLayerDMLayerManager::~vtkMRMLLayerDMLayerManager()
{
  // Releasing the pipelines below destroys them, which would otherwise invoke the delete callback and
  // re-enter this object while its members are being destroyed.
  this->Observer->ClearCallbacks();
}

//-----------------------------------------------------------------------------
vtkRenderer* vtkMRMLLayerDMLayerManager::GetRendererMatchingKey(const LayerKey& key)
{
  // If key index matches the default layer, return the render window's first renderer
  int keyIndex = this->GetKeyIndex(key);
  if (keyIndex == 0)
  {
    return this->GetDefaultRenderer();
  }

  // Otherwise, convert key index to matching managed renderer index and return the associated renderer
  int rendererIndex = keyIndex - 1;
  if (rendererIndex < 0 || rendererIndex >= this->GetNumberOfRenderers())
  {
    return nullptr;
  }
  return this->Renderers[rendererIndex];
}

//-----------------------------------------------------------------------------
vtkRenderer* vtkMRMLLayerDMLayerManager::GetDefaultRenderer() const
{
  if (!this->RenderWindow)
  {
    return nullptr;
  }
  return this->RenderWindow->GetRenderers()->GetFirstRenderer();
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMLayerManager::AddMissingLayers()
{
  while (this->GetNumberOfRenderers() < this->GetNumberOfManagedLayers())
  {
    // Managed renderers are displayed as overlays and should not catch any events.
    // Events handling is done using the DM mechanism.
    auto renderer = vtkSmartPointer<vtkRenderer>::New();
    renderer->InteractiveOff();
    this->RenderWindow->AddRenderer(renderer);
    this->Renderers.emplace_back(renderer);
  }
}

//-----------------------------------------------------------------------------
std::array<double, 6> vtkMRMLLayerDMLayerManager::ComputeRenderersVisibleBounds(const std::vector<vtkWeakPointer<vtkRenderer>>& renderers)
{
  vtkBoundingBox bbox;

  for (const auto& renderer : renderers)
  {
    if (!renderer)
    {
      continue;
    }
    bbox.AddBounds(renderer->ComputeVisiblePropBounds());
  }

  std::array<double, 6> bounds{};
  bbox.GetBounds(bounds.data());
  return bounds;
}

//-----------------------------------------------------------------------------
bool vtkMRMLLayerDMLayerManager::ContainsLayerKey(const LayerKey& key)
{
  return this->PipelineLayers.find(key) != this->PipelineLayers.end();
}

//-----------------------------------------------------------------------------
std::uintptr_t vtkMRMLLayerDMLayerManager::GetCameraId(vtkCamera* camera)
{
  if (!camera)
  {
    return 0;
  }
  return reinterpret_cast<std::uintptr_t>(camera);
}

//-----------------------------------------------------------------------------
vtkCamera* vtkMRMLLayerDMLayerManager::GetCameraForLayer(const LayerKey& key, const std::set<vtkMRMLLayerDMPipeline*>& pipelines) const
{
  if (const auto cameraId = std::get<1>(key); cameraId == 0)
  {
    return this->DefaultCamera;
  }

  if (pipelines.empty())
  {
    return nullptr;
  }

  return (*pipelines.begin())->GetCustomCamera(std::get<0>(key));
}

//-----------------------------------------------------------------------------
int vtkMRMLLayerDMLayerManager::GetKeyIndex(const LayerKey& key) const
{
  int index = 0;
  for (const auto& pair : this->PipelineLayers)
  {
    if (pair.first == key)
    {
      return index;
    }
    ++index;
  }
  return -1;
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMLayerManager::RemoveAllLayers()
{
  // Iterate over a copy as RemoveRenderer erases from this->Renderers
  const auto renderers = this->Renderers;
  for (const auto& renderer : renderers)
  {
    this->RemoveRenderer(renderer);
  }
  this->UpdateRenderWindowNumberOfLayers();
  this->Renderers.clear();
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMLayerManager::RemoveAllPipelineRenderers()
{
  // if the render window is null, notify pipelines
  for (const auto& [key, pipelines] : this->PipelineLayers)
  {
    for (const auto& pipeline : pipelines)
    {
      this->RemovePipelineRenderer(pipeline);
    }
  }
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMLayerManager::RemovePipelineRenderer(vtkMRMLLayerDMPipeline* pipeline)
{
  if (pipeline)
  {
    pipeline->SetRenderers({}, {});
  }
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMLayerManager::RemoveOutdatedLayers()
{
  while (this->GetNumberOfRenderers() && (this->GetNumberOfRenderers() > this->GetNumberOfManagedLayers()))
  {
    this->RemoveRenderer(this->Renderers[this->GetNumberOfRenderers() - 1]);
  }
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMLayerManager::RemoveOutdatedPipelines()
{
  // Remove the layers which no longer contain any pipeline. Destroyed pipelines are removed from their layers
  // when they are destroyed, so only empty layers are left to clean up here.
  // \sa vtkMRMLLayerDMObjectEventObserver::SetDeleteCallback
  for (auto layerIt = this->PipelineLayers.begin(); layerIt != this->PipelineLayers.end();)
  {
    if (layerIt->second.empty())
    {
      layerIt = this->PipelineLayers.erase(layerIt);
    }
    else
    {
      ++layerIt;
    }
  }
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMLayerManager::RemoveRenderer(const vtkSmartPointer<vtkRenderer>& renderer)
{
  if (this->RenderWindow && this->RenderWindow->HasRenderer(renderer))
  {
    this->RenderWindow->RemoveRenderer(renderer);
  }

  const auto rendererIt = std::find(this->Renderers.begin(), this->Renderers.end(), renderer);
  if (rendererIt != this->Renderers.end())
  {
    this->Renderers.erase(rendererIt);
  }
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMLayerManager::ResetRenderersCameraClippingRange(const std::vector<vtkWeakPointer<vtkRenderer>>& renderers, const std::array<double, 6>& bounds)
{
  for (const auto& renderer : renderers)
  {
    if (!renderer)
    {
      continue;
    }
    renderer->ResetCameraClippingRange(const_cast<double*>(bounds.data()));
  }
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMLayerManager::SynchronizePipelineRenderers()
{
  std::map<vtkMRMLLayerDMPipeline*, std::vector<vtkRenderer*>> pipelineRenderers;
  std::map<vtkMRMLLayerDMPipeline*, std::vector<unsigned int>> pipelineOrders;

  for (const auto& [key, pipelines] : this->PipelineLayers)
  {
    auto renderer = this->GetRendererMatchingKey(key);
    auto order = std::get<0>(key);
    for (const auto& pipeline : pipelines)
    {
      pipelineRenderers[pipeline].push_back(renderer);
      pipelineOrders[pipeline].push_back(order);
    }
  }

  for (auto& [pipeline, renderers] : pipelineRenderers)
  {
    pipeline->SetRenderers(renderers, pipelineOrders[pipeline]);
  }
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMLayerManager::UpdateRenderWindowNumberOfLayers() const
{
  if (!this->RenderWindow)
  {
    return;
  }

  // Synchronize the render window number of layers with its actual number of renderers
  int numberOfRenderers = this->RenderWindow->GetRenderers()->GetNumberOfItems();
  int iMax = 0;
  for (int iRenderer = 0; iRenderer < numberOfRenderers; iRenderer++)
  {
    if (auto renderer = vtkRenderer::SafeDownCast(this->RenderWindow->GetRenderers()->GetItemAsObject(iRenderer)))
    {
      iMax = std::max(iMax, renderer->GetLayer());
    }
  }

  this->RenderWindow->SetNumberOfLayers(iMax + 1);
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMLayerManager::UpdateLayers()
{
  if (!this->RenderWindow)
  {
    this->RemoveAllPipelineRenderers();
    return;
  }

  this->RemoveOutdatedPipelines();
  this->RemoveOutdatedLayers();
  this->AddMissingLayers();
  this->UpdateRendererLayerOrdering();
  this->UpdateRendererCamera();
  this->SynchronizePipelineRenderers();
  this->UpdateRenderWindowNumberOfLayers();
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMLayerManager::UpdateRendererLayerOrdering() const
{
  // Managed layers are always ordered from layer 1 to the number of managed renderers
  for (int iRenderer = 0; iRenderer < this->GetNumberOfRenderers(); iRenderer++)
  {
    this->Renderers[iRenderer]->SetLayer(iRenderer + 1);
  }
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMLayerManager::UpdateRendererCamera()
{
  // Set the camera for the managed renderers
  // Layer 0 is unmanaged and its camera is left unchanged by the layer manager
  // Pipelines with no explicit camera map to the default camera
  // Pipelines with custom camera are grouped and use their cameras
  this->CameraRendererMap.clear();

  int iRenderer = -1;
  for (const auto& pair : this->PipelineLayers)
  {
    if (iRenderer >= 0 && iRenderer < this->GetNumberOfRenderers())
    {
      auto camera = this->GetCameraForLayer(pair.first, pair.second);
      this->Renderers[iRenderer]->SetActiveCamera(camera);
      this->CameraRendererMap[GetCameraId(camera)].emplace_back(this->Renderers[iRenderer]);
    }

    iRenderer++;
  }
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMLayerManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Number of layers: " << this->PipelineLayers.size() << std::endl;
  os << indent << "Number of renderers: " << this->Renderers.size() << std::endl;
  os << indent << "Number of cameras: " << this->CameraRendererMap.size() << std::endl;
  os << indent << "Render window: " << (this->RenderWindow ? "set" : "(none)") << std::endl;
  os << indent << "Default camera: " << (this->DefaultCamera ? "set" : "(none)") << std::endl;
}
