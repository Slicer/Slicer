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

#include "vtkActor.h"
#include "vtkAxisActor2D.h"
#include "vtkCallbackCommand.h"
#include "vtkCamera.h"
#include "vtkCommand.h"
#include "vtkCornerAnnotation.h"
#include "vtkCutter.h"
#include "vtkFloatArray.h"
#include "vtkImageData.h"
#include "vtkInteractorStyleSwitch.h"
#include "vtkLookupTable.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPlane.h"
#include "vtkPointData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
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
#include "vtkKWCanvas.h"

#include "vtkImageDataGeometryFilter.h"
#include "vtkStripper.h"
#include "vtkCellArray.h"

#include "vtkSphereSource.h"

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
int vtkGaussian2DWidget::AddGaussian(
    double meanX,
    double meanY,
    double varianceX,
    double varianceY,
    double covariance,
    double hue)
{
  double sine    = 0.0;
  double cosine  = 1.0;
  double lambda1 = 1.0;
  double lambda2 = 1.0;

  vtkErrorMacro(
      << " varianceX " << varianceX
      << " varianceY " << varianceY
      << " covariance " << covariance);

  if(varianceX < 1e-6 || varianceY < 1e-6)
  {
    vtkErrorMacro("Variances must be greater than 1e-6. varianceX: " <<
        varianceX << " varianceY: " << varianceY);
    return -1;
  }

  double eigenVector[2];

  if(fabs(covariance) < 1e-6)
  {
    if(varianceX > varianceY)
    {
      lambda1 = varianceX;
      lambda2 = varianceY;

      eigenVector[0] = 1.0;
      eigenVector[1] = 0.0;

      sine   = 0.0;
      cosine = 1.0;
    }
    else
    {
      lambda1 = varianceY;
      lambda2 = varianceX;

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

    lambda1 = halfTrace + delta;
    lambda2 = halfTrace - delta;

    eigenVector[0] = lambda1 - varianceX;
    eigenVector[1] = covariance;

    vtkErrorMacro("eigen vector: "<< eigenVector[0] << " " <<
        eigenVector[1]);

    double diff1 = fabs(lambda1 - varianceX);
    double diff2 = fabs(lambda2 - varianceX);

    if (diff1 < 1e-6 ||
        diff2 < 1e-6 ||
        fabs(lambda1 - varianceY) < 1e-6 ||
        fabs(lambda2 - varianceY) < 1e-6 ||
        fabs(covariance) < 1e-6 * diff1  ||
        fabs(covariance) < 1e-6 * diff2)
    {
      if (varianceX > varianceY)
      {
        lambda1 = varianceX;
        lambda2 = varianceY;

        eigenVector[0] = 1.0;
        eigenVector[1] = 0.0;

        sine   = 0.0;
        cosine = 1.0;
      }
      else
      {
        lambda1 = varianceY;
        lambda2 = varianceX;

        eigenVector[0] = 0.0;
        eigenVector[1] = 1.0;

        sine   = 1.0;
        cosine = 0.0;
      }
    }
    else
    {
      double angle = atan(eigenVector[0] / covariance);

      vtkErrorMacro("angle " << angle * 180.0 / 3.14 );

      cosine = cos(angle);
      sine   = sin(angle);

      {
        angle = atan((lambda2 - varianceX) / covariance);

        vtkErrorMacro("angle " << angle * 180.0 / 3.14 << " alternatively");
      }
    }
  }

  int id = this->DrawGaussian(meanX,meanY,varianceX,varianceY,covariance,
    sine,cosine,lambda1,lambda2,hue);

  return id;
}

//----------------------------------------------------------------------------
int vtkGaussian2DWidget::DrawGaussian(
    double meanX,
    double meanY,
    double varianceX,
    double varianceY,
    double covariance,
    double vtkNotUsed(sine),
    double vtkNotUsed(cosine),
    double vtkNotUsed(lambda1),
    double vtkNotUsed(lambda2),
    double hue)
{
  if(!this->IsCreated())
  {
    vtkErrorMacro(<< this->GetClassName() << " not created");
    return -1;
  }

  if(this->Resolution[0] < 2 || this->Resolution[1] < 2)
  {
    vtkErrorMacro("Resolution " << this->Resolution[0] << " " <<
        this->Resolution[1]);
    return -1;
  }

  if(this->ScalarRangeX[1] <= this->ScalarRangeX[0])
  {
    vtkErrorMacro("ScalarRangeX must be a strictly increasing range of "
        << "intensity values. ScalarRangeX " << this->ScalarRangeX[0] << " "
        << this->ScalarRangeX[1]);
    return -1;
  }

  if(this->ScalarRangeY[1] <= this->ScalarRangeY[0])
  {
    vtkErrorMacro("ScalarRangeY must be a strictly increasing range of "
        << "intensity values. ScalarRangeY " << this->ScalarRangeY[0] << " "
        << this->ScalarRangeY[1]);
    return -1;
  }

  double range[2] = { VTK_DOUBLE_MAX, VTK_DOUBLE_MIN };

  //double x, y;
  double z;
  double dist;

  double originX = this->ScalarRangeX[0];
  double originY = this->ScalarRangeY[0];

  double spacingX = this->ScalarRangeX[1] - this->ScalarRangeX[0];
  double spacingY = this->ScalarRangeY[1] - this->ScalarRangeY[0];

  spacingX /= this->Resolution[0] - 1;
  spacingY /= this->Resolution[1] - 1;

  int numPts = this->Resolution[0] * this->Resolution[1];
#if 0
  int numStrips      = this->Resolution[1] - 1;
  int numPtsPerStrip = 2 * this->Resolution[0];
  int numCellPts     = numStrips * numPtsPerStrip;

  if(this->Strips->GetNumberOfCells() != numStrips)
  {
    this->Strips->Initialize();
    this->Strips->Allocate(numStrips + numCellPts);

    for(int j=0; j<numStrips; j++)
    {
      this->Strips->InsertNextCell(numPtsPerStrip);

      for(int i=0; i<this->Resolution[0]; i++)
      {
        this->Strips->InsertCellPoint(i);
        this->Strips->InsertCellPoint(i+this->Resolution[0]);
      }
    }
  }
#endif
  vtkImageData *imageData = vtkImageData::New();
  imageData->SetDimensions(this->Resolution[0],this->Resolution[1],1);

  vtkImageDataGeometryFilter *geometryFilter =
    vtkImageDataGeometryFilter::New();
  geometryFilter->SetInput(imageData);
  imageData->Delete();
  geometryFilter->Update();
/*
  vtkStripper *stripper = vtkStripper::New();
  stripper->SetInput(geometryFilter->GetOutput());
  stripper->Update();

  int numStrips = stripper->GetOutput()->GetNumberOfStrips();
  vtkErrorMacro("numStrips " << numStrips);

  vtkIdType npts;
  vtkIdType *pts;

  stripper->GetOutput()->GetStrips()->InitTraversal();

  while(stripper->GetOutput()->GetStrips()->GetNextCell(npts,pts))
  {
    std::cout<<"\nnpts "<<npts<<":";
    for(int i=0;i<npts;i++) { std::cout<<" "<<pts[i]; }
    std::cout << std::endl;
  }

  while(1) {}
*/
  vtkPoints *newPoints = vtkPoints::New();
  newPoints->SetNumberOfPoints(numPts);

  vtkFloatArray *newScalars = vtkFloatArray::New();
  newScalars->SetNumberOfValues(numPts);

  double sigmaX = sqrt(varianceX);
  double sigmaY = sqrt(varianceY);
  double rho    = covariance/(sigmaX*sigmaY);

  double factorX = 1/(varianceX);//fabs(lambda1) > 1e-6 ? 1.0 / lambda1 : 1.0;
  double factorY = 1/(varianceY);//fabs(lambda2) > 1e-6 ? 1.0 / lambda2 : 1.0;
  double factorRho = -2/(sigmaX*sigmaY);

  double factorExpo = -1/(2*(1-rho*rho));
  //double amplitude = 1/(2*3.14*sigmaX*sigmaY*sqrt(1-rho*rho));

  double Dx;
  double Dy;

  if(rho < 0.999)
  {
  //vtkErrorMacro("rho diff "
  //    << rho);
    for(int j=0, id=0; j < this->Resolution[1]; j++)
    {
      Dy = originY + j * spacingY - meanY;

      for(int i=0; i < this->Resolution[0]; i++, id++)
      {
        Dx = originX + i * spacingX - meanX;

        //x =  Dx * cosine + Dy *   sine;
        //y = -Dx *   sine + Dy * cosine;

        //dist = x * x * factorX + y * y * factorY;
        //z = exp(-dist);

        z = exp(factorExpo*(Dx*Dx*factorX+Dy*Dy*factorY+rho*Dx*Dy*factorRho));

        newPoints->SetPoint(id,i,j,z);
        newScalars->SetValue(id,z);

        if( z < range[0] ) { range[0] = z; }
        if( z > range[1] ) { range[1] = z; }
      }
    }
  }
  else
  {
    vtkErrorMacro("rho: " << rho);

    for(int j=0, id=0; j < this->Resolution[1]; j++)
    {
      Dy = originY + j * spacingY - meanY;

      for(int i=0; i < this->Resolution[0]; i++, id++)
      {
        Dx = originX + i * spacingX - meanX;

        //z= exp( (-1/2)*(Dx*Dx*factorX + Dy*Dy*factorY
        // /*+ rho*Dx*Dy*factorRho*/) );

        //cosine = 0.45*3.14/180;
        //sine = 0.45*3.14/180;

        //x =  Dx * cosine + Dy *   sine;
        //y = -Dx *   sine + Dy * cosine;

      dist = Dx * Dx * factorX/2 + Dy * Dy * factorY/2;
      z = exp(-dist*dist);

      newPoints->SetPoint(id,i,j,z);
      newScalars->SetValue(id,z);

      if( z < range[0] ) { range[0] = z; }
      if( z > range[1] ) { range[1] = z; }
      }
    }
  }

  vtkPolyData *polyData = vtkPolyData::New();
  polyData->SetPoints(newPoints);
  newPoints->Delete();
  polyData->SetPolys(geometryFilter->GetOutput()->GetPolys());
  geometryFilter->Delete();
#if 0
  int numPts = polyData->GetNumberOfPoints();
  int numCells = polyData->GetNumberOfCells();

  std::cout << *polyData;

  vtkErrorMacro("numPts " << numPts);
  vtkErrorMacro("numCells " << numCells);

  vtkPolyDataWriter *writer = vtkPolyDataWriter::New();
  writer->SetInput(polyData);
  writer->SetFileName("/Users/sylvain/polyData.vtk");
  writer->Write();
  writer->Delete();

  while(1) {}
#endif
  vtkPlane *plane = vtkPlane::New();
  plane->SetOrigin(meanX,meanY,0.9);
  plane->SetNormal(0,0,1);

  /*vtkPlane *planeX = vtkPlane::New();
  planeX->SetOrigin(meanX*this->Resolution[0],meanY*this->Resolution[0],0);
  planeX->SetNormal(-sine,cosine,0);

  vtkPlane *planeY = vtkPlane::New();
  planeY->SetOrigin(meanX*this->Resolution[0],meanY*this->Resolution[0],0);
  planeY->SetNormal(cosine,sine,0);*/

  vtkCutter *cutter = vtkCutter::New();
  cutter->SetInput(polyData);
  cutter->SetCutFunction(plane);
  plane->Delete();
  cutter->Update();

  /*vtkCutter *cutterX = vtkCutter::New();
  cutterX->SetInput(polyData);
  cutterX->SetCutFunction(planeX);
  planeX->Delete();
  cutterX->Update();

  vtkCutter *cutterY = vtkCutter::New();
  cutterY->SetInput(polyData);
  cutterY->SetCutFunction(planeY);
  planeY->Delete();
  cutterY->Update();*/

  vtkErrorMacro("numLines " <<
    cutter->GetOutput()->GetNumberOfLines());

 // if (cutter->GetOutput()->GetNumberOfLines() > 10)
 // {
    vtkPolyDataMapper *mapperZ = vtkPolyDataMapper::New();
    mapperZ->SetInput(cutter->GetOutput());
    mapperZ->SetResolveCoincidentTopologyToPolygonOffset();

    vtkActor *actorZ = vtkActor::New();
    actorZ->SetMapper(mapperZ);
    mapperZ->Delete();
    actorZ->GetProperty()->SetColor(0,0,0);

    this->AddViewProp(actorZ);
    actorZ->Delete();
 // }
  /*else
  {

  vtkSphereSource *newSphere = vtkSphereSource::New();
  newSphere->SetCenter(meanX*100,meanY*100,range[1]);

  vtkPolyDataMapper *mapper = vtkPolyDataMapper::New();
  mapper->SetInput(newSphere->GetOutput());

  double hsv[3] = {hue,1,1};
  double rgb[3];

  vtkMath::HSVToRGB(hsv,rgb);

  vtkErrorMacro("R "<< rgb[0]);

  //mapper->SetResolveCoincidentTopologyToPolygonOffset();

  vtkActor *actor = vtkActor::New();
  actor->SetMapper(mapper);
  mapper->Delete();
  actor->GetProperty()->SetColor(rgb);

  this->AddViewProp(actor);
  actor->Delete();

  newSphere->Delete();
  }*/

  cutter->Delete();

  polyData->GetPointData()->SetScalars(newScalars);
  newScalars->Delete();

  double center[3];
  double bounds[6];

  polyData->GetCenter(center);
  polyData->GetBounds(bounds);

  vtkErrorMacro("bounds"
    << " " << bounds[0] << " " << bounds[1]
    << " " << bounds[2] << " " << bounds[3]
    << " " << bounds[4] << " " << bounds[5]);

  vtkErrorMacro("center "
      << center[0] << " "
      << center[1] << " "
      << center[2]);

  vtkErrorMacro("range " << range[0] << " " << range[1]);

  vtkLookupTable *LUT = vtkLookupTable::New();
  LUT->SetTableRange(range);
  LUT->SetNumberOfTableValues(256);
  LUT->SetHueRange(hue,hue);
  LUT->SetSaturationRange(0,1);
  LUT->SetValueRange(0,1);
  LUT->Build();

  vtkPolyDataMapper *mapper = vtkPolyDataMapper::New();
  mapper->SetInput(polyData);
  polyData->Delete();
  mapper->SetLookupTable(LUT);
  LUT->Delete();
  mapper->SetScalarRange(range);

  /*vtkPolyDataMapper *mapperX = vtkPolyDataMapper::New();
  mapperX->SetInputConnection(cutterX->GetOutputPort());
  cutterX->Delete();
  mapperX->SetResolveCoincidentTopologyToPolygonOffset();
  mapperX->ScalarVisibilityOff();

  vtkPolyDataMapper *mapperY = vtkPolyDataMapper::New();
  mapperY->SetInputConnection(cutterY->GetOutputPort());
  cutterY->Delete();
  mapperY->SetResolveCoincidentTopologyToPolygonOffset();
  mapperY->ScalarVisibilityOff();*/

  vtkActor *actor = vtkActor::New();
  actor->SetMapper(mapper);
  mapper->Delete();

  /*vtkActor *actorX = vtkActor::New();
  actorX->SetMapper(mapperX);
  mapperX->Delete();
  actorX->GetProperty()->SetColor(0,0,0);

  vtkActor *actorY = vtkActor::New();
  actorY->SetMapper(mapperY);
  mapperY->Delete();
  actorY->GetProperty()->SetColor(0,0,0);*/

  this->AddViewProp(actor);
  actor->Delete();

  //this->AddViewProp(actorX);
  //actorX->Delete();

  //this->AddViewProp(actorY);
  //actorY->Delete();

  if (!this->NumberOfGaussians)
  {
    vtkAxisActor2D *XaxisActor2D = vtkAxisActor2D::New();
    XaxisActor2D->SetPoint1( 0.1, 0.1);
    XaxisActor2D->SetPoint2( 0.9, 0.1);

    vtkAxisActor2D *YaxisActor2D = vtkAxisActor2D::New();
    YaxisActor2D->SetPoint1( 0.1, 0.1);
    YaxisActor2D->SetPoint2( 0.1, 0.9);

    this->AddOverlayViewProp(XaxisActor2D);
    XaxisActor2D->Delete();
    this->AddOverlayViewProp(YaxisActor2D);
    YaxisActor2D->Delete();
  }
  /*
  vtkCamera *camera = vtkCamera::New();
  camera->SetPosition( center[0], center[1], 800.0 );
  camera->SetFocalPoint( center[0], center[1], 0.0 );
  camera->SetViewUp( 0, 1, 0 );
  camera->ParallelProjectionOn();

  this->GetRenderer()->SetActiveCamera(camera);
  camera->Delete();
  */
  this->Render();
  this->ResetCamera();
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

  os << indent << "Resolution "
    << this->Resolution[0] << " "
    << this->Resolution[1] << "\n";

  os << indent << "ScalarRangeX "
    << this->ScalarRangeX[0] << " "
    << this->ScalarRangeX[1] << "\n";

  os << indent << "ScalarRangeY "
    << this->ScalarRangeY[0] << " "
    << this->ScalarRangeY[1] << "\n";
}

