#ifndef __vtkCudaVolumeMapper_h
#define __vtkCudaVolumeMapper_h

#include "vtkVolumeMapper.h"
#include "vtkVolumeRenderingCuda.h"

class vtkVolumeProperty;
class vtkMatrix4x4;
class vtkImageData;

class vtkCudaRendererInformationHandler;
class vtkCudaVolumeInformationHandler;
class vtkCudaMemoryTexture;

class VTK_VOLUMERENDERINGCUDA1_EXPORT vtkCudaVolumeMapper : public vtkVolumeMapper
{
public:
    vtkTypeRevisionMacro(vtkCudaVolumeMapper, vtkVolumeMapper);
    static vtkCudaVolumeMapper *New();

    virtual void SetInput( vtkImageData * );
    virtual void Render(vtkRenderer *, vtkVolume *);

    // Should be in Property??
    void SetThreshold(float min, float max);
    void SetThreshold(double* range) { SetThreshold((float)range[0], (float)range[1]); }
    void SetSampleDistance(float sampleDistance);

    void SetRenderOutputScaleFactor(float scaleFactor);
    
    void SetImageData(vtkImageData* data);
        
   //BTX
   void SetRenderMode(int mode);
   void SetRayCastingMethod(int mode);
   void SetInterpolationMethod(int mode);
   void SetOrientationMatrix(vtkMatrix4x4* matrix);
   void SetTransformationMatrix(vtkMatrix4x4* matrix);
   void SetIntendedFrameRate(float frameRate);

   void ClippingOn();
   void ClippingOff();

   void ShadingOn();
   void ShadingOff();

   int GetCurrentRenderMode() const;// { return this->CurrentRenderMode; }
   //ETX

   vtkImageData* GetOutput() { return NULL; /*this->LocalOutputImage;*/ }

   bool GetCUDAEnabled(){return this->CUDAEnabled;}

   void PrintSelf(ostream& os, vtkIndent indent);

protected:
    vtkCudaVolumeMapper();
    virtual ~vtkCudaVolumeMapper();

    void UpdateOutputResolution(unsigned int width, unsigned int height, bool TypeChanged = false);

    vtkCudaRendererInformationHandler* RendererInfoHandler;
    vtkCudaVolumeInformationHandler* VolumeInfoHandler;

private:
    vtkCudaVolumeMapper operator=(const vtkCudaVolumeMapper&);
    vtkCudaVolumeMapper(const vtkCudaVolumeMapper&);

    vtkImageData* ImageData;

    int RenderObjectMode; //0:Volume 1:Slice

    float IntendedFrameRate;

    void AdjustSampleDistance(float time);

    bool CUDAEnabled;
};

#endif /* __vtkCudaVolumeMapper_h */
