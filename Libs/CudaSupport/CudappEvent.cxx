#include "CudappEvent.h"
#include "CudappStream.h"
#include "CudappBase.h"

#include "cuda_runtime_api.h"
namespace Cudapp
{
    Event::Event()
    {
        cudaEventCreate(&this->CudaEvent);
    }

    Event::~Event()
    {
        cudaEventDestroy(this->CudaEvent);
    }

    void Event::Record()
    {
        cudaEventRecord(this->CudaEvent, 0);  
    }

    void Event::Record(Stream* stream)
    {
        if (stream == NULL)
            this->Record();
        else
            cudaEventRecord(this->CudaEvent, stream->GetStream());
    }

    Base::State Event::Query()
    {
        switch(cudaEventQuery(this->CudaEvent))
        {
        case cudaSuccess:
            return Base::Success;
        case cudaErrorNotReady:
            return Base::NotReadyError;
        case cudaErrorInvalidValue:
        default:
            return Base::InvalidValueError;    
        }
    }

    void Event::Synchronize()
    {
        if (cudaEventSynchronize(this->CudaEvent) == cudaErrorInvalidValue)
            Base::PrintError(cudaErrorInvalidValue);
    }

    /**
    * @returns the time between the finish of two events.
    * @param otherEvent the event that finished later than this event (the end event if this is the start-event.
    */
    float Event::ElapsedTime(Event* otherEvent)
    {
        float elapsedTime = 0.0;
        cudaEventElapsedTime(&elapsedTime, this->CudaEvent, otherEvent->GetEvent());
        return elapsedTime;
    }

    void Event::PrintSelf(std::ostream&  os) const
    {
    }
}
