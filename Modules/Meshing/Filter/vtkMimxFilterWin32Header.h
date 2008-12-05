#ifndef __vtkMimxFilterWin32Header_h
        #define __vtkMimxFilterWin32Header_h

        #include <vtkMimxFilterConfigure.h>

        #if defined(WIN32) && !defined(VTKSLICER_STATIC)
                #if defined(mimxFilter_EXPORTS)
                        #define VTK_MIMXFILTER_EXPORT __declspec( dllexport ) 
                #else
                        #define VTK_MIMXFILTER_EXPORT __declspec( dllimport ) 
                #endif
        #else
                #define VTK_MIMXFILTER_EXPORT 
        #endif
#endif
