#pragma once

#include "vtkSlicerLayerDMModuleMRMLDisplayableManagerExport.h"

#include "vtkMRMLLayerDMPipelineI.h"

// VTK includes
#include <vtkObject.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

// STL includes
#include <array>
#include <cstdint>
#include <map>
#include <set>

class vtkMRMLLayerDMPipelineI;
class vtkRenderWindow;
class vtkRenderer;
class vtkCamera;

/// \brief Responsible for adding and removing renderer layers to a vtkRenderWindow depending on the
/// display pipeline preferred render order value.
///
/// Display pipeline order values are arbitrary unsigned int. This class will create and set the actual
/// Renderer layers so that their ordering is consistent with expected VTK behavior.
///
/// Renderer 0 is left unchanged by the class and is expected to already exist in the render window.
///
/// When pipelines are added / removed, renderers are created or deleted, and renderer layers are optimized
/// depending on the pipelines' preferred render order number.
/// Order number is read-only during update and is expected to be static per pipeline.
class VTK_SLICER_LAYERDM_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLLayerDMLayerManager : public vtkObject
{
public:
  using LayerKey = std::tuple<unsigned int, std::uintptr_t>;

  static vtkMRMLLayerDMLayerManager* New();
  vtkTypeMacro(vtkMRMLLayerDMLayerManager, vtkObject);

  void AddPipeline(vtkMRMLLayerDMPipelineI* pipeline);
  static LayerKey GetPipelineLayerKey(vtkMRMLLayerDMPipelineI* pipeline);
  int GetNumberOfDistinctLayers() const;
  int GetNumberOfManagedLayers() const;
  int GetNumberOfRenderers() const;
  void RemovePipeline(vtkMRMLLayerDMPipelineI* pipeline);
  void ResetCameraClippingRange() const;
  void SetRenderWindow(vtkRenderWindow* renderWindow);
  void SetDefaultCamera(const vtkSmartPointer<vtkCamera>& camera);

protected:
  vtkMRMLLayerDMLayerManager();
  ~vtkMRMLLayerDMLayerManager() override = default;

private:
  vtkRenderer* GetRendererMatchingKey(const LayerKey& key);
  vtkRenderer* GetDefaultRenderer() const;

  void AddMissingLayers();
  static std::array<double, 6> ComputeRenderersVisibleBounds(const std::set<vtkWeakPointer<vtkRenderer>>& renderers);
  bool ContainsLayerKey(const LayerKey& key);
  static std::uintptr_t GetCameraId(vtkCamera* camera);
  vtkCamera* GetCameraForLayer(const LayerKey& key, const std::set<vtkWeakPointer<vtkMRMLLayerDMPipelineI>>& pipelines) const;
  int GetKeyIndex(const LayerKey& key) const;
  void RemoveAllLayers();
  void RemoveAllPipelineRenderers();
  void RemoveOutdatedLayers();
  void RemoveOutdatedPipelines();
  void RemoveRenderer(const vtkSmartPointer<vtkRenderer>& renderer);
  static void ResetRenderersCameraClippingRange(const std::set<vtkWeakPointer<vtkRenderer>>& renderers, const std::array<double, 6>& bounds);
  void SynchronizePipelineRenderers();
  void UpdateRenderWindowNumberOfLayers() const;
  void UpdateLayers();
  void UpdateRenderWindowLayerOrdering() const;
  void UpdateRendererCamera();

  // Map of pipeline layers ordered by ascending <layer value, camera synchronization mode>
  std::map<LayerKey, std::set<vtkWeakPointer<vtkMRMLLayerDMPipelineI>>> m_pipelineLayers;

  // Placeholder empty pipeline with target layer = 0 and camera sync to layer 0 for default renderer
  vtkSmartPointer<vtkMRMLLayerDMPipelineI> m_emptyPipeline;

  // Pointer to the current render window
  vtkWeakPointer<vtkRenderWindow> m_renderWindow;

  // Pointer to the default camera
  vtkSmartPointer<vtkCamera> m_defaultCamera;

  // Renderers managed by the layer manager
  std::vector<vtkSmartPointer<vtkRenderer>> m_renderers;

  // Camera to renderer map
  std::map<vtkWeakPointer<vtkCamera>, std::set<vtkWeakPointer<vtkRenderer>>> m_cameraRendererMap;
};
