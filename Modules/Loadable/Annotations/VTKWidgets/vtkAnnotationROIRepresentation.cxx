/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkAnnotationROIRepresentation.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

// Annotations includes
#include "vtkAnnotationROIRepresentation.h"

// VTK includes
#include "vtkActor.h"
#include "vtkAssemblyPath.h"
#include "vtkBox.h"
#include "vtkCallbackCommand.h"
#include "vtkCamera.h"
#include "vtkCellArray.h"
#include "vtkCellPicker.h"
#include "vtkDoubleArray.h"
#include "vtkInteractorObserver.h"
#include "vtkMath.h"
#include "vtkMatrix4x4.h"
#include "vtkObjectFactory.h"
#include "vtkPlanes.h"
#include "vtkPolyData.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkSphereSource.h"
#include "vtkTransform.h"
#include <vtkVersion.h>
#include "vtkWindow.h"


vtkStandardNewMacro(vtkAnnotationROIRepresentation);

//----------------------------------------------------------------------------
vtkAnnotationROIRepresentation::vtkAnnotationROIRepresentation()
{
  // The initial state
  this->InteractionState = vtkAnnotationROIRepresentation::Outside;

  // Handle size is in pixels for this widget
  this->HandleSize = 5.0;

  // Control orientation of normals
  this->InsideOut = 0;
  this->OutlineFaceWires = 0;
  this->OutlineCursorWires = 1;

  // Set up the initial properties
  this->CreateDefaultProperties();

  // Construct the poly data representing the hex
  this->HexPolyData = vtkPolyData::New();
  this->HexMapper = vtkPolyDataMapper::New();
  this->HexMapper->SetInputData(HexPolyData);
  this->HexActor = vtkActor::New();
  this->HexActor->SetMapper(this->HexMapper);
  this->HexActor->SetProperty(this->OutlineProperty);

  // Construct initial points
  this->Points = vtkPoints::New(VTK_DOUBLE);
  this->Points->SetNumberOfPoints(15);//8 corners; 6 faces; 1 center
  this->HexPolyData->SetPoints(this->Points);

  // Construct connectivity for the faces. These are used to perform
  // the picking.
  int i;
  vtkIdType pts[4];
  vtkCellArray *cells = vtkCellArray::New();
  cells->Allocate(cells->EstimateSize(6,4));
  pts[0] = 3; pts[1] = 0; pts[2] = 4; pts[3] = 7;
  cells->InsertNextCell(4,pts);
  pts[0] = 1; pts[1] = 2; pts[2] = 6; pts[3] = 5;
  cells->InsertNextCell(4,pts);
  pts[0] = 0; pts[1] = 1; pts[2] = 5; pts[3] = 4;
  cells->InsertNextCell(4,pts);
  pts[0] = 2; pts[1] = 3; pts[2] = 7; pts[3] = 6;
  cells->InsertNextCell(4,pts);
  pts[0] = 0; pts[1] = 3; pts[2] = 2; pts[3] = 1;
  cells->InsertNextCell(4,pts);
  pts[0] = 4; pts[1] = 5; pts[2] = 6; pts[3] = 7;
  cells->InsertNextCell(4,pts);
  this->HexPolyData->SetPolys(cells);
  cells->Delete();
  this->HexPolyData->BuildCells();

  // The face of the hexahedra
  cells = vtkCellArray::New();
  cells->Allocate(cells->EstimateSize(1,4));
  cells->InsertNextCell(4,pts); //temporary, replaced later
  this->HexFacePolyData = vtkPolyData::New();
  this->HexFacePolyData->SetPoints(this->Points);
  this->HexFacePolyData->SetPolys(cells);
  this->HexFaceMapper = vtkPolyDataMapper::New();
  this->HexFaceMapper->SetInputData(HexFacePolyData);
  this->HexFace = vtkActor::New();
  this->HexFace->SetMapper(this->HexFaceMapper);
  this->HexFace->SetProperty(this->FaceProperty);
  cells->Delete();

  // Create the outline for the hex
  this->OutlinePolyData = vtkPolyData::New();
  this->OutlinePolyData->SetPoints(this->Points);
  this->OutlineMapper = vtkPolyDataMapper::New();
  this->OutlineMapper->SetInputData(this->OutlinePolyData);
  this->HexOutline = vtkActor::New();
  this->HexOutline->SetMapper(this->OutlineMapper);
  this->HexOutline->SetProperty(this->OutlineProperty);
  cells = vtkCellArray::New();
  cells->Allocate(cells->EstimateSize(15,2));
  this->OutlinePolyData->SetLines(cells);
  cells->Delete();

  // Create the outline
  this->GenerateOutline();

  // Create the handles
  this->Handle = new vtkActor* [7];
  this->HandleMapper = new vtkPolyDataMapper* [7];
  this->HandleGeometry = new vtkSphereSource* [7];
  for (i=0; i<7; i++)
    {
    this->HandleGeometry[i] = vtkSphereSource::New();
    this->HandleGeometry[i]->SetThetaResolution(16);
    this->HandleGeometry[i]->SetPhiResolution(8);
    this->HandleMapper[i] = vtkPolyDataMapper::New();
    this->HandleMapper[i]->SetInputConnection(this->HandleGeometry[i]->GetOutputPort());
    this->Handle[i] = vtkActor::New();
    this->Handle[i]->SetProperty(this->HandleProperties[i]);
    this->Handle[i]->SetMapper(this->HandleMapper[i]);
    }

  // Define the point coordinates
  double bounds[6];
  bounds[0] = -0.5;
  bounds[1] = 0.5;
  bounds[2] = -0.5;
  bounds[3] = 0.5;
  bounds[4] = -0.5;
  bounds[5] = 0.5;
  // Points 8-14 are down by PositionHandles();
  this->BoundingBox = vtkBox::New();
  this->PlaceWidget(bounds);

  //Manage the picking stuff
  this->HandlePicker = vtkCellPicker::New();
  this->HandlePicker->SetTolerance(0.001);
  for (i=0; i<7; i++)
    {
    this->HandlePicker->AddPickList(this->Handle[i]);
    }
  this->HandlePicker->PickFromListOn();

  //this->HexPicker = vtkCellPicker::New();
  //this->HexPicker->SetTolerance(0.001);
  //this->HexPicker->AddPickList(HexActor);
  //this->HexPicker->PickFromListOn();

  this->CurrentHandle = nullptr;

  // Internal data members for performance
  this->Transform = vtkTransform::New();
  this->PlanePoints = vtkPoints::New(VTK_DOUBLE);
  this->PlanePoints->SetNumberOfPoints(6);
  this->PlaneNormals = vtkDoubleArray::New();
  this->PlaneNormals->SetNumberOfComponents(3);
  this->PlaneNormals->SetNumberOfTuples(6);
  this->Matrix = vtkMatrix4x4::New();

  this->WorldToLocalMatrix = vtkMatrix4x4::New();
  this->WorldToLocalMatrix->Identity();
}

