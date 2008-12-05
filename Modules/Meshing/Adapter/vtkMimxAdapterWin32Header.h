#ifndef __vtkMimxAdapterWin32Header_h
        #define __vtkMimxAdapterWin32Header_h

        #include <vtkMimxAdapterConfigure.h>

        #if defined(WIN32) && !defined(VTKSLICER_STATIC)
                #if defined(mimxAdapter_EXPORTS)
                        #define VTK_MIMXADAPTER_EXPORT __declspec( dllexport ) 
                #else
                        #define VTK_MIMXADAPTER_EXPORT __declspec( dllimport ) 
                #endif
        #else
                #define VTK_MIMXADAPTER_EXPORT 
        #endif
#endif
