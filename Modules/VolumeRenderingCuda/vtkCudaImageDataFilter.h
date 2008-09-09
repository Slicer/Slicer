#ifndef VTKCUDAIMAGEDATAFILTER_H_
#define VTKCUDAIMAGEDATAFILTER_H_

#include "vtkImageShiftScale.h"
#include "vtkVolumeRenderingCudaModule.h"

class vtkCudaImageData;

class VTK_VOLUMERENDERINGCUDAMODULE_EXPORT vtkCudaImageDataFilter : public vtkImageShiftScale
{
public:
    vtkTypeRevisionMacro(vtkCudaImageDataFilter, vtkImageShiftScale);
    void PrintSelf(ostream& os, vtkIndent indent);
    static vtkCudaImageDataFilter* New();

    vtkCudaImageData* GetOutput ();
    vtkCudaImageData* GetOutput (int port);
//    virtual void SetOutput (vtkCudaImageData *d);  

protected:
    vtkCudaImageDataFilter();
    virtual ~vtkCudaImageDataFilter();

    vtkCudaImageData*   OutputCudaImage;

private:
    vtkCudaImageDataFilter(const vtkCudaImageDataFilter&);
    vtkCudaImageDataFilter& operator=(const vtkCudaImageDataFilter&) const;
};

#endif /*VTKCUDAIMAGEDATAFILTER_H_*/
