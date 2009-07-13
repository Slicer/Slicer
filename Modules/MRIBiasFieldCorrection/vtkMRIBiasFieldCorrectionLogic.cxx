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

#include "vtkMRIBiasFieldCorrectionLogic.h"
#include "vtkMRIBiasFieldCorrection.h"

#include "vtkObjectFactory.h"
#include "vtkDataArray.h"
#include "vtkCellData.h"
#include "vtkGenericAttribute.h"

#include "vtkImageClip.h"
#include "vtkImageCast.h"
#include "vtkImageMathematics.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLScalarVolumeNode.h"

//----------------------------------------------------------------------------
#include "vtkSlicerSliceControllerWidget.h"
#include "vtkSlicerSlicesControlGUI.h"
#include "vtkKWScale.h"

//----------------------------------------------------------------------------
#include "itkImage.h"
#include "itkShrinkImageFilter.h"
#include "itkImageRegionIterator.h"
#include "itkBSplineControlPointImageFilter.h"
#include "itkN3MRIBiasFieldCorrectionImageFilter.h"
#include "itkImageToVTKImageFilter.h"
#include "itkVTKImageToImageFilter.h"

//----------------------------------------------------------------------------
#include "vtkMRIBiasFieldCorrectionGUI.h"

//----------------------------------------------------------------------------
vtkMRIBiasFieldCorrectionLogic* vtkMRIBiasFieldCorrectionLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance(
    "vtkMRIBiasFieldCorrectionLogic");

  if (ret)
    {
    return (vtkMRIBiasFieldCorrectionLogic*)ret;
    }

  // If the factory was unable to create the object, then create it here.
  return new vtkMRIBiasFieldCorrectionLogic;
}

//----------------------------------------------------------------------------
vtkMRIBiasFieldCorrectionLogic::vtkMRIBiasFieldCorrectionLogic()
{
  this->MRIBiasFieldCorrectionNode = NULL;
}

//----------------------------------------------------------------------------
vtkMRIBiasFieldCorrectionLogic::~vtkMRIBiasFieldCorrectionLogic()
{
  vtkSetMRMLNodeMacro(this->MRIBiasFieldCorrectionNode, NULL);
}

//----------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionLogic::PrintSelf(ostream& os, vtkIndent indent)
{
}

