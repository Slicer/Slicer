#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif

#include <iostream>
#include "itkTestMain.h" 


void RegisterTests()
{
REGISTER_TEST(GaussianBlurImageFilterTest);
}

#undef main
#define main GaussianBlurImageFilterTest
#include "GaussianBlurImageFilter.cxx"
