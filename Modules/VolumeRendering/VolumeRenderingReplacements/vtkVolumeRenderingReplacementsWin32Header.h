#ifndef __vtkVolumeRenderingReplacementsWin32Header_h
        #define __vtkVolumeRenderingReplacementsWin32Header_h

        #include <vtkVolumeRenderingReplacementsConfigure.h>

        #if defined(WIN32) && !defined(VTKSLICER_STATIC)
                #if defined(VolumeRenderingReplacements_EXPORTS)
                        #define VTK_VOLUMERENDERINGREPLACEMENTS_EXPORT __declspec( dllexport ) 
                #else
                        #define VTK_VOLUMERENDERINGREPLACEMENTS_EXPORT __declspec( dllimport ) 
                #endif
        #else
                #define VTK_VOLUMERENDERINGREPLACEMENTS_EXPORT 
        #endif
#endif
