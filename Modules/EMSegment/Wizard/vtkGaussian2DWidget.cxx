/*=auto==============================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All
Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkGaussian2DWidget.cxx,v $
Date:      $Date: 2006/01/06 17:56:51 $
Version:   $Revision: 1.6 $
Author:    $Sylvain Jaume (MIT)$

==============================================================auto=*/

#include "vtkGaussian2DWidget.h"

#include "vtkCallbackCommand.h"
#include "vtkCamera.h"
#include "vtkCommand.h"
#include "vtkCornerAnnotation.h"
#include "vtkInteractorStyleSwitch.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkProperty2D.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkRendererCollection.h"
#include "vtkTextActor.h"
#include "vtkTextProperty.h"

#include "vtkKWApplication.h"
#include "vtkKWColorPickerWidget.h"
#include "vtkKWColorPickerDialog.h"
#include "vtkKWEvent.h"
#include "vtkKWGenericRenderWindowInteractor.h"
#include "vtkKWIcon.h"
#include "vtkKWInternationalization.h"
#include "vtkKWMenu.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWWindow.h"

#include "vtkImageEllipsoidSource.h"
#include "vtkImageDataGeometryFilter.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"

#include "vtkFloatArray.h"
#include "vtkDoubleArray.h"
#include "vtkImageData.h"
#include "vtkPointData.h"
#include "vtkLookupTable.h"
#include "vtkAxisActor2D.h"

#include "vtkKWCanvas.h"

#ifdef _WIN32
#include "vtkWin32OpenGLRenderWindow.h"
#endif

#include <vtksys/stl/string>
#include <vtksys/stl/vector>
#include <vtksys/stl/map>

vtkStandardNewMacro(vtkGaussian2DWidget);
vtkCxxRevisionMacro(vtkGaussian2DWidget, "$Revision: 1.170 $");

//-------------------------------------------------------------------
vtkGaussian2DWidget::vtkGaussian2DWidget()
{
  this->XAxisRange[0] = 0.0;
  this->XAxisRange[1] = 1.0;

  this->YAxisRange[0] = 0.0;
  this->YAxisRange[1] = 1.0;

  this->MeanXArray  = vtkDoubleArray::New();
  this->MeanYArray  = vtkDoubleArray::New();
  this->VarianceXArray = vtkDoubleArray::New();
  this->VarianceYArray = vtkDoubleArray::New();
  //this->CovarianceArray = vtkDoubleArray::New();
  this->RotationAngleArray = vtkDoubleArray::New();
  
  this->RGBArray = vtkDoubleArray::New();
  this->RGBArray->SetNumberOfComponents(3);
  
  this->NbOfGaussians = 0;
}

//-------------------------------------------------------------------
vtkGaussian2DWidget::~vtkGaussian2DWidget()
{
  this->MeanXArray->Delete();
  this->MeanXArray = NULL;

  this->MeanYArray->Delete();
  this->MeanYArray = NULL;

  this->VarianceXArray->Delete();
  this->VarianceXArray = NULL;
  
  this->VarianceYArray->Delete();
  this->VarianceYArray = NULL;
  
  //this->CovarianceArray->Delete();
  //this->CovarianceArray = NULL;
  
  this->RotationAngleArray->Delete();
  this->RotationAngleArray = NULL;

  this->RGBArray->Delete();
  this->RGBArray = NULL;
}

