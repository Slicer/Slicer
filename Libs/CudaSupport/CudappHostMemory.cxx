#include "CudappHostMemory.h"
#include "CudappBase.h"
#include "cuda_runtime_api.h"

#include <string.h>
namespace Cudapp
{
    HostMemory::HostMemory()
    {
    }

    HostMemory::~HostMemory()
    {
        this->Free();
    }

    HostMemory::HostMemory(const HostMemory& other) : LocalMemory()
    {
       *this = other;
    }

    HostMemory& HostMemory::operator=(const HostMemory& other)
    { 
        other.CopyTo(this);
        return *this;
    }

    void* HostMemory::AllocateBytes(size_t count)
    {
        this->Free();
        cudaError_t error = 
            cudaMallocHost(&this->MemPointer, count);
        this->Size = count;
        if (error != cudaSuccess)
            Base::PrintError(error);

        return (void*) this->MemPointer;
    }

    void HostMemory::Free()
    {
        if (this->MemPointer != NULL)
        {
            cudaFreeHost(this->MemPointer);
            this->MemPointer = NULL;  
            this->Size = 0;
        }
    }

    void HostMemory::PrintSelf(std::ostream&  os) const
    {
        this->LocalMemory::PrintSelf(os);
    }
}
