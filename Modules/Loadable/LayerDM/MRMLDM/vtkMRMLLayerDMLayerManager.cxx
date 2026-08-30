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

vtkStandardNewMacro(vtkMRMLLayerDMLayerManager);

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

void vtkMRMLLayerDMLayerManager::AddPipeline(vtkMRMLLayerDMPipeline* pipeline)
{
  if (!pipeline)
  {
    return;
  }

  this->Observer->UpdateObserver(nullptr, pipeline, vtkMRMLLayerDMPipeline::RenderGroupingModified);
  this->AddPipelineLayers(pipeline);
  this->UpdateLayers();
}

int vtkMRMLLayerDMLayerManager::GetNumberOfDistinctLayers() const
{
  return static_cast<int>(this->PipelineLayers.size());
}

int vtkMRMLLayerDMLayerManager::GetNumberOfManagedLayers() const
{
  return this->GetNumberOfDistinctLayers() - 1;
}

int vtkMRMLLayerDMLayerManager::GetNumberOfRenderers() const
{
  return static_cast<int>(this->Renderers.size());
}

void vtkMRMLLayerDMLayerManager::RemovePipelineLayers(vtkMRMLLayerDMPipeline* pipeline)
{
  for (auto& [key, pipelines] : this->PipelineLayers)
  {
    pipelines.erase(pipeline);
  }
}

void vtkMRMLLayerDMLayerManager::RemovePipeline(vtkMRMLLayerDMPipeline* pipeline)
{
  if (!pipeline)
  {
    return;
  }

  this->Observer->UpdateObserver(pipeline, nullptr);
  this->RemovePipelineRenderer(pipeline);
  this->RemovePipelineLayers(pipeline);
  this->UpdateLayers();
}

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

void vtkMRMLLayerDMLayerManager::SetDefaultCamera(const vtkSmartPointer<vtkCamera>& camera)
{
  if (this->DefaultCamera == camera)
  {
    return;
  }

  this->DefaultCamera = camera;
  this->UpdateLayers();
}

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
  this->AddPipeline(this->EmptyPipeline);
}

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

vtkRenderer* vtkMRMLLayerDMLayerManager::GetDefaultRenderer() const
{
  if (!this->RenderWindow)
  {
    return nullptr;
  }
  return this->RenderWindow->GetRenderers()->GetFirstRenderer();
}

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

std::array<double, 6> vtkMRMLLayerDMLayerManager::ComputeRenderersVisibleBounds(const std::set<vtkWeakPointer<vtkRenderer>>& renderers)
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

bool vtkMRMLLayerDMLayerManager::ContainsLayerKey(const LayerKey& key)
{
  return this->PipelineLayers.find(key) != this->PipelineLayers.end();
}

std::uintptr_t vtkMRMLLayerDMLayerManager::GetCameraId(vtkCamera* camera)
{
  if (!camera)
  {
    return 0;
  }
  return reinterpret_cast<std::uintptr_t>(camera);
}

vtkCamera* vtkMRMLLayerDMLayerManager::GetCameraForLayer(const LayerKey& key, const std::set<vtkWeakPointer<vtkMRMLLayerDMPipeline>>& pipelines) const
{
  if (const auto cameraId = std::get<1>(key); cameraId == 0)
  {
    return this->DefaultCamera;
  }

  for (const auto& pipeline : pipelines)
  {
    if (pipeline)
    {
      return pipeline->GetCustomCamera(std::get<0>(key));
    }
  }

  return nullptr;
}

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

void vtkMRMLLayerDMLayerManager::RemoveAllLayers()
{
  for (const auto& renderer : this->Renderers)
  {
    this->RemoveRenderer(renderer);
  }
  this->UpdateRenderWindowNumberOfLayers();
  this->Renderers.clear();
}

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

void vtkMRMLLayerDMLayerManager::RemovePipelineRenderer(vtkMRMLLayerDMPipeline* pipeline)
{
  if (pipeline)
  {
    pipeline->SetRenderers({}, {});
  }
}

void vtkMRMLLayerDMLayerManager::RemoveOutdatedLayers()
{
  while (this->GetNumberOfRenderers() && (this->GetNumberOfRenderers() > this->GetNumberOfManagedLayers()))
  {
    this->RemoveRenderer(this->Renderers[this->GetNumberOfRenderers() - 1]);
  }
}

void vtkMRMLLayerDMLayerManager::RemoveOutdatedPipelines()
{
  // Remove pipelines which have been garbage collected
  for (auto& [key, pipelines] : this->PipelineLayers)
  {
    for (const auto& pipeline : pipelines)
    {
      if (!pipeline)
      {
        pipelines.erase(pipeline);
      }
    }

    if (pipelines.empty())
    {
      this->PipelineLayers.erase(key);
    }
  }
}

void vtkMRMLLayerDMLayerManager::RemoveRenderer(const vtkSmartPointer<vtkRenderer>& renderer)
{
  if (this->RenderWindow && this->RenderWindow->HasRenderer(renderer))
  {
    this->RenderWindow->RemoveRenderer(renderer);
  }

  this->Renderers.erase(std::find(this->Renderers.begin(), this->Renderers.end(), renderer));
}

void vtkMRMLLayerDMLayerManager::ResetRenderersCameraClippingRange(const std::set<vtkWeakPointer<vtkRenderer>>& renderers, const std::array<double, 6>& bounds)
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
      if (pipeline)
      {
        pipelineRenderers[pipeline.GetPointer()].push_back(renderer);
        pipelineOrders[pipeline.GetPointer()].push_back(order);
      }
    }
  }

  for (auto& [pipeline, renderers] : pipelineRenderers)
  {
    pipeline->SetRenderers(renderers, pipelineOrders[pipeline]);
  }
}

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

void vtkMRMLLayerDMLayerManager::UpdateRendererLayerOrdering() const
{
  // Managed layers are always ordered from layer 1 to the number of managed renderers
  for (int iRenderer = 0; iRenderer < this->GetNumberOfRenderers(); iRenderer++)
  {
    this->Renderers[iRenderer]->SetLayer(iRenderer + 1);
  }
}

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
      this->CameraRendererMap[camera].emplace(this->Renderers[iRenderer]);
    }

    iRenderer++;
  }
}
