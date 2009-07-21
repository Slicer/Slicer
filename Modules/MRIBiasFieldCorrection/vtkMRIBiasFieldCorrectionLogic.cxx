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
#include "vtkImageResample.h"
#include "vtkImageThreshold.h"
#include "vtkImageMathematics.h"
#include "vtkImageConstantPad.h"
#include "vtkImageEllipsoidSource.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLScalarVolumeNode.h"

//----------------------------------------------------------------------------
#include "vtkSlicerSliceControllerWidget.h"
#include "vtkSlicerSlicesControlGUI.h"

//----------------------------------------------------------------------------
#include "itkImage.h"
#include "itkImageRegionIterator.h"
#include "itkBSplineControlPointImageFilter.h"
#include "itkN3MRIBiasFieldCorrectionImageFilter.h"
#include "itkImageToVTKImageFilter.h"
#include "itkVTKImageToImageFilter.h"

//----------------------------------------------------------------------------
//#include "vtkMRIBiasFieldCorrectionGUI.h"

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
    vtkErrorMacro("No image data for the input volume");
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

  if (maskVolume->GetImageData() == NULL)
    {
    vtkErrorMacro("No image data for the mask volume");
    return;
    }

  vtkErrorMacro("inVolume");
  std::cout << *inVolume;

  vtkErrorMacro("maskVolume");
  std::cout << *maskVolume;

  vtkErrorMacro("outVolume");
  std::cout << *outVolume;

  return;

  // copy RASToIJK matrix, and other attributes from input to output
  outVolume->CopyOrientation(inVolume);
  outVolume->SetAndObserveTransformNodeID(inVolume->GetTransformNodeID());

  //-------------------------------------------------------------------------
  double maskThreshold = this->MRIBiasFieldCorrectionNode->GetMaskThreshold();

  double maskRange[2];
  maskVolume->GetImageData()->GetScalarRange(maskRange);

  std::cout << __LINE__ << " " << __FILE__ << std::endl;
  std::cout << " maskThreshold " << maskThreshold << std::endl;
  std::cout << " maskRange " << maskRange[0] << " " << maskRange[1] <<
    std::endl;

  maskThreshold = maskRange[0] + maskThreshold * (maskRange[1]-maskRange[0]);
  std::cout << " maskThreshold " << maskThreshold << std::endl;

  double outputSize = this->MRIBiasFieldCorrectionNode->GetOutputSize();

  vtkImageResample *imageResample = vtkImageResample::New();
  imageResample->SetInput(inVolume->GetImageData());
  imageResample->SetAxisMagnificationFactor(0, outputSize);
  imageResample->SetAxisMagnificationFactor(1, outputSize);
  imageResample->SetAxisMagnificationFactor(2, outputSize);
  imageResample->Update();

  vtkImageResample *maskResample = vtkImageResample::New();
  maskResample->SetInput(maskVolume->GetImageData());
  maskResample->SetAxisMagnificationFactor(0, outputSize);
  maskResample->SetAxisMagnificationFactor(1, outputSize);
  maskResample->SetAxisMagnificationFactor(2, outputSize);
  maskResample->Update();

  vtkImageCast *imageCast = vtkImageCast::New();
  imageCast->SetInput(imageResample->GetOutput());
  imageCast->SetOutputScalarTypeToFloat();
  imageCast->Update();

  vtkImageThreshold *imageThreshold = vtkImageThreshold::New();
  imageThreshold->SetInput(maskResample->GetOutput());
  imageThreshold->ThresholdByUpper(maskThreshold);
  imageThreshold->SetOutputScalarTypeToUnsignedChar();
  imageThreshold->SetInValue(255);
  imageThreshold->SetOutValue(0);
  imageThreshold->Update();

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
  itkMaskConnector->SetInput( imageThreshold->GetOutput() );
  imageThreshold->Delete();
  itkMaskConnector->Update();

  typedef itk::N3MRIBiasFieldCorrectionImageFilter< ImageType, MaskType,
          ImageType > CorrecterType;
  CorrecterType::Pointer correcter = CorrecterType::New();
  correcter->SetInput( itkImageConnector->GetOutput() );
  correcter->SetMaskImage( itkMaskConnector->GetOutput() );

  unsigned int numIterations    = this->MRIBiasFieldCorrectionNode->
    GetNumberOfIterations();
  unsigned int numFittingLevels = this->MRIBiasFieldCorrectionNode->
    GetNumberOfFittingLevels();

  double wienerFilterNoise      = this->MRIBiasFieldCorrectionNode->
    GetWienerFilterNoise();
  double widthAtHalfMaximum     = this->MRIBiasFieldCorrectionNode->
    GetBiasField();
  double convergenceThreshold   = this->MRIBiasFieldCorrectionNode->
    GetConvergenceThreshold();

  correcter->SetMaximumNumberOfIterations(numIterations);
  correcter->SetNumberOfFittingLevels(numFittingLevels);
  correcter->SetWienerFilterNoise(wienerFilterNoise);
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
  bSpliner->SetSize( itkImageConnector->GetOutput()->
      GetLargestPossibleRegion().GetSize() );
  bSpliner->SetOrigin( itkImageConnector->GetOutput()->GetOrigin() );
  bSpliner->SetSpacing( itkImageConnector->GetOutput()->GetSpacing() );
  bSpliner->SetDirection( itkImageConnector->GetOutput()->GetDirection() );
  bSpliner->Update();

  ImageType::Pointer logField = ImageType::New();
  logField->SetRegions( bSpliner->GetOutput()->GetLargestPossibleRegion().
      GetSize() );
  logField->SetOrigin( bSpliner->GetOutput()->GetOrigin() );
  logField->SetSpacing( bSpliner->GetOutput()->GetSpacing() );
  logField->SetDirection( bSpliner->GetOutput()->GetDirection() );
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

  vtkImageResample *maskUpsample = vtkImageResample::New();
  maskUpsample->SetInput(expFilter->GetOutput());
  expFilter->Delete();

  if (outputSize > 0.0)
    {
    maskUpsample->SetAxisMagnificationFactor(0, 1.0/outputSize);
    maskUpsample->SetAxisMagnificationFactor(1, 1.0/outputSize);
    maskUpsample->SetAxisMagnificationFactor(2, 1.0/outputSize);
    }

  maskUpsample->Update();

  vtkImageMathematics *imageMathematics = vtkImageMathematics::New();
  imageMathematics->SetInput1(inVolume->GetImageData());
  imageMathematics->SetInput2(maskUpsample->GetOutput());
  maskUpsample->Delete();
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
  // Check if MRML node is present
  if (this->MRIBiasFieldCorrectionNode == NULL)
    {
    vtkErrorMacro("No input MRIBiasFieldCorrectionNode found");
    return;
    }

  // Find input volume
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

  // create preview volume for preview
  vtkMRMLScalarVolumeNode *previewVolume = vtkMRMLScalarVolumeNode::
    SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->
      MRIBiasFieldCorrectionNode->GetPreviewVolumeRef()));

  if (previewVolume == NULL)
    {
    vtkErrorMacro("No preview volume with id= " << this->
      MRIBiasFieldCorrectionNode->GetPreviewVolumeRef());
    return;
    }

  // Create mask volume for processing
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

  vtkErrorMacro("inVolume");
  std::cout << *inVolume;

  vtkErrorMacro("maskVolume");
  std::cout << *maskVolume;

  vtkErrorMacro("previewVolume");
  std::cout << *previewVolume;

  vtkErrorMacro("outVolume");
  std::cout << *outVolume;

  // copy RASToIJK matrix, and other attributes from input to output

  std::string previewName(previewVolume->GetName());
  vtkErrorMacro("previewVolume name " << previewName);

  std::string outName(outVolume->GetName());
  vtkErrorMacro("outVolume name " << outName);

  previewVolume->CopyOrientation(inVolume);
  previewVolume->SetAndObserveTransformNodeID(inVolume->GetTransformNodeID());

  outVolume->CopyOrientation(inVolume);
  outVolume->SetAndObserveTransformNodeID(inVolume->GetTransformNodeID());
  outVolume->SetName(previewName.c_str());
  /*
  vtkImageEllipsoidSource *imageSphere = vtkImageEllipsoidSource::New();
  imageSphere->SetWholeExtent(0,63,0,63,0,63);
  imageSphere->SetCenter(32,32,32);
  imageSphere->SetRadius(20,20,20);
  imageSphere->SetInValue(255);
  imageSphere->SetOutValue(0);
  imageSphere->SetOutputScalarTypeToUnsignedChar();
  imageSphere->Update();

  previewVolume->SetAndObserveImageData(imageSphere->GetOutput());
  previewVolume->SetModifiedSinceRead(1);

  outVolume->SetAndObserveImageData(imageSphere->GetOutput());
  outVolume->SetModifiedSinceRead(1);

  return;
  */
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

  vtkErrorMacro("xyOrigin "
      << xyOrigin[0] << " " << xyOrigin[1] << " " << xyOrigin[2]);

  vtkErrorMacro("xyCornerX "
      << xyCornerX[0] << " " << xyCornerX[1] << " " << xyCornerX[2]);

  vtkErrorMacro("xyCornerY "
      << xyCornerY[0] << " " << xyCornerY[1] << " " << xyCornerY[2]);

  vtkErrorMacro("ijkOrigin "
      << ijkOrigin[0] << " " << ijkOrigin[1] << " " << ijkOrigin[2]);

  vtkErrorMacro("ijkCornerX "
      << ijkCornerX[0] << " " << ijkCornerX[1] << " " << ijkCornerX[2]);

  vtkErrorMacro("ijkCornerY "
      << ijkCornerY[0] << " " << ijkCornerY[1] << " " << ijkCornerY[2]);

  if (ijkOrigin[0] > ijkCornerX[0])
    {
    vtkErrorMacro("ijkOrigin[0] > ijkCornerX[0]");
    //return;
    }

  if (ijkOrigin[1] > ijkCornerX[1])
    {
    vtkErrorMacro("ijkOrigin[1] > ijkCornerX[1]");
    //return;
    }

  if (ijkOrigin[2] > ijkCornerX[2])
    {
    vtkErrorMacro("ijkOrigin[2] > ijkCornerX[2]");
    //return;
    }

  if (ijkOrigin[0] > ijkCornerY[0])
    {
    vtkErrorMacro("ijkOrigin[0] > ijkCornerY[0]");
    //return;
    }

  if (ijkOrigin[1] > ijkCornerY[1])
    {
    vtkErrorMacro("ijkOrigin[1] > ijkCornerY[1]");
    //return;
    }

  if (ijkOrigin[2] > ijkCornerY[2])
    {
    vtkErrorMacro("ijkOrigin[2] > ijkCornerY[2]");
    //return;
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

  if (clipExtent[0] > (int) ijkBounds[0]) { clipExtent[0]--; }
  if (clipExtent[1] < (int) ijkBounds[1]) { clipExtent[1]++; }

  if (clipExtent[2] > (int) ijkBounds[2]) { clipExtent[2]--; }
  if (clipExtent[3] < (int) ijkBounds[3]) { clipExtent[3]++; }

  if (clipExtent[4] > (int) ijkBounds[4]) { clipExtent[4]--; }
  if (clipExtent[5] < (int) ijkBounds[5]) { clipExtent[5]++; }

  double maskThreshold = this->MRIBiasFieldCorrectionNode->GetMaskThreshold();
  int inWholeExtent[6];
  inVolume->GetImageData()->GetWholeExtent(inWholeExtent);

  int wholeExt[6], ext[6], updateExt[6];

  inVolume->GetImageData()->GetWholeExtent(wholeExt);
  inVolume->GetImageData()->GetExtent(ext);
  inVolume->GetImageData()->GetUpdateExtent(updateExt);

  std::cout << __LINE__ << " inVolume wholeExtent"
    << " " << wholeExt[0] << " " << wholeExt[1]
    << " " << wholeExt[2] << " " << wholeExt[3]
    << " " << wholeExt[4] << " " << wholeExt[5] << std::endl;

  std::cout << __LINE__ << " inVolume extent"
    << " " << ext[0] << " " << ext[1]
    << " " << ext[2] << " " << ext[3]
    << " " << ext[4] << " " << ext[5] << std::endl;

  std::cout << __LINE__ << " inVolume updateExtent"
    << " " << updateExt[0] << " " << updateExt[1]
    << " " << updateExt[2] << " " << updateExt[3]
    << " " << updateExt[4] << " " << updateExt[5] << std::endl;

  vtkImageClip *imageClip = vtkImageClip::New();
  imageClip->SetInput(inVolume->GetImageData());
  imageClip->SetOutputWholeExtent(clipExtent);
  imageClip->ClipDataOn();
  imageClip->Update();

  double maskRange[2];
  imageClip->GetOutput()->GetScalarRange(maskRange);

  maskThreshold = maskRange[0] + maskThreshold * (maskRange[1]-maskRange[0]);

  imageClip->GetOutput()->GetWholeExtent(wholeExt);
  imageClip->GetOutput()->GetExtent(ext);

  std::cout << __LINE__ << " imageClip wholeExtent"
    << " " << wholeExt[0] << " " << wholeExt[1]
    << " " << wholeExt[2] << " " << wholeExt[3]
    << " " << wholeExt[4] << " " << wholeExt[5] << std::endl;

  std::cout << __LINE__ << " imageClip extent"
    << " " << ext[0] << " " << ext[1]
    << " " << ext[2] << " " << ext[3]
    << " " << ext[4] << " " << ext[5] << std::endl;

  vtkImageCast *imageCast = vtkImageCast::New();
  imageCast->SetInput(imageClip->GetOutput());
  imageClip->Delete();
  imageCast->SetOutputScalarTypeToFloat();
  imageCast->Update();

  imageCast->GetOutput()->GetWholeExtent(wholeExt);
  imageCast->GetOutput()->GetExtent(ext);

  std::cout << __LINE__ << " imageCast wholeExtent"
    << " " << wholeExt[0] << " " << wholeExt[1]
    << " " << wholeExt[2] << " " << wholeExt[3]
    << " " << wholeExt[4] << " " << wholeExt[5] << std::endl;

  std::cout << __LINE__ << " imageCast extent"
    << " " << ext[0] << " " << ext[1]
    << " " << ext[2] << " " << ext[3]
    << " " << ext[4] << " " << ext[5] << std::endl;

  vtkImageClip *maskClip = vtkImageClip::New();
  maskClip->SetInput(maskVolume->GetImageData());
  maskClip->SetOutputWholeExtent(clipExtent);
  maskClip->ClipDataOn();
  maskClip->Update();

  maskClip->GetOutput()->GetWholeExtent(wholeExt);
  maskClip->GetOutput()->GetExtent(ext);

  std::cout << __LINE__ << " maskClip wholeExtent"
    << " " << wholeExt[0] << " " << wholeExt[1]
    << " " << wholeExt[2] << " " << wholeExt[3]
    << " " << wholeExt[4] << " " << wholeExt[5] << std::endl;

  std::cout << __LINE__ << " maskClip extent"
    << " " << ext[0] << " " << ext[1]
    << " " << ext[2] << " " << ext[3]
    << " " << ext[4] << " " << ext[5] << std::endl;

  vtkImageThreshold *imageThreshold = vtkImageThreshold::New();
  imageThreshold->SetInput(maskClip->GetOutput());
  maskClip->Delete();
  imageThreshold->ThresholdByUpper(maskThreshold);
  imageThreshold->SetInValue(255);
  imageThreshold->SetOutValue(0);
  imageThreshold->SetOutputScalarTypeToUnsignedChar();
  imageThreshold->Update();

  imageThreshold->GetOutput()->GetWholeExtent(wholeExt);
  imageThreshold->GetOutput()->GetExtent(ext);

  std::cout << __LINE__ << " imageThreshold wholeExtent"
    << " " << wholeExt[0] << " " << wholeExt[1]
    << " " << wholeExt[2] << " " << wholeExt[3]
    << " " << wholeExt[4] << " " << wholeExt[5] << std::endl;

  std::cout << __LINE__ << " imageThreshold extent"
    << " " << ext[0] << " " << ext[1]
    << " " << ext[2] << " " << ext[3]
    << " " << ext[4] << " " << ext[5] << std::endl;

  double thresholdRange[2];
  imageThreshold->GetOutput()->GetScalarRange(thresholdRange);

  std::cout << __LINE__ << " thresholdRange "
    << thresholdRange[0] << " " << thresholdRange[1] << std::endl;
/*
  vtkImageConstantPad *imageConstantPad = vtkImageConstantPad::New();
  imageConstantPad->SetInput(imageThreshold->GetOutput());
  imageThreshold->Delete();
  imageConstantPad->SetOutputWholeExtent(inWholeExtent);
  imageConstantPad->SetConstant(0);
  imageConstantPad->Update();
*/
  //this->...Node->Get

  outVolume->SetAndObserveImageData(imageCast->GetOutput());
  outVolume->SetModifiedSinceRead(1);

  vtkImageData *previewImage = imageCast->GetOutput();
  previewImage->Register(NULL);
  imageCast->Delete();

  previewVolume->SetAndObserveImageData(previewImage);
  previewVolume->SetModifiedSinceRead(1);

  //vtkMRIBiasFieldCorrectionGUI::
/*
  typedef itk::Image<float,         2> ImageType;
  typedef itk::Image<unsigned char, 2> MaskType;

  typedef itk::VTKImageToImageFilter< ImageType > ImageConnectorType;
  typedef itk::VTKImageToImageFilter< MaskType  > MaskConnectorType;

  ImageConnectorType::Pointer itkPreviewConnector = ImageConnectorType::New();
  itkPreviewConnector->SetInput(imageCast->GetOutput());
  itkPreviewConnector->Update();

  MaskConnectorType::Pointer itkMaskConnector = MaskConnectorType::New();
  itkMaskConnector->SetInput(imageThreshold->GetOutput());
  imageThreshold->Delete();
  itkMaskConnector->Update();

  typedef itk::N3MRIBiasFieldCorrectionImageFilter<ImageType, MaskType,
          ImageType> BiasFieldType;
  BiasFieldType::Pointer biasField = BiasFieldType::New();
  biasField->SetInput( itkPreviewConnector->GetOutput() );
  biasField->SetMaskImage( itkMaskConnector->GetOutput() );

  double convergenceThreshold   = this->MRIBiasFieldCorrectionNode->
    GetConvergenceThreshold();
  double WienerFilterNoise      = this->MRIBiasFieldCorrectionNode->
    GetWienerFilterNoise();
  double widthAtHalfMaximum     = this->MRIBiasFieldCorrectionNode->
    GetBiasField();

  unsigned int numIterations    = this->MRIBiasFieldCorrectionNode->
    GetNumberOfIterations();
  unsigned int numFittingLevels = this->MRIBiasFieldCorrectionNode->
    GetNumberOfFittingLevels();

  std::cout << __LINE__ << " convergenceThreshold " << convergenceThreshold
    << std::endl;
  std::cout << __LINE__ << " WienerFilterNoise    " << WienerFilterNoise
    << std::endl;
  std::cout << __LINE__ << " widthAtHalfMaximum   " << widthAtHalfMaximum
    << std::endl;

  biasField->SetConvergenceThreshold(convergenceThreshold);
  biasField->SetMaximumNumberOfIterations(numIterations);
  biasField->SetNumberOfFittingLevels(numFittingLevels);
  biasField->SetWienerFilterNoise(WienerFilterNoise);
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
  bspliner->SetOrigin( itkPreviewConnector->GetOutput()->GetOrigin() );
  bspliner->SetSpacing( itkPreviewConnector->GetOutput()->GetSpacing() );
  bspliner->SetDirection( itkPreviewConnector->GetOutput()->GetDirection() );
  bspliner->Update();

  ImageType::Pointer logField = ImageType::New();
  logField->SetOrigin( bspliner->GetOutput()->GetOrigin() );
  logField->SetSpacing( bspliner->GetOutput()->GetSpacing() );
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

  expFilter->GetOutput()->GetWholeExtent(wholeExt);
  expFilter->GetOutput()->GetExtent(ext);
  expFilter->GetOutput()->GetUpdateExtent(updateExt);

  std::cout << __LINE__ << " expFilter wholeExtent"
    << " " << wholeExt[0] << " " << wholeExt[1]
    << " " << wholeExt[2] << " " << wholeExt[3]
    << " " << wholeExt[4] << " " << wholeExt[5] << std::endl;

  std::cout << __LINE__ << " expFilter extent"
    << " " << ext[0] << " " << ext[1]
    << " " << ext[2] << " " << ext[3]
    << " " << ext[4] << " " << ext[5] << std::endl;

  std::cout << __LINE__ << " expFilter updateExtent"
    << " " << updateExt[0] << " " << updateExt[1]
    << " " << updateExt[2] << " " << updateExt[3]
    << " " << updateExt[4] << " " << updateExt[5] << std::endl;

  vtkImageMathematics *imageMathematics = vtkImageMathematics::New();
  imageMathematics->SetInput1(imageCast->GetOutput());
  imageCast->Delete();
  imageMathematics->SetInput2(expFilter->GetOutput());
  expFilter->Delete();
  imageMathematics->SetOperationToDivide();
  imageMathematics->Update();

  std::cout << __FILE__ << std::endl;
  std::cout << __LINE__ << ": CORRECTION DONE" <<std::endl;

  vtkImageData *previewImage = imageMathematics->GetOutput();
  previewImage->Register(NULL);
  imageMathematics->Delete();
*/
  /*
  vtkImageData *previewImage = imageCast->GetOutput();
  previewImage->Register(NULL);
  imageCast->Delete();

  previewVolume->SetAndObserveImageData(previewImage);
  previewVolume->SetModifiedSinceRead(1);
  */
}

