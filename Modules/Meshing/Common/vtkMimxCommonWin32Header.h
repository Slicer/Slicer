#ifndef __vtkMimxCommonWin32Header_h
        #define __vtkMimxCommonWin32Header_h

        #include <vtkMimxCommonConfigure.h>

        #if defined(WIN32) && !defined(VTKSLICER_STATIC)
                #if defined(mimxCommon_EXPORTS)
                        #define VTK_MIMXCOMMON_EXPORT __declspec( dllexport ) 
                #else
                        #define VTK_MIMXCOMMON_EXPORT __declspec( dllimport ) 
                #endif
        #else
                #define VTK_MIMXCOMMON_EXPORT 
        #endif
#endif
