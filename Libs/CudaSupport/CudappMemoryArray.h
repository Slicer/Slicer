#ifndef CUDAPPMEMORYARRAY_H_
#define CUDAPPMEMORYARRAY_H_

#include "CudappMemoryBase.h"
#include "channel_descriptor.h"

namespace Cudapp
{
    class CUDA_SUPPORT_EXPORT MemoryArray : public MemoryBase
    {
    public:
        MemoryArray();
        virtual ~MemoryArray();
        MemoryArray(const MemoryArray&);
        MemoryArray& operator=(const MemoryArray&);

        template<typename T>
        void SetFormat() { this->Descriptor = cudaCreateChannelDesc<T>(); }
        void SetChannelDescriptor(const cudaChannelFormatDesc& desc) { this->Descriptor = desc; }

        void Allocate(size_t width, size_t height);
        virtual void Free();
        virtual void MemSet(int value) {}

        void DeepCopy(MemoryArray* source); 

        const cudaChannelFormatDesc& GetDescriptor() const { return this->Descriptor; } 
        cudaArray* GetArray() const { return this->Array; }

        size_t GetWidth() const { return this->Width; }
        size_t GetHeight() const { return this->Height; }

        //HACK    
        virtual bool CopyTo(void* dst, size_t byte_count, size_t offset = 0, MemoryLocation dst_loc = MemoryOnHost) const { return false; }
        virtual bool CopyFrom(const void* src, size_t byte_count, size_t offset = 0, MemoryLocation src_loc = MemoryOnHost) { return false; }

        virtual void PrintSelf(std::ostream &os) const;

    protected:

        cudaChannelFormatDesc Descriptor; //!< The Descriptor used to allocate memory
        cudaArray* Array; //!< The Array with the memory that was allocated.
        size_t Width;  //!< The Width of the Array
        size_t Height; //!< The Height of the Array
    };
}
#endif /*CUDAPPMEMORYARRAY_H_*/
