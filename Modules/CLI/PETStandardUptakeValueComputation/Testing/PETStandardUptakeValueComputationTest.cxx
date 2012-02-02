#include "itkTestMain.h"

void RegisterTests()
{
  REGISTER_TEST(PETStandardUptakeValueComputationTest);
}

#undef main
#define main PETStandardUptakeValueComputationTest

#include "../PETStandardUptakeValueComputation.cxx"
