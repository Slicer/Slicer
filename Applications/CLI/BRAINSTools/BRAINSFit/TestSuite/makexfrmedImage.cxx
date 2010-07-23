#include <itksys/SystemTools.hxx>
#include <iostream>
#include <stdlib.h>

#include <itkImage.h>
#include <itkAffineTransform.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkResampleImageFilter.h>
#include <itkTransformFileReader.h>
#include <itkTransformFileWriter.h>
#include "itkIO.h"
#include "ReadMask.h"
#include <math.h>

#ifndef M_PI
#  define M_PI 3.1415926
#endif
#ifndef M_TWOPI
#  define M_TWOPI ( 2.0 * M_PI )
#endif
inline double DEGREES(double x)
{
  double rval = x * ( M_PI / 180 );

  return rval;
}

bool keepOutputs(false);

//
// typedefs
typedef itk::Image< unsigned char, 3 > ImageType;
typedef itk::AffineTransform< double,
                              3 >                        AffineTransformType;
typedef itk::LinearInterpolateImageFunction< ImageType, double > InterpolatorType;
typedef itk::ResampleImageFilter< ImageType,
                                  ImageType >         ResampleImageFilter;

//
// apply an affine transform to an image, and
// return the transformed image
ImageType::Pointer
Resample(ImageType::Pointer & inputImage,
         AffineTransformType::Pointer & transform)
{
  ImageType::IndexType index = { { 0, 0, 0}};
  ImageType::PointType origin;
  ImageType::SizeType  size = inputImage->GetLargestPossibleRegion().GetSize();

  InterpolatorType::Pointer interp = InterpolatorType::New();

  interp->SetInputImage(inputImage);

  ResampleImageFilter::Pointer resample =
    ResampleImageFilter::New();
  resample->SetInput(inputImage);
  resample->SetSize(size);
  resample->SetTransform(transform);
  resample->SetInterpolator(interp);
  resample->SetOutputStartIndex(index);
  resample->SetOutputOrigin( inputImage->GetOrigin() );
  resample->SetOutputSpacing( inputImage->GetSpacing() );
  resample->Update();
  ImageType::Pointer returnval = resample->GetOutput();
  returnval->SetDirection( inputImage->GetDirection() );
  return returnval;
}

int main(int argc, char **argv)
{
  std::string startImageName( itksys::SystemTools::CollapseFullPath(argv[1]) );
  std::string xfrmImageName( itksys::SystemTools::CollapseFullPath(argv[2]) );

  // read input image
  ImageType::Pointer startImage =
    itkUtil::ReadImageCoronal< ImageType >(startImageName);

  if ( startImage.IsNull() )
    {
    std::cerr << "Can't read test image "
              << startImageName << std::endl;
    exit(3);
    }

  AffineTransformType::Pointer transform = AffineTransformType::New();

  ImageType::SpacingType spacing = startImage->GetSpacing();
  ImageType::PointType   origin = startImage->GetOrigin();
  ImageType::SizeType    size = startImage->GetLargestPossibleRegion().GetSize();

  // do rotation around image center;
  double imageCenter[3];
  imageCenter[0] = -1 * ( origin[0] + spacing[0] * size[0] / 2.0 );
  imageCenter[1] = -1 * ( origin[1] + spacing[1] * size[1] / 2.0 );
  imageCenter[2] = -1 * ( origin[2] + spacing[1] * size[2] / 2.0 );
  transform->Translate(imageCenter);

  AffineTransformType::OutputVectorType scale;
  scale[0] = 1.2;
  scale[1] = 1.3;
  scale[2] = 1.15;
  transform->Scale(scale);

  AffineTransformType::OutputVectorType rotationAxis;
  rotationAxis[0] = 0.0; rotationAxis[1] = 0.0; rotationAxis[2] = 1.0;
  transform->Rotate3D( rotationAxis, DEGREES(6.0) );
  rotationAxis[0] = 1.0;                        rotationAxis[2] = 0.0;
  transform->Rotate3D( rotationAxis, DEGREES(-5.0) );
  rotationAxis[0] = 0.0; rotationAxis[1] = 1.0;
  transform->Rotate3D( rotationAxis, DEGREES(4.0) );

  AffineTransformType::OutputVectorType offset;
  offset[0] = 4.0;
  offset[1] = -3.0;
  offset[2] = 2.0;
  transform->Translate(offset);

  imageCenter[0] *= -1;
  imageCenter[1] *= -1;
  imageCenter[2] *= -1;
  transform->Translate(imageCenter);

  ImageType::Pointer xfrmImage = Resample(startImage, transform);
  itkUtil::WriteImage< ImageType >(xfrmImage, xfrmImageName);
  exit(0);
}
