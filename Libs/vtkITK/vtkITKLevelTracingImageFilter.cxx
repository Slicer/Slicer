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
#include "vtkFloatArray.h"
#include "vtkUnsignedCharArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkStructuredPoints.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkSmartPointer.h"

#include "itkImage.h"
#include "itkExtractImageFilter.h"
#include "itkChainCodePath.h"
#include "itkNumericTraits.h"

vtkCxxRevisionMacro(vtkITKLevelTracingImageFilter, "$Revision: 1.0 $");
vtkStandardNewMacro(vtkITKLevelTracingImageFilter);

// Description:
// Construct object with initial range (0,1) and single contour value
// of 0.0. ComputeNormal is on, ComputeGradients is off and ComputeScalars is on.
vtkITKLevelTracingImageFilter::vtkITKLevelTracingImageFilter()
{
  this->Seed[0] = 0;
  this->Seed[1] = 0;
  this->Seed[2] = 0;

  this->Plane = 2;  // Default to XY plane
}

vtkITKLevelTracingImageFilter::~vtkITKLevelTracingImageFilter()
{
}


template <class T>
void vtkITKLevelTracingTrace(vtkITKLevelTracingImageFilter *self, T* scalars,
                             int dims[3], int extent[6], double origin[3], double spacing[3],
                             vtkPoints *newPoints,
                             vtkCellArray *newPolys,
                             int seed[3], int plane)
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

  // Trace the level curve using itk::LevelTracingImageFilter
  typedef itk::LevelTracingImageFilter<Image2DType, Image2DType> LevelTracingType;
  typename LevelTracingType::Pointer tracing = LevelTracingType::New();

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

  tracing->SetSeed(seed2D);

  tracing->SetInput( extract->GetOutput() );
  tracing->Update();

  // Convert chain code output to points and polys (remember to put
  // them on the right slice: IJ, IK, JK)
  // Also put the scalars in...

  typedef itk::ChainCodePath<2> ChainCodePathType;
  ChainCodePathType::Pointer chain;
  typedef ChainCodePathType::OffsetType OffsetType;

  chain = tracing->GetPathOutput();

  typename Image2DType::IndexType chainTemp = chain->GetStart();
  OffsetType offset;

  const unsigned int numberChain = chain->NumberOfSteps();

  if ( numberChain == 0 )
    {
    return;
    }

  vtkIdType * ptIds;
  ptIds = new vtkIdType [numberChain];

  unsigned int i=0;
  typename ImageType::IndexType chain3D;

  do
    {
    switch( plane )
      {
      default:
      case 0:  //JK plane
        chain3D[0] = seed[0];
        chain3D[1] = chainTemp[0] ;
        chain3D[2] = chainTemp[1] ;
        break;
      case 1:  //IK plane
        chain3D[1] = seed[1];
        chain3D[0] = chainTemp[0] ;
        chain3D[2] = chainTemp[1] ;
        break;
      case 2:  //IJ plane
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

////////// These types are not defined in itk::NumericTraits ////////////
#ifdef vtkTemplateMacroCase_ui64
#undef vtkTemplateMacroCase_ui64
# define vtkTemplateMacroCase_ui64(typeN, type, call)
#endif
#ifdef vtkTemplateMacroCase_si64
#undef vtkTemplateMacroCase_si64
# define vtkTemplateMacroCase_si64(typeN, type, call)
#endif
#ifdef vtkTemplateMacroCase_ll
#undef vtkTemplateMacroCase_ll
# define vtkTemplateMacroCase_ll(typeN, type, call)
#endif
  if (inScalars->GetNumberOfComponents() == 1 )
  {
    void* scalars = inScalars->GetVoidPointer(0);
    switch (inScalars->GetDataType())
    {
      vtkTemplateMacro(
        vtkITKLevelTracingTrace(this, static_cast<VTK_TT*>(scalars),
        dims,extent,origin,spacing,
        newPts,newPolys,this->Seed, this->Plane
        )
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

    vtkITKLevelTracingTrace(this,
                            (unsigned char *)grayScalars->GetVoidPointer(0),
                            dims, extent, origin, spacing, 
                            newPts, newPolys, this->Seed, this->Plane);
    }
  else
    {
    vtkErrorMacro(<< "Can only trace scalar and RGB images.");
    }

  vtkDebugMacro(<<"Created: " 
    << newPts->GetNumberOfPoints() << " points. " );
  //
  // Update ourselves.  Because we don't know up front how many edges
  // we've created, take care to reclaim memory. 
  //
  output->SetPoints(newPts);
  newPts->Delete();

  output->SetLines(newPolys);
  newPolys->Delete();

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
}
