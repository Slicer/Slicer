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
    : Camera(camera)
    , InvokeModifiedEvent{ std::move(invokeModifiedEvent) }
  {
  }
  virtual ~CameraSynchronizeStrategy() = default;
  virtual void UpdateCamera() = 0;

protected:
  vtkSmartPointer<vtkCamera> Camera;
  vtkNew<vtkMRMLLayerDMObjectEventObserver> EventObserver;
  std::function<void()> InvokeModifiedEvent;
};

/// Default camera synchronization consists in updating the camera when the first renderer active camera is updated.
class DefaultCameraSynchronizeStrategy : public CameraSynchronizeStrategy
{
public:
  explicit DefaultCameraSynchronizeStrategy(const vtkSmartPointer<vtkCamera>& camera, vtkRenderer* renderer, std::function<void()> invokeModifiedEvent)
    : CameraSynchronizeStrategy(camera, std::move(invokeModifiedEvent))
    , Renderer(renderer)
  {
    this->EventObserver->SetUpdateCallback(
      [this](vtkObject* object)
      {
        if (object == this->Renderer)
        {
          this->ObserveActiveCamera();
        }
        this->UpdateCamera();
      });

    this->EventObserver->UpdateObserver(nullptr, this->Renderer, vtkCommand::ActiveCameraEvent);
    this->ObserveActiveCamera();
  }

  void UpdateCamera() override
  {
    if (!this->ObservedCamera)
    {
      return;
    }

    // Update camera and preserve clipping range
    double clippingRange[2];
    this->Camera->GetClippingRange(clippingRange);
    this->Camera->DeepCopy(this->ObservedCamera);
    this->Camera->SetClippingRange(clippingRange);
    this->InvokeModifiedEvent();
  }

private:
  void ObserveActiveCamera() { this->SetObservedCamera(this->Renderer ? this->Renderer->GetActiveCamera() : nullptr); }

  void SetObservedCamera(vtkCamera* camera)
  {
    if (this->ObservedCamera == camera)
    {
      return;
    }

    this->EventObserver->UpdateObserver(this->ObservedCamera, camera);
    this->ObservedCamera = camera;
  }

  vtkWeakPointer<vtkRenderer> Renderer;
  vtkWeakPointer<vtkCamera> ObservedCamera;
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
    , SliceNode{ sliceNode }
  {
    this->EventObserver->SetUpdateCallback(
      [this](vtkObject* object)
      {
        if (object == this->SliceNode)
        {
          this->UpdateCamera();
        }
      });
    this->EventObserver->UpdateObserver(nullptr, this->SliceNode);
  }

  void UpdateCamera() override
  {
    if (!this->SliceNode)
    {
      return;
    }

    // Compute view center
    vtkMatrix4x4* xyToRas = this->SliceNode->GetXYToRAS();
    std::array<double, 4> viewCenterXY = { 0.5 * this->SliceNode->GetDimensions()[0], 0.5 * this->SliceNode->GetDimensions()[1], 0.0, 1.0 };
    std::array<double, 4> viewCenterRAS = {};
    xyToRas->MultiplyPoint(viewCenterXY.data(), viewCenterRAS.data());

    // Current slice RAS coordinate is invalid (Slice was probably just created and not already displayed).
    // Avoid propagating NaN.
    if (std::isnan(viewCenterRAS[0]))
    {
      return;
    }

    // Parallel projection and scale
    this->Camera->ParallelProjectionOn();
    this->Camera->SetParallelScale(0.5 * this->SliceNode->GetFieldOfView()[1]);

    // Set focal point
    this->Camera->SetFocalPoint(viewCenterRAS.data());

    // View directions
    vtkMatrix4x4* sliceToRAS = this->SliceNode->GetSliceToRAS();

    std::array<double, 3> vRight = { sliceToRAS->GetElement(0, 0), sliceToRAS->GetElement(1, 0), sliceToRAS->GetElement(2, 0) };

    std::array<double, 3> vUp = { sliceToRAS->GetElement(0, 1), sliceToRAS->GetElement(1, 1), sliceToRAS->GetElement(2, 1) };
    this->Camera->SetViewUp(vUp.data());

    // Position
    double d = this->Camera->GetDistance();
    std::array<double, 3> normal{};
    vtkMath::Cross(vRight.data(), vUp.data(), normal.data());
    double position[3] = { viewCenterRAS[0] + normal[0] * d, viewCenterRAS[1] + normal[1] * d, viewCenterRAS[2] + normal[2] * d };
    this->Camera->SetPosition(position);
    this->InvokeModifiedEvent();
  }

private:
  vtkWeakPointer<vtkMRMLSliceNode> SliceNode;
};

vtkStandardNewMacro(vtkMRMLLayerDMCameraSynchronizer);

void vtkMRMLLayerDMCameraSynchronizer::SetViewNode(vtkMRMLAbstractViewNode* viewNode)
{
  if (this->ViewNode == viewNode)
  {
    return;
  }

  this->ViewNode = viewNode;
  this->UpdateStrategy();
}

void vtkMRMLLayerDMCameraSynchronizer::SetDefaultCamera(const vtkSmartPointer<vtkCamera>& camera)
{
  if (this->DefaultCamera == camera)
  {
    return;
  }
  this->DefaultCamera = camera;
  this->UpdateStrategy();
}

void vtkMRMLLayerDMCameraSynchronizer::SetRenderer(vtkRenderer* renderer)
{
  if (this->Renderer == renderer)
  {
    return;
  }
  this->Renderer = renderer;
  this->UpdateStrategy();
}

vtkMRMLLayerDMCameraSynchronizer::vtkMRMLLayerDMCameraSynchronizer() = default;

vtkMRMLLayerDMCameraSynchronizer::~vtkMRMLLayerDMCameraSynchronizer() = default;

void vtkMRMLLayerDMCameraSynchronizer::UpdateStrategy()
{
  if (!this->DefaultCamera || !this->Renderer)
  {
    this->SynchronizeStrategy = nullptr;
    return;
  }

  const auto invokeModifiedEvent = [this]
  {
    if (this->IsBlocked)
    {
      return;
    }
    this->Modified();
  };

  if (auto sliceNode = vtkMRMLSliceNode::SafeDownCast(this->ViewNode))
  {
    this->SynchronizeStrategy = std::make_unique<SliceViewCameraSynchronizeStrategy>(this->DefaultCamera, sliceNode, invokeModifiedEvent);
  }
  else
  {
    this->SynchronizeStrategy = std::make_unique<DefaultCameraSynchronizeStrategy>(this->DefaultCamera, this->Renderer, invokeModifiedEvent);
  }
  this->SynchronizeStrategy->UpdateCamera();
}

bool vtkMRMLLayerDMCameraSynchronizer::BlockModified(bool isBlocked)
{
  bool wasBlocked = this->IsBlocked;
  this->IsBlocked = isBlocked;
  return wasBlocked;
}
