#include "itkTestMain.h"

void RegisterTests()
{
  REGISTER_TEST(DiffusionTensorEstimationTest);
}

#undef main
#define main DiffusionTensorEstimationTest

#include "../DiffusionTensorEstimation.cxx"
