#include <itkIO.h>
#include <itkSphereSpatialFunction.h>
#include <itkImageRegionIterator.h>
#include <vnl/vnl_random.h>
#if !defined( _WIN32 )
#  include <unistd.h>
#else
#  include <process.h>
inline int getpid()
{
  return _getpid();
}

#endif
unsigned int imageDim(32);

int main(int, char *argv[])
{
  typedef itk::Image< unsigned char, 3 > ImageType;
  ImageType::RegionType            region;
  ImageType::RegionType::IndexType index;
  index[0] = index[1] = index[2] = 0;
  region.SetIndex(index);
  ImageType::RegionType::SizeType size;
  size[0] = imageDim;
  size[1] = imageDim;
  size[2] = imageDim;
  region.SetSize(size);
  ImageType::SpacingType spacing;
  spacing[0] =
    spacing[1] =
      spacing[2] = 1.0;
  ImageType::Pointer theImage = ImageType::New();
  theImage->SetRegions(region);
  theImage->SetSpacing(spacing);
  ImageType::Pointer theMaskImage = ImageType::New();
  theMaskImage->SetRegions(region);
  theMaskImage->SetSpacing(spacing);
  ImageType::PointType origin;
  origin[0] =
    origin[1] =
      origin[2] = 0.0;
  ImageType::DirectionType direction;
  for ( unsigned i = 0; i < 3; i++ )
    {
    for ( unsigned j = 0; j < 3; j++ )
      {
      direction[i][j] = i == j ? 1.0 : 0.0;
      }
    }
  theImage->SetDirection(direction);
  theImage->SetOrigin(origin);
  theMaskImage->SetDirection(direction);
  theMaskImage->SetOrigin(origin);

  vnl_random randgen;
  randgen.reseed( getpid() );

  //
  //  theImage->FillBuffer(0);
  //  fill buffer with low level noise
  itk::ImageRegionIterator< ImageType >
  it( theImage, theImage->GetLargestPossibleRegion() );
  for ( it.GoToBegin(); !it.IsAtEnd(); ++it )
    {
    it.Set( static_cast< ImageType::PixelType >( randgen.lrand32(16) ) );
    }
  theMaskImage->FillBuffer(0);

  typedef itk::Point< double, 3 >                    PointType;
  typedef itk::SphereSpatialFunction< 3, PointType > SphereFunctionType;
  PointType                   pt;
  SphereFunctionType::Pointer sphereFunc = SphereFunctionType::New();
  sphereFunc->SetRadius(static_cast< double >( imageDim ) / 4.0);
  pt[0] = pt[1] = pt[2] = static_cast< double >( imageDim ) / 2.0;
  sphereFunc->SetCenter(pt);

  for ( unsigned i = 0; i < imageDim; i++ )
    {
    index[0] = i;
    pt[0] = static_cast< double >( i );
    for ( unsigned j = 0; j < imageDim; j++ )
      {
      index[1] = j;
      pt[1] = static_cast< double >( j );
      for ( unsigned k = 0; k < imageDim; k++ )
        {
        pt[2] = static_cast< double >( k );
        index[2] = k;
        if ( sphereFunc->Evaluate(pt) )
          {
          theImage->SetPixel( index,
                              static_cast< ImageType::PixelType >
                              ( randgen.lrand32(32, 255) ) );
          theMaskImage->SetPixel(index, 255);
          }
        }
      }
    }
  std::string filename(argv[1]);
  itkUtil::WriteImage< ImageType >(theImage, filename);
  std::string maskFilename(argv[2]);
  itkUtil::WriteImage< ImageType >(theMaskImage, maskFilename);
  exit(0);
}
