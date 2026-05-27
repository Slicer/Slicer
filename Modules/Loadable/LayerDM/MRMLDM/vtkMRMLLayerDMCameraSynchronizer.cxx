#include "vtkMRMLLayerDMCameraSynchronizer.h"

// Layer DM includes
#include "vtkMRMLLayerDMObjectEventObserver.h"

// Slicer includes
#include "vtkMRMLAbstractViewNode.h"
#include "vtkMRMLSliceNode.h"

// VTK includes
#include <vtkCamera.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>

// STL includes
#include <array>

/// \brief Abstract class for the camera strategies.
/// Implements only the reset camera clipping range logic for the layer cameras.
/// Other methods are expected to be implemented by deriving classes.
class CameraSynchronizeStrategy
{
public:
  explicit CameraSynchronizeStrategy(const vtkSmartPointer<vtkCamera>& camera, std::function<void()> invokeModifiedEvent)
    : m_camera(camera)
    , m_invokeModifiedEvent{ std::move(invokeModifiedEvent) }
  {
  }
  virtual ~CameraSynchronizeStrategy() = default;
  virtual void UpdateCamera() = 0;

protected:
  vtkSmartPointer<vtkCamera> m_camera;
  vtkNew<vtkMRMLLayerDMObjectEventObserver> m_eventObserver;
  std::function<void()> m_invokeModifiedEvent;
};

/// Default camera synchronization consists in updating the camera when the first renderer active camera is updated.
class DefaultCameraSynchronizeStrategy : public CameraSynchronizeStrategy
{
public:
  explicit DefaultCameraSynchronizeStrategy(const vtkSmartPointer<vtkCamera>& camera, vtkRenderer* renderer, std::function<void()> invokeModifiedEvent)
    : CameraSynchronizeStrategy(camera, std::move(invokeModifiedEvent))
    , m_renderer(renderer)
  {
    this->m_eventObserver->SetUpdateCallback(
      [this](vtkObject* object)
      {
        if (object == this->m_renderer)
        {
          this->ObserveActiveCamera();
        }
        this->UpdateCamera();
      });

    this->m_eventObserver->UpdateObserver(nullptr, this->m_renderer, vtkCommand::ActiveCameraEvent);
    this->ObserveActiveCamera();
  }

  void UpdateCamera() override
  {
    if (!this->m_observedCamera)
    {
      return;
    }

    // Update camera and preserve clipping range
    double clippingRange[2];
    this->m_camera->GetClippingRange(clippingRange);
    this->m_camera->DeepCopy(this->m_observedCamera);
    this->m_camera->SetClippingRange(clippingRange);
    this->m_invokeModifiedEvent();
  }

private:
  void ObserveActiveCamera() { this->SetObservedCamera(this->m_renderer ? this->m_renderer->GetActiveCamera() : nullptr); }

  void SetObservedCamera(vtkCamera* camera)
  {
    if (this->m_observedCamera == camera)
    {
      return;
    }

    this->m_eventObserver->UpdateObserver(this->m_observedCamera, camera);
    this->m_observedCamera = camera;
  }

  vtkWeakPointer<vtkRenderer> m_renderer;
  vtkWeakPointer<vtkCamera> m_observedCamera;
};

/// Synchronizes the default camera to the current slice node view configuration.
/// The Slice renderer 0 camera is not configured nor modified during camera changes.
/// All of its actors are set to render in 2D.
///
/// To simplify adding new pipelines, the sync adjusts the default camera to render in parallel projection in the correct
/// orientation with respect to the current node configuration.
///
/// Clipping range is configured to show all actors attached to the default camera.
/// If clipping is required, then it should be done inside the specific pipeline.
class SliceViewCameraSynchronizeStrategy : public CameraSynchronizeStrategy
{
public:
  explicit SliceViewCameraSynchronizeStrategy(const vtkSmartPointer<vtkCamera>& camera, vtkMRMLSliceNode* sliceNode, std::function<void()> invokeModifiedEvent)
    : CameraSynchronizeStrategy(camera, std::move(invokeModifiedEvent))
    , m_sliceNode{ sliceNode }
  {
    this->m_eventObserver->SetUpdateCallback(
      [this](vtkObject* object)
      {
        if (object == this->m_sliceNode)
        {
          this->UpdateCamera();
        }
      });
    this->m_eventObserver->UpdateObserver(nullptr, this->m_sliceNode);
  }

