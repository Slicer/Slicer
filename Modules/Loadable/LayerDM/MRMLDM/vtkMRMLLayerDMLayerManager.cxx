#include "vtkMRMLLayerDMLayerManager.h"

// Layer DM includes
#include "vtkMRMLLayerDMPipelineI.h"

// VTK includes
#include <vtkBoundingBox.h>
#include <vtkCamera.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>

vtkStandardNewMacro(vtkMRMLLayerDMLayerManager);

void vtkMRMLLayerDMLayerManager::AddPipeline(vtkMRMLLayerDMPipelineI* pipeline)
{
  if (!pipeline)
  {
    return;
  }

  auto key = this->GetPipelineLayerKey(pipeline);
  if (!this->ContainsLayerKey(key))
  {
    this->m_pipelineLayers[key] = {};
  }
  this->m_pipelineLayers[key].emplace(pipeline);
  this->UpdateLayers();
}

vtkMRMLLayerDMLayerManager::LayerKey vtkMRMLLayerDMLayerManager::GetPipelineLayerKey(vtkMRMLLayerDMPipelineI* pipeline)
{
  if (!pipeline)
  {
    return {};
  }
  return { pipeline->GetRenderOrder(), vtkMRMLLayerDMLayerManager::GetCameraId(pipeline->GetCustomCamera()) };
}

int vtkMRMLLayerDMLayerManager::GetNumberOfDistinctLayers() const
{
  return static_cast<int>(this->m_pipelineLayers.size());
}

int vtkMRMLLayerDMLayerManager::GetNumberOfManagedLayers() const
{
  return this->GetNumberOfDistinctLayers() - 1;
}

int vtkMRMLLayerDMLayerManager::GetNumberOfRenderers() const
{
  return static_cast<int>(this->m_renderers.size());
}

void vtkMRMLLayerDMLayerManager::RemovePipeline(vtkMRMLLayerDMPipelineI* pipeline)
{
  if (!pipeline)
  {
    return;
  }

  auto key = this->GetPipelineLayerKey(pipeline);
  if (!this->ContainsLayerKey(key))
  {
    return;
  }

  // Remove pipeline from its renderer
  this->RemovePipelineRenderer(pipeline);

  // Update the other pipeline layers if needed
  this->m_pipelineLayers[key].erase(pipeline);
  this->UpdateLayers();
}

void vtkMRMLLayerDMLayerManager::ResetCameraClippingRange() const
{
  // Reset first renderer clipping range
  if (auto defaultRenderer = this->GetDefaultRenderer())
  {
    defaultRenderer->ResetCameraClippingRange();
  }

  // Reset the managed renderers grouped by common cameras
  for (const auto& pair : m_cameraRendererMap)
  {
    this->ResetRenderersCameraClippingRange(pair.second, this->ComputeRenderersVisibleBounds(pair.second));
  }
}

void vtkMRMLLayerDMLayerManager::SetRenderWindow(vtkRenderWindow* renderWindow)
{
  if (this->m_renderWindow == renderWindow)
  {
    return;
  }

  this->RemoveAllLayers();
  this->m_renderWindow = renderWindow;
  this->UpdateLayers();
}

void vtkMRMLLayerDMLayerManager::SetDefaultCamera(const vtkSmartPointer<vtkCamera>& camera)
{
  if (this->m_defaultCamera == camera)
  {
    return;
  }
  this->m_defaultCamera = camera;
  this->UpdateLayers();
}

vtkMRMLLayerDMLayerManager::vtkMRMLLayerDMLayerManager()
  : m_emptyPipeline(vtkSmartPointer<vtkMRMLLayerDMPipelineI>::New())
{
  this->AddPipeline(this->m_emptyPipeline);
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
  return this->m_renderers[rendererIndex];
}

vtkRenderer* vtkMRMLLayerDMLayerManager::GetDefaultRenderer() const
{
  if (!this->m_renderWindow)
  {
    return nullptr;
  }
  return this->m_renderWindow->GetRenderers()->GetFirstRenderer();
}

void vtkMRMLLayerDMLayerManager::AddMissingLayers()
{
  while (this->GetNumberOfRenderers() < this->GetNumberOfManagedLayers())
  {
    // Managed renderers are displayed as overlays and should not catch any events.
    // Events handling is done using the DM mechanism.
    auto renderer = vtkSmartPointer<vtkRenderer>::New();
    renderer->InteractiveOff();
    this->m_renderWindow->AddRenderer(renderer);
    this->m_renderers.emplace_back(renderer);
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
  return this->m_pipelineLayers.find(key) != this->m_pipelineLayers.end();
}

std::uintptr_t vtkMRMLLayerDMLayerManager::GetCameraId(vtkCamera* camera)
{
  if (!camera)
  {
    return 0;
  }
  return reinterpret_cast<std::uintptr_t>(camera);
}

vtkCamera* vtkMRMLLayerDMLayerManager::GetCameraForLayer(const LayerKey& key, const std::set<vtkWeakPointer<vtkMRMLLayerDMPipelineI>>& pipelines) const
{
  auto cameraId = std::get<1>(key);
  if (cameraId == 0)
  {
    return this->m_defaultCamera;
  }

  for (const auto& pipeline : pipelines)
  {
    if (pipeline)
    {
      return pipeline->GetCustomCamera();
    }
  }

  return nullptr;
}

int vtkMRMLLayerDMLayerManager::GetKeyIndex(const LayerKey& key) const
{
  int index = 0;
  for (const auto& pair : m_pipelineLayers)
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
  for (const auto& renderer : m_renderers)
  {
    this->RemoveRenderer(renderer);
  }
  this->UpdateRenderWindowNumberOfLayers();
  this->m_renderers.clear();
}

void vtkMRMLLayerDMLayerManager::RemoveAllPipelineRenderers()
{
  // if the render window is null, notify pipelines
  for (const auto& [key, pipelines] : m_pipelineLayers)
  {
    for (const auto& pipeline : pipelines)
    {
      this->RemovePipelineRenderer(pipeline);
    }
  }
}

void vtkMRMLLayerDMLayerManager::RemovePipelineRenderer(vtkMRMLLayerDMPipelineI* pipeline)
{
  if (pipeline)
  {
    pipeline->SetRenderer(nullptr);
  }
}

void vtkMRMLLayerDMLayerManager::RemoveOutdatedLayers()
{
  while (this->GetNumberOfRenderers() && (this->GetNumberOfRenderers() > this->GetNumberOfManagedLayers()))
  {
    this->RemoveRenderer(this->m_renderers[this->GetNumberOfRenderers() - 1]);
  }
}

void vtkMRMLLayerDMLayerManager::RemoveOutdatedPipelines()
{
  // Remove pipelines which have been garbage collected
  for (auto& [key, pipelines] : m_pipelineLayers)
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
      this->m_pipelineLayers.erase(key);
    }
  }
}

void vtkMRMLLayerDMLayerManager::RemoveRenderer(const vtkSmartPointer<vtkRenderer>& renderer)
{
  if (this->m_renderWindow && this->m_renderWindow->HasRenderer(renderer))
  {
    this->m_renderWindow->RemoveRenderer(renderer);
  }

  this->m_renderers.erase(std::find(this->m_renderers.begin(), this->m_renderers.end(), renderer));
}

void vtkMRMLLayerDMLayerManager::ResetRenderersCameraClippingRange(const std::set<vtkWeakPointer<vtkRenderer>>& renderers, const std::array<double, 6>& bounds)
{
  for (const auto& renderer : renderers)
  {
    if (!renderer)
    {
      continue;
    }
    renderer->ResetCameraClippingRange(bounds.data());
  }
}

void vtkMRMLLayerDMLayerManager::SynchronizePipelineRenderers()
{
  for (const auto& pair : m_pipelineLayers)
  {
    auto renderer = this->GetRendererMatchingKey(pair.first);
    for (const auto& pipeline : pair.second)
    {
      if (pipeline)
      {
        pipeline->SetRenderer(renderer);
      }
    }
  }
}

void vtkMRMLLayerDMLayerManager::UpdateRenderWindowNumberOfLayers() const
{
  if (!this->m_renderWindow)
  {
    return;
  }

  // Synchronize the render window number of layers with its actual number of renderers
  int numberOfRenderers = this->m_renderWindow->GetRenderers()->GetNumberOfItems();
  int iMax = 0;
  for (int iRenderer = 0; iRenderer < numberOfRenderers; iRenderer++)
  {
    if (auto renderer = vtkRenderer::SafeDownCast(this->m_renderWindow->GetRenderers()->GetItemAsObject(iRenderer)))
    {
      iMax = std::max(iMax, renderer->GetLayer());
    }
  }

  this->m_renderWindow->SetNumberOfLayers(iMax + 1);
}

void vtkMRMLLayerDMLayerManager::UpdateLayers()
{
  if (!this->m_renderWindow)
  {
    this->RemoveAllPipelineRenderers();
    return;
  }

  this->RemoveOutdatedPipelines();
  this->RemoveOutdatedLayers();
  this->AddMissingLayers();
  this->UpdateRenderWindowLayerOrdering();
  this->UpdateRendererCamera();
  this->SynchronizePipelineRenderers();
}

void vtkMRMLLayerDMLayerManager::UpdateRenderWindowLayerOrdering() const
{
  // Managed layers are always ordered from layer 1 to the number of managed renderers
  for (int iRenderer = 0; iRenderer < this->GetNumberOfRenderers(); iRenderer++)
  {
    this->m_renderers[iRenderer]->SetLayer(iRenderer + 1);
  }
  this->UpdateRenderWindowNumberOfLayers();
}

void vtkMRMLLayerDMLayerManager::UpdateRendererCamera()
{
  // Set the camera for the managed renderers
  // Layer 0 is unmanaged and its camera is left unchanged by the layer manager
  // Pipelines with no explicit camera map to the default camera
  // Pipelines with custom camera are grouped and use their cameras
  this->m_cameraRendererMap.clear();

  int iRenderer = -1;
  for (const auto& pair : m_pipelineLayers)
  {
    if (iRenderer >= 0 && iRenderer < this->GetNumberOfRenderers())
    {
      auto camera = this->GetCameraForLayer(pair.first, pair.second);
      this->m_renderers[iRenderer]->SetActiveCamera(camera);
      this->m_cameraRendererMap[camera].emplace(this->m_renderers[iRenderer]);
    }

    iRenderer++;
  }
}
