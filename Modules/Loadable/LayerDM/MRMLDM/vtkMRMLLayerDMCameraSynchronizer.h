#pragma once

#include "vtkSlicerLayerDMModuleMRMLDisplayableManagerExport.h"

// VTK includes
#include <vtkObject.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

// STL includes
#include <memory>

class vtkCamera;
class vtkRenderer;
class CameraSynchronizeStrategy;
class vtkMRMLAbstractViewNode;

/// \brief Class responsible for synchronizing the camera of the different display layers.
/// If a pipeline doesn't use a specific camera, its layer will be set the default camera.
/// The default camera is synchronized to the renderer 0 byt this class and its default implementation is to
/// copy the content of the main camera when the camera has changed.
///
/// For SliceViews, the class monitors modified events to set the default camera aligned with the Slice view
/// properties.
class VTK_SLICER_LAYERDM_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLLayerDMCameraSynchronizer : public vtkObject
{
public:
  static vtkMRMLLayerDMCameraSynchronizer* New();
  vtkTypeMacro(vtkMRMLLayerDMCameraSynchronizer, vtkObject);

  /// Set the view node for which the camera will be synchronized.
  void SetViewNode(vtkMRMLAbstractViewNode* viewNode);

  /// Set the default camera which will be synchronized by this class.
  void SetDefaultCamera(const vtkSmartPointer<vtkCamera>& camera);

  /// Set the default renderer used by the displayable manager.
  /// The renderer active camera will be monitored for change when applicable (for instance 3D views).
  void SetRenderer(vtkRenderer* renderer);

protected:
  vtkMRMLLayerDMCameraSynchronizer();
  ~vtkMRMLLayerDMCameraSynchronizer() override;

private:
  /// Reset the internal strategy given current view node.
  void UpdateStrategy();

  vtkSmartPointer<vtkCamera> m_defaultCamera;
  vtkWeakPointer<vtkRenderer> m_renderer;
  vtkWeakPointer<vtkMRMLAbstractViewNode> m_viewNode;
  std::unique_ptr<CameraSynchronizeStrategy> m_syncStrategy;
};