/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerTractographyFiducialSeedingLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

#include "vtkSlicerTractographyFiducialSeedingLogic.h"

// MRML includes
#include "vtkMRMLTractographyFiducialSeedingNode.h"
#include <vtkMRMLTransformableNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLDiffusionTensorVolumeNode.h>
#include <vtkMRMLFiducialListNode.h>
#include <vtkMRMLFiberBundleNode.h>
#include <vtkMRMLFiberBundleStorageNode.h>
#include <vtkMRMLTransformNode.h>

// VTK includes
#include <vtkDiffusionTensorMathematics.h>
#include <vtkSeedTracts.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkMaskPoints.h>
#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>
#include <vtkImageChangeInformation.h>
#include <vtkSmartPointer.h>

// ITKSYS includes
#include <itksys/SystemTools.hxx> 
#include <itksys/Directory.hxx>

// STD includes
#include <sstream>

vtkCxxRevisionMacro(vtkSlicerTractographyFiducialSeedingLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerTractographyFiducialSeedingLogic);

//----------------------------------------------------------------------------
vtkSlicerTractographyFiducialSeedingLogic::vtkSlicerTractographyFiducialSeedingLogic()
{
  this->MaskPoints = vtkMaskPoints::New();
}

//----------------------------------------------------------------------------
vtkSlicerTractographyFiducialSeedingLogic::~vtkSlicerTractographyFiducialSeedingLogic()
{
  this->MaskPoints->Delete();
}


//----------------------------------------------------------------------------
void vtkSlicerTractographyFiducialSeedingLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
}

