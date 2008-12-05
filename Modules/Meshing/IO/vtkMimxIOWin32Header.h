#ifndef __vtkMimxIOWin32Header_h
        #define __vtkMimxIOWin32Header_h

        #include <vtkMimxIOConfigure.h>

        #if defined(WIN32) && !defined(VTKSLICER_STATIC)
                #if defined(mimxMeshIO_EXPORTS)
                        #define VTK_MIMXIO_EXPORT __declspec( dllexport ) 
                #else
                        #define VTK_MIMXIO_EXPORT __declspec( dllimport ) 
                #endif
        #else
                #define VTK_MIMXIO_EXPORT 
        #endif
#endif
