/*=========================================================================

Program:   Visualization Toolkit
Module:    $RCSfile: vtkMarchingCubes.cxx,v $

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.


=========================================================================*/
#include "vtkITKWandImageFilter.h"
#include "itkConnectedThresholdImageFilter.h"

#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkUnsignedCharArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkPointData.h"
#include "vtkStructuredPoints.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkSmartPointer.h"

#include "itkImage.h"
#include "itkExtractImageFilter.h"
#include "itkChainCodePath.h"
#include "itkNumericTraits.h"

vtkCxxRevisionMacro(vtkITKWandImageFilter, "$Revision: 1.0 $");
vtkStandardNewMacro(vtkITKWandImageFilter);

// Description:
// Construct object with initial range (0,1) and single contour value
// of 0.0. ComputeNormal is on, ComputeGradients is off and ComputeScalars is on.
vtkITKWandImageFilter::vtkITKWandImageFilter()
{
  this->Seed[0] = 0;
  this->Seed[1] = 0;
  this->Seed[2] = 0;

  this->Plane = 2;  // Default to XY plane

  this->DynamicRangePercentage = 0.1;
}

vtkITKWandImageFilter::~vtkITKWandImageFilter()
{
}


template <class T>
void vtkITKWand(vtkITKWandImageFilter *self, T* scalars,
                int dims[3], int extent[6], double origin[3], double spacing[3],
                unsigned char *oscalars, 
                int seed[3], int plane, double delta)
{
  // Wrap scalars into an ITK image
  typedef itk::Image<T, 3> ImageType;
  typename ImageType::Pointer image = ImageType::New();
  image->GetPixelContainer()->SetImportPointer(scalars, dims[0]*dims[1]*dims[2], false);
  image->SetOrigin( origin );
  image->SetSpacing( spacing );

  typename ImageType::RegionType region;
  typename ImageType::IndexType index;
  typename ImageType::SizeType size;
  index[0] = extent[0];   
  index[1] = extent[2];
  index[2] = extent[4];
  region.SetIndex( index );
  size[0] = extent[1] - extent[0] + 1;
  size[1] = extent[3] - extent[2] + 1;
  size[2] = extent[5] - extent[4] + 1;
  region.SetSize( size );
  image->SetRegions(region);

  // get the value at the seed location
  typename ImageType::IndexType ind;
  ind[0] = seed[0];
  ind[1] = seed[1];
  ind[2] = seed[2];

  if (!image->GetRequestedRegion().IsInside( ind ))
    {
    return;
    }
  
  T threshold = image->GetPixel(ind);

  // Extract the 2D slice to process
  typedef itk::Image<T,2> Image2DType;
  typedef itk::ExtractImageFilter<ImageType, Image2DType> ExtractType;
  typename ExtractType::Pointer extract = ExtractType::New();

  typedef typename ExtractType::InputImageRegionType ExtractionRegionType;
  ExtractionRegionType extractRegion;
  typename ExtractionRegionType::IndexType extractIndex;
  typename ExtractionRegionType::SizeType extractSize;

  extractIndex = index;
  extractSize = size;
  extract->SetInput( image );

  // Segment using itk::WandImageFilter
  typedef itk::Image<unsigned char,2> SegmentImage2DType;  
  typedef itk::ConnectedThresholdImageFilter<Image2DType, SegmentImage2DType> WandType;
  typename WandType::Pointer wand = WandType::New();

  itk::Index<2> seed2D = {{0,0}};
  switch(plane)
  {
  case 0: //JK plane
    seed2D[0] = seed[1];
    seed2D[1] = seed[2];
    extractSize[0] = 0;
    extractIndex[0] = seed[0];
    break;
  case 1:  //IK plane
    seed2D[0] = seed[0];
    seed2D[1] = seed[2];
    extractSize[1] = 0;
    extractIndex[1] = seed[1];
    break;
  case 2:  //IJ plane (axials)
    seed2D[0] = seed[0];
    seed2D[1] = seed[1];
    extractSize[2] = 0;
    extractIndex[2] = seed[2];
    break;
  }

  extractRegion.SetIndex( extractIndex );
  extractRegion.SetSize( extractSize );
  extract->SetExtractionRegion( extractRegion );  

  wand->SetSeed(seed2D);
  wand->SetLower(threshold - static_cast<T>(delta));
  wand->SetUpper(threshold + static_cast<T>(delta));
  wand->SetReplaceValue(1);

  wand->SetInput( extract->GetOutput() );
  wand->Update();

  // Copy the output
  itk::ImageRegionIterator<SegmentImage2DType>
    inIt(wand->GetOutput(), wand->GetOutput()->GetBufferedRegion());
  unsigned char *oit = oscalars;

  while (!inIt.IsAtEnd())
    {
    *oit = inIt.Get();
    ++inIt;
    ++oit;
    }
}



