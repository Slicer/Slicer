#include "vtkCudaRendererInformationHandler.h"

// std
#include <vector>
// cuda functions
#include "vector_functions.h"

// vtk base
#include "vtkObjectFactory.h"

// Renderer Information
#include "vtkCamera.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkMatrix4x4.h"
#include "vtkPlaneCollection.h"

// vtkCuda
#include "vtkCudaMemoryTexture.h"
vtkCxxRevisionMacro(vtkCudaRendererInformationHandler, "$Revision: 1.0 $");
vtkStandardNewMacro(vtkCudaRendererInformationHandler);

vtkCudaRendererInformationHandler::vtkCudaRendererInformationHandler()
{
  this->Renderer = NULL;
  this->RendererInfo.ActualResolution.x = this->RendererInfo.ActualResolution.y = 0;
  this->MemoryTexture = vtkCudaMemoryTexture::New();
  
  this->SetRenderOutputScaleFactor(0.3f);
  this->SetLensMappingMode(2);
  this->SetRayCastingMethod(2);
  this->SetInterpolationMethod(0);
  this->SetProjectionMethod(0);

  this->RendererInfo.CroppingPlaneNumber=0;
  this->ClippingOff();
  this->ShadingOff();
}

vtkCudaRendererInformationHandler::~vtkCudaRendererInformationHandler()
{
  this->Renderer = NULL;
  this->MemoryTexture->Delete();
}


void vtkCudaRendererInformationHandler::SetRenderer(vtkRenderer* renderer)
{
  this->Renderer = renderer;
}

void vtkCudaRendererInformationHandler::SetRenderOutputScaleFactor(float scaleFactor) 
{
  this->RenderOutputScaleFactor = (scaleFactor > 0.1) ? scaleFactor : 0.1;
}

void vtkCudaRendererInformationHandler::SetLensMappingMode(int mode) 
{
  this->LensMappingMode=mode;
}

void vtkCudaRendererInformationHandler::SetRayCastingMethod(int mode) 
{
  this->RayCastingMethod=mode;
}

void vtkCudaRendererInformationHandler::SetInterpolationMethod(int mode) 
{
  this->InterpolationMethod=mode;
}

void vtkCudaRendererInformationHandler::SetProjectionMethod(int mode) 
{
  this->RendererInfo.projectionMethod=mode;
}

void vtkCudaRendererInformationHandler::ClippingOn(){
  this->RendererInfo.ClippingOn = 1;
}

void vtkCudaRendererInformationHandler::ClippingOff(){
  this->RendererInfo.ClippingOn = 0;
}

void vtkCudaRendererInformationHandler::ShadingOn(){
  this->RendererInfo.ShadingOn = 1;
}

void vtkCudaRendererInformationHandler::ShadingOff(){
  this->RendererInfo.ShadingOn = 0;
}

void vtkCudaRendererInformationHandler::SetClippingPlanes(vtkPlaneCollection* collection){
  if(collection!=NULL){
    int num = collection->GetNumberOfItems();
    if(num>10)num = 10;
    this->RendererInfo.CroppingPlaneNumber = num;
    vtkPlane *plane;
    double normal[3];
    double origin[3];
    for(int i=0; i<num; i++){
      plane = static_cast<vtkPlane*>(collection->GetItemAsObject(i));
      if(plane != NULL){
        plane->GetOrigin(origin);
        plane->GetNormal(normal);

        this->RendererInfo.CroppingNormal[i].x=(float)normal[0];
        this->RendererInfo.CroppingNormal[i].y=(float)normal[1];
        this->RendererInfo.CroppingNormal[i].z=(float)normal[2];
        this->RendererInfo.CroppingOrigin[i].x=(float)origin[0];
        this->RendererInfo.CroppingOrigin[i].y=(float)origin[1];
        this->RendererInfo.CroppingOrigin[i].z=(float)origin[2];

      }
    }
  }
}

void vtkCudaRendererInformationHandler::Bind()
{
  this->MemoryTexture->BindTexture();
  this->MemoryTexture->BindBuffer();
  this->RendererInfo.OutputImage = (uchar4*)this->MemoryTexture->GetRenderDestination();
}

void vtkCudaRendererInformationHandler::Unbind()
{
  this->MemoryTexture->UnbindBuffer();
  this->MemoryTexture->UnbindTexture();
}

