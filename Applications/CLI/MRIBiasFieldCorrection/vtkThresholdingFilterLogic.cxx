/*=auto====================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkThresholdingFilterLogic.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

====================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkThresholdingFilterLogic.h"
#include "vtkThresholdingFilter.h"

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
#include "vtkThresholdingFilterGUI.h"

///////////////////////////////////////////////////////

vtkThresholdingFilterLogic* vtkThresholdingFilterLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance(
      "vtkThresholdingFilterLogic");
  if(ret)
    {
      return (vtkThresholdingFilterLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkThresholdingFilterLogic;
}

//-------------------------------------------------------------------------
vtkThresholdingFilterLogic::vtkThresholdingFilterLogic()
{
  this->ThresholdingFilterNode = NULL;
}

//-------------------------------------------------------------------------
vtkThresholdingFilterLogic::~vtkThresholdingFilterLogic()
{
  vtkSetMRMLNodeMacro(this->ThresholdingFilterNode, NULL);
}

//-------------------------------------------------------------------------
void vtkThresholdingFilterLogic::PrintSelf(ostream& os, vtkIndent indent)
{
}

void vtkThresholdingFilterLogic::Apply()
{
  // check if MRML node is present
  if (this->ThresholdingFilterNode == NULL)
    {
    vtkErrorMacro("No input GradientAnisotropicDiffusionFilterNode found");
    return;
    }

  // find input volume
    vtkMRMLScalarVolumeNode *inVolume =
      vtkMRMLScalarVolumeNode::SafeDownCast(
          this->GetMRMLScene()->GetNodeByID(
            this->ThresholdingFilterNode->GetInputVolumeRef()));

  if (inVolume == NULL)
    {
    vtkErrorMacro("No input volume found");
    return;
    }

  // find output volume
  vtkMRMLScalarVolumeNode *outVolume =
    vtkMRMLScalarVolumeNode::SafeDownCast(
        this->GetMRMLScene()->GetNodeByID(
          this->ThresholdingFilterNode->GetOutputVolumeRef()));

  if (outVolume == NULL)
    {
    vtkErrorMacro("No output volume found with id= "
        << this->ThresholdingFilterNode->GetOutputVolumeRef());
    return;
    }

  // copy RASToIJK matrix, and other attributes from input to output
  std::string name (outVolume->GetName());
  std::string id (outVolume->GetID());

  outVolume->CopyOrientation(inVolume);
  outVolume->SetAndObserveTransformNodeID(inVolume->GetTransformNodeID());

  outVolume->SetName(name.c_str());

  typedef itk::Image< float,  3 >   InputImageType;
  typedef itk::Image< unsigned char,   3>   MaskImageType;
  typedef itk::Image< float,  3 >   OutputImageType;
  typedef itk::Image< float, 3 >  InternalImageType;

  std::cout<<"3d"<< std::endl;

  typedef itk::ImageFileReader< InputImageType >  ReaderType;

  ReaderType::Pointer  reader = ReaderType::New();

  vtkImageCast* VtkCaster = vtkImageCast::New();
  VtkCaster->SetInput(inVolume->GetImageData());
  VtkCaster->SetOutputScalarTypeToFloat();
  VtkCaster->Modified();
  VtkCaster->Update();

  typedef itk::VTKImageToImageFilter< InternalImageType >
    VTK2ITKConnectorFilterType;
  VTK2ITKConnectorFilterType::Pointer VTK2ITKconnector =
    VTK2ITKConnectorFilterType::New();
  VTK2ITKconnector->SetInput( VtkCaster->GetOutput() );
  VTK2ITKconnector->GetImporter()->Update();
  VTK2ITKconnector->Update();

  typedef itk::ShrinkImageFilter<InputImageType, InputImageType> ShrinkerType;
  ShrinkerType::Pointer shrinker = ShrinkerType::New();
  shrinker->SetInput( VTK2ITKconnector->GetOutput() );
  shrinker->SetShrinkFactors( 1 );

  typedef itk::BinaryThresholdImageFilter<InputImageType, MaskImageType>
    mFilterType;

  mFilterType::Pointer mfilter = mFilterType::New();
  mfilter->SetInput( VTK2ITKconnector->GetOutput() );

  mfilter->SetLowerThreshold(
      this->ThresholdingFilterNode->GetConductance());
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

  shrinker->SetShrinkFactors( this->ThresholdingFilterNode->GetShrink());
  maskshrinker->SetShrinkFactors(
      this->ThresholdingFilterNode->GetShrink());

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
      this->ThresholdingFilterNode->GetMax() );

  correcter->SetNumberOfFittingLevels(
      this->ThresholdingFilterNode->GetNum());

  correcter->SetWeinerFilterNoise(this->ThresholdingFilterNode->GetWien());

  correcter->SetBiasFieldFullWidthAtHalfMaximum(
      this->ThresholdingFilterNode->GetField());

  correcter->SetConvergenceThreshold(this->ThresholdingFilterNode->GetCon());
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

  correcter->SetConvergenceThreshold(
      this->ThresholdingFilterNode->GetCon());

  InputImageType::Pointer logField = InputImageType::New();
  logField->SetOrigin( bspliner->GetOutput()->GetOrigin() );
  logField->SetSpacing( bspliner->GetOutput()->GetSpacing() );
  logField->SetRegions(
    bspliner->GetOutput()->GetLargestPossibleRegion().GetSize() );
  logField->SetDirection( bspliner->GetOutput()->GetDirection() );
  logField->Allocate();

  correcter->SetConvergenceThreshold(
      this->ThresholdingFilterNode->GetCon());

  itk::ImageRegionIterator<CorrecterType::ScalarImageType> ItB(
    bspliner->GetOutput(),
    bspliner->GetOutput()->GetLargestPossibleRegion() );
  itk::ImageRegionIterator<InputImageType> ItF( logField,
    logField->GetLargestPossibleRegion() );
  for( ItB.GoToBegin(), ItF.GoToBegin(); !ItB.IsAtEnd(); ++ItB, ++ItF )
    {
    ItF.Set( ItB.Get()[0] );
    }

  correcter->SetConvergenceThreshold(
      this->ThresholdingFilterNode->GetCon());

  typedef itk::ExpImageFilter<InputImageType, InputImageType>
    ExpFilterType;
  ExpFilterType::Pointer expFilter = ExpFilterType::New();
  expFilter->SetInput( logField );
  expFilter->Update();

  correcter->SetConvergenceThreshold(
      this->ThresholdingFilterNode->GetCon());

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

  vtkImageData *image = vtkImageData::New();
  image->DeepCopy( ITK2VTKconnector->GetImporter()->GetOutput());
  outVolume->SetAndObserveImageData(image);
  image->Delete();
  outVolume->SetModifiedSinceRead(1);
}

void vtkThresholdingFilterLogic::ApplyPreview()
{
  // check if MRML node is present
  if (this->ThresholdingFilterNode == NULL)
    {
    vtkErrorMacro("No input GradientAnisotropicDiffusionFilterNode found");
    return;
    }

  // find input volume
  vtkMRMLScalarVolumeNode *inVolume =
    vtkMRMLScalarVolumeNode::SafeDownCast(
        this->GetMRMLScene()->GetNodeByID(
          this->ThresholdingFilterNode->GetInputVolumeRef()));

  if (inVolume == NULL)
    {
    vtkErrorMacro("No input volume found");
    return;
    }

  // find output volume
  vtkMRMLScalarVolumeNode *outVolume =
    vtkMRMLScalarVolumeNode::SafeDownCast(
        this->GetMRMLScene()->GetNodeByID(
          this->ThresholdingFilterNode->GetOutputVolumeRef()));

  if (outVolume == NULL)
    {
    vtkErrorMacro("No output volume found with id= "
        << this->ThresholdingFilterNode->GetOutputVolumeRef());
    return;
    }

  // copy RASToIJK matrix, and other attributes from input to output
  std::string name (outVolume->GetName());
  std::string id (outVolume->GetID());

  // outVolume->CopyOrientation(inVolume);
  outVolume->SetAndObserveTransformNodeID(inVolume->GetTransformNodeID());

  outVolume->SetName(name.c_str());

  typedef itk::Image< float,         2 >  InputImageType;
  typedef itk::Image< unsigned char, 2 >  MaskImageType;
  typedef itk::Image< float,         2 >  OutputImageType;
  typedef itk::Image< float,         2 >  InternalImageType;

  typedef itk::ImageFileReader< InputImageType >  ReaderType;

  ReaderType::Pointer  reader = ReaderType::New();

  vtkImageCast* VtkCaster = vtkImageCast::New();
  VtkCaster->SetInput(this->CurrentSlide);

  VtkCaster->SetOutputScalarTypeToFloat();
  VtkCaster->Modified();
  VtkCaster->UpdateWholeExtent();

  typedef itk::VTKImageToImageFilter< InternalImageType >
    VTK2ITKConnectorFilterType;
  VTK2ITKConnectorFilterType::Pointer VTK2ITKconnector =
    VTK2ITKConnectorFilterType::New();
  VTK2ITKconnector->SetInput( VtkCaster->GetOutput() );
  VTK2ITKconnector->Update();

  typedef itk::ShrinkImageFilter<InputImageType, InputImageType> ShrinkerType;
  ShrinkerType::Pointer shrinker = ShrinkerType::New();
  shrinker->SetInput( VTK2ITKconnector->GetOutput() );
  shrinker->SetShrinkFactors( 1 );

  typedef itk::BinaryThresholdImageFilter<InputImageType, MaskImageType>
    mFilterType;
  mFilterType::Pointer mfilter = mFilterType::New();

  mfilter->SetInput( VTK2ITKconnector->GetOutput() );

  mfilter->SetLowerThreshold(this->ThresholdingFilterNode->GetConductance());
  mfilter->SetOutsideValue(0);
  mfilter->SetInsideValue(1);

  MaskImageType::Pointer maskImage = NULL;

  maskImage = mfilter->GetOutput();

  typedef itk::ShrinkImageFilter<MaskImageType, MaskImageType>
    MaskShrinkerType;
  MaskShrinkerType::Pointer maskshrinker = MaskShrinkerType::New();

  maskshrinker->SetInput( maskImage );
  maskshrinker->SetShrinkFactors( 1 );

  shrinker->SetShrinkFactors( this->ThresholdingFilterNode->GetShrink());
  maskshrinker->SetShrinkFactors(this->ThresholdingFilterNode->GetShrink());

  shrinker->Update();

  maskshrinker->Update();

  typedef itk::N3MRIBiasFieldCorrectionImageFilter<InputImageType,
    MaskImageType, InputImageType> CorrecterType;
  CorrecterType::Pointer correcter = CorrecterType::New();
  correcter->SetInput( shrinker->GetOutput() );
  correcter->SetMaskImage( maskshrinker->GetOutput() );

  correcter->SetMaximumNumberOfIterations(
      this->ThresholdingFilterNode->GetMax() );

  correcter->SetNumberOfFittingLevels(this->ThresholdingFilterNode->GetNum());

  correcter->SetWeinerFilterNoise(this->ThresholdingFilterNode->GetWien());

  correcter->SetBiasFieldFullWidthAtHalfMaximum(
      this->ThresholdingFilterNode->GetField());

  correcter->SetConvergenceThreshold(this->ThresholdingFilterNode->GetCon());

  correcter->Update();

  correcter->SetConvergenceThreshold(this->ThresholdingFilterNode->GetCon());

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
  image->UpdateInformation();
  outVolume->SetAndObserveImageData(image);
  outVolume->SetOrigin(-originOutvolume[0],-originOutvolume[1],0);

  outVolume->SetModifiedSinceRead(1);
}

void vtkThresholdingFilterLogic::Preview()
{
  // check if MRML node is present
  if (this->ThresholdingFilterNode == NULL)
    {
    vtkErrorMacro("No input ThresholdingFilterNode found");
    return;
    }

  // find input volume
  vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
      this->GetMRMLScene()->GetNodeByID(
        this->ThresholdingFilterNode->GetInputVolumeRef()));

  if (inVolume == NULL)
    {
    vtkErrorMacro("No input volume found");
    return;
    }

  // find output volume
  vtkMRMLScalarVolumeNode *outVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
      this->GetMRMLScene()->GetNodeByID(
        this->ThresholdingFilterNode->GetOutputVolumeRef()));

  if (outVolume == NULL)
    {
    vtkErrorMacro("No output volume found with id= " <<
        this->ThresholdingFilterNode->GetOutputVolumeRef());
    return;
    }

  // copy RASToIJK matrix, and other attributes from input to output
  std::string name (outVolume->GetName());
  std::string id (outVolume->GetID());

  //outVolume->CopyOrientation(inVolume);

  outVolume->SetAndObserveTransformNodeID(inVolume->GetTransformNodeID());

  outVolume->SetName(name.c_str());

  double maxmin[2];
  inVolume->GetImageData()->GetScalarRange(maxmin);

  this->ImageThreshold=vtkImageThreshold::New();
  this->ImageThreshold->SetInput(this->CurrentSlide);
  this->ImageThreshold->ThresholdByUpper(
      this->ThresholdingFilterNode->GetConductance());
  this->ImageThreshold->SetInValue(maxmin[1]);
  this->ImageThreshold->SetOutValue(maxmin[0]);
  this->ImageThreshold->Update();

  // set ouput of the filter to VolumeNode's ImageData
  // TODO FIX the bug of the image is deallocated unless we do DeepCopy
  vtkImageData* image = vtkImageData::New();
  image->DeepCopy( this->ImageThreshold->GetOutput());
  image->UpdateInformation();

  outVolume->SetAndObserveImageData(image);
  outVolume->SetOrigin(-originOutvolume[0],-originOutvolume[1],0);

  outVolume->SetModifiedSinceRead(1);
}

void vtkThresholdingFilterLogic::Cut()
{
  // check if MRML node is present
  if (this->ThresholdingFilterNode == NULL)
    {
    vtkErrorMacro("No input GradientAnisotropicDiffusionFilterNode found");
    return;
    }

  // find input volume
  vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
      this->GetMRMLScene()->GetNodeByID(
        this->ThresholdingFilterNode->GetInputVolumeRef()));

  if (inVolume == NULL)
    {
    vtkErrorMacro("No input volume found");
    return;
    }

  double bounds[6];
  inVolume->GetImageData()->GetBounds(bounds);

  // Calculate the center of the volume
  int extent[6];
  double spacing[3];
  double origin[3];
  inVolume->GetImageData()->GetWholeExtent(extent);
  inVolume->GetImageData()->GetSpacing(spacing);
  inVolume->GetImageData()->GetOrigin(origin);

  double center[4];
  center[0] = origin[0] + spacing[0] * 0.5 * (extent[0] + extent[1]);
  center[1] = origin[1] + spacing[1] * 0.5 * (extent[2] + extent[3]);
  center[2] = origin[2] + spacing[2] * 0.5 * (extent[4] + extent[5]);
  center[4] = 1;

  // Matrices for axial, coronal, sagittal, oblique view orientations
  static double axialElements[9] = {
           1, 0, 0,
           0, 1, 0,
           0, 0, 1};

  static double coronalElements[9] = {
           1, 0, 0,
           0, 0, 1,
           0,-1, 0};

  static double sagittalElements[9] = {
           0, 0,-1,
           1, 0, 0,
           0,-1, 0};

  // Extract a slice in the desired orientation
  int wholeExtent[6],dims[3];
  inVolume->GetImageData()->GetDimensions(dims);

  wholeExtent[0]=0;
  wholeExtent[2]=0;
  wholeExtent[4]=0;
  wholeExtent[5]=0;

  wholeExtent[1] = dims[0]-1;
  wholeExtent[3] = dims[1]-1;

  this->CurrentSlide = vtkImageData::New();
  this->CurrentSlide->SetWholeExtent( wholeExtent );
  this->CurrentSlide->SetScalarType(
      inVolume->GetImageData()->GetScalarType());
  this->CurrentSlide->SetNumberOfScalarComponents(1);

  int dim[3];
  inVolume->GetImageData()->GetDimensions(dim);

  double ori[3];
  inVolume->GetImageData()->GetOrigin(ori);

  double sp[3];
  inVolume->GetImageData()->GetSpacing(sp);

  vtkDataArray* inp=inVolume->GetImageData()->GetPointData()->GetScalars();

  int dty=inp->GetDataType();

  this->CurrentSlide->SetSpacing(sp);
  this->CurrentSlide->SetScalarType(dty);
  this->CurrentSlide->SetNumberOfScalarComponents(1);
  this->CurrentSlide->SetOrigin(0.0,0.0,0.0);

  this->CurrentSlide->SetDimensions(dim[0],dim[1],1);
  this->CurrentSlide->SetWholeExtent(0,dim[0]-1,0,dim[1]-1,0,0);
  this->CurrentSlide->AllocateScalars();

  vtkDataArray* out=this->CurrentSlide->GetPointData()->GetScalars();

  originOutvolume[0] = dim[0]/2;
  originOutvolume[1] = dim[1]/2;
  originOutvolume[2] = dim[2]/2;

  //image 1

  int offset=dim[0]*dim[1]*1;//this->SliceNo;

  for (int j=0;j<dim[1];j++)
    for (int i=0;i<dim[0];i++)
      out->SetComponent(i+((dim[1]-1)*dim[0])-j*dim[0],0,
          inp->GetComponent(i+dim[0]*j+offset,1));

  this->Image1 = vtkImageData::New();
  this->Image1->DeepCopy( this->CurrentSlide);

  //image 2

  int offset2=dim[0]*dim[1]*round(dim[2]/4);//this->SliceNo

  for (int j=0;j<dim[1];j++)
    for (int i=0;i<dim[0];i++)
      out->SetComponent(i+((dim[1]-1)*dim[0])-j*dim[0],0,
          inp->GetComponent(i+dim[0]*j+offset2,1));

  this->Image2 = vtkImageData::New();
  this->Image2->DeepCopy( this->CurrentSlide);

  //image 3

  int offset3=dim[0]*dim[1]*round(dim[2]/2);//this->SliceNo

  for (int j=0;j<dim[1];j++)
    for (int i=0;i<dim[0];i++)
      out->SetComponent(i+((dim[1]-1)*dim[0])-j*dim[0],0,
          inp->GetComponent(i+dim[0]*j+offset3,1));

  this->Image3 = vtkImageData::New();
  this->Image3->DeepCopy( this->CurrentSlide);

  //image 4

  int offset4=dim[0]*dim[1]*round(dim[2]*3/4);//this->SliceNo

  for (int j=0;j<dim[1];j++)
    for (int i=0;i<dim[0];i++)
      out->SetComponent(i+((dim[1]-1)*dim[0])-j*dim[0],0,
          inp->GetComponent(i+dim[0]*j+offset4,1));

  this->Image4 = vtkImageData::New();
  this->Image4->DeepCopy( this->CurrentSlide);

  //image 5

  int offset5=dim[0]*dim[1]*(dim[2]-1);//this->SliceNo

  for (int j=0;j<dim[1];j++)
    for (int i=0;i<dim[0];i++)
      out->SetComponent(i+((dim[1]-1)*dim[0])-j*dim[0],0,
          inp->GetComponent(i+dim[0]*j+offset5,1));

  this->Image5 = vtkImageData::New();
  this->Image5->DeepCopy( this->CurrentSlide);
}

void vtkThresholdingFilterLogic::ChangeSlide()
{
  if(this->ThresholdingFilterNode->GetSagittal1() == 1)
    {
    this->CurrentSlide->DeepCopy( this->Image1);
    }

  if(this->ThresholdingFilterNode->GetSagittal1() == 2)
    {
    this->CurrentSlide->DeepCopy( this->Image2);
    }

  if(this->ThresholdingFilterNode->GetSagittal1() == 3)
    {
    this->CurrentSlide->DeepCopy( this->Image3);
    }

  if(this->ThresholdingFilterNode->GetSagittal1() == 4)
    {
    this->CurrentSlide->DeepCopy( this->Image4);
    }

  if(this->ThresholdingFilterNode->GetSagittal1() == 5)
    {
    this->CurrentSlide->DeepCopy( this->Image5);
    }
}

void vtkThresholdingFilterLogic::ChangeSlide2()
{
  if(this->ThresholdingFilterNode->GetPosition2() == 1)
    {
    this->CurrentSlide->DeepCopy( this->Image1);
    }

  if(this->ThresholdingFilterNode->GetPosition2() == 2)
    {
    this->CurrentSlide->DeepCopy( this->Image2);
    }

  if(this->ThresholdingFilterNode->GetPosition2() == 3)
    {
    this->CurrentSlide->DeepCopy( this->Image3);
    }

  if(this->ThresholdingFilterNode->GetPosition2() == 4)
    {
    this->CurrentSlide->DeepCopy( this->Image4);
    }

  if(this->ThresholdingFilterNode->GetPosition2() == 5)
    {
    this->CurrentSlide->DeepCopy( this->Image5);
    }

  if(this->ThresholdingFilterNode == NULL)
    {
    vtkErrorMacro("No input ThresholdingFilterNode found");
    return;
    }

  // find input volume
  vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
      this->GetMRMLScene()->GetNodeByID(
        this->ThresholdingFilterNode->GetInputVolumeRef()));

  if (inVolume == NULL)
    {
    vtkErrorMacro("No input volume found");
    return;
    }

  // find output volume
  vtkMRMLScalarVolumeNode *outVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
      this->GetMRMLScene()->GetNodeByID(
        this->ThresholdingFilterNode->GetOutputVolumeRef()));

  if (outVolume == NULL)
    {
    vtkErrorMacro("No output volume found with id= "
        << this->ThresholdingFilterNode->GetOutputVolumeRef());
    return;
    }

  std::string name (outVolume->GetName());
  std::string id (outVolume->GetID());

  outVolume->SetAndObserveTransformNodeID(inVolume->GetTransformNodeID());
  outVolume->SetName(name.c_str());
  outVolume->SetAndObserveImageData(this->CurrentSlide);
  outVolume->SetOrigin(-originOutvolume[0],-originOutvolume[1],0);
  outVolume->SetModifiedSinceRead(1);
}


int vtkThresholdingFilterLogic::InitMaxThreshold()
{
  // find input volume
    vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
        this->GetMRMLScene()->GetNodeByID(
          this->ThresholdingFilterNode->GetInputVolumeRef()));

  double maxmin[2];
  inVolume->GetImageData()->GetScalarRange(maxmin);

  return maxmin[1];
}

int vtkThresholdingFilterLogic::InitMinThreshold()
{
  // find input volume
    vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
        this->GetMRMLScene()->GetNodeByID(
          this->ThresholdingFilterNode->GetInputVolumeRef()));

  double maxmin[2];
  inVolume->GetImageData()->GetScalarRange(maxmin);

  return maxmin[0];
}

int vtkThresholdingFilterLogic::AxialMin()
{
  // find input volume
  vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
      this->GetMRMLScene()->GetNodeByID(
        this->ThresholdingFilterNode->GetInputVolumeRef()));

  double bounds[6];
  inVolume->GetImageData()->GetBounds(bounds);

  return bounds[0];
}

int vtkThresholdingFilterLogic::AxialMax()
{
  // find input volume
  vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
      this->GetMRMLScene()->GetNodeByID(
        this->ThresholdingFilterNode->GetInputVolumeRef()));

  double bounds[6];
  inVolume->GetImageData()->GetBounds(bounds);

  return bounds[1];
}
int vtkThresholdingFilterLogic::SagittalMax()
{
  // find input volume
    vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
        this->GetMRMLScene()->GetNodeByID(
          this->ThresholdingFilterNode->GetInputVolumeRef()));

  double bounds[6];
  inVolume->GetImageData()->GetBounds(bounds);

  return bounds[3];
}

int vtkThresholdingFilterLogic::CoronalMax()
{
  // find input volume
  vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
      this->GetMRMLScene()->GetNodeByID(
        this->ThresholdingFilterNode->GetInputVolumeRef()));

  double bounds[6];
  inVolume->GetImageData()->GetBounds(bounds);

return bounds[5];
}