//-------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionLogic::Preview()
{
  // Check if MRML node is present
  if (this->MRIBiasFieldCorrectionNode == NULL)
    {
    vtkErrorMacro("No input MRIBiasFieldCorrectionNode found");
    return;
    }

  // Find input volume
  vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::
    SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->
      MRIBiasFieldCorrectionNode->GetInputVolumeRef()));

  if (inVolume == NULL)
    {
    vtkErrorMacro("No input volume");
    return;
    }

  // Find output volume
  vtkMRMLScalarVolumeNode *previewVolume = vtkMRMLScalarVolumeNode::
    SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->
      MRIBiasFieldCorrectionNode->GetPreviewVolumeRef()));

  if (previewVolume == NULL)
    {
    vtkErrorMacro("No preview volume with id " << this->
      MRIBiasFieldCorrectionNode->GetPreviewVolumeRef());
    return;
    }

  vtkErrorMacro("inVolume");
  std::cout << *inVolume;

  vtkErrorMacro("previewVolume");
  std::cout << *previewVolume;

  return;

  // copy RASToIJK matrix, and other attributes from input to output
  std::string name (previewVolume->GetName());

  previewVolume->CopyOrientation(inVolume);
  previewVolume->SetAndObserveTransformNodeID(inVolume->GetTransformNodeID());
  previewVolume->SetName(name.c_str());

  // copy RASToIJK matrix, and other attributes from input to output
  previewVolume->SetAndObserveImageData(inVolume->GetImageData());

  // set ouput of the filter to VolumeNode's ImageData
  // TODO FIX the bug of the image is deallocated unless we do DeepCopy
  previewVolume->SetModifiedSinceRead(1);
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

