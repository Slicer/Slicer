#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif

#include "itkTestMain.h"

void RegisterTests()
{
  REGISTER_TEST(GrayscaleModelMakerTest);
}

#undef main
#define main GrayscaleModelMakerTest

#include "../GrayscaleModelMaker.cxx"
