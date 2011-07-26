#include "ScalingHeuristics.h"

#include "itkCenteredTransformInitializer.h"


typedef itk::Image<short, 3> ImageType;
typedef ImageType::RegionType  RegionType;
typedef RegionType::SizeType   SizeType;
typedef RegionType::IndexType   IndexType;
typedef ImageType::SpacingType SpacingType;

namespace
{

ImageType::Pointer
createTestImage(SizeType size,
                SpacingType spacing)
{
  ImageType::Pointer image = ImageType::New();

  IndexType ind = {{0,0,0}};
  ImageType::PointType origin;
  origin[0] = 0.0;
  origin[1] = 0.0;
  origin[2] = 0.0;

  RegionType region;
  region.SetIndex(ind);
  region.SetSize(size);

  image->SetRegions(region);
  image->SetSpacing(spacing); 
  image->Allocate();
  image->SetOrigin(origin);
  
  return image;
}

  void PrintScalings(const ScalingValues & sv)
  {
    std::cout << "delta t : " << sv.TranslationScale << std::endl;
    std::cout << "delta r : " << sv.RotationScale << std::endl;
    std::cout << "delta sc: " << sv.ScalingScale << std::endl;
    std::cout << "delta sk: " << sv.SkewingScale << std::endl;
  }

}

int ScalingHeuristicsTest(int, char* [])
{
  const unsigned int NTESTS = 4;
  unsigned int sizes[NTESTS][3] = {{128,128,63},
                                   {128,128,96},
                                   {512,512,64},
                                   {64,64,64}
                                   };

  double spacings[NTESTS][3] = {{2.0,2.0,2.0},
                                {1.0,1.0,2.0},
                                {0.2734,0.2734,3.00001},
                                {1.0,1.0,1.0}
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
    
    typedef itk::MatrixOffsetTransformBase<double, 3, 3> TransformType;
    TransformType::Pointer t = TransformType::New();
    
    typedef itk::CenteredTransformInitializer<TransformType, ImageType, ImageType> TransformInitializerType;
    TransformInitializerType::Pointer initializer = TransformInitializerType::New();

    initializer->SetTransform(t);
    initializer->SetFixedImage(image);
    initializer->SetMovingImage(image);
    initializer->GeometryOn();
    initializer->InitializeTransform();

    std::cout << "spacing: " << spacing << std::endl;
    std::cout << "center: " << t->GetCenter() << std::endl;

    ScalingValues scaleh(image, t->GetCenter());    
    PrintScalings(scaleh);

    ImageType::PointType cp1 = t->GetCenter();
    cp1[0] += spacing[0];
    cp1[1] += spacing[1];
    cp1[2] += spacing[2];
    
    std::cout << "center+1: " << cp1 << std::endl;

    ScalingValues scaleh2(image, cp1);
    PrintScalings(scaleh2);
    

    }


  return EXIT_SUCCESS;
}