//-------------------------------------------------------------------
void vtkGaussian2DWidget::AddGaussian(double meanX, double meanY,
    double varianceX, double varianceY, double covariance, double rgb[3])
{
  std::cout << "AddGaussian meanX " << meanX << " meanY " << meanY
    << " varianceX " << varianceX << " varianceY " << varianceY << " covariance " << covariance << std::endl;
  
  this->MeanXArray->InsertNextValue(meanX);
  this->MeanYArray->InsertNextValue(meanY);
  this->VarianceXArray->InsertNextValue(varianceX);
  this->VarianceYArray->InsertNextValue(varianceY);
  //this->CovarianceArray->InsertNextValue(covariance);

  this->RGBArray->InsertNextTuple(rgb);
  
  //May be usefull for deleting only one gaussian
  //this->NbOfGaussians += this->NbOfGaussians;
  
  //std::cout << "Nb of gaussians: " << this->NbOfGaussians << std::endl;
  
  //Compute eigen vectors the rotation angle
  
  double rotationAngle;
  double eigenValues[2];
  double eigenVector[2];
  double eigenMax;
  
  double determinant;
     
  double a;
  double b;
  double c;
  
  double tanValue;
    
  //double maximumVarianceAxe;
  
  a = 1;
  b = -(varianceX+varianceY);
  c = varianceX*varianceY - (covariance)*(covariance);

  determinant = (b)*(b) - 4*a*c;

  if(determinant>=0)
    {
    eigenValues[0] = (-b+sqrt(determinant))/(2*a);
    eigenValues[1] = (-b-sqrt(determinant))/(2*a);
    }
  else
    {
    std::cout<<"Determinant NULL"<<std::endl;
    }
      
  if(eigenValues[0]>eigenValues[1])
    {
    eigenMax=eigenValues[0];
    //maximumVarianceAxe=1;
    }
  else
    {
    eigenMax=eigenValues[1];
    //maximumVarianceAxe=2;
    }
    
// calculating the eigenVector
  
  if(((varianceX-eigenMax)/(covariance)-(covariance)/(varianceY-eigenMax)) < 1e-5 && ((varianceX-eigenMax)/(covariance)-(covariance)/(varianceY-eigenMax)) > -1e-5)
    {
    eigenVector[0] = 1;
    eigenVector[1] = (varianceX-eigenMax)/(covariance);
    
    std::cout<<"Maximum Variance Eigen Vector: "<<eigenVector[0]<<" :: "<<eigenVector[1]<<std::endl;
    }
  else
    {
    std::cout<<"ERROR"<<std::endl;
    std::cout<<"Values: "<< (varianceX-eigenMax)/(covariance) <<" :: "<< (covariance)/(varianceY-eigenMax) <<std::endl;
    }
  
  tanValue = eigenVector[0]/eigenVector[1];
    
  rotationAngle = atan(tanValue);
  
  std::cout << "tan value" << tanValue << std::endl;   
  std::cout << "rotation angle" << rotationAngle << std::endl;  
  
  
  this->RotationAngleArray->InsertNextValue(rotationAngle);
}

//-------------------------------------------------------------------
void vtkGaussian2DWidget::RemoveAllGaussians()
{

  //Delete all the arrays

  this->MeanXArray->Delete();
  this->MeanXArray = NULL;

  this->MeanYArray->Delete();
  this->MeanYArray = NULL;

  this->VarianceXArray->Delete();
  this->VarianceXArray = NULL;
  
  this->VarianceYArray->Delete();
  this->VarianceYArray = NULL;
  
  this->RotationAngleArray->Delete();
  this->RotationAngleArray = NULL;

  this->RGBArray->Delete();
  this->RGBArray = NULL;
  
  //Remove all the actors
  
  this->RemoveAllViewProps();
  
  //Reinitialize the arrays and the number of gaussians
  
  this->MeanXArray  = vtkDoubleArray::New();
  this->MeanYArray  = vtkDoubleArray::New();
  this->VarianceXArray = vtkDoubleArray::New();
  this->VarianceYArray = vtkDoubleArray::New();
  this->RotationAngleArray = vtkDoubleArray::New();
  
  this->RGBArray = vtkDoubleArray::New();
  this->RGBArray->SetNumberOfComponents(3);
  
  this->NbOfGaussians = 0;
}

