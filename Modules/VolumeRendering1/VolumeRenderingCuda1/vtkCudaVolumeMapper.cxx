// Type
#include "vtkCudaVolumeMapper.h"

//#include "vtkVolumeRenderingCudaFactory.h"
#include "vtkObjectFactory.h"
#include "vtkPlaneCollection.h"
#include "vtkPlane.h"

// Volume
#include "vtkVolume.h"
#include "vtkVolumeProperty.h"

// Rendering
#include "vtkCamera.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkImageData.h"

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
}

vtkCxxRevisionMacro(vtkCudaVolumeMapper, "$Revision: 1.8 $");
vtkStandardNewMacro(vtkCudaVolumeMapper);

vtkCudaVolumeMapper::vtkCudaVolumeMapper()
{
  this->VolumeInfoHandler = vtkCudaVolumeInformationHandler::New();
  this->RendererInfoHandler = vtkCudaRendererInformationHandler::New();

  this->IntendedFrameRate = 10;
  this->CUDAEnabled = true;
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

void vtkCudaVolumeMapper::SetOrientationMatrix(vtkMatrix4x4* matrix){
  this->VolumeInfoHandler->SetOrientationMatrix(matrix);
}

void vtkCudaVolumeMapper::SetTransformationMatrix(vtkMatrix4x4* matrix){
  this->VolumeInfoHandler->SetTransformationMatrix(matrix);
}

void vtkCudaVolumeMapper::SetImageData(vtkImageData* data){
  this->ImageData=data;
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

void vtkCudaVolumeMapper::ClippingOn(){
  this->RendererInfoHandler->ClippingOn();
}

void vtkCudaVolumeMapper::ClippingOff(){
  this->RendererInfoHandler->ClippingOff();
}

void vtkCudaVolumeMapper::ShadingOn(){
  this->RendererInfoHandler->ShadingOn();
}

void vtkCudaVolumeMapper::ShadingOff(){
  this->RendererInfoHandler->ShadingOff();
}

#include "vtkTimerLog.h"
#include "cuda_runtime_api.h"

void vtkCudaVolumeMapper::Render(vtkRenderer *renderer, vtkVolume *volume)
{
  // This should update the the CudaInputBuffer only when needed.

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

    this->RendererInfoHandler->SetClippingPlanes(this->GetClippingPlanes());
    
    this->RendererInfoHandler->SetRenderer(renderer);
    this->RendererInfoHandler->Update();

    this->RendererInfoHandler->Bind();
    log->StopTimer();
    //    std::cout << "LoadTime: " << log->GetElapsedTime() * 1000.0 << std::flush; 

    log->StartTimer();
    
    CUDArenderBase_doRender(
                            this->RendererInfoHandler->GetRendererInfo(),
                            this->VolumeInfoHandler->GetVolumeInfo());         
    
    log->StopTimer();
    //    std::cout << "  RenderTime: " << (float)log->GetElapsedTime() * 1000.0 << std::flush;
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

    //    cout << " Overall Time: "<< overallTimer->GetElapsedTime()*1000.0 << endl;

    this->AdjustSampleDistance(overallTimer->GetElapsedTime());
    
    //    std::cout << "  Display Time: " << log->GetElapsedTime() << std::endl;
    log->Delete();
    overallTimer->Delete();
    return;
}

void vtkCudaVolumeMapper::AdjustSampleDistance(float time){
  float frameRate = 1.0/time;
  
  if(frameRate>this->IntendedFrameRate){
    this->VolumeInfoHandler->SetSampleDistance(this->VolumeInfoHandler->GetSampleDistance()/1.1);
  }else{
    this->VolumeInfoHandler->SetSampleDistance(this->VolumeInfoHandler->GetSampleDistance()*1.1);
  }

  if(this->VolumeInfoHandler->GetSampleDistance()<1.0){
    this->VolumeInfoHandler->SetSampleDistance(1.0);
  }
}

void vtkCudaVolumeMapper::SetIntendedFrameRate(float frameRate){
  if(frameRate<0.1){
    frameRate = 0.1;
  }
  this->IntendedFrameRate = frameRate;
}

void vtkCudaVolumeMapper::PrintSelf(ostream& os, vtkIndent indent)
{
    vtkVolumeMapper::PrintSelf(os, indent);
}