//----------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionLogic::Apply()
{
  // check if MRML node is present
  if (this->MRIBiasFieldCorrectionNode == NULL)
    {
    vtkErrorMacro("No input");
    return;
    }

  std::string volumeId = this->MRIBiasFieldCorrectionNode->
    GetOutputVolumeRef();
  std::string maskId   = this->MRIBiasFieldCorrectionNode->GetMaskVolumeRef();

  // find input volume
  vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::
    SafeDownCast( this->GetMRMLScene()->GetNodeByID(volumeId) );

  if (inVolume == NULL)
    {
    vtkErrorMacro("No input volume");
    return;
    }

  if (inVolume->GetImageData() == NULL)
    {
    vtkErrorMacro("No input image data");
    return;
    }

  int numComponents = inVolume->GetImageData()->GetNumberOfScalarComponents();

  if (numComponents != 1)
    {
    vtkErrorMacro("Input image voxels have " << numComponents << " components"
        ". The Bias Field Correction algorithm only applies to 1-component "
        << "images");
    return;
    }

  vtkMRMLScalarVolumeNode *outVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
      this->GetMRMLScene()->GetNodeByID(volumeId) );

  if (outVolume == NULL)
    {
    vtkErrorMacro("No output volume with id " << volumeId);
    return;
    }

  vtkMRMLScalarVolumeNode *maskVolume = vtkMRMLScalarVolumeNode::
    SafeDownCast( this->GetMRMLScene()->GetNodeByID(maskId) );

  if (maskVolume == NULL)
    {
    vtkErrorMacro("No mask volume with id " << maskId);
    return;
    }

  // copy RASToIJK matrix, and other attributes from input to output
  std::string name (outVolume->GetName());
  std::string id   (outVolume->GetID()  );

  outVolume->CopyOrientation(inVolume);
  outVolume->SetAndObserveTransformNodeID(inVolume->GetTransformNodeID());
  outVolume->SetName(name.c_str());

  vtkImageCast *imageCast = vtkImageCast::New();
  imageCast->SetInput(inVolume->GetImageData());
  imageCast->SetOutputScalarTypeToFloat();
  imageCast->Update();

  vtkImageCast* maskCast = vtkImageCast::New();
  maskCast->SetInput(maskVolume->GetImageData());
  maskCast->SetOutputScalarTypeToFloat();
  maskCast->Update();

  typedef itk::Image< float,         3 > ImageType;
  typedef itk::Image< unsigned char, 3 > MaskType;

  typedef itk::VTKImageToImageFilter< ImageType > ITKConnectorType;
  typedef itk::VTKImageToImageFilter< MaskType  > ITKMaskConnectorType;

  ITKConnectorType::Pointer itkImageConnector = ITKConnectorType::New();
  itkImageConnector->SetInput(imageCast->GetOutput());
  imageCast->Delete();
  itkImageConnector->Update();

  ITKMaskConnectorType::Pointer itkMaskConnector = ITKMaskConnectorType::
    New();
  itkMaskConnector->SetInput( maskCast->GetOutput() );
  maskCast->Delete();
  itkMaskConnector->Update();

  typedef itk::ShrinkImageFilter< ImageType, ImageType > ShrinkerType;
  typedef itk::ShrinkImageFilter< MaskType,  MaskType  > MaskShrinkType;

  ShrinkerType::Pointer shrinker = ShrinkerType::New();
  shrinker->SetInput( itkImageConnector->GetOutput() );

  typedef itk::BinaryThresholdImageFilter< MaskType, MaskType >
    binaryThresholdType;

  binaryThresholdType::Pointer binaryThresholdFilter =
    binaryThresholdType::New();
  binaryThresholdFilter->SetInput( itkMaskConnector->GetOutput() );
  binaryThresholdFilter->SetLowerThreshold(1);
  binaryThresholdFilter->SetOutsideValue(0);
  binaryThresholdFilter->SetInsideValue(1);
  binaryThresholdFilter->UpdateLargestPossibleRegion();

  MaskShrinkType::Pointer maskshrinker = MaskShrinkType::New();
  maskshrinker->SetInput(binaryThresholdFilter->GetOutput());

  unsigned int shrinkFactor = this->MRIBiasFieldCorrectionNode->
    GetShrinkFactor();

  shrinker->SetShrinkFactors(shrinkFactor);
  maskshrinker->SetShrinkFactors(shrinkFactor);

  shrinker->Update();
  shrinker->UpdateLargestPossibleRegion();

  maskshrinker->Update();
  maskshrinker->UpdateLargestPossibleRegion();

  typedef itk::N3MRIBiasFieldCorrectionImageFilter< ImageType, MaskType,
          ImageType > CorrecterType;
  CorrecterType::Pointer correcter = CorrecterType::New();
  correcter->SetInput( shrinker->GetOutput() );
  correcter->SetMaskImage( maskshrinker->GetOutput() );

  unsigned int numIterations = this->MRIBiasFieldCorrectionNode->
    GetNumberOfIterations();
  unsigned int numFittingLevels = this->MRIBiasFieldCorrectionNode->
    GetNumberOfFittingLevels();

  double wienerFilterNoise = this->MRIBiasFieldCorrectionNode->
    GetWienerFilterNoise();
  double widthAtHalfMaximum = this->MRIBiasFieldCorrectionNode->
    GetBiasField();
  double convergenceThreshold = this->MRIBiasFieldCorrectionNode->
    GetConvergenceThreshold();

  correcter->SetMaximumNumberOfIterations(numIterations);
  correcter->SetNumberOfFittingLevels(numFittingLevels);
  correcter->SetWeinerFilterNoise(wienerFilterNoise);
  correcter->SetBiasFieldFullWidthAtHalfMaximum(widthAtHalfMaximum);
  correcter->SetConvergenceThreshold(convergenceThreshold);
  correcter->Update();

  typedef CorrecterType::BiasFieldControlPointLatticeType LatticeType;
  typedef CorrecterType::ScalarImageType                  ScalarType;

  typedef itk::BSplineControlPointImageFilter< LatticeType, ScalarType >
    BSplinerType;

  BSplinerType::Pointer bSpliner = BSplinerType::New();
  bSpliner->SetInput( correcter->GetBiasFieldControlPointLattice() );
  bSpliner->SetSplineOrder( correcter->GetSplineOrder() );
  bSpliner->SetSize(
    itkImageConnector->GetOutput()->GetLargestPossibleRegion().GetSize() );
  //bspliner->SetOrigin(    itkImageConnector->GetOutput()->GetOrigin()    );
  //bspliner->SetDirection( itkImageConnector->GetOutput()->GetDirection() );
  //bspliner->SetSpacing(   itkImageConnector->GetOutput()->GetSpacing()   );
  bSpliner->Update();

  ImageType::Pointer logField = ImageType::New();
  logField->SetRegions(
    bSpliner->GetOutput()->GetLargestPossibleRegion().GetSize() );
  //logField->SetOrigin(  bspliner->GetOutput()->GetOrigin()  );
  //logField->SetSpacing( bspliner->GetOutput()->GetSpacing() );
  //logField->SetDirection( bspliner->GetOutput()->GetDirection() );
  logField->Allocate();

  itk::ImageRegionIterator< ScalarType > ItB( bSpliner->GetOutput(),
      bSpliner->GetOutput()->GetLargestPossibleRegion());
  itk::ImageRegionIterator< ImageType > ItF( logField,
      logField->GetLargestPossibleRegion() );

  for( ItB.GoToBegin(), ItF.GoToBegin(); !ItB.IsAtEnd(); ++ItB, ++ItF )
    {
    ItF.Set( ItB.Get()[0] );
    }

  typedef itk::ImageToVTKImageFilter< ImageType > VTKConnectorType;
  VTKConnectorType::Pointer logFieldConnector = VTKConnectorType::New();
  logFieldConnector->SetInput(logField);
  logFieldConnector->Update();

  vtkImageMathematics *expFilter = vtkImageMathematics::New();
  expFilter->SetInput(logFieldConnector->GetOutput());
  logFieldConnector->Delete();
  expFilter->SetOperationToExp();
  expFilter->Update();

  vtkImageMathematics *imageMathematics = vtkImageMathematics::New();
  imageMathematics->SetInput1(inVolume->GetImageData());
  imageMathematics->SetInput2(expFilter->GetOutput());
  expFilter->Delete();
  imageMathematics->SetOperationToDivide();
  imageMathematics->Update();

  outVolume->SetAndObserveImageData(imageMathematics->GetOutput());
  imageMathematics->Delete();
  outVolume->SetModifiedSinceRead(1);
}

