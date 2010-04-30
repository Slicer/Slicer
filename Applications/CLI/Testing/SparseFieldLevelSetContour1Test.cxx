#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif

#ifdef WIN32
#define MODULE_IMPORT __declspec(dllimport)
#else
#define MODULE_IMPORT
#endif

#include <iostream>
#include "itkTestMain.h"

void RegisterTests()
{
REGISTER_TEST(SparseFieldLevelSetContour1Test);
}

#undef main
#define main SparseFieldLevelSetContour1Test
#include "SparseFieldLevelSetContour/SparseFieldLevelSetContour.cxx"


