#ifndef VTKCUDARENDERERINFORMATIONHANDLER_H_
#define VTKCUDARENDERERINFORMATIONHANDLER_H_

#include "vtkObject.h"
#include "vtkVolumeRenderingCuda.h"

class vtkRenderer;
class vtkMatrix4x4;
class vtkCudaMemoryTexture;
class vtkPlaneCollection;

//BTX
#include "CudappDeviceMemory.h"
#include "CudappHostMemory.h"
#include "cudaRendererInformation.h"
//ETX
class VTK_VOLUMERENDERINGCUDA_EXPORT vtkCudaRendererInformationHandler : public vtkObject
{
    vtkTypeRevisionMacro(vtkCudaRendererInformationHandler, vtkObject);
public:
    static vtkCudaRendererInformationHandler* New();

    //BTX
    void SetRenderer(vtkRenderer* renderer);
    vtkGetMacro(Renderer, vtkRenderer*);
    //const cudaRendererInformation& GetRendererInfo() { return this->RendererInfo; }
    cudaRendererInformation& GetRendererInfo() { return this->RendererInfo; }
    //ETX

    void Bind();
    void Unbind();

    void SetRenderOutputScaleFactor(float scaleFactor);
    void SetLensMappingMode(int mode);
    void SetRayCastingMethod(int mode);
    void SetInterpolationMethod(int mode);
    void SetProjectionMethod(int mode);
    void SetClippingPlanes(vtkPlaneCollection* collection);

    void ClippingOn();
    void ClippingOff();

    void ShadingOn();
    void ShadingOff();

    virtual void Update();

protected:
    vtkCudaRendererInformationHandler();
    ~vtkCudaRendererInformationHandler();

    void UpdateResolution(unsigned int width, unsigned int height);
private:
    vtkCudaRendererInformationHandler& operator=(const vtkCudaRendererInformationHandler&); // not implemented
    vtkCudaRendererInformationHandler(const vtkCudaRendererInformationHandler&); // not implemented


private:
    vtkRenderer*             Renderer;
    //BTX
    cudaRendererInformation  RendererInfo;

    float                    RenderOutputScaleFactor;
    int LensMappingMode;
    int RayCastingMethod;
    int InterpolationMethod;

    vtkCudaMemoryTexture*    MemoryTexture;
    Cudapp::HostMemory       LocalZBuffer;
    Cudapp::DeviceMemory     CudaZBuffer;

    //ETX
};

#endif /* VTKCUDARENDERERINFORMATIONHANDLER_H_ */
