/*=========================================================================

  Adapted from AddTest.cxx available in Slicer3: www.slicer.org
  "All or portions of this licensed product (such portions are the "Software") have been obtained under license from The Brigham and Women's Hospital, Inc. and are subject to the following terms and conditions:"
    See License.txt or http://www.slicer.org/copyright/copyright.txt for details.


=========================================================================*/
#include "itkTestMainExtended.h"

#ifdef WIN32
#define MODULE_IMPORT __declspec(dllimport)
#else
#define MODULE_IMPORT
#endif

extern "C" MODULE_IMPORT int ModuleEntryPoint(int, char * []);

void RegisterTests()
{
  StringToTestFunctionMap["ModuleEntryPoint"] = ModuleEntryPoint;
}
