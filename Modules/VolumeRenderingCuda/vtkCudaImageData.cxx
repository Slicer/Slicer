#include "vtkCudaImageData.h"

#include "vtkObjectFactory.h"

#include "CudappDeviceMemory.h"


vtkCxxRevisionMacro(vtkCudaImageData, "$ Revision: 1.0 $");
vtkStandardNewMacro(vtkCudaImageData);


vtkCudaImageData::vtkCudaImageData()
{
    this->Data = new Cudapp::DeviceMemory;
}

vtkCudaImageData::~vtkCudaImageData()
{
 delete this->Data;
}

void vtkCudaImageData::PrintSelf (ostream &os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os, indent);
}
