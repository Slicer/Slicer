#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif

#include <iostream>
#include "itkTestMain.h"
// #include "DicomToNrrdConverter.cxx"

/*
extern "C" MODULE_IMPORT int ModuleEntryPoint(int, char* []);

void RegisterTests()
{
  StringToTestFunctionMap["ModuleEntryPoint"] = ModuleEntryPoint;
}
*/

void RegisterTests()
{
  REGISTER_TEST(DicomToNrrdConverterTest);
}

#undef main
#define main DicomToNrrdConverterTest
#include "../DicomToNrrdConverter.cxx"