void vtkCudaRendererInformationHandler::Update()
{
  if (this->Renderer != NULL){
    // Renderplane Update.
    vtkRenderWindow *renWin= this->Renderer->GetRenderWindow();
    int *size=renWin->GetSize();
    if (size[0] != this->RendererInfo.ActualResolution.x ||
        size[1] != this->RendererInfo.ActualResolution.y)
      {
        this->RendererInfo.ActualResolution.x = size[0];
        this->RendererInfo.ActualResolution.y = size[1];
          
        // HACK -> Allocate is too slow!!
        LocalZBuffer.Allocate<float>(this->RendererInfo.ActualResolution.x * this->RendererInfo.ActualResolution.y);
        CudaZBuffer.Allocate<float>(this->RendererInfo.ActualResolution.x * this->RendererInfo.ActualResolution.y);
          
      }
    this->RendererInfo.Resolution.x = this->RendererInfo.ActualResolution.x ;/// this->RenderOutputScaleFactor;
    this->RendererInfo.Resolution.y = this->RendererInfo.ActualResolution.y ;/// this->RenderOutputScaleFactor;
      
    this->RendererInfo.ScaleFactor = this->RenderOutputScaleFactor;
      
    this->MemoryTexture->SetSize(this->RendererInfo.Resolution.x, this->RendererInfo.Resolution.y);
    this->RendererInfo.OutputImage = (uchar4*)this->MemoryTexture->GetRenderDestination();
      
    vtkCamera* cam = this->Renderer->GetActiveCamera();
      
    // Update Lights.
    std::vector<float3> lights;
    lights.push_back(make_float3(0,0,1));
      
    this->RendererInfo.LightCount = lights.size();
    if (!lights.empty())
      this->RendererInfo.LightVectors = &lights[0];
      
    // Update Camera
    this->RendererInfo.CameraPos.x = cam->GetPosition()[0];
    this->RendererInfo.CameraPos.y = cam->GetPosition()[1];
    this->RendererInfo.CameraPos.z = cam->GetPosition()[2];
      
    this->RendererInfo.CameraDirection.x= cam->GetDirectionOfProjection()[0];
    this->RendererInfo.CameraDirection.y= cam->GetDirectionOfProjection()[1];
    this->RendererInfo.CameraDirection.z= cam->GetDirectionOfProjection()[2];
        
    this->RendererInfo.ViewUp.x = cam->GetViewUp()[0];
    this->RendererInfo.ViewUp.y = cam->GetViewUp()[1];
    this->RendererInfo.ViewUp.z = cam->GetViewUp()[2];
      
    float dot = this->RendererInfo.ViewUp.x * this->RendererInfo.CameraDirection.x +
      this->RendererInfo.ViewUp.y * this->RendererInfo.CameraDirection.y + 
      this->RendererInfo.ViewUp.z * this->RendererInfo.CameraDirection.z;
      
    this->RendererInfo.VerticalVec.x = (this->RendererInfo.ViewUp.x - dot * this->RendererInfo.CameraDirection.x)/this->RenderOutputScaleFactor;
    this->RendererInfo.VerticalVec.y = (this->RendererInfo.ViewUp.y - dot * this->RendererInfo.CameraDirection.y)/this->RenderOutputScaleFactor;
    this->RendererInfo.VerticalVec.z = (this->RendererInfo.ViewUp.z - dot * this->RendererInfo.CameraDirection.z)/this->RenderOutputScaleFactor;
      
    this->RendererInfo.HorizontalVec.x = (this->RendererInfo.VerticalVec.y * this->RendererInfo.CameraDirection.z - 
                                          this->RendererInfo.VerticalVec.z * this->RendererInfo.CameraDirection.y);
    this->RendererInfo.HorizontalVec.y = (this->RendererInfo.VerticalVec.z * this->RendererInfo.CameraDirection.x - 
                                          this->RendererInfo.VerticalVec.x * this->RendererInfo.CameraDirection.z);
    this->RendererInfo.HorizontalVec.z = (this->RendererInfo.VerticalVec.x * this->RendererInfo.CameraDirection.y - 
                                          this->RendererInfo.VerticalVec.y * this->RendererInfo.CameraDirection.x);
      
    // additional
      
    this->RendererInfo.rayCastingMethod=this->RayCastingMethod;
    this->RendererInfo.interpolationMethod=this->InterpolationMethod;
      
    // Getting Points along the ray.
      
    double Point[4];
    double PointX[4];
    double PointY[4];
      
    this->Renderer->SetDisplayPoint(this->RendererInfo.ActualResolution.x-1,  
                                    0, 
                                    0); 
    this->Renderer->DisplayToWorld();
      
    this->Renderer->GetWorldPoint(Point);
    this->Renderer->SetDisplayPoint(0,
                                    0, 
                                    0); 
    this->Renderer->DisplayToWorld();
      
    this->Renderer->GetWorldPoint(PointX);
    this->Renderer->SetDisplayPoint(this->RendererInfo.ActualResolution.x-1,  
                                    this->RendererInfo.ActualResolution.y-1, 
                                    0); 
    this->Renderer->DisplayToWorld();
    this->Renderer->GetWorldPoint(PointY);
      
    this->RendererInfo.CameraRayStart.x = Point[0]; 
    this->RendererInfo.CameraRayStart.y = Point[1]; 
    this->RendererInfo.CameraRayStart.z = Point[2]; 
      
    this->RendererInfo.CameraRayStartX.x = (PointX[0] - Point[0]); 
    this->RendererInfo.CameraRayStartX.y = (PointX[1] - Point[1]); 
    this->RendererInfo.CameraRayStartX.z = (PointX[2] - Point[2]);
      
    this->RendererInfo.CameraRayStartY.x = (PointY[0] - Point[0]); 
    this->RendererInfo.CameraRayStartY.y = (PointY[1] - Point[1]); 
    this->RendererInfo.CameraRayStartY.z = (PointY[2] - Point[2]);
      
    // Calculate Ray end and Perpendicular Vectors
      
    this->Renderer->SetDisplayPoint(this->RendererInfo.ActualResolution.x-1,
                                    0, 
                                    1); 
    this->Renderer->DisplayToWorld();
    this->Renderer->GetWorldPoint(Point);
      
    this->Renderer->SetDisplayPoint(0,
                                    0, 
                                    1); 
    this->Renderer->DisplayToWorld();
    this->Renderer->GetWorldPoint(PointX);
      
    this->Renderer->SetDisplayPoint(this->RendererInfo.ActualResolution.x-1, 
                                    this->RendererInfo.ActualResolution.y-1, 
                                    1); 
    this->Renderer->DisplayToWorld();
    this->Renderer->GetWorldPoint(PointY);
      
    this->RendererInfo.CameraRayEnd.x = Point[0]; 
    this->RendererInfo.CameraRayEnd.y = Point[1]; 
    this->RendererInfo.CameraRayEnd.z = Point[2]; 
      
    this->RendererInfo.CameraRayEndX.x = (PointX[0] - Point[0]); 
    this->RendererInfo.CameraRayEndX.y = (PointX[1] - Point[1]); 
    this->RendererInfo.CameraRayEndX.z = (PointX[2] - Point[2]);
      
    this->RendererInfo.CameraRayEndY.x = (PointY[0] - Point[0]); 
    this->RendererInfo.CameraRayEndY.y = (PointY[1] - Point[1]); 
    this->RendererInfo.CameraRayEndY.z = (PointY[2] - Point[2]);
      
    this->RendererInfo.ClippingPlaneNormal.x=this->RendererInfo.CameraRayStartX.y*this->RendererInfo.CameraRayStartY.z-this->RendererInfo.CameraRayStartX.z*this->RendererInfo.CameraRayStartY.y;
    this->RendererInfo.ClippingPlaneNormal.y=this->RendererInfo.CameraRayStartX.z*this->RendererInfo.CameraRayStartY.x-this->RendererInfo.CameraRayStartX.x*this->RendererInfo.CameraRayStartY.z;
    this->RendererInfo.ClippingPlaneNormal.z=this->RendererInfo.CameraRayStartX.x*this->RendererInfo.CameraRayStartY.y-this->RendererInfo.CameraRayStartX.y*this->RendererInfo.CameraRayStartY.x;
      
      
    float length= sqrt(this->RendererInfo.ClippingPlaneNormal.x*this->RendererInfo.ClippingPlaneNormal.x+
                       this->RendererInfo.ClippingPlaneNormal.y*this->RendererInfo.ClippingPlaneNormal.y+
                       this->RendererInfo.ClippingPlaneNormal.z*this->RendererInfo.ClippingPlaneNormal.z);
      
    this->RendererInfo.ClippingPlaneNormal.x/=length;
    this->RendererInfo.ClippingPlaneNormal.y/=length;
    this->RendererInfo.ClippingPlaneNormal.z/=length;
      
    double clipRange[2];
    cam->GetClippingRange(clipRange);
    this->RendererInfo.ClippingRange.x = (float)clipRange[0];
    this->RendererInfo.ClippingRange.y = (float)clipRange[1];

    this->RendererInfo.ZBufferA=clipRange[1]/(clipRange[1]-clipRange[0]);
    this->RendererInfo.ZBufferB=clipRange[1]*clipRange[0]/(clipRange[0]-clipRange[1]);
      
    // Update the Z-Buffer
    renWin->GetZbufferData(0,0,this->RendererInfo.ActualResolution.x-1, this->RendererInfo.ActualResolution.y-1, this->LocalZBuffer.GetMemPointerAs<float>());
    this->LocalZBuffer.CopyTo(&this->CudaZBuffer);
    this->RendererInfo.ZBuffer = CudaZBuffer.GetMemPointerAs<float>();
      
    if(this->Renderer->GetActiveCamera()->GetParallelProjection()){
      this->SetProjectionMethod(1);
    }else{
      this->SetProjectionMethod(0);
    }
  }
}
