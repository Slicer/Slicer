
// ITK includes
#include <itkAffineTransform.h>
#include <itkBSplineDeformableTransform.h>
#include <itkEuler3DTransform.h>
#include <itkImageFileWriter.h>
#include <itkImage.h>
#include <itkResampleImageFilter.h>
#include <itkTransformFactory.h>
#include <itkTransformFileReader.h>
#include <itkFactoryRegistration.h>

// STD includes
#include <list>

#ifndef M_PI
#define M_PI 3.1415926
#endif

int main(int argc, char * * argv)
{
  itk::itkFactoryRegistration();

  if( argc < 3 )
    {
    std::cerr << argv[0]
              << " <freq> <outFile>"
              << " [-t transform]"
              << " [-p positionX positionY positionZ]"
              << " [-o orientationX orientationY orientationZ]"
              << std::endl;
    return EXIT_FAILURE;
    }

  typedef itk::Image<float, 3> ImageType;

  ImageType::Pointer  img = ImageType::New();
  ImageType::SizeType size;
  size[0] = 32;
  size[1] = 64;
  size[2] = 32;
  img->SetRegions( size );
  img->Allocate();

  double               scaleMax = M_PI / atof(argv[1]);
  double               scaleMin = M_PI / (0.5 * atof(argv[1]) );
  double               scale;
  double               v, vTot;
  int                  i, j, k;
  ImageType::IndexType indx;
  for( indx[2] = 0; indx[2] < (int)size[2]; indx[2]++ )
    {
    for( indx[1] = 0; indx[1] < (int)size[1]; indx[1]++ )
      {
      for( indx[0] = 0; indx[0] < (int)size[0]; indx[0]++ )
        {
        scale =
          (scaleMax
           - scaleMin)
          * (indx[0] / size[0] + indx[1] / (size[1] / 10.0) + indx[2] / 1.0) / (size[2] + size[1] / 10 + 1) + scaleMin;
        v = indx[0] * scale;
        i = (int)(v / M_PI);
        vTot = sin( v - ( (int)(v / M_PI) * M_PI) );
        v = indx[1] * scale;
        j = (int)(v / M_PI);
        vTot += sin( v - ( (int)(v / M_PI) * M_PI) );
        v = indx[2] * scale;
        k = (int)(v / M_PI);
        vTot += sin( v - ( (int)(v / M_PI) * M_PI) );
        if( (i + j + k) % 2 == 0 )
          {
          if( (i + j + k) % 4 == 0 )
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
          if( (indx[0] + indx[1] + indx[2]) % 2 == 0 )
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

  int argNum = 2;
  while( argNum < argc - 1 )
    {
    if( argv[++argNum][0] != '-' )
      {
      std::cout << "Unknown arg = " << argv[argNum] << std::endl;
      return 0;
      }
    else
      {
      switch( argv[argNum][1] )
        {
        case 't':
          {
          typedef itk::BSplineDeformableTransform<double, 3, 3> BSplineTransformType;

          typedef itk::TransformFileReader TransformReaderType;
          TransformReaderType::Pointer transformReader = TransformReaderType::New();
          transformReader->SetFileName( argv[++argNum] );
          itk::TransformFactory<BSplineTransformType>::RegisterTransform();
          transformReader->Update();

          typedef TransformReaderType::TransformListType TransformListType;
          TransformListType *         transformList = transformReader->GetTransformList();
          TransformListType::iterator transformListIt = transformList->begin();

          typedef itk::ResampleImageFilter<ImageType, ImageType> ResamplerType;
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
              transform = static_cast<itk::AffineTransform<double, 3> *>( transformListIt->GetPointer() );
              resampler->SetTransform( transform );
              found = true;
              }
            else if( (*transformListIt)->GetTransformTypeAsString() == "BSplineDeformableTransform_double_3_3" )
              {
              std::cout << "Apply an bspline transform..." << std::endl;
              BSplineTransformType::Pointer transform;
              transform = static_cast<BSplineTransformType *>( transformListIt->GetPointer() );
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
              try
                {
                resampler->Update();
                }
              catch( ... )
                {
                std::cerr << "tfm = " << resampler->GetTransform() << std::endl;
                std::cerr << "Error during resampling using transform"
                          << std::endl;
                }
              img = resampler->GetOutput();
              }
            ++transformListIt;
            }

          break;
          }
        case 'p':
          {
          ImageType::PointType pnt;
          pnt[0] = atof( argv[++argNum] );
          pnt[1] = atof( argv[++argNum] );
          pnt[2] = atof( argv[++argNum] );
          img->SetOrigin(pnt);
          break;
          }
        case 'o':
          {
          typedef itk::Euler3DTransform<double> TransformType;
          TransformType::Pointer tfm = TransformType::New();
          double                 rotX = atof( argv[++argNum] );
          double                 rotY = atof( argv[++argNum] );
          double                 rotZ = atof( argv[++argNum] );
          tfm->SetRotation( rotX, rotY, rotZ );
          img->SetDirection( tfm->GetMatrix() );
          break;
          }
        }
      }
    }

  typedef itk::ImageFileWriter<ImageType> WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetInput( img );
  writer->SetFileName( argv[2] );
  try
    {
    writer->Update();
    }
  catch( itk::ExceptionObject & error )
    {
    std::cerr << "Error: " << error << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