//
// 
//
int vtkITKWandImageFilter::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and ouptut
  vtkImageData *input = vtkImageData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkImageData *output = vtkImageData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  output->SetExtent(
    outInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT()));
  output->AllocateScalars();

  vtkUnsignedCharArray *oScalars
    = vtkUnsignedCharArray::SafeDownCast(output->GetPointData()->GetScalars());
  void* os = oScalars->GetVoidPointer(0);
  
  vtkDataArray *inScalars;
  int dims[3], extent[6];
  double spacing[3], origin[3];

  vtkDebugMacro(<< "Executing wand selection");

  //
  // Initialize and check input
  //
  vtkPointData *pd = input->GetPointData();
  pd=input->GetPointData();
  if (pd ==NULL)
  {
    vtkErrorMacro(<<"PointData is NULL");
    return 1;
  }
  inScalars=pd->GetScalars();
  if ( inScalars == NULL )
  {
    vtkErrorMacro(<<"Scalars must be defined for wand selection");
    return 1;
  }

  input->GetDimensions(dims);
  input->GetOrigin(origin);
  input->GetSpacing(spacing);

  inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), extent);

  if (inScalars->GetNumberOfComponents() == 1 )
    {
    double *range, delta;
    range = inScalars->GetRange(0);
    delta = this->DynamicRangePercentage*(range[1] - range[0]);

    void* scalars = inScalars->GetVoidPointer(0);
    switch (inScalars->GetDataType())
      {
      vtkTemplateMacro(
        vtkITKWand(this, static_cast<VTK_TT*>(scalars),
                   dims,extent,origin,spacing,
                   (unsigned char *)os,this->Seed, this->Plane, delta)
        );
      } //switch
    }
  else if (inScalars->GetNumberOfComponents() == 3) 
    {
    // RGB - convert for now...
    vtkSmartPointer<vtkUnsignedCharArray> grayScalars
      = vtkUnsignedCharArray::New();
    grayScalars->SetNumberOfTuples( inScalars->GetNumberOfTuples() );
    
    double in[3];
    unsigned char out;
    for (vtkIdType i=0; i < inScalars->GetNumberOfTuples(); ++i)
      {
      inScalars->GetTuple(i, in);

      out = static_cast<unsigned char>((2125.0 * in[0] +  7154.0 * in[1] +  0721.0 * in[2]) / 10000.0);

      grayScalars->SetTupleValue(i, &out);
      }

    double *range, delta;
    range = grayScalars->GetRange(0);
    delta = this->DynamicRangePercentage*(range[1] - range[0]);

    vtkITKWand(this,
               (unsigned char *)grayScalars->GetVoidPointer(0),
               dims, extent, origin, spacing, 
               (unsigned char *)os, this->Seed, this->Plane, delta);
    }
  else
    {
    vtkErrorMacro(<< "Can only select scalar and RGB images.");
    }

  return 1;
}

int vtkITKWandImageFilter::FillInputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
  return 1;
}

int vtkITKWandImageFilter::RequestInformation(
  vtkInformation *request,
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  this->Superclass::RequestInformation(request, inputVector, outputVector);

  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkDataObject::SetPointDataActiveScalarInfo(outInfo, VTK_UNSIGNED_CHAR, 1);

  return 1;
}

void vtkITKWandImageFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Seed point location: [" << Seed[0] << "," << Seed[1] << "," << Seed[2] << "]"
    << std::endl;
  os << indent << "Plane: ";
  switch (Plane)
    {
    case 2: os << "IJ" << std::endl;
      break;
    case 1: os << "IK" << std::endl;
      break;
    case 0: os << "JK" << std::endl;
      break;
    default: os << "(unknown)" << std::endl;
      break;
    }
  os << indent << "Dynamic range percentage: " << DynamicRangePercentage << std::endl;
}
