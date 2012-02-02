#include "itkTestMain.h"

void RegisterTests()
{
  REGISTER_TEST(DWIToDTIEstimationTest);
}

#undef main
#define main DWIToDTIEstimationTest

#include "../DWIToDTIEstimation.cxx"
