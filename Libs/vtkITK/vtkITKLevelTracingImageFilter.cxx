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
#include "vtkITKLevelTracingImageFilter.h"
#include "itkLevelTracingImageFilter.h"

#include "vtkCellArray.h"
#include "vtkCharArray.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkLongArray.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkShortArray.h"
#include "vtkStructuredPoints.h"
#include "vtkUnsignedCharArray.h"
#include "vtkUnsignedIntArray.h"
#include "vtkUnsignedLongArray.h"
#include "vtkUnsignedShortArray.h"
#include "vtkStreamingDemandDrivenPipeline.h"

#include "itkImage.h"
#include "itkExtractImageFilter.h"
#include "itkChainCodePath.h"

#include "itkImageFileWriter.h"

vtkCxxRevisionMacro(vtkITKLevelTracingImageFilter, "$Revision: 1.0 $");
vtkStandardNewMacro(vtkITKLevelTracingImageFilter);

const __int64 itk::NumericTraits<__int64>::Zero = 0;
const __int64 itk::NumericTraits<__int64>::One = 1;

const unsigned __int64 itk::NumericTraits<unsigned __int64>::Zero = 0ui64;
const unsigned __int64 itk::NumericTraits<unsigned __int64>::One = 1ui64;

// Description:
// Construct object with initial range (0,1) and single contour value
// of 0.0. ComputeNormal is on, ComputeGradients is off and ComputeScalars is on.
vtkITKLevelTracingImageFilter::vtkITKLevelTracingImageFilter()
{
  this->Seed[0] = 0;
  this->Seed[1] = 0;
  this->Seed[2] = 0;
}

vtkITKLevelTracingImageFilter::~vtkITKLevelTracingImageFilter()
{
}


template <class T>
void vtkITKLevelTracingTrace(vtkITKLevelTracingImageFilter *self, T* scalars,
                             int dims[3], int extent[6], double origin[3], double spacing[3],
                             vtkDataArray *newScalars,
                             vtkPoints *newPoints,
                             vtkCellArray *newPolys,
                             int seed[3], int plane)
{

  // Wrap scalars into an ITK image
  typedef itk::Image<T, 3> ImageType;
  ImageType::Pointer image = ImageType::New();
  image->GetPixelContainer()->SetImportPointer(scalars, dims[0]*dims[1]*dims[2], false);
  image->SetOrigin( origin );
  image->SetSpacing( spacing );

  ImageType::RegionType region;
  ImageType::IndexType index;
  ImageType::SizeType size;
  index[0] = extent[0];   
  index[1] = extent[2];
  index[2] = extent[4];
  region.SetIndex( index );
  size[0] = extent[1] - extent[0] + 1;
  size[1] = extent[3] - extent[2] + 1;
  size[2] = extent[5] - extent[4] + 1;
  region.SetSize( size );
  image->SetRegions(region);

  // Extract the 2D slice to process
  typedef itk::Image<T,2> Image2DType;
  typedef itk::ExtractImageFilter<ImageType, Image2DType> ExtractType;
  ExtractType::Pointer extract = ExtractType::New();

  typedef ExtractType::InputImageRegionType ExtractionRegionType;
  ExtractionRegionType extractRegion;
  ExtractionRegionType::IndexType extractIndex;
  ExtractionRegionType::SizeType extractSize;

  extractIndex = index;
  extractSize = size;
  extract->SetInput( image );

  // Trace the level curve using itk::LevelTracingImageFilter
  typedef itk::LevelTracingImageFilter<Image2DType, Image2DType> LevelTracingType;
  LevelTracingType::Pointer tracing = LevelTracingType::New();

  itk::Index<2> seed2D = {{0,0}};
  switch(plane)
  {
  case 0: //YZ plane
    seed2D[0] = seed[1];
    seed2D[1] = seed[2];
    extractSize[0] = 0;
    break;
  case 1:  //XZ plane
    seed2D[0] = seed[0];
    seed2D[1] = seed[2];
    extractSize[1] = 0;
    break;
  case 2:  // XY plane (axials)
    seed2D[0] = seed[0];
    seed2D[1] = seed[1];
    extractSize[2] = 0;
    break;
  }

  extractRegion.SetIndex( extractIndex );
  extractRegion.SetSize( extractSize );
  extract->SetExtractionRegion( extractRegion );  

  tracing->SetSeed(seed2D);

  tracing->SetInput( extract->GetOutput() );
  tracing->Update();

  // Convert chain code output to points and polys (remember to put them on the right slice: XY, XZ, YZ)
  // Also put the scalars in...

  typedef itk::ChainCodePath<2> ChainCodePathType;
  ChainCodePathType::Pointer chain;
  typedef ChainCodePathType::OffsetType OffsetType;

  chain = tracing->GetPathOutput();

  Image2DType::IndexType chainTemp = chain->GetStart();
  OffsetType offset;

  const unsigned int numberChain = chain->NumberOfSteps();
  vtkIdType * ptIds;
  ptIds = new vtkIdType [numberChain];

  unsigned int i=0;
  ImageType::IndexType chain3D;

  do
  {
    switch( plane )
    {
    case 0:  //YZ plane
      chain3D[0] = seed[0];
      chain3D[1] = chainTemp[0] ;
      chain3D[2] = chainTemp[1] ;
      break;
    case 1:  //XZ plane
      chain3D[1] = seed[1];
      chain3D[0] = chainTemp[0] ;
      chain3D[2] = chainTemp[1] ;
      break;
    case 2:  //XY plane
      chain3D[2] = seed[2];
      chain3D[0] = chainTemp[0] ;
      chain3D[1] = chainTemp[1] ;
      break;
    }

    newPoints->InsertPoint(i, chain3D[0], chain3D[1], chain3D[2]);
    ptIds[i] = i;
    offset = chain->IncrementInput(i);
    chainTemp[0] = chainTemp[0] + offset[0];
    chainTemp[1] = chainTemp[1] + offset[1];
    //vtkGenericWarningMacro( << "Chain point: "  << chainTemp );  
  }
  while( i < numberChain );

  newPolys->InsertNextCell( i, ptIds );

  delete [] ptIds;
}

