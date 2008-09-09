#include "CudappStream.h"
#include "cuda_runtime_api.h"

#include "CudappEvent.h"
namespace Cudapp
{
    Stream::Stream()
    {
        cudaStreamCreate(&this->CudaStream);
    }

    Stream::~Stream()
    {
        cudaStreamDestroy(this->CudaStream);
    }

    void Stream::Synchronize()
    {
        cudaStreamSynchronize(this->CudaStream);  
    }

    /**
    * @brief Creates and returns a new CudappEvent that triggers when the Stream is finished.
    * @returns a new CudappEvent triggering on this Stream.
    */
    Event* Stream::GetStreamEvent()
    {
        Event* event = new Event;
        event->Record(this);
        return event;  
    }
}
