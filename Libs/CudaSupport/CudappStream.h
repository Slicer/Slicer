#ifndef CUDAPPSTREAM_H_
#define CUDAPPSTREAM_H_

#include "CudappBase.h"
namespace Cudapp
{
    class Event;
    class CUDA_SUPPORT_EXPORT Stream
    {
    public:
        Stream();
        virtual ~Stream();

        Base::State e();
        void Synchronize();

        cudaStream_t GetStream() const { return this->CudaStream; }
        Event* GetStreamEvent();

        virtual void PrintSelf(std::ostream& os) const {};
    protected:
        cudaStream_t CudaStream;
    };
    inline std::ostream& operator<<(std::ostream& os, const Stream& in){
    in.PrintSelf(os);
        return os; 
    }

}
#endif /*CUDAPPSTREAM_H_*/
