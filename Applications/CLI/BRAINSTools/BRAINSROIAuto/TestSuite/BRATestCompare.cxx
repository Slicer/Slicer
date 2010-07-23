#include <itkImage.h>
#include <itkImageRegionIterator.h>
#include "itkIO.h"

int
main(int argc, char **argv)
{
  typedef itk::Image< char, 3 >                 ImageType;
  typedef itk::ImageRegionIterator< ImageType > IteratorType;

  if ( argc != 3 )
    {
    std::cerr << "Missing filename arguments" << std::endl;
    return -1;
    }
  std::string        file1(argv[1]);
  std::string        file2(argv[2]);
  ImageType::Pointer im1( itkUtil::ReadImage< ImageType >(file1) ),
  im2( itkUtil::ReadImage< ImageType >(file2) );
  double totpixels(0);
  double samepixels(0);
  double setPixels1(0);
  double setPixels2(0);
  for ( IteratorType it1( im1, im1->GetLargestPossibleRegion() ),
        it2( im2, im2->GetLargestPossibleRegion() );
        !it1.IsAtEnd() && !it2.IsAtEnd(); ++it1, ++it2, totpixels++ )
    {
    if ( it1.Value() != 0 )
      {
      setPixels1++;
      }
    if ( it2.Value() != 0 )
      {
      setPixels2++;
      }
    if ( it1.Value() == it2.Value() )
      {
      samepixels++;
      }
    }
  double voldiff = fabs(setPixels1 - setPixels2)
                   / ( ( setPixels1 + setPixels2 ) / 2.0 );
  double limit = samepixels / totpixels;
  std::cerr
  << ( totpixels - samepixels )
  << " out of "
  << totpixels
  << " differ."
  << std::endl
  << file1 << " volume is "
  << setPixels1 << std::endl
  << file2 << " volume is "
  << setPixels2 << std::endl
  << "Pixel match "
  << ( limit * 100 )
  << "%" << std::endl
  << "Volume difference is "
  << ( voldiff * 100 )
  << "%" << std::endl;

  if ( limit < 0.93 || voldiff > 0.04 )
    {
    return 1;
    }
  return 0;
}
