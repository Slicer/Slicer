#include "CudappDeviceMemory.h"

#include "cuda_runtime_api.h"
#include "CudappBase.h"

namespace Cudapp
{
    DeviceMemory::DeviceMemory()
    {
        this->Location = MemoryBase::MemoryOnDevice;
    }

    DeviceMemory::DeviceMemory(const Memory& src) : Memory()
    {
        this->Location = MemoryBase::MemoryOnDevice;
        *this = src;
    }

    DeviceMemory::DeviceMemory(const DeviceMemory& src) : Memory()
    {
        this->Location = MemoryBase::MemoryOnDevice;
        *this = src;
    }

    DeviceMemory& DeviceMemory::operator=(const Memory& src)
    {
        src.CopyTo(this);
        return *this;
    }


    DeviceMemory::~DeviceMemory()
    {
        // so the virtual function call will not be false.
        // each subclass must call free by its own and set MemPointer to NULL in its Destructor!
        if (this->MemPointer != NULL)
            this->Free();
    }

    void DeviceMemory::Free()
    {
        if (this->MemPointer != NULL)
        {
            cudaFree(this->MemPointer);  
            this->MemPointer = NULL;
            this->Size = 0;
        }
    }

    void* DeviceMemory::AllocateBytes(size_t byte_count)
    {
        // do nothing in case we already allocated the desired size.
        if (this->GetSize() == byte_count)
            return this->MemPointer;

        this->Free();
        cudaError_t error = 
            cudaMalloc(&this->MemPointer, byte_count);
        this->Size = byte_count;
        if (error != cudaSuccess)
            Base::PrintError(error);

        return (void*) this->MemPointer;
    }

    void DeviceMemory::MemSet(int value)
    {
        cudaMemset(this->MemPointer, value, this->Size);
    }

    bool DeviceMemory::CopyTo(void* dst, size_t byte_count, size_t offset, MemoryLocation dst_loc) const
    {
        if(cudaMemcpy(dst, 
            this->GetMemPointer(), //HACK + offset,
            byte_count,
            (dst_loc == MemoryOnHost) ? cudaMemcpyDeviceToHost : cudaMemcpyDeviceToDevice
            ) == cudaSuccess)
            return true;
        else 
            return false;
    }

    bool DeviceMemory::CopyFrom(const void* src, size_t byte_count, size_t offset, MemoryLocation src_loc)
    {
        if(cudaMemcpy(this->GetMemPointer(), //HACK + offset, 
            src,
            byte_count,
            (src_loc == MemoryOnHost) ? cudaMemcpyHostToDevice : cudaMemcpyDeviceToDevice
            ) == cudaSuccess)
            return true;
        else 
            return false;
    }

    void DeviceMemory::PrintSelf (std::ostream &os) const
    {
        this->Memory::PrintSelf(os);
    }
}
