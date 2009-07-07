#include "vtkVolumeRenderingLogic.h"
#include "vtkObjectFactory.h"
#include "vtkObject.h"
#include "vtkVolumeProperty.h"
#include "vtkImageData.h"
#include "vtkPointData.h"

#include "vtkMRMLVolumeRenderingNode.h"
#include "vtkMRMLVolumeRenderingParametersNode.h"

#include "vtkSlicerVolumeTextureMapper3D.h"
#include "vtkSlicerFixedPointVolumeRayCastMapper.h"
#include "vtkSlicerGPURayCastVolumeTextureMapper3D.h"
#include "vtkCudaVolumeMapper.h"

bool vtkVolumeRenderingLogic::First = true;

vtkVolumeRenderingLogic::vtkVolumeRenderingLogic(void)
{
  //Init the texture mapper
  this->MapperTexture = vtkSlicerVolumeTextureMapper3D::New();
  this->VolumeMappers[std::string("MapperTexture")] = this->MapperTexture;

  //create the CUDA raycast mapper
  this->MapperCUDARaycast = vtkCudaVolumeMapper::New();
  this->VolumeMappers[std::string("MapperCUDARaycast")] = this->MapperCUDARaycast;

  //create the raycast mapper
  this->MapperGPURaycast = vtkSlicerGPURayCastVolumeTextureMapper3D::New();
  this->VolumeMappers[std::string("MapperGPURaycast")] = this->MapperGPURaycast;
 
  
  //Also take care about Ray Cast
  this->MapperRaycast=vtkSlicerFixedPointVolumeRayCastMapper::New();
  this->VolumeMappers[std::string("MapperRaycast")] = this->MapperRaycast;

  this->MapperRaycast->ManualInteractiveOff();      
  this->MapperRaycast->SetImageSampleDistance(1.0f);
  this->MapperRaycast->SetMinimumImageSampleDistance(1.0f);
  this->MapperRaycast->SetMaximumImageSampleDistance(20.0f);

  this->CurrentVolumeMapper = this->MapperRaycast;
  this->Volume->SetMapper(this->CurrentVolumeMapper);
}

vtkVolumeRenderingLogic::~vtkVolumeRenderingLogic(void)
{
}
vtkVolumeRenderingLogic* vtkVolumeRenderingLogic::New()
{
 // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkVolumeRenderingLogic");
  if(ret)
    {
      return (vtkVolumeRenderingLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkVolumeRenderingLogic;
}
void vtkVolumeRenderingLogic::PrintSelf(std::ostream &os, vtkIndent indent)
{
    os<<indent<<"Print logic"<<endl;
}


void vtkVolumeRenderingLogic::SetMRMLScene(vtkMRMLScene *scene)
{
  vtkSlicerModuleLogic::SetMRMLScene(scene);
  this->RegisterNodes();
}
void vtkVolumeRenderingLogic::RegisterNodes()
{
  if (this->MRMLScene && this->First)
    {
      // :NOTE: 20050513 tgl: Guard this so it is only registered once.
      vtkMRMLVolumeRenderingNode *vrNode=vtkMRMLVolumeRenderingNode::New();
      this->MRMLScene->RegisterNodeClass(vrNode);
      vrNode->Delete();

      vtkMRMLVolumeRenderingParametersNode *vrpNode=vtkMRMLVolumeRenderingParametersNode::New();
      this->MRMLScene->RegisterNodeClass(vrpNode);
      vrpNode->Delete();
      
      vtkMRMLVolumeRenderingSelectionNode *vrsNode=vtkMRMLVolumeRenderingSelectionNode::New();
      this->MRMLScene->RegisterNodeClass(vrsNode);
      vrsNode->Delete();
      
      this->First = false;
    }
}

vtkMRMLVolumeRenderingSelectionNode* vtkVolumeRenderingLogic::GetSelectionNode()
{
  vtkMRMLVolumeRenderingSelectionNode *node = NULL;
  if (this->MRMLScene) 
    {
    node = vtkMRMLVolumeRenderingSelectionNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass(0, "vtkMRMLVolumeRenderingSelectionNode"));
    if (node == NULL)
      {
      node = vtkMRMLVolumeRenderingSelectionNode::New();
      vtkMRMLVolumeRenderingSelectionNode *snode = vtkMRMLVolumeRenderingSelectionNode::SafeDownCast(this->MRMLScene->AddNode(node));
      if (snode == node)
        {
        node->Delete();
        }
      node = snode;
      }
    }
  return node;
}

vtkMRMLVolumeRenderingParametersNode* vtkVolumeRenderingLogic::GetParametersNode()
{
  vtkMRMLVolumeRenderingParametersNode *node = NULL;
  if (this->MRMLScene) 
    {
    node = vtkMRMLVolumeRenderingParametersNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass(0, "vtkMRMLVolumeRenderingParametersNode"));
    if (node == NULL)
      {
      node = vtkMRMLVolumeRenderingParametersNode::New();
      vtkMRMLVolumeRenderingParametersNode *snode = vtkMRMLVolumeRenderingParametersNode::SafeDownCast(this->MRMLScene->AddNode(node));
      if (snode == node)
        {
        node->Delete();
        }
      node = snode;
      }
    }
  return node;
}

