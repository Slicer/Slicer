/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care
  and CANARIE.

==============================================================================*/

// Volume Rendering includes
#include "vtkMRMLVolumeRenderingDisplayableManager.h"

#include "vtkSlicerVolumeRenderingLogic.h"
#include "vtkMRMLCPURayCastVolumeRenderingDisplayNode.h"
#include "vtkMRMLGPURayCastVolumeRenderingDisplayNode.h"
#include "vtkMRMLMultiVolumeRenderingDisplayNode.h"

// MRML includes
#include "vtkMRMLAnnotationROINode.h"
#include "vtkMRMLFolderDisplayNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLViewNode.h"
#include "vtkMRMLVolumePropertyNode.h"
#include "vtkMRMLShaderPropertyNode.h"
#include "vtkEventBroker.h"

// VTK includes
#include <vtkVersion.h> // must precede reference to VTK_MAJOR_VERSION
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>
#include <vtkFixedPointVolumeRayCastMapper.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkInteractorStyle.h>
#include <vtkMatrix4x4.h>
#include <vtkPlane.h>
#include <vtkPlanes.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#if VTK_MAJOR_VERSION >= 9 || (VTK_MAJOR_VERSION >= 8 && VTK_MINOR_VERSION >= 2)
#include <vtkMultiVolume.h>
#endif
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkDoubleArray.h>
#include <vtkVolumePicker.h>

#include <vtkImageData.h> //TODO: Used for workaround. Remove when fixed
#include <vtkTrivialProducer.h> //TODO: Used for workaround. Remove when fixed
#include <vtkPiecewiseFunction.h> //TODO: Used for workaround. Remove when fixed

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLVolumeRenderingDisplayableManager);

//---------------------------------------------------------------------------
int vtkMRMLVolumeRenderingDisplayableManager::DefaultGPUMemorySize = 256;

//---------------------------------------------------------------------------
class vtkMRMLVolumeRenderingDisplayableManager::vtkInternal
{
public:
  vtkInternal(vtkMRMLVolumeRenderingDisplayableManager* external);
  ~vtkInternal();

  //-------------------------------------------------------------------------
  class Pipeline
  {
  public:
    Pipeline()
    {
      this->VolumeActor = vtkSmartPointer<vtkVolume>::New();
      this->IJKToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
    }
    virtual ~Pipeline()  = default;

    vtkWeakPointer<vtkMRMLVolumeRenderingDisplayNode> DisplayNode;
    vtkSmartPointer<vtkVolume> VolumeActor;
    vtkSmartPointer<vtkMatrix4x4> IJKToWorldMatrix;
  };

  //-------------------------------------------------------------------------
  class PipelineCPU : public Pipeline
  {
  public:
    PipelineCPU() : Pipeline()
    {
      this->RayCastMapperCPU = vtkSmartPointer<vtkFixedPointVolumeRayCastMapper>::New();
    }
    vtkSmartPointer<vtkFixedPointVolumeRayCastMapper> RayCastMapperCPU;
  };
  //-------------------------------------------------------------------------
  class PipelineGPU : public Pipeline
  {
  public:
    PipelineGPU() : Pipeline()
    {
      this->RayCastMapperGPU = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
    }
    vtkSmartPointer<vtkGPUVolumeRayCastMapper> RayCastMapperGPU;
  };
  //-------------------------------------------------------------------------
  class PipelineMultiVolume : public Pipeline
  {
  public:
    PipelineMultiVolume(int actorPortIndex) : Pipeline()
    {
      this->ActorPortIndex = actorPortIndex;
    }

    unsigned int ActorPortIndex;
  };

  //-------------------------------------------------------------------------
  typedef std::vector< Pipeline* > PipelineListType;
  PipelineListType DisplayPipelines;

  std::vector< vtkWeakPointer<vtkMRMLVolumeNode> > ObservedVolumeNodes;

  vtkVolumeMapper* GetVolumeMapper(vtkMRMLVolumeRenderingDisplayNode* displayNode);

  // Volumes
  void AddVolumeNode(vtkMRMLVolumeNode* displayableNode);
  void RemoveVolumeNode(vtkMRMLVolumeNode* displayableNode);

  // Transforms
  // Return with true if pipelines may have changed.
  // If node==nullptr then all pipelines are updated.
  bool UpdatePipelineTransforms(vtkMRMLVolumeNode *node);
  bool GetVolumeTransformToWorld(vtkMRMLVolumeNode* node, vtkMatrix4x4* ijkToWorldMatrix);

  // ROIs
  void UpdatePipelineROIs(vtkMRMLVolumeRenderingDisplayNode* displayNode, const Pipeline* pipeline);

  // Display Nodes
  void AddDisplayNode(vtkMRMLVolumeRenderingDisplayNode* displayNode);
  void RemoveDisplayNode(vtkMRMLVolumeRenderingDisplayNode* displayNode);
  PipelineListType::iterator RemovePipelineIt(PipelineListType::iterator pipelineIt);
  void UpdateDisplayNode(vtkMRMLVolumeRenderingDisplayNode* displayNode);
  void UpdateDisplayNodePipeline(vtkMRMLVolumeRenderingDisplayNode* displayNode, const Pipeline* pipeline);

  double GetFramerate();
  vtkIdType GetMaxMemoryInBytes(vtkMRMLVolumeRenderingDisplayNode* displayNode);
  void UpdateDesiredUpdateRate(vtkMRMLVolumeRenderingDisplayNode* displayNode);

  // Observations
  void AddObservations(vtkMRMLVolumeNode* node);
  void RemoveObservations(vtkMRMLVolumeNode* node);
  bool IsNodeObserved(vtkMRMLVolumeNode* node);

  // Helper functions
  void RemoveOrphanPipelines();
  PipelineListType::iterator GetPipelineIt(vtkMRMLVolumeRenderingDisplayNode* displayNode);
  Pipeline* GetPipeline(vtkMRMLVolumeRenderingDisplayNode* displayNode);
  bool IsVisible(vtkMRMLVolumeRenderingDisplayNode* displayNode);
  bool UseDisplayNode(vtkMRMLVolumeRenderingDisplayNode* displayNode);
  bool UseDisplayableNode(vtkMRMLVolumeNode* node);
  void ClearDisplayableNodes();
  /// Calculate minimum sample distance as minimum of that for shown volumes, and set it to multi-volume mapper
  void UpdateMultiVolumeMapperSampleDistance();
  int GetNextAvailableMultiVolumeActorPortIndex();

  void FindPickedDisplayNodeFromVolumeActor(vtkVolume* volume);

public:
  vtkMRMLVolumeRenderingDisplayableManager* External;

  /// Flag indicating whether adding volume node is in progress
  bool AddingVolumeNode;

  /// Original desired update rate of renderer. Restored when volume hidden
  double OriginalDesiredUpdateRate;

  /// Observed events for the display nodes
  vtkIntArray* DisplayObservedEvents;

  /// When interaction is >0, we are in interactive mode (low level of detail)
  int Interaction;

  /// Picker of volume in renderer
  vtkSmartPointer<vtkVolumePicker> VolumePicker;

  /// Last picked volume rendering display node ID
  std::string PickedNodeID;

private:
#if VTK_MAJOR_VERSION >= 9 || (VTK_MAJOR_VERSION >= 8 && VTK_MINOR_VERSION >= 2)
  /// Multi-volume actor using a common mapper for rendering the multiple volumes
  vtkSmartPointer<vtkMultiVolume> MultiVolumeActor;
  /// Common GPU mapper for the multi-volume actor.
  /// Note: vtkMultiVolume only supports the GPU raycast mapper
  vtkSmartPointer<vtkGPUVolumeRayCastMapper> MultiVolumeMapper;
#endif

