
#include "itkTestMain.h"

void RegisterTests()
{
  REGISTER_TEST(SeedingTest);
}

#undef main
#define main SeedingTest

#include "../Seeding.cxx"
