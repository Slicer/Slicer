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

#ifndef __vtkMRMLLayerDMLayerManager_h
#define __vtkMRMLLayerDMLayerManager_h

#include "vtkSlicerLayerDMModuleMRMLDisplayableManagerExport.h"

#include "vtkMRMLLayerDMPipeline.h"

// VTK includes
#include <vtkObject.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

// STL includes
#include <array>
#include <cstdint>
#include <map>
#include <set>
#include <vector>

class vtkMRMLLayerDMPipeline;
class vtkRenderWindow;
class vtkRenderer;
class vtkCamera;
class vtkMRMLLayerDMObjectEventObserver;

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
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Adds the pipeline to the layers.
  /// May change an update of the layer ordering.
  /// Will trigger the SetRenderer call on the pipeline when it's added to its layer.
  void AddPipeline(vtkMRMLLayerDMPipeline* pipeline);

  int GetNumberOfDistinctLayers() const;
  int GetNumberOfManagedLayers() const;

  /// Returns the current number of managed renderers in the render window.
  int GetNumberOfRenderers() const;

  /// Removes the pipeline from the layers.
  /// May change the layer ordering if pipeline was the last one of its current renderer.
  void RemovePipeline(vtkMRMLLayerDMPipeline* pipeline);

  /// Iterates over the renderers and resets their clipping range to visible bounds
  void ResetCameraClippingRange() const;

  /// Changes the render window managed by the layer manager.
  /// Will trigger a removal of all managed layers and creation of new layers if the render window is not null.
  void SetRenderWindow(vtkRenderWindow* renderWindow);

  /// If the default camera has changed, update the layers with ne new camera
  void SetDefaultCamera(const vtkSmartPointer<vtkCamera>& camera);

protected:
  vtkMRMLLayerDMLayerManager();
  ~vtkMRMLLayerDMLayerManager() override;

private:
  vtkRenderer* GetRendererMatchingKey(const LayerKey& key);
  vtkRenderer* GetDefaultRenderer() const;

  void AddMissingLayers();
  static std::array<double, 6> ComputeRenderersVisibleBounds(const std::vector<vtkWeakPointer<vtkRenderer>>& renderers);
  bool ContainsLayerKey(const LayerKey& key);
  static std::uintptr_t GetCameraId(vtkCamera* camera);
  vtkCamera* GetCameraForLayer(const LayerKey& key, const std::set<vtkMRMLLayerDMPipeline*>& pipelines) const;
  int GetKeyIndex(const LayerKey& key) const;
  void RemoveAllLayers();
  void RemoveAllPipelineRenderers();
  static void RemovePipelineRenderer(vtkMRMLLayerDMPipeline* pipeline);
  void RemoveOutdatedLayers();
  void RemoveOutdatedPipelines();
  void RemoveRenderer(const vtkSmartPointer<vtkRenderer>& renderer);
  static void ResetRenderersCameraClippingRange(const std::vector<vtkWeakPointer<vtkRenderer>>& renderers, const std::array<double, 6>& bounds);
  void SynchronizePipelineRenderers();
  void UpdateRenderWindowNumberOfLayers() const;
  void UpdateLayers();
  void UpdateRendererLayerOrdering() const;
  void UpdateRendererCamera();

  bool AddPipelineLayers(vtkMRMLLayerDMPipeline* pipeline);
  void RemovePipelineLayers(vtkMRMLLayerDMPipeline* pipeline);

  /// Map of pipeline layers ordered by ascending <layer value, camera synchronization mode>
  ///
  /// The pipelines order the sets and are never dereferenced without being known to be alive: every pipeline
  /// is observed for its destruction and removed from the sets during that event, so the sets only ever
  /// contain live pipelines. A weak pointer must not be used as the element of an ordered container, as it
  /// nulls itself in place when its object is destroyed, silently changing the value of a live set element and
  /// breaking the ordering of the set.
  ///
  /// \sa vtkMRMLLayerDMObjectEventObserver::SetDeleteCallback
  std::map<LayerKey, std::set<vtkMRMLLayerDMPipeline*>> PipelineLayers;

  /// Pipeline observer listening for \sa vtkMRMLLayerDMPipeline::RenderGroupingModified events.
  vtkSmartPointer<vtkMRMLLayerDMObjectEventObserver> Observer;

  // Placeholder empty pipeline with target layer = 0 and camera sync to layer 0 for default renderer
  vtkSmartPointer<vtkMRMLLayerDMPipeline> EmptyPipeline;

  // Pointer to the current render window
  vtkWeakPointer<vtkRenderWindow> RenderWindow;

  // Pointer to the default camera
  vtkSmartPointer<vtkCamera> DefaultCamera;

  // Renderers managed by the layer manager
  std::vector<vtkSmartPointer<vtkRenderer>> Renderers;

  // Camera to renderer map
  /// Renderers grouped by the camera they are synchronized on.
  ///
  /// Keyed by camera id rather than by the camera itself: the cameras returned by the pipelines are not
  /// observed by this class, so a weak pointer key could null itself in place and break the ordering of the
  /// map. The id is only used to group renderers and is never dereferenced.
  ///
  /// \sa GetCameraId
  std::map<std::uintptr_t, std::vector<vtkWeakPointer<vtkRenderer>>> CameraRendererMap;
};

#endif
