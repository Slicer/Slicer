/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women’s Hospital through NIH grant R01MH112748.

==============================================================================*/

/// vtkITK includes
#include "vtkITKImageMargin.h"

/// VTK includes
#include <vtkAlgorithm.h>
#include <vtkDataArray.h>
#include <vtkImageData.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>

/// ITK includes
#include <itkBinaryThresholdImageFilter.h>
#include <itkCommand.h>
#include <itkSignedMaurerDistanceMapImageFilter.h>

vtkStandardNewMacro(vtkITKImageMargin);

//----------------------------------------------------------------------------
vtkITKImageMargin::vtkITKImageMargin()
  : InnerMarginMM(vtkMath::NegInf())
  , InnerMarginVoxels(vtkMath::NegInf())
{
}

//----------------------------------------------------------------------------
vtkITKImageMargin::~vtkITKImageMargin() = default;

//----------------------------------------------------------------------------
void vtkITKImageMargin::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
// signed distance field
// sdf and sdfMargin are based on:
// https://github.com/KitwareMedical/HASI/blob/fe38e0f08682fd3ba2fd9ec816118b844321fdb8/segmentBonesInMicroCT.cxx#L102-L155
template <typename InputPixelType, unsigned Dimension>
itk::SmartPointer<itk::Image<float, Dimension> >
sdf(itk::SmartPointer<itk::Image<InputPixelType, Dimension> > labelImage, int backgroundValue)
{
  using RealImageType = itk::Image<float, Dimension>;
  using LabelImageType = itk::Image<InputPixelType, Dimension>;
  using DistanceFieldType = itk::SignedMaurerDistanceMapImageFilter<LabelImageType, RealImageType>;

  typename DistanceFieldType::Pointer distF = DistanceFieldType::New();
  distF->SetInput(labelImage);
  distF->SetSquaredDistance(true);
  distF->SetBackgroundValue(backgroundValue);
  distF->Update();
  return distF->GetOutput();
}

//----------------------------------------------------------------------------
template <typename ImageType>
itk::SmartPointer<ImageType>
sdfMargin(itk::SmartPointer<ImageType> labelImage, int backgroundValue, double innerMarginMM, double outerMarginMM)
{
  innerMarginMM -= std::numeric_limits<double>::epsilon();
  outerMarginMM += std::numeric_limits<double>::epsilon();

  using RealImageType = itk::Image<float, ImageType::ImageDimension>;
  using FloatThresholdType = itk::BinaryThresholdImageFilter<RealImageType, ImageType>;
  typename FloatThresholdType::Pointer sdfTh = FloatThresholdType::New();
  sdfTh->SetInput(sdf<typename ImageType::PixelType, ImageType::ImageDimension>(labelImage, backgroundValue));
  if (innerMarginMM > vtkMath::NegInf())
    {
    sdfTh->SetLowerThreshold(innerMarginMM*std::abs(innerMarginMM));
    }
  sdfTh->SetUpperThreshold(outerMarginMM*std::abs(outerMarginMM));
  sdfTh->Update();
  return sdfTh->GetOutput();
}

//----------------------------------------------------------------------------
template <class T>
void vtkITKImageMarginExecute(vtkITKImageMargin *self, vtkImageData* input,
                vtkImageData* vtkNotUsed(output),
                T* inPtr, T* outPtr)
{
  try
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

    inImage->GetPixelContainer()->SetImportPointer(inPtr, dims[0] * dims[1] * dims[2], false);
    index[0] = index[1] = index[2] = 0;
    region.SetIndex(index);
    size[0] = dims[0]; size[1] = dims[1]; size[2] = dims[2];
    region.SetSize(size);
    inImage->SetLargestPossibleRegion(region);
    inImage->SetBufferedRegion(region);

    double innerMarginDistance = self->GetInnerMarginVoxels();
    double outerMarginDistance = self->GetOuterMarginVoxels();
    if (self->GetCalculateMarginInMM())
      {
      inImage->SetSpacing(spacing);
      innerMarginDistance = self->GetInnerMarginMM();
      outerMarginDistance = self->GetOuterMarginMM();
      }

    itk::SmartPointer<ImageType> outputImage;
    outputImage = sdfMargin<ImageType>(inImage, self->GetBackgroundValue(), innerMarginDistance, outerMarginDistance);

    // Copy to the output
    memcpy(outPtr, outputImage->GetBufferPointer(), outputImage->GetBufferedRegion().GetNumberOfPixels() * sizeof(T));
    }
  catch (itk::ExceptionObject & err)
    {
    vtkErrorWithObjectMacro(self, "Failed to compute margin. Details: " << err);
    }
}

//----------------------------------------------------------------------------
void vtkITKImageMargin::SimpleExecute(vtkImageData *input, vtkImageData *output)
{
  vtkDebugMacro(<< "Executing Image Margin");

  if (this->GetInnerMarginMM() > this->GetOuterMarginMM())
    {
    vtkErrorMacro(<< "Outer margin must be greater than inner margin");
    }

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
  if (inScalars == nullptr)
    {
    vtkErrorMacro(<< "Scalars must be defined for image margin");
    return;
    }

  if (inScalars->GetNumberOfComponents() == 1)
    {

////////// These types are not defined in itk ////////////
#undef VTK_TYPE_USE_LONG_LONG
#undef VTK_TYPE_USE___INT64

#define CALL  vtkITKImageMarginExecute(this, input, output, static_cast<VTK_TT *>(inPtr), static_cast<VTK_TT *>(outPtr));

    void* inPtr = input->GetScalarPointer();
    void* outPtr = output->GetScalarPointer();

    switch (inScalars->GetDataType())
      {
      vtkTemplateMacroCase(VTK_LONG, long, CALL);                               \
      vtkTemplateMacroCase(VTK_UNSIGNED_LONG, unsigned long, CALL);             \
      vtkTemplateMacroCase(VTK_INT, int, CALL);                                 \
      vtkTemplateMacroCase(VTK_UNSIGNED_INT, unsigned int, CALL);               \
      vtkTemplateMacroCase(VTK_SHORT, short, CALL);                             \
      vtkTemplateMacroCase(VTK_UNSIGNED_SHORT, unsigned short, CALL);           \
      vtkTemplateMacroCase(VTK_CHAR, char, CALL);                               \
      vtkTemplateMacroCase(VTK_SIGNED_CHAR, signed char, CALL);                 \
      vtkTemplateMacroCase(VTK_UNSIGNED_CHAR, unsigned char, CALL);             \
      default:
        {
        vtkErrorMacro(<< "Incompatible data type for this version of ITK.");
        }
      } //switch
    }
  else
    {
    vtkErrorMacro(<< "Only single component images supported.");
    }
}
