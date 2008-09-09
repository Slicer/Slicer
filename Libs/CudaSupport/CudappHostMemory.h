#ifndef CUDAPPHOSTMEMORY_H_
#define CUDAPPHOSTMEMORY_H_

#include "CudappLocalMemory.h"
namespace Cudapp
{

    //! Cuda Host Memory is page-locked host memory that is directly accesible from the cuda device
    /**
    * This memory is known to the graphics card and can be accessed very quickly.
    * 
    * @note to much host memory in page locked mode can reduce overall system performance.
    */
    class CUDA_SUPPORT_EXPORT HostMemory : public LocalMemory
    {
    public:
        HostMemory();
        virtual ~HostMemory();
        HostMemory(const HostMemory& other);
        HostMemory& operator=(const HostMemory& other);
        virtual void* AllocateBytes(size_t count);
        virtual void Free();

        virtual void PrintSelf(std::ostream& os) const;
    };
}
#endif /*CUDAPPHOSTMEMORY_H_*/
