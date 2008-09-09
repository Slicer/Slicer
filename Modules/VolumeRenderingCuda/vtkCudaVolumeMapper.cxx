// Type
#include "vtkCudaVolumeMapper.h"
#include "vtkVolumeRenderingCudaFactory.h"
#include "vtkObjectFactory.h"

// Volume
#include "vtkVolume.h"
#include "vtkVolumeProperty.h"

// Rendering
#include "vtkCamera.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkImageData.h"
#include "vtkImageViewer2.h"

// CUDA
#include "CudappSupport.h"
#include "vtkImageData.h"
#include "CudappDeviceMemory.h"
#include "CudappHostMemory.h"

// VTKCUDA
#include "vtkCudaVolumeInformationHandler.h"
#include "vtkCudaRendererInformationHandler.h"
#include "vtkCudaMemoryTexture.h"

#include "vtkgl.h"
extern "C" {
#include "CUDA_renderBase.h"
#include "CUDA_renderSlice.h"
#include "CUDA_renderSingleSlice.h"
}


vtkCxxRevisionMacro(vtkCudaVolumeMapper, "$Revision: 1.8 $");
vtkStandardNewMacro(vtkCudaVolumeMapper);

vtkCudaVolumeMapper::vtkCudaVolumeMapper()
{
  this->VolumeInfoHandler = vtkCudaVolumeInformationHandler::New();
  this->RendererInfoHandler = vtkCudaRendererInformationHandler::New();
  this->RenderObjectMode = 0;
}  

vtkCudaVolumeMapper::~vtkCudaVolumeMapper()
{
  this->VolumeInfoHandler->Delete();
  this->RendererInfoHandler->Delete();
}

void vtkCudaVolumeMapper::SetInput(vtkImageData * input)
{
  
  this->Superclass::SetInput(input);
  this->VolumeInfoHandler->SetInputData(input);
}

void vtkCudaVolumeMapper::SetRenderMode(int mode)
{
    //HACK
    //this->MemoryTexture->SetRenderMode(mode);
}

void vtkCudaVolumeMapper::SetRayCastingMethod(int mode)
{
  this->RendererInfoHandler->SetRayCastingMethod(mode);
}

void vtkCudaVolumeMapper::SetInterpolationMethod(int mode)
{
  this->RendererInfoHandler->SetInterpolationMethod(mode);
}

void vtkCudaVolumeMapper::SetRenderObjectMode(int mode)
{
  this->RenderObjectMode = mode;
}

void vtkCudaVolumeMapper::SetVolumeRenderDirection(int mode)
{
  this->VolumeInfoHandler->SetVolumeRenderDirection(mode);
}

void vtkCudaVolumeMapper::SetOrientationMatrix(vtkMatrix4x4* matrix){
  this->VolumeInfoHandler->SetOrientationMatrix(matrix);
}

void vtkCudaVolumeMapper::SetTransformationMatrix(vtkMatrix4x4* matrix){
  this->VolumeInfoHandler->SetTransformationMatrix(matrix);
}

void vtkCudaVolumeMapper::SetSliceMatrix(vtkMatrix4x4* matrix){
  this->VolumeInfoHandler->SetSliceMatrix(matrix);
}

void vtkCudaVolumeMapper::SetImageData(vtkImageData* data){
  this->ImageData=data;
}

void vtkCudaVolumeMapper::SetImageViewer(vtkImageViewer2* viewer){
  this->ImageViewer=viewer;
}

int vtkCudaVolumeMapper::GetCurrentRenderMode() const
{
    //HACK
    return 0; //this->MemoryTexture->GetCurrentRenderMode();
    //TODO
}

/**
* @brief sets the Threshold of the Input Array
*/
void vtkCudaVolumeMapper::SetThreshold(float min, float max)
{
    this->VolumeInfoHandler->SetThreshold(min, max);
}

void vtkCudaVolumeMapper::SetSampleDistance(float sampleDistance)
{
    this->VolumeInfoHandler->SetSampleDistance(sampleDistance);
}

void vtkCudaVolumeMapper::SetRenderOutputScaleFactor(float scaleFactor)
{
    this->RendererInfoHandler->SetRenderOutputScaleFactor(scaleFactor);
}


#include "vtkTimerLog.h"
#include "cuda_runtime_api.h"

void vtkCudaVolumeMapper::Render(vtkRenderer *renderer, vtkVolume *volume)
{
    // This should update the the CudaInputBuffer only when needed.
    //if (this->GetInput()->GetMTime() > this->GetMTime())
    //  this->CudaInputBuffer->CopyFrom(this->GetInput()->GetScalarPointer(), this->GetInput()->GetScalarSize());

    vtkTimerLog* overallTimer = vtkTimerLog::New();
    vtkTimerLog* log = vtkTimerLog::New();

    overallTimer->StartTimer();
    log->StartTimer();
    // Renderer Information Setter.

    if (volume != this->VolumeInfoHandler->GetVolume())
      this->VolumeInfoHandler->SetVolume(volume);

    this->VolumeInfoHandler->Update();
    cudaThreadSynchronize();
    log->StopTimer();
    
    this->RendererInfoHandler->SetRenderer(renderer);
    this->RendererInfoHandler->Bind();
    log->StopTimer();
    std::cout << "LoadTime: " << log->GetElapsedTime() * 1000.0 << std::flush; 

    log->StartTimer();
    
    if(this->VolumeInfoHandler->GetVolumeInfo().VolumeSize.z==1){
      CUDArenderSingleSlice_doRender(
                              this->RendererInfoHandler->GetRendererInfo(),
                              this->VolumeInfoHandler->GetVolumeInfo());         

    }else{
      if(this->RenderObjectMode == 0){
        CUDArenderBase_doRender(
                                this->RendererInfoHandler->GetRendererInfo(),
                                this->VolumeInfoHandler->GetVolumeInfo());         
      }else{
        CUDArenderSlice_doRender(
                                 this->RendererInfoHandler->GetRendererInfo(),
                                 this->VolumeInfoHandler->GetVolumeInfo());         
      }
    }
    
    log->StopTimer();
    std::cout << "  RenderTime: " << (float)log->GetElapsedTime() * 1000.0 << std::flush;
    log->StartTimer();
    
    // Enter 2D Mode
    
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, 1.0, 1.0, 0.0, 0.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Actual Rendering
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glTexCoord2i(1,0);   glVertex2i(0,1);
    glTexCoord2i(0,0);   glVertex2i(1,1);
    glTexCoord2i(0,1);   glVertex2i(1,0);
    glTexCoord2i(1,1);   glVertex2i(0,0);
    glEnd();
    
    this->RendererInfoHandler->Unbind();

    // Leave the 2D Mode again.
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
    
    cudaThreadSynchronize();
    overallTimer->StopTimer();

    cout << " Overall Time: "<< overallTimer->GetElapsedTime()*1000.0 << endl;

    //    std::cout << "  Display Time: " << log->GetElapsedTime() << std::endl;
    log->Delete();

    return;
}

void vtkCudaVolumeMapper::PrintSelf(ostream& os, vtkIndent indent)
{
    vtkVolumeMapper::PrintSelf(os, indent);
}
