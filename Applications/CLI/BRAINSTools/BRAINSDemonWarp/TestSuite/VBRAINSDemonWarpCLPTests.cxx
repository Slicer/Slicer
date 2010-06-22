// this file defines the BRAINSDemonWarp-Tests for the test driver
// and all it expects is that you have a function called RegisterTests
#if defined( _MSC_VER )
#pragma warning ( disable : 4786 )
#endif
#include <iostream>
#include "itkTestMain.h"

void RegisterTests()
{
  REGISTER_TEST(VBRAINSDemonWarpTest);
}

#undef main
#define main VBRAINSDemonWarpTest
#include "../VBRAINSDemonWarp.cxx"
