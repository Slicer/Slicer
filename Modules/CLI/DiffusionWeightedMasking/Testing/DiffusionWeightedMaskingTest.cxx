#include <iostream>
#include "itkTestMain.h"

void RegisterTests()
{
  REGISTER_TEST(DiffusionWeightedMaskingTest);
}

#undef main
#define main DiffusionWeightedMaskingTest
#include "../DiffusionWeightedMasking.cxx"
