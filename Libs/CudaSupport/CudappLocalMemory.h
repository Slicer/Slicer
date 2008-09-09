#ifndef CUDAPPLOCALMEMORY_H_
#define CUDAPPLOCALMEMORY_H_

#include "CudappMemory.h"

namespace Cudapp
{
    class CUDA_SUPPORT_EXPORT LocalMemory : public Memory
    {
    public:
        LocalMemory();
        virtual ~LocalMemory();
        LocalMemory(const LocalMemory&);
        LocalMemory& operator=(const LocalMemory&);

        virtual void* AllocateBytes(size_t count);
        virtual void Free();
        virtual void MemSet(int value); 

        virtual bool CopyTo(void* dst, size_t byte_count, size_t offset = 0, MemoryLocation dst_loc = MemoryOnHost) const;
        virtual bool CopyFrom(const void* src, size_t byte_count, size_t offset = 0, MemoryLocation src_loc = MemoryOnHost);

        virtual bool CopyTo(MemoryBase* dst) const { return this->Memory::CopyTo(dst); }

        virtual void PrintSelf(std::ostream& os) const;
    };
}
#endif /* CUDAPPLOCALMEMORY_H_ */