  void UpdateCamera() override
  {
    if (!this->m_sliceNode)
    {
      return;
    }

    // Compute view center
    vtkMatrix4x4* xyToRas = this->m_sliceNode->GetXYToRAS();
    std::array<double, 4> viewCenterXY = { 0.5 * this->m_sliceNode->GetDimensions()[0], 0.5 * this->m_sliceNode->GetDimensions()[1], 0.0, 1.0 };
    std::array<double, 4> viewCenterRAS = {};
    xyToRas->MultiplyPoint(viewCenterXY.data(), viewCenterRAS.data());

    // Current slice RAS coordinate is invalid (Slice was probably just created and not already displayed).
    // Avoid propagating NaN.
    if (std::isnan(viewCenterRAS[0]))
    {
      return;
    }

    // Parallel projection and scale
    this->m_camera->ParallelProjectionOn();
    this->m_camera->SetParallelScale(0.5 * this->m_sliceNode->GetFieldOfView()[1]);

    // Set focal point
    this->m_camera->SetFocalPoint(viewCenterRAS.data());

    // View directions
    vtkMatrix4x4* sliceToRAS = this->m_sliceNode->GetSliceToRAS();

    std::array<double, 3> vRight = { sliceToRAS->GetElement(0, 0), sliceToRAS->GetElement(1, 0), sliceToRAS->GetElement(2, 0) };

    std::array<double, 3> vUp = { sliceToRAS->GetElement(0, 1), sliceToRAS->GetElement(1, 1), sliceToRAS->GetElement(2, 1) };
    this->m_camera->SetViewUp(vUp.data());

    // Position
    double d = this->m_camera->GetDistance();
    std::array<double, 3> normal{};
    vtkMath::Cross(vRight.data(), vUp.data(), normal.data());
    double position[3] = { viewCenterRAS[0] + normal[0] * d, viewCenterRAS[1] + normal[1] * d, viewCenterRAS[2] + normal[2] * d };
    this->m_camera->SetPosition(position);
    this->m_invokeModifiedEvent();
  }

private:
  vtkWeakPointer<vtkMRMLSliceNode> m_sliceNode;
};

vtkStandardNewMacro(vtkMRMLLayerDMCameraSynchronizer);

void vtkMRMLLayerDMCameraSynchronizer::SetViewNode(vtkMRMLAbstractViewNode* viewNode)
{
  if (this->m_viewNode == viewNode)
  {
    return;
  }

  this->m_viewNode = viewNode;
  this->UpdateStrategy();
}

void vtkMRMLLayerDMCameraSynchronizer::SetDefaultCamera(const vtkSmartPointer<vtkCamera>& camera)
{
  if (this->m_defaultCamera == camera)
  {
    return;
  }
  this->m_defaultCamera = camera;
  this->UpdateStrategy();
}

void vtkMRMLLayerDMCameraSynchronizer::SetRenderer(vtkRenderer* renderer)
{
  if (this->m_renderer == renderer)
  {
    return;
  }
  this->m_renderer = renderer;
  this->UpdateStrategy();
}

vtkMRMLLayerDMCameraSynchronizer::vtkMRMLLayerDMCameraSynchronizer()
  : m_defaultCamera{ nullptr }
  , m_renderer{ nullptr }
  , m_viewNode{ nullptr }
  , m_syncStrategy{ nullptr }
{
}

vtkMRMLLayerDMCameraSynchronizer::~vtkMRMLLayerDMCameraSynchronizer() = default;

void vtkMRMLLayerDMCameraSynchronizer::UpdateStrategy()
{
  if (!this->m_defaultCamera || !this->m_renderer)
  {
    this->m_syncStrategy = nullptr;
    return;
  }

  const auto invokeModifiedEvent = [this]
  {
    if (this->m_isBlocked)
    {
      return;
    }
    this->Modified();
  };

  if (auto sliceNode = vtkMRMLSliceNode::SafeDownCast(this->m_viewNode))
  {
    this->m_syncStrategy = std::make_unique<SliceViewCameraSynchronizeStrategy>(this->m_defaultCamera, sliceNode, invokeModifiedEvent);
  }
  else
  {
    this->m_syncStrategy = std::make_unique<DefaultCameraSynchronizeStrategy>(this->m_defaultCamera, this->m_renderer, invokeModifiedEvent);
  }
  this->m_syncStrategy->UpdateCamera();
}

bool vtkMRMLLayerDMCameraSynchronizer::BlockModified(bool isBlocked)
{
  bool wasBlocked = this->m_isBlocked;
  m_isBlocked = isBlocked;
  return wasBlocked;
}