//
// Contouring filter specialized for volumes and "short int" data values.  
//
int vtkITKLevelTracingImageFilter::RequestData(
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
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkPoints *newPts;
  vtkCellArray *newPolys;
  vtkFloatArray *newScalars;
  vtkPointData *pd;
  vtkDataArray *inScalars;
  int dims[3], extent[6];
  int estimatedSize;
  double spacing[3], origin[3];

  vtkDebugMacro(<< "Executing level tracing");

  //
  // Initialize and check input
  //
  pd=input->GetPointData();
  if (pd ==NULL)
  {
    vtkErrorMacro(<<"PointData is NULL");
    return 1;
  }
  inScalars=pd->GetScalars();
  if ( inScalars == NULL )
  {
    vtkErrorMacro(<<"Scalars must be defined for level tracing");
    return 1;
  }

  input->GetDimensions(dims);
  input->GetOrigin(origin);
  input->GetSpacing(spacing);

  inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), extent);

  // estimate the number of points from the volume dimensions
  estimatedSize = (int) pow ((double) (dims[0] * dims[1] * dims[2]), .75);
  estimatedSize = estimatedSize / 1024 * 1024; //multiple of 1024
  if (estimatedSize < 1024)
  {
    estimatedSize = 1024;
  }
  vtkDebugMacro(<< "Estimated allocation size is " << estimatedSize);
  
  newPts = vtkPoints::New(); 
  newPts->Allocate(estimatedSize,estimatedSize/2);

  newPolys = vtkCellArray::New();
  newPolys->Allocate(newPolys->EstimateSize(estimatedSize,2));

  newScalars = vtkFloatArray::New();
  newScalars->Allocate(estimatedSize,estimatedSize/2);

  if (inScalars->GetNumberOfComponents() == 1 )
  {
    void* scalars = inScalars->GetVoidPointer(0);
    switch (inScalars->GetDataType())
    {
      vtkTemplateMacro(
        vtkITKLevelTracingTrace(this, static_cast<VTK_TT*>(scalars),
        dims,extent,origin,spacing,
        newScalars,newPts,newPolys,this->Seed, this->Plane
        )
        );
    } //switch
  }

  else //multiple components - have to convert
  {
    vtkErrorMacro(<< "Can only trace scalar images.");
  }

  vtkDebugMacro(<<"Created: " 
    << newPts->GetNumberOfPoints() << " points. " );
  //
  // Update ourselves.  Because we don't know up front how many edges
  // we've created, take care to reclaim memory. 
  //
  output->SetPoints(newPts);
  newPts->Delete();

  output->SetPolys(newPolys);
  newPolys->Delete();

  if (newScalars)
  {
    int idx = output->GetPointData()->AddArray(newScalars);
    output->GetPointData()->SetActiveAttribute(idx, vtkDataSetAttributes::SCALARS);
    newScalars->Delete();
  }

  output->Squeeze();
  return 1;
}





int vtkITKLevelTracingImageFilter::FillInputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
  return 1;
}

void vtkITKLevelTracingImageFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Seed point location: [" << Seed[0] << "," << Seed[1] << "," << Seed[2] << "]"
    << std::endl;

}
