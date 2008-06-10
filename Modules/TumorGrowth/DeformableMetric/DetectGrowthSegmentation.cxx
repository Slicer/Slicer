#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkVector.h"
#include "itkImageRegionConstIterator.h"

int main(int argc, char *argv[])
{ 
  if( argc < 4 )
    {
      std::cerr << "Missing Parameters " << std::endl;
      std::cerr << "Usage: " << argv[0];
      std::cerr << " Initial Segmentation ";
      std::cerr << " Final Segmentation ";
      std::cerr << " Name of the output file" << std::endl;
      return 1;
    }

  const unsigned int Dimension=3;
  typedef double SegImPixelType;  
  typedef itk::Image< SegImPixelType,Dimension > SegImageType;
  typedef itk::ImageFileReader< SegImageType > SegImageReaderType;
  typedef itk::ImageRegionConstIterator< SegImageType > SegImageIteratorType;
  
  
  

  SegImageReaderType::Pointer segImageReader1 = SegImageReaderType::New();
  SegImageReaderType::Pointer segImageReader2 = SegImageReaderType::New();

  segImageReader1->SetFileName( argv[1] );
  segImageReader2->SetFileName( argv[2] );

  SegImageType::Pointer segImage1 = segImageReader1->GetOutput();
  SegImageType::Pointer segImage2 = segImageReader2->GetOutput();
  
  segImageReader1->Update();
  segImageReader2->Update();

  SegImageIteratorType si1( segImage1, segImage1->GetRequestedRegion() );
  SegImageIteratorType si2( segImage2, segImage2->GetRequestedRegion() );

  double numVox1 = 0;
  double numVox2 = 0;
  double numVox2_threshold=0;
  double numVox2_threshold2=0;
  double sumVox1 = 0;
  double sumVox2 = 0;
  
  SegImageType::SpacingType spacing = segImageReader1->GetOutput()->GetSpacing();
  SegImageType::SizeType size = segImageReader1->GetOutput()->GetLargestPossibleRegion().GetSize();
  SegImageType::IndexType index;
  
  double max_value1 = 1.0;
  double max_value2 = 1.0;
  for( si1.GoToBegin(), si2.GoToBegin(); !si1.IsAtEnd(); ++si1, ++si2)
    {
      index = si1.GetIndex();
      if( (index[2] > 1) && (index[2] < size[2] - 2) )
    {// I'm getting rid of the first and last two slices because
      // the demons registration algorithm doesn't handle boundaries
      // correctly.
      if( si1.Get() > max_value1 )
        max_value1 = si1.Get();
      if( si2.Get() > max_value2 )
        max_value2 = si2.Get();
    }
    }

  for( si1.GoToBegin(), si2.GoToBegin(); !si1.IsAtEnd(); ++si1, ++si2 )
    {
      index = si1.GetIndex();
      if( (index[2] > 1) && (index[2] < size[2] - 2) )
    {// I'm getting rid of the first and last two slices because
      // the demons registration algorithm doesn't handle boundaries
      // correctly.
      if( si1.Get() > 0 )
        numVox1++;
      /*if( si1.Get() > 0.9 )
        sumVox1 = sumVox1 + si1.Get();*/
      if( si1.Get() > 0 )
        sumVox1 = sumVox1 + si1.Get()/max_value1;
      if( si2.Get() > 0 )
        numVox2++;
      if( si2.Get() > 0.1 )
        numVox2_threshold++;
      if( si2.Get() > 0.5 )
        numVox2_threshold2++;
      if( si2.Get() > 0 )
        sumVox2 = sumVox2 + si2.Get()/max_value2;
    }
    }
    
  std::cout << "Number of Voxels at seg 1: " << numVox1 << std::endl;
  std::cout << "Number of Voxels at seg 2: " << numVox2 << std::endl;
  std::cout << "Number of Voxels at seg 2 thresholded: " 
        << numVox2_threshold << std::endl;
  std::cout << "Sum of Voxels at seg 1: " << sumVox1 << std::endl;
  std::cout << "Sum of Voxels at seg 2: " << sumVox2 << std::endl;
  
  double perGrowth = (numVox2 - numVox1)/(numVox1)*100;
  std::cout << "Percentage Growth: " << perGrowth << std::endl;
  double perSumGrowth = (sumVox2 - sumVox1)/sumVox1*100;
  std::cout << "Percentage Growth in Sum: " << perSumGrowth << std::endl;
  double perThGrowth = (numVox2_threshold - numVox1)/(numVox1)*100;
  std::cout << "Percentage Growth when thresholded: " << perThGrowth 
        << std::endl;
  double perThGrowth2 = (numVox2_threshold2 - numVox1)/(numVox1)*100;
  std::cout << "Percentage Growth when thresholded: " << perThGrowth2 
        << std::endl;
  
  std::cout << spacing << std::endl;
  std::cout << "Difference in sum (partial volume): " << (sumVox2 - sumVox1) << std::endl;
  std::cout << "Difference in voxel: " << (numVox2 - numVox1) << std::endl;

  FILE* fp = fopen( argv[3],"a" );
  /*fprintf( fp, "%lf\t %lf\t %lf\t %lf\t %lf\t %lf\n", perGrowth, perSumGrowth, 
    perThGrowth, perThGrowth2, (sumVox2 - sumVox1)*spacing[0]*spacing[1]*spacing[2],(numVox2 - numVox1)*spacing[0]*spacing[1]*spacing[2]);*/
  fprintf( fp, "%f\t%f\n",(sumVox2 - sumVox1)*spacing[0]*spacing[1]*spacing[2], (sumVox2 - sumVox1)  );
  fclose(fp);

  return(0);
}