//-------------------------------------------------------------------
void vtkGaussian2DWidget::CreateGaussian2D()
{
  std::cout << "Create Gaussian 2D start" << std::endl;
  // Check if already created

  if (!this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " not created");
    return;
    }

  // Create a 2D Gaussian
  int dim[3] = { this->GetWidth(), this->GetHeight(), 1 };
  int numPts = dim[0] * dim[1];

  std::cout << "dim " << dim[0] << " " << dim[1] << " " << dim[2]
    << std::endl;

  if(dim[0] < 1) exit(0);
  if(dim[1] < 1) exit(0);
  if(dim[2] < 1) exit(0);

  double origin[3]  = { 0.0, 0.0, 0.0 };
  double spacing[3] = { 1.0, 1.0, 1.0 };

  vtkImageData *imageData = vtkImageData::New();
  imageData->SetWholeExtent( 0, dim[0]-1, 0, dim[1]-1, 0, 0 );
  imageData->SetExtent( 0, dim[0]-1, 0, dim[1]-1, 0, 0 );
  imageData->SetOrigin( origin );
  imageData->SetSpacing( spacing );
  imageData->SetScalarTypeToUnsignedChar();
  imageData->AllocateScalars();
  imageData->Update();

  vtkImageDataGeometryFilter *imageDataGeometryFilter =
    vtkImageDataGeometryFilter::New();
  imageDataGeometryFilter->SetInput(imageData);
  imageData->Delete();
  imageDataGeometryFilter->Update();

  int numMeanX = this->MeanXArray->GetNumberOfTuples();
  int numMeanY = this->MeanYArray->GetNumberOfTuples();
  int numVarianceX = this->VarianceXArray->GetNumberOfTuples();
  int numVarianceY = this->VarianceYArray->GetNumberOfTuples();

  std::cout << "numMeanX " << numMeanX << std::endl;
  std::cout << "numMeanY " << numMeanY << std::endl;
  std::cout << "numVarianceX " << numVarianceX << std::endl;
  std::cout << "numVarianceY " << numVarianceY << std::endl;

  if(numMeanX != numMeanY || numMeanY != numVarianceX || numVarianceX != numMeanX)
  {
    exit(0);
  }

  int numGaussians = numVarianceX;

  double x, y, dist;
  double meanX, meanY, varianceX, varianceY;
  double rotationAngle;
  double amplitudeGaussian[numGaussians];
  double amplitudeGaussianMax;

  double pt[3], range[2];
  double center[3], bounds[6];

  amplitudeGaussianMax = 0.0;

  // Get the maximum variances product throuhgh all the gaussians
  // to get an amplitude normalization factor

  for(int m=0; m<numGaussians; m++)
  {
    varianceX = this->VarianceXArray->GetValue(m)+1;
    varianceY = this->VarianceYArray->GetValue(m)+1;
    if(varianceX*varianceY > amplitudeGaussianMax)
    {
    amplitudeGaussianMax = varianceX*varianceY;
    }
  }
  
  for(int m=0; m<numGaussians; m++)
  {
    varianceX = this->VarianceXArray->GetValue(m)+1;
    varianceY = this->VarianceYArray->GetValue(m)+1;
    
    amplitudeGaussian[m] = 1;//(int)(sqrt(amplitudeGaussianMax)*10/sqrt(varianceX*varianceY) + 0.5);

  }
  

  for(int m=0; m<numGaussians; m++)
  {
    vtkPolyData *polyData = vtkPolyData::New();
    polyData->SetPolys(imageDataGeometryFilter->GetOutput()->GetPolys());

    vtkPoints *newPoints = vtkPoints::New();
    newPoints->SetNumberOfPoints(numPts);

    vtkFloatArray *newScalars = vtkFloatArray::New();
    newScalars->SetNumberOfValues(numPts);

    meanX = this->MeanXArray->GetValue(m);
    meanY = this->MeanYArray->GetValue(m);
    varianceX = this->VarianceXArray->GetValue(m);
    varianceY = this->VarianceYArray->GetValue(m);
    
    rotationAngle = this->RotationAngleArray->GetValue(m);
    
    // Normalized amplitude of the gaussian
    
    //amplitudeGaussian = (varianceX*varianceY)/amplitudeGaussianMax;
    
    std::cout << "Gaussian Ampli: " << amplitudeGaussian[m] << std::endl;

    range[0] = VTK_DOUBLE_MAX;
    range[1] = VTK_DOUBLE_MIN;

    pt[0] = 0.0;
    pt[1] = 0.0;
    pt[2] = 0.0;

    for(int j=0, id=0; j<dim[1]; j++)
    {
      pt[1] = origin[1] + j * spacing[1];

      for(int i=0; i<dim[0]; i++, id++)
      {
        pt[0] = origin[0] + i * spacing[0];
        
        // apply the rotation
        x = ((pt[0]-meanX)*cos(rotationAngle) + (pt[1]-meanY)*sin(rotationAngle));
        y = (-(pt[0]-meanX)*sin(rotationAngle) + (pt[1]-meanY)*cos(rotationAngle));
               
        dist = x*x/varianceX + y*y/varianceY;
       
        pt[2] = exp(-dist);
               
        //if( fabs(pt[2]) < 1e-10 ) { pt[2] = 0.0; }

        newPoints->SetPoint( id, pt );
        newScalars->SetValue( id, pt[2] );

        if( pt[2] < range[0] ) { range[0] = pt[2]; }
        if( pt[2] > range[1] ) { range[1] = pt[2]; }
      }
    }

    polyData->SetPoints(newPoints);
    newPoints->Delete();

    //vtkCutter *cutter = vtkCutter::New();
    //cutter->SetInput(polyData);
    //cutter->SetNumberOfContours(10);

    polyData->GetPointData()->SetScalars(newScalars);
    newScalars->Delete();

  polyData->GetCenter(center);
  polyData->GetBounds(bounds);

  std::cout << "bounds"
    << " " << bounds[0] << " " << bounds[1]
    << " " << bounds[2] << " " << bounds[3]
    << " " << bounds[4] << " " << bounds[5]
    << std::endl;

  std::cout << "center " << center[0] << " " << center[1] << " "
    << center[2] << std::endl;
    
  std::cout << "range " << range[0] << " " << range[1] << std::endl;
  
  vtkLookupTable *LUT = vtkLookupTable::New();
  //range[0] = range[0]-1;
  LUT->SetTableRange(range);
  LUT->SetNumberOfTableValues(4096);

  double rgb[3];
  this->RGBArray->GetTuple(m,rgb);

//Convert RGB to Hue
  double hue;
  double rgbMax;
  
  if(rgb[0]>rgb[1])
    {
    if(rgb[0]>rgb[2])
      {
      rgbMax = rgb[0];
      }
    else
      {
      rgbMax = rgb[2];
      }
    }
  else
    {
    if(rgb[1]>rgb[2])
      {
      rgbMax = rgb[1];
      }
    else
      {
      rgbMax = rgb[2];
      }
    }

  if (rgbMax == rgb[0])
    {
    hue = (0.0 + 60.0*(rgb[1] - rgb[2]))/360.0;
    if (hue < 0.0)
      {
      hue += 1;
      }
    LUT->SetHueRange(hue,hue);
    }
  else if (rgbMax == rgb[1])
    {
    hue = (120.0 + 60.0*(rgb[2] - rgb[0]))/360.0;
    LUT->SetHueRange(hue,hue);
    }
  else /* rgbMax == rgb.b */ 
    {
    hue = (240.0 + 60.0*(rgb[0] - rgb[1]))/360.0;
    LUT->SetHueRange(hue,hue);
    }
  
  std::cout << "hue " << hue << std::endl;
  
  LUT->SetSaturationRange(1,0);
  LUT->SetValueRange(1,1);
  LUT->Build();
/*
  for(int i=0; i<256; i+=10)
  {
    LUT->SetTableValue(i,0,0,0);
  }*/
  
  LUT->SetTableValue(0,0,0,0);

  vtkPolyDataMapper *polyDataMapper = vtkPolyDataMapper::New();
  polyDataMapper->SetInput(polyData);
  polyData->Delete();
  polyDataMapper->SetLookupTable(LUT);
  LUT->Delete();
  polyDataMapper->SetScalarRange(range);

  //vtkPolyDataMapper *cutterMapper = vtkPolyDataMapper::New();
  //cutterMapper->SetInputConnection(cutter->GetOutputPort());
  //cutter->Delete();

  vtkActor *actor = vtkActor::New();
  actor->SetMapper(polyDataMapper);
  polyDataMapper->Delete();

  //vtkActor *cutterActor = vtkActor::New();
  //cutterActor->SetMapper(cutterMapper);
  //cutterMapper->Delete();

  this->AddViewProp(actor);
  actor->Delete();

  //this->AddViewProp(cutterActor);
  //cutterActor->Delete();
  }

  imageDataGeometryFilter->Delete();

  //vtkImageViewer2 *imageViewer2 = vtkImageViewer2::New();
  //imageViewer2->SetInput(imageData);
  //imageData->Delete();
  //imageViewer2->SetRenderWindow(renderWidget->GetRenderWindow());
  //imageViewer2->SetRenderer(renderWidget->GetRenderer());
  //imageViewer2->SetupInteractor(
  //renderWidget->GetRenderWindow()->GetInteractor());
  //imageViewer2->SetColorWindow(range[1] - range[0]);
  //imageViewer2->SetColorLevel(0.5 * (range[1] + range[0]));

  //imageViewer2->GetWindowLevel()->SetLookupTable(LUT);
  //LUT->Delete();

  // TODO: Destructor of this class must have imageViewer2->Delete();

  vtkAxisActor2D *XaxisActor2D = vtkAxisActor2D::New();
  //XaxisActor2D->SetTitle("Volume 1");
  XaxisActor2D->SetPoint1( 0.1, 0.1 );
  XaxisActor2D->SetPoint2( 0.9, 0.1 );

  vtkAxisActor2D *YaxisActor2D = vtkAxisActor2D::New();
  //YaxisActor2D->SetTitle("Volume 2");
  YaxisActor2D->SetPoint1( 0.1, 0.1 );
  YaxisActor2D->SetPoint2( 0.1, 0.9 );

  this->AddOverlayViewProp(XaxisActor2D);
  XaxisActor2D->Delete();
  this->AddOverlayViewProp(YaxisActor2D);
  YaxisActor2D->Delete();