//----------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionLogic::SliceProcess(vtkTransform* xyToijk,
  double dimX, double dimY)
{
  // check if MRML node is present
  if (this->MRIBiasFieldCorrectionNode == NULL)
    {
    vtkErrorMacro("No input MRIBiasFieldCorrectionNode found");
    return;
    }

  // find input volume
  vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
    this->GetMRMLScene()->GetNodeByID(this->MRIBiasFieldCorrectionNode->
      GetInputVolumeRef()));

  if (inVolume == NULL)
    {
    vtkErrorMacro("No input volume");
    return;
    }

  if (inVolume->GetImageData() == NULL)
    {
    vtkErrorMacro("Input volume has no image data");
    return;
    }

  if (inVolume->GetImageData()->GetNumberOfScalarComponents() != 1)
    {
    vtkErrorMacro("Input image must have 1 component");
    return;
    }

  // create storage volume for preview
  vtkMRMLScalarVolumeNode *storageVolume = vtkMRMLScalarVolumeNode::
    SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->
      MRIBiasFieldCorrectionNode->GetStorageVolumeRef()));

  if (storageVolume == NULL)
    {
    vtkErrorMacro("No storage volume with id= " << this->
      MRIBiasFieldCorrectionNode->GetStorageVolumeRef());
    return;
    }

  // create mask volume for processing
  vtkMRMLScalarVolumeNode *maskVolume = vtkMRMLScalarVolumeNode::
    SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->
      MRIBiasFieldCorrectionNode->GetMaskVolumeRef()));

  if (maskVolume == NULL)
    {
    vtkErrorMacro("No mask volume with id " <<
      this->MRIBiasFieldCorrectionNode->GetMaskVolumeRef());
    return;
    }

  // create output volume for preview
  vtkMRMLScalarVolumeNode *outVolume = vtkMRMLScalarVolumeNode::
    SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->
      MRIBiasFieldCorrectionNode->GetOutputVolumeRef()));

  if (outVolume == NULL)
    {
    vtkErrorMacro("No output volume with id " <<
      this->MRIBiasFieldCorrectionNode->GetOutputVolumeRef());
    return;
    }

  // copy RASToIJK matrix, and other attributes from input to output

  std::string name (storageVolume->GetName());
  std::string id (storageVolume->GetID());

  storageVolume->CopyOrientation(inVolume);
  storageVolume->SetAndObserveTransformNodeID(inVolume->GetTransformNodeID());
  storageVolume->SetName(name.c_str());

  outVolume->CopyOrientation(inVolume);
  outVolume->SetAndObserveTransformNodeID(inVolume->GetTransformNodeID());
  outVolume->SetName(name.c_str());

  // get bounds of the array in ijk

  double xyOrigin[4];
  double xyCornerX[4];
  double xyCornerY[4];

  double ijkOrigin[4];
  double ijkCornerX[4];
  double ijkCornerY[4];

  xyOrigin[0] = 0;
  xyOrigin[1] = 0;
  xyOrigin[2] = 0;
  xyOrigin[3] = 1;

  xyCornerX[0] = dimX;
  xyCornerX[1] = 0;
  xyCornerX[2] = 0;
  xyCornerX[3] = 1;

  xyCornerY[0] = 0;
  xyCornerY[1] = dimY;
  xyCornerY[2] = 0;
  xyCornerY[3] = 1;

  xyToijk->MultiplyPoint( xyOrigin,  ijkOrigin  );
  xyToijk->MultiplyPoint( xyCornerX, ijkCornerX );
  xyToijk->MultiplyPoint( xyCornerY, ijkCornerY );

  if( ijkOrigin[0] > ijkCornerX[0] )
  {
    std::cout << __LINE__ << " " << __FILE__ << " Error" << std::endl;
  }

  if( ijkOrigin[1] > ijkCornerX[1] )
  {
    std::cout << __LINE__ << " " << __FILE__ << " Error" << std::endl;
  }

  if( ijkOrigin[2] > ijkCornerX[2] )
  {
    std::cout << __LINE__ << " " << __FILE__ << " Error" << std::endl;
  }

  if( ijkOrigin[0] > ijkCornerY[0] )
  {
    std::cout << __LINE__ << " " << __FILE__ << " Error" << std::endl;
  }

  if( ijkOrigin[1] > ijkCornerY[1] )
  {
    std::cout << __LINE__ << " " << __FILE__ << " Error" << std::endl;
  }

  if( ijkOrigin[2] > ijkCornerY[2] )
  {
    std::cout << __LINE__ << " " << __FILE__ << " Error" << std::endl;
  }

  double ijkBounds[6];
  int clipExtent[6];

  ijkBounds[0] = ijkOrigin[0];
  ijkBounds[1] = ijkOrigin[0];

  ijkBounds[2] = ijkOrigin[1];
  ijkBounds[3] = ijkOrigin[1];

  ijkBounds[4] = ijkOrigin[2];
  ijkBounds[5] = ijkOrigin[2];

  if( ijkCornerX[0] < ijkBounds[0] ) { ijkBounds[0] = ijkCornerX[0]; }
  if( ijkCornerX[0] > ijkBounds[1] ) { ijkBounds[1] = ijkCornerX[0]; }

  if( ijkCornerX[1] < ijkBounds[2] ) { ijkBounds[2] = ijkCornerX[1]; }
  if( ijkCornerX[1] > ijkBounds[3] ) { ijkBounds[3] = ijkCornerX[1]; }

  if( ijkCornerX[2] < ijkBounds[4] ) { ijkBounds[4] = ijkCornerX[2]; }
  if( ijkCornerX[2] > ijkBounds[5] ) { ijkBounds[5] = ijkCornerX[2]; }

  if( ijkCornerY[0] < ijkBounds[0] ) { ijkBounds[0] = ijkCornerY[0]; }
  if( ijkCornerY[0] > ijkBounds[1] ) { ijkBounds[1] = ijkCornerY[0]; }

  if( ijkCornerY[1] < ijkBounds[2] ) { ijkBounds[2] = ijkCornerY[1]; }
  if( ijkCornerY[1] > ijkBounds[3] ) { ijkBounds[3] = ijkCornerY[1]; }

  if( ijkCornerY[2] < ijkBounds[4] ) { ijkBounds[4] = ijkCornerY[2]; }
  if( ijkCornerY[2] > ijkBounds[5] ) { ijkBounds[5] = ijkCornerY[2]; }

  clipExtent[0] = (int) ijkBounds[0];
  clipExtent[1] = (int) ijkBounds[1];

  clipExtent[2] = (int) ijkBounds[2];
  clipExtent[3] = (int) ijkBounds[3];

  clipExtent[4] = (int) ijkBounds[4];
  clipExtent[5] = (int) ijkBounds[5];

  if( clipExtent[0] > (int) ijkBounds[0] ) { clipExtent[0]--; }
  if( clipExtent[1] < (int) ijkBounds[1] ) { clipExtent[1]++; }

  if( clipExtent[2] > (int) ijkBounds[2] ) { clipExtent[2]--; }
  if( clipExtent[3] < (int) ijkBounds[3] ) { clipExtent[3]++; }

  if( clipExtent[4] > (int) ijkBounds[4] ) { clipExtent[4]--; }
  if( clipExtent[5] < (int) ijkBounds[5] ) { clipExtent[5]++; }

  vtkImageClip *imageClip = vtkImageClip::New();
  imageClip->SetInput(inVolume->GetImageData());
  imageClip->SetOutputWholeExtent(clipExtent);
  imageClip->Update();

  vtkImageCast *imageCast = vtkImageCast::New();
  imageCast->SetInput(imageClip->GetOutput());
  imageClip->Delete();
  imageCast->SetOutputScalarTypeToFloat();
  imageCast->Update();

  vtkImageClip *maskClip = vtkImageClip::New();
  maskClip->SetInput(maskVolume->GetImageData());
  maskClip->SetOutputWholeExtent(clipExtent);
  maskClip->Update();

  outVolume->SetAndObserveImageData(maskClip->GetOutput());
  outVolume->SetModifiedSinceRead(1);

  typedef itk::Image< float,         2 > ImageType;
  typedef itk::Image< unsigned char, 2 > MaskType;

  typedef itk::VTKImageToImageFilter< ImageType > ImageConnectorType;
  typedef itk::VTKImageToImageFilter< MaskType  > MaskConnectorType;

  ImageConnectorType::Pointer itkPreviewConnector = ImageConnectorType::New();
  itkPreviewConnector->SetInput(imageCast->GetOutput());
  itkPreviewConnector->Update();

  MaskConnectorType::Pointer itkMaskConnector = MaskConnectorType::New();
  itkMaskConnector->SetInput(maskClip->GetOutput());
  maskClip->Delete();
  itkMaskConnector->Update();

  typedef itk::N3MRIBiasFieldCorrectionImageFilter<ImageType, MaskType,
          ImageType> BiasFieldType;
  BiasFieldType::Pointer biasField = BiasFieldType::New();
  biasField->SetInput( itkPreviewConnector->GetOutput() );
  biasField->SetMaskImage( itkMaskConnector->GetOutput() );

  double convergenceThreshold = this->MRIBiasFieldCorrectionNode->
    GetConvergenceThreshold();
  double WeinerFilterNoise = this->MRIBiasFieldCorrectionNode->
    GetWienerFilterNoise();
  double widthAtHalfMaximum = this->MRIBiasFieldCorrectionNode->
    GetBiasField();

  unsigned int numIterations = this->MRIBiasFieldCorrectionNode->
    GetNumberOfIterations();
  unsigned int numFittingLevels = this->MRIBiasFieldCorrectionNode->
    GetNumberOfFittingLevels();

  biasField->SetConvergenceThreshold(convergenceThreshold);
  biasField->SetMaximumNumberOfIterations(numIterations);
  biasField->SetNumberOfFittingLevels(numFittingLevels);
  biasField->SetWeinerFilterNoise(WeinerFilterNoise);
  biasField->SetBiasFieldFullWidthAtHalfMaximum(widthAtHalfMaximum);
  biasField->SetConvergenceThreshold(convergenceThreshold);
  biasField->Update();

  typedef BiasFieldType::BiasFieldControlPointLatticeType LatticeType;
  typedef BiasFieldType::ScalarImageType                  ScalarType;

  typedef itk::BSplineControlPointImageFilter< LatticeType, ScalarType >
    BSplinerType;
  BSplinerType::Pointer bspliner = BSplinerType::New();
  bspliner->SetInput( biasField->GetBiasFieldControlPointLattice() );
  bspliner->SetSplineOrder( biasField->GetSplineOrder() );
  bspliner->SetSize(itkPreviewConnector->GetOutput()->
      GetLargestPossibleRegion().GetSize() );
  //bspliner->SetOrigin( itkPreviewConnector->GetOutput()->GetOrigin() );
  bspliner->SetDirection( itkPreviewConnector->GetOutput()->GetDirection() );
  //bspliner->SetSpacing( itkPreviewConnector->GetOutput()->GetSpacing() );
  bspliner->Update();

  ImageType::Pointer logField = ImageType::New();
  //logField->SetOrigin( bspliner->GetOutput()->GetOrigin() );
  //logField->SetSpacing( bspliner->GetOutput()->GetSpacing() );
  logField->SetRegions( bspliner->GetOutput()->GetLargestPossibleRegion().
      GetSize() );
  logField->SetDirection( bspliner->GetOutput()->GetDirection() );
  logField->Allocate();

  itk::ImageRegionIterator< ScalarType > ItB( bspliner->
      GetOutput(), bspliner->GetOutput()->GetLargestPossibleRegion() );

  itk::ImageRegionIterator< ImageType > ItF( logField,
      logField->GetLargestPossibleRegion() );

  for( ItB.GoToBegin(), ItF.GoToBegin(); !ItB.IsAtEnd(); ++ItB, ++ItF )
  {
    ItF.Set( ItB.Get()[0] );
  }

  typedef itk::ImageToVTKImageFilter< ImageType > VTKConnectorType;
  VTKConnectorType::Pointer logFieldConnector = VTKConnectorType::New();
  //logFieldConnector->GetExporter()->SetInput( logField );
  logFieldConnector->SetInput( logField );
  logFieldConnector->GetImporter()->Update();

  vtkImageMathematics *expFilter = vtkImageMathematics::New();
  expFilter->SetInput( logFieldConnector->GetOutput() );
  logFieldConnector->Delete();
  expFilter->SetOperationToExp();
  expFilter->Update();

  vtkImageMathematics *imageMathematics = vtkImageMathematics::New();
  imageMathematics->SetInput1(imageCast->GetOutput());
  imageCast->Delete();
  imageMathematics->SetInput2(expFilter->GetOutput());
  expFilter->Delete();
  imageMathematics->SetOperationToDivide();
  imageMathematics->Update();

  std::cout << __FILE__ << std::endl;
  std::cout << "line " << __LINE__ << ": CORRECTION DONE" <<std::endl;

  this->PreviewImage = imageMathematics->GetOutput();
  this->PreviewImage->Register(NULL);
  imageMathematics->Delete();

  // copy origin and spacing

  storageVolume->SetAndObserveImageData(this->PreviewImage);
  storageVolume->SetModifiedSinceRead(1);
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
    vtkErrorMacro("No input volume");
    return;
    }

  // find output volume
  vtkMRMLScalarVolumeNode *storageVolume = vtkMRMLScalarVolumeNode::
    SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->
      MRIBiasFieldCorrectionNode->GetStorageVolumeRef()));

  if (storageVolume == NULL)
    {
    vtkErrorMacro("No storage volume with id " << this->
      MRIBiasFieldCorrectionNode->GetStorageVolumeRef());
    return;
    }

  // copy RASToIJK matrix, and other attributes from input to output
  std::string name (storageVolume->GetName());
  std::string id (storageVolume->GetID());

  storageVolume->CopyOrientation(inVolume);
  storageVolume->SetAndObserveTransformNodeID(inVolume->GetTransformNodeID());
  storageVolume->SetName(name.c_str());

  // copy RASToIJK matrix, and other attributes from input to output
  storageVolume->SetAndObserveImageData(inVolume->GetImageData());

  // set ouput of the filter to VolumeNode's ImageData
  // TODO FIX the bug of the image is deallocated unless we do DeepCopy
  storageVolume->SetModifiedSinceRead(1);
}

