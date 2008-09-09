#include "CudappDevice.h"
#include "cuda_runtime_api.h"
#include "CudappMemoryArray.h"

namespace Cudapp
{
    Device::Device(unsigned int deviceNumber)
    {
        this->Initialized = false;

        // set the device properties to a 'don't care' value
        
        cudaDeviceProp prop = cudaDevicePropDontCare;
        this->DeviceProp = prop;

        this->SetDeviceNumber(deviceNumber);
    }

    Device::~Device()
    {
    }

    /**
    * TODO Remove this function. just for trial
    */
    bool Device::AllocateMemory()
    {
        return false;
    }

    void Device::SetDeviceNumber(unsigned int deviceNumber)
    {
        this->DeviceNumber = deviceNumber;
        this->LoadDeviceProperties();
    }

    void Device::LoadDeviceProperties()
    {
        cudaGetDeviceProperties(&DeviceProp, this->DeviceNumber);
    }

    void Device::MakeActive()
    {
        cudaSetDevice(this->DeviceNumber);
        this->Initialized = true;
    }

    void Device::SynchronizeThread()
    {
        cudaThreadSynchronize();
    }
    void Device::ExitThread()
    {

    }

    void Device::PrintSelf(std::ostream&  os) const
    {
        os << "Device Name = " << this->GetName() << 
            "DeviceNumber = " << this->GetDeviceNumber() <<
            "Version = " << this->GetMajor() << "." << this->GetMinor() <<
            "Memory: Global = " << this->GetTotalGlobalMem() <<
            " Shared Per Block = " << this->GetSharedMemPerBlock() <<
            " Regisers Per Block = " << this->GetRegsPerBlock() << 
            " Wrap Size " << this->GetWrapSize() <<
            " Pitch Size = " << this->GetMemPitch() <<
            " Threads Per Block = " << this->GetMaxThreadsPerBlock() <<
            " Max Threads Dimension = " << this->GetMaxThreadsDim()[0] << "x" << this->GetMaxThreadsDim()[1] << "x" << this->GetMaxThreadsDim()[2] <<
            " Max Grid Size = " << this->GetMaxGridSize()[0] << "x" << this->GetMaxGridSize()[1] << "x" << this->GetMaxGridSize()[2] <<
            " Total Constant Memory = " << this->GetTotalConstMem() << 
            " Clock Rate = " << this->GetClockRate() << "kHz" << 
            " Texture Alignment = " << this->GetTextureAlignment();
    }
}
