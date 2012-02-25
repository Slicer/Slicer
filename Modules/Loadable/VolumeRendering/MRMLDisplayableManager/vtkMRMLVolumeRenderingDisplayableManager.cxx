

// Slicer includes
#include "vtkImageGradientMagnitude.h"
#include "vtkMRMLVolumeRenderingDisplayableManager.h"
#include "vtkSlicerFixedPointVolumeRayCastMapper.h"
#include "vtkSlicerGPURayCastMultiVolumeMapper.h"
#include "vtkSlicerGPURayCastVolumeMapper.h"
#include "vtkSlicerVolumeRenderingLogic.h"
#include "vtkSlicerVolumeTextureMapper3D.h"

// MRML includes
#include "vtkEventBroker.h"
#include "vtkMRMLAnnotationROINode.h"
#include "vtkMRMLLabelMapVolumeDisplayNode.h"
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
#include "vtkVolumeProperty.h"

// ITKSys includes
//#include <itksys/SystemTools.hxx>
//#include <itksys/Directory.hxx>

// STD includes
#include <cassert>
#include <cmath>
#include <algorithm> // for std::min

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLVolumeRenderingDisplayableManager);
vtkCxxRevisionMacro (vtkMRMLVolumeRenderingDisplayableManager, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
bool vtkMRMLVolumeRenderingDisplayableManager::First = true;
int vtkMRMLVolumeRenderingDisplayableManager::DefaultGPUMemorySize = 256;

//---------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayableManager::vtkMRMLVolumeRenderingDisplayableManager()
{
  this->MapperRaycast = NULL;
  this->MapperTexture = NULL;
  this->MapperGPURaycast = NULL;
  this->MapperGPURaycastII = NULL;
  this->MapperGPURaycast3 = NULL;
  this->Volume = NULL;
  this->VolumePropertyGPURaycastII = NULL;
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
  vtkSetMRMLNodeMacro(this->MapperTexture, NULL);
  vtkSetMRMLNodeMacro(this->MapperGPURaycast, NULL);
  vtkSetMRMLNodeMacro(this->MapperGPURaycastII, NULL);
  vtkSetMRMLNodeMacro(this->MapperGPURaycast3, NULL);
  vtkSetMRMLNodeMacro(this->Volume, NULL);
  vtkSetMRMLNodeMacro(this->VolumePropertyGPURaycastII, NULL);
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
  this->MapperTexture->AddObserver(vtkCommand::VolumeMapperComputeGradientsProgressEvent, callback);

  //cpu ray casting
  this->MapperRaycast->AddObserver(vtkCommand::VolumeMapperComputeGradientsProgressEvent, callback);
  this->MapperRaycast->AddObserver(vtkCommand::ProgressEvent,callback);

  //hook up the gpu mapper
  this->MapperGPURaycast->AddObserver(vtkCommand::VolumeMapperComputeGradientsProgressEvent, callback);

  this->MapperGPURaycastII->AddObserver(vtkCommand::VolumeMapperComputeGradientsProgressEvent, callback);
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
  // 3D Texture
  vtkNew<vtkSlicerVolumeTextureMapper3D> newMapperTexture;
  vtkSetAndObserveMRMLNodeEventsMacro(this->MapperTexture,
                                      newMapperTexture.GetPointer(),
                                      mapperEvents.GetPointer());
  // GPU raycast
  vtkNew<vtkSlicerGPURayCastVolumeMapper> newMapperGPURaycast;
  vtkSetAndObserveMRMLNodeEventsMacro(this->MapperGPURaycast,
                                      newMapperGPURaycast.GetPointer(),
                                      mapperEvents.GetPointer());
  // GPU raycast II
  vtkNew<vtkSlicerGPURayCastMultiVolumeMapper> newMapperGPURaycastII;
  vtkNew<vtkIntArray> newMapperGPURaycastIIEvents;
  newMapperGPURaycastIIEvents->InsertNextValue(
    vtkCommand::VolumeMapperComputeGradientsProgressEvent);
  vtkSetAndObserveMRMLNodeEventsMacro(this->MapperGPURaycastII,
                                      newMapperGPURaycastII.GetPointer(),
                                      newMapperGPURaycastIIEvents.GetPointer());
  // GPU raycast 3
  vtkNew<vtkGPUVolumeRayCastMapper> newMapperGPURaycast3;
  vtkSetAndObserveMRMLNodeEventsMacro(this->MapperGPURaycast3,
                                      newMapperGPURaycast3.GetPointer(),
                                      mapperEvents.GetPointer());

  // Volume
  vtkNew<vtkVolume> newVolume;
  vtkSetMRMLNodeMacro(this->Volume, newVolume.GetPointer());

  // Volume Property
  vtkSetMRMLNodeMacro(this->VolumePropertyGPURaycastII, NULL);

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
  grad->SetInput(input);
  grad->Update();

  vtkKWHistogram *gradHisto = vtkKWHistogram::New();
  gradHisto->BuildHistogram(grad->GetOutput()->GetPointData()->GetScalars(), 0);
  this->Histograms->AddHistogram(gradHisto, "0gradient");

  grad->Delete();
  gradHisto->Delete();
****/
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::SetupHistogramsFg(vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  vtkImageData *input = vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetFgVolumeNode())->GetImageData();
  if (input == NULL)
    {
    return;
    }
/****
  //-----------------------------------------
  //  remove old histogram
  //-----------------------------------------
  if(this->HistogramsFg != NULL)
  {
    this->HistogramsFg->RemoveAllHistograms();
    this->HistogramsFg->Delete();
    this->HistogramsFg = vtkKWHistogramSet::New();
  }

  //setup histograms
  this->HistogramsFg->AddHistograms(input->GetPointData()->GetScalars());

  //gradient histogram
  vtkImageGradientMagnitude *grad = vtkImageGradientMagnitude::New();
  grad->SetDimensionality(3);
  grad->SetInput(input);
  grad->Update();

  vtkKWHistogram *gradHisto = vtkKWHistogram::New();
  gradHisto->BuildHistogram(grad->GetOutput()->GetPointData()->GetScalars(), 0);
  this->HistogramsFg->AddHistogram(gradHisto, "0gradient");

  grad->Delete();
  gradHisto->Delete();
  ****/
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::ComputeInternalVolumeSize(int memory)
{
  if (memory <= 128)
    {
    this->MapperGPURaycast->SetInternalVolumeSize(200);
    this->MapperGPURaycastII->SetInternalVolumeSize(200);
    this->MapperGPURaycast3->SetMaxMemoryInBytes(128*1024*1024);
    this->MapperTexture->SetInternalVolumeSize(128);//has to be power-of-two in this mapper
    }
  else if (memory <= 256)
    {
    this->MapperGPURaycast->SetInternalVolumeSize(256);//256^3
    this->MapperGPURaycastII->SetInternalVolumeSize(320);
    this->MapperGPURaycast3->SetMaxMemoryInBytes(256*1024*1024);
    this->MapperTexture->SetInternalVolumeSize(256);
    }
  else if (memory <= 512)
    {
    this->MapperGPURaycast->SetInternalVolumeSize(320);
    this->MapperGPURaycastII->SetInternalVolumeSize(500);
    this->MapperGPURaycast3->SetMaxMemoryInBytes(512*1024*1024);
    this->MapperTexture->SetInternalVolumeSize(256);
    }
  else if (memory <= 1024)
    {
    this->MapperGPURaycast->SetInternalVolumeSize(400);
    this->MapperGPURaycastII->SetInternalVolumeSize(620);
    this->MapperGPURaycast3->SetMaxMemoryInBytes(1024*1024*1024);
    this->MapperTexture->SetInternalVolumeSize(256);
    }
  else if (memory <= 1536)
    {
    this->MapperGPURaycast->SetInternalVolumeSize(460);
    this->MapperGPURaycastII->SetInternalVolumeSize(700);
    this->MapperGPURaycast3->SetMaxMemoryInBytes(1536*1024*1024);
    this->MapperTexture->SetInternalVolumeSize(256);
    }
  else if (memory <= 2048)
    {
    this->MapperGPURaycast->SetInternalVolumeSize(512);
    this->MapperGPURaycastII->SetInternalVolumeSize(775);
    this->MapperGPURaycast3->SetMaxMemoryInBytes(2047*1024*1024);
    this->MapperTexture->SetInternalVolumeSize(512);
    }
  else if (memory <= 3072)
    {
    this->MapperGPURaycast->SetInternalVolumeSize(700);
    this->MapperGPURaycastII->SetInternalVolumeSize(900);
//    this->MapperGPURaycast3->SetMaxMemoryInBytes(3071*1024*1024);
    this->MapperGPURaycast3->SetMaxMemoryInBytes(2047*1024*1024);
    this->MapperTexture->SetInternalVolumeSize(512);
    }
  else if (memory <= 4096)
    {
    this->MapperGPURaycast->SetInternalVolumeSize(800);
    this->MapperGPURaycastII->SetInternalVolumeSize(1000);
//    this->MapperGPURaycast3->SetMaxMemoryInBytes(4095*1024*1024);
    this->MapperGPURaycast3->SetMaxMemoryInBytes(2047*1024*1024);
    this->MapperTexture->SetInternalVolumeSize(512);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::CalculateMatrix(vtkMRMLVolumeRenderingDisplayNode *vspNode, vtkMatrix4x4 *output)
{
  //Update matrix
  //Check for NUll Pointer
  if (vspNode == NULL)
    {
    return;
    }
  vtkMRMLTransformNode *tmp = vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetVolumeNode())->GetParentTransformNode();
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
void vtkMRMLVolumeRenderingDisplayableManager::SetExpectedFPS(double fps)
{
  fps = std::max(fps, 0.0001);

  this->MapperTexture->SetFramerate(fps);
  this->MapperGPURaycast->SetFramerate(fps);
  this->MapperGPURaycastII->SetFramerate(fps);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::SetCPURaycastParameters(vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  switch(vspNode->GetRaycastTechnique())
    {
    case vtkMRMLVolumeRenderingDisplayNode::MaximumIntensityProjection:
      this->MapperRaycast->SetBlendMode(vtkVolumeMapper::MAXIMUM_INTENSITY_BLEND);
      break;
    case vtkMRMLVolumeRenderingDisplayNode::MinimumIntensityProjection:
      this->MapperRaycast->SetBlendMode(vtkVolumeMapper::MINIMUM_INTENSITY_BLEND);
      break;
    case vtkMRMLVolumeRenderingDisplayNode::Composite:
    default:
      this->MapperRaycast->SetBlendMode(vtkVolumeMapper::COMPOSITE_BLEND);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::SetGPURaycastParameters(vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  this->MapperGPURaycast->SetDepthPeelingThreshold(vspNode->GetDepthPeelingThreshold());
  this->MapperGPURaycast->SetDistanceColorBlending(vspNode->GetDistanceColorBlending());
  this->MapperGPURaycast->SetICPEScale(vspNode->GetICPEScale());
  this->MapperGPURaycast->SetICPESmoothness(vspNode->GetICPESmoothness());
  this->MapperGPURaycast->SetTechnique(vspNode->GetRaycastTechnique());
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::SetGPURaycastIIParameters(vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  //ratio may not be used depending on techniques selected
  this->MapperGPURaycastII->SetFgBgRatio(vspNode->GetBgFgRatio());
  this->MapperGPURaycastII->SetTechniques(
    vspNode->GetRaycastTechnique(), vspNode->GetRaycastTechniqueFg());
  this->MapperGPURaycastII->SetColorOpacityFusion(vspNode->GetMultiVolumeFusionMethod());
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::SetGPURaycast3Parameters(vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  switch(vspNode->GetRaycastTechnique())
    {
    case vtkMRMLVolumeRenderingDisplayNode::MaximumIntensityProjection:
      this->MapperGPURaycast3->SetBlendMode(vtkVolumeMapper::MAXIMUM_INTENSITY_BLEND);
      break;
    case vtkMRMLVolumeRenderingDisplayNode::MinimumIntensityProjection:
      this->MapperGPURaycast3->SetBlendMode(vtkVolumeMapper::MINIMUM_INTENSITY_BLEND);
      break;
    case vtkMRMLVolumeRenderingDisplayNode::Composite:
    default:
      this->MapperGPURaycast3->SetBlendMode(vtkVolumeMapper::COMPOSITE_BLEND);
      break;
    }
}

//---------------------------------------------------------------------------
int vtkMRMLVolumeRenderingDisplayableManager::IsCurrentMapperSupported(vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  if (vspNode == NULL)
    return 0;

  vtkRenderWindow* window = this->GetRenderer()->GetRenderWindow();

  switch(vspNode->GetCurrentVolumeMapper())//mapper specific initialization
  {
  case 0:
    return 1;
  case 3:
    {
      vtkSlicerGPURayCastVolumeMapper* MapperGPURaycast = vtkSlicerGPURayCastVolumeMapper::New();

      MapperGPURaycast->SetInput( vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetVolumeNode())->GetImageData() );

      if (MapperGPURaycast->IsRenderSupported(window,vspNode->GetVolumePropertyNode()->GetVolumeProperty()))
      {
        MapperGPURaycast->Delete();
        return 1;
      }
      else
      {
        MapperGPURaycast->Delete();
        return 0;
      }
    }
  case 4:
    {
      vtkSlicerGPURayCastMultiVolumeMapper* MapperGPURaycastII = vtkSlicerGPURayCastMultiVolumeMapper::New();

      MapperGPURaycastII->SetNthInput(0, vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetVolumeNode())->GetImageData());
      if (vspNode->GetFgVolumeNode())
        MapperGPURaycastII->SetNthInput(1, vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetFgVolumeNode())->GetImageData());

      if (MapperGPURaycastII->IsRenderSupported(window, vspNode->GetVolumePropertyNode()->GetVolumeProperty()))
      {
        MapperGPURaycastII->Delete();
        return 1;
      }
      else
      {
        MapperGPURaycastII->Delete();
        return 0;
      }
    }
  case 2:
    {
      vtkSlicerVolumeTextureMapper3D* MapperTexture = vtkSlicerVolumeTextureMapper3D::New();

      MapperTexture->SetInput( vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetVolumeNode())->GetImageData() );

      if (MapperTexture->IsRenderSupported(window, vspNode->GetVolumePropertyNode()->GetVolumeProperty()))
      {
        MapperTexture->Delete();
        return 1;
      }
      else
      {
        MapperTexture->Delete();
        return 0;
      }
    }
  case 1:
    {
    vtkGPUVolumeRayCastMapper* VTKGPURaycast = vtkGPUVolumeRayCastMapper::New();
    VTKGPURaycast->SetInput( vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetVolumeNode())->GetImageData() );
    if (VTKGPURaycast->IsRenderSupported(window,vspNode->GetVolumePropertyNode()->GetVolumeProperty()))
      {
        VTKGPURaycast->Delete();
        return 1;
      }
      else
      {
        VTKGPURaycast->Delete();
        return 0;
      }
    }
  default:
    return 0;
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
  if (vspNode == NULL || vspNode->GetVolumeNode() == NULL)
    {
    return 0;
    }

  double minSpacing = volumeNode->GetMinSpacing() > 0 ?
    volumeNode->GetMinSpacing() : 1.;
  double sampleDistance = minSpacing / vspNode->GetEstimatedSampleDistance();

  vtkRenderWindow* window = this->GetRenderer()->GetRenderWindow();

  switch(vspNode->GetCurrentVolumeMapper())//mapper specific initialization
    {
    case vtkMRMLVolumeRenderingDisplayNode::VTKCPURayCast:
      this->MapperRaycast->SetInput(volumeNode->GetImageData() );
      this->MapperRaycast->SetSampleDistance(sampleDistance);
      this->SetCPURaycastParameters(vspNode);
      this->Volume->SetMapper(this->MapperRaycast);
      if (vspNode->GetVolumePropertyNode())
        {
        this->Volume->SetProperty(vspNode->GetVolumePropertyNode()->GetVolumeProperty());
        }
      break;
    case vtkMRMLVolumeRenderingDisplayNode::NCIGPURayCast:
      this->MapperGPURaycast->SetInput( volumeNode->GetImageData() );
      this->SetGPURaycastParameters(vspNode);
      if ( vspNode->GetVolumePropertyNode() && vspNode->GetVolumePropertyNode()->GetVolumeProperty() )
        {
        if (this->MapperGPURaycast->IsRenderSupported(window,vspNode->GetVolumePropertyNode()->GetVolumeProperty()))
          {
          this->Volume->SetMapper(this->MapperGPURaycast);
          if (vspNode->GetVolumePropertyNode())
            {
            this->Volume->SetProperty(vspNode->GetVolumePropertyNode()->GetVolumeProperty());
            }
          }
        else
          {
          this->Volume->SetMapper(0);
          this->Volume->SetProperty(0);
          }
        }
      else
        {
        return -1;
        }
      break;
    case vtkMRMLVolumeRenderingDisplayNode::NCIGPURayCastMultiVolume:
      this->MapperGPURaycastII->SetNthInput(0, volumeNode->GetImageData());
      if (vspNode->GetFgVolumeNode())
        {
        this->MapperGPURaycastII->SetNthInput(1, vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetFgVolumeNode())->GetImageData());
        }
      this->SetGPURaycastIIParameters(vspNode);
      if (this->MapperGPURaycastII->IsRenderSupported(window, vspNode->GetVolumePropertyNode()->GetVolumeProperty()))
        {
        this->Volume->SetMapper(this->MapperGPURaycastII);
        this->CreateVolumePropertyGPURaycastII(vspNode);
        this->Volume->SetProperty(this->VolumePropertyGPURaycastII);
        }
      else
        {
        this->Volume->SetMapper(0);
        this->Volume->SetProperty(0);
        }
      break;
    case vtkMRMLVolumeRenderingDisplayNode::VTKGPUTextureMapping:
      this->MapperTexture->SetInput( volumeNode->GetImageData() );
      this->MapperTexture->SetSampleDistance(sampleDistance);
      if (this->MapperTexture->IsRenderSupported(window, vspNode->GetVolumePropertyNode()->GetVolumeProperty()))
        {
        this->Volume->SetMapper(this->MapperTexture);
        if (vspNode->GetVolumePropertyNode())
          {
          this->Volume->SetProperty(vspNode->GetVolumePropertyNode()->GetVolumeProperty());
          }
        }
      else
        {
        this->Volume->SetMapper(0);
        this->Volume->SetProperty(0);
        }
      break;
    case vtkMRMLVolumeRenderingDisplayNode::VTKGPURayCast:
      this->MapperGPURaycast3->SetInput(volumeNode->GetImageData());
      this->MapperGPURaycast3->SetSampleDistance(sampleDistance);
      this->SetGPURaycast3Parameters(vspNode);
       if (this->MapperGPURaycast3->IsRenderSupported(window, vspNode->GetVolumePropertyNode()->GetVolumeProperty()))
        {
        this->Volume->SetMapper(this->MapperGPURaycast3);
        if (vspNode->GetVolumePropertyNode())
          {
          this->Volume->SetProperty(vspNode->GetVolumePropertyNode()->GetVolumeProperty());
          }
        }
      else
        {
        this->Volume->SetMapper(0);
        this->Volume->SetProperty(0);
        }
      //else
      //  return -1;
      break;
    default:
      this->Volume->SetMapper(0);
      this->Volume->SetProperty(0);
      break;
    }

  double fps = vspNode->GetExpectedFPS();
  vtkRenderWindow* renderWindow = this->GetRenderer()->GetRenderWindow();
  vtkRenderWindowInteractor* renderWindowInteractor =
    renderWindow ? renderWindow->GetInteractor() : 0;
  if (renderWindowInteractor)
    {
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
  this->SetExpectedFPS(fps);

  // fps is set when interaction occurs
  this->ComputeInternalVolumeSize( vspNode->GetGPUMemorySize() ?
    vspNode->GetGPUMemorySize() :
    vtkMRMLVolumeRenderingDisplayableManager::DefaultGPUMemorySize);

  vtkMatrix4x4 *matrix = vtkMatrix4x4::New();
  this->CalculateMatrix(vspNode, matrix);
  this->Volume->PokeMatrix(matrix);
  matrix->Delete();

  return 1;
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::SetVolumeVisibility(int isVisible)
{
  if (isVisible)
    this->Volume->VisibilityOn();
  else
    this->Volume->VisibilityOff();
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::SetROI(vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  if (!vspNode
      || vspNode->GetROINode() == NULL
      || !vspNode->GetCroppingEnabled())
    {
    this->MapperTexture->RemoveAllClippingPlanes();
    this->MapperRaycast->RemoveAllClippingPlanes();
    this->MapperGPURaycast->RemoveAllClippingPlanes();
    this->MapperGPURaycast->ClippingOff();
    this->MapperGPURaycastII->RemoveAllClippingPlanes();
    this->MapperGPURaycastII->ClippingOff();
    this->MapperGPURaycast3->RemoveAllClippingPlanes();

    return;
    }

  vtkPlanes *planes = vtkPlanes::New();
  vspNode->GetROINode()->GetTransformedPlanes(planes);
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

  this->MapperTexture->SetClippingPlanes(planes);
  this->MapperRaycast->SetClippingPlanes(planes);

  this->MapperGPURaycast->SetClippingPlanes(planes);
  this->MapperGPURaycast->ClippingOn();

  this->MapperGPURaycastII->SetClippingPlanes(planes);
  this->MapperGPURaycastII->ClippingOn();

  this->MapperGPURaycast3->SetClippingPlanes(planes);

  planes->Delete();
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::TransformModified(vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  if (vspNode == NULL)
    {
    return;
    }
  vtkMatrix4x4 *matrix = vtkMatrix4x4::New();
  this->CalculateMatrix(vspNode, matrix);
  this->Volume->PokeMatrix(matrix);

  this->VolumeRenderingLogic->FitROIToVolume(vspNode);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::UpdateVolumePropertyGPURaycastII(vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  if (vspNode->GetCurrentVolumeMapper() == 4)
  {
    this->CreateVolumePropertyGPURaycastII(vspNode);
    this->Volume->SetProperty(this->VolumePropertyGPURaycastII);
  }
}

//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayableManager::CreateVolumePropertyGPURaycastII(vtkMRMLVolumeRenderingDisplayNode* vspNode)
{
  if (vspNode->GetCurrentVolumeMapper() != 4)
    return;

  vtkNew<vtkVolumeProperty> newVolumeProperty;
  vtkSetMRMLNodeMacro(this->VolumePropertyGPURaycastII,
                      newVolumeProperty.GetPointer());

  //copy bg property into 1st compoent property
  vtkVolumeProperty* prop = vspNode->GetVolumePropertyNode()->GetVolumeProperty();
  {
    int colorChannels = prop->GetColorChannels(0);

    switch(colorChannels)
    {
    case 1:
      this->VolumePropertyGPURaycastII->SetColor(0, prop->GetGrayTransferFunction(0));
      break;
    case 3:
      this->VolumePropertyGPURaycastII->SetColor(0, prop->GetRGBTransferFunction(0));
      break;
    }

    this->VolumePropertyGPURaycastII->SetScalarOpacity(0, prop->GetScalarOpacity(0));
    this->VolumePropertyGPURaycastII->SetGradientOpacity(0, prop->GetGradientOpacity(0));
    this->VolumePropertyGPURaycastII->SetScalarOpacityUnitDistance(0, prop->GetScalarOpacityUnitDistance(0));

    this->VolumePropertyGPURaycastII->SetDisableGradientOpacity(0, prop->GetDisableGradientOpacity(0));

    this->VolumePropertyGPURaycastII->SetShade(0, prop->GetShade(0));
    this->VolumePropertyGPURaycastII->SetAmbient(0, prop->GetAmbient(0));
    this->VolumePropertyGPURaycastII->SetDiffuse(0, prop->GetDiffuse(0));
    this->VolumePropertyGPURaycastII->SetSpecular(0, prop->GetSpecular(0));
    this->VolumePropertyGPURaycastII->SetSpecularPower(0, prop->GetSpecularPower(0));

    this->VolumePropertyGPURaycastII->SetIndependentComponents(prop->GetIndependentComponents());
    this->VolumePropertyGPURaycastII->SetInterpolationType(prop->GetInterpolationType());
  }

  if (vspNode->GetUseSingleVolumeProperty())
  {
    vtkVolumeProperty* propFg = vspNode->GetVolumePropertyNode()->GetVolumeProperty();
    int colorChannels = propFg->GetColorChannels(0);

    switch(colorChannels)
    {
    case 1:
      this->VolumePropertyGPURaycastII->SetColor(1, propFg->GetGrayTransferFunction(0));
      break;
    case 3:
      this->VolumePropertyGPURaycastII->SetColor(1, propFg->GetRGBTransferFunction(0));
      break;
    }

    this->VolumePropertyGPURaycastII->SetScalarOpacity(1, propFg->GetScalarOpacity(0));
    this->VolumePropertyGPURaycastII->SetGradientOpacity(1, propFg->GetGradientOpacity(0));
    this->VolumePropertyGPURaycastII->SetScalarOpacityUnitDistance(1, propFg->GetScalarOpacityUnitDistance(0));
    this->VolumePropertyGPURaycastII->SetDisableGradientOpacity(1, propFg->GetDisableGradientOpacity(0));
  }
  else if (vspNode->GetFgVolumePropertyNode())//copy fg property into 2nd component property
  {
    vtkVolumeProperty* propFg = vspNode->GetFgVolumePropertyNode()->GetVolumeProperty();
    int colorChannels = propFg->GetColorChannels(0);

    switch(colorChannels)
    {
    case 1:
      this->VolumePropertyGPURaycastII->SetColor(1, propFg->GetGrayTransferFunction(0));
      break;
    case 3:
      this->VolumePropertyGPURaycastII->SetColor(1, propFg->GetRGBTransferFunction(0));
      break;
    }

    this->VolumePropertyGPURaycastII->SetScalarOpacity(1, propFg->GetScalarOpacity(0));
    this->VolumePropertyGPURaycastII->SetGradientOpacity(1, propFg->GetGradientOpacity(0));
    this->VolumePropertyGPURaycastII->SetScalarOpacityUnitDistance(1, propFg->GetScalarOpacityUnitDistance(0));
    this->VolumePropertyGPURaycastII->SetDisableGradientOpacity(1, propFg->GetDisableGradientOpacity(0));
  }

  this->Volume->SetProperty(this->VolumePropertyGPURaycastII);
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

      this->SetROI(vspNode);
      this->RequestRender();
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
  if (!vspNode ||
      !this->ValidateDisplayNode(vspNode))
    {
    return;
    }

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
  this->SetROI(vspNode);

  //prepare rendering frame
  //this->DeleteRenderingFrame();
  //this->CreateRenderingFrame();


  vtkMRMLVolumeNode *volumeNode = vspNode->GetVolumeNode();
  //Add observer to trigger update of transform
  if (volumeNode && !vtkIsObservedMRMLNodeEventMacro(
        volumeNode, vtkMRMLTransformableNode::TransformModifiedEvent))
    {
    vtkNew<vtkIntArray> events;
    events->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);
    events->InsertNextValue(vtkMRMLScalarVolumeNode::ImageDataModifiedEvent);
    vtkObserveMRMLNodeEventsMacro(volumeNode, events.GetPointer());
    }

  volumeNode = vspNode->GetFgVolumeNode();
  //Add observer to trigger update of transform
  if (volumeNode && !vtkIsObservedMRMLNodeEventMacro(
        volumeNode, vtkMRMLTransformableNode::TransformModifiedEvent))
    {
    vtkNew<vtkIntArray> events;
    events->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);
    events->InsertNextValue(vtkMRMLScalarVolumeNode::ImageDataModifiedEvent);
    vtkObserveMRMLNodeEventsMacro(volumeNode, events.GetPointer());
    }

  this->SetupMapperFromParametersNode(vspNode);

  this->DisplayedNode = vspNode;

  if (vspNode->GetVisibility())
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
      this->SetExpectedFPS(0.0001);
      break;
    case vtkCommand::StartInteractionEvent:
      this->SetExpectedFPS(
        this->DisplayedNode ? this->DisplayedNode->GetExpectedFPS() : 15);
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

  if (vspNode->GetVolumeNodeID() && strcmp(vspNode->GetVolumeNodeID(), "NULL") != 0 && vspNode->GetVolumeNode() == NULL)
    {
    return 0;
    }
  if (vspNode->GetFgVolumeNodeID() && strcmp(vspNode->GetFgVolumeNodeID(), "NULL") != 0 && vspNode->GetFgVolumeNode() == NULL)
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