//-------------------------------------------------------------------
double vtkMRIBiasFieldCorrectionLogic::InitMaxThreshold()
{
  // find input volume
  vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::
    SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->
      MRIBiasFieldCorrectionNode->GetInputVolumeRef()));

  double maxmin[2];
  inVolume->GetImageData()->GetScalarRange(maxmin);

  return maxmin[1];
}

//-------------------------------------------------------------------
double vtkMRIBiasFieldCorrectionLogic::InitMinThreshold()
{
  // find input volume
  vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::
    SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->
      MRIBiasFieldCorrectionNode->GetInputVolumeRef()));

  double maxmin[2];
  inVolume->GetImageData()->GetScalarRange(maxmin);

  return maxmin[0];
}

//-------------------------------------------------------------------
double vtkMRIBiasFieldCorrectionLogic::AxialMin()
{
  // find input volume
  vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::
    SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->
      MRIBiasFieldCorrectionNode->GetInputVolumeRef()));

  double bounds[6];
  inVolume->GetImageData()->GetBounds(bounds);

  return bounds[0];
}

//-------------------------------------------------------------------
double vtkMRIBiasFieldCorrectionLogic::AxialMax()
{
  // find input volume
  vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::
    SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->
      MRIBiasFieldCorrectionNode->GetInputVolumeRef()));

  double bounds[6];
  inVolume->GetImageData()->GetBounds(bounds);

  return bounds[1];
}

//-------------------------------------------------------------------
double vtkMRIBiasFieldCorrectionLogic::SagittalMax()
{
  // find input volume
  vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::
    SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->
      MRIBiasFieldCorrectionNode->GetInputVolumeRef()));

  double bounds[6];
  inVolume->GetImageData()->GetBounds(bounds);

  return bounds[3];
}

//-------------------------------------------------------------------
double vtkMRIBiasFieldCorrectionLogic::CoronalMax()
{
  // find input volume
  vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::
    SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->
      MRIBiasFieldCorrectionNode->GetInputVolumeRef()));

  double bounds[6];
  inVolume->GetImageData()->GetBounds(bounds);

  return bounds[5];
}

