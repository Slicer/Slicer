#include "CudappSupport.h"
#include "CudappDevice.h"

#include <cutil.h>
#include <cuda_runtime_api.h>

namespace Cudapp
{
    Support::Support()
    {
        CheckSupportedCudaVersion();
    }

    Support::~Support()
    {
    }

    int Support::CheckSupportedCudaVersion()
    {
        int deviceCount = 0;
        cudaGetDeviceCount(&deviceCount);
        int device;
        for (device = 0; device < deviceCount; ++device)
        {

            this->Devices.push_back(Device(device));
        }

        /// HACK BY NOW
        return 0;
    }

    void Support::PrintSelf(std::ostream& os) const
    {
        os << "Cuda Support Listing all Children: "<< std::endl;
        for (int i = 0; i < this->GetDeviceCount(); ++i)
        {
            this->Devices[i].PrintSelf(os);
        }
    }
}
