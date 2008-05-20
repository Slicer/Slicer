/* 
 * Here is where system computed values get stored.
 * These values should only change when the target compile platform changes.
 */

#include "vtkSlicerConfigure.h"

#define BUILD_SHARED_LIBS
#ifndef BUILD_SHARED_LIBS
#define VTKSLICER_STATIC
#endif
