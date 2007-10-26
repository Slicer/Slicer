#ifndef __vtkVolumeRenderingModuleWin32Header_h
        #define __vtkVolumeRenderingModuleWin32Header_h

        #include <vtkVolumeRenderingModuleConfigure.h>

        #if defined(WIN32) && !defined(VTKSLICER_STATIC)
                #if defined(VolumeRenderingModule_EXPORTS)
                        #define VTK_VOLUMERENDERINGMODULE_EXPORT __declspec( dllexport ) 
                #else
                        #define VTK_VOLUMERENDERINGMODULE_EXPORT __declspec( dllimport ) 
                #endif
        #else
                #define VTK_VOLUMERENDERINGMODULE_EXPORT 
        #endif
#endif
