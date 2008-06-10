#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkVector.h"
#include "itkResampleImageFilter.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkAffineTransform.h"

int main(int argc, char* argv[])

{

  if( argc < 4 )
    {
      std::cerr << "Missing Parameters " << std::endl;
      std::cerr << "Usage: " << argv[0];
      std::cerr << " <Moving Image in metafile format> ";
      std::cerr << " <Transformation Matrix in trsf file format of baladin> ";
      std::cerr << " <Output Image in metafile format> ";
      std::cerr << " [Output Image size: %d %d %d] ";
      std::cerr << " [Output Image spacing: %f %f %f] " << std::endl;
      return 1;
    }
  
  const unsigned int Dimension=3;

  // reading the moving image and creating the writer.
  typedef itk::Image< float, Dimension > MovingImageType;
  typedef itk::ImageFileReader< MovingImageType > mImageReaderType;
  typedef itk::ImageFileWriter< MovingImageType > oImageWriterType;
  mImageReaderType::Pointer imageReader = mImageReaderType::New();
  oImageWriterType::Pointer imageWriter = oImageWriterType::New();
  imageReader->SetFileName( argv[1] );
  imageWriter->SetFileName( argv[3] );
  
  // Creating the filter.
  typedef itk::ResampleImageFilter< 
  MovingImageType, MovingImageType > FilterType;
  FilterType::Pointer filter = FilterType::New();
  typedef itk::LinearInterpolateImageFunction< 
  MovingImageType, double >  InterpolatorType;
  InterpolatorType::Pointer interpolator = InterpolatorType::New();
  filter->SetInterpolator( interpolator );
  filter->SetDefaultPixelValue( 0 );
  
  imageReader->Update();
  MovingImageType::SpacingType spacing;
  MovingImageType::SizeType size;
  MovingImageType::PointType origin;
  if(argc == 4){

    spacing = imageReader->GetOutput()->GetSpacing();
    size = imageReader->GetOutput()->GetLargestPossibleRegion().GetSize();
  }else{
    
    size[0] = atoi( argv[4] );
    size[1] = atoi( argv[5] );
    size[2] = atoi( argv[6] );
    spacing[0] = atof( argv[7] );
    spacing[1] = atof( argv[8] );
    spacing[2] = atof( argv[9] );
  }

  origin[0] = -(int)round(size[0]/2);
  origin[1] = -(int)round(size[1]/2);
  origin[2] = -(int)round(size[2]/2);
  std::cout << origin << std::endl;
  filter->SetOutputOrigin( origin );
  filter->SetOutputSpacing( spacing );
  filter->SetSize( size );
  filter->SetInput( imageReader->GetOutput() );
  imageWriter->SetInput( filter->GetOutput() );
  
  typedef itk::AffineTransform< 
  double,Dimension > TransformType;
  TransformType::Pointer transform = TransformType::New();
  
  TransformType::MatrixType rotMatrix;
  TransformType::OutputVectorType transVec;
  FILE* fid;
  fid = fopen( argv[2],"r" );
  char ahmet;
  double value;
  fscanf(fid,"%c\n",&ahmet);
  fscanf(fid,"%c",&ahmet);
  fscanf(fid,"%lf\n",&value);
  for( int j=0;j<3;j++ )
    {
      for( int k=0;k<3;k++ )
    {
      fscanf( fid,"%lf", &value);
      rotMatrix[j][k] = value;
    }
      fscanf( fid, "%lf\n", &value);
      transVec[j] = value+(int)round(size[j]/2);
    }

  transform->Translate( transVec );
  transform->SetMatrix( rotMatrix );

  filter->SetTransform( transform );
  
  try
    {
      imageWriter->Update();
    }
  catch( itk::ExceptionObject & excep )
    {
      std::cerr << "Exception caught !" << std::endl;
      std::cerr << excep << std::endl;
    }


  return(0);
}
