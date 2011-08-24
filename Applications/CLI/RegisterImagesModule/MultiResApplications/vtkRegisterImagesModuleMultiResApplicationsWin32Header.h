#ifndef __vtkRegisterImagesModuleMultiResApplicationsWin32Header_h
#define __vtkRegisterImagesModuleMultiResApplicationsWin32Header_h

#include <vtkRegisterImagesModuleMultiResApplicationsConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(RegisterImagesMultiResModule_EXPORTS)
#define VTK_REGISTERIMAGESMODULEMULTIRESAPPLICATIONS_EXPORT __declspec( dllexport )
#else
#define VTK_REGISTERIMAGESMODULEMULTIRESAPPLICATIONS_EXPORT __declspec( dllimport )
#endif
#else
#define VTK_REGISTERIMAGESMODULEMULTIRESAPPLICATIONS_EXPORT
#endif
#endif
