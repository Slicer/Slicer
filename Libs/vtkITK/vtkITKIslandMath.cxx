/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkITK
  Module:    $HeadURL: http://www.na-mic.org/svn/Slicer3/trunk/Libs/vtkITK/vtkITKIslandMath.cxx $
  Date:      $Date: 2006-12-21 07:21:52 -0500 (Thu, 21 Dec 2006) $
  Version:   $Revision: 1900 $

==========================================================================*/

#include "vtkITKIslandMath.h"
#include "vtkObjectFactory.h"

#include "vtkPointData.h"
#include "vtkDataArray.h"
#include "vtkImageData.h"

#include "itkImage.h"
#include "itkConnectedComponentImageFilter.h"
#include "itkRelabelComponentImageFilter.h"

vtkCxxRevisionMacro(vtkITKIslandMath, "$Revision: 1900 $");
vtkStandardNewMacro(vtkITKIslandMath);

vtkITKIslandMath::vtkITKIslandMath()
{
  this->FullyConnected = 0;
  this->SliceBySlice = 0;
  this->MinimumSize = 0;
  this->MaximumSize = VTK_LARGE_ID;
}

vtkITKIslandMath::~vtkITKIslandMath()
{
}

void vtkITKIslandMath::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "FullyConnected: " << FullyConnected << std::endl;
  os << indent << "SliceBySlice: " << SliceBySlice << std::endl;
  os << indent << "MinimumSize: " << MinimumSize << std::endl;
  os << indent << "MaximumSize: " << MaximumSize << std::endl;
}

template <class T>
void vtkITKIslandMathExecute(vtkITKIslandMath *self, vtkImageData* input,
                vtkImageData* output,
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
  typename ImageType::RegionType region;
  typename ImageType::IndexType index;
  typename ImageType::SizeType size;

  inImage->GetPixelContainer()->SetImportPointer(inPtr, dims[0]*dims[1]*dims[2], false);
  index[0] = index[1] = index[2] = 0;
  region.SetIndex(index);
  size[0] = dims[0]; size[1] = dims[1]; size[2] = dims[2];
  region.SetSize(size);
  inImage->SetLargestPossibleRegion(region);
  inImage->SetBufferedRegion(region);
  inImage->SetSpacing(spacing);


  // Calculate the island operation
  // ccfilter - identifies the islands
  // relabel - sorts them by size
  typedef itk::ConnectedComponentImageFilter<ImageType, ImageType> ConnectedComponentType;
  typename ConnectedComponentType::Pointer ccfilter = ConnectedComponentType::New();
  typedef itk::RelabelComponentImageFilter<ImageType, ImageType> RelabelComponentType;
  typename RelabelComponentType::Pointer relabel = RelabelComponentType::New();

  ccfilter->SetFullyConnected(self->GetFullyConnected());

  ccfilter->SetInput( inImage );
  relabel->SetInput( ccfilter->GetOutput() );
  relabel->SetMinimumObjectSize( self->GetMinimumSize() );
  relabel->Update();

  // Copy to the output
  memcpy(outPtr, relabel->GetOutput()->GetBufferPointer(),
         relabel->GetOutput()->GetBufferedRegion().GetNumberOfPixels() * sizeof(T));

}




//
// 
//
void vtkITKIslandMath::SimpleExecute(vtkImageData *input, vtkImageData *output)
{
  vtkDebugMacro(<< "Executing Island Math");

  //
  // Initialize and check input
  //
  vtkPointData *pd = input->GetPointData();
  pd=input->GetPointData();
  if (pd ==NULL)
    {
    vtkErrorMacro(<<"PointData is NULL");
    return;
    }
  vtkDataArray *inScalars=pd->GetScalars();
  if ( inScalars == NULL )
    {
    vtkErrorMacro(<<"Scalars must be defined for island math");
    return;
    }

  if (inScalars->GetNumberOfComponents() == 1 )
    {

////////// These types are not defined in itk ////////////
#undef VTK_TYPE_USE_LONG_LONG
#undef VTK_TYPE_USE___INT64

#define CALL  vtkITKIslandMathExecute(this, input, output, static_cast<VTK_TT *>(inPtr), static_cast<VTK_TT *>(outPtr));

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
    vtkErrorMacro(<< "Can only calculate on single component scalar.");
    }
}
