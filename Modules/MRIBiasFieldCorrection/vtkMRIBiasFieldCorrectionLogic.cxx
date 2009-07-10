/*=auto==============================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All
Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRIBiasFieldCorrectionLogic.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $
Author:    $Nicolas Rannou (BWH), Sylvain Jaume (MIT)$

==============================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkMRIBiasFieldCorrectionLogic.h"
#include "vtkMRIBiasFieldCorrection.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLScalarVolumeNode.h"

#include "vtkExtractVOI.h"
#include "vtkImageThreshold.h"
#include "vtkImageClip.h"
#include "vtkImageReslice.h"

#include "vtkDataArray.h"
#include "vtkObjectFactory.h"
#include "vtkCellData.h"
#include "vtkPointData.h"
#include "vtkGenericAttribute.h"

////////////////////////////////////////////////////////
#include "vtkSlicerSliceControllerWidget.h"
#include "vtkSlicerSlicesControlGUI.h"
#include "vtkKWScale.h"

////////////////////////////////////////////////////////
#include "itkImage.h"
#include "itkCastImageFilter.h"
#include "itkImageFileReader.h"

#include "itkN3MRIBiasFieldCorrectionImageFilter.h"
#include "itkBSplineControlPointImageFilter.h"

#include "itkBinaryThresholdImageFilter.h"

#include "itkBSplineControlPointImageFilter.h"
#include "itkExpImageFilter.h"
#include "itkImageRegionIterator.h"
#include "itkShrinkImageFilter.h"

#include "itkImageToVTKImageFilter.h"
#include "itkVTKImageToImageFilter.h"

#include "vtkImageCast.h"
////////////////////////////////////////////////////////

#include "vtkSlicerSliceControllerWidget.h"
#include "vtkSlicerSlicesControlGUI.h"
#include "vtkKWScale.h"
#include "vtkMRIBiasFieldCorrectionGUI.h"

///////////////////////////////////////////////////////


vtkMRIBiasFieldCorrectionLogic* vtkMRIBiasFieldCorrectionLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance(
    "vtkMRIBiasFieldCorrectionLogic");

  if(ret)
    {
    return (vtkMRIBiasFieldCorrectionLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRIBiasFieldCorrectionLogic;
}

//-------------------------------------------------------------------
vtkMRIBiasFieldCorrectionLogic::vtkMRIBiasFieldCorrectionLogic()
{
  this->MRIBiasFieldCorrectionNode = NULL;
}

//-------------------------------------------------------------------
vtkMRIBiasFieldCorrectionLogic::~vtkMRIBiasFieldCorrectionLogic()
{
  vtkSetMRMLNodeMacro(this->MRIBiasFieldCorrectionNode, NULL);
}

//-------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionLogic::PrintSelf(ostream& os, vtkIndent
  indent)
{
}

//-------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionLogic::Apply()
{
  // check if MRML node is present
  if (this->MRIBiasFieldCorrectionNode == NULL)
    {
    vtkErrorMacro(
      "No input GradientAnisotropicDiffusionFilterNode found");
    return;
    }

  // find input volume
  vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::
    SafeDownCast(this->GetMRMLScene()->GetNodeByID(
    this->MRIBiasFieldCorrectionNode->GetInputVolumeRef()));

  if (inVolume == NULL)
    {
    vtkErrorMacro("No input volume found");
    return;
    }

  // find mask volume
  vtkMRMLScalarVolumeNode *maskVolume = vtkMRMLScalarVolumeNode::
    SafeDownCast(this->GetMRMLScene()->GetNodeByID(
    this->MRIBiasFieldCorrectionNode->GetMaskVolumeRef()));

  if (maskVolume == NULL)
    {
    vtkErrorMacro("No mask volume found with id= " <<
      this->MRIBiasFieldCorrectionNode->GetMaskVolumeRef());
    return;
    }

  // find output volume
  vtkMRMLScalarVolumeNode *outVolume = vtkMRMLScalarVolumeNode::
    SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->
    MRIBiasFieldCorrectionNode->GetOutputVolumeRef()));

  if (outVolume == NULL)
    {
    vtkErrorMacro("No output volume found with id= " <<
      this->MRIBiasFieldCorrectionNode->GetOutputVolumeRef());
    return;
    }

  // copy RASToIJK matrix, and other attributes from input to output
  std::string name (outVolume->GetName());
  std::string id (outVolume->GetID());

  outVolume->CopyOrientation(inVolume);
  outVolume->SetAndObserveTransformNodeID(inVolume->
    GetTransformNodeID());

  outVolume->SetName(name.c_str());

  typedef itk::Image< float, 3 >        InputImageType;
  typedef itk::Image< unsigned char, 3> MaskImageType;
  typedef itk::Image< float, 3 >        OutputImageType;
  typedef itk::Image< float, 3 >        InternalImageType;

  typedef itk::ImageFileReader< InputImageType > ReaderType;

  ReaderType::Pointer reader = ReaderType::New();
  // input volume conversion
  vtkImageCast* imageCaster = vtkImageCast::New();
  imageCaster->SetInput(inVolume->GetImageData());
  imageCaster->SetOutputScalarTypeToFloat();
  imageCaster->Modified();
  imageCaster->Update();

  typedef itk::VTKImageToImageFilter< InternalImageType >

  VTK2ITKConnectorFilterType;
  VTK2ITKConnectorFilterType::Pointer VTK2ITKconnector =
    VTK2ITKConnectorFilterType::New();
  VTK2ITKconnector->SetInput( imageCaster->GetOutput() );
  VTK2ITKconnector->GetImporter()->Update();
  VTK2ITKconnector->Update();

  // mask volume conversion
  vtkImageCast* imageCasterM = vtkImageCast::New();
  imageCasterM->SetInput(maskVolume->GetImageData());
  imageCasterM->SetOutputScalarTypeToFloat();
  imageCasterM->Modified();
  imageCasterM->Update();

  typedef itk::VTKImageToImageFilter< InternalImageType >
    VTK2ITKConnectorFilterTypeM;
  VTK2ITKConnectorFilterTypeM::Pointer VTK2ITKconnectorM =
    VTK2ITKConnectorFilterTypeM::New();
  VTK2ITKconnectorM->SetInput( imageCasterM->GetOutput() );
  VTK2ITKconnectorM->GetImporter()->Update();
  VTK2ITKconnectorM->Update();

  // processing
  typedef itk::ShrinkImageFilter<InputImageType, InputImageType>
    ShrinkerType;
  ShrinkerType::Pointer shrinker = ShrinkerType::New();
  shrinker->SetInput( VTK2ITKconnector->GetOutput() );
  shrinker->SetShrinkFactors( 1 );

  typedef itk::BinaryThresholdImageFilter<InputImageType,
    MaskImageType> mFilterType;

  mFilterType::Pointer mfilter = mFilterType::New();
  mfilter->SetInput( VTK2ITKconnectorM->GetOutput() );

  mfilter->SetLowerThreshold(1);
  mfilter->SetOutsideValue(0);
  mfilter->SetInsideValue(1);
  mfilter->UpdateLargestPossibleRegion();

  MaskImageType::Pointer maskImage = NULL;

  maskImage = mfilter->GetOutput();
  typedef itk::ShrinkImageFilter<MaskImageType, MaskImageType>
    MaskShrinkerType;
  MaskShrinkerType::Pointer maskshrinker = MaskShrinkerType::New();
  maskshrinker->SetInput( maskImage );
  maskshrinker->SetShrinkFactors( 1 );

  shrinker->SetShrinkFactors( (unsigned int)round(this->MRIBiasFieldCorrectionNode->GetShrink()));
  maskshrinker->SetShrinkFactors( (unsigned int)round(this->MRIBiasFieldCorrectionNode->GetShrink()));

  shrinker->Update();
  shrinker->UpdateLargestPossibleRegion();

  maskshrinker->Update();
  maskshrinker->UpdateLargestPossibleRegion();

  typedef itk::N3MRIBiasFieldCorrectionImageFilter<InputImageType,
    MaskImageType, InputImageType> CorrecterType;
  CorrecterType::Pointer correcter = CorrecterType::New();
  correcter->SetInput( shrinker->GetOutput() );
  correcter->SetMaskImage( maskshrinker->GetOutput() );

  correcter->SetMaximumNumberOfIterations(
              (unsigned int)round(this->MRIBiasFieldCorrectionNode->GetMax()));

  correcter->SetNumberOfFittingLevels(
              (unsigned int)round(this->MRIBiasFieldCorrectionNode->GetNum()));

  correcter->SetWeinerFilterNoise(this->MRIBiasFieldCorrectionNode->
    GetWien());

  correcter->SetBiasFieldFullWidthAtHalfMaximum(
    this->MRIBiasFieldCorrectionNode->GetField());

  correcter->SetConvergenceThreshold(this->MRIBiasFieldCorrectionNode
    ->GetCon());

  correcter->Update();

  typedef itk::BSplineControlPointImageFilter<
    CorrecterType::BiasFieldControlPointLatticeType,
    CorrecterType::ScalarImageType> BSplinerType;

  BSplinerType::Pointer bspliner = BSplinerType::New();
  bspliner->SetInput( correcter->GetBiasFieldControlPointLattice() );
  bspliner->SetSplineOrder( correcter->GetSplineOrder() );
  bspliner->SetSize(
    VTK2ITKconnector->GetOutput()->GetLargestPossibleRegion().GetSize() );
  bspliner->SetOrigin( VTK2ITKconnector->GetOutput()->GetOrigin() );
  bspliner->SetDirection( VTK2ITKconnector->GetOutput()->GetDirection() );
  bspliner->SetSpacing( VTK2ITKconnector->GetOutput()->GetSpacing() );
  bspliner->Update();

  correcter->SetConvergenceThreshold(this->MRIBiasFieldCorrectionNode
    ->GetCon());

  InputImageType::Pointer logField = InputImageType::New();
  logField->SetOrigin( bspliner->GetOutput()->GetOrigin() );
  logField->SetSpacing( bspliner->GetOutput()->GetSpacing() );
  logField->SetRegions(
    bspliner->GetOutput()->GetLargestPossibleRegion().GetSize() );
  logField->SetDirection( bspliner->GetOutput()->GetDirection() );
  logField->Allocate();

  correcter->SetConvergenceThreshold(this->MRIBiasFieldCorrectionNode->GetCon());

  itk::ImageRegionIterator<CorrecterType::ScalarImageType> ItB(
    bspliner->GetOutput(),
    bspliner->GetOutput()->GetLargestPossibleRegion() );
  itk::ImageRegionIterator<InputImageType> ItF( logField,
    logField->GetLargestPossibleRegion() );

  for( ItB.GoToBegin(), ItF.GoToBegin(); !ItB.IsAtEnd(); ++ItB, ++ItF )
    {
    ItF.Set( ItB.Get()[0] );
    }

  correcter->SetConvergenceThreshold(this->MRIBiasFieldCorrectionNode
    ->GetCon());

  typedef itk::ExpImageFilter<InputImageType, InputImageType>
    ExpFilterType;
  ExpFilterType::Pointer expFilter = ExpFilterType::New();
  expFilter->SetInput( logField );
  expFilter->Update();

  correcter->SetConvergenceThreshold(this->MRIBiasFieldCorrectionNode
    ->GetCon());

  typedef itk::DivideImageFilter<InputImageType, InputImageType,
    InputImageType> DividerType;
  DividerType::Pointer divider = DividerType::New();
  divider->SetInput1( VTK2ITKconnector->GetOutput() );
  divider->SetInput2( expFilter->GetOutput() );
  divider->Update();

  typedef itk::ImageToVTKImageFilter< InternalImageType >
    ITK2VTKConnectorFilterType;
  ITK2VTKConnectorFilterType::Pointer ITK2VTKconnector =
  ITK2VTKConnectorFilterType::New();
  ITK2VTKconnector->GetExporter()->SetInput(divider->GetOutput());
  ITK2VTKconnector->GetImporter()->Update();

  vtkImageData* image = vtkImageData::New();
  image->DeepCopy( ITK2VTKconnector->GetImporter()->GetOutput());
  outVolume->SetAndObserveImageData(image);
  image->Delete();
  outVolume->SetModifiedSinceRead(1);
}

void vtkMRIBiasFieldCorrectionLogic::SliceProcess(
  vtkTransform* xyToijk,double dim0,double dim1)
{
  // check if MRML node is present
  if (this->MRIBiasFieldCorrectionNode == NULL)
    {
    vtkErrorMacro("No input MRIBiasFieldCorrectionNode found");
    return;
    }

  // find input volume
  vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::
    SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->
      MRIBiasFieldCorrectionNode->GetInputVolumeRef()));

  if (inVolume == NULL)
    {
    vtkErrorMacro("No input volume found");
    return;
    }

  // create storage volume for preview
  vtkMRMLScalarVolumeNode *stoVolume = vtkMRMLScalarVolumeNode::
    SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->
      MRIBiasFieldCorrectionNode->GetStorageVolumeRef()));

  if (stoVolume == NULL)
    {
    vtkErrorMacro("No storage volume found with id= " << this->
        MRIBiasFieldCorrectionNode->GetStorageVolumeRef());
    return;
    }

  // create Mask volume for processing
  vtkMRMLScalarVolumeNode *maskVolume = vtkMRMLScalarVolumeNode::
    SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->
      MRIBiasFieldCorrectionNode->GetMaskVolumeRef()));

  if( maskVolume == NULL )
    {
    vtkErrorMacro("No mask volume found with id= " <<
      this->MRIBiasFieldCorrectionNode->GetMaskVolumeRef());
    return;
    }

  // create output volume for preview
  vtkMRMLScalarVolumeNode *outVolume = vtkMRMLScalarVolumeNode::
    SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->
      MRIBiasFieldCorrectionNode->GetOutputVolumeRef()));

  if (outVolume == NULL)
    {
    vtkErrorMacro("No output volume found with id= " <<
      this->MRIBiasFieldCorrectionNode->GetOutputVolumeRef());
    return;
    }

  // copy RASToIJK matrix, and other attributes from input to output
  std::string name (stoVolume->GetName());
  std::string id (stoVolume->GetID());

  stoVolume->CopyOrientation(inVolume);
  stoVolume->SetAndObserveTransformNodeID(inVolume->GetTransformNodeID());

  stoVolume->SetName(name.c_str());

  outVolume->CopyOrientation(inVolume);
  outVolume->SetAndObserveTransformNodeID(inVolume->GetTransformNodeID());

  outVolume->SetName(name.c_str());

  this->STORAGE = vtkImageData::New();
  this->PREVIEW = vtkImageData::New();

  this->STORAGE->DeepCopy(inVolume->GetImageData());
  this->PREVIEW->DeepCopy(inVolume->GetImageData());

  //GET SIZE OF THE ARRAY TO BE PROCESSED AND THE FIRST PIXEL IN FRAME

  double size1 = 0;
  double size2 = 0;
  double xyPt[4];
  double ijkPt[3];
  int begin[2];

  xyPt[1] = round(dim1/2);
  xyPt[2] = 0;
  xyPt[3] = 1;

  int* extent = inVolume->GetImageData()->GetWholeExtent();

  for(int i = 0; i < dim0; i++)
    {
    xyPt[0] = round(i);
    xyToijk->MultiplyPoint(xyPt,ijkPt);

    if( ijkPt[0]<0 || ijkPt[0]>=extent[1] ||ijkPt[1]<0 ||
        ijkPt[1]>=extent[3] ||ijkPt[2]<0 || ijkPt[2]>=extent[5] )
      {
      // OUT OF VOI
      }
    else
      {
      if(size1 == 0)
        {
        begin[0] = i;
        }
      size1++;
      }
    }

  xyPt[0] = round(dim0/2);

  for(int i = 0; i < dim1; i++)
    {
    xyPt[1] = round(i);
    xyToijk->MultiplyPoint(xyPt,ijkPt);

  if(ijkPt[0]<0 || ijkPt[0]>=extent[1] ||ijkPt[1]<0 ||
    ijkPt[1]>=extent[3] ||ijkPt[2]<0 || ijkPt[2]>=extent[5] )
    {
    // OUT OF VOI
    }
  else
    {
    if(size2 == 0)
      {
      begin[1] = i;
      }
  size2++;
  }
  }

  // GET BOUNDS OF THE ARRAY IN IJK
  int size[6];

  xyPt[0] = size1;
  xyPt[1] = 0;
  xyPt[2] = 0;
  xyPt[3] = 1;

  double ijkPt1[3];
  double ijkPt2[3];
  double ijkPt3[3];

  xyToijk->MultiplyPoint(xyPt,ijkPt1);

  xyPt[0] = 0;
  xyPt[1] = size2;
  xyPt[2] = 0;
  xyPt[3] = 1;

  xyToijk->MultiplyPoint(xyPt,ijkPt2);

  xyPt[0] = 0;
  xyPt[1] = 0;
  xyPt[2] = 0;
  xyPt[3] = 1;

  xyToijk->MultiplyPoint(xyPt,ijkPt3);

  size[0] = (int)round(sqrt((ijkPt1[0]-ijkPt3[0])*(ijkPt1[0]-ijkPt3[0])));
  size[1] = (int)round(sqrt((ijkPt2[0]-ijkPt3[0])*(ijkPt2[0]-ijkPt3[0])));
  size[2] = (int)round(sqrt((ijkPt1[1]-ijkPt3[1])*(ijkPt1[1]-ijkPt3[1])));
  size[3] = (int)round(sqrt((ijkPt2[1]-ijkPt3[1])*(ijkPt2[1]-ijkPt3[1])));
  size[4] = (int)round(sqrt((ijkPt1[2]-ijkPt3[2])*(ijkPt1[2]-ijkPt3[2])));
  size[5] = (int)round(sqrt((ijkPt2[2]-ijkPt3[2])*(ijkPt2[2]-ijkPt3[2])));

  int compt = 0;
  int pos[2];

  for(int i = 0; i < 6; i++)
    {
    if (size[i] > 0)
      {
      size[compt] = size[i];
      if(i<2)
      {
        pos[compt] = 0;
      }
      else if(i>1 && i<4)
      {
        pos[compt] = 1;
      }
      else if(i>3)
      {
        pos[compt] = 2;
        compt ++;
      }
   }
 }

  double populateXY[4];
  double populateIJK[4];

  populateXY[2] = 0;
  populateXY[3] = 1;

  //GET EVOLUTION TO POPULATE (POSITION INCREMENT OR DECREMENT)
  double direction[4][2];
  int start = begin[0]+begin[1];

  for(int i = begin[0]; i < begin[0]+2; i++)
    {
    for(int j = begin[1]; j < begin[1]+2; j++)
      {
      populateXY[0] = i;
      populateXY[1] = j;

      xyToijk->MultiplyPoint(populateXY,populateIJK);

      direction[i+j-start][0] = populateIJK[pos[0]];
      direction[i+j-start][1] = populateIJK[pos[1]];
      }
    }

  double evolution[2];

  if(direction[0][0] == direction [1][0])
    {
    if(direction[0][0]-direction[2][0] < 0)
      {
      evolution[0] = 1;
      }
    else
      {
      evolution[0] = 0;
      }
    }
  else
    {
    if(direction[0][0]-direction[1][0] < 0)
      {
      evolution[0] = 1;
      }
    else
      {
      evolution[0] = 0;
      }
    }

  if(direction[0][1] == direction [1][1])
    {
    if(direction[0][1]-direction[2][1] < 0)
      {
      evolution[1] = 1;
      }
    else
      {
      evolution[1] = 0;
      }
    }
    else
    {
    if(direction[0][1] - direction[1][1] < 0)
      {
      evolution[1] = 1;
      }
    else
      {
      evolution[1] = 0;
      }
    }

  xyPt[0] = begin[0];
  xyPt[1] = begin[1];
  xyPt[2] = 0;
  xyPt[3] = 1;

  double originIJK[3];

  xyToijk->MultiplyPoint(xyPt,originIJK);

  // POPULATE THE CORRESPONDING ARRAY AND MASK

  int wholeExtent[6];

  wholeExtent[0] = 0;
  wholeExtent[2] = 0;
  wholeExtent[4] = 0;
  wholeExtent[5] = 0;

  wholeExtent[1] = size[0]-1;
  wholeExtent[3] = size[1]-1;

  this->STORAGE = vtkImageData::New();
  this->STORAGE->SetWholeExtent( wholeExtent );
  this->STORAGE->SetNumberOfScalarComponents(1);
  this->STORAGE->SetOrigin(0.0,0.0,0.0);
  this->STORAGE->SetDimensions(size[0],size[1],1);
  this->STORAGE->AllocateScalars();

  vtkDataArray* outStorage=this->STORAGE->GetPointData()->GetScalars();

  this->MASK = vtkImageData::New();
  this->MASK->SetWholeExtent( wholeExtent );
  this->MASK->SetNumberOfScalarComponents(1);
  this->MASK->SetOrigin(0.0,0.0,0.0);
  this->MASK->SetDimensions(size[0],size[1],1);
  this->MASK->AllocateScalars();

  vtkDataArray* outMask=this->MASK->GetPointData()->GetScalars();

  // ARRAY EXTRACTION

  if(pos[0]==0 && pos[1] == 1){
  if(evolution[0] == 0 && evolution[1] == 0)
    {
    for (int j=0;j<size[0];j++)
      {
      for (int i=0;i<size[1];i++)
        {
        outStorage->SetComponent(i*(size[0])+j,0,
            inVolume->GetImageData()->GetScalarComponentAsDouble(
              (int)round(originIJK[0]-j),(int)round(originIJK[1]-i),(int)round(originIJK[2]),0));

        outMask->SetComponent(i*(size[0])+j,0,
            maskVolume->GetImageData()->GetScalarComponentAsDouble(
              (int)round(originIJK[0]-j),(int)round(originIJK[1]-i),(int)round(originIJK[2]),0));
        }
      }
    }

  if(evolution[0] == 0 && evolution[1] == 1)
    {
    for(int j=0;j<size[0];j++)
      {
      for(int i=0;i<size[1];i++)
        {
        outStorage->SetComponent(i*(size[0])+j,0,
          inVolume->GetImageData()->GetScalarComponentAsDouble(
            (int)round(originIJK[0]-j),(int)round(originIJK[1]+i),(int)round(originIJK[2]),0));

        outMask->SetComponent(i*(size[0])+j,0,
            maskVolume->GetImageData()->GetScalarComponentAsDouble(
              (int)round(originIJK[0]-j),(int)round(originIJK[1]+i),(int)round(originIJK[2]),0));
        }
      }
    }

  if(evolution[0] == 1 && evolution[1] == 0)
    {
    for (int j=0;j<size[0];j++)
      {
      for (int i=0;i<size[1];i++)
        {
        outStorage->SetComponent(i*(size[0])+j,0,
          inVolume->GetImageData()->GetScalarComponentAsDouble(
            (int)round(originIJK[0]+j),(int)round(originIJK[1]-i),(int)round(originIJK[2]),0));

        outMask->SetComponent(i*(size[0])+j,0,
          maskVolume->GetImageData()->GetScalarComponentAsDouble(
            (int)round(originIJK[0]+j),(int)round(originIJK[1]-i),(int)round(originIJK[2]),0));
        }
      }
    }

  if(evolution[0] == 1 && evolution[1] == 1)
    {
    for (int j=0;j<size[0];j++)
      {
      for (int i=0;i<size[1];i++)
        {
        outStorage->SetComponent(i*(size[0])+j,0,
          inVolume->GetImageData()->GetScalarComponentAsDouble(
            (int)round(originIJK[0]+j),(int)round(originIJK[1]+i),(int)round(originIJK[2]),0));

        outMask->SetComponent(i*(size[0])+j,0,
          maskVolume->GetImageData()->GetScalarComponentAsDouble(
                                                                 (int)round(originIJK[0]+j),(int)round(originIJK[1]+i),(int)round(originIJK[2]),0));
        }
      }
    }
  }

  if(pos[0]==1 && pos[1] == 2)
    {
    if(evolution[0] == 0 && evolution[1] == 0)
      {
      for (int j=0;j<size[0];j++)
        {
        for (int i=0;i<size[1];i++)
          {
          outStorage->SetComponent(i*(size[0])+j,0,inVolume->
              GetImageData()->GetScalarComponentAsDouble((int)round(originIJK[0]),
              (int)round(originIJK[1]-j),(int)round(originIJK[2]-i),0));

          outMask->SetComponent(i*(size[0])+j,0,maskVolume->
              GetImageData()->GetScalarComponentAsDouble((int)round(originIJK[0]),
              (int)round(originIJK[1]-j),(int)round(originIJK[2]-i),0));
          }
        }
      }

    if(evolution[0] == 0 && evolution[1] == 1)
      {
      for(int j=0;j<size[0];j++)
        {
        for(int i=0;i<size[1];i++)
          {
          outStorage->SetComponent(i*(size[0])+j,0,
            inVolume->GetImageData()->GetScalarComponentAsDouble(
              (int)round(originIJK[0]),(int)round(originIJK[1]-j),(int)round(originIJK[2]+i),0));

          outMask->SetComponent(i*(size[0])+j,0,
            maskVolume->GetImageData()->GetScalarComponentAsDouble(
              (int)round(originIJK[0]),(int)round(originIJK[1]-j),(int)round(originIJK[2]+i),0));
          }
        }
      }

    if(evolution[0] == 1 && evolution[1] == 0)
      {
      for (int j=0;j<size[0];j++)
        {
        for (int i=0;i<size[1];i++)
          {
          outStorage->SetComponent(i*(size[0])+j,0,
            inVolume->GetImageData()->GetScalarComponentAsDouble(
              (int)round(originIJK[0]),(int)round(originIJK[1]+j),(int)round(originIJK[2]-i),0));

          outMask->SetComponent(i*(size[0])+j,0,
            maskVolume->GetImageData()->GetScalarComponentAsDouble(
              (int)round(originIJK[0]),(int)round(originIJK[1]+j),(int)round(originIJK[2]-i),0));
          }
        }
      }

  if(evolution[0] == 1 && evolution[1] == 1)
    {
    for (int j=0;j<size[0];j++)
      {
      for(int i=0;i<size[1];i++)
        {
        outStorage->SetComponent(i*(size[0])+j,0,
          inVolume->GetImageData()->GetScalarComponentAsDouble(
            (int)round(originIJK[0]),(int)round(originIJK[1]+j),(int)round(originIJK[2]+i),0));

        outMask->SetComponent(i*(size[0])+j,0,
          maskVolume->GetImageData()->GetScalarComponentAsDouble(
            (int)round(originIJK[0]),(int)round(originIJK[1]+j),(int)round(originIJK[2]+i),0));
        }
      }
    }
    }

  if(pos[0]==0 && pos[1] == 2)
    {
    if(evolution[0] == 0 && evolution[1] == 0)
      {
      for (int j=0;j<size[0];j++)
        {
        for (int i=0;i<size[1];i++)
          {
          outStorage->SetComponent(i*(size[0])+j,0,
            inVolume->GetImageData()->GetScalarComponentAsDouble(
              (int)round(originIJK[0]-j),(int)round(originIJK[1]),(int)round(originIJK[2]-i),0));

          outMask->SetComponent(i*(size[0])+j,0,
            maskVolume->GetImageData()->GetScalarComponentAsDouble(
              (int)round(originIJK[0]-j),(int)round(originIJK[1]),(int)round(originIJK[2]-i),0));
          }
        }
      }


    if(evolution[0] == 0 && evolution[1] == 1)
      {
      for(int j=0;j<size[0];j++)
        {
        for(int i=0;i<size[1];i++)
          {
          outStorage->SetComponent(i*(size[0])+j,0,
            inVolume->GetImageData()->GetScalarComponentAsDouble(
              (int)round(originIJK[0]-j),(int)round(originIJK[1]),(int)round(originIJK[2]+i),0));

          outMask->SetComponent(i*(size[0])+j,0,
            maskVolume->GetImageData()->GetScalarComponentAsDouble(
              (int)round(originIJK[0]-j),(int)round(originIJK[1]),(int)round(originIJK[2]+i),0));
          }
        }
      }

    if(evolution[0] == 1 && evolution[1] == 0)
      {
      for (int j=0;j<size[0];j++)
        {
        for (int i=0;i<size[1];i++)
          {
          outStorage->SetComponent(i*(size[0])+j,0,
            inVolume->GetImageData()->GetScalarComponentAsDouble(
              (int)round(originIJK[0]+j),(int)round(originIJK[1]),(int)round(originIJK[2]-i),0));

          outMask->SetComponent(i*(size[0])+j,0,
            maskVolume->GetImageData()->GetScalarComponentAsDouble(
              (int)round(originIJK[0]+j),(int)round(originIJK[1]),(int)round(originIJK[2]-i),0));
          }
        }
      }

    if(evolution[0] == 1 && evolution[1] == 1)
      {
      for (int j=0;j<size[0];j++)
        {
        for (int i=0;i<size[1];i++)
          {
            outStorage->SetComponent(i*(size[0])+j,0,inVolume->GetImageData()->GetScalarComponentAsDouble((int)round(originIJK[0]+j),(int)round(originIJK[1]),(int)round(originIJK[2]+i),0));
            outMask->SetComponent(i*(size[0])+j,0,maskVolume->GetImageData()->GetScalarComponentAsDouble((int)round(originIJK[0]+j),(int)round(originIJK[1]),(int)round(originIJK[2]+i),0));
          }}}
    }

  outVolume->SetAndObserveImageData(this->MASK);
  outVolume->SetModifiedSinceRead(1);

  // PROCESS THE SLICE

  typedef itk::Image< float,  2 >   InputImageType;
  typedef itk::Image< unsigned char,   2 >   MaskImageType;
  typedef itk::Image< float,  2 >   OutputImageType;
  typedef itk::Image< float, 2 >  InternalImageType;

  typedef itk::ImageFileReader< InputImageType >  ReaderType;

  ReaderType::Pointer  reader = ReaderType::New();

  // CREATION OF THE ITK IMAGES

  // FOR THE SLICE TO BE PROCESSED
  vtkImageCast* imageCaster = vtkImageCast::New();
  imageCaster->SetInput(this->STORAGE);

  imageCaster->SetOutputScalarTypeToFloat(); 
  imageCaster->Modified();
  imageCaster->UpdateWholeExtent();

  typedef itk::VTKImageToImageFilter< InternalImageType >
    VTK2ITKConnectorFilterType;
  VTK2ITKConnectorFilterType::Pointer VTK2ITKconnector =
    VTK2ITKConnectorFilterType::New();
  VTK2ITKconnector->SetInput( imageCaster->GetOutput() );
  VTK2ITKconnector->Update();

  // FOR THE MASK TO BE USED

  vtkImageCast* imageCasterM = vtkImageCast::New();
  imageCasterM->SetInput(this->MASK);

  imageCasterM->SetOutputScalarTypeToFloat(); 
  imageCasterM->Modified();
  imageCasterM->UpdateWholeExtent();

  typedef itk::VTKImageToImageFilter< InternalImageType > VTK2ITKConnectorFilterTypeM;
  VTK2ITKConnectorFilterTypeM::Pointer VTK2ITKconnectorM = VTK2ITKConnectorFilterTypeM::New();
  VTK2ITKconnectorM->SetInput( imageCasterM->GetOutput() );
  VTK2ITKconnectorM->Update();


  //PROCESSING    

  typedef itk::ShrinkImageFilter<InputImageType, InputImageType> ShrinkerType;
  ShrinkerType::Pointer shrinker = ShrinkerType::New();
  shrinker->SetInput( VTK2ITKconnector->GetOutput() );
  shrinker->SetShrinkFactors( 1 );                                     


  typedef itk::BinaryThresholdImageFilter<
    InputImageType, MaskImageType>  mFilterType;

  mFilterType::Pointer mfilter = mFilterType::New();  

  mfilter->SetInput( VTK2ITKconnectorM->GetOutput() );

  mfilter->SetLowerThreshold(1);
  mfilter->SetOutsideValue(0);
  mfilter->SetInsideValue(1);  

  MaskImageType::Pointer maskImage = NULL;

  maskImage = mfilter->GetOutput();

  typedef itk::ShrinkImageFilter<MaskImageType, MaskImageType> MaskShrinkerType;
  MaskShrinkerType::Pointer maskshrinker = MaskShrinkerType::New();

  maskshrinker->SetInput( maskImage );
  maskshrinker->SetShrinkFactors( 1 );

  shrinker->SetShrinkFactors( (unsigned int)round(this->MRIBiasFieldCorrectionNode->GetShrink()));
  maskshrinker->SetShrinkFactors((unsigned int)round(this->MRIBiasFieldCorrectionNode->GetShrink()));


  shrinker->Update();

  maskshrinker->Update();


  typedef itk::N3MRIBiasFieldCorrectionImageFilter<InputImageType, MaskImageType,
          InputImageType> CorrecterType;
  CorrecterType::Pointer correcter = CorrecterType::New();
  correcter->SetInput( shrinker->GetOutput() );
  correcter->SetMaskImage( maskshrinker->GetOutput() );

  correcter->SetMaximumNumberOfIterations( (unsigned int)round(this->MRIBiasFieldCorrectionNode->GetMax()) );

  correcter->SetNumberOfFittingLevels((unsigned int)round(this->MRIBiasFieldCorrectionNode->GetNum()));

  correcter->SetWeinerFilterNoise(this->MRIBiasFieldCorrectionNode->GetWien());

  correcter->SetBiasFieldFullWidthAtHalfMaximum(this->MRIBiasFieldCorrectionNode->GetField());

  correcter->SetConvergenceThreshold(this->MRIBiasFieldCorrectionNode->GetCon());

  correcter->Update();

  correcter->SetConvergenceThreshold(this->MRIBiasFieldCorrectionNode->GetCon());

  typedef itk::BSplineControlPointImageFilter<
    CorrecterType::BiasFieldControlPointLatticeType, 
    CorrecterType::ScalarImageType> BSplinerType;
  BSplinerType::Pointer bspliner = BSplinerType::New();
  bspliner->SetInput( correcter->GetBiasFieldControlPointLattice() );
  bspliner->SetSplineOrder( correcter->GetSplineOrder() );
  bspliner->SetSize(
      VTK2ITKconnector->GetOutput()->GetLargestPossibleRegion().GetSize() );
  bspliner->SetOrigin( VTK2ITKconnector->GetOutput()->GetOrigin() );
  bspliner->SetDirection( VTK2ITKconnector->GetOutput()->GetDirection() );
  bspliner->SetSpacing( VTK2ITKconnector->GetOutput()->GetSpacing() );
  bspliner->Update();

  InputImageType::Pointer logField = InputImageType::New();
  logField->SetOrigin( bspliner->GetOutput()->GetOrigin() );
  logField->SetSpacing( bspliner->GetOutput()->GetSpacing() );
  logField->SetRegions(
      bspliner->GetOutput()->GetLargestPossibleRegion().GetSize() );
  logField->SetDirection( bspliner->GetOutput()->GetDirection() );
  logField->Allocate();

  itk::ImageRegionIterator<CorrecterType::ScalarImageType> ItB(
      bspliner->GetOutput(),
      bspliner->GetOutput()->GetLargestPossibleRegion() );
  itk::ImageRegionIterator<InputImageType> ItF( logField,
      logField->GetLargestPossibleRegion() );
  for( ItB.GoToBegin(), ItF.GoToBegin(); !ItB.IsAtEnd(); ++ItB, ++ItF )
  {
    ItF.Set( ItB.Get()[0] );
  }

  typedef itk::ExpImageFilter<InputImageType, InputImageType> ExpFilterType;
  ExpFilterType::Pointer expFilter = ExpFilterType::New();
  expFilter->SetInput( logField );
  expFilter->Update();

  typedef itk::DivideImageFilter<InputImageType, InputImageType, InputImageType> DividerType;
  DividerType::Pointer divider = DividerType::New();
  divider->SetInput1( VTK2ITKconnector->GetOutput() );
  divider->SetInput2( expFilter->GetOutput() );
  divider->Update();

  typedef itk::ImageToVTKImageFilter< InternalImageType >
    ITK2VTKConnectorFilterType;
  ITK2VTKConnectorFilterType::Pointer ITK2VTKconnector =
    ITK2VTKConnectorFilterType::New();
  ITK2VTKconnector->GetExporter()->SetInput(divider->GetOutput());
  ITK2VTKconnector->GetImporter()->Update();

  std::cout << __FILE__ << std::endl;
  std::cout << "line " << __LINE__ << ": CORRECTION DONE" <<std::endl;

  vtkImageData* image = vtkImageData::New();
  image->SetWholeExtent(wholeExtent);
  image->SetNumberOfScalarComponents(1);
  image->SetOrigin(0.0,0.0,0.0);
  image->SetDimensions(size[0],size[1],1);
  image->AllocateScalars();
  image->DeepCopy( ITK2VTKconnector->GetImporter()->GetOutput());

  vtkDataArray* outStorage2=image->GetPointData()->GetScalars();
  // PUT ARRAY BACK IN THE VOLUME

  if(pos[0]==0 && pos[1] == 1)
  {
    if(evolution[0] == 0 && evolution[1] == 0)
    {
      for (int j=0;j<size[0];j++)
      {
        for (int i=0;i<size[1];i++)
        {
          this->PREVIEW->SetScalarComponentFromDouble(
            (int)round(originIJK[0]-j-1), (int)round(originIJK[1]-i), (int)round(originIJK[2]), 0,
            outStorage2->GetComponent( i*(size[0])+j, 1 ) );
        }
      }
    }

    if(evolution[0] == 0 && evolution[1] == 1)
    {
      for (int j=0;j<size[0];j++)
      {
        for (int i=0;i<size[1];i++)
        {
          this->PREVIEW->SetScalarComponentFromDouble(
            (int)round(originIJK[0]-j-1), (int)round(originIJK[1]+i), (int)round(originIJK[2]), 0,
            outStorage2->GetComponent(i*(size[0])+j,1));
        }
      }
    }

    if(evolution[0] == 1 && evolution[1] == 0)
    {
      for (int j=0;j<size[0];j++)
      {
        for (int i=0;i<size[1];i++)
        {
          this->PREVIEW->SetScalarComponentFromDouble(
            (int)round(originIJK[0]+j+1), (int)round(originIJK[1]-i), (int)round(originIJK[2]), 0,
            outStorage2->GetComponent( i*(size[0])+j, 1 ) );
        }
      }
    }

    if(evolution[0] == 1 && evolution[1] == 1)
    {
      for (int j=0;j<size[0];j++)
      {
        for (int i=0;i<size[1];i++)
        {
          this->PREVIEW->SetScalarComponentFromDouble(
            (int)round(originIJK[0]+j+1), (int)round(originIJK[1]+i), (int)round(originIJK[2]), 0,
            outStorage2->GetComponent( i*(size[0])+j, 1 ) );
        }
      }
    }
  }

  if(pos[0]==1 && pos[1] == 2)
  {
    if(evolution[0] == 0 && evolution[1] == 0)
    {
      for (int j=0;j<size[0];j++)
      {
        for (int i=0;i<size[1];i++)
        {
          this->PREVIEW->SetScalarComponentFromDouble(
             (int)round(originIJK[0]),(int)round(originIJK[1]-j-1),(int)round(originIJK[2]-i),0,
             outStorage2->GetComponent(i*(size[0])+j,1));
        }
      }
    }

    if(evolution[0] == 0 && evolution[1] == 1)
    {
      for (int j=0;j<size[0];j++)
        for (int i=0;i<size[1];i++)
          this->PREVIEW->SetScalarComponentFromDouble(
              (int)round(originIJK[0]), (int)round(originIJK[1]-j-1), (int)round(originIJK[2]+i), 0,
              outStorage2->GetComponent(i*(size[0])+j,1));
    }

    if(evolution[0] == 1 && evolution[1] == 0)
    {
      for (int j=0;j<size[0];j++)
        for (int i=0;i<size[1];i++)
          this->PREVIEW->SetScalarComponentFromDouble(
              (int)round(originIJK[0]), (int)round(originIJK[1]+j), (int)round(originIJK[2]-i), 0,
              outStorage2->GetComponent(i*(size[0])+j,1));
    }

    if(evolution[0] == 1 && evolution[1] == 1)
    {
      for (int j=0;j<size[0];j++)
        for (int i=0;i<size[1];i++)
          this->PREVIEW->SetScalarComponentFromDouble(
            (int)round(originIJK[0]), (int)round(originIJK[1]+j+1), (int)round(originIJK[2]+i), 0,
            outStorage2->GetComponent(i*(size[0])+j,1));
    }
  }

  if(pos[0]==0 && pos[1] == 2)
  {
    if(evolution[0] == 0 && evolution[1] == 0)
    {
      for (int j=0;j<size[0];j++)
        for (int i=0;i<size[1];i++)
          this->PREVIEW->SetScalarComponentFromDouble(
              (int)round(originIJK[0]-j),(int)round(originIJK[1]),(int)round(originIJK[2]-i),0,
              outStorage2->GetComponent(i*(size[0])+j,1));
    }

    if(evolution[0] == 0 && evolution[1] == 1)
    {
      for (int j=0;j<size[0];j++)
        for (int i=0;i<size[1];i++)
          this->PREVIEW->SetScalarComponentFromDouble(
              (int)round(originIJK[0]-j),(int)round(originIJK[1]),(int)round(originIJK[2]+i),0,
              outStorage2->GetComponent(i*(size[0])+j,1));
    }

    if(evolution[0] == 1 && evolution[1] == 0){
      for (int j=0;j<size[0];j++)
        for (int i=0;i<size[1];i++)
          this->PREVIEW->SetScalarComponentFromDouble(
              (int)round(originIJK[0]+j),(int)round(originIJK[1]),(int)round(originIJK[2]-i),0,
              outStorage2->GetComponent(i*(size[0])+j,1));
    }

    if(evolution[0] == 1 && evolution[1] == 1)
    {
      for (int j=0;j<size[0];j++)
        for (int i=0;i<size[1];i++)
          this->PREVIEW->SetScalarComponentFromDouble(
             (int)round(originIJK[0]+j+1),(int)round(originIJK[1]),(int)round(originIJK[2]+i),0,
              outStorage2->GetComponent(i*(size[0])+j,1));
    }
  }

  stoVolume->SetAndObserveImageData(this->PREVIEW);
  stoVolume->SetModifiedSinceRead(1);
}

//-------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionLogic::Preview()
{
  // check if MRML node is present
  if (this->MRIBiasFieldCorrectionNode == NULL)
    {
    vtkErrorMacro("No input MRIBiasFieldCorrectionNode found");
    return;
    }

  // find input volume
  vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::
    SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->
      MRIBiasFieldCorrectionNode->GetInputVolumeRef()));

  if (inVolume == NULL)
    {
    vtkErrorMacro("No input volume found");
    return;
    }

  // find output volume
  vtkMRMLScalarVolumeNode *stoVolume = vtkMRMLScalarVolumeNode::
    SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->
      MRIBiasFieldCorrectionNode->GetStorageVolumeRef()));

  if(stoVolume == NULL)
    {
    vtkErrorMacro("No storage volume found with id= " << this->
      MRIBiasFieldCorrectionNode->GetStorageVolumeRef());
    return;
    }

  // copy RASToIJK matrix, and other attributes from input to output
  std::string name (stoVolume->GetName());
  std::string id (stoVolume->GetID());

  stoVolume->CopyOrientation(inVolume);
  stoVolume->SetAndObserveTransformNodeID(inVolume->GetTransformNodeID());

  stoVolume->SetName(name.c_str());

  // copy RASToIJK matrix, and other attributes from input to output
  stoVolume->SetAndObserveImageData(inVolume->GetImageData());

  // set ouput of the filter to VolumeNode's ImageData
  // TODO FIX the bug of the image is deallocated unless we do DeepCopy
  stoVolume->SetModifiedSinceRead(1);
}

//-------------------------------------------------------------------
int vtkMRIBiasFieldCorrectionLogic::InitMaxThreshold()
{
  // find input volume
  vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::
    SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->
      MRIBiasFieldCorrectionNode->GetInputVolumeRef()));

  double maxmin[2];
  inVolume->GetImageData()->GetScalarRange(maxmin);

  return (int)round(maxmin[1]);
}

//-------------------------------------------------------------------
int vtkMRIBiasFieldCorrectionLogic::InitMinThreshold()
{
  // find input volume
  vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::
    SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->
      MRIBiasFieldCorrectionNode->GetInputVolumeRef()));

  double maxmin[2];
  inVolume->GetImageData()->GetScalarRange(maxmin);

  return (int)round(maxmin[0]);
}

//-------------------------------------------------------------------
int vtkMRIBiasFieldCorrectionLogic::AxialMin()
{
  // find input volume
  vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::
    SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->
      MRIBiasFieldCorrectionNode->GetInputVolumeRef()));

  double bounds[6];
  inVolume->GetImageData()->GetBounds(bounds);

  return (int)round(bounds[0]);
}

//-------------------------------------------------------------------
int vtkMRIBiasFieldCorrectionLogic::AxialMax()
{
  // find input volume
  vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::
    SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->
      MRIBiasFieldCorrectionNode->GetInputVolumeRef()));

  double bounds[6];
  inVolume->GetImageData()->GetBounds(bounds);

  return (int)round(bounds[1]);
}

//-------------------------------------------------------------------
int vtkMRIBiasFieldCorrectionLogic::SagittalMax()
{
  // find input volume
  vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::
    SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->
      MRIBiasFieldCorrectionNode->GetInputVolumeRef()));

  double bounds[6];
  inVolume->GetImageData()->GetBounds(bounds);

  return (int)round(bounds[3]);
}

//-------------------------------------------------------------------
int vtkMRIBiasFieldCorrectionLogic::CoronalMax()
{
  // find input volume
  vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::
    SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->
      MRIBiasFieldCorrectionNode->GetInputVolumeRef()));

  double bounds[6];
  inVolume->GetImageData()->GetBounds(bounds);

  return (int)round(bounds[5]);
}

