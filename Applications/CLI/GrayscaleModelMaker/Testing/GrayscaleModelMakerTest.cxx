#include "itkTestMain.h"

void RegisterTests()
{
  REGISTER_TEST(GrayscaleModelMakerTest);
}

#undef main
#define main GrayscaleModelMakerTest

#include "../GrayscaleModelMaker.cxx"
