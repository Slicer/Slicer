#include "itkTestMain.h"

void RegisterTests()
{
  REGISTER_TEST(DiffusionTensorScalarMeasurementsTest);
}

#undef main
#define main DiffusionTensorScalarMeasurementsTest

#include "../DiffusionTensorScalarMeasurements.cxx"
