#ifndef __vtkVolumeRenderingWin32Header_h
        #define __vtkVolumeRenderingWin32Header_h

        #include <VolumeRenderingModuleConfigure.h>

        #if defined(WIN32) && !defined(VTKSLICER_STATIC)
                #if defined(VolumeRenderingModule_EXPORTS)
                        #define VTK_VRMODULE_EXPORT __declspec( dllexport ) 
                #else
                        #define VTK_VRMODULE_EXPORT __declspec( dllimport ) 
                #endif
        #else
                #define VTK_VRMODULE_EXPORT 
        #endif
#endif