//----------------------------------------------------------------------------
vtkAnnotationROIRepresentation::~vtkAnnotationROIRepresentation()
{
  this->HexActor->Delete();
  this->HexMapper->Delete();
  this->HexPolyData->Delete();
  this->Points->Delete();

  this->HexFace->Delete();
  this->HexFaceMapper->Delete();
  this->HexFacePolyData->Delete();

  this->HexOutline->Delete();
  this->OutlineMapper->Delete();
  this->OutlinePolyData->Delete();

  for (int i=0; i<7; i++)
    {
    this->HandleGeometry[i]->Delete();
    this->HandleMapper[i]->Delete();
    this->Handle[i]->Delete();
    }
  delete [] this->Handle;
  delete [] this->HandleMapper;
  delete [] this->HandleGeometry;

  this->HandlePicker->Delete();
  //  this->HexPicker->Delete();

  this->Transform->Delete();
  this->BoundingBox->Delete();
  this->PlanePoints->Delete();
  this->PlaneNormals->Delete();
  this->Matrix->Delete();

  //  this->HandleProperty->Delete();
 for(int i=0;i<NUMBER_HANDLES;i++)
  {
      this->HandleProperties[i]->Delete();
      this->HandleProperties[i]=nullptr;
  }

  this->SelectedHandleProperty->Delete();
  this->FaceProperty->Delete();
  this->SelectedFaceProperty->Delete();
  this->OutlineProperty->Delete();
  this->SelectedOutlineProperty->Delete();

  this->WorldToLocalMatrix->Delete();

}

//----------------------------------------------------------------------
void vtkAnnotationROIRepresentation::GetActors(vtkPropCollection *actors)
{
  actors->RemoveAllItems();
  actors->AddItem(this->HexActor);
  actors->AddItem(this->HexFace);
  actors->AddItem(this->HexOutline);

  for (int i=0; i<7; i++)
    {
    actors->AddItem(this->Handle[i]);
    }
}


//----------------------------------------------------------------------
void vtkAnnotationROIRepresentation::GetPolyData(vtkPolyData *pd)
{
  pd->SetPoints(this->HexPolyData->GetPoints());
  pd->SetPolys(this->HexPolyData->GetPolys());
}

//----------------------------------------------------------------------
void vtkAnnotationROIRepresentation::StartWidgetInteraction(double e[2])
{
  // Store the start position
  this->StartEventPosition[0] = e[0];
  this->StartEventPosition[1] = e[1];
  this->StartEventPosition[2] = 0.0;

  // Store the start position
  this->LastEventPosition[0] = e[0];
  this->LastEventPosition[1] = e[1];
  this->LastEventPosition[2] = 0.0;

  this->ComputeInteractionState(static_cast<int>(e[0]),static_cast<int>(e[1]),0);
}

