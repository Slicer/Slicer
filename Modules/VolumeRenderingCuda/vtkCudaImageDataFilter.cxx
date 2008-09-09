#include "vtkCudaImageDataFilter.h"
#include "vtkCudaImageData.h"

#include "vtkObjectFactory.h"
#include "vtkCellData.h"
#include "vtkPointData.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkStreamingDemandDrivenPipeline.h"


vtkCxxRevisionMacro(vtkCudaImageDataFilter, "$Revision: 1.0 $");
vtkStandardNewMacro(vtkCudaImageDataFilter);

vtkCudaImageDataFilter::vtkCudaImageDataFilter()
{
    this->Superclass::SetOutputScalarTypeToChar();
    this->SetNumberOfOutputPorts(1);

    // by default process active point scalars
    this->SetInputArrayToProcess(0,0,0,vtkDataObject::FIELD_ASSOCIATION_POINTS,
        vtkDataSetAttributes::SCALARS);

    OutputCudaImage = vtkCudaImageData::New();
}

// TODO Optimize
vtkCudaImageData* vtkCudaImageDataFilter::GetOutput ()
{
    this->OutputCudaImage->ShallowCopy(this->Superclass::GetOutput());
    return this->OutputCudaImage;
}

vtkCudaImageData* vtkCudaImageDataFilter::GetOutput (int n)
{
    return this->GetOutput();
}

vtkCudaImageDataFilter::~vtkCudaImageDataFilter()
{
}


//----------------------------------------------------------------------------
void vtkCudaImageDataFilter::PrintSelf(ostream& os, vtkIndent indent)
{  
    this->Superclass::PrintSelf(os,indent);
}
