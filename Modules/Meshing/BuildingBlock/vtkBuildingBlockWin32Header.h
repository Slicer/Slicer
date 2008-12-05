#ifndef __vtkBuildingBlockWin32Header_h
        #define __vtkBuildingBlockWin32Header_h

        #include <vtkBuildingBlockConfigure.h>

        #if defined(WIN32) && !defined(VTKSLICER_STATIC)
                #if defined(BuildingBlock_EXPORTS)
                        #define VTK_BUILDINGBLOCK_EXPORT __declspec( dllexport ) 
                #else
                        #define VTK_BUILDINGBLOCK_EXPORT __declspec( dllimport ) 
                #endif
        #else
                #define VTK_BUILDINGBLOCK_EXPORT 
        #endif
#endif
