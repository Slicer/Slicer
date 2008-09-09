#ifndef __CudappModuleWin32Header_h
        #define __CudappModuleWin32Header_h

        #include <CudappSupportConfigure.h>

        #if defined(WIN32) && !defined(VTKSLICER_STATIC)
                #if defined(CudaSupport_EXPORTS)
                        #define CUDA_SUPPORT_EXPORT __declspec( dllexport ) 
                #else
                        #define CUDA_SUPPORT_EXPORT __declspec( dllimport ) 
                #endif
        #else
                #define CUDA_SUPPORT_EXPORT 
        #endif
#endif