//----------------------------------------------------------------------------
void vtkVolumeRenderingLogic::ProcessMRMLEvents(vtkObject *caller, 
                                            unsigned long event, 
                                            void *callData)
{
  //set volume property (transfer function)
  vtkVolumeProperty *volumeProperty = NULL;
  if (this->GetParametersNode() && this->GetParametersNode()->GetVolumePropertyNode())
    {
    volumeProperty = this->GetParametersNode()->GetVolumePropertyNode()->GetVolumeProperty();
    }
  this->Volume->SetProperty(volumeProperty);

  vtkImageData *imageData = NULL;
  if (this->GetParametersNode() && this->GetParametersNode()->GetVolumeNode())
    {
    imageData = this->GetParametersNode()->GetVolumeNode()->GetImageData();
    }

  if (this->GetParametersNode())
    {
    std::map<std::string, vtkVolumeMapper *>::iterator iter;
    iter = this->VolumeMappers.find(this->GetParametersNode()->GetCurrentVolumeMapper());
    if (iter != this->VolumeMappers.end())
      {
      this->CurrentVolumeMapper = iter->second;
      }
    }

  this->CurrentVolumeMapper->SetInput(imageData);

  this->Volume->SetMapper(this->CurrentVolumeMapper);


  //Init the mappers
  if (this->GetParametersNode()) 
    {
    this->MapperTexture->SetSampleDistance(this->GetParametersNode()->GetEstimatedSampleDistance());
    this->MapperCUDARaycast->SetIntendedFrameRate(this->GetParametersNode()->GetExpectedFPS());
    this->MapperGPURaycast->SetFramerate(this->GetParametersNode()->GetExpectedFPS());
    this->MapperRaycast->SetSampleDistance(this->GetParametersNode()->GetEstimatedSampleDistance());
    }
    //create the CUDA raycast mapper

  if (imageData)
    {
    double scalarRange[2];
    imageData->GetPointData()->GetScalars()->GetRange(scalarRange, 0);
    this->MapperGPURaycast->SetDepthPeelingThreshold(scalarRange[0]);
    }
}


double vtkVolumeRenderingLogic::EstimateSampleDistances(void)
{
  if (this->GetParametersNode() && this->GetParametersNode()->GetVolumeNode())
    {
    double *spacing = vtkMRMLScalarVolumeNode::SafeDownCast(this->GetParametersNode()->GetVolumeNode())->GetSpacing();

    double minSpace = spacing[0];
    double maxSpace = spacing[0];
    
    for(int i = 1; i < 3; i++)
    {
      if (spacing[i] > maxSpace)
          maxSpace = spacing[i];
      if (spacing[i] < minSpace)
          minSpace = spacing[i];
    }
    
/*    vtkImageData *imageData = vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData();
    int *dims = imageData->GetDimensions();
    int minDim = dims[0];
    minDim = minDim > dims[1] ? dims[1] : minDim;
    minDim = minDim > dims[2] ? dims[2] : minDim;
*/    
    return minSpace * 0.5f;
    }
  else {
    return 1.0;
    }
}

