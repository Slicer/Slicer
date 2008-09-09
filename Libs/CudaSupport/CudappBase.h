#ifndef CUDAPPBASE_H_
#define CUDAPPBASE_H_

#include <ostream>
#include "driver_types.h"
#include "CudappSupportModule.h"
namespace Cudapp
{
    /// THIS IS A STATIC CLASS USED FOR BASIC CUDA FUNCTIONALITY!!
    class CUDA_SUPPORT_EXPORT Base
    {
    public:
        typedef enum {
            Success,
            NotReadyError,
            InvalidValueError
        } State;

        static cudaError_t GetLastError();
        static const char* GetLastErrorString();
        static const char* GetErrorString(cudaError_t error);
        static void PrintError(cudaError_t error);

    protected:
        virtual ~Base();
        Base();
        //Base(const Base&);
    };
}
#endif /*CUDAPPBASE_H_*/
