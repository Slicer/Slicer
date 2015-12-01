/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Slicer includes
#include "vtkImageGradientMagnitude.h"
#include "vtkMRMLVolumeRenderingDisplayableManager.h"
#include "vtkSlicerFixedPointVolumeRayCastMapper.h"
#include "vtkSlicerVolumeRenderingLogic.h"

#include "vtkMRMLCPURayCastVolumeRenderingDisplayNode.h"
#include "vtkMRMLGPURayCastVolumeRenderingDisplayNode.h"

// MRML includes
#include "vtkEventBroker.h"
#include "vtkMRMLAnnotationROINode.h"
#include "vtkMRMLLabelMapVolumeDisplayNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLSliceLogic.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLViewNode.h"
#include "vtkMRMLVolumePropertyNode.h"
#include "vtkMRMLVolumePropertyStorageNode.h"
#include "vtkMRMLVolumeRenderingDisplayNode.h"
#include "vtkMRMLVolumeRenderingScenarioNode.h"

// VTK includes
#include <vtkAbstractTransform.h>
#include <vtkCallbackCommand.h>
#include "vtkFixedPointVolumeRayCastMapper.h"
#include "vtkGPUVolumeRayCastMapper.h"
#include "vtkImageData.h"
#include "vtkInteractorStyle.h"
#include "vtkLookupTable.h"
#include "vtkMatrix4x4.h"
#include <vtkNew.h>
#include "vtkObjectFactory.h"
#include "vtkPlane.h"
#include "vtkPlanes.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkVolume.h"
#include "vtkVolumeProperty.h"
#include <vtkVersion.h>

// ITKSys includes
//#include <itksys/SystemTools.hxx>
//#include <itksys/Directory.hxx>

// STD includes
#include <cassert>
#include <cmath>
#include <algorithm> // for std::min

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLVolumeRenderingDisplayableManager);

//---------------------------------------------------------------------------
bool vtkMRMLVolumeRenderingDisplayableManager::First = true;
int vtkMRMLVolumeRenderingDisplayableManager::DefaultGPUMemorySize = 256;

