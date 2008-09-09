#ifndef __vtkVolumeRenderingCudaModuleWin32Header_h
        #define __vtkVolumeRenderingCudaModuleWin32Header_h

        #include <vtkVolumeRenderingCudaModuleConfigure.h>

        #if defined(WIN32) && !defined(VTKSLICER_STATIC)
                #if defined(VolumeRenderingCudaModule_EXPORTS)
                        #define VTK_VOLUMERENDERINGCUDAMODULE_EXPORT __declspec( dllexport ) 
                #else
                        #define VTK_VOLUMERENDERINGCUDAMODULE_EXPORT __declspec( dllimport ) 
                #endif
        #else
                #define VTK_VOLUMERENDERINGCUDAMODULE_EXPORT 
        #endif
#endif
