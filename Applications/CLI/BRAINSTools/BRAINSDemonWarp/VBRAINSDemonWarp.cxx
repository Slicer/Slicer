#include "BRAINSDemonWarpTemplates.h"

#ifdef WIN32
#define MODULE_IMPORT __declspec(dllimport)
#else
#define MODULE_IMPORT
#endif

extern "C" MODULE_IMPORT int VBRAINSDemonWarpPrimary(int, char* []);


// main function built in BRAINSDemonWarpPrimary.cxx so that testing only builds
// templates once.
int main(int argc, char *argv[])
{
  return VBRAINSDemonWarpPrimary(argc, argv);
}