//---------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayableManager::vtkMRMLVolumeRenderingDisplayableManager()
{
  this->MapperRaycast = NULL;
  this->MapperGPURaycast3 = NULL;
  this->Volume = NULL;
  //this->Histograms = vtkKWHistogramSet::New();
  //this->HistogramsFg = vtkKWHistogramSet::New();
  //this->VolumePropertyGPURaycast3 = NULL;

  // Initialize the raycasters in Reset
  this->Reset();

  this->VolumeRenderingLogic =  vtkSlicerVolumeRenderingLogic::New();;

  this->DisplayedNode = NULL;

  this->DisplayObservedEvents = vtkIntArray::New();
  this->DisplayObservedEvents->InsertNextValue(vtkCommand::StartEvent);
  this->DisplayObservedEvents->InsertNextValue(vtkCommand::EndEvent);
  this->DisplayObservedEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  this->DisplayObservedEvents->InsertNextValue(vtkCommand::StartInteractionEvent);
  this->DisplayObservedEvents->InsertNextValue(vtkCommand::InteractionEvent);
  this->DisplayObservedEvents->InsertNextValue(vtkCommand::EndInteractionEvent);

  this->Interaction = 0;
  // 0fps is a special value that means it hasn't been set.
  this->OriginalDesiredUpdateRate = 0.;

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
  this->RemoveDisplayNodes();

  if (this->VolumeRenderingLogic)
  {
    this->VolumeRenderingLogic->Delete();
    this->VolumeRenderingLogic = NULL;
  }
  if (this->DisplayObservedEvents)
    {
    this->DisplayObservedEvents->Delete();
    }

  //delete instances
  vtkSetMRMLNodeMacro(this->MapperRaycast, NULL);
  vtkSetMRMLNodeMacro(this->MapperGPURaycast3, NULL);
  vtkSetMRMLNodeMacro(this->Volume, NULL);
  /**
  if(this->Histograms != NULL)
  {
    this->Histograms->RemoveAllHistograms();
    this->Histograms->Delete();
    this->Histograms = NULL;
  }
  if(this->HistogramsFg != NULL)
  {
    this->HistogramsFg->RemoveAllHistograms();
    this->HistogramsFg->Delete();
    this->HistogramsFg = NULL;
  }
  **/
  //if (this->VolumePropertyGPURaycast3 != NULL)
  //{
  //  this->VolumePropertyGPURaycast3->Delete();
  //  this->VolumePropertyGPURaycast3 = NULL;
  //}
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::PrintSelf(std::ostream &os, vtkIndent indent)
{
  os<<indent<<"Print logic"<<endl;
}

//---------------------------------------------------------------------------
int vtkMRMLVolumeRenderingDisplayableManager::ActiveInteractionModes()
{
  // observe all the modes
  return ~0;
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::SetGUICallbackCommand(vtkCommand* callback)
{
  //cpu ray casting
  this->MapperRaycast->AddObserver(vtkCommand::VolumeMapperComputeGradientsProgressEvent, callback);
  this->MapperRaycast->AddObserver(vtkCommand::ProgressEvent,callback);

  //hook up the gpu mapper

  this->MapperGPURaycast3->AddObserver(vtkCommand::VolumeMapperComputeGradientsProgressEvent, callback);

  //this->GetMRMLNodesCallbackCommand() = callback;
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::Reset()
{
  vtkNew<vtkIntArray> mapperEvents;
  //mapperEvents->InsertNextValue(
  //  vtkCommand::VolumeMapperComputeGradientsProgressEvent);

  vtkNew<vtkIntArray> mapperEventsWithProgress;
  //mapperEventsWithProgress->InsertNextValue(
  //  vtkCommand::VolumeMapperComputeGradientsProgressEvent);
  //mapperEventsWithProgress->InsertNextValue(vtkCommand::ProgressEvent);

  // CPU mapper
  vtkNew<vtkFixedPointVolumeRayCastMapper> newMapperRaycast;
  vtkSetAndObserveMRMLNodeEventsMacro(this->MapperRaycast,
                                      newMapperRaycast.GetPointer(),
                                      mapperEventsWithProgress.GetPointer());

  // GPU raycast 3
  vtkNew<vtkGPUVolumeRayCastMapper> newMapperGPURaycast3;
  vtkSetAndObserveMRMLNodeEventsMacro(this->MapperGPURaycast3,
                                      newMapperGPURaycast3.GetPointer(),
                                      mapperEvents.GetPointer());

  // Volume
  vtkNew<vtkVolume> newVolume;
  vtkSetMRMLNodeMacro(this->Volume, newVolume.GetPointer());

  /**
  if(this->Histograms != NULL)
  {
    this->Histograms->RemoveAllHistograms();
    this->Histograms->Delete();
    this->Histograms = NULL;
  }
  if(this->HistogramsFg != NULL)
  {
    this->HistogramsFg->RemoveAllHistograms();
    this->HistogramsFg->Delete();
    this->HistogramsFg = NULL;
  }
  ***/
  //if (this->VolumePropertyGPURaycast3 != NULL)
  //{
  //  this->VolumePropertyGPURaycast3->Delete();
  //  this->VolumePropertyGPURaycast3 = NULL;
  //}

  //this->Histograms = vtkKWHistogramSet::New();
  //this->HistogramsFg = vtkKWHistogramSet::New();
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::SetupHistograms(vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  vtkImageData *input = vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetVolumeNode())->GetImageData();
  if (input == NULL)
    {
    return;
    }
/***
  //-----------------------------------------
  //  remove old histogram
  //-----------------------------------------
  if(this->Histograms != NULL)
  {
    this->Histograms->RemoveAllHistograms();
    this->Histograms->Delete();
    this->Histograms = vtkKWHistogramSet::New();
  }

  //setup histograms
  this->Histograms->AddHistograms(input->GetPointData()->GetScalars());

  //gradient histogram
  vtkImageGradientMagnitude *grad = vtkImageGradientMagnitude::New();
  grad->SetDimensionality(3);
  grad->SetInputData(input);
  grad->Update();

  vtkKWHistogram *gradHisto = vtkKWHistogram::New();
  gradHisto->BuildHistogram(grad->GetOutput()->GetPointData()->GetScalars(), 0);
  this->Histograms->AddHistogram(gradHisto, "0gradient");

  grad->Delete();
  gradHisto->Delete();
****/
}

//---------------------------------------------------------------------------
int vtkMRMLVolumeRenderingDisplayableManager
::GetMaxMemory(vtkVolumeMapper* volumeMapper,
               vtkMRMLVolumeRenderingDisplayNode* vspNode)
{

  int memory = vspNode && vspNode->GetGPUMemorySize() ?
    vspNode->GetGPUMemorySize() :
    vtkMRMLVolumeRenderingDisplayableManager::DefaultGPUMemorySize;

  int gpuRayCastMapper = 128*1024*1024;
  if (memory <= 128)
    {
    }
  else if (memory <= 256)
    {
    gpuRayCastMapper = 256*1024*1024;
    }
  else if (memory <= 512)
    {
    gpuRayCastMapper = 512*1024*1024;
    }
  else if (memory <= 1024)
    {
    gpuRayCastMapper = 1024*1024*1024;
    }
  else if (memory <= 1536)
    {
    gpuRayCastMapper = 1536*1024*1024;
    }
  else if (memory <= 2048)
    {
    gpuRayCastMapper = 2047*1024*1024;
    }
  else if (memory <= 3072)
    {
    gpuRayCastMapper = 2047*1024*1024;
    }
  else if (memory <= 4096)
    {
    gpuRayCastMapper = 2047*1024*1024;
    }

  if (volumeMapper->IsA("vtkGPUVolumeRayCastMapper"))
    {
    return gpuRayCastMapper;
    }

  // By default, return the node memory
  return memory * 1024 * 1024;
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager
::CalculateMatrix(vtkMRMLVolumeRenderingDisplayNode *vspNode, vtkMatrix4x4 *output)
{
  //Update matrix
  //Check for NUll Pointer
  if (vspNode == NULL)
    {
    return;
    }
  vtkMRMLTransformNode *tmp = vtkMRMLScalarVolumeNode::SafeDownCast(
    vspNode->GetVolumeNode())->GetParentTransformNode();
  //check if we have a TransformNode
  if(tmp == NULL)
    {
    vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetVolumeNode())->GetIJKToRASMatrix(output);
    return;
    }

  //IJK to ras
  vtkMatrix4x4 *matrix = vtkMatrix4x4::New();
  vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetVolumeNode())->GetIJKToRASMatrix(matrix);

  // Parent transforms
  vtkMatrix4x4   *transform = vtkMatrix4x4::New();
  tmp->GetMatrixTransformToWorld(transform);

  //Transform world to ras
  vtkMatrix4x4::Multiply4x4(transform, matrix, output);

  matrix->Delete();
  transform->Delete();
}

//---------------------------------------------------------------------------
double vtkMRMLVolumeRenderingDisplayableManager
::GetFramerate(vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  double framerate = vspNode ? vspNode->GetExpectedFPS() : 15.;
  //if (!this->GetRenderer()->GetRenderWindow()->GetEventPending())
  //  {
  //  framerate = 0.0001;
  //  }
  framerate = std::max(framerate, 0.0001);
  if (vspNode->GetPerformanceControl() ==
      vtkMRMLVolumeRenderingDisplayNode::MaximumQuality)
    {
    framerate = 0.0; // special value meaning full quality
    }
  return framerate;
}

//---------------------------------------------------------------------------
double vtkMRMLVolumeRenderingDisplayableManager
::GetSampleDistance(vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  vtkMRMLVolumeNode* volumeNode = vspNode ? vspNode->GetVolumeNode() : 0;
  const double minSpacing = volumeNode->GetMinSpacing() > 0 ?
    volumeNode->GetMinSpacing() : 1.;
  double sampleDistance = minSpacing / vspNode->GetEstimatedSampleDistance();
  if (vspNode->GetPerformanceControl() ==
      vtkMRMLVolumeRenderingDisplayNode::MaximumQuality)
    {
    sampleDistance = minSpacing / 10.; // =10x smaller than pixel is high quality
    }
  return sampleDistance;
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager
::UpdateMapper(
  vtkVolumeMapper* mapper,
  vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  this->UpdateClipping(mapper, vspNode);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager
::UpdateCPURaycastMapper(
  vtkFixedPointVolumeRayCastMapper* mapper,
  vtkMRMLCPURayCastVolumeRenderingDisplayNode* vspNode)
{
  this->UpdateMapper(mapper, vspNode);
  const bool highDef = vspNode->GetPerformanceControl() ==
    vtkMRMLVolumeRenderingDisplayNode::MaximumQuality;
  mapper->SetAutoAdjustSampleDistances( highDef ? 0 : 1);
  mapper->SetSampleDistance(this->GetSampleDistance(vspNode));
  mapper->SetInteractiveSampleDistance(this->GetSampleDistance(vspNode));
  mapper->SetImageSampleDistance(highDef ? 0.5 : 1.);

  switch(vspNode->GetRaycastTechnique())
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
    }
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager
::UpdateGPURaycastMapper(
  vtkGPUVolumeRayCastMapper* mapper,
  vtkMRMLGPURayCastVolumeRenderingDisplayNode* vspNode)
{
  this->UpdateMapper(mapper, vspNode);
  const bool highDef = vspNode->GetPerformanceControl() ==
    vtkMRMLVolumeRenderingDisplayNode::MaximumQuality;
  mapper->SetAutoAdjustSampleDistances( highDef ? 0 : 1);
  mapper->SetSampleDistance(this->GetSampleDistance(vspNode));
  mapper->SetImageSampleDistance(highDef ? 1. : 1.);
  mapper->SetMaxMemoryInBytes(this->GetMaxMemory(mapper, vspNode));

  switch(vspNode->GetRaycastTechnique())
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
}

//---------------------------------------------------------------------------
int vtkMRMLVolumeRenderingDisplayableManager
::IsMapperSupported(vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  return this->IsMapperSupported( this->GetVolumeMapper(vspNode), vspNode);
}

//---------------------------------------------------------------------------
int vtkMRMLVolumeRenderingDisplayableManager
::IsMapperSupported(vtkVolumeMapper* volumeMapper, vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  if (volumeMapper == 0)
    {
    return 0;
    }
  vtkRenderWindow* window = this->GetRenderer()->GetRenderWindow();

  volumeMapper->SetInputData(vtkMRMLScalarVolumeNode::SafeDownCast(
                           vspNode->GetVolumeNode())->GetImageData() );
  int supported = 0;
  if (volumeMapper->IsA("vtkFixedPointVolumeRayCastMapper"))
    {
    supported = 1;
    }
  else if (volumeMapper->IsA("vtkGPUVolumeRayCastMapper"))
    {
    supported = vtkGPUVolumeRayCastMapper::SafeDownCast(volumeMapper)->IsRenderSupported(
      window,vspNode->GetVolumePropertyNode()->GetVolumeProperty());
    }
  //std::cout << vspNode->GetClassName() << " mapper supported: " << supported << std::endl;
  return supported;
}

//---------------------------------------------------------------------------
vtkVolumeMapper* vtkMRMLVolumeRenderingDisplayableManager
::GetVolumeMapper(vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  if (!vspNode)
    {
    return 0;
    }
  if (vspNode->IsA("vtkMRMLCPURayCastVolumeRenderingDisplayNode"))
    {
    return this->MapperRaycast;
    }
  else if (vspNode->IsA("vtkMRMLGPURayCastVolumeRenderingDisplayNode"))
    {
    return this->MapperGPURaycast3;
    }
  return 0;
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager
::SetupMapperFromVolumeNode(vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  vtkMRMLVolumeNode* volumeNode = vspNode ? vspNode->GetVolumeNode() : 0;
  //Add observer to trigger update of transform
  if (volumeNode && !vtkIsObservedMRMLNodeEventMacro(
        volumeNode, vtkMRMLTransformableNode::TransformModifiedEvent))
    {
    vtkNew<vtkIntArray> events;
    events->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);
    events->InsertNextValue(vtkMRMLVolumeNode::ImageDataModifiedEvent);
    events->InsertNextValue(vtkCommand::ModifiedEvent);
    vtkObserveMRMLNodeEventsMacro(volumeNode, events.GetPointer());
    }
  this->SetupMapperFromVolumeNode(volumeNode, this->GetVolumeMapper(vspNode), 0);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager
::SetupMapperFromVolumeNode(vtkMRMLVolumeNode* volumeNode,
                            vtkVolumeMapper* volumeMapper,
                            int index)
{
  if (volumeMapper &&
      volumeMapper->GetNumberOfInputPorts() > index)
    {
    volumeMapper->SetInputConnection(
      index, volumeNode ? volumeNode->GetImageDataConnection() : 0);
    }
}

/*
 * return values:
 * -1: requested mapper not supported
 *  0: invalid input parameter
 *  1: success
 */
//---------------------------------------------------------------------------
int vtkMRMLVolumeRenderingDisplayableManager
::SetupMapperFromParametersNode(vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  vtkMRMLVolumeNode* volumeNode = vspNode ? vspNode->GetVolumeNode() : NULL;
  if (vspNode == NULL || volumeNode == NULL)
    {
    return 0;
    }

  vtkVolumeMapper* volumeMapper = 0;
  vtkVolumeProperty* volumeProperty = 0;

  if ( this->IsMapperSupported(vspNode) &&
       this->UpdateMapper(vspNode))
    {
    volumeMapper = this->GetVolumeMapper(vspNode);
    volumeProperty = vspNode->GetVolumePropertyNode() ?
      vspNode->GetVolumePropertyNode()->GetVolumeProperty() : 0;
    }

  this->Volume->SetMapper(volumeMapper);
  this->Volume->SetProperty(volumeProperty);

  vtkNew<vtkMatrix4x4> matrix;
  this->CalculateMatrix(vspNode, matrix.GetPointer());
  this->Volume->PokeMatrix(matrix.GetPointer());

  this->UpdateDesiredUpdateRate(vspNode);

  return (volumeMapper != 0 ? 1 : -1);
}

//---------------------------------------------------------------------------
bool vtkMRMLVolumeRenderingDisplayableManager::UpdateMapper(
  vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  vtkVolumeMapper* volumeMapper = this->GetVolumeMapper(vspNode);
  if (vspNode->IsA("vtkMRMLCPURayCastVolumeRenderingDisplayNode"))
    {
    this->UpdateCPURaycastMapper(vtkFixedPointVolumeRayCastMapper::SafeDownCast(volumeMapper),
                                 vtkMRMLCPURayCastVolumeRenderingDisplayNode::SafeDownCast(vspNode));
    }
  else if (vspNode->IsA("vtkMRMLGPURayCastVolumeRenderingDisplayNode"))
    {
    this->UpdateGPURaycastMapper(vtkGPUVolumeRayCastMapper::SafeDownCast(volumeMapper),
                                 vtkMRMLGPURayCastVolumeRenderingDisplayNode::SafeDownCast(vspNode));
    }
  else
    {
    return false;
    }
  return true;
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::SetVolumeVisibility(int isVisible)
{
  this->Volume->SetVisibility(isVisible);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::UpdateDesiredUpdateRate(
  vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  vtkRenderWindow* renderWindow = this->GetRenderer()->GetRenderWindow();
  vtkRenderWindowInteractor* renderWindowInteractor =
    renderWindow ? renderWindow->GetInteractor() : 0;
  if (!renderWindowInteractor)
    {
    return;
    }
  double fps = this->GetFramerate(vspNode);
  if (vspNode->GetVisibility())
    {
    if (this->OriginalDesiredUpdateRate == 0.)
      {
      // Save the DesiredUpdateRate before it is changed.
      // It will then be restored when the volume rendering is hidden
      this->OriginalDesiredUpdateRate =
        renderWindowInteractor->GetDesiredUpdateRate();
      }
    renderWindowInteractor->SetDesiredUpdateRate(fps);
    }
  else if (this->OriginalDesiredUpdateRate != 0.)
    {
    // Restore the DesiredUpdateRate to its original value.
    renderWindowInteractor->SetDesiredUpdateRate(
      this->OriginalDesiredUpdateRate);
    this->OriginalDesiredUpdateRate = 0;
    }
  //renderWindowInteractor->SetStillUpdateRate(0.0001);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::UpdateClipping(
  vtkVolumeMapper* volumeMapper,
  vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  if (!volumeMapper)
    {
    return;
    }
  if (!vspNode
      || vspNode->GetROINode() == NULL
      || !vspNode->GetCroppingEnabled())
    {
    volumeMapper->RemoveAllClippingPlanes();
    return;
    }

  vtkNew<vtkPlanes> planes;
  vspNode->GetROINode()->GetTransformedPlanes(planes.GetPointer());
  if ( planes->GetTransform() )
    {
    double zero[3] = {0,0,0};
    double translation[3];
    planes->GetTransform()->TransformPoint(zero, translation);

    // apply the translation to the planes

    int numPlanes = planes->GetNumberOfPlanes();
    vtkPoints *points = planes->GetPoints();
    for (int i=0; i<numPlanes && i<6; i++)
      {
      vtkPlane *plane = planes->GetPlane(i);
      double origin[3];
      plane->GetOrigin(origin);
      points->GetData()->SetTuple3(i,
                                   origin[0]-translation[0],
                                   origin[1]-translation[1],
                                   origin[2]-translation[2]);
      }
    }
  volumeMapper->SetClippingPlanes(planes.GetPointer());
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::TransformModified(vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  if (vspNode == NULL || vspNode != this->DisplayedNode)
    {
    // Only the current DisplayedNode is visible, therefore we can safely ignore all other transform updates.
    // Ideally, event management in the Volume Rendering displayable manager should be refactored to not observe irrelevant events.
    return;
    }
  vtkNew<vtkMatrix4x4> matrix;
  this->CalculateMatrix(vspNode, matrix.GetPointer());
  this->Volume->PokeMatrix(matrix.GetPointer());

  this->VolumeRenderingLogic->FitROIToVolume(vspNode);
}

/*
//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::UpdateVolumePropertyGPURaycast3(vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  if (vspNode->GetCurrentVolumeMapper() == 5)
  {
    this->CreateVolumePropertyGPURaycast3(vspNode);
    this->Volume->SetProperty(this->VolumePropertyGPURaycast3);
  }
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::CreateVolumePropertyGPURaycast3(vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  if (vspNode->GetCurrentVolumeMapper() != 5)
    return;

  if (this->VolumePropertyGPURaycast3 != NULL)
    this->VolumePropertyGPURaycast3->Delete();

  this->VolumePropertyGPURaycast3 = vtkVolumeProperty::New();

  //copy bg property into 1st compoent property
  vtkVolumeProperty* prop = vspNode->GetVolumePropertyNode()->GetVolumeProperty();
  {
    int colorChannels = prop->GetColorChannels(0);

    switch(colorChannels)
    {
    case 1:
      this->VolumePropertyGPURaycast3->SetColor(0, prop->GetGrayTransferFunction(0));
      break;
    case 3:
      this->VolumePropertyGPURaycast3->SetColor(0, prop->GetRGBTransferFunction(0));
      break;
    }

    this->VolumePropertyGPURaycast3->SetScalarOpacity(0, prop->GetScalarOpacity(0));
    this->VolumePropertyGPURaycast3->SetGradientOpacity(0, prop->GetGradientOpacity(0));
    this->VolumePropertyGPURaycast3->SetScalarOpacityUnitDistance(0, prop->GetScalarOpacityUnitDistance(0));

    this->VolumePropertyGPURaycast3->SetDisableGradientOpacity(0, prop->GetDisableGradientOpacity(0));

    this->VolumePropertyGPURaycast3->SetShade(0, prop->GetShade(0));
    this->VolumePropertyGPURaycast3->SetAmbient(0, prop->GetAmbient(0));
    this->VolumePropertyGPURaycast3->SetDiffuse(0, prop->GetDiffuse(0));
    this->VolumePropertyGPURaycast3->SetSpecular(0, prop->GetSpecular(0));
    this->VolumePropertyGPURaycast3->SetSpecularPower(0, prop->GetSpecularPower(0));

    this->VolumePropertyGPURaycast3->SetIndependentComponents(prop->GetIndependentComponents());
    this->VolumePropertyGPURaycast3->SetInterpolationType(prop->GetInterpolationType());
  }

  this->Volume->SetProperty(this->VolumePropertyGPURaycast3);
}
*/

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::OnScenarioNodeModified()
{
  /* Not sure what special should be done when the scenario (not used) is
     modified
  vtkMRMLViewNode *viewNode = this->GetMRMLViewNode();
  if (viewNode)
    {
    this->UpdateDisplayNodeList();
    this->UpdatePipelineFromDisplayNode();
    }
  */
}



//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::OnVolumeRenderingDisplayNodeModified(vtkMRMLVolumeRenderingDisplayNode* dnode)
{
  if (!dnode)
    {
    return;
    }
  if (!this->ValidateDisplayNode(dnode))
    {
    if (dnode == this->DisplayedNode)
      {
      this->DisplayedNode = 0;
      this->RemoveVolumeFromView();
      }
    return;
    }
  bool wasVolumeVisible = this->IsVolumeInView();

  this->UpdatePipelineFromDisplayNode(dnode);

  bool hasVolumeBeenRemoved = !this->IsVolumeInView() && wasVolumeVisible;
  if (dnode->GetVisibility() ||
      hasVolumeBeenRemoved)
    {
    this->RequestRender();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::Create()
{
//  if (this->ProcessingMRMLFlag)
//    {
//    return;
//    }
//  this->ProcessingMRMLFlag = 1;

  this->OnCreate();

//  this->ProcessingMRMLFlag = 0;
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::OnCreate()
{
  vtkMRMLViewNode* viewNode = this->GetMRMLViewNode();
  assert(viewNode);
  if (viewNode && !vtkIsObservedMRMLNodeEventMacro(
        viewNode, vtkMRMLViewNode::GraphicalResourcesCreatedEvent))
    {
    vtkNew<vtkIntArray> events;
    events->InsertNextValue(vtkMRMLViewNode::GraphicalResourcesCreatedEvent);
    vtkObserveMRMLNodeEventsMacro(viewNode, events.GetPointer());
    }

  this->UpdateDisplayNodeList();

  //this->OnVolumeRenderingDisplayNodeModified();
}

//----------------------------------------------------------------------------
bool vtkMRMLVolumeRenderingDisplayableManager::EnterMRMLNodesCallback()const
{
  // Don't support nested calls
  return this->GetInMRMLNodesCallbackFlag() == 1 ? false : true;
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager
::ProcessMRMLNodesEvents(vtkObject *caller,
                         unsigned long event,
                         void *vtkNotUsed(callData))
{
  if (!this->GetMRMLScene())
    {
    return;
    }
  vtkMRMLNode *node = NULL;

  // Observe ViewNode, Scenario Node, and Parameter node for modify events
  if (event == vtkCommand::ModifiedEvent)
    {
    node = reinterpret_cast<vtkMRMLNode *>(caller);

    if (node != NULL && node->IsA("vtkMRMLVolumeRenderingScenarioNode") )
      {
      this->OnScenarioNodeModified();
      }
// on view node modified should be processed in OnMRMLViewNodeModifiedEvent()
//    else if (node != NULL && node == this->GetMRMLViewNode() )
//      {
//      //this->UpdateDisplayNodeList();
//      }
//    else if (node != NULL && node == this->GetDisplayNode() )
//      {
//      this->OnVolumeRenderingDisplayNodeModified();
//      }
    else if (node->IsA("vtkMRMLVolumeRenderingDisplayNode") )
      {
      // We don't update if we are in an interaction mode (
      // vtkCommand::InteractionEvent will be fired, so we can ignore
      // ModifiedEvents)
      if (this->Interaction == 0)
        {
        this->OnVolumeRenderingDisplayNodeModified(
          vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(node));
        }
      }
    else if(vtkMRMLAnnotationROINode::SafeDownCast(caller))
      {
      vtkMRMLAnnotationROINode *roiNode =
        vtkMRMLAnnotationROINode::SafeDownCast(caller);
      vtkMRMLVolumeRenderingDisplayNode* vspNode =
        this->VolumeRenderingLogic->
        GetFirstVolumeRenderingDisplayNodeByROINode(roiNode);

      this->UpdateClipping(this->GetVolumeMapper(vspNode), vspNode);
      this->RequestRender();
      }
    else if(vtkMRMLVolumeNode::SafeDownCast(caller))
      {
        node = reinterpret_cast<vtkMRMLNode *>(caller);
        vtkMRMLVolumeRenderingDisplayNode* vspNode =
          this->VolumeRenderingLogic->GetVolumeRenderingDisplayNodeForViewNode(
          vtkMRMLVolumeNode::SafeDownCast(node),
          this->GetMRMLViewNode());
        // We are notified about the change of all volume nodes that have a VR display node, but
        // we should only request render if the currently displayed volume node is changed
        if (vspNode && vspNode->GetVisibility())
          {
          this->SetupMapperFromParametersNode(vspNode);
          this->RequestRender();
          }
      }
    }
  else if (event == vtkCommand::StartEvent ||
           event == vtkCommand::StartInteractionEvent)
    {
    ++this->Interaction;
    // We request the interactive mode, we might have nested interactions
    // so we just start the mode for the first time.
    if (this->Interaction == 1)
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
    --this->Interaction;
    if (this->Interaction == 0)
      {
      vtkInteractorStyle* interactorStyle = vtkInteractorStyle::SafeDownCast(
        this->GetInteractor()->GetInteractorStyle());
      if (interactorStyle->GetState() == VTKIS_VOLUME_PROPS)
        {
        interactorStyle->StopState();
        }
      if (caller->IsA("vtkMRMLVolumeRenderingDisplayNode"))
        {
        this->OnVolumeRenderingDisplayNodeModified(
          vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(caller));
        }
      }
    }
  else if (event == vtkCommand::InteractionEvent)
    {
    if (caller->IsA("vtkMRMLVolumeRenderingDisplayNode"))
      {
      this->OnVolumeRenderingDisplayNodeModified(
        vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(caller));
      }
    }
  else if (event == vtkMRMLScalarVolumeNode::ImageDataModifiedEvent)
    {
    this->SetupMapperFromVolumeNode(this->DisplayedNode);
    this->RequestRender();
    }
  else if (event == vtkMRMLTransformableNode::TransformModifiedEvent)
    {
    node = reinterpret_cast<vtkMRMLNode *>(caller);
    vtkMRMLVolumeRenderingDisplayNode* vspNode =
      this->VolumeRenderingLogic->GetVolumeRenderingDisplayNodeForViewNode(
        vtkMRMLVolumeNode::SafeDownCast(node),
        this->GetMRMLViewNode());
    this->TransformModified(vspNode);

    this->RequestRender();
    }
  else if (event == vtkMRMLVolumeNode::DisplayModifiedEvent)
    {
    // who calls that ?
    assert(false);
    //vtkMRMLVolumeRenderingDisplayNode* vspNode = this->GetDisplayNode();
    //this->VolumeRenderingLogic->UpdateVolumePropertyFromDisplayNode(vspNode);
    }
  else if (event == vtkMRMLViewNode::GraphicalResourcesCreatedEvent)
    {
    this->RemoveVolumeFromView();
    this->AddVolumeToView();
    }

  // TODO add code from vtkVolumeRenderingGUI::ProcessGUIEvents
  // to observe ParametersNode (ROI, VolumeProp, volumes etc)
}


//initialize pipeline from a loaded or user selected parameters node
//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager
::UpdatePipelineFromDisplayNode(vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  vtkDebugMacro("UpdatePipelineFromDisplayNode ");
  if (!vspNode ||
      !this->ValidateDisplayNode(vspNode))
    {
    vtkDebugMacro("not valid");
    return;
    }

  vtkDebugMacro("valid: " << vspNode->GetClassName());
  //this->RemoveVolumeFromView();

  //this->GetInteractor()->Disable();

  //this->SetupHistograms(vspNode);
  //if (vspNode->GetFgVolumeNode())
  //  this->SetupHistogramsFg(vspNode);

  vtkMRMLAnnotationROINode* roiNode = vspNode->GetROINode();
  if (roiNode)
    {
    roiNode->InsideOutOn();
    }

  //prepare rendering frame
  //this->DeleteRenderingFrame();
  //this->CreateRenderingFrame();

  this->SetupMapperFromVolumeNode(vspNode);
  this->SetupMapperFromParametersNode(vspNode);

  this->DisplayedNode = vspNode;

  if (vspNode->GetVisibility() && this->Volume->GetMapper())
    {
    this->AddVolumeToView();
    }
  else
    {
    this->RemoveVolumeFromView();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::OnInteractorStyleEvent(int eventid)
{
  // This is for the mappers that don't support SetDesiredUpdateRate
  switch(eventid)
    {
    case vtkCommand::EndInteractionEvent:
      //this->SetExpectedFPS(0.0001);
      this->SetupMapperFromParametersNode(this->DisplayedNode);
      break;
    case vtkCommand::StartInteractionEvent:
      this->SetupMapperFromParametersNode(this->DisplayedNode);
      //this->SetExpectedFPS(
      //  this->DisplayedNode ? this->DisplayedNode->GetExpectedFPS() : 15);
      break;
    default:
      break;
    }
  this->Superclass::OnInteractorStyleEvent(eventid);
}

//----------------------------------------------------------------------------
int vtkMRMLVolumeRenderingDisplayableManager::ValidateDisplayNode(vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  if (vspNode == NULL)
    {
    return 0;
    }
  //check all inputs
  if (vspNode->GetVolumeNode() == NULL)
    {
    return 0;
    }
  if (vspNode->GetROINode() == NULL)
    {
    //return 0;
    }
  if (vspNode->GetVolumePropertyNode() == NULL)
    {
    vtkWarningMacro("No volume property node set");
    return 0;
    }

  if (vspNode->GetVolumeNodeID() &&
      strcmp(vspNode->GetVolumeNodeID(), "NULL") != 0 &&
      vspNode->GetVolumeNode() == NULL)
    {
    return 0;
    }
  if (this->GetMRMLViewNode() == NULL)
    {
    return 0;
    }
  if (!vspNode->IsDisplayableInView(this->GetMRMLViewNode()->GetID()))
    {
    return 0;
    }
  return 1;
}

//----------------------------------------------------------------------------
bool vtkMRMLVolumeRenderingDisplayableManager::AddVolumeToView()
{
  bool modified = false;
//  vtkMRMLVolumeRenderingDisplayNode* vspNode = this->GetDisplayNode();

  // Only support 1 volume per view, remove any existing volume
  vtkVolumeCollection *vols = this->GetRenderer()->GetVolumes();
  vols->InitTraversal();
  vtkVolume* firstVolume = vols ? vols->GetNextVolume() : 0;
  if (firstVolume && firstVolume != this->GetVolumeActor())
    {
    this->RemoveVolumeFromView(firstVolume);
    modified = true;
    }

  if (vols != NULL && this->GetVolumeActor() &&
     !vols->IsItemPresent(this->GetVolumeActor()) )
    {
    this->GetRenderer()->AddVolume(this->GetVolumeActor() );
    modified = true;
    }
  return modified;
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::RemoveVolumeFromView()
{
  this->RemoveVolumeFromView(this->GetVolumeActor());
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::RemoveVolumeFromView(vtkVolume* volume)
{
  this->GetRenderer()->RemoveVolume( volume );
}

//----------------------------------------------------------------------------
bool vtkMRMLVolumeRenderingDisplayableManager::IsVolumeInView()
{
  return this->GetRenderer()->HasViewProp(this->GetVolumeActor());
}

//----------------------------------------------------------------------------
bool vtkMRMLVolumeRenderingDisplayableManager::IsVolumeInView(vtkVolume* volume)
{
  return this->GetRenderer()->HasViewProp(volume);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::OnMRMLSceneStartClose()
{
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::OnMRMLSceneEndClose()
{
  // Clean
  this->SetUpdateFromMRMLRequested(1);
  this->RemoveVolumeFromView();
  this->RemoveDisplayNodes();
  this->Reset();

  //this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::OnMRMLSceneEndImport()
{
  // UpdateFromMRML will be executed only if there has been some actions
  // during the import that requested it (don't call
  // SetUpdateFromMRMLRequested(1) here, it should be done somewhere else
  // maybe in OnMRMLSceneNodeAddedEvent, OnMRMLSceneNodeRemovedEvent or
  // OnMRMLDisplayableModelNodeModifiedEvent).
  if (this->GetMRMLViewNode())
    {
    this->OnCreate();
    }
  //this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::OnMRMLSceneEndRestore()
{
  // UpdateFromMRML will be executed only if there has been some actions
  // during the restoration that requested it (don't call
  // SetUpdateFromMRMLRequested(1) here, it should be done somewhere else
  // maybe in OnMRMLSceneNodeAddedEvent, OnMRMLSceneNodeRemovedEvent or
  // OnMRMLDisplayableModelNodeModifiedEvent).
  this->OnCreate();
  //this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (!node->IsA("vtkMRMLVolumeNode") &&
      !node->IsA("vtkMRMLVolumeRenderingDisplayNode") &&
      !node->IsA("vtkMRMLVolumePropertyNode") &&
      !node->IsA("vtkMRMLAnnotationROINode") &&
      !node->IsA("vtkMRMLVolumeRenderingScenarioNode"))
    {
    return;
    }

  // Escape if the scene a scene is being closed, imported or connected
  if (this->GetMRMLScene()->IsBatchProcessing())
    {
    return;
    }

  if (node->IsA("vtkMRMLVolumeRenderingDisplayNode") )
    {
    this->AddDisplayNode(vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(node));
    }

  if (node->IsA("vtkMRMLVolumeRenderingScenarioNode") )
    {
    //remember the newly added scenarioNode
    //vtkMRMLVolumeRenderingScenarioNode *sNode = vtkMRMLVolumeRenderingScenarioNode::SafeDownCast(node);
    this->OnScenarioNodeModified();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  assert(node);

  if (!node->IsA("vtkMRMLVolumeNode") &&
      !node->IsA("vtkMRMLVolumeRenderingDisplayNode") &&
      !node->IsA("vtkMRMLVolumePropertyNode") &&
      !node->IsA("vtkMRMLAnnotationROINode") &&
      !node->IsA("vtkMRMLVolumeRenderingScenarioNode"))
    {
    return;
    }

  // Escape if the scene a scene is being closed, imported or connected
  if (this->GetMRMLScene()->IsBatchProcessing())
    {
    return;
    }

  if (node->IsA("vtkMRMLVolumeRenderingDisplayNode"))
    {
    this->RemoveDisplayNode(vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(node));
    }
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::UpdateDisplayNodeList()
{
  std::vector<vtkMRMLNode *> nodes;
  if (this->GetMRMLScene())
    {
    this->GetMRMLScene()->GetNodesByClass(
      "vtkMRMLVolumeRenderingDisplayNode", nodes);
    }

  // Add display nodes in the scene
  for (unsigned int i = 0; i < nodes.size(); ++i)
    {
    this->AddDisplayNode(
      vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(nodes[i]));
    }
  // Remove display nodes that don't belong to the scene anymore
  for (DisplayNodesType::iterator it = this->DisplayNodes.begin();
       it != this->DisplayNodes.end(); )
    {
    std::vector<vtkMRMLNode *>::iterator isInScene =
      std::find(nodes.begin(), nodes.end(), it->second);
    if (isInScene == nodes.end())
      {
      this->RemoveDisplayNode(it->second);
      it = this->DisplayNodes.begin();
      }
    else
      {
      ++it;
      }
    }
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager
::AddDisplayNode(vtkMRMLVolumeRenderingDisplayNode *dnode)
{
  if (!dnode || this->DisplayNodes.count(std::string(dnode->GetID())) )
    {
    return;
    }
  vtkSetAndObserveMRMLNodeEventsMacro(
    this->DisplayNodes[dnode->GetID()], dnode, this->DisplayObservedEvents);
  this->OnVolumeRenderingDisplayNodeModified(dnode);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager
::RemoveDisplayNode(vtkMRMLVolumeRenderingDisplayNode *dnode)
{
  DisplayNodesType::iterator it = this->DisplayNodes.find(dnode->GetID());
  if (it != this->DisplayNodes.end())
    {
    // unobserve and release
    vtkSetAndObserveMRMLNodeMacro(it->second, 0);
    this->DisplayNodes.erase(it);
    }
  if (dnode == this->DisplayedNode)
    {
    this->DisplayedNode = 0;
    this->RemoveVolumeFromView();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::RemoveDisplayNodes()
{
  while (!this->DisplayNodes.empty())
    {
    this->RemoveDisplayNode(this->DisplayNodes.begin()->second);
    }
}
