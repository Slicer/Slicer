#include <itkImage.h>
#include <itkImageFileWriter.h>
#include <itkResampleImageFilter.h>
#include <itkTransformFactory.h>
#include <itkTransformFileReader.h>
#include <itkBSplineDeformableTransform.h>

#include <list>

#ifndef M_PI
#define M_PI 3.1415926
#endif

int main(int argc, char ** argv)
{
  if( argc != 3 && argc != 4 )
    {
    std::cout << "GenerateRegisterImagesTestData <freq> <outFile> [transform]" << std::endl;
    return EXIT_FAILURE;
    }

  typedef itk::Image<float, 3> ImageType;

  ImageType::Pointer img = ImageType::New();
  ImageType::SizeType size;
  size[0] = 176;
  size[1] = 256;
  size[2] = 176;
  img->SetRegions( size );
  img->Allocate();

  double scaleMax = M_PI / atof(argv[1]);
  double scaleMin = M_PI / (0.5 * atof(argv[1]));
  double scale;
  double v, vTot;
  int i, j, k;
  ImageType::IndexType indx;
  for(indx[2]=0; indx[2]<(int)size[2]; indx[2]++)
    {
    for(indx[1]=0; indx[1]<(int)size[1]; indx[1]++)
      {
      for(indx[0]=0; indx[0]<(int)size[0]; indx[0]++)
        {
        scale = (scaleMax-scaleMin) * (indx[0]/size[0] + indx[1]/(size[1] / 10.0) + indx[2]/1.0) / (size[2]+size[1]/10+1) + scaleMin;
        v = indx[0] * scale;
        i = (int)(v / M_PI);
        vTot = sin( v - ((int)(v / M_PI) * M_PI) );
        v = indx[1] * scale;
        j = (int)(v / M_PI);
        vTot += sin( v - ((int)(v / M_PI) * M_PI) );
        v = indx[2] * scale;
        k = (int)(v / M_PI);
        vTot += sin( v - ((int)(v / M_PI) * M_PI) );
        if( (i+j+k)%2 == 0 )
          {
          if( (i+j+k)%4 == 0 )
            {
            img->SetPixel(indx, 100 - vTot * 33.33333);
            }
          else
            {
            img->SetPixel(indx, vTot * 33.3333);
            }
          }
        else
          {
          if( (indx[0]+indx[1]+indx[2])%2 == 0 )
            {
            img->SetPixel(indx, 100);
            }
          else
            {
            img->SetPixel(indx, 0);
            }

          }
        }
      }
    }

  if( argc == 4 )
    {
    typedef itk::BSplineDeformableTransform< double, 3, 3 > BSplineTransformType;

    typedef itk::TransformFileReader  TransformReaderType;
    TransformReaderType::Pointer transformReader = TransformReaderType::New();
    transformReader->SetFileName( argv[3] );
    itk::TransformFactory< BSplineTransformType >::RegisterTransform();
    transformReader->Update();
    
    typedef TransformReaderType::TransformListType TransformListType;
    TransformListType * transformList = transformReader->GetTransformList();
    TransformListType::iterator transformListIt = transformList->begin();

    typedef itk::ResampleImageFilter< ImageType, ImageType > ResamplerType;
    while( transformListIt != transformList->end() )
      {
      ResamplerType::Pointer resampler = ResamplerType::New();
      resampler->SetInput( img );
      bool found = false;
      if( (*transformListIt)->GetTransformTypeAsString() == "AffineTransform_double_3_3" )
        {
        std::cout << "Apply an affine transform..." << std::endl;
        typedef itk::AffineTransform<double, 3> TransformType;
        TransformType::Pointer transform;
        transform = static_cast< itk::AffineTransform<double, 3> * >( transformListIt->GetPointer() );
        resampler->SetTransform( transform );
        found = true;
        }
      else if( (*transformListIt)->GetTransformTypeAsString() == "BSplineDeformableTransform_double_3_3" )
        {
        std::cout << "Apply an bspline transform..." << std::endl;
        BSplineTransformType::Pointer transform;
        transform = static_cast< BSplineTransformType * >( transformListIt->GetPointer() );
        resampler->SetTransform( transform );
        found = true;
        }
      else
        {
        std::cerr << "Transform type not supported." << std::endl;
        }
      if( found )
        {
        resampler->SetDefaultPixelValue( 0 );
        resampler->SetOutputParametersFromImage( img );
        resampler->Update();
        img = resampler->GetOutput();
        }
      ++transformListIt;
      }
    }

  typedef itk::ImageFileWriter< ImageType > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetInput( img );
  writer->SetFileName( argv[2] );
  writer->Update();

  return EXIT_SUCCESS;
}
