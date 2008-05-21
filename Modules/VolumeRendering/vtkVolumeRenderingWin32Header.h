#ifndef __vtkVolumeRenderingWin32Header_h
        #define __vtkVolumeRenderingWin32Header_h

        #include <vtkVolumeRenderingConfigure.h>

        #if defined(WIN32) && !defined(VTKSLICER_STATIC)
                #if defined(VolumeRendering_EXPORTS)
                        #define VTK_VOLUMERENDERING_EXPORT __declspec( dllexport ) 
                #else
                        #define VTK_VOLUMERENDERING_EXPORT __declspec( dllimport ) 
                #endif
        #else
                #define VTK_VOLUMERENDERING_EXPORT 
        #endif
#endif
