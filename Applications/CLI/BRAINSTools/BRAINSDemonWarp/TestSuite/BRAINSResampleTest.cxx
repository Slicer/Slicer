#if defined( _MSC_VER )
#pragma warning ( disable : 4786 )
#endif
#include <iostream>
#include "itkTestMain.h"

void RegisterTests()
{
  REGISTER_TEST(BRAINSResampleTest);
}

#undef main
#define main BRAINSResampleTest
#include "../BRAINSResample.cxx"
