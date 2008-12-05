#ifndef __vtkMimxWidgetsWin32Header_h
        #define __vtkMimxWidgetsWin32Header_h

        #include <vtkMimxWidgetsConfigure.h>

        #if defined(WIN32) && !defined(VTKSLICER_STATIC)
                #if defined(mimxWidgets_EXPORTS)
                        #define VTK_MIMXWIDGETS_EXPORT __declspec( dllexport ) 
                #else
                        #define VTK_MIMXWIDGETS_EXPORT __declspec( dllimport ) 
                #endif
        #else
                #define VTK_MIMXWIDGETS_EXPORT 
        #endif
#endif
