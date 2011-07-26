#include "DownsampleHeuristics.h"

#include "itkRecursiveMultiResolutionPyramidImageFilter.h"


namespace
{

typedef itk::Image<short, 3> ImageType;
typedef ImageType::RegionType  RegionType;
typedef RegionType::SizeType   SizeType;
typedef RegionType::IndexType   IndexType;
typedef ImageType::SpacingType SpacingType;

ImageType::Pointer
createTestImage(SizeType size,
                SpacingType spacing)
{
  ImageType::Pointer image = ImageType::New();

  IndexType ind = {{0,0,0}};

  RegionType region;
  region.SetIndex(ind);
  region.SetSize(size);

  image->SetRegions(region);
  image->SetSpacing(spacing);
  //image->Allocate();
  
  return image;
}

}

int DownsampleHeuristicsTest(int, char* [])
{
  const unsigned int NTESTS = 7;
  unsigned int sizes[NTESTS][3] = {{128,128,63},
                                   {128,128,64},
                                   {128,128,65},
                                   {128,128,96},
                                   {256,256,64},
                                   {512,512,33},
                                   {512,512,64}
                                   };

  double spacings[NTESTS][3] = {{2.0,2.0,2.0},
                                {2.0,2.0,2.0},
                                {2.0,2.0,2.0},
                                {2.0,2.0,2.0},
                                {1.0,1.0,2.0},
                                {0.2734,0.2734,3.00001},
                                {0.2734,0.2734,3.00001}
                                };
  
  for(unsigned int i = 0; i < NTESTS; ++i)
    {
    std::cout << "===== TEST " << i  << " =====" << std::endl;
    SizeType size;
    size[0] = sizes[i][0];
    size[1] = sizes[i][1];
    size[2] = sizes[i][2];

    SpacingType spacing;
    spacing[0] = spacings[i][0];
    spacing[1] = spacings[i][1];
    spacing[2] = spacings[i][2];
    

    ImageType::Pointer image = createTestImage(size, spacing);

    std::cout << "spacing: " << image->GetSpacing() << std::endl;
    std::cout << "size: " << image->GetLargestPossibleRegion().GetSize() << std::endl;

    typedef itk::RecursiveMultiResolutionPyramidImageFilter<ImageType, ImageType> PyramidType;
    PyramidType::Pointer pyramid = PyramidType::New();
    pyramid->SetInput(image);
    scheduleImagePyramid(pyramid.GetPointer());
    
    std::cout << "schedule" << std::endl;
    std::cout << pyramid->GetSchedule() << std::endl;
    }


  return EXIT_SUCCESS;
}
