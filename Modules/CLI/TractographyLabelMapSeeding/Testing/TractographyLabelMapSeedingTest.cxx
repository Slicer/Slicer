
#include "itkTestMain.h"

void RegisterTests()
{
  REGISTER_TEST(TractographyLabelMapSeedingTest);
}

#undef main
#define main TractographyLabelMapSeedingTest

#include "../TractographyLabelMapSeeding.cxx"
