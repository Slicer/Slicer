#ifndef CUDAPPMEMORYPITCH_H_
#define CUDAPPMEMORYPITCH_H_

#include "CudappMemoryBase.h"
namespace Cudapp
{
    class CUDA_SUPPORT_EXPORT MemoryPitch : public MemoryBase
    {
    public:
        MemoryPitch();
        virtual ~MemoryPitch();
        MemoryPitch(const MemoryPitch&);
        MemoryPitch& operator=(const MemoryPitch&);

        void* AllocatePitchBytes(size_t width, size_t height, size_t typeSize);
        virtual void Free();

        virtual void MemSet(int value);

        template<typename T> T* AllocatePitch(size_t width, size_t height)
            { return (T*)this->AllocatePitchBytes(width, height, sizeof(T)); }

        void* GetMemPointer() { return this->MemPointer; }
        const void* GetMemPointer() const { return this->MemPointer; }

        size_t GetPitch() const { return this->Pitch; }
        size_t GetWidth() const { return this->Width; } 
        size_t GetHeight() const { return this->Height; }

        //HACK    
        virtual bool CopyTo(void* dst, size_t byte_count, size_t offset = 0, MemoryLocation dst_loc = MemoryOnHost) { return false; }
        virtual bool CopyFrom(void* src, size_t byte_count, size_t offset = 0, MemoryLocation src_loc = MemoryOnHost) { return false; }


        virtual void PrintSelf (std::ostream &os) const;
    protected:

        size_t Pitch;
        size_t Width;
        size_t Height;

        void* MemPointer;
    };
}
#endif /*CUDAPPMEMORYPITCH_H_*/