  friend class vtkMRMLVolumeRenderingDisplayableManager;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::vtkInternal(vtkMRMLVolumeRenderingDisplayableManager* external)
: External(external)
, AddingVolumeNode(false)
, OriginalDesiredUpdateRate(0.0) // 0 fps is a special value that means it hasn't been set
, Interaction(0)
, PickedNodeID("")
{
#if VTK_MAJOR_VERSION >= 9 || (VTK_MAJOR_VERSION >= 8 && VTK_MINOR_VERSION >= 2)
  this->MultiVolumeActor = vtkSmartPointer<vtkMultiVolume>::New();
  this->MultiVolumeMapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();

  // Set GPU mapper to the multi-volume actor. Both objects are only used for multi-volume GPU ray casting.
  // vtkMultiVolume works differently than the other two rendering modes, in the sense that those use a
  // separate mapper instance for each volume. Instead, vtkMultiVolume operates like this:
  //
  //                             <-- vtkVolume #1 (transfer functions, transform, etc.)
  // Renderer <-- vtkMultiVolume <-- vtkVolume #2
  //                    ^        <-- vtkVolume #3
  //                    |
  //         vtkGPUVolumeRayCastMapper
  //         ^          ^            ^
  //         |          |            |
  //     InputCon1  InputCon2   InputCon3
  //  (vtkImageData)
  //
  this->MultiVolumeActor->SetMapper(this->MultiVolumeMapper);
#endif

  this->DisplayObservedEvents = vtkIntArray::New();
  this->DisplayObservedEvents->InsertNextValue(vtkCommand::StartEvent);
  this->DisplayObservedEvents->InsertNextValue(vtkCommand::EndEvent);
  this->DisplayObservedEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  this->DisplayObservedEvents->InsertNextValue(vtkCommand::StartInteractionEvent);
  this->DisplayObservedEvents->InsertNextValue(vtkCommand::InteractionEvent);
  this->DisplayObservedEvents->InsertNextValue(vtkCommand::EndInteractionEvent);

  this->VolumePicker = vtkSmartPointer<vtkVolumePicker>::New();
  this->VolumePicker->SetTolerance(0.005);
}

//---------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::~vtkInternal()
{
  this->ClearDisplayableNodes();

  if (this->DisplayObservedEvents)
    {
    this->DisplayObservedEvents->Delete();
    this->DisplayObservedEvents = nullptr;
    }
}

//---------------------------------------------------------------------------
vtkVolumeMapper* vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::GetVolumeMapper(vtkMRMLVolumeRenderingDisplayNode* displayNode)
{
  if (!displayNode)
    {
    return nullptr;
    }
  if ( displayNode->IsA("vtkMRMLCPURayCastVolumeRenderingDisplayNode")
    || displayNode->IsA("vtkMRMLGPURayCastVolumeRenderingDisplayNode") )
    {
    vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::Pipeline* pipeline = this->GetPipeline(displayNode);
    if (!pipeline)
      {
      vtkErrorWithObjectMacro(this->External, "GetVolumeMapper: Failed to find pipeline for display node with ID " << displayNode->GetID());
      return nullptr;
      }
    if (displayNode->IsA("vtkMRMLCPURayCastVolumeRenderingDisplayNode"))
      {
      const PipelineCPU* pipelineCpu = dynamic_cast<const PipelineCPU*>(pipeline);
      if (pipelineCpu)
        {
        return pipelineCpu->RayCastMapperCPU;
        }
      }
    else if (displayNode->IsA("vtkMRMLGPURayCastVolumeRenderingDisplayNode"))
      {
      const PipelineGPU* pipelineGpu = dynamic_cast<const PipelineGPU*>(pipeline);
      if (pipelineGpu)
        {
        return pipelineGpu->RayCastMapperGPU;
        }
      }
    }
#if VTK_MAJOR_VERSION >= 9 || (VTK_MAJOR_VERSION >= 8 && VTK_MINOR_VERSION >= 2)
  else if (displayNode->IsA("vtkMRMLMultiVolumeRenderingDisplayNode"))
    {
    return this->MultiVolumeMapper;
    }
#endif
  vtkErrorWithObjectMacro(this->External, "GetVolumeMapper: Unsupported display class " << displayNode->GetClassName());
  return nullptr;
};

//---------------------------------------------------------------------------
bool vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::UseDisplayNode(vtkMRMLVolumeRenderingDisplayNode* displayNode)
{
  // Allow volumes to appear only in designated viewers
  if (displayNode && !displayNode->IsDisplayableInView(this->External->GetMRMLViewNode()->GetID()))
    {
    return false;
    }

  // Check whether display node can be shown in this view
  vtkMRMLVolumeRenderingDisplayNode* volRenDispNode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(displayNode);
  if ( !volRenDispNode
    || !volRenDispNode->GetVolumeNodeID()
    || !volRenDispNode->GetROINodeID()
    || !volRenDispNode->GetVolumePropertyNodeID() )
    {
    return false;
    }

  return true;
}

//---------------------------------------------------------------------------
bool vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::IsVisible(vtkMRMLVolumeRenderingDisplayNode* displayNode)
{
  if (displayNode && displayNode->GetFolderDisplayOverrideAllowed())
    {
    vtkMRMLDisplayableNode* displayableNode = displayNode->GetDisplayableNode();
    if (!vtkMRMLFolderDisplayNode::GetHierarchyVisibility(displayableNode))
      {
      return false;
      }
    }

  return displayNode && displayNode->GetVisibility() && displayNode->GetVisibility3D()
    && displayNode->GetVisibility(this->External->GetMRMLViewNode()->GetID())
    && displayNode->GetOpacity() > 0;
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::RemoveOrphanPipelines()
{
  PipelineListType::iterator it;
  for (it = this->DisplayPipelines.begin(); it != this->DisplayPipelines.end();)
    {
    Pipeline* pipeline = *it;
    if (!pipeline->DisplayNode.GetPointer() || !pipeline->DisplayNode->GetDisplayableNode())
      {
      it = this->RemovePipelineIt(it);
      }
    else
      {
      ++it;
      }
    }
}

//---------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::PipelineListType::iterator
vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::GetPipelineIt(vtkMRMLVolumeRenderingDisplayNode* displayNode)
{
  PipelineListType::iterator it;
  for (it = this->DisplayPipelines.begin(); it != this->DisplayPipelines.end(); ++it)
    {
    if ((*it)->DisplayNode == displayNode)
      {
      break;
      }
    }
  return it;
}

//---------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::Pipeline*
vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::GetPipeline(vtkMRMLVolumeRenderingDisplayNode* displayNode)
{
  for (Pipeline* pipeline : this->DisplayPipelines)
  {
    if (pipeline->DisplayNode == displayNode)
    {
      return pipeline;
    }
  }
  return nullptr;
}

//---------------------------------------------------------------------------
int vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::GetNextAvailableMultiVolumeActorPortIndex()
{
  //TODO: Change back "port = 1" to to "port = 0" once the VTK issue https://gitlab.kitware.com/vtk/vtk/issues/17325 is fixed
  const int MAXIMUM_NUMBER_OF_MULTIVOLUME_ACTORS = 10;
  for (int port = 1; port < MAXIMUM_NUMBER_OF_MULTIVOLUME_ACTORS; port++)
    {
    // Find out if port is used
    bool portIsUsed = false;
    for (Pipeline* pipeline : this->DisplayPipelines)
      {
      PipelineMultiVolume* pipelineMulti = dynamic_cast<PipelineMultiVolume*>(pipeline);
      if (!pipelineMulti)
        {
        continue;
        }
      if (pipelineMulti->ActorPortIndex == port)
        {
        portIsUsed = true;
        break;
        }
      }
    // If not used then it is good, this is the next available port
    if (!portIsUsed)
      {
      return port;
      }
    }
  // No available port is found
  vtkErrorWithObjectMacro(this->External, "Maximum number of multivolumes (" << MAXIMUM_NUMBER_OF_MULTIVOLUME_ACTORS << ") reached.");
  return -1;
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::AddVolumeNode(vtkMRMLVolumeNode* node)
{
  if (this->AddingVolumeNode)
    {
    return;
    }
  // Check if node should be used
  if (!this->UseDisplayableNode(node))
    {
    return;
    }

  this->AddingVolumeNode = true;

  // Add Display Nodes
  int numDisplayNodes = node->GetNumberOfDisplayNodes();

  this->AddObservations(node);

  for (int i=0; i<numDisplayNodes; i++)
    {
    vtkMRMLVolumeRenderingDisplayNode *displayNode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(node->GetNthDisplayNode(i));
    if (this->UseDisplayNode(displayNode))
      {
      this->AddDisplayNode(displayNode);
      }
    }
  this->AddingVolumeNode = false;
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::RemoveVolumeNode(vtkMRMLVolumeNode* node)
{
  if (!node)
    {
    return;
    }
  for (int displayNodeIndex = 0; displayNodeIndex < node->GetNumberOfDisplayNodes(); ++displayNodeIndex)
    {
    this->RemoveDisplayNode(vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(node->GetNthDisplayNode(displayNodeIndex)));
    }
  this->RemoveObservations(node);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::AddDisplayNode(vtkMRMLVolumeRenderingDisplayNode* displayNode)
{
  if (!displayNode)
    {
    return;
    }
  vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(displayNode->GetDisplayableNode());
  if (!volumeNode)
    {
    return;
    }

  // Do not add the display node if it is already associated with a pipeline object.
  // This happens when a node already associated with a display node is copied into another
  // (using vtkMRMLNode::Copy()) and is added to the scene afterward.
  // Related issue are #3428 and #2608
  if (this->GetPipeline(displayNode))
    {
    return;
    }

  if (displayNode->IsA("vtkMRMLCPURayCastVolumeRenderingDisplayNode"))
    {
    PipelineCPU* pipelineCpu = new PipelineCPU();
    pipelineCpu->DisplayNode = displayNode;
    // Set volume to the mapper
    // Reconnection is expensive operation, therefore only do it if needed
    if (pipelineCpu->RayCastMapperCPU->GetInputConnection(0, 0) != volumeNode->GetImageDataConnection())
      {
      pipelineCpu->RayCastMapperCPU->SetInputConnection(0, volumeNode->GetImageDataConnection());
      }
    // Add volume actor to renderer and local cache
    this->External->GetRenderer()->AddVolume(pipelineCpu->VolumeActor);
    // Add pipeline
    this->DisplayPipelines.push_back(pipelineCpu);
    }
  else if (displayNode->IsA("vtkMRMLGPURayCastVolumeRenderingDisplayNode"))
    {
    PipelineGPU* pipelineGpu = new PipelineGPU();
    pipelineGpu->DisplayNode = displayNode;
    // Set volume to the mapper
    // Reconnection is expensive operation, therefore only do it if needed
    if (pipelineGpu->RayCastMapperGPU->GetInputConnection(0, 0) != volumeNode->GetImageDataConnection())
      {
      pipelineGpu->RayCastMapperGPU->SetInputConnection(0, volumeNode->GetImageDataConnection());
      }
    // Add volume actor to renderer and local cache
    this->External->GetRenderer()->AddVolume(pipelineGpu->VolumeActor);
    // Add pipeline
    this->DisplayPipelines.push_back(pipelineGpu);
    }
#if VTK_MAJOR_VERSION >= 9 || (VTK_MAJOR_VERSION >= 8 && VTK_MINOR_VERSION >= 2)
  else if (displayNode->IsA("vtkMRMLMultiVolumeRenderingDisplayNode"))
    {
    int actorPortIndex = this->GetNextAvailableMultiVolumeActorPortIndex();
    if (actorPortIndex < 0)
      {
      vtkErrorWithObjectMacro(this->External, "AddDisplayNode: Cannot add Cannot add volume " << volumeNode->GetName() << "to multi-volume renderer");
      return;
      }
    PipelineMultiVolume* pipelineMulti = new PipelineMultiVolume(actorPortIndex);
    pipelineMulti->DisplayNode = displayNode;
    // Create a dummy volume for port zero if this is the first volume. Necessary because the first transform is ignored,
    // see https://gitlab.kitware.com/vtk/vtk/issues/17325
    //TODO: Remove this workaround when the issue is fixed in VTK
    double* multiVolumeBounds = this->MultiVolumeActor->GetBounds();
    if (multiVolumeBounds[0] > multiVolumeBounds[1]) // Prevent error that GetVolume throws if volume is null (TODO: need GetNumberOfVolumes)
      {
      vtkNew<vtkImageData> dummyImage;
      dummyImage->SetExtent(0,1,0,1,0,1);
      dummyImage->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
      dummyImage->SetScalarComponentFromDouble(0,0,0,0, 0.0);
      vtkNew<vtkTrivialProducer> dummyTrivialProducer;
      dummyTrivialProducer->SetOutput(dummyImage);
      this->MultiVolumeMapper->SetInputConnection(0, dummyTrivialProducer->GetOutputPort());

      vtkNew<vtkPiecewiseFunction> dummyOpacity;
      dummyOpacity->AddPoint(0.0, 0.0);
      dummyOpacity->AddPoint(1.0, 0.0);
      vtkNew<vtkVolumeProperty> dummyVolumeProperty;
      dummyVolumeProperty->SetScalarOpacity(dummyOpacity);
      vtkNew<vtkVolume> dummyVolumeActor;
      dummyVolumeActor->SetProperty(dummyVolumeProperty);
      this->MultiVolumeActor->SetVolume(dummyVolumeActor, 0);
      }
    //TODO: Uncomment when https://gitlab.kitware.com/vtk/vtk/issues/17302 is fixed in VTK
    // Set image data to mapper
    //this->MultiVolumeMapper->SetInputConnection(pipelineMulti->ActorPortIndex, volumeNode->GetImageDataConnection());
    // Add volume to multi-volume actor
    //this->MultiVolumeActor->SetVolume(pipelineMulti->VolumeActor, pipelineMulti->ActorPortIndex);
    // Make sure common actor is added to renderer and local cache
    this->External->GetRenderer()->AddVolume(this->MultiVolumeActor);
    // Add pipeline
    this->DisplayPipelines.push_back(pipelineMulti);
    // Update sample distance considering the new volume
    this->UpdateMultiVolumeMapperSampleDistance();
    }
#endif

  this->External->GetMRMLNodesObserverManager()->AddObjectEvents(displayNode, this->DisplayObservedEvents);

  // Update cached matrix. Calls UpdateDisplayNodePipeline
  this->UpdatePipelineTransforms(volumeNode);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::RemoveDisplayNode(vtkMRMLVolumeRenderingDisplayNode* displayNode)
{
  if (!displayNode)
    {
    return;
    }
  
  PipelineListType::iterator pipelineIt = this->GetPipelineIt(displayNode);
  if (pipelineIt == this->DisplayPipelines.end())
    {
    return;
    }
  this->RemovePipelineIt(pipelineIt);
}

//---------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::PipelineListType::iterator
vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::RemovePipelineIt(PipelineListType::iterator pipelineIt)
{
  if (pipelineIt == this->DisplayPipelines.end())
    {
    // already removed
    return this->DisplayPipelines.end();
    }

  Pipeline* pipeline = *pipelineIt;

  if (pipeline)
  {
    if (pipeline->VolumeActor)
      {
      this->External->GetRenderer()->RemoveVolume(pipeline->VolumeActor);
      }

#if VTK_MAJOR_VERSION >= 9 || (VTK_MAJOR_VERSION >= 8 && VTK_MINOR_VERSION >= 2)
    PipelineMultiVolume* pipelineMulti = dynamic_cast<PipelineMultiVolume*>(pipeline);
    if (pipelineMulti)
      {
      // Remove volume actor from multi-volume actor collection
      this->MultiVolumeMapper->RemoveInputConnection(pipelineMulti->ActorPortIndex, 0);
      this->MultiVolumeActor->RemoveVolume(pipelineMulti->ActorPortIndex);

      // Remove common actor from renderer and local cache if the last volume have been removed
      bool foundMultiVolumeActor = false;
      for (Pipeline* pipeline : this->DisplayPipelines)
        {
        PipelineMultiVolume* otherPipelineMulti = dynamic_cast<PipelineMultiVolume*>(pipeline);
        if (otherPipelineMulti == pipelineMulti)
          {
          // this is the current pipeline, which will be deleted in a moment
          // so it does not count as a pipeline that still uses multivolume actor
          continue;
          }
        if (otherPipelineMulti)
          {
          foundMultiVolumeActor = true;
          break;
          }
        }
      if (!foundMultiVolumeActor)
        {
        this->External->GetRenderer()->RemoveVolume(this->MultiVolumeActor);
        }
      }
#endif

    delete pipeline;
    }

  return this->DisplayPipelines.erase(pipelineIt);
}

//---------------------------------------------------------------------------
bool vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::UpdatePipelineTransforms(vtkMRMLVolumeNode* volumeNode)
{
  // Update the pipeline for all tracked DisplayableNode
  bool pipelineModified = false;
  for (auto pipeline : this->DisplayPipelines)
  {
    if (!pipeline->DisplayNode)
      {
      continue;
      }
    vtkMRMLVolumeNode* currentVolumeNode = vtkMRMLVolumeNode::SafeDownCast(pipeline->DisplayNode->GetDisplayableNode());
    if (currentVolumeNode == nullptr
      || (volumeNode != nullptr && currentVolumeNode != volumeNode))
      {
      continue;
      }
    this->UpdateDisplayNodePipeline(pipeline->DisplayNode, pipeline);

    // Calculate and apply transform matrix
    this->GetVolumeTransformToWorld(currentVolumeNode, pipeline->IJKToWorldMatrix);
    pipeline->VolumeActor->SetUserMatrix(pipeline->IJKToWorldMatrix.GetPointer());
    pipelineModified = true;
    }
  return pipelineModified;
}

//---------------------------------------------------------------------------
bool vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::GetVolumeTransformToWorld(
  vtkMRMLVolumeNode* volumeNode, vtkMatrix4x4* outputIjkToWorldMatrix)
{
  if (volumeNode == nullptr)
    {
    vtkErrorWithObjectMacro(this->External, "GetVolumeTransformToWorld: Invalid volume node");
    return false;
    }

  // Check if we have a transform node
  vtkMRMLTransformNode* transformNode = volumeNode->GetParentTransformNode();
  if (transformNode == nullptr)
    {
    volumeNode->GetIJKToRASMatrix(outputIjkToWorldMatrix);
    return true;
    }

  // IJK to RAS
  vtkMatrix4x4* ijkToRasMatrix = vtkMatrix4x4::New();
  volumeNode->GetIJKToRASMatrix(ijkToRasMatrix);

  // Parent transforms
  vtkMatrix4x4* nodeToWorldMatrix = vtkMatrix4x4::New();
  int success = transformNode->GetMatrixTransformToWorld(nodeToWorldMatrix);
  if (!success)
    {
    vtkWarningWithObjectMacro(this->External, "GetVolumeTransformToWorld: Non-linear parent transform found for volume node " << volumeNode->GetName());
    outputIjkToWorldMatrix->Identity();
    return false;
    }

  // Transform world to RAS
  vtkMatrix4x4::Multiply4x4(nodeToWorldMatrix, ijkToRasMatrix, outputIjkToWorldMatrix);
  outputIjkToWorldMatrix->Modified(); // Needed because Multiply4x4 does not invoke Modified

  ijkToRasMatrix->Delete();
  nodeToWorldMatrix->Delete();
  return true;
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::UpdateDisplayNode(vtkMRMLVolumeRenderingDisplayNode* displayNode)
{
  // If the display node already exists, just update. Otherwise, add as new node
  if (!displayNode)
    {
    return;
    }
  Pipeline* pipeline = this->GetPipeline(displayNode);
  if (pipeline)
    {
    this->UpdateDisplayNodePipeline(displayNode, pipeline);
    }
  else
    {
    this->AddVolumeNode( vtkMRMLVolumeNode::SafeDownCast(displayNode->GetDisplayableNode()) );
    }
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::UpdateDisplayNodePipeline(
  vtkMRMLVolumeRenderingDisplayNode* displayNode, const Pipeline* pipeline)
{
  if (!displayNode || !pipeline)
    {
    vtkErrorWithObjectMacro(this->External, "UpdateDisplayNodePipeline: Display node or pipeline is invalid");
    return;
    }
  vtkMRMLViewNode* viewNode = this->External->GetMRMLViewNode();
  if (!viewNode)
    {
    vtkErrorWithObjectMacro(this->External, "UpdateDisplayNodePipeline: Failed to access view node");
    return;
    }

  // Get volume node
  vtkMRMLVolumeNode* volumeNode = displayNode ? displayNode->GetVolumeNode() : nullptr;
  if (!volumeNode)
    {
    return;
    }

  bool displayNodeVisible = this->IsVisible(displayNode);

  // Set volume visibility, return if hidden
  pipeline->VolumeActor->SetVisibility(displayNodeVisible);
#if VTK_MAJOR_VERSION >= 9 || (VTK_MAJOR_VERSION >= 8 && VTK_MINOR_VERSION >= 2)
  // Workaround for lack of support for Visibility flag in vtkMultiVolume's vtkVolume members
  //TODO: Remove when https://gitlab.kitware.com/vtk/vtk/issues/17302 is fixed in VTK
  if (displayNode->IsA("vtkMRMLMultiVolumeRenderingDisplayNode"))
    {
    PipelineMultiVolume* pipelineMulti = dynamic_cast<PipelineMultiVolume*>(this->GetPipeline(displayNode));
    if (pipelineMulti)
      {
      if (displayNodeVisible)
        {
        this->MultiVolumeMapper->SetInputConnection(pipelineMulti->ActorPortIndex, volumeNode->GetImageDataConnection());
        this->MultiVolumeActor->SetVolume(pipelineMulti->VolumeActor, pipelineMulti->ActorPortIndex);
        }
      else
        {
        this->MultiVolumeMapper->RemoveInputConnection(pipelineMulti->ActorPortIndex, 0);
        this->MultiVolumeActor->RemoveVolume(pipelineMulti->ActorPortIndex);
        }

      // Workaround: if none of the volumes are visible then VTK renders a gray box,
      // so we need to hide the actor to prevent this.
      bool foundVisibleMultiVolumeActor = false;
      for (Pipeline* pipeline : this->DisplayPipelines)
        {
        PipelineMultiVolume* pipelineMulti = dynamic_cast<PipelineMultiVolume*>(pipeline);
        if (pipelineMulti && pipelineMulti->VolumeActor && pipelineMulti->VolumeActor->GetVisibility())
          {
          foundVisibleMultiVolumeActor = true;
          break;
          }
        }
      this->MultiVolumeActor->SetVisibility(foundVisibleMultiVolumeActor);
      }
    }
#endif
  if (!displayNodeVisible)
    {
    return;
    }

  // Get generic volume mapper
  vtkVolumeMapper* mapper = this->GetVolumeMapper(displayNode);
  if (!mapper)
    {
    vtkErrorWithObjectMacro(this->External, "UpdateDisplayNodePipeline: Unable to get volume mapper");
    return;
    }

  // Update specific volume mapper
  if (displayNode->IsA("vtkMRMLCPURayCastVolumeRenderingDisplayNode"))
    {
    vtkFixedPointVolumeRayCastMapper* cpuMapper = vtkFixedPointVolumeRayCastMapper::SafeDownCast(mapper);

    switch (viewNode->GetRaycastTechnique())
      {
      case vtkMRMLViewNode::Adaptive:
        cpuMapper->SetAutoAdjustSampleDistances(true);
        cpuMapper->SetLockSampleDistanceToInputSpacing(false);
        cpuMapper->SetImageSampleDistance(1.0);
        break;
      case vtkMRMLViewNode::Normal:
        cpuMapper->SetAutoAdjustSampleDistances(false);
        cpuMapper->SetLockSampleDistanceToInputSpacing(true);
        cpuMapper->SetImageSampleDistance(1.0);
        break;
      case vtkMRMLViewNode::Maximum:
        cpuMapper->SetAutoAdjustSampleDistances(false);
        cpuMapper->SetLockSampleDistanceToInputSpacing(false);
        cpuMapper->SetImageSampleDistance(0.5);
        break;
      }

    cpuMapper->SetSampleDistance(displayNode->GetSampleDistance());
    cpuMapper->SetInteractiveSampleDistance(displayNode->GetSampleDistance());

    // Make sure the correct mapper is set to the volume
    pipeline->VolumeActor->SetMapper(mapper);
    // Make sure the correct volume is set to the mapper
    // Reconnection is expensive operation, therefore only do it if needed
    if (mapper->GetInputConnection(0, 0) != volumeNode->GetImageDataConnection())
      {
      mapper->SetInputConnection(0, volumeNode->GetImageDataConnection());
      }
    }
  else if (displayNode->IsA("vtkMRMLGPURayCastVolumeRenderingDisplayNode"))
    {
    vtkMRMLGPURayCastVolumeRenderingDisplayNode* gpuDisplayNode =
      vtkMRMLGPURayCastVolumeRenderingDisplayNode::SafeDownCast(displayNode);
    vtkGPUVolumeRayCastMapper* gpuMapper = vtkGPUVolumeRayCastMapper::SafeDownCast(mapper);

    switch (viewNode->GetVolumeRenderingQuality())
      {
      case vtkMRMLViewNode::Adaptive:
        gpuMapper->SetAutoAdjustSampleDistances(true);
        gpuMapper->SetLockSampleDistanceToInputSpacing(false);
        gpuMapper->SetUseJittering(viewNode->GetVolumeRenderingSurfaceSmoothing());
        break;
      case vtkMRMLViewNode::Normal:
        gpuMapper->SetAutoAdjustSampleDistances(false);
        gpuMapper->SetLockSampleDistanceToInputSpacing(true);
        gpuMapper->SetUseJittering(viewNode->GetVolumeRenderingSurfaceSmoothing());
        break;
      case vtkMRMLViewNode::Maximum:
        gpuMapper->SetAutoAdjustSampleDistances(false);
        gpuMapper->SetLockSampleDistanceToInputSpacing(false);
        gpuMapper->SetUseJittering(viewNode->GetVolumeRenderingSurfaceSmoothing());
        break;
      }

    gpuMapper->SetSampleDistance(gpuDisplayNode->GetSampleDistance());
    gpuMapper->SetMaxMemoryInBytes(this->GetMaxMemoryInBytes(gpuDisplayNode));

    // Make sure the correct mapper is set to the volume
    pipeline->VolumeActor->SetMapper(mapper);
    // Make sure the correct volume is set to the mapper
    // Reconnection is expensive operation, therefore only do it if needed
    if (mapper->GetInputConnection(0, 0) != volumeNode->GetImageDataConnection())
      {
      mapper->SetInputConnection(0, volumeNode->GetImageDataConnection());
      }
    }
  else if (displayNode->IsA("vtkMRMLMultiVolumeRenderingDisplayNode"))
    {
    vtkMRMLMultiVolumeRenderingDisplayNode* multiDisplayNode =
      vtkMRMLMultiVolumeRenderingDisplayNode::SafeDownCast(displayNode);
    vtkGPUVolumeRayCastMapper* gpuMultiMapper = vtkGPUVolumeRayCastMapper::SafeDownCast(mapper);

    switch (viewNode->GetRaycastTechnique())
      {
      case vtkMRMLViewNode::Adaptive:
        gpuMultiMapper->SetAutoAdjustSampleDistances(true);
        gpuMultiMapper->SetLockSampleDistanceToInputSpacing(false);
        gpuMultiMapper->SetUseJittering(viewNode->GetVolumeRenderingSurfaceSmoothing());
        break;
      case vtkMRMLViewNode::Normal:
        gpuMultiMapper->SetAutoAdjustSampleDistances(false);
        gpuMultiMapper->SetLockSampleDistanceToInputSpacing(true);
        gpuMultiMapper->SetUseJittering(viewNode->GetVolumeRenderingSurfaceSmoothing());
        break;
      case vtkMRMLViewNode::Maximum:
        gpuMultiMapper->SetAutoAdjustSampleDistances(false);
        gpuMultiMapper->SetLockSampleDistanceToInputSpacing(false);
        gpuMultiMapper->SetUseJittering(viewNode->GetVolumeRenderingSurfaceSmoothing());
        break;
      }

    gpuMultiMapper->SetMaxMemoryInBytes(this->GetMaxMemoryInBytes(multiDisplayNode));
    }
  else
    {
    vtkErrorWithObjectMacro(this->External, "UpdateDisplayNodePipeline: Display node type " << displayNode->GetNodeTagName() << " is not supported");
    return;
    }

  // Set ray casting technique
  switch (viewNode->GetRaycastTechnique())
    {
    case vtkMRMLViewNode::MaximumIntensityProjection:
      mapper->SetBlendMode(vtkVolumeMapper::MAXIMUM_INTENSITY_BLEND);
      break;
    case vtkMRMLViewNode::MinimumIntensityProjection:
      mapper->SetBlendMode(vtkVolumeMapper::MINIMUM_INTENSITY_BLEND);
      break;
    case vtkMRMLViewNode::Composite:
    default:
      mapper->SetBlendMode(vtkVolumeMapper::COMPOSITE_BLEND);
      break;
    }

  // Update ROI clipping planes
  this->UpdatePipelineROIs(displayNode, pipeline);

  // Set volume property
  vtkVolumeProperty* volumeProperty = displayNode->GetVolumePropertyNode() ? displayNode->GetVolumePropertyNode()->GetVolumeProperty() : nullptr;
  pipeline->VolumeActor->SetProperty(volumeProperty);
  // vtkMultiVolume's GetProperty returns the volume property from the first volume actor, and that is used when assembling the
  // shader, so need to set the volume property to the the first volume actor (in this case dummy actor, see above TODO)
  if (this->MultiVolumeActor)
    {
    double* multiVolumeBounds = this->MultiVolumeActor->GetBounds();
    if (multiVolumeBounds[0] < multiVolumeBounds[1]) // Prevent error that GetVolume throws if volume is null (TODO: need GetNumberOfVolumes)
      {
      this->MultiVolumeActor->GetVolume(0)->SetProperty(volumeProperty);
      }
    }

  // Set shader property
  vtkShaderProperty* shaderProperty = displayNode->GetShaderPropertyNode() ? displayNode->GetShaderPropertyNode()->GetShaderProperty() : nullptr;
  pipeline->VolumeActor->SetShaderProperty(shaderProperty);

  pipeline->VolumeActor->SetPickable(volumeNode->GetSelectable());

  this->UpdateDesiredUpdateRate(displayNode);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::UpdatePipelineROIs(
  vtkMRMLVolumeRenderingDisplayNode* displayNode, const Pipeline* pipeline)
{
  if (!pipeline)
    {
    return;
    }
  vtkVolumeMapper* volumeMapper = this->GetVolumeMapper(displayNode);
  if (!volumeMapper)
    {
    vtkErrorWithObjectMacro(this->External, "UpdatePipelineROIs: Unable to get volume mapper");
    return;
    }
  if (!displayNode || displayNode->GetROINode() == nullptr || !displayNode->GetCroppingEnabled())
    {
    volumeMapper->RemoveAllClippingPlanes();
    return;
    }

  // Make sure the ROI node's inside out flag is on
  displayNode->GetROINode()->InsideOutOn();

  // Calculate and set clipping planes
  vtkNew<vtkPlanes> planes;
  displayNode->GetROINode()->GetTransformedPlanes(planes.GetPointer());
  volumeMapper->SetClippingPlanes(planes.GetPointer());
}

//---------------------------------------------------------------------------
double vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::GetFramerate()
{
  vtkMRMLViewNode* viewNode = this->External->GetMRMLViewNode();
  if (!viewNode)
    {
    vtkErrorWithObjectMacro(this->External, "GetFramerate: Failed to access view node");
    return 15.;
    }

  return ( viewNode->GetVolumeRenderingQuality() == vtkMRMLViewNode::Maximum ?
           0.0 : // special value meaning full quality
           std::max(viewNode->GetExpectedFPS(), 0.0001) );
}

//---------------------------------------------------------------------------
vtkIdType vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::GetMaxMemoryInBytes(
  vtkMRMLVolumeRenderingDisplayNode* displayNode)
{
  vtkMRMLViewNode* viewNode = this->External->GetMRMLViewNode();
  if (!viewNode)
    {
    vtkErrorWithObjectMacro(this->External, "GetFramerate: Failed to access view node");
    }

  int gpuMemorySizeMB = vtkMRMLVolumeRenderingDisplayableManager::DefaultGPUMemorySize;
  if (viewNode && viewNode->GetGPUMemorySize() > 0)
    {
    gpuMemorySizeMB = viewNode->GetGPUMemorySize();
    }

  // Special case: for GPU volume raycast mapper, round up to nearest 128MB
  if ( displayNode->IsA("vtkMRMLGPURayCastVolumeRenderingDisplayNode")
    || displayNode->IsA("vtkMRMLMultiVolumeRenderingDisplayNode") )
    {
    if (gpuMemorySizeMB < 128)
      {
      gpuMemorySizeMB = 128;
      }
    else
      {
      gpuMemorySizeMB = ((gpuMemorySizeMB - 1) / 128 + 1) * 128;
      }
    }

  vtkIdType gpuMemorySizeB = vtkIdType(gpuMemorySizeMB) * vtkIdType(1024 * 1024);
  return gpuMemorySizeB;
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::UpdateDesiredUpdateRate(vtkMRMLVolumeRenderingDisplayNode* displayNode)
{
  vtkRenderWindow* renderWindow = this->External->GetRenderer()->GetRenderWindow();
  vtkRenderWindowInteractor* renderWindowInteractor = renderWindow ? renderWindow->GetInteractor() : nullptr;
  if (!renderWindowInteractor)
    {
    return;
    }
  double fps = this->GetFramerate();
  if (displayNode->GetVisibility())
    {
    if (this->OriginalDesiredUpdateRate == 0.0)
      {
      // Save the DesiredUpdateRate before it is changed.
      // It will then be restored when the volume rendering is hidden
      this->OriginalDesiredUpdateRate = renderWindowInteractor->GetDesiredUpdateRate();
      }

    // VTK is overly cautious when estimates rendering speed.
    // This usually results in lower quality and higher frame rates than requested.
    // We update the the desired update rate of the renderer
    // to make the actual update more closely match the desired.
    // desired fps -> correctedFps
    //           1 -> 0.1
    //          10 -> 3.1
    //          50 -> 35
    //         100 -> 100
    double correctedFps = pow(fps, 1.5) / 10.0;
    renderWindowInteractor->SetDesiredUpdateRate(correctedFps);
    }
  else if (this->OriginalDesiredUpdateRate != 0.0)
    {
    // Restore the DesiredUpdateRate to its original value.
    renderWindowInteractor->SetDesiredUpdateRate(this->OriginalDesiredUpdateRate);
    this->OriginalDesiredUpdateRate = 0.0;
    }
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::AddObservations(vtkMRMLVolumeNode* node)
{
  for (auto observedVolumeNode : this->ObservedVolumeNodes)
    {
    if (observedVolumeNode == node)
      {
      // already observed
      return;
      }
    }
  this->ObservedVolumeNodes.push_back(node);
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  broker->AddObservation(node, vtkCommand::ModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand());
  broker->AddObservation(node, vtkMRMLDisplayableNode::TransformModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
  broker->AddObservation(node, vtkMRMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
  broker->AddObservation(node, vtkMRMLVolumeNode::ImageDataModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::RemoveObservations(vtkMRMLVolumeNode* node)
{
  for (auto volumeIt = this->ObservedVolumeNodes.begin(); volumeIt != this->ObservedVolumeNodes.end(); ++volumeIt)
    {
    if (volumeIt->GetPointer() == node)
      {
      this->ObservedVolumeNodes.erase(volumeIt);
      break;
      }
    }

  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  vtkEventBroker::ObservationVector observations;
  observations = broker->GetObservations(node, vtkCommand::ModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand());
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(node, vtkMRMLTransformableNode::TransformModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand());
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(node, vtkMRMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
  observations = broker->GetObservations(node, vtkMRMLVolumeNode::ImageDataModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
  broker->RemoveObservations(observations);
}

//---------------------------------------------------------------------------
bool vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::IsNodeObserved(vtkMRMLVolumeNode* node)
{
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  vtkCollection* observations = broker->GetObservationsForSubject(node);
  if (observations->GetNumberOfItems() > 0)
    {
    return true;
    }
  else
    {
    return false;
    }
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::ClearDisplayableNodes()
{
  auto observedVolumeNodesCopy = this->ObservedVolumeNodes;
  for (auto observedVolumeNode : observedVolumeNodesCopy)
    {
    this->RemoveVolumeNode(observedVolumeNode);
    }
  this->RemoveOrphanPipelines();
}

//---------------------------------------------------------------------------
bool vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::UseDisplayableNode(vtkMRMLVolumeNode* node)
{
  bool use = node && node->IsA("vtkMRMLVolumeNode");
  return use;
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::UpdateMultiVolumeMapperSampleDistance()
{
  if (this->AddingVolumeNode)
    {
    return;
    }

  double minimumSampleDistance = VTK_DOUBLE_MAX;
  for (Pipeline* pipeline : this->DisplayPipelines)
    {
    vtkMRMLMultiVolumeRenderingDisplayNode* multiDisplayNode =
      vtkMRMLMultiVolumeRenderingDisplayNode::SafeDownCast(pipeline->DisplayNode);
    if (!multiDisplayNode)
      {
      continue;
      }
    double currentSampleDistance = multiDisplayNode->GetSampleDistance();
    if (this->IsVisible(multiDisplayNode))
      {
      minimumSampleDistance = std::min(minimumSampleDistance, currentSampleDistance);
      }
    }

#if VTK_MAJOR_VERSION >= 9 || (VTK_MAJOR_VERSION >= 8 && VTK_MINOR_VERSION >= 2)
  vtkGPUVolumeRayCastMapper* gpuMultiMapper = vtkGPUVolumeRayCastMapper::SafeDownCast(this->MultiVolumeMapper);
  gpuMultiMapper->SetSampleDistance(minimumSampleDistance);
#endif
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::FindPickedDisplayNodeFromVolumeActor(vtkVolume* volume)
{
  this->PickedNodeID = "";
  if (!volume)
    {
    return;
    }
  for (Pipeline* pipeline : this->DisplayPipelines)
    {
    vtkMRMLVolumeRenderingDisplayNode* currentDisplayNode = pipeline->DisplayNode;
    vtkVolume* currentVolumeActor = pipeline->VolumeActor.GetPointer();
    if (currentVolumeActor == volume && currentDisplayNode)
      {
      vtkDebugWithObjectMacro(currentDisplayNode, "FindPickedDisplayNodeFromVolumeActor: Found matching volume, pick was on volume "
        << (currentDisplayNode->GetDisplayableNode() ? currentDisplayNode->GetDisplayableNode()->GetName() : "NULL"));
      this->PickedNodeID = currentDisplayNode->GetID();
      }
    }
}

//---------------------------------------------------------------------------
// vtkMRMLVolumeRenderingDisplayableManager methods

//---------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayableManager::vtkMRMLVolumeRenderingDisplayableManager()
{
  this->Internal = new vtkInternal(this);

  this->RemoveInteractorStyleObservableEvent(vtkCommand::LeftButtonPressEvent);
  this->RemoveInteractorStyleObservableEvent(vtkCommand::LeftButtonReleaseEvent);
  this->RemoveInteractorStyleObservableEvent(vtkCommand::RightButtonPressEvent);
  this->RemoveInteractorStyleObservableEvent(vtkCommand::RightButtonReleaseEvent);
  this->RemoveInteractorStyleObservableEvent(vtkCommand::MiddleButtonPressEvent);
  this->RemoveInteractorStyleObservableEvent(vtkCommand::MiddleButtonReleaseEvent);
  this->RemoveInteractorStyleObservableEvent(vtkCommand::MouseWheelBackwardEvent);
  this->RemoveInteractorStyleObservableEvent(vtkCommand::MouseWheelForwardEvent);
  this->RemoveInteractorStyleObservableEvent(vtkCommand::EnterEvent);
  this->RemoveInteractorStyleObservableEvent(vtkCommand::LeaveEvent);
  this->AddInteractorStyleObservableEvent(vtkCommand::StartInteractionEvent);
  this->AddInteractorStyleObservableEvent(vtkCommand::EndInteractionEvent);
}

//---------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayableManager::~vtkMRMLVolumeRenderingDisplayableManager()
{
  delete this->Internal;
  this->Internal=nullptr;
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::PrintSelf( ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf ( os, indent );
  os << indent << "vtkMRMLVolumeRenderingDisplayableManager: " << this->GetClassName() << "\n";
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::Create()
{
  Superclass::Create();
  this->ObserveGraphicalResourcesCreatedEvent();
  this->SetUpdateFromMRMLRequested(true);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::ObserveGraphicalResourcesCreatedEvent()
{
  vtkMRMLViewNode* viewNode = this->GetMRMLViewNode();
  if (viewNode == nullptr)
    {
    vtkErrorMacro("OnCreate: Failed to access view node");
    return;
    }
  if (!vtkIsObservedMRMLNodeEventMacro(viewNode, vtkMRMLViewNode::GraphicalResourcesCreatedEvent))
    {
    vtkNew<vtkIntArray> events;
    events->InsertNextValue(vtkMRMLViewNode::GraphicalResourcesCreatedEvent);
    vtkObserveMRMLNodeEventsMacro(viewNode, events.GetPointer());
    }
}

//---------------------------------------------------------------------------
int vtkMRMLVolumeRenderingDisplayableManager::ActiveInteractionModes()
{
  // Observe all the modes
  return ~0;
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::UnobserveMRMLScene()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::OnMRMLSceneStartClose()
{
  this->Internal->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::OnMRMLSceneEndClose()
{
  this->SetUpdateFromMRMLRequested(true);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::OnMRMLSceneEndBatchProcess()
{
  this->SetUpdateFromMRMLRequested(true);
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::OnMRMLSceneEndImport()
{
  // UpdateFromMRML will be executed only if there has been some actions
  // during the import that requested it (don't call
  // SetUpdateFromMRMLRequested(1) here, it should be done somewhere else
  // maybe in OnMRMLSceneNodeAddedEvent, OnMRMLSceneNodeRemovedEvent or
  // OnMRMLDisplayableModelNodeModifiedEvent).
  this->ObserveGraphicalResourcesCreatedEvent();
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::OnMRMLSceneEndRestore()
{
  // UpdateFromMRML will be executed only if there has been some actions
  // during the restoration that requested it (don't call
  // SetUpdateFromMRMLRequested(1) here, it should be done somewhere else
  // maybe in OnMRMLSceneNodeAddedEvent, OnMRMLSceneNodeRemovedEvent or
  // OnMRMLDisplayableModelNodeModifiedEvent).
  this->ObserveGraphicalResourcesCreatedEvent();
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (node->IsA("vtkMRMLVolumeNode"))
    {
    // Escape if the scene is being closed, imported or connected
    if (this->GetMRMLScene()->IsBatchProcessing())
      {
      this->SetUpdateFromMRMLRequested(true);
      return;
      }

    this->Internal->AddVolumeNode(vtkMRMLVolumeNode::SafeDownCast(node));
    this->RequestRender();
    }
  else if (node->IsA("vtkMRMLViewNode"))
    {
    vtkEventBroker* broker = vtkEventBroker::GetInstance();
    if (!broker->GetObservationExist(node, vtkCommand::ModifiedEvent, this, this->GetMRMLNodesCallbackCommand()))
      {
      broker->AddObservation(node, vtkCommand::ModifiedEvent, this, this->GetMRMLNodesCallbackCommand());
      }
    }
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  vtkMRMLVolumeNode* volumeNode = nullptr;
  vtkMRMLVolumeRenderingDisplayNode* displayNode = nullptr;

  if ( (volumeNode = vtkMRMLVolumeNode::SafeDownCast(node)) )
    {
    this->Internal->RemoveVolumeNode(volumeNode);
    this->RequestRender();
    }
  else if ( (displayNode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(node)) )
    {
    this->Internal->RemoveDisplayNode(displayNode);
    this->RequestRender();
    }
  else if (node->IsA("vtkMRMLViewNode"))
    {
    vtkEventBroker* broker = vtkEventBroker::GetInstance();
    vtkEventBroker::ObservationVector observations;
    observations = broker->GetObservations(node, vtkCommand::ModifiedEvent, this, this->GetMRMLNodesCallbackCommand());
    broker->RemoveObservations(observations);
    }

  this->Internal->RemoveOrphanPipelines();
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
{
  vtkMRMLScene* scene = this->GetMRMLScene();

  if (scene->IsBatchProcessing())
    {
    return;
    }

  //
  // Volume node events
  //
  vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(caller);
  if (volumeNode)
    {
    if (event == vtkMRMLDisplayableNode::DisplayModifiedEvent)
      {
      vtkMRMLNode* callDataNode = reinterpret_cast<vtkMRMLDisplayNode *> (callData);
      vtkMRMLVolumeRenderingDisplayNode* displayNode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(callDataNode);
      if (displayNode)
        {
        // Don't update if we are in an interaction mode
        // (vtkCommand::InteractionEvent will be fired, so we can ignore Modified events)
        if (this->Internal->Interaction == 0)
          {
          this->Internal->UpdateDisplayNode(displayNode);
          this->Internal->UpdateMultiVolumeMapperSampleDistance();
          this->RequestRender();
          }
        }
      }
    else if ( (event == vtkMRMLDisplayableNode::TransformModifiedEvent)
           || (event == vtkMRMLTransformableNode::TransformModifiedEvent)
           || (event == vtkCommand::ModifiedEvent))
      {
      // Parent transforms, volume origin, etc. changed, so we need to recompute transforms
      if (this->Internal->UpdatePipelineTransforms(volumeNode))
        {
        // ROI must not be reset here, as it would make it impossible to replay clipped volume sequences
        this->RequestRender();
        }
      }
    else if (event == vtkMRMLScalarVolumeNode::ImageDataModifiedEvent)
      {
      int numDisplayNodes = volumeNode->GetNumberOfDisplayNodes();
      for (int i=0; i<numDisplayNodes; i++)
        {
        vtkMRMLVolumeRenderingDisplayNode* displayNode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(volumeNode->GetNthDisplayNode(i));
        if (this->Internal->UseDisplayNode(displayNode))
          {
          this->Internal->UpdateDisplayNode(displayNode);
          this->Internal->UpdateMultiVolumeMapperSampleDistance();
          this->RequestRender();
          }
        }
      }
    }
  //
  // View node events
  //
  else if (caller->IsA("vtkMRMLViewNode"))
    {
    this->Internal->UpdatePipelineTransforms(nullptr);
    }
  //
  // Other events
  //
  else if (event == vtkCommand::StartEvent ||
           event == vtkCommand::StartInteractionEvent)
    {
    ++this->Internal->Interaction;
    // We request the interactive mode, we might have nested interactions
    // so we just start the mode for the first time.
    if (this->Internal->Interaction == 1)
      {
      vtkInteractorStyle* interactorStyle = vtkInteractorStyle::SafeDownCast(this->GetInteractor()->GetInteractorStyle());
      if (interactorStyle->GetState() == VTKIS_NONE)
        {
        interactorStyle->StartState(VTKIS_VOLUME_PROPS);
        }
      }
    }
  else if (event == vtkCommand::EndEvent ||
           event == vtkCommand::EndInteractionEvent)
    {
    --this->Internal->Interaction;
    if (this->Internal->Interaction == 0)
      {
      vtkInteractorStyle* interactorStyle = vtkInteractorStyle::SafeDownCast(this->GetInteractor()->GetInteractorStyle());
      if (interactorStyle->GetState() == VTKIS_VOLUME_PROPS)
        {
        interactorStyle->StopState();
        }
      if (caller->IsA("vtkMRMLVolumeRenderingDisplayNode"))
        {
        this->Internal->UpdateDisplayNode(vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(caller));
        }
      }
    }
  else if (event == vtkCommand::InteractionEvent)
    {
    if (caller->IsA("vtkMRMLVolumeRenderingDisplayNode"))
      {
      this->Internal->UpdateDisplayNode(vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(caller));
      this->RequestRender();
      }
    }
  else if (event == vtkMRMLViewNode::GraphicalResourcesCreatedEvent)
    {
    this->UpdateFromMRML();
    }
  else
    {
    this->Superclass::ProcessMRMLNodesEvents(caller, event, callData);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::OnInteractorStyleEvent(int eventID)
{
  switch (eventID)
    {
    case vtkCommand::EndInteractionEvent:
    case vtkCommand::StartInteractionEvent:
      this->Internal->UpdatePipelineTransforms(nullptr);
      break;
    default:
      break;
    }
  this->Superclass::OnInteractorStyleEvent(eventID);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::UpdateFromMRML()
{
  this->SetUpdateFromMRMLRequested(false);

  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
    {
    vtkDebugMacro( "vtkMRMLVolumeRenderingDisplayableManager::UpdateFromMRML: Scene is not set");
    return;
    }
  this->Internal->ClearDisplayableNodes();

  vtkMRMLVolumeNode* volumeNode = nullptr;
  std::vector<vtkMRMLNode*> volumeNodes;
  int numOfVolumeNodes = scene ? scene->GetNodesByClass("vtkMRMLVolumeNode", volumeNodes) : 0;
  for (int i=0; i<numOfVolumeNodes; i++)
    {
    volumeNode = vtkMRMLVolumeNode::SafeDownCast(volumeNodes[i]);
    if (volumeNode && this->Internal->UseDisplayableNode(volumeNode))
      {
      this->Internal->AddVolumeNode(volumeNode);
      }
    }

  this->RequestRender();
}

//---------------------------------------------------------------------------
vtkVolumeMapper* vtkMRMLVolumeRenderingDisplayableManager::GetVolumeMapper(vtkMRMLVolumeNode* volumeNode)
{
  if (!volumeNode)
    {
    return nullptr;
    }
  vtkMRMLVolumeRenderingDisplayNode* displayNode = nullptr;
  for (auto pipeline : this->Internal->DisplayPipelines)
    {
    if (pipeline->DisplayNode && pipeline->DisplayNode->GetDisplayableNode() == volumeNode)
      {
      // found a match
      if (!displayNode)
        {
        // first match
        displayNode = pipeline->DisplayNode;
        }
      else
        {
        // second match
        vtkWarningMacro("GetVolumeMapper: More than one display node found, using the first one");
        break;
        }
      }
    }
  if (!displayNode)
    {
    vtkErrorMacro("GetVolumeMapper: No volume rendering display node found for volume " << volumeNode->GetName());
    return nullptr;
    }
  return this->Internal->GetVolumeMapper(displayNode);
}

//---------------------------------------------------------------------------
vtkVolume* vtkMRMLVolumeRenderingDisplayableManager::GetVolumeActor(vtkMRMLVolumeNode* volumeNode)
{
  if (!volumeNode)
    {
    return nullptr;
    }
  vtkVolume* volumeActor = nullptr;
  for (auto pipeline : this->Internal->DisplayPipelines)
    {
    if (pipeline->VolumeActor && pipeline->DisplayNode->GetDisplayableNode() == volumeNode)
      {
      // found a match
      if (!volumeActor)
        {
        // first match
        volumeActor = pipeline->VolumeActor;
        }
      else
        {
        // second match
        vtkWarningMacro("GetVolumeActor: More than one volume rendering actor found, using the first one");
        break;
        }
      }
    }
  if (!volumeActor)
    {
    vtkErrorMacro("GetVolumeActor: No volume rendering actor found for volume " << volumeNode->GetName());
    }
  return volumeActor;
}

//---------------------------------------------------------------------------
int vtkMRMLVolumeRenderingDisplayableManager::Pick3D(double ras[3])
{
  this->Internal->PickedNodeID = "";

  vtkRenderer* ren = this->GetRenderer();
  if (!ren)
    {
    vtkErrorMacro("Pick3D: Unable to get renderer");
    return 0;
    }

#if VTK_MAJOR_VERSION >= 9 || (VTK_MAJOR_VERSION >= 8 && VTK_MINOR_VERSION >= 2)
  if (this->Internal->VolumePicker->Pick3DPoint(ras, ren))
    {
    vtkVolume* volume = vtkVolume::SafeDownCast(this->Internal->VolumePicker->GetProp3D());
    // Find the volume this image data belongs to
    this->Internal->FindPickedDisplayNodeFromVolumeActor(volume);
    }
#else
  vtkErrorMacro("Pick3D: This function is only accessible in newer VTK version");
  (void)ras; // not used
#endif

  return 1;
}

//---------------------------------------------------------------------------
const char* vtkMRMLVolumeRenderingDisplayableManager::GetPickedNodeID()
{
  return this->Internal->PickedNodeID.c_str();
}
