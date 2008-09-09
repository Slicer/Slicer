#include "CudappMemoryPitch.h"
#include "cuda_runtime_api.h"
#include "CudappBase.h"

namespace Cudapp
{
    MemoryPitch::MemoryPitch()
    {
        this->Location = MemoryBase::MemoryOnDevice;
        this->Location = MemoryBase::MemoryOnDevice;

        this->Pitch = 0;
    }

    MemoryPitch::~MemoryPitch()
    {
        this->Free();
    }


    void* MemoryPitch::AllocatePitchBytes(size_t width, size_t height, size_t typeSize)
    {
        this->Free();
        cudaError_t error = 
            cudaMallocPitch(&this->MemPointer, &this->Pitch, width * typeSize, height);
        this->Width = width;
        this->Height = height;
        if (error != cudaSuccess)
            Base::PrintError(error);

        return (void*)this->MemPointer;
    }



    void MemoryPitch::Free()
    {  
        if (this->MemPointer != NULL)
        {
            cudaFree(this->MemPointer);  
            this->MemPointer = NULL;
            this->Size = 0;
            this->Pitch = 0;
        }
    }


    void MemoryPitch::MemSet(int value)
    {
        cudaMemset2D(this->MemPointer, this->Pitch, value, this->Width, this->Height);  
    }

    void MemoryPitch::PrintSelf(std::ostream &os) const
    {
        this->MemoryBase::PrintSelf(os);
        os << " Width: "<< this->GetWidth() << 
            " Height: " << this->GetHeight() <<
            " Pitch: " << this->GetPitch();
    }
}
