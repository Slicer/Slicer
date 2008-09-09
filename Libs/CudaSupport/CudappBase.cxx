#include "CudappBase.h"

#include "cuda_runtime_api.h"

namespace Cudapp
{

    cudaError_t Base::GetLastError()
    {
        return cudaGetLastError();
    }

    const char* Base::GetLastErrorString()
    {
        return Base::GetErrorString(Base::GetLastError());
    }

    const char* Base::GetErrorString(cudaError_t error)
    {
        return cudaGetErrorString(error);
    }

    void Base::PrintError(cudaError_t error)
    {
        printf(Base::GetErrorString(error));
    }

    Base::~Base()
    {
    }

    Base::Base()
    {
    }
}
