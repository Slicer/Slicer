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
#include "vtkMRMLScene.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLViewNode.h"
#include "vtkMRMLVolumePropertyNode.h"
#include "vtkEventBroker.h"

// VTK includes
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
#include <vtkMultiVolume.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkDoubleArray.h>

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
    virtual ~Pipeline() { };
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
    PipelineMultiVolume() : Pipeline()
    {
      this->ActorPortIndex = NextActorPortIndex++;
    }
    ~PipelineMultiVolume()
    {
      NextActorPortIndex--;
    }

    unsigned int ActorPortIndex;
    /// Used to determine the port index in the multi-volume actor
    static unsigned int NextActorPortIndex;
  };

  //-------------------------------------------------------------------------
  typedef std::map < vtkMRMLVolumeRenderingDisplayNode*, const Pipeline* > PipelinesCacheType;
  PipelinesCacheType DisplayPipelines;

  typedef std::map < vtkMRMLVolumeNode*, std::set< vtkMRMLVolumeRenderingDisplayNode* > > VolumeToDisplayCacheType;
  VolumeToDisplayCacheType VolumeToDisplayNodes;

  vtkVolumeMapper* GetVolumeMapper(vtkMRMLVolumeRenderingDisplayNode* displayNode)const;

  // Volumes
  void AddVolumeNode(vtkMRMLVolumeNode* displayableNode);
  void RemoveVolumeNode(vtkMRMLVolumeNode* displayableNode);

  // Transforms
  void UpdatePipelineTransforms(vtkMRMLVolumeNode *node);
  bool GetVolumeTransformToWorld(vtkMRMLVolumeNode* node, vtkMatrix4x4* ijkToWorldMatrix);

  // ROIs
  void UpdatePipelineROIs(vtkMRMLVolumeRenderingDisplayNode* displayNode, const Pipeline* pipeline);

  // Display Nodes
  void AddDisplayNode(vtkMRMLVolumeNode* volumeNode, vtkMRMLVolumeRenderingDisplayNode* displayNode);
  void RemoveDisplayNode(vtkMRMLVolumeRenderingDisplayNode* displayNode);
  void UpdateDisplayNode(vtkMRMLVolumeRenderingDisplayNode* displayNode);
  void UpdateDisplayNodePipeline(vtkMRMLVolumeRenderingDisplayNode* displayNode, const Pipeline* pipeline);

  double GetSampleDistance(vtkMRMLVolumeRenderingDisplayNode* displayNode);
  double GetFramerate(vtkMRMLVolumeRenderingDisplayNode* displayNode);
  vtkIdType GetMaxMemoryInBytes(vtkMRMLVolumeRenderingDisplayNode* displayNode);
  void UpdateDesiredUpdateRate(vtkMRMLVolumeRenderingDisplayNode* displayNode);

  // Observations
  void AddObservations(vtkMRMLVolumeNode* node);
  void RemoveObservations(vtkMRMLVolumeNode* node);
  bool IsNodeObserved(vtkMRMLVolumeNode* node);

  // Helper functions
  bool IsVisible(vtkMRMLVolumeRenderingDisplayNode* displayNode);
  bool UseDisplayNode(vtkMRMLVolumeRenderingDisplayNode* displayNode);
  bool UseDisplayableNode(vtkMRMLVolumeNode* node);
  void ClearDisplayableNodes();

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

private:
  /// Multi-volume actor using a common mapper for rendering the multiple volumes
  vtkSmartPointer<vtkMultiVolume> MultiVolumeActor;
  /// Common GPU mapper for the multi-volume actor.
  /// Note: vtkMultiVolume only supports the GPU raycast mapper
  vtkSmartPointer<vtkGPUVolumeRayCastMapper> MultiVolumeMapper;
};