/*
  ClippingRange: (851.976, 875.25)
  DirectionOfProjection: (0, 0, -1)
  Distance: 861.36
  EyeAngle: 2
  FocalDisk: 1
  FocalPoint: (199.5, 99.5, 0.611278)
  ViewShear: (0, 0, 1)
  ParallelProjection: On
  ParallelScale: 222.936
  Position: (199.5, 99.5, 861.971)
  Stereo: Off
  Left Eye: 1
  Thickness: 23.2742
  ViewAngle: 30
  UseHorizontalViewAngle: 0
  UserTransform: (none)
  ViewPlaneNormal: (-0, -0, 1)
  ViewUp: (0, 1, 0)
  WindowCenter: (0, 0)
*/
  vtkCamera *camera = vtkCamera::New();
  camera->SetPosition( center[0], center[1], 800.0 );
  camera->SetFocalPoint( center[0], center[1], 0.0 );
  camera->SetViewUp( 0, 1, 0 );
  camera->ParallelProjectionOn();
  this->ResetCamera();
  //this->GetRenderer()->SetActiveCamera(camera);
  camera->Delete();
  this->Render();

  //std::cout << *this->GetRenderer()->GetActiveCamera();
  std::cout << "Create Gaussian 2D end" << std::endl;
}

//-------------------------------------------------------------------
void vtkGaussian2DWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

