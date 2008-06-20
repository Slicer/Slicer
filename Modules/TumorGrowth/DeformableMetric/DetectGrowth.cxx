#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkVector.h"
#include "itkImageRegionConstIterator.h"
#include "itkDeformationFieldJacobianDeterminantFilter.h"
#include "itkWarpJacobianDeterminantFilter.h"
int main(int argc, char *argv[])
{ 
  if( argc < 4 )
    {
      std::cerr << "Missing Parameters " << std::endl;
      std::cerr << "Usage: " << argv[0];
      std::cerr << " <Deformation field (inverse)> " ;
      std::cerr << " <Initial Time Segmentation> " ;
      std::cerr << " <Output file name>" << std::endl;
      return 1;
    }

  const unsigned int Dimension=3;

  // READING IMAGES
  typedef double SegImPixelType;
  typedef itk::Vector< double,Dimension> VectorPixelType;
  typedef itk::Image< VectorPixelType,Dimension > DefImageType;
  typedef itk::Image< SegImPixelType,Dimension > SegImageType;
  typedef itk::ImageFileReader< DefImageType > DefImageReaderType;
  typedef itk::ImageFileReader< SegImageType > SegImageReaderType;
  typedef itk::ImageRegionConstIterator< SegImageType > SegImageIteratorType;
  
  DefImageReaderType::Pointer defImageReader = DefImageReaderType::New();
  SegImageReaderType::Pointer segImageReader = SegImageReaderType::New();
  defImageReader->SetFileName( argv[1] );
  segImageReader->SetFileName( argv[2] );
  DefImageType::Pointer defImage = defImageReader->GetOutput();
  SegImageType::Pointer segImage = segImageReader->GetOutput();
  
  segImageReader->Update();
  defImageReader->Update();

  
  // COMPUTING JACOBIAN DETERMINANT
  typedef itk::Image< double,Dimension> JacImageType;
  //typedef itk::DeformationFieldJacobianDeterminantFilter< DefImageType,
  //  double, JacImageType > JacFilterType;
  typedef itk::WarpJacobianDeterminantFilter< DefImageType, JacImageType> 
    JacFilterType;
  JacFilterType::Pointer jacFilter = JacFilterType::New();
  jacFilter->SetInput( defImage );
  jacFilter->SetUseImageSpacingOn();
  JacImageType::Pointer jacImage = jacFilter->GetOutput();
  jacFilter->Update();
  
  typedef itk::ImageFileWriter< JacImageType > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName("deneme-jacobian-det.mha");
  writer->SetInput( jacFilter->GetOutput() );
  writer->Update();

  typedef itk::ImageRegionConstIterator< JacImageType > JacImageIteratorType;
  JacImageIteratorType ji( jacImage, jacImage->GetRequestedRegion() );
  SegImageIteratorType si( segImage, segImage->GetRequestedRegion() );

  double jacDetSum = 0.0;
  double numVox = 0;
  double numVoxBoundary = 0;
  double jacDetSumBoundary = 0.0;
  bool temp = false;
  
  SegImageType::SpacingType spacing = segImage->GetSpacing();  
  SegImageType::RegionType region = segImage->GetLargestPossibleRegion();
  SegImageType::SizeType size = region.GetSize();
  SegImageType::IndexType index;

  std::cout << spacing << std::endl;
  
  for( ji.GoToBegin(), si.GoToBegin(); !ji.IsAtEnd(); ++ji, ++si )
    {
      index = si.GetIndex();
      if( (index[2] > 1) && (index[2] < size[2] - 2) )
    {// I'm getting rid of the first and last two slices because
      // the demons registration algorithm doesn't handle boundaries
      // correctly.
      if( si.Get() > 0 )
        {
          numVox++;
          jacDetSum = jacDetSum + ji.Get();
        }
    }
     
    }
  
  std::cout << "!!Growth in mm^3: " << (jacDetSum)*spacing[0]*spacing[1]*spacing[2] << std::endl;
  std::cout << "Growth in mm^3: " << (jacDetSum - numVox)*spacing[0]*spacing[1]*spacing[2] << std::endl;
  std::cout << "Growth in # of voxels: " << (jacDetSum - numVox) << std::endl;
  
  FILE* fp = fopen( argv[3],"w" );
  fprintf( fp, "%lf  %lf\n", (jacDetSum - numVox)*spacing[0]*spacing[1]*spacing[2],
       (jacDetSum - numVox) );
  fclose(fp);

  return(0);
}
