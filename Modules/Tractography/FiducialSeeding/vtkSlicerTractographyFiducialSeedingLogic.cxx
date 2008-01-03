/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerTractographyFiducialSeedingLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include <itksys/SystemTools.hxx> 
#include <itksys/Directory.hxx> 

#include "vtkSlicerTractographyFiducialSeedingLogic.h"

#include "vtkDiffusionTensorMathematics.h"
#include "vtkSeedTracts.h"
#include "vtkMath.h"
#include "vtkPointData.h"

#include "vtkMRMLTransformableNode.h"
#include "vtkMRMLDiffusionTensorVolumeNode.h"
#include "vtkMRMLFiducialListNode.h"
#include "vtkMRMLFiberBundleNode.h"
#include "vtkMRMLTransformNode.h"

#include <sstream>

vtkCxxRevisionMacro(vtkSlicerTractographyFiducialSeedingLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerTractographyFiducialSeedingLogic);

//----------------------------------------------------------------------------
vtkSlicerTractographyFiducialSeedingLogic::vtkSlicerTractographyFiducialSeedingLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerTractographyFiducialSeedingLogic::~vtkSlicerTractographyFiducialSeedingLogic()
{
}


//----------------------------------------------------------------------------
void vtkSlicerTractographyFiducialSeedingLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
}

int vtkSlicerTractographyFiducialSeedingLogic::CreateTracts(vtkMRMLDiffusionTensorVolumeNode *volumeNode,
                                                            vtkMRMLFiducialListNode *fiducialListNode,
                                                            vtkMRMLFiberBundleNode *fiberNode,
                                                            const char * stoppingMode, 
                                                            double stoppingValue, 
                                                            double stoppingCurvature, 
                                                            double integrationStepLength)
{
  // 0. check inputs
  if (volumeNode == NULL || fiducialListNode == NULL || fiberNode == NULL ||
      volumeNode->GetImageData() == NULL || fiducialListNode->GetNumberOfFiducials() == 0)
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
  seed->SetInputTensorField(volumeNode->GetImageData());
  
  //2. Set Up matrices
  vtkMRMLTransformNode* vxformNode = volumeNode->GetParentTransformNode();
  vtkMRMLTransformNode* fxformNode = fiducialListNode->GetParentTransformNode();
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

  //Do scale IJK
  double sp[3];
  double spold[3];
  volumeNode->GetSpacing(sp);
  // putr spacing into image so that tractography knows about
  volumeNode->GetImageData()->GetSpacing(spold);
  volumeNode->GetImageData()->SetSpacing(sp);

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
 
  //if (this->StoppingMode && std::string(this->StoppingMode) == std::string("LinearMeasurement"))
  if (stoppingMode && strcmp("Linear Measurement", stoppingMode) != 0)
    {
     streamer->SetStoppingModeToLinearMeasure();
    }
  //else if (this->StoppingMode && std::string(this->StoppingMode) == std::string("FractionalAnisotropy"))
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
  streamer->SetStepLength(integrationStepLength);
  
  // Temp fix to provide a scalar
  seed->GetInputTensorField()->GetPointData()->SetScalars(volumeNode->GetImageData()->GetPointData()->GetScalars());
  
  // loop over fiducials
  int nf = fiducialListNode->GetNumberOfFiducials();
  for (int f=0; f<nf; f++)
    {
    float *xyzf = fiducialListNode->GetNthFiducialXYZ(f);
    float *xyz = transFiducial->TransformFloatPoint(xyzf);
    //Run the thing
    seed->SeedStreamlineFromPoint(xyz[0], xyz[1], xyz[2]);
    }
    
  //6. Extra5ct PolyData in RAS
  vtkPolyData *outFibers = vtkPolyData::New();
  
  seed->TransformStreamlinesToRASAndAppendToPolyData(outFibers);
  
  fiberNode->SetAndObservePolyData(outFibers);
  
  vtkMRMLFiberBundleDisplayNode *dnode = fiberNode->GetLineDisplayNode();
  if (dnode == NULL || oldPoly == NULL)
    {
    dnode = fiberNode->AddLineDisplayNode();
    dnode->SetVisibility(0);
    }
    
  dnode = fiberNode->GetTubeDisplayNode();
  if (dnode == NULL || oldPoly == NULL)
    {
    dnode = fiberNode->AddTubeDisplayNode();
    dnode->SetVisibility(1);
    }
  
  dnode = fiberNode->GetGlyphDisplayNode();
  if (dnode == NULL || oldPoly == NULL)
    {
    dnode = fiberNode->AddGlyphDisplayNode();
    dnode->SetVisibility(0);
    }
  // Restore the original spacing
  volumeNode->GetImageData()->SetSpacing(spold);

  fiberNode->InvokeEvent(vtkMRMLFiberBundleNode::PolyDataModifiedEvent, NULL);
  
  // Delete everything: Still trying to figure out what is going on
  outFibers->Delete();
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
