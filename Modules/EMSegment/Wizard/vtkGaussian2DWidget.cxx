/*=auto=======================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights
  Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkGaussian2DWidget.cxx,v $
  Date:      $Date: 2006/01/06 17:56:51 $
  Version:   $Revision: 1.6 $
  Author:    $Sylvain Jaume (MIT), Nicolas Rannou (BWH)$

=======================================================================auto=*/

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

#include "vtkCellArray.h"
#include "vtkImageDataGeometryFilter.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"

#include "vtkFloatArray.h"
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

//----------------------------------------------------------------------------
vtkGaussian2DWidget::vtkGaussian2DWidget()
{
  this->Resolution[0] = 100;
  this->Resolution[1] = 100;

  this->ScalarRangeX[0] = 0.0;
  this->ScalarRangeX[1] = 1.0;

  this->ScalarRangeY[0] = 0.0;
  this->ScalarRangeY[1] = 1.0;

  this->NumberOfGaussians = 0;
}

//----------------------------------------------------------------------------
vtkGaussian2DWidget::~vtkGaussian2DWidget()
{
}

//----------------------------------------------------------------------------
int vtkGaussian2DWidget::AddGaussian(double meanX, double meanY,
    double varianceX, double varianceY, double covariance, double hue)
{
  vtkErrorMacro("AddGaussian meanX " << meanX << " meanY " << meanY <<
      " varianceX " << varianceX << " varianceY " << varianceY <<
      " covariance " << covariance);

  if (!this->IsCreated())
  {
    vtkErrorMacro(<< this->GetClassName() << " not created");
    return -1;
  }

  if (this->Resolution[0] < 2 || this->Resolution[1] < 2)
  {
    vtkErrorMacro("Resolution " << this->Resolution[0] << " " <<
        this->Resolution[1]);
    return -1;
  }

  if (this->ScalarRangeX[1] <= this->ScalarRangeX[0])
  {
    vtkErrorMacro("ScalarRangeX must be a strictly increasing range of "
        << "intensity values. ScalarRangeX " << this->ScalarRangeX[0] << " "
        << this->ScalarRangeX[1]);
    return -1;
  }

  if (this->ScalarRangeY[1] <= this->ScalarRangeY[0])
  {
    vtkErrorMacro("ScalarRangeY must be a strictly increasing range of "
        << "intensity values. ScalarRangeY " << this->ScalarRangeY[0] << " "
        << this->ScalarRangeY[1]);
    return -1;
  }

  if (varianceX < 1e-6 || varianceY < 1e-6)
  {
    vtkErrorMacro("Variances must be greater than 1e-6. varianceX: " <<
        varianceX << " varianceY: " << varianceY);
    return -1;
  }

  if (covariance > varianceX || covariance > varianceY)
  {
    vtkErrorMacro("Covariance must be smaller than the variances. "
        << "varianceX: " << varianceX << " varianceY: " << varianceY
        << "covariance " << covariance);
    return -1;
  }

  // Compute the main eigenvector and the rotation angle

  double eigenValues[2];
  double eigenVector[2];

  double sine, cosine;

  if (covariance < 1e-6)
  {
    if (varianceX > varianceY)
    {
      eigenValues[0] = varianceX;
      eigenValues[1] = varianceY;

      eigenVector[0] = 1.0;
      eigenVector[1] = 0.0;

      sine   = 0.0;
      cosine = 1.0;
    }
    else
    {
      eigenValues[0] = varianceY;
      eigenValues[1] = varianceX;

      eigenVector[0] = 0.0;
      eigenVector[1] = 1.0;

      sine   = 1.0;
      cosine = 0.0;
    }
  }
  else
  {
    double halfTrace   = 0.5 * ( varianceX + varianceY );
    double determinant = varianceX * varianceY - covariance * covariance;

    double d = halfTrace * halfTrace - determinant;
    double delta = d > 0.0 ? sqrt(d) : 0.0;

    eigenValues[0] = halfTrace + delta;
    eigenValues[1] = halfTrace - delta;

    // compute the eigenvector
    // double ratio1 = ( varianceX - eigenValues[0] ) / covariance;
    // double ratio2 = covariance / ( varianceY - eigenValues[0] );

    // (varianceX - l1) * (varianceY - l1)  = covariance * covariance
    // (fabs(ratio1 - ratio2) < 1e-5)

    eigenVector[0] = covariance;
    eigenVector[1] = varianceX - eigenValues[0];

    vtkErrorMacro("eigen vector: "<< eigenVector[0] << " " <<
        eigenVector[1]);

    // eigenValues[0] - varianceY
    // covariance

    double tangent = covariance / eigenVector[1];
    double rotationAngle = atan(tangent);

    cosine = cos( rotationAngle );
    sine   = sin( rotationAngle );
  }

  double pt[3]    = { 0.0, 0.0, 0.0 };
  double range[2] = { VTK_DOUBLE_MAX, VTK_DOUBLE_MIN };

  double x, y;
  double dist;

  double center[3];
  double bounds[6];

  int extent[6] = { 0,0, 0,0, 0,0 };

  extent[1] = this->Resolution[0] - 1;
  extent[3] = this->Resolution[1] - 1;

  double origin[3]  = { 0.0, 0.0, 0.0 };
  double spacing[3] = { 1.0, 1.0, 1.0 };

  origin[0] = this->ScalarRangeX[0];
  origin[1] = this->ScalarRangeY[0];

  spacing[0] = (this->ScalarRangeX[1] - this->ScalarRangeX[0]) / extent[1];
  spacing[1] = (this->ScalarRangeY[1] - this->ScalarRangeY[0]) / extent[3];

  vtkImageData *imageData = vtkImageData::New();
  imageData->SetWholeExtent(extent);
  imageData->SetExtent(extent);
  imageData->SetOrigin(origin);
  imageData->SetSpacing(spacing);
  imageData->SetScalarTypeToFloat();
  imageData->AllocateScalars();

  vtkImageDataGeometryFilter *imageDataGeometryFilter =
    vtkImageDataGeometryFilter::New();
  imageDataGeometryFilter->SetInput(imageData);
  imageData->Delete();
  imageDataGeometryFilter->Update();

  vtkCellArray *newPolys = imageDataGeometryFilter->GetOutput()->GetPolys();
  newPolys->Register(NULL);

  vtkPoints *newPoints = imageDataGeometryFilter->GetOutput()->GetPoints();
  newPoints->Register(NULL);

  vtkFloatArray *newScalars = vtkFloatArray::SafeDownCast(
      imageDataGeometryFilter->GetOutput()->GetPointData()->GetScalars());
  newScalars->Register(NULL);

  imageDataGeometryFilter->Delete();

  double factorX = 1.0 /varianceX;
  double factorY = 1.0 /varianceY;

  

  for(int j=0, id=0; j<=extent[3]; j++)
  {
    pt[1]  = origin[1] + j * spacing[1];
    //pt[1] -= meanY;

    for(int i=0; i<=extent[1]; i++, id++)
    {
      pt[0]  = origin[0] + i * spacing[0];
      //pt[0] -= meanX;

      // apply the rotation
      x =  (pt[0]-meanX) * cosine + (pt[1]-meanY) *   sine;
      y = -(pt[0]-meanX) *   sine + (pt[1]-meanY) * cosine;

    //  x *= factorX;
    //  y *= factorY;

      dist = x*x*factorX + y*y*factorY;
      pt[2] = exp(-dist);

      newPoints->SetPoint(  id, pt );
      newScalars->SetValue( id, pt[2] );

      if( pt[2] < range[0] ) { range[0] = pt[2]; }
      if( pt[2] > range[1] ) { range[1] = pt[2]; }
    }
  }

  vtkPolyData *polyData = vtkPolyData::New();
  polyData->SetPoints(newPoints);
  newPoints->Delete();
  polyData->SetPolys(newPolys);
  newPolys->Delete();
  polyData->GetPointData()->SetScalars(newScalars);
  newScalars->Delete();

  //vtkCutter *cutter = vtkCutter::New();
  //cutter->SetInput(polyData);
  //cutter->SetNumberOfContours(10);

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
  LUT->SetNumberOfTableValues(256);
  LUT->SetHueRange(hue,hue);
  LUT->SetSaturationRange(1,0);
  LUT->SetValueRange(1,1);
  LUT->Build();
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
/*
  imageDataGeometryFilter->Delete();

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

  vtkCamera *camera = vtkCamera::New();
  camera->SetPosition( center[0], center[1], 800.0 );
  camera->SetFocalPoint( center[0], center[1], 0.0 );
  camera->SetViewUp( 0, 1, 0 );
  camera->ParallelProjectionOn();*/
  this->ResetCamera();
  //this->GetRenderer()->SetActiveCamera(camera);
 // camera->Delete();
  this->Render();

  return this->NumberOfGaussians++;
  }