//TODO: Change back to 0 once the VTK issue https://gitlab.kitware.com/vtk/vtk/issues/17325 is fixed
unsigned int vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::PipelineMultiVolume::NextActorPortIndex = 1;

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::vtkInternal(vtkMRMLVolumeRenderingDisplayableManager* external)
: External(external)
, AddingVolumeNode(false)
, OriginalDesiredUpdateRate(0.0) // 0 fps is a special value that means it hasn't been set
, Interaction(0)
{
  this->MultiVolumeActor = vtkSmartPointer<vtkMultiVolume>::New();
  this->MultiVolumeMapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
  // Set GPU mapper to the multi-volume actor. Both objects are only used for GPU ray casting
  this->MultiVolumeActor->SetMapper(this->MultiVolumeMapper);

  this->DisplayObservedEvents = vtkIntArray::New();
  this->DisplayObservedEvents->InsertNextValue(vtkCommand::StartEvent);
  this->DisplayObservedEvents->InsertNextValue(vtkCommand::EndEvent);
  this->DisplayObservedEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  this->DisplayObservedEvents->InsertNextValue(vtkCommand::StartInteractionEvent);
  this->DisplayObservedEvents->InsertNextValue(vtkCommand::InteractionEvent);
  this->DisplayObservedEvents->InsertNextValue(vtkCommand::EndInteractionEvent);
}

//---------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::~vtkInternal()
{
  this->ClearDisplayableNodes();

  if (this->DisplayObservedEvents)
    {
    this->DisplayObservedEvents->Delete();
    this->DisplayObservedEvents = NULL;
    }
}

