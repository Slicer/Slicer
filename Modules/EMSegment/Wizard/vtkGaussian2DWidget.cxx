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
  this->Sigma2Array = vtkDoubleArray::New();

  this->RGBArray = vtkDoubleArray::New();
  this->RGBArray->SetNumberOfComponents(3);
}

//-------------------------------------------------------------------
vtkGaussian2DWidget::~vtkGaussian2DWidget()
{
  this->MeanXArray->Delete();
  this->MeanXArray = NULL;

  this->MeanYArray->Delete();
  this->MeanYArray = NULL;

  this->Sigma2Array->Delete();
  this->Sigma2Array = NULL;

  this->RGBArray->Delete();
  this->RGBArray = NULL;
}

//-------------------------------------------------------------------
void vtkGaussian2DWidget::AddGaussian(double meanX, double meanY,
    double sigma2, double rgb[3])
{
  std::cout << "AddGaussian meanX " << meanX << " meanY " << meanY
    << " sigma2 " << sigma2 << std::endl;

  this->MeanXArray->InsertNextValue(meanX);
  this->MeanYArray->InsertNextValue(meanY);
  this->Sigma2Array->InsertNextValue(sigma2);

  this->RGBArray->InsertNextTuple(rgb);
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
  int numSigma2 = this->Sigma2Array->GetNumberOfTuples();

  std::cout << "numMeanX " << numMeanX << std::endl;
  std::cout << "numMeanY " << numMeanY << std::endl;
  std::cout << "numSigma2 " << numSigma2 << std::endl;

  if(numMeanX != numMeanY || numMeanY != numSigma2 || numSigma2 != numMeanX)
  {
    exit(0);
  }

  int numGaussians = numSigma2;

  double x, y, dist;
  double meanX, meanY, sigma2;

  double pt[3], range[2];
  double center[3], bounds[6];

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
    sigma2 = this->Sigma2Array->GetValue(m);

    range[0] = VTK_DOUBLE_MAX;
    range[1] = VTK_DOUBLE_MIN;

    pt[0] = 0.0;
    pt[1] = 0.0;
    pt[2] = 0.0;

    for(int j=0, id=0; j<dim[1]; j++)
    {
      pt[1] = origin[1] + j * spacing[1];

      y = pt[1] - meanY;
      y *= y;

      for(int i=0; i<dim[0]; i++, id++)
      {
        pt[0] = origin[0] + i * spacing[0];
        x = pt[0] - meanX;

        x *= x;
        dist = x + y;

        pt[2] = exp( -dist / sigma2 );
        if( fabs(pt[2]) < 1e-9 ) { pt[2] = 0.0; }

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

  vtkLookupTable *LUT = vtkLookupTable::New();
  LUT->SetTableRange(range);
  LUT->SetNumberOfTableValues(256);

  double rgb[3];
  this->RGBArray->GetTuple(m,rgb);

  double hue = 0.0;

  if(rgb[1] == 1.0)
  {
    hue = 0.333;
  }
  else if(rgb[2] == 1.0)
  {
    hue = 0.667;
  }

  LUT->SetHueRange(hue,hue);
  LUT->SetSaturationRange(1,0);
  LUT->SetValueRange(1,1);
  LUT->Build();

  for(int i=0; i<256; i+=10)
  {
    LUT->SetTableValue(i,0,0,0);
  }

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

  std::cout << *this->GetRenderer()->GetActiveCamera();
  std::cout << "Create Gaussian 2D end" << std::endl;
}

//-------------------------------------------------------------------
void vtkGaussian2DWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