int vtkSlicerTractographyFiducialSeedingLogic::CreateTracts(vtkMRMLDiffusionTensorVolumeNode *volumeNode,
                                                            vtkMRMLTransformableNode *transformableNode,
                                                            vtkMRMLFiberBundleNode *fiberNode,
                                                            const char * stoppingMode, 
                                                            double stoppingValue, 
                                                            double stoppingCurvature, 
                                                            double integrationStepLength,
                                                            double mnimumPathLength,
                                                            double regionSize, double sampleStep,
                                                            int maxNumberOfSeeds,
                                                            int seedSelectedFiducials,
                                                            int displayMode)
{
  // 0. check inputs
  if (volumeNode == NULL || transformableNode == NULL || fiberNode == NULL ||
      volumeNode->GetImageData() == NULL)
    {
    if (fiberNode && fiberNode->GetPolyData())
      {
      fiberNode->GetPolyData()->Reset();
      }
    return 0;
    }
    
  vtkPolyData *oldPoly = fiberNode->GetPolyData();
   
  vtkSeedTracts *seed = vtkSeedTracts::New();
  
  //1. Set Input
  
  //Do scale IJK
  double sp[3];
  volumeNode->GetSpacing(sp);
  vtkImageChangeInformation *ici = vtkImageChangeInformation::New();
  ici->SetOutputSpacing(sp);
  ici->SetInput(volumeNode->GetImageData());
  ici->GetOutput()->Update();

  seed->SetInputTensorField(ici->GetOutput());
  
  //2. Set Up matrices
  vtkMRMLTransformNode* vxformNode = volumeNode->GetParentTransformNode();
  vtkMRMLTransformNode* fxformNode = transformableNode->GetParentTransformNode();
  vtkMatrix4x4* transformVolumeToFifucial = vtkMatrix4x4::New();
  transformVolumeToFifucial->Identity();
  if (fxformNode != NULL )
    {
    fxformNode->GetMatrixTransformToNode(vxformNode, transformVolumeToFifucial);
    }
  vtkTransform *transFiducial = vtkTransform::New();
  transFiducial->Identity();
  transFiducial->PreMultiply();
  transFiducial->SetMatrix(transformVolumeToFifucial);

  vtkMatrix4x4 *mat = vtkMatrix4x4::New();
  volumeNode->GetRASToIJKMatrix(mat);
  
  vtkMatrix4x4 *TensorRASToIJK = vtkMatrix4x4::New();
  TensorRASToIJK->DeepCopy(mat);
  mat->Delete();


  vtkTransform *trans = vtkTransform::New();
  trans->Identity();
  trans->PreMultiply();
  trans->SetMatrix(TensorRASToIJK);
  // Trans from IJK to RAS
  trans->Inverse();
  // Take into account spacing to compute Scaled IJK
  trans->Scale(1/sp[0],1/sp[1],1/sp[2]);
  trans->Inverse();
  
  //Set Transformation to seeding class
  seed->SetWorldToTensorScaledIJK(trans);
  
  vtkMatrix4x4 *TensorRASToIJKRotation = vtkMatrix4x4::New();
  TensorRASToIJKRotation->DeepCopy(TensorRASToIJK);
  
  //Set Translation to zero
  for (int i=0;i<3;i++)
    {
    TensorRASToIJKRotation->SetElement(i,3,0);
    }
  //Remove scaling in rasToIjk to make a real roation matrix
  double col[3];
  for (int jjj = 0; jjj < 3; jjj++) 
    {
    for (int iii = 0; iii < 3; iii++)
      {
      col[iii]=TensorRASToIJKRotation->GetElement(iii,jjj);
      }
    vtkMath::Normalize(col);
    for (int iii = 0; iii < 3; iii++)
      {
      TensorRASToIJKRotation->SetElement(iii,jjj,col[iii]);
     }  
  }
  TensorRASToIJKRotation->Invert();
  seed->SetTensorRotationMatrix(TensorRASToIJKRotation);  

  //ROI comes from tensor, IJKToRAS is the same
  // as the tensor
  vtkTransform *trans2 = vtkTransform::New();
  trans2->Identity();
  trans2->SetMatrix(TensorRASToIJK);
  trans2->Inverse();
  seed->SetROIToWorld(trans2);
  
  seed->UseVtkHyperStreamlinePoints();
  vtkHyperStreamlineDTMRI *streamer=vtkHyperStreamlineDTMRI::New();
  seed->SetVtkHyperStreamlinePointsSettings(streamer);
  seed->SetMinimumPathLength(mnimumPathLength);

  if ( stoppingMode && 
        ( (strcmp("Linear Measurement", stoppingMode) != 0) || (strcmp("Linear Measure", stoppingMode) != 0) ) )
    {
     streamer->SetStoppingModeToLinearMeasure();
    }
  else if (stoppingMode && strcmp("Fractional Anisotropy", stoppingMode) != 0)
    {  
    streamer->SetStoppingModeToFractionalAnisotropy();
    }
  else
    {
    std::cerr << "No stopping criteria is defined. Using default";
    }
  //streamer->SetMaximumPropagationDistance(this->MaximumPropagationDistance);
  streamer->SetStoppingThreshold(stoppingValue);
  streamer->SetRadiusOfCurvature(stoppingCurvature);
  streamer->SetIntegrationStepLength(integrationStepLength);

  // Temp fix to provide a scalar
  seed->GetInputTensorField()->GetPointData()->SetScalars(volumeNode->GetImageData()->GetPointData()->GetScalars());
  
  vtkMRMLFiducialListNode *fiducialListNode = vtkMRMLFiducialListNode::SafeDownCast(transformableNode);
  vtkMRMLModelNode *modelNode = vtkMRMLModelNode::SafeDownCast(transformableNode);


  // loop over fiducials
  if (fiducialListNode) 
    {
    int nf = fiducialListNode->GetNumberOfFiducials();
    for (int f=0; f<nf; f++)
      {
      if (seedSelectedFiducials && !fiducialListNode->GetNthFiducialSelected(f))
        {
        continue;
        }

      float *xyzf = fiducialListNode->GetNthFiducialXYZ(f);
      for (float x = -regionSize/2.0; x <= regionSize/2.0; x+=sampleStep)
        {
        for (float y = -regionSize/2.0; y <= regionSize/2.0; y+=sampleStep)
          {
          for (float z = -regionSize/2.0; z <= regionSize/2.0; z+=sampleStep)
            {
            float newXYZ[3];
            newXYZ[0] = xyzf[0] + x;
            newXYZ[1] = xyzf[1] + y;
            newXYZ[2] = xyzf[2] + z;
            float *xyz = transFiducial->TransformFloatPoint(newXYZ);
            //Run the thing
            seed->SeedStreamlineFromPoint(xyz[0], xyz[1], xyz[2]);
            }
          }
        }
      }
    }

  // loop over points in the models
  if (modelNode) 
    {
    this->MaskPoints->SetInput(modelNode->GetPolyData());
    this->MaskPoints->SetRandomMode(1);
    this->MaskPoints->SetMaximumNumberOfPoints(maxNumberOfSeeds);
    this->MaskPoints->Update();
    vtkPolyData *mpoly = this->MaskPoints->GetOutput();

    int nf = mpoly->GetNumberOfPoints();
    for (int f=0; f<nf; f++)
      {
      double *xyzf = mpoly->GetPoint(f);

      double *xyz = transFiducial->TransformDoublePoint(xyzf);
           
      //Run the thing
      seed->SeedStreamlineFromPoint(xyz[0], xyz[1], xyz[2]);
      }
    }

    
  //6. Extra5ct PolyData in RAS
  vtkPolyData *outFibers = vtkPolyData::New();
  
  seed->TransformStreamlinesToRASAndAppendToPolyData(outFibers);

  fiberNode->SetAndObservePolyData(outFibers);
  
  vtkMRMLFiberBundleDisplayNode *dnode = fiberNode->GetLineDisplayNode();
  if (dnode == NULL || oldPoly == NULL)
    {
    dnode = fiberNode->AddLineDisplayNode();
    }

  dnode->DisableModifiedEventOn();
  if (displayMode == 0)
    {
    dnode->SetScalarVisibility(1);
    dnode->SetVisibility(1);
    }
  else
    {
    dnode->SetVisibility(0);
    dnode->SetScalarVisibility(0);
    }

  dnode->DisableModifiedEventOff();

  dnode = fiberNode->GetTubeDisplayNode();
  if (dnode == NULL || oldPoly == NULL)
    {
    dnode = fiberNode->AddTubeDisplayNode();
    }

  dnode->DisableModifiedEventOn();
  if (displayMode == 1)
    {
    dnode->SetScalarVisibility(1);
    dnode->SetVisibility(1);
    }
  else
    {
    dnode->SetVisibility(0);
    dnode->SetScalarVisibility(0);
    }
  dnode->DisableModifiedEventOff();

  dnode = fiberNode->GetGlyphDisplayNode();
  if (dnode == NULL || oldPoly == NULL)
    {
    dnode = fiberNode->AddGlyphDisplayNode();
    }

  dnode->DisableModifiedEventOn();
  dnode->SetVisibility(0);
  dnode->SetScalarVisibility(0);
  dnode->DisableModifiedEventOff();

  if (fiberNode->GetStorageNode() == NULL) 
    {
    vtkMRMLFiberBundleStorageNode *storageNode = vtkMRMLFiberBundleStorageNode::New();
    fiberNode->GetScene()->AddNodeNoNotify(storageNode);
    fiberNode->SetAndObserveStorageNodeID(storageNode->GetID());
    storageNode->Delete();
    }

  fiberNode->InvokeEvent(vtkMRMLFiberBundleNode::PolyDataModifiedEvent, NULL);
  
  volumeNode->SetModifiedSinceRead(0);
  fiberNode->SetModifiedSinceRead(1);

  // Delete everything: Still trying to figure out what is going on
  outFibers->Delete();
  ici->Delete();
  seed->Delete();
  TensorRASToIJK->Delete();
  TensorRASToIJKRotation->Delete();
  trans2->Delete();
  trans->Delete();
  streamer->Delete();
  transformVolumeToFifucial->Delete();
  transFiducial->Delete();
  return 1;
}

//----------------------------------------------------------------------------
void vtkSlicerTractographyFiducialSeedingLogic::RegisterNodes()
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
    {
    return;
    }
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLTractographyFiducialSeedingNode>::New());
}
