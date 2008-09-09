#include "CudappLocalMemory.h"

#include "CudappBase.h"

#include <string.h>
#include "cuda_runtime_api.h"
namespace Cudapp
{
    LocalMemory::LocalMemory()
    {
        this->Location = MemoryBase::MemoryOnHost;
    }

    LocalMemory::~LocalMemory()
    {
        this->Free();
    }

    LocalMemory::LocalMemory(const LocalMemory& other) : Memory()
    {
        this->Location = MemoryBase::MemoryOnHost;
       *this = other;
    }

    LocalMemory& LocalMemory::operator=(const LocalMemory& other)
    { 
        other.CopyTo(this);
        return *this;
    }

    void* LocalMemory::AllocateBytes(size_t count)
    {
        this->Free();
        this->MemPointer = malloc(count);
        this->Size = count;
        if (this->MemPointer == NULL)
            return NULL; // HACK

        return (void*)this->MemPointer;
    }

    void LocalMemory::Free()
    {
        if (this->MemPointer != NULL)
        {
            free (this->MemPointer);
            this->MemPointer = NULL;  
            this->Size = 0;
        }
    }

    /**
    * @brief host implementation of the MemorySetter Value
    */
    void LocalMemory::MemSet(int value)
    {
        memset(this->MemPointer, value, Size);
    }


    bool LocalMemory::CopyTo(void* dst, size_t byte_count, size_t offset, MemoryLocation dst_loc) const
    {
        if(cudaMemcpy(dst,
            this->GetMemPointer(), //HACK  + offset,
            byte_count,
            (dst_loc == MemoryOnHost) ? cudaMemcpyHostToHost : cudaMemcpyHostToDevice
            ) == cudaSuccess)
            return true;
        else 
            return false;
    }

    bool LocalMemory::CopyFrom(const void* src, size_t byte_count, size_t offset, MemoryLocation src_loc)
    {
        if(cudaMemcpy(this->GetMemPointer(), //HACK  + offset, 
            src,
            byte_count,
            (src_loc == MemoryOnHost) ? cudaMemcpyHostToHost : cudaMemcpyDeviceToHost
            ) == cudaSuccess)
            return true;
        else 
            return false;
    }

    void LocalMemory::PrintSelf(std::ostream&  os) const
    {
        this->Memory::PrintSelf(os);
    }
}
