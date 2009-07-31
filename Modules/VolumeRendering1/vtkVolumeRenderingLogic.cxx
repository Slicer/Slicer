#include "vtkVolumeRenderingLogic.h"
#include "vtkObjectFactory.h"
#include "vtkObject.h"
#include "vtkVolumeProperty.h"
#include "vtkImageData.h"
#include "vtkPointData.h"
#include "vtkMatrix4x4.h"

#include "vtkMRMLVolumeRenderingParametersNode.h"
#include "vtkMRMLTransformNode.h"

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

  this->Volume = vtkVolume::New();
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
      vtkMRMLVolumeRenderingParametersNode *vrpNode=vtkMRMLVolumeRenderingParametersNode::New();
      this->MRMLScene->RegisterNodeClass(vrpNode);
      vrpNode->Delete();
            
      this->First = false;
    }
}


vtkMRMLVolumeRenderingParametersNode* vtkVolumeRenderingLogic::GetParametersNode()
{
  vtkMRMLVolumeRenderingParametersNode *node = NULL;
  if (this->MRMLScene) 
    {
    node = vtkMRMLVolumeRenderingParametersNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass(0, "vtkMRMLVolumeRenderingParametersNode"));
    /**
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
    **/
    }
  return node;
}

//----------------------------------------------------------------------------
void vtkVolumeRenderingLogic::ProcessMRMLEvents(vtkObject *caller, 
                                            unsigned long event, 
                                            void *callData)
{
}

//----------------------------------------------------------------------------
void vtkVolumeRenderingLogic::SetParametersNode(vtkMRMLVolumeRenderingParametersNode *node)
{
  if (node == NULL)
    {
    this->Volume->SetProperty(NULL);
    this->CurrentVolumeMapper->SetInput((vtkImageData *)NULL);
    return;
    }

  //set volume property (transfer function)
  vtkVolumeProperty *volumeProperty = NULL;
  if (node->GetVolumePropertyNode())
    {
    volumeProperty = node->GetVolumePropertyNode()->GetVolumeProperty();
    }
  this->Volume->SetProperty(volumeProperty);

  vtkImageData *imageData = NULL;
  if (node->GetVolumeNode())
    {
    imageData = node->GetVolumeNode()->GetImageData();
    }
 
  std::map<std::string, vtkVolumeMapper *>::iterator iter;
  iter = this->VolumeMappers.find(node->GetCurrentVolumeMapper());
  if (iter != this->VolumeMappers.end())
    {
    this->CurrentVolumeMapper = iter->second;
    }

  this->CurrentVolumeMapper->SetInput(imageData);

  this->Volume->SetMapper(this->CurrentVolumeMapper);


  //Init the mappers

  this->MapperCUDARaycast->SetIntendedFrameRate(node->GetExpectedFPS());
  this->MapperGPURaycast->SetFramerate(node->GetExpectedFPS());

  if (imageData)
    {
    double scalarRange[2];
    imageData->GetPointData()->GetScalars()->GetRange(scalarRange, 0);
    this->MapperGPURaycast->SetDepthPeelingThreshold(scalarRange[0]);
    this->MapperTexture->SetSampleDistance(node->GetEstimatedSampleDistance());
    this->MapperRaycast->SetSampleDistance(node->GetEstimatedSampleDistance());
    }
  this->UpdateTransform(node->GetVolumeNode());
}


double vtkVolumeRenderingLogic::EstimateSampleDistances(vtkImageData *imageData)
{
  if (imageData)
    {
    double *spacing = imageData->GetSpacing();

    double minSpace = spacing[0];
    double maxSpace = spacing[0];
    
    for(int i = 1; i < 3; i++)
    {
      if (spacing[i] > maxSpace)
          maxSpace = spacing[i];
      if (spacing[i] < minSpace)
          minSpace = spacing[i];
    }
    
/*    vtkImageData *imageData = vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetVolumeNodeSelector()->GetSelected())->GetImageData();
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

void vtkVolumeRenderingLogic::GetVolumeIJKToWorldMatrix(vtkMRMLScalarVolumeNode *volumeNode, vtkMatrix4x4 *output)
{
  output->Identity();
  if (volumeNode == NULL)
    {
    return;
    }

  vtkMRMLTransformNode *transformNode = vtkMRMLScalarVolumeNode::SafeDownCast(volumeNode)->GetParentTransformNode();

  //check if we have a TransformNode
  if(transformNode == NULL)
    {
    vtkMRMLScalarVolumeNode::SafeDownCast(volumeNode)->GetIJKToRASMatrix(output);
    return;
    }

  //IJK to ras
  vtkMatrix4x4 *matrix=vtkMatrix4x4::New();
  vtkMRMLScalarVolumeNode::SafeDownCast(volumeNode)->GetIJKToRASMatrix(matrix);
   
  // Parent transforms
  vtkMatrix4x4   *transform=vtkMatrix4x4::New();        
  transformNode->GetMatrixTransformToWorld(transform);

  //Transform world to ras
  vtkMatrix4x4::Multiply4x4(transform,matrix,output);

  matrix->Delete();
  transform->Delete();
}

void vtkVolumeRenderingLogic::UpdateTransform(vtkMRMLScalarVolumeNode *volumeNode)
{
  // update transform
  vtkMatrix4x4 *matrix = vtkMatrix4x4::New();
  this->GetVolumeIJKToWorldMatrix(volumeNode, matrix);
  this->GetVolume()->PokeMatrix(matrix);
  matrix->Delete();
}
