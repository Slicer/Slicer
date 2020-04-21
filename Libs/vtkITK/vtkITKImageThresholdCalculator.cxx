/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkITK
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/

// vtkITK includes
#include "vtkITKImageThresholdCalculator.h"

// VTK includes
#include <vtkDataArray.h>
#include <vtkImageExport.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkITKUtility.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkVersion.h>

// VTKsys includes
//#include <vtksys/SystemTools.hxx>

// ITK includes
#include "itkHistogramThresholdCalculator.h"
#include "itkHuangThresholdCalculator.h"
#include "itkImageToHistogramFilter.h"
#include "itkIntermodesThresholdCalculator.h"
#include "itkIsoDataThresholdCalculator.h"
#include "itkKittlerIllingworthThresholdCalculator.h"
#include "itkLiThresholdCalculator.h"
#include "itkMaximumEntropyThresholdCalculator.h"
#include "itkMomentsThresholdCalculator.h"
#include "itkOtsuThresholdCalculator.h"
#include "itkRenyiEntropyThresholdCalculator.h"
#include "itkShanbhagThresholdCalculator.h"
#include "itkTriangleThresholdCalculator.h"
#include "itkVTKImageImport.h"
#include "itkYenThresholdCalculator.h"

vtkStandardNewMacro(vtkITKImageThresholdCalculator);

// helper function
template <class TPixelType>
void ITKComputeThresholdFromVTKImage(vtkITKImageThresholdCalculator *self, vtkImageData *inputImage, double& computedThreshold)
{
  typedef itk::Image<TPixelType, 3> ImageType;
  typedef itk::Statistics::ImageToHistogramFilter<ImageType> HistogramGeneratorType;
  typedef typename HistogramGeneratorType::HistogramType HistogramType;
  typedef itk::HistogramThresholdCalculator<HistogramType, double> CalculatorType;

  // itk import for input itk images
  typedef typename itk::VTKImageImport<ImageType> ImageImportType;
  typename ImageImportType::Pointer itkImporter = ImageImportType::New();

  // vtk export for  vtk image
  vtkNew<vtkImageExport> vtkExporter;

  vtkExporter->SetInputData ( inputImage );

  ConnectPipelines(vtkExporter.GetPointer(), itkImporter);
  itkImporter->UpdateLargestPossibleRegion();

  typename HistogramGeneratorType::Pointer histGenerator = HistogramGeneratorType::New();
  histGenerator->SetInput(itkImporter->GetOutput());
  typename HistogramGeneratorType::HistogramSizeType hsize(1);
  hsize[0] = 64;
  histGenerator->SetHistogramSize( hsize );
  histGenerator->SetAutoMinimumMaximum( true );

  // Create and initialize the calculator
  typename CalculatorType::Pointer calculator;
  switch (self->GetMethod())
    {
    case vtkITKImageThresholdCalculator::METHOD_HUANG: calculator = itk::HuangThresholdCalculator<HistogramType>::New(); break;
    case vtkITKImageThresholdCalculator::METHOD_INTERMODES: calculator = itk::IntermodesThresholdCalculator<HistogramType>::New(); break;
    case vtkITKImageThresholdCalculator::METHOD_ISO_DATA: calculator = itk::IsoDataThresholdCalculator<HistogramType>::New(); break;
    case vtkITKImageThresholdCalculator::METHOD_KITTLER_ILLINGWORTH: calculator = itk::KittlerIllingworthThresholdCalculator<HistogramType>::New(); break;
    case vtkITKImageThresholdCalculator::METHOD_LI: calculator = itk::LiThresholdCalculator<HistogramType>::New(); break;
    case vtkITKImageThresholdCalculator::METHOD_MAXIMUM_ENTROPY: calculator = itk::MaximumEntropyThresholdCalculator<HistogramType>::New(); break;
    case vtkITKImageThresholdCalculator::METHOD_MOMENTS: calculator = itk::MomentsThresholdCalculator<HistogramType>::New(); break;
    case vtkITKImageThresholdCalculator::METHOD_OTSU: calculator = itk::OtsuThresholdCalculator<HistogramType>::New(); break;
    case vtkITKImageThresholdCalculator::METHOD_RENYI_ENTROPY: calculator = itk::RenyiEntropyThresholdCalculator<HistogramType>::New(); break;
    case vtkITKImageThresholdCalculator::METHOD_SHANBHAG: calculator = itk::ShanbhagThresholdCalculator<HistogramType>::New(); break;
    case vtkITKImageThresholdCalculator::METHOD_TRIANGLE: calculator = itk::TriangleThresholdCalculator<HistogramType>::New(); break;
    case vtkITKImageThresholdCalculator::METHOD_YEN: calculator = itk::YenThresholdCalculator<HistogramType>::New(); break;
    default:
      vtkErrorWithObjectMacro(self, "ITKComputeThresholdFromVTKImage failed: invalid method: " << self->GetMethod());
      return;
    }

  calculator->SetInput( histGenerator->GetOutput() );

  try
    {
    calculator->Update();
    }
  catch (itk::ExceptionObject &err)
    {
    vtkErrorWithObjectMacro(self, "Failed to compute threshold value using method " << self->GetMethodAsString(self->GetMethod())
      << ". Details: " << err);
    }

  computedThreshold = calculator->GetThreshold();
}

