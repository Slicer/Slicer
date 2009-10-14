//
// use an ifdef on Slicer3_VTK5 to flag code that won't
// compile on vtk4.4 and before
//
#if ( (VTK_MAJOR_VERSION >= 5) || ( VTK_MAJOR_VERSION == 4 && VTK_MINOR_VERSION >= 5 ) )
#define Slicer3_VTK5
#endif

#include "vtkPETCTFusionWin32Header.h"
