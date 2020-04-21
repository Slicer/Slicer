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
#include "vtkITKLevelTracing3DImageFilter.h"
#include "itkLevelTracingImageFilter.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkStructuredPoints.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkSmartPointer.h"
#include "vtkUnsignedCharArray.h"
#include <vtkVersion.h>


vtkStandardNewMacro(vtkITKLevelTracing3DImageFilter);

// Description:
// Construct object with initial range (0,1) and single contour value
// of 0.0. ComputeNormal is on, ComputeGradients is off and ComputeScalars is on.
vtkITKLevelTracing3DImageFilter::vtkITKLevelTracing3DImageFilter()
{
  this->Seed[0] = 0;
  this->Seed[1] = 0;
  this->Seed[2] = 0;
}

vtkITKLevelTracing3DImageFilter::~vtkITKLevelTracing3DImageFilter() = default;


template <class T>
void vtkITKLevelTracing3DTrace(vtkITKLevelTracing3DImageFilter *vtkNotUsed(self),
                               T* scalars, int dims[3], int extent[6],
                               double origin[3], double spacing[3],
                               unsigned char *oscalars,
                               int seed[3])
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

  // Trace the level curve using itk::LevelTracingImageFilter
  typedef itk::Image<unsigned char, 3> LabelImageType;
  typedef itk::LevelTracingImageFilter<ImageType, LabelImageType> LevelTracingType;
  typename LevelTracingType::Pointer tracing = LevelTracingType::New();

  typename ImageType::IndexType seedIndex;
  seedIndex[0] = seed[0];
  seedIndex[1] = seed[1];
  seedIndex[2] = seed[2];

  tracing->SetSeed(seedIndex);

  tracing->SetInput( image );
  tracing->Update();

  // Copy to the output
  memcpy(oscalars, tracing->GetOutput()->GetBufferPointer(),
         tracing->GetOutput()->GetBufferedRegion().GetNumberOfPixels());

}

//
// Contouring filter specialized for volumes and "short int" data values.
//
int vtkITKLevelTracing3DImageFilter::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkImageData *input = vtkImageData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkImageData *output = vtkImageData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  output->SetExtent(
    outInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT()));
  output->AllocateScalars(outInfo);

  vtkUnsignedCharArray *oScalars
    = vtkUnsignedCharArray::SafeDownCast(output->GetPointData()->GetScalars());
  void* os = oScalars->GetVoidPointer(0);

  vtkDataArray *inScalars;
  int dims[3], extent[6];
  double spacing[3], origin[3];

  vtkDebugMacro(<< "Executing level tracing");

  //
  // Initialize and check input
  //
  vtkPointData *pd=input->GetPointData();
  if (pd ==nullptr)
  {
    vtkErrorMacro(<<"PointData is NULL");
    return 1;
  }
  inScalars=pd->GetScalars();
  if ( inScalars == nullptr )
  {
    vtkErrorMacro(<<"Scalars must be defined for level tracing");
    return 1;
  }

  input->GetDimensions(dims);
  input->GetOrigin(origin);
  input->GetSpacing(spacing);

  inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), extent);

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
        vtkITKLevelTracing3DTrace(this, static_cast<VTK_TT*>(scalars),
                                  dims,extent,origin,spacing,
                                  (unsigned char*) os,this->Seed)
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

      grayScalars->SetTypedTuple(i, &out);
      }

    vtkITKLevelTracing3DTrace(this,
                              (unsigned char *)grayScalars->GetVoidPointer(0),
                              dims, extent, origin, spacing,
                              (unsigned char *)os, this->Seed);
    }
  else
    {
    vtkErrorMacro(<< "Can only trace scalar and RGB images.");
    }

  return 1;
}



int vtkITKLevelTracing3DImageFilter::RequestInformation(
  vtkInformation *request,
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  this->Superclass::RequestInformation(request, inputVector, outputVector);

  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);

  vtkDataObject::SetPointDataActiveScalarInfo(outInfo, VTK_UNSIGNED_CHAR, 1);

  int wholeExtent[6];
  inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),wholeExtent);
  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),
               wholeExtent, 6);

  return 1;
}


int vtkITKLevelTracing3DImageFilter::FillInputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
  return 1;
}

void vtkITKLevelTracing3DImageFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Seed point location: [" << Seed[0] << "," << Seed[1] << "," << Seed[2] << "]"
    << std::endl;
}
