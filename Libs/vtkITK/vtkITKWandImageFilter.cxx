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

#include "vtkDataArray.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkImageData.h"

#include "itkConnectedThresholdImageFilter.h"

vtkStandardNewMacro(vtkITKWandImageFilter);

// Description:
vtkITKWandImageFilter::vtkITKWandImageFilter()
{
  this->Seed[0] = 0;
  this->Seed[1] = 0;
  this->Seed[2] = 0;

  this->DynamicRangePercentage = 0.1;
}

vtkITKWandImageFilter::~vtkITKWandImageFilter() = default;


template <class T>
void vtkITKWandExecute(vtkITKWandImageFilter *self, vtkImageData* input,
                vtkImageData* vtkNotUsed(output),
                T* inPtr, T* outPtr)
{

  int dims[3];
  input->GetDimensions(dims);

  // Wrap scalars into an ITK image
  // - mostly rely on defaults for spacing, origin etc for this filter
  typedef itk::Image<T, 3> ImageType;
  typename ImageType::Pointer inImage = ImageType::New();
  inImage->GetPixelContainer()->SetImportPointer(inPtr, dims[0]*dims[1]*dims[2], false);
  typename ImageType::RegionType region;
  typename ImageType::IndexType index;
  typename ImageType::SizeType size;
  index[0] = index[1] = index[2] = 0;
  size[0] = dims[0];
  size[1] = dims[1];
  size[2] = dims[2];
  region.SetIndex(index);
  region.SetSize(size);
  inImage->SetLargestPossibleRegion(region);
  inImage->SetBufferedRegion(region);

  // get the value at the seed location
  typename ImageType::IndexType ind;
  ind[0] = self->GetSeed()[0];
  ind[1] = self->GetSeed()[1];
  ind[2] = self->GetSeed()[2];

  if ( ind[0] < 0 || ind[0] >= dims[0] ||
       ind[1] < 0 || ind[1] >= dims[1] ||
       ind[2] < 0 || ind[2] >= dims[2] )
    {
    vtkWarningWithObjectMacro(self, "Seed (" << ind[0] << ", " << ind[1] << ", " << ind[2] << ") is not in input region ("
      << dims[0] << ", " << dims[1] << ", " << dims[2] << ")" );
    return;
    }

  // Segment using itk::WandImageFilter
  typedef itk::Image<T,3> SegmentImageType;
  typedef itk::ConnectedThresholdImageFilter<ImageType, SegmentImageType> WandType;
  typename WandType::Pointer wand = WandType::New();

  wand->SetSeed(ind);

  double *range, delta;
  range = input->GetScalarRange();
  delta = self->GetDynamicRangePercentage()*(range[1] - range[0]);
  T threshold = static_cast<T> (input->GetScalarComponentAsDouble(ind[0], ind[1], ind[2], 0));
  wand->SetLower(threshold - static_cast<T>(delta));
  wand->SetUpper(threshold + static_cast<T>(delta));

  wand->SetReplaceValue(1);

  wand->SetInput( inImage );
  wand->Update();

  // Copy to the output
  memcpy(outPtr, wand->GetOutput()->GetBufferPointer(),
         wand->GetOutput()->GetBufferedRegion().GetNumberOfPixels() * sizeof(T));

}



//
//
//
void vtkITKWandImageFilter::SimpleExecute(vtkImageData *input, vtkImageData *output)
{
  vtkDebugMacro(<< "Executing wand selection");

  //
  // Initialize and check input
  //
  vtkPointData *pd = input->GetPointData();
  pd=input->GetPointData();
  if (pd ==nullptr)
    {
    vtkErrorMacro(<<"PointData is NULL");
    return;
    }
  vtkDataArray *inScalars=pd->GetScalars();
  if ( inScalars == nullptr )
    {
    vtkErrorMacro(<<"Scalars must be defined for wand selection");
    return;
    }

  if (inScalars->GetNumberOfComponents() == 1 )
    {

////////// These types are not defined in itk ////////////
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

    void* inPtr = input->GetScalarPointer();
    void* outPtr = output->GetScalarPointer();

    switch (inScalars->GetDataType())
      {
      vtkTemplateMacro(
        vtkITKWandExecute(this, input, output,
            static_cast<VTK_TT *>(inPtr),
            static_cast<VTK_TT *>(outPtr)));
      } //switch
    }
  else
    {
    vtkErrorMacro(<< "Can only select scalar.");
    }
}

void vtkITKWandImageFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Seed point location: ["
    << Seed[0] << "," << Seed[1] << "," << Seed[2] << "]" << std::endl;
  os << indent << "Dynamic range percentage: " << DynamicRangePercentage << std::endl;
}
