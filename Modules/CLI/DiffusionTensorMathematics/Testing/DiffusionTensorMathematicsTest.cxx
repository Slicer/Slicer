#include "itkTestMain.h"

void RegisterTests()
{
  REGISTER_TEST(DiffusionTensorMathematicsTest);
}

#undef main
#define main DiffusionTensorMathematicsTest

#include "../DiffusionTensorMathematics.cxx"
