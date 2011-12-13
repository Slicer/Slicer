#include "itkTestMain.h"

void RegisterTests()
{
  REGISTER_TEST(ComputeSUVBodyWeightTest);
}

#undef main
#define main ComputeSUVBodyWeightTest

#include "../ComputeSUVBodyWeight.cxx"