//----------------------------------------------------------------------------
void vtkGaussian2DWidget::ShowGaussian(int gaussianID)
{
  vtkActorCollection *actors = this->GetRenderer()->GetActors();

  vtkObject *object = actors->GetItemAsObject(gaussianID);
  vtkActor  *actor  = vtkActor::SafeDownCast(object);

  if (actor)
  {
    actor->VisibilityOn();
  }
}

//----------------------------------------------------------------------------
void vtkGaussian2DWidget::HideGaussian(int gaussianID)
{
  vtkActorCollection *actors = this->GetRenderer()->GetActors();

  vtkObject *object = actors->GetItemAsObject(gaussianID);
  vtkActor  *actor  = vtkActor::SafeDownCast(object);

  if (actor)
  {
    actor->VisibilityOff();
  }
}

//----------------------------------------------------------------------------
void vtkGaussian2DWidget::RemoveAllGaussians()
{
  // Remove all the actors
  this->RemoveAllViewProps();

  // Reset the number of gaussians
  this->NumberOfGaussians = 0;
}

//----------------------------------------------------------------------------
void vtkGaussian2DWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Resolution " << this->Resolution[0] << " "
    << this->Resolution[1] << "\n";

  os << indent << "ScalarRangeX " << this->ScalarRangeX[0] << " "
    << this->ScalarRangeX[1] << "\n";

  os << indent << "ScalarRangeY " << this->ScalarRangeY[0] << " "
    << this->ScalarRangeY[1] << "\n";
}

