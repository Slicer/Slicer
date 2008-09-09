#ifndef CUDAPPEVENT_H_
#define CUDAPPEVENT_H_

#include "CudappBase.h"
namespace Cudapp
{
    class Stream;
    class CUDA_SUPPORT_EXPORT Event
    {
    public:
        Event();
        virtual ~Event();

        void Record();
        void Record(Stream* stream);
        Base::State Query();
        void Synchronize();
        float ElapsedTime(Event* otherEvent);

        /** @returns the Event */
        cudaEvent_t GetEvent() { return this->CudaEvent; }

        virtual void PrintSelf(std::ostream&  os) const;

    private:
        cudaEvent_t CudaEvent;
    };
    inline std::ostream& operator<<(std::ostream& os, const Event& in){
        in.PrintSelf(os);
        return os; 
    }

}
#endif /*CUDAPPEVENT_H_*/
