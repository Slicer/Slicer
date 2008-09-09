#ifndef CUDAPPMEMORYBASE_H_
#define CUDAPPMEMORYBASE_H_

#include "CudappSupportModule.h"
#include <stddef.h>
#include <ostream>
namespace Cudapp
{
    class Memory;
    class DeviceMemory;
    class LocalMemory;
    class HostMemory;
    class MemoryArray;
    class MemoryPitch;

    class CUDA_SUPPORT_EXPORT MemoryBase
    {
        friend class Memory;
    public:
        virtual ~MemoryBase();

        /** @brief frees the memory (this must be called in each of the derived destructors) */
        virtual void Free() = 0;
        virtual void MemSet(int value) = 0;
        size_t GetSize() const { return Size; }

        //! The Location of the memory is either on the Device or in Main Memory (paged or unpaged) 
        typedef enum {
            MemoryOnDevice, //!< The memory is located on the Device
            MemoryOnHost,  //!< The memory is located on the Host side
        } MemoryLocation;

        MemoryLocation GetMemoryLocation() const { return this->Location; }

        virtual bool CopyTo(void* dst, size_t byte_count, size_t offset = 0, MemoryLocation dst_loc = MemoryOnHost) const = 0;
        virtual bool CopyFrom(const void* src, size_t byte_count, size_t offset = 0, MemoryLocation src_loc = MemoryOnHost) = 0;

        // This function does a cast of this to the specified type and then a cast of the other to the specified type, so we are sure from what memory to what we are copying.
        virtual bool CopyTo(MemoryBase* dst) const { return false; /* To give you a sense what this does:  dst->CopyFromInternal(this); */ }

        virtual void PrintSelf(std::ostream &os) const;

    protected:
        MemoryBase();
        MemoryBase(const MemoryBase&);
        MemoryBase& operator=(const MemoryBase&);

        size_t Size;    //!< The size of the Allocated Memory
        MemoryLocation Location;

        virtual bool CopyFromInternal(const Memory* src) { return false; } // HACK -> Temporary
        virtual bool CopyFromInternal(const MemoryPitch* src) { return false; } // HACK -> Temporary
        virtual bool CopyFromInternal(const MemoryArray* src) { return false; } // HACK -> Temporary
    };

    inline std::ostream& operator<<(std::ostream& os, const MemoryBase& in){
        in.PrintSelf(os);
        return os; 
    }
}
#endif /*CUDAPPMEMORYBASE_H_*/