//---------------------------------------------------------------------------
vtkVolumeMapper* vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::GetVolumeMapper(vtkMRMLVolumeRenderingDisplayNode* displayNode)const
{
  if (!displayNode)
    {
    return NULL;
    }
  if ( displayNode->IsA("vtkMRMLCPURayCastVolumeRenderingDisplayNode")
    || displayNode->IsA("vtkMRMLGPURayCastVolumeRenderingDisplayNode") )
    {
    PipelinesCacheType::const_iterator it = this->DisplayPipelines.find(displayNode);
    if (it == this->DisplayPipelines.end())
      {
      vtkErrorWithObjectMacro(this->External, "GetVolumeMapper: Failed to find pipeline for display node with ID " << displayNode->GetID());
      return NULL;
      }
    const Pipeline* foundPipeline = it->second;
    if (displayNode->IsA("vtkMRMLCPURayCastVolumeRenderingDisplayNode"))
      {
      const PipelineCPU* pipelineCpu = dynamic_cast<const PipelineCPU*>(foundPipeline);
      if (pipelineCpu)
        {
        return pipelineCpu->RayCastMapperCPU;
        }
      }
    else if (displayNode->IsA("vtkMRMLGPURayCastVolumeRenderingDisplayNode"))
      {
      const PipelineGPU* pipelineGpu = dynamic_cast<const PipelineGPU*>(foundPipeline);
      if (pipelineGpu)
        {
        return pipelineGpu->RayCastMapperGPU;
        }
      }
    }
  else if (displayNode->IsA("vtkMRMLMultiVolumeRenderingDisplayNode"))
    {
    return this->MultiVolumeMapper;
    }
  vtkErrorWithObjectMacro(this->External, "GetVolumeMapper: Unsupported display class " << displayNode->GetClassName());
  return NULL;
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
  return displayNode && displayNode->GetVisibility();
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
      this->VolumeToDisplayNodes[node].insert(displayNode);
      this->AddDisplayNode(node, displayNode);
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
  vtkInternal::VolumeToDisplayCacheType::iterator displayableIt = this->VolumeToDisplayNodes.find(node);
  if (displayableIt == this->VolumeToDisplayNodes.end())
    {
    return;
    }

  std::set<vtkMRMLVolumeRenderingDisplayNode *> dnodes = displayableIt->second;
  std::set<vtkMRMLVolumeRenderingDisplayNode *>::iterator diter;
  for (diter = dnodes.begin(); diter != dnodes.end(); ++diter)
    {
    this->RemoveDisplayNode(*diter);
    }
  this->RemoveObservations(node);
  this->VolumeToDisplayNodes.erase(displayableIt);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::AddDisplayNode(vtkMRMLVolumeNode* volumeNode, vtkMRMLVolumeRenderingDisplayNode* displayNode)
{
  if (!volumeNode || !displayNode)
    {
    return;
    }

  // Do not add the display node if it is already associated with a pipeline object.
  // This happens when a node already associated with a display node is copied into another
  // (using vtkMRMLNode::Copy()) and is added to the scene afterward.
  // Related issue are #3428 and #2608
  PipelinesCacheType::iterator pipelineIt = this->DisplayPipelines.find(displayNode);
  if (pipelineIt != this->DisplayPipelines.end())
    {
    return;
    }

  if (displayNode->IsA("vtkMRMLCPURayCastVolumeRenderingDisplayNode"))
    {
    PipelineCPU* pipelineCpu = new PipelineCPU();
    // Set volume to the mapper
    pipelineCpu->RayCastMapperCPU->SetInputConnection(0, volumeNode->GetImageDataConnection());
    // Add volume actor to renderer and local cache
    this->External->GetRenderer()->AddVolume(pipelineCpu->VolumeActor);
    // Add pipeline
    this->DisplayPipelines.insert( std::make_pair(displayNode, pipelineCpu) );
    }
  else if (displayNode->IsA("vtkMRMLGPURayCastVolumeRenderingDisplayNode"))
    {
    PipelineGPU* pipelineGpu = new PipelineGPU();
    // Set volume to the mapper
    pipelineGpu->RayCastMapperGPU->SetInputConnection(0, volumeNode->GetImageDataConnection());
    // Add volume actor to renderer and local cache
    this->External->GetRenderer()->AddVolume(pipelineGpu->VolumeActor);
    // Add pipeline
    this->DisplayPipelines.insert( std::make_pair(displayNode, pipelineGpu) );
    }
  else if (displayNode->IsA("vtkMRMLMultiVolumeRenderingDisplayNode"))
    {
    PipelineMultiVolume* pipelineMulti = new PipelineMultiVolume();
    // Create a dummy volume for port zero if this is the first volume. Necessary because the first transform is ignored,
    // see https://gitlab.kitware.com/vtk/vtk/issues/17325
    //TODO: Remove this workaround when the issue is fixed in VTK
    if (this->MultiVolumeActor->GetVolume(0) == NULL)
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
    this->DisplayPipelines.insert( std::make_pair(displayNode, pipelineMulti) );
    }

  this->External->GetMRMLNodesObserverManager()->AddObjectEvents(displayNode, this->DisplayObservedEvents);

  // Update cached matrix. Calls UpdateDisplayNodePipeline
  this->UpdatePipelineTransforms(volumeNode);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::RemoveDisplayNode(vtkMRMLVolumeRenderingDisplayNode* displayNode)
{
  PipelinesCacheType::iterator pipelineIt = this->DisplayPipelines.find(displayNode);
  if (pipelineIt == this->DisplayPipelines.end())
    {
    return;
    }

  const Pipeline* pipeline = pipelineIt->second;

  if ( displayNode->IsA("vtkMRMLCPURayCastVolumeRenderingDisplayNode")
    || displayNode->IsA("vtkMRMLGPURayCastVolumeRenderingDisplayNode") )
    {
    // Remove volume actor from renderer and local cache
    this->External->GetRenderer()->RemoveVolume(pipeline->VolumeActor);
    }
  else if (displayNode->IsA("vtkMRMLMultiVolumeRenderingDisplayNode"))
    {
    const PipelineMultiVolume* pipelineMulti = dynamic_cast<const PipelineMultiVolume*>(pipeline);
    if (pipelineMulti)
      {
      // Remove volume actor from multi-volume actor collection
      this->MultiVolumeMapper->RemoveInputConnection(pipelineMulti->ActorPortIndex, 0);
      this->MultiVolumeActor->RemoveVolume(pipelineMulti->ActorPortIndex);

      // Remove common actor from renderer and local cache if the last volume have been removed
      if (this->DisplayPipelines.size() == 0)
        {
        this->External->GetRenderer()->RemoveVolume(this->MultiVolumeActor);
        }
      }
    }

  delete pipeline;
  this->DisplayPipelines.erase(pipelineIt);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::UpdatePipelineTransforms(vtkMRMLVolumeNode* volumeNode)
{
  // Update the pipeline for all tracked DisplayableNode
  PipelinesCacheType::iterator pipelineIt;
  std::set< vtkMRMLVolumeRenderingDisplayNode* > displayNodes = this->VolumeToDisplayNodes[volumeNode];
  std::set< vtkMRMLVolumeRenderingDisplayNode* >::iterator displayNodeIt;
  for (displayNodeIt = displayNodes.begin(); displayNodeIt != displayNodes.end(); displayNodeIt++)
    {
    if (((pipelineIt = this->DisplayPipelines.find(*displayNodeIt)) != this->DisplayPipelines.end()))
      {
      vtkMRMLVolumeRenderingDisplayNode* currentDisplayNode = pipelineIt->first;
      const Pipeline* currentPipeline = pipelineIt->second;
      this->UpdateDisplayNodePipeline(currentDisplayNode, currentPipeline);

      // Calculate and apply transform matrix
      this->GetVolumeTransformToWorld(volumeNode, currentPipeline->IJKToWorldMatrix);
      currentPipeline->VolumeActor->SetUserMatrix(currentPipeline->IJKToWorldMatrix.GetPointer());
      }
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::GetVolumeTransformToWorld(
  vtkMRMLVolumeNode* volumeNode, vtkMatrix4x4* outputIjkToWorldMatrix)
{
  if (volumeNode == NULL)
    {
    vtkErrorWithObjectMacro(this->External, "GetVolumeTransformToWorld: Invalid volume node");
    return false;
    }

  // Check if we have a transform node
  vtkMRMLTransformNode* transformNode = volumeNode->GetParentTransformNode();
  if (transformNode == NULL)
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
  PipelinesCacheType::iterator displayNodeIt;
  displayNodeIt = this->DisplayPipelines.find(displayNode);
  if (displayNodeIt != this->DisplayPipelines.end())
    {
    this->UpdateDisplayNodePipeline(displayNode, displayNodeIt->second);
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
  bool displayNodeVisible = displayNode->GetVisibility() && displayNode->GetOpacity() > 0
    && displayNode->GetVisibility(this->External->GetMRMLViewNode()->GetID());

  // Get volume node
  vtkMRMLVolumeNode* volumeNode = displayNode ? displayNode->GetVolumeNode() : NULL;
  if (!volumeNode)
    {
    return;
    }

  // Set volume visibility, return if hidden
  pipeline->VolumeActor->SetVisibility(displayNodeVisible);
  // Workaround for lack of support for Visibility flag in vtkMultiVolume's vtkVolume members
  //TODO: Remove when https://gitlab.kitware.com/vtk/vtk/issues/17302 is fixed in VTK
  if (displayNode->IsA("vtkMRMLMultiVolumeRenderingDisplayNode"))
    {
    PipelinesCacheType::iterator pipelineIt = this->DisplayPipelines.find(displayNode);
    if (pipelineIt != this->DisplayPipelines.end())
      {
      const PipelineMultiVolume* pipelineMulti = dynamic_cast<const PipelineMultiVolume*>(pipelineIt->second);
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
      }
    }
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

    switch (displayNode->GetPerformanceControl())
      {
      case vtkMRMLVolumeRenderingDisplayNode::AdaptiveQuality:
        cpuMapper->SetAutoAdjustSampleDistances(true);
        cpuMapper->SetLockSampleDistanceToInputSpacing(false);
        cpuMapper->SetImageSampleDistance(1.0);
        break;
      case vtkMRMLVolumeRenderingDisplayNode::NormalQuality:
        cpuMapper->SetAutoAdjustSampleDistances(false);
        cpuMapper->SetLockSampleDistanceToInputSpacing(true);
        cpuMapper->SetImageSampleDistance(1.0);
        break;
      case vtkMRMLVolumeRenderingDisplayNode::MaximumQuality:
        cpuMapper->SetAutoAdjustSampleDistances(false);
        cpuMapper->SetLockSampleDistanceToInputSpacing(false);
        cpuMapper->SetImageSampleDistance(0.5);
        break;
      }

    cpuMapper->SetSampleDistance(this->GetSampleDistance(displayNode));
    cpuMapper->SetInteractiveSampleDistance(this->GetSampleDistance(displayNode));

    // Make sure the correct mapper is set to the volume
    pipeline->VolumeActor->SetMapper(mapper);
    // Make sure the correct volume is set to the mapper
    mapper->SetInputConnection(0, volumeNode->GetImageDataConnection());
    }
  else if (displayNode->IsA("vtkMRMLGPURayCastVolumeRenderingDisplayNode"))
    {
    vtkMRMLGPURayCastVolumeRenderingDisplayNode* gpuDisplayNode =
      vtkMRMLGPURayCastVolumeRenderingDisplayNode::SafeDownCast(displayNode);
    vtkGPUVolumeRayCastMapper* gpuMapper = vtkGPUVolumeRayCastMapper::SafeDownCast(mapper);

    switch (displayNode->GetPerformanceControl())
      {
      case vtkMRMLVolumeRenderingDisplayNode::AdaptiveQuality:
        gpuMapper->SetAutoAdjustSampleDistances(true);
        gpuMapper->SetLockSampleDistanceToInputSpacing(false);
        gpuMapper->SetUseJittering(gpuDisplayNode->GetSurfaceSmoothing());
        break;
      case vtkMRMLVolumeRenderingDisplayNode::NormalQuality:
        gpuMapper->SetAutoAdjustSampleDistances(false);
        gpuMapper->SetLockSampleDistanceToInputSpacing(true);
        gpuMapper->SetLockSampleDistanceToInputSpacing(true);
        gpuMapper->SetUseJittering(gpuDisplayNode->GetSurfaceSmoothing());
        break;
      case vtkMRMLVolumeRenderingDisplayNode::MaximumQuality:
        gpuMapper->SetAutoAdjustSampleDistances(false);
        gpuMapper->SetLockSampleDistanceToInputSpacing(false);
        gpuMapper->SetUseJittering(gpuDisplayNode->GetSurfaceSmoothing());
        break;
      }

    gpuMapper->SetSampleDistance(this->GetSampleDistance(gpuDisplayNode));
    gpuMapper->SetMaxMemoryInBytes(this->GetMaxMemoryInBytes(gpuDisplayNode));

    // Make sure the correct mapper is set to the volume
    pipeline->VolumeActor->SetMapper(mapper);
    // Make sure the correct volume is set to the mapper
    mapper->SetInputConnection(0, volumeNode->GetImageDataConnection());
    }
  else if (displayNode->IsA("vtkMRMLMultiVolumeRenderingDisplayNode"))
    {
    vtkMRMLMultiVolumeRenderingDisplayNode* multiDisplayNode =
      vtkMRMLMultiVolumeRenderingDisplayNode::SafeDownCast(displayNode);
    vtkGPUVolumeRayCastMapper* gpuMapper = vtkGPUVolumeRayCastMapper::SafeDownCast(mapper);

    //TODO: All the multi-volume display nodes must have the same settings that concern the mapper
    switch (displayNode->GetPerformanceControl())
      {
      case vtkMRMLVolumeRenderingDisplayNode::AdaptiveQuality:
        gpuMapper->SetAutoAdjustSampleDistances(true);
        gpuMapper->SetLockSampleDistanceToInputSpacing(false);
        gpuMapper->SetUseJittering(multiDisplayNode->GetSurfaceSmoothing());
        break;
      case vtkMRMLVolumeRenderingDisplayNode::NormalQuality:
        gpuMapper->SetAutoAdjustSampleDistances(false);
        gpuMapper->SetLockSampleDistanceToInputSpacing(true);
        gpuMapper->SetLockSampleDistanceToInputSpacing(true);
        gpuMapper->SetUseJittering(multiDisplayNode->GetSurfaceSmoothing());
        break;
      case vtkMRMLVolumeRenderingDisplayNode::MaximumQuality:
        gpuMapper->SetAutoAdjustSampleDistances(false);
        gpuMapper->SetLockSampleDistanceToInputSpacing(false);
        gpuMapper->SetUseJittering(multiDisplayNode->GetSurfaceSmoothing());
        break;
      }

    gpuMapper->SetSampleDistance(this->GetSampleDistance(multiDisplayNode));
    gpuMapper->SetMaxMemoryInBytes(this->GetMaxMemoryInBytes(multiDisplayNode));
    }
  else
    {
    vtkErrorWithObjectMacro(this->External, "UpdateDisplayNodePipeline: Display node type " << displayNode->GetNodeTagName() << " is not supported");
    return;
    }

  // Set raycast technique
  switch (displayNode->GetRaycastTechnique())
    {
    case vtkMRMLVolumeRenderingDisplayNode::MaximumIntensityProjection:
      mapper->SetBlendMode(vtkVolumeMapper::MAXIMUM_INTENSITY_BLEND);
      break;
    case vtkMRMLVolumeRenderingDisplayNode::MinimumIntensityProjection:
      mapper->SetBlendMode(vtkVolumeMapper::MINIMUM_INTENSITY_BLEND);
      break;
    case vtkMRMLVolumeRenderingDisplayNode::Composite:
    default:
      mapper->SetBlendMode(vtkVolumeMapper::COMPOSITE_BLEND);
      break;
    }

  // Update ROI clipping planes
  this->UpdatePipelineROIs(displayNode, pipeline);

  // Set volume property
  vtkVolumeProperty* volumeProperty = displayNode->GetVolumePropertyNode() ? displayNode->GetVolumePropertyNode()->GetVolumeProperty() : NULL;
  pipeline->VolumeActor->SetProperty(volumeProperty);

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
  if (!displayNode || displayNode->GetROINode() == NULL || !displayNode->GetCroppingEnabled())
    {
    volumeMapper->RemoveAllClippingPlanes();
    return;
    }

  // Make sure the ROI node's inside out flag is on
  displayNode->GetROINode()->InsideOutOn();

  // Calculate and set clipping planes
  vtkNew<vtkPlanes> planes;
  displayNode->GetROINode()->GetTransformedPlanes(planes);
  volumeMapper->SetClippingPlanes(planes);
}

//---------------------------------------------------------------------------
double vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::GetSampleDistance(
  vtkMRMLVolumeRenderingDisplayNode* displayNode)
{
  vtkMRMLVolumeNode* volumeNode = displayNode ? displayNode->GetVolumeNode() : NULL;
  if (!volumeNode)
    {
    vtkErrorWithObjectMacro(this->External, "GetSampleDistance: Failed to access volume node");
    return displayNode->GetEstimatedSampleDistance();
    }
  const double minSpacing = volumeNode->GetMinSpacing() > 0 ? volumeNode->GetMinSpacing() : 1.;
  double sampleDistance = minSpacing / displayNode->GetEstimatedSampleDistance();
  if (displayNode->GetPerformanceControl() == vtkMRMLVolumeRenderingDisplayNode::MaximumQuality)
    {
    sampleDistance = minSpacing / 10.; // =10x smaller than pixel is high quality
    }
  return sampleDistance;
}

//---------------------------------------------------------------------------
double vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::GetFramerate(vtkMRMLVolumeRenderingDisplayNode* displayNode)
{
  if (!displayNode)
  {
    return 15.;
  }
  double framerate = 0.0001;
  switch (displayNode->GetPerformanceControl())
  {
    case vtkMRMLVolumeRenderingDisplayNode::AdaptiveQuality:
      framerate = std::max(displayNode->GetExpectedFPS(), 0.0001);
      break;
    case vtkMRMLVolumeRenderingDisplayNode::NormalQuality:
    case vtkMRMLVolumeRenderingDisplayNode::MaximumQuality:
      // special value meaning start render as soon as possible
      // (next time the application processes events)
      framerate = 0.0;
      break;
  }
  return framerate;
}

//---------------------------------------------------------------------------
vtkIdType vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::GetMaxMemoryInBytes(
  vtkMRMLVolumeRenderingDisplayNode* displayNode)
{
  int gpuMemorySizeMB = vtkMRMLVolumeRenderingDisplayableManager::DefaultGPUMemorySize;
  if (displayNode && displayNode->GetGPUMemorySize() > 0)
    {
    gpuMemorySizeMB = displayNode->GetGPUMemorySize();
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
  vtkRenderWindowInteractor* renderWindowInteractor = renderWindow ? renderWindow->GetInteractor() : 0;
  if (!renderWindowInteractor)
    {
    return;
    }
  double fps = this->GetFramerate(displayNode);
  if (displayNode->GetVisibility())
    {
    if (this->OriginalDesiredUpdateRate == 0.0)
      {
      // Save the DesiredUpdateRate before it is changed.
      // It will then be restored when the volume rendering is hidden
      this->OriginalDesiredUpdateRate = renderWindowInteractor->GetDesiredUpdateRate();
      }
    renderWindowInteractor->SetDesiredUpdateRate(fps);
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
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  if (!broker->GetObservationExist(node, vtkCommand::ModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand()))
    {
    broker->AddObservation(node, vtkCommand::ModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand());
    }
  if (!broker->GetObservationExist(node, vtkMRMLDisplayableNode::TransformModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand()))
    {
    broker->AddObservation(node, vtkMRMLDisplayableNode::TransformModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
    }
  if (!broker->GetObservationExist(node, vtkMRMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() ))
    {
    broker->AddObservation(node, vtkMRMLDisplayableNode::DisplayModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
    }
  if (!broker->GetObservationExist(node, vtkMRMLVolumeNode::ImageDataModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() ))
    {
    broker->AddObservation(node, vtkMRMLVolumeNode::ImageDataModifiedEvent, this->External, this->External->GetMRMLNodesCallbackCommand() );
    }
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::RemoveObservations(vtkMRMLVolumeNode* node)
{
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
  while (this->VolumeToDisplayNodes.size() > 0)
    {
    this->RemoveVolumeNode(this->VolumeToDisplayNodes.begin()->first);
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLVolumeRenderingDisplayableManager::vtkInternal::UseDisplayableNode(vtkMRMLVolumeNode* node)
{
  bool use = node && node->IsA("vtkMRMLVolumeNode");
  return use;
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
  this->Internal=NULL;
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
  this->SetUpdateFromMRMLRequested(1);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::ObserveGraphicalResourcesCreatedEvent()
{
  vtkMRMLViewNode* viewNode = this->GetMRMLViewNode();
  if (viewNode == NULL)
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
  this->SetUpdateFromMRMLRequested(1);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::OnMRMLSceneEndBatchProcess()
{
  this->SetUpdateFromMRMLRequested(1);
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
  if ( !node->IsA("vtkMRMLVolumeNode") )
    {
    return;
    }

  // Escape if the scene is being closed, imported or connected
  if (this->GetMRMLScene()->IsBatchProcessing())
    {
    this->SetUpdateFromMRMLRequested(1);
    return;
    }

  this->Internal->AddVolumeNode(vtkMRMLVolumeNode::SafeDownCast(node));
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  if ( node
    && (!node->IsA("vtkMRMLVolumeNode"))
    && (!node->IsA("vtkMRMLVolumeRenderingDisplayNode")) )
    {
    return;
    }

  vtkMRMLVolumeNode* volumeNode = NULL;
  vtkMRMLVolumeRenderingDisplayNode* displayNode = NULL;

  bool modified = false;
  if ( (volumeNode = vtkMRMLVolumeNode::SafeDownCast(node)) )
    {
    this->Internal->RemoveVolumeNode(volumeNode);
    modified = true;
    }
  else if ( (displayNode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(node)) )
    {
    this->Internal->RemoveDisplayNode(displayNode);
    modified = true;
    }
  if (modified)
    {
    this->RequestRender();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
{
  vtkMRMLScene* scene = this->GetMRMLScene();

  if (scene->IsBatchProcessing())
    {
    return;
    }

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
          this->RequestRender();
          }
        }
      }
    else if ( (event == vtkMRMLDisplayableNode::TransformModifiedEvent)
           || (event == vtkMRMLTransformableNode::TransformModifiedEvent) )
      {
      this->Internal->UpdatePipelineTransforms(volumeNode);

      // Reset ROI
      std::set< vtkMRMLVolumeRenderingDisplayNode* > displayNodes = this->Internal->VolumeToDisplayNodes[volumeNode];
      std::set< vtkMRMLVolumeRenderingDisplayNode* >::iterator displayNodeIt;
      for (displayNodeIt = displayNodes.begin(); displayNodeIt != displayNodes.end(); displayNodeIt++)
        {
        this->VolumeRenderingLogic->FitROIToVolume(*displayNodeIt);
        }

      this->RequestRender();
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
          this->RequestRender();
          }
        }
      }
    }
  else if (event == vtkCommand::StartEvent ||
           event == vtkCommand::StartInteractionEvent)
    {
    ++this->Internal->Interaction;
    // We request the interactive mode, we might have nested interactions
    // so we just start the mode for the first time.
    if (this->Internal->Interaction == 1)
      {
      vtkInteractorStyle* interactorStyle = vtkInteractorStyle::SafeDownCast(
        this->GetInteractor()->GetInteractorStyle());
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
      vtkInteractorStyle* interactorStyle = vtkInteractorStyle::SafeDownCast(
        this->GetInteractor()->GetInteractorStyle());
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
    {
      vtkInternal::VolumeToDisplayCacheType::iterator displayableIt;
      for ( displayableIt = this->Internal->VolumeToDisplayNodes.begin();
            displayableIt!=this->Internal->VolumeToDisplayNodes.end(); ++displayableIt )
        {
        this->Internal->UpdatePipelineTransforms(displayableIt->first);
        }
      break;
    }
    default:
      break;
    }
  this->Superclass::OnInteractorStyleEvent(eventID);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::UpdateFromMRML()
{
  this->SetUpdateFromMRMLRequested(0);

  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
    {
    vtkDebugMacro( "vtkMRMLVolumeRenderingDisplayableManager::UpdateFromMRML: Scene is not set")
    return;
    }
  this->Internal->ClearDisplayableNodes();

  vtkMRMLVolumeNode* volumeNode = NULL;
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
