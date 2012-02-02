#include <iostream>
#include "itkTestMain.h"

void RegisterTests()
{
  REGISTER_TEST(DiffusionWeightedVolumeMaskingTest);
}

#undef main
#define main DiffusionWeightedVolumeMaskingTest
#include "../DiffusionWeightedVolumeMasking.cxx"
