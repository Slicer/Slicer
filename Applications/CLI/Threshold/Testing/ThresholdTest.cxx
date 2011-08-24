
#include "itkTestMain.h"

#ifdef WIN32
#define MODULE_IMPORT __declspec(dllimport)
#else
#define MODULE_IMPORT
#endif

// This will be linked against the ModuleEntryPoint in ThresholdLib
extern "C" MODULE_IMPORT int ModuleEntryPoint(int, char * []);

void RegisterTests()
{
  StringToTestFunctionMap["ModuleEntryPoint"] = ModuleEntryPoint;
}