//----------------------------------------------------------------------------
vtkITKImageThresholdCalculator::vtkITKImageThresholdCalculator()
{
  this->Method = METHOD_OTSU;
  this->Threshold = 0.0;
}

//----------------------------------------------------------------------------
vtkITKImageThresholdCalculator::~vtkITKImageThresholdCalculator() = default;

//----------------------------------------------------------------------------
void vtkITKImageThresholdCalculator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Method: " << this->GetMethodAsString(this->Method) << "\n";
  os << indent << "Threshold: " << this->Threshold << "\n";
}

//----------------------------------------------------------------------------
// Writes all the data from the input.
void vtkITKImageThresholdCalculator::Update()
{
  vtkImageData *inputImage = this->GetImageDataInput(0);
  vtkPointData* pointData = nullptr;
  if (inputImage)
    {
    pointData = inputImage->GetPointData();
    }
  if (pointData == nullptr)
    {
    vtkErrorMacro(<<"vtkITKImageThresholdCalculator: No input image");
    return;
    }

  this->UpdateInformation();
  if (this->GetOutputInformation(0))
    {
    this->GetOutputInformation(0)->Set(
      vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(),
      this->GetOutputInformation(0)->Get(
        vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT()), 6);
    }

  if ( pointData->GetScalars() == nullptr)
    {
    vtkErrorMacro(<<"vtkITKImageThresholdCalculator: Scalar input image is required");
    return;
    }
  int inputNumberOfScalarComponents = pointData->GetScalars()->GetNumberOfComponents();
  if (inputNumberOfScalarComponents != 1)
    {
    vtkErrorMacro(<<"vtkITKImageThresholdCalculator: Scalar input image with a single component is required");
    return;
    }

  int inputDataType = pointData->GetScalars()->GetDataType();
  switch (inputDataType)
    {
    vtkTemplateMacro(ITKComputeThresholdFromVTKImage<VTK_TT>(this, inputImage, this->Threshold));
    default:
      vtkErrorMacro("Execute: Unknown ScalarType" << inputDataType);
      return;
    }
}

//----------------------------------------------------------------------------
const char *vtkITKImageThresholdCalculator::GetMethodAsString(int method)
{
  switch (method)
  {
    case vtkITKImageThresholdCalculator::METHOD_HUANG:
      return "Huang";
    case vtkITKImageThresholdCalculator::METHOD_INTERMODES:
      return "Intermodes";
    case vtkITKImageThresholdCalculator::METHOD_ISO_DATA:
      return "IsoData";
    case vtkITKImageThresholdCalculator::METHOD_KITTLER_ILLINGWORTH:
      return "KittlerIllingworth";
    case vtkITKImageThresholdCalculator::METHOD_LI:
      return "Li";
    case vtkITKImageThresholdCalculator::METHOD_MAXIMUM_ENTROPY:
      return "MaximumEntropy";
    case vtkITKImageThresholdCalculator::METHOD_MOMENTS:
      return "Moments";
    case vtkITKImageThresholdCalculator::METHOD_OTSU:
      return "Otsu";
    case vtkITKImageThresholdCalculator::METHOD_RENYI_ENTROPY:
      return "RenyiEntropy";
    case vtkITKImageThresholdCalculator::METHOD_SHANBHAG:
      return "Shanbhag";
    case vtkITKImageThresholdCalculator::METHOD_TRIANGLE:
      return "Triangle";
    case vtkITKImageThresholdCalculator::METHOD_YEN:
      return "Yen";
  }
  return "";
}
