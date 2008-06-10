#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkVector.h"
#include "itkWarpImageFilter.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkIterativeInverseDeformationFieldImageFilter.h"

int main(int argc, char* argv[])
{
  if( argc < 5 )
    {
      std::cerr << "Missing Parameters " << std::endl;
      std::cerr << "Usage: " << argv[0];
      std::cerr << " <Moving Image in metafile format> ";
      std::cerr << " <Deformation Field in metafile format> ";
      std::cerr << " <Output Image in metafile format> ";
      std::cerr << " [invert deformation field(0) OR apply deformation field (1)] ";
      std::cerr << " [use Nearest Neighbor (0) OR Linear (1) interpolation] (only important for 'apply deformation field' " << std::endl;
      return 1;
    }

  const unsigned int Dimension=3;
  typedef itk::Image< float, 3> MovingImageType;
  typedef itk::Image< itk::Vector<double,3>,3 > DeformationFieldType;
  typedef itk::ImageFileReader< MovingImageType > mImageReaderType;
  typedef itk::ImageFileReader< DeformationFieldType > dFieldReaderType;
  mImageReaderType::Pointer imageReader = mImageReaderType::New();
  dFieldReaderType::Pointer defReader = dFieldReaderType::New();
  imageReader->SetFileName(argv[1]);
  defReader->SetFileName(argv[2]);
  
  MovingImageType::Pointer movingImage = imageReader->GetOutput();
  DeformationFieldType::Pointer defField = defReader->GetOutput();
  imageReader->Update();
  defReader->Update();

  std::cout << argv[4] << std::endl;

   if( atoi(argv[4]) == 1 )
    {
      // APPLYING THE DEFORMATION FIELD.
      typedef itk::LinearInterpolateImageFunction<MovingImageType,double> LinearInterpolatorType;
      LinearInterpolatorType::Pointer linearInterpolator = NULL;
     
      typedef itk::NearestNeighborInterpolateImageFunction<MovingImageType,double> NearestInterpolatorType;
      NearestInterpolatorType::Pointer nearestInterpolator = NULL;

      typedef itk::WarpImageFilter< MovingImageType, 
      MovingImageType, DeformationFieldType > WarperType;
      WarperType::Pointer warper = WarperType::New();
      std::cout << movingImage->GetSpacing() << std::endl;
      warper->SetDeformationField( defField );
      warper->SetInput( movingImage );

      if (atoi(argv[5]) == 1 ) {
    linearInterpolator  = LinearInterpolatorType::New();
    warper->SetInterpolator( linearInterpolator );
      } else {
    nearestInterpolator  = NearestInterpolatorType::New();
    warper->SetInterpolator( nearestInterpolator );
      }

      warper->SetOutputSpacing( movingImage->GetSpacing() );
      warper->SetOutputOrigin( movingImage->GetOrigin() );
      
      
      typedef itk::ImageFileWriter< MovingImageType > WriterType;
      WriterType::Pointer writer = WriterType::New();
      writer->SetFileName( argv[3] );
      writer->SetInput( warper->GetOutput() );
      writer->Update();
    }
  else
    {
      // INVERTING THE DEFORMATION FIELD.
      typedef itk::IterativeInverseDeformationFieldImageFilter<
    DeformationFieldType, DeformationFieldType > InverterType;
      InverterType::Pointer inverter = InverterType::New();
      inverter->SetInput( defField );
      typedef itk::ImageFileWriter< DeformationFieldType > WriterType;
      WriterType::Pointer writer = WriterType::New();
      writer->SetFileName( argv[3] );
      writer->SetInput( inverter->GetOutput() );
      writer->Update();
      
    }
  //outputImage->SetSpacing(imageReader->GetOutput()->GetSpacing());
  
  
  

  return(0);
}
