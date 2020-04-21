/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/

#include "vtkITKDistanceTransform.h"
#include "vtkObjectFactory.h"

#include "vtkDataArray.h"
#include "vtkPointData.h"
#include "vtkImageData.h"

#include "itkSignedMaurerDistanceMapImageFilter.h"

vtkStandardNewMacro(vtkITKDistanceTransform);

vtkITKDistanceTransform::vtkITKDistanceTransform()
{
  this->SquaredDistance = 1;
  this->InsideIsPositive = 0;
  this->UseImageSpacing = 0;
  this->BackgroundValue = 0;
}

vtkITKDistanceTransform::~vtkITKDistanceTransform() = default;


template <class T>
void vtkITKDistanceTransformExecute(vtkITKDistanceTransform *self, vtkImageData* input,
                vtkImageData* vtkNotUsed(output),
                T* inPtr, T* outPtr)
{

  int dims[3];
  input->GetDimensions(dims);
  double spacing[3];
  input->GetSpacing(spacing);

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
  inImage->SetSpacing(spacing);


  // Calculate the distance transform
  typedef itk::Image<T,3> DistanceImageType;
  typedef itk::SignedMaurerDistanceMapImageFilter<ImageType, DistanceImageType> DistanceType;
  typename DistanceType::Pointer dist = DistanceType::New();

  dist->SetBackgroundValue(static_cast<T>(self->GetBackgroundValue()));
  dist->SetUseImageSpacing(self->GetUseImageSpacing());
  dist->SetInsideIsPositive(self->GetInsideIsPositive());
  dist->SetSquaredDistance(self->GetSquaredDistance());

  dist->SetInput( inImage );
  dist->Update();

  // Copy to the output
  memcpy(outPtr, dist->GetOutput()->GetBufferPointer(),
         dist->GetOutput()->GetBufferedRegion().GetNumberOfPixels() * sizeof(T));

}




//
//
//
void vtkITKDistanceTransform::SimpleExecute(vtkImageData *input, vtkImageData *output)
{
  vtkDebugMacro(<< "Executing distance transform");

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
    vtkErrorMacro(<<"Scalars must be defined for distance transform");
    return;
    }

  if (inScalars->GetNumberOfComponents() == 1 )
    {

////////// These types are not defined in itk ////////////
#undef VTK_TYPE_USE_LONG_LONG
#undef VTK_TYPE_USE___INT64

#define CALL  vtkITKDistanceTransformExecute(this, input, output, static_cast<VTK_TT *>(inPtr), static_cast<VTK_TT *>(outPtr));

    void* inPtr = input->GetScalarPointer();
    void* outPtr = output->GetScalarPointer();

    switch (inScalars->GetDataType())
      {
      vtkTemplateMacroCase(VTK_DOUBLE, double, CALL);                           \
      vtkTemplateMacroCase(VTK_FLOAT, float, CALL);                             \
      vtkTemplateMacroCase(VTK_LONG, long, CALL);                               \
      vtkTemplateMacroCase(VTK_UNSIGNED_LONG, unsigned long, CALL);             \
      vtkTemplateMacroCase(VTK_INT, int, CALL);                                 \
      vtkTemplateMacroCase(VTK_UNSIGNED_INT, unsigned int, CALL);               \
      vtkTemplateMacroCase(VTK_SHORT, short, CALL);                             \
      vtkTemplateMacroCase(VTK_UNSIGNED_SHORT, unsigned short, CALL);           \
      vtkTemplateMacroCase(VTK_CHAR, char, CALL);                               \
      vtkTemplateMacroCase(VTK_SIGNED_CHAR, signed char, CALL);                 \
      vtkTemplateMacroCase(VTK_UNSIGNED_CHAR, unsigned char, CALL);
      } //switch
    }
  else
    {
    vtkErrorMacro(<< "Can only calculate on scalar.");
    }
}

void vtkITKDistanceTransform::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "BackgroundValue: " << BackgroundValue << std::endl;
  os << indent << "InsideIsPositive: " << InsideIsPositive << std::endl;
  os << indent << "UseImageSpacing: " << UseImageSpacing << std::endl;
  os << indent << "SquaredDistance: " << SquaredDistance << std::endl;
}