//----------------------------------------------------------------------
void vtkAnnotationROIRepresentation::WidgetInteraction(double e[2])
{
  // Convert events to appropriate coordinate systems
  vtkCamera *camera = this->Renderer->IsActiveCameraCreated() ? this->Renderer->GetActiveCamera() : nullptr;
  if ( !camera )
    {
    return;
    }
  double focalPoint[4], pickPoint[4], prevPickPoint[4];
  double pickPointWorld[4], prevPickPointWorld[4];
  double z, vpn[3];
  camera->GetViewPlaneNormal(vpn);

  // Compute the two points defining the motion vector
  double pos[3];
  if ( this->LastPicker == this->HandlePicker)
    {
    this->HandlePicker->GetPickPosition(pos);
    vtkInteractorObserver::ComputeWorldToDisplay(this->Renderer,
                                                 pos[0], pos[1], pos[2],
                                                 focalPoint);
    }
  z = focalPoint[2];
  vtkInteractorObserver::ComputeDisplayToWorld(this->Renderer,this->LastEventPosition[0],
                                               this->LastEventPosition[1], z, prevPickPointWorld);
  vtkInteractorObserver::ComputeDisplayToWorld(this->Renderer, e[0], e[1], z, pickPointWorld);

  this->WorldToLocalMatrix->MultiplyPoint(prevPickPointWorld, prevPickPoint);
  this->WorldToLocalMatrix->MultiplyPoint(pickPointWorld, pickPoint);


  // Process the motion
  if ( this->InteractionState == vtkAnnotationROIRepresentation::MoveF0 )
    {
    this->MoveMinusXFace(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkAnnotationROIRepresentation::MoveF1 )
    {
    this->MovePlusXFace(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkAnnotationROIRepresentation::MoveF2 )
    {
    this->MoveMinusYFace(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkAnnotationROIRepresentation::MoveF3 )
    {
    this->MovePlusYFace(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkAnnotationROIRepresentation::MoveF4 )
    {
    this->MoveMinusZFace(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkAnnotationROIRepresentation::MoveF5 )
    {
    this->MovePlusZFace(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkAnnotationROIRepresentation::Translating )
    {
    this->Translate(prevPickPoint, pickPoint);
    }

  else if ( this->InteractionState == vtkAnnotationROIRepresentation::Scaling )
    {
    this->Scale(prevPickPoint, pickPoint,
                static_cast<int>(e[0]), static_cast<int>(e[1]));
    }

  else if ( this->InteractionState == vtkAnnotationROIRepresentation::Rotating )
    {
    this->Rotate(static_cast<int>(e[0]), static_cast<int>(e[1]), prevPickPoint, pickPoint, vpn);
    }

  // Store the start position
  this->LastEventPosition[0] = e[0];
  this->LastEventPosition[1] = e[1];
  this->LastEventPosition[2] = 0.0;
}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation::MoveFace(double *p1, double *p2, double *dir,
                                    double *x1, double *x2, double *x3, double *x4,
                                    double *x5)
  {
  int i;
  double v[3], v2[3];

  for (i=0; i<3; i++)
    {
    v[i] = p2[i] - p1[i];
    v2[i] = dir[i];
    }

  vtkMath::Normalize(v2);
  double f = vtkMath::Dot(v,v2);

  for (i=0; i<3; i++)
    {
    v[i] = f*v2[i];

    x1[i] += v[i];
    x2[i] += v[i];
    x3[i] += v[i];
    x4[i] += v[i];
    x5[i] += v[i];
    }
  this->PositionHandles();
}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation::GetDirection(const double Nx[3],const double Ny[3],
                                        const double Nz[3], double dir[3])
{
  double dotNy, dotNz;
  double y[3];

  if(vtkMath::Dot(Nx,Nx)!=0)
    {
    dir[0] = Nx[0];
    dir[1] = Nx[1];
    dir[2] = Nx[2];
    }
  else
    {
    dotNy = vtkMath::Dot(Ny,Ny);
    dotNz = vtkMath::Dot(Nz,Nz);
    if(dotNy != 0 && dotNz != 0)
      {
      vtkMath::Cross(Ny,Nz,dir);
      }
    else if(dotNy != 0)
      {
      //dir must have been initialized to the
      //corresponding coordinate direction before calling
      //this method
      vtkMath::Cross(Ny,dir,y);
      vtkMath::Cross(y,Ny,dir);
      }
    else if(dotNz != 0)
      {
      //dir must have been initialized to the
      //corresponding coordinate direction before calling
      //this method
      vtkMath::Cross(Nz,dir,y);
      vtkMath::Cross(y,Nz,dir);
      }
    }
}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation::MovePlusXFace(double *p1, double *p2)
{
  double *pts =
    static_cast<vtkDoubleArray *>(this->Points->GetData())->GetPointer(0);

  double *h1 = pts + 3*9;

  double *x1 = pts + 3*1;
  double *x2 = pts + 3*2;
  double *x3 = pts + 3*5;
  double *x4 = pts + 3*6;

  double dir[3] = { 1 , 0 , 0};
  this->ComputeNormals();
  this->GetDirection(this->N[1],this->N[3],this->N[5],dir);
  this->MoveFace(p1,p2,dir,x1,x2,x3,x4,h1);
}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation::MoveMinusXFace(double *p1, double *p2)
{
  double *pts =
    static_cast<vtkDoubleArray *>(this->Points->GetData())->GetPointer(0);

  double *h1 = pts + 3*8;

  double *x1 = pts + 3*0;
  double *x2 = pts + 3*3;
  double *x3 = pts + 3*4;
  double *x4 = pts + 3*7;

  double dir[3]={-1,0,0};
  this->ComputeNormals();
  this->GetDirection(this->N[0],this->N[4],this->N[2],dir);

  this->MoveFace(p1,p2,dir,x1,x2,x3,x4,h1);
}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation::MovePlusYFace(double *p1, double *p2)
{
  double *pts =
     static_cast<vtkDoubleArray *>(this->Points->GetData())->GetPointer(0);

  double *h1 = pts + 3*11;

  double *x1 = pts + 3*2;
  double *x2 = pts + 3*3;
  double *x3 = pts + 3*6;
  double *x4 = pts + 3*7;

  double dir[3]={0,1,0};
  this->ComputeNormals();
  this->GetDirection(this->N[3],this->N[5],this->N[1],dir);

  this->MoveFace(p1,p2,dir,x1,x2,x3,x4,h1);
}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation::MoveMinusYFace(double *p1, double *p2)
{
  double *pts =
    static_cast<vtkDoubleArray *>(this->Points->GetData())->GetPointer(0);

  double *h1 = pts + 3*10;

  double *x1 = pts + 3*0;
  double *x2 = pts + 3*1;
  double *x3 = pts + 3*4;
  double *x4 = pts + 3*5;

  double dir[3] = {0, -1, 0};
  this->ComputeNormals();
  this->GetDirection(this->N[2],this->N[0],this->N[4],dir);

  this->MoveFace(p1,p2,dir,x1,x2,x3,x4,h1);
}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation::MovePlusZFace(double *p1, double *p2)
{
  double *pts =
    static_cast<vtkDoubleArray *>(this->Points->GetData())->GetPointer(0);

  double *h1 = pts + 3*13;

  double *x1 = pts + 3*4;
  double *x2 = pts + 3*5;
  double *x3 = pts + 3*6;
  double *x4 = pts + 3*7;

  double dir[3]={0,0,1};
  this->ComputeNormals();
  this->GetDirection(this->N[5],this->N[1],this->N[3],dir);

  this->MoveFace(p1,p2,dir,x1,x2,x3,x4,h1);
}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation::MoveMinusZFace(double *p1, double *p2)
{
  double *pts =
    static_cast<vtkDoubleArray *>(this->Points->GetData())->GetPointer(0);

  double *h1 = pts + 3*12;

  double *x1 = pts + 3*0;
  double *x2 = pts + 3*1;
  double *x3 = pts + 3*2;
  double *x4 = pts + 3*3;

  double dir[3]={0,0,-1};
  this->ComputeNormals();
  this->GetDirection(this->N[4],this->N[2],this->N[0],dir);

  this->MoveFace(p1,p2,dir,x1,x2,x3,x4,h1);
}

//----------------------------------------------------------------------------
// Loop through all points and translate them
void vtkAnnotationROIRepresentation::Translate(double *p1, double *p2)
{
  double *pts =
    static_cast<vtkDoubleArray *>(this->Points->GetData())->GetPointer(0);
  double v[3];

  v[0] = p2[0] - p1[0];
  v[1] = p2[1] - p1[1];
  v[2] = p2[2] - p1[2];

  // Move the corners
  for (int i=0; i<8; i++)
    {
    *pts++ += v[0];
    *pts++ += v[1];
    *pts++ += v[2];
    }
  this->PositionHandles();
}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation::Scale(double *vtkNotUsed(p1),
                                 double *vtkNotUsed(p2),
                                 int vtkNotUsed(X),
                                 int Y)
{
  double *pts =
      static_cast<vtkDoubleArray *>(this->Points->GetData())->GetPointer(0);
  double *center
    = static_cast<vtkDoubleArray *>(this->Points->GetData())->GetPointer(3*14);
  double sf;

  if ( Y > this->LastEventPosition[1] )
    {
    sf = 1.03;
    }
  else
    {
    sf = 0.97;
    }

  // Move the corners
  for (int i=0; i<8; i++, pts+=3)
    {
    pts[0] = sf * (pts[0] - center[0]) + center[0];
    pts[1] = sf * (pts[1] - center[1]) + center[1];
    pts[2] = sf * (pts[2] - center[2]) + center[2];
    }
  this->PositionHandles();
}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation::ComputeNormals()
{
  double *pts =
     static_cast<vtkDoubleArray *>(this->Points->GetData())->GetPointer(0);
  double *p0 = pts;
  double *px = pts + 3*1;
  double *py = pts + 3*3;
  double *pz = pts + 3*4;
  int i;

  for (i=0; i<3; i++)
    {
    this->N[0][i] = p0[i] - px[i];
    this->N[2][i] = p0[i] - py[i];
    this->N[4][i] = p0[i] - pz[i];
    }
  vtkMath::Normalize(this->N[0]);
  vtkMath::Normalize(this->N[2]);
  vtkMath::Normalize(this->N[4]);
  for (i=0; i<3; i++)
    {
    this->N[1][i] = -this->N[0][i];
    this->N[3][i] = -this->N[2][i];
    this->N[5][i] = -this->N[4][i];
    }
}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation::GetPlanes(vtkPlanes *planes)
{
  if ( ! planes )
    {
    return;
    }

  this->ComputeNormals();

  // Set the normals and coordinate values
  double factor = (this->InsideOut ? -1.0 : 1.0);
  for (int i=0; i<6; i++)
    {
    this->PlanePoints->SetPoint(i,this->Points->GetPoint(8+i));
    this->PlaneNormals->SetTuple3(i, factor*this->N[i][0],
                                  factor*this->N[i][1], factor*this->N[i][2]);
    }

  planes->SetPoints(this->PlanePoints);
  planes->SetNormals(this->PlaneNormals);
  planes->Modified();
}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation::Rotate(int X,
                                  int Y,
                                  double *p1,
                                  double *p2,
                                  double *vpn)
{
  double *pts =
    static_cast<vtkDoubleArray *>(this->Points->GetData())->GetPointer(0);
  double *center =
    static_cast<vtkDoubleArray *>(this->Points->GetData())->GetPointer(3*14);
  double v[3]; //vector of motion
  double axis[3]; //axis of rotation
  double theta; //rotation angle
  int i;

  v[0] = p2[0] - p1[0];
  v[1] = p2[1] - p1[1];
  v[2] = p2[2] - p1[2];

  // Create axis of rotation and angle of rotation
  vtkMath::Cross(vpn,v,axis);
  if ( vtkMath::Normalize(axis) == 0.0 )
    {
    return;
    }
  int *size = this->Renderer->GetSize();
  double l2 = (X-this->LastEventPosition[0])*(X-this->LastEventPosition[0])
             + (Y-this->LastEventPosition[1])*(Y-this->LastEventPosition[1]);
  theta = 360.0 * sqrt(l2/(size[0]*size[0]+size[1]*size[1]));

  //Manipulate the transform to reflect the rotation
  this->Transform->Identity();
  this->Transform->Translate(center[0],center[1],center[2]);
  this->Transform->RotateWXYZ(theta,axis);
  this->Transform->Translate(-center[0],-center[1],-center[2]);

  //Set the corners
  vtkPoints *newPts = vtkPoints::New(VTK_DOUBLE);
  this->Transform->TransformPoints(this->Points,newPts);

  for (i=0; i<8; i++, pts+=3)
    {
    this->Points->SetPoint(i, newPts->GetPoint(i));
    }

  newPts->Delete();
  this->PositionHandles();
}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation::CreateDefaultProperties()
{
  // Handle properties
  //  this->HandleProperty = vtkProperty::New();
  //this->HandleProperty->SetColor(1,1,1);
  for(int i=0;i<NUMBER_HANDLES;i++)
    {
      this->HandleProperties[i]=vtkProperty::New();
    }

  // lavender
  this->HandleProperties[0]->SetColor(.781, .633, .867);
  // dark violet
  this->HandleProperties[1]->SetColor(.5585, .343, .91);
  // dark red
  this->HandleProperties[2]->SetColor(.75, .121, .26953);
  // orange
  this->HandleProperties[3]->SetColor(.9765, .588, .1133);
  // dark turquoise
  this->HandleProperties[4]->SetColor(.1328, .4531, .5351);
  // cyan
  this->HandleProperties[5]->SetColor(.582, .898, .871);
  // yellow
  this->HandleProperties[6]->SetColor(0.973125, .898281, 0.2);

  this->SelectedHandleProperty = vtkProperty::New();
  // green
  this->SelectedHandleProperty->SetColor(.453125, .96796, .33593);


  // Face properties
  this->FaceProperty = vtkProperty::New();
  this->FaceProperty->SetColor(1,1,1);
  this->FaceProperty->SetOpacity(0.0);

  this->SelectedFaceProperty = vtkProperty::New();
  this->SelectedFaceProperty->SetColor(1,1,0);
  this->SelectedFaceProperty->SetOpacity(0.25);

  // Outline properties
  this->OutlineProperty = vtkProperty::New();
  this->OutlineProperty->SetRepresentationToWireframe();
  this->OutlineProperty->SetAmbient(1.0);
  this->OutlineProperty->SetAmbientColor(1.0,1.0,1.0);
  this->OutlineProperty->SetLineWidth(2.0);

  this->SelectedOutlineProperty = vtkProperty::New();
  this->SelectedOutlineProperty->SetRepresentationToWireframe();
  this->SelectedOutlineProperty->SetAmbient(1.0);
  this->SelectedOutlineProperty->SetAmbientColor(0.0,1.0,0.0);
  this->SelectedOutlineProperty->SetLineWidth(2.0);
}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation::PlaceWidget(double bds[6])
{
  int i;
  double bounds[6], center[3];

  this->AdjustBounds(bds,bounds,center);

  this->Points->SetPoint(0, bounds[0], bounds[2], bounds[4]);
  this->Points->SetPoint(1, bounds[1], bounds[2], bounds[4]);
  this->Points->SetPoint(2, bounds[1], bounds[3], bounds[4]);
  this->Points->SetPoint(3, bounds[0], bounds[3], bounds[4]);
  this->Points->SetPoint(4, bounds[0], bounds[2], bounds[5]);
  this->Points->SetPoint(5, bounds[1], bounds[2], bounds[5]);
  this->Points->SetPoint(6, bounds[1], bounds[3], bounds[5]);
  this->Points->SetPoint(7, bounds[0], bounds[3], bounds[5]);

  for (i=0; i<6; i++)
    {
    this->InitialBounds[i] = bounds[i];
    }
  this->InitialLength = sqrt((bounds[1]-bounds[0])*(bounds[1]-bounds[0]) +
                             (bounds[3]-bounds[2])*(bounds[3]-bounds[2]) +
                             (bounds[5]-bounds[4])*(bounds[5]-bounds[4]));

  this->PositionHandles();
  this->ComputeNormals();
  this->ValidPick = 1; //since we have set up widget
  this->SizeHandles();
}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation::GetCenter(double center[])
{
  double *pts =
    static_cast<vtkDoubleArray *>(this->Points->GetData())->GetPointer(0);
  double *p14 = pts + 3*14;
    for (int i=0; i<3; i++)
    {
    center[i] = p14[i];
    }

}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation::GetExtents(double bounds[])
{
  double *pts =
     static_cast<vtkDoubleArray *>(this->Points->GetData())->GetPointer(0);
  double *p0 = pts;
  double *p1 = pts + 3*1;
  double *p2 = pts + 3*3;
  double *p3 = pts + 3*4;

  bounds[0] = sqrt( (p1[0]-p0[0])*(p1[0]-p0[0]) + (p1[1]-p0[1])*(p1[1]-p0[1]) + (p1[2]-p0[2])*(p1[2]-p0[2]) );
  bounds[1] = sqrt( (p2[0]-p0[0])*(p2[0]-p0[0]) + (p2[1]-p0[1])*(p2[1]-p0[1]) + (p2[2]-p0[2])*(p2[2]-p0[2]) );
  bounds[2] = sqrt( (p3[0]-p0[0])*(p3[0]-p0[0]) + (p3[1]-p0[1])*(p3[1]-p0[1]) + (p3[2]-p0[2])*(p3[2]-p0[2]) );

}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation
::SetWorldToLocalMatrix(vtkMatrix4x4 *worldToLocalMatrix)
{
  this->WorldToLocalMatrix->DeepCopy(worldToLocalMatrix);
}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation::GetTransform(vtkTransform *t)
{
  double *pts =
    static_cast<vtkDoubleArray *>(this->Points->GetData())->GetPointer(0);
  double *p0 = pts;
  double *p1 = pts + 3*1;
  double *p3 = pts + 3*3;
  double *p4 = pts + 3*4;
  double *p14 = pts + 3*14;
  double center[3], translate[3], scale[3], scaleVec[3][3];
  double InitialCenter[3];
  int i;

  // The transformation is relative to the initial bounds.
  // Initial bounds are set when PlaceWidget() is invoked.
  t->Identity();

  // Translation
  for (i=0; i<3; i++)
    {
    InitialCenter[i] =
      (this->InitialBounds[2*i+1]+this->InitialBounds[2*i]) / 2.0;
    center[i] = p14[i] - InitialCenter[i];
    }
  translate[0] = center[0] + InitialCenter[0];
  translate[1] = center[1] + InitialCenter[1];
  translate[2] = center[2] + InitialCenter[2];
  t->Translate(translate[0], translate[1], translate[2]);

  // Orientation
  this->Matrix->Identity();
  this->PositionHandles();
  this->ComputeNormals();
  for (i=0; i<3; i++)
    {
    this->Matrix->SetElement(i,0,this->N[1][i]);
    this->Matrix->SetElement(i,1,this->N[3][i]);
    this->Matrix->SetElement(i,2,this->N[5][i]);
    }
  t->Concatenate(this->Matrix);

  // Scale
  for (i=0; i<3; i++)
    {
    scaleVec[0][i] = (p1[i] - p0[i]);
    scaleVec[1][i] = (p3[i] - p0[i]);
    scaleVec[2][i] = (p4[i] - p0[i]);
    }

  scale[0] = vtkMath::Norm(scaleVec[0]);
  if (this->InitialBounds[1] != this->InitialBounds[0])
    {
    scale[0] = scale[0] / (this->InitialBounds[1]-this->InitialBounds[0]);
    }
  scale[1] = vtkMath::Norm(scaleVec[1]);
  if (this->InitialBounds[3] != this->InitialBounds[2])
    {
    scale[1] = scale[1] / (this->InitialBounds[3]-this->InitialBounds[2]);
    }
  scale[2] = vtkMath::Norm(scaleVec[2]);
  if (this->InitialBounds[5] != this->InitialBounds[4])
    {
    scale[2] = scale[2] / (this->InitialBounds[5]-this->InitialBounds[4]);
    }
  t->Scale(scale[0],scale[1],scale[2]);

  // Add back in the contribution due to non-origin center
  t->Translate(-InitialCenter[0], -InitialCenter[1], -InitialCenter[2]);
}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation::SetTransform(vtkTransform* t)
{
  if (!t)
    {
    vtkErrorMacro(<<"vtkTransform t must be non-NULL");
    return;
    }

  double *pts =
     static_cast<vtkDoubleArray *>(this->Points->GetData())->GetPointer(0);
  double xIn[3];
  // make sure the transform is up-to-date before using it
  t->Update();

  // Position the eight points of the box and then update the
  // position of the other handles.
  double *bounds=this->InitialBounds;

  xIn[0] = bounds[0]; xIn[1] = bounds[2]; xIn[2] = bounds[4];
  t->InternalTransformPoint(xIn,pts);

  xIn[0] = bounds[1]; xIn[1]= bounds[2]; xIn[2] = bounds[4];
  t->InternalTransformPoint(xIn,pts+3);

  xIn[0] = bounds[1]; xIn[1]= bounds[3]; xIn[2] = bounds[4];
  t->InternalTransformPoint(xIn,pts+6);

  xIn[0] = bounds[0]; xIn[1]= bounds[3]; xIn[2] = bounds[4];
  t->InternalTransformPoint(xIn,pts+9);

  xIn[0] = bounds[0]; xIn[1]= bounds[2]; xIn[2] = bounds[5];
  t->InternalTransformPoint(xIn,pts+12);

  xIn[0] = bounds[1]; xIn[1]= bounds[2]; xIn[2] = bounds[5];
  t->InternalTransformPoint(xIn,pts+15);

  xIn[0] = bounds[1]; xIn[1]= bounds[3]; xIn[2] = bounds[5];
  t->InternalTransformPoint(xIn,pts+18);

  xIn[0] = bounds[0]; xIn[1]= bounds[3]; xIn[2] = bounds[5];
  t->InternalTransformPoint(xIn,pts+21);

  this->PositionHandles();
}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation::SetOutlineFaceWires(int newValue)
{
  if (this->OutlineFaceWires != newValue)
    {
    this->OutlineFaceWires = newValue;
    this->Modified();
    // the outline is dependent on this value, so we have to regen
    this->GenerateOutline();
    }
}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation::SetOutlineCursorWires(int newValue)
{
  if (this->OutlineCursorWires != newValue)
    {
    this->OutlineCursorWires = newValue;
    this->Modified();
    // the outline is dependent on this value, so we have to regen
    this->GenerateOutline();
    }
}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation::GenerateOutline()
{
  // Whatever the case may be, we have to reset the Lines of the
  // OutlinePolyData (i.e. nuke all current line data)
  vtkCellArray *cells = this->OutlinePolyData->GetLines();
  cells->Reset();

  // Now the outline lines
  if ( ! this->OutlineFaceWires && ! this->OutlineCursorWires )
    {
    return;
    }

  vtkIdType pts[2];

  if ( this->OutlineFaceWires )
    {
    pts[0] = 0; pts[1] = 7;       //the -x face
    cells->InsertNextCell(2,pts);
    pts[0] = 3; pts[1] = 4;
    cells->InsertNextCell(2,pts);
    pts[0] = 1; pts[1] = 6;       //the +x face
    cells->InsertNextCell(2,pts);
    pts[0] = 2; pts[1] = 5;
    cells->InsertNextCell(2,pts);
    pts[0] = 1; pts[1] = 4;       //the -y face
    cells->InsertNextCell(2,pts);
    pts[0] = 0; pts[1] = 5;
    cells->InsertNextCell(2,pts);
    pts[0] = 3; pts[1] = 6;       //the +y face
    cells->InsertNextCell(2,pts);
    pts[0] = 2; pts[1] = 7;
    cells->InsertNextCell(2,pts);
    pts[0] = 0; pts[1] = 2;       //the -z face
    cells->InsertNextCell(2,pts);
    pts[0] = 1; pts[1] = 3;
    cells->InsertNextCell(2,pts);
    pts[0] = 4; pts[1] = 6;       //the +Z face
    cells->InsertNextCell(2,pts);
    pts[0] = 5; pts[1] = 7;
    cells->InsertNextCell(2,pts);
    }
  if ( this->OutlineCursorWires )
    {
    pts[0] = 8; pts[1] = 9;         //the x cursor line
    cells->InsertNextCell(2,pts);
    pts[0] = 10; pts[1] = 11;       //the y cursor line
    cells->InsertNextCell(2,pts);
    pts[0] = 12; pts[1] = 13;       //the z cursor line
    cells->InsertNextCell(2,pts);
    }
  this->OutlinePolyData->Modified();
  if ( this->OutlineProperty)
    {
    this->OutlineProperty->SetRepresentationToWireframe();
    this->SelectedOutlineProperty->SetRepresentationToWireframe();
    }
}

//----------------------------------------------------------------------------
int vtkAnnotationROIRepresentation::ComputeInteractionState(int X, int Y, int vtkNotUsed(modify))
{
  // Okay, we can process this. Try to pick handles first;
  // if no handles picked, then pick the bounding box.
  if (!this->Renderer || !this->Renderer->IsInViewport(X, Y))
    {
    this->InteractionState = vtkAnnotationROIRepresentation::Outside;
    return this->InteractionState;
    }

  vtkAssemblyPath *path;
  // Try and pick a handle first
  this->LastPicker = nullptr;
  this->CurrentHandle = nullptr;
  this->HandlePicker->Pick(X,Y,0.0,this->Renderer);
  path = this->HandlePicker->GetPath();
  if ( path != nullptr )
    {
    this->ValidPick = 1;
    this->LastPicker = this->HandlePicker;
    this->CurrentHandle =
           reinterpret_cast<vtkActor *>(path->GetFirstNode()->GetViewProp());
    if ( this->CurrentHandle == this->Handle[0] )
      {
      this->InteractionState = vtkAnnotationROIRepresentation::MoveF0;
      }
    else if ( this->CurrentHandle == this->Handle[1] )
      {
      this->InteractionState = vtkAnnotationROIRepresentation::MoveF1;
      }
    else if ( this->CurrentHandle == this->Handle[2] )
      {
      this->InteractionState = vtkAnnotationROIRepresentation::MoveF2;
      }
    else if ( this->CurrentHandle == this->Handle[3] )
      {
      this->InteractionState = vtkAnnotationROIRepresentation::MoveF3;
      }
    else if ( this->CurrentHandle == this->Handle[4] )
      {
      this->InteractionState = vtkAnnotationROIRepresentation::MoveF4;
      }
    else if ( this->CurrentHandle == this->Handle[5] )
      {
      this->InteractionState = vtkAnnotationROIRepresentation::MoveF5;
      }
    else if ( this->CurrentHandle == this->Handle[6] )
      {
      this->InteractionState = vtkAnnotationROIRepresentation::Translating;
      }
    }

  return this->InteractionState;
}

//----------------------------------------------------------------------
void vtkAnnotationROIRepresentation::SetInteractionState(int state)
{
  // Clamp to allowable values
  state = ( state < vtkAnnotationROIRepresentation::Outside ? vtkAnnotationROIRepresentation::Outside :
            (state > vtkAnnotationROIRepresentation::Scaling ? vtkAnnotationROIRepresentation::Scaling : state) );

  // Depending on state, highlight appropriate parts of representation
  int handle;
  this->InteractionState = state;
  switch (state)
    {
    case vtkAnnotationROIRepresentation::MoveF0:
    case vtkAnnotationROIRepresentation::MoveF1:
    case vtkAnnotationROIRepresentation::MoveF2:
    case vtkAnnotationROIRepresentation::MoveF3:
    case vtkAnnotationROIRepresentation::MoveF4:
    case vtkAnnotationROIRepresentation::MoveF5:
      this->HighlightOutline(0);
      handle = this->HighlightHandle(this->CurrentHandle);
      this->HighlightFace(handle);
      break;
    case vtkAnnotationROIRepresentation::Rotating:
      //this->HighlightOutline(0);
      //this->HighlightHandle(nullptr);
      //this->HighlightFace(this->HexPicker->GetCellId());
      break;
    case vtkAnnotationROIRepresentation::Translating:
    case vtkAnnotationROIRepresentation::Scaling:
      this->HighlightOutline(1);
      this->HighlightHandle(this->Handle[6]);
      this->HighlightFace(-1);
      break;
    default:
      this->HighlightOutline(0);
      this->HighlightHandle(nullptr);
      this->HighlightFace(-1);
    }
}

//----------------------------------------------------------------------
double *vtkAnnotationROIRepresentation::GetBounds()
{
  this->BuildRepresentation();
  this->BoundingBox->SetBounds(this->HexActor->GetBounds());
  return this->BoundingBox->GetBounds();
}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation::BuildRepresentation()
{
  // Rebuild only if necessary
  if ( this->GetMTime() > this->BuildTime ||
       (this->Renderer && this->Renderer->GetVTKWindow() &&
        this->Renderer->GetVTKWindow()->GetMTime() > this->BuildTime) )
    {

    this->SizeHandles();
    this->BuildTime.Modified();
    }
}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation::ReleaseGraphicsResources(vtkWindow *w)
{
  this->HexActor->ReleaseGraphicsResources(w);
  this->HexOutline->ReleaseGraphicsResources(w);
  this->HexFace->ReleaseGraphicsResources(w);
  // render the handles
  for (int j=0; j<7; j++)
    {
    this->Handle[j]->ReleaseGraphicsResources(w);
    }

}

//----------------------------------------------------------------------------
int vtkAnnotationROIRepresentation::RenderOpaqueGeometry(vtkViewport *v)
{
  int count=0;
  this->BuildRepresentation();

  this->HexActor->SetPropertyKeys(this->GetPropertyKeys());
  this->HexOutline->SetPropertyKeys(this->GetPropertyKeys());
  this->HexFace->SetPropertyKeys(this->GetPropertyKeys());

  count += this->HexActor->RenderOpaqueGeometry(v);
  count += this->HexOutline->RenderOpaqueGeometry(v);
  count += this->HexFace->RenderOpaqueGeometry(v);
  // render the handles
  for (int j=0; j<7; j++)
    {
    this->Handle[j]->SetPropertyKeys(this->GetPropertyKeys());
    count += this->Handle[j]->RenderOpaqueGeometry(v);
    }

  return count;
}

//----------------------------------------------------------------------------
int vtkAnnotationROIRepresentation::RenderTranslucentPolygonalGeometry(vtkViewport *v)
{
  int count=0;
  this->BuildRepresentation();

  this->HexActor->SetPropertyKeys(this->GetPropertyKeys());
  this->HexOutline->SetPropertyKeys(this->GetPropertyKeys());
  this->HexFace->SetPropertyKeys(this->GetPropertyKeys());

  count += this->HexActor->RenderTranslucentPolygonalGeometry(v);
  count += this->HexOutline->RenderTranslucentPolygonalGeometry(v);
  count += this->HexFace->RenderTranslucentPolygonalGeometry(v);
  // render the handles
  for (int j=0; j<7; j++)
    {
    this->Handle[j]->SetPropertyKeys(this->GetPropertyKeys());
    count += this->Handle[j]->RenderTranslucentPolygonalGeometry(v);
    }

  return count;
}

//----------------------------------------------------------------------------
int vtkAnnotationROIRepresentation::HasTranslucentPolygonalGeometry()
{
  int result=0;
  this->BuildRepresentation();

  result |= this->HexActor->HasTranslucentPolygonalGeometry();
  result |= this->HexOutline->HasTranslucentPolygonalGeometry();
  result |= this->HexFace->HasTranslucentPolygonalGeometry();
  // render the handles
  for (int j=0; j<7; j++)
    {
    result |= this->Handle[j]->HasTranslucentPolygonalGeometry();
    }

  return result;
}

#define VTK_AVERAGE(a,b,c) \
  c[0] = (a[0] + b[0])/2.0; \
  c[1] = (a[1] + b[1])/2.0; \
  c[2] = (a[2] + b[2])/2.0;

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation::PositionHandles()
{
  double *pts =
     static_cast<vtkDoubleArray *>(this->Points->GetData())->GetPointer(0);
  double *p0 = pts;
  double *p1 = pts + 3*1;
  double *p2 = pts + 3*2;
  double *p3 = pts + 3*3;
  //double *p4 = pts + 3*4;
  double *p5 = pts + 3*5;
  double *p6 = pts + 3*6;
  double *p7 = pts + 3*7;
  double x[3];

  VTK_AVERAGE(p0,p7,x);
  this->Points->SetPoint(8, x);
  VTK_AVERAGE(p1,p6,x);
  this->Points->SetPoint(9, x);
  VTK_AVERAGE(p0,p5,x);
  this->Points->SetPoint(10, x);
  VTK_AVERAGE(p2,p7,x);
  this->Points->SetPoint(11, x);
  VTK_AVERAGE(p1,p3,x);
  this->Points->SetPoint(12, x);
  VTK_AVERAGE(p5,p7,x);
  this->Points->SetPoint(13, x);
  VTK_AVERAGE(p0,p6,x);
  this->Points->SetPoint(14, x);

  int i;
  for (i = 0; i < 7; ++i)
    {
    this->HandleGeometry[i]->SetCenter(this->Points->GetPoint(8+i));
    }

  this->Points->GetData()->Modified();
  this->HexFacePolyData->Modified();
  this->HexPolyData->Modified();
  this->GenerateOutline();
}
#undef VTK_AVERAGE

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation::HandlesOn()
{
  for (int i=0; i<7; i++)
    {
    this->Handle[i]->VisibilityOn();
    }
}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation::HandlesOff()
{
  for (int i=0; i<7; i++)
    {
    this->Handle[i]->VisibilityOff();
    }
}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation::SizeHandles()
{
  double *center
    = static_cast<vtkDoubleArray *>(this->Points->GetData())->GetPointer(3*14);
  double radius =
      this->vtkWidgetRepresentation::SizeHandlesInPixels(1.5,center);
  for(int i=0; i<7; i++)
    {
    this->HandleGeometry[i]->SetRadius(this->Handle[i]->GetVisibility()*radius);
    }
}

//----------------------------------------------------------------------------
int vtkAnnotationROIRepresentation::HighlightHandle(vtkProp *prop)
{
  // first unhighlight anything picked
  this->HighlightOutline(0);

  for (int i = 0; i < NUMBER_HANDLES; i++)
    {
      this->Handle[i]->SetProperty(this->HandleProperties[i]);
    }
  this->CurrentHandle = static_cast<vtkActor *>(prop);

  if ( this->CurrentHandle )
    {
    this->CurrentHandle->SetProperty(this->SelectedHandleProperty);
    for (int i=0; i<6; i++) //find attached face
      {
      if ( this->CurrentHandle == this->Handle[i] )
        {
        return i;
        }
      }
    }

  if ( this->CurrentHandle == this->Handle[6] )
    {
    this->HighlightOutline(1);
    return 6;
    }

  return -1;
}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation::HighlightFace(int cellId)
{
  if ( cellId >= 0 )
    {
    vtkIdType npts;
#if VTK_MAJOR_VERSION >= 9 || (VTK_MAJOR_VERSION >= 8 && VTK_MINOR_VERSION >= 90)
    const vtkIdType *pts;
#else
    vtkIdType *pts;
#endif
    vtkCellArray *cells = this->HexFacePolyData->GetPolys();
    this->HexPolyData->GetCellPoints(cellId, npts, pts);
    this->HexFacePolyData->Modified();
    cells->ReplaceCell(0,npts,pts);
    this->CurrentHexFace = cellId;
    this->HexFace->SetProperty(this->SelectedFaceProperty);
    if ( !this->CurrentHandle )
      {
      this->CurrentHandle = this->HexFace;
      }
    }
  else
    {
    this->HexFace->SetProperty(this->FaceProperty);
    this->CurrentHexFace = -1;
    }
}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation::HighlightOutline(int highlight)
{
  if ( highlight )
    {
    this->HexActor->SetProperty(this->SelectedOutlineProperty);
    this->HexOutline->SetProperty(this->SelectedOutlineProperty);
    }
  else
    {
    this->HexActor->SetProperty(this->OutlineProperty);
    this->HexOutline->SetProperty(this->OutlineProperty);
    }
}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  double *bounds=this->InitialBounds;
  os << indent << "Initial Bounds: "
     << "(" << bounds[0] << "," << bounds[1] << ") "
     << "(" << bounds[2] << "," << bounds[3] << ") "
     << "(" << bounds[4] << "," << bounds[5] << ")\n";

  for(int i=0;i<NUMBER_HANDLES;i++)
    {
    os << indent << "Handle Property: " <<i<< this->HandleProperties[i] << "\n";
    }

  if ( this->SelectedHandleProperty )
    {
    os << indent << "Selected Handle Property: "
       << this->SelectedHandleProperty << "\n";
    }
  else
    {
    os << indent << "SelectedHandle Property: (none)\n";
    }

  if ( this->FaceProperty )
    {
    os << indent << "Face Property: " << this->FaceProperty << "\n";
    }
  else
    {
    os << indent << "Face Property: (none)\n";
    }
  if ( this->SelectedFaceProperty )
    {
    os << indent << "Selected Face Property: "
       << this->SelectedFaceProperty << "\n";
    }
  else
    {
    os << indent << "Selected Face Property: (none)\n";
    }

  if ( this->OutlineProperty )
    {
    os << indent << "Outline Property: " << this->OutlineProperty << "\n";
    }
  else
    {
    os << indent << "Outline Property: (none)\n";
    }
  if ( this->SelectedOutlineProperty )
    {
    os << indent << "Selected Outline Property: "
       << this->SelectedOutlineProperty << "\n";
    }
  else
    {
    os << indent << "Selected Outline Property: (none)\n";
    }

  os << indent << "Outline Face Wires: "
     << (this->OutlineFaceWires ? "On\n" : "Off\n");
  os << indent << "Outline Cursor Wires: "
     << (this->OutlineCursorWires ? "On\n" : "Off\n");
  os << indent << "Inside Out: " << (this->InsideOut ? "On\n" : "Off\n");
}
