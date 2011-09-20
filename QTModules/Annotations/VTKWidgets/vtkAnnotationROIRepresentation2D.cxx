/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkAnnotationROIRepresentation2D.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkAnnotationROIRepresentation2D.h"
#include "vtkActor2D.h"
#include "vtkSphereSource.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkPolyData.h"
#include "vtkCallbackCommand.h"
#include "vtkBox.h"
#include "vtkPolyData.h"
#include "vtkProperty2D.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkInteractorObserver.h"
#include "vtkMath.h"
#include "vtkCellArray.h"
#include "vtkPropPicker.h"
#include "vtkTransform.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkPlanes.h"
#include "vtkCamera.h"
#include "vtkAssemblyPath.h"
#include "vtkWindow.h"
#include "vtkProperty2D.h"
#include "vtkObjectFactory.h"

#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkCutter.h>
#include <vtkPlane.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkMath.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>


vtkCxxRevisionMacro(vtkAnnotationROIRepresentation2D, "$Revision: 12141 $");
vtkStandardNewMacro(vtkAnnotationROIRepresentation2D);

//----------------------------------------------------------------------------
vtkAnnotationROIRepresentation2D::vtkAnnotationROIRepresentation2D()
{
  this->LastEventPosition2D[0]=0;
  this->LastEventPosition2D[1]=0;
  this->LastEventPosition2D[2]=0;
  this->LastEventPosition2D[3]=1;
  this->LastPicker2D = NULL;

  this->HandleSizeInPixels = 4;
  this->HandlesVisibility = 1;
  
  // Set up the initial properties
  this->CreateDefaultProperties();
  this->SelectedHandleProperty2D = vtkProperty2D::New();
  this->SelectedHandleProperty2D->SetColor(1,0,0);

  // The face of the hexahedra
  this->HexFaceMapper2D = vtkPolyDataMapper2D::New();
  this->HexFaceMapper2D->SetInput(HexFacePolyData);
  this->HexFace2D = vtkActor2D::New();
  this->HexFace2D->SetMapper(this->HexFaceMapper2D);
  //this->HexFace2D->SetProperty(this->FaceProperty);

  // Create the handles
  this->Handle2D = new vtkActor2D* [7];
  this->HandleMapper2D = new vtkPolyDataMapper2D* [7];
  this->HandleToPlaneTransformFilters = new vtkTransformPolyDataFilter* [7];

  this->IntersectionPlane = vtkPlane::New();
  double normal[3]={1,0,0};
  double origin[3]={0,0,0};
  this->IntersectionPlane->SetNormal(normal);
  this->IntersectionPlane->SetOrigin(origin);

  this->IntersectionPlaneTransform = vtkTransform::New();

  this->HandlePicker2D = vtkPropPicker::New();
  this->HandlePicker2D->PickFromListOn();

  this->CreateFaceIntersections();

  // 2D handles pipelines
  int i;
  for (i=0; i<7; i++)
    {
    this->HandleGeometry[i]->SetRadius(0);

    this->HandleToPlaneTransformFilters[i] = vtkTransformPolyDataFilter::New();
    this->HandleToPlaneTransformFilters[i]->SetInput(this->HandleGeometry[i]->GetOutput());
    this->HandleToPlaneTransformFilters[i]->SetTransform(this->IntersectionPlaneTransform);

    this->HandleMapper2D[i] = vtkPolyDataMapper2D::New();
    this->HandleMapper2D[i]->SetInput(this->HandleToPlaneTransformFilters[i]->GetOutput());
    this->Handle2D[i] = vtkActor2D::New();
    //this->Handle2D[i]->SetProperty(this->HandleProperties[i]);
    this->Handle2D[i]->SetMapper(this->HandleMapper2D[i]);

    this->HandlePicker2D->AddPickList(this->Handle2D[i]);
    this->Handle2D[i]->SetProperty(this->HandleProperties2D[i]);
    //this->Handle2D[i]->SetVisibility(0);
    }

  this->DefaultFaceProperty2D = NULL;
  this->LastPicker2D = NULL;
  this->CurrentHandle2D = NULL;
  
  this->PositionHandles();

}

//----------------------------------------------------------------------------
vtkAnnotationROIRepresentation2D::~vtkAnnotationROIRepresentation2D()
{  
  this->HandlePicker2D->Delete();
  this->HexFace2D->Delete();
  this->HexFaceMapper2D->Delete();
  int i;
  for (i=0; i<7; i++)
    {
    this->HandleToPlaneTransformFilters[i]->Delete();
    this->HandleMapper2D[i]->Delete();
    this->Handle2D[i]->Delete();
    }
  delete [] this->Handle2D;
  delete [] this->HandleMapper2D;
  delete [] this->HandleToPlaneTransformFilters;
 
  this->IntersectionPlane->Delete();
  this->IntersectionPlaneTransform->Delete();
  for (i=0; i<6; i++)
    {
    this->IntersectionFaces[i]->Delete();
    this->IntersectionCutters[i]->Delete();
    this->IntersectionPlaneTransformFilters[i]->Delete();
    this->IntersectionMappers[i]->Delete();
    this->IntersectionActors[i]->Delete();
    }
  for(int i=0;i<NUMBER_HANDLES;i++)
    {
    this->HandleProperties2D[i]->Delete();
    this->HandleProperties2D[i]=NULL;
    }
  this->SelectedHandleProperty2D->Delete();
  this->SelectedFaceProperty2D->Delete();
  if (this->DefaultFaceProperty2D)
    {
    this->DefaultFaceProperty2D->Delete();
    }
  if (this->LastPicker2D)
    {
    this->LastPicker2D->Delete();
    }
}

//----------------------------------------------------------------------
void vtkAnnotationROIRepresentation2D::CreateFaceIntersections()
{
  int i;

  // Create Plane/Face intersection pipelines
  int faceIndex[6][4] = {
    {3, 0, 4, 7},
    {1, 2, 6, 5},
    {0, 1, 5, 4},
    {2, 3, 7, 6},
    {0, 3, 2, 1},
    {4, 5, 6, 7}};

  for (i=0; i<6; i++)
    {
    this->IntersectionFaces[i] = vtkPolyData::New();
    this->IntersectionFaces[i]->SetPoints(this->Points);
    vtkIdType pts[4];
    for (int j=0; j<4; j++)
      {
      pts[j] = faceIndex[i][j];
      }
    vtkCellArray *cells = vtkCellArray::New();
    cells->Allocate(cells->EstimateSize(1,4));
    cells->InsertNextCell(4,pts);
    this->IntersectionFaces[i]->SetPolys(cells);
    this->IntersectionFaces[i]->BuildCells();
    cells->Delete();

    this->IntersectionCutters[i] = vtkCutter::New();
    this->IntersectionCutters[i]->SetInput(this->IntersectionFaces[i]);
    this->IntersectionCutters[i]->SetCutFunction(this->IntersectionPlane);

    this->IntersectionPlaneTransformFilters[i] = vtkTransformPolyDataFilter::New();
    this->IntersectionPlaneTransformFilters[i]->SetInput(this->IntersectionCutters[i]->GetOutput());
    this->IntersectionPlaneTransformFilters[i]->SetTransform(this->IntersectionPlaneTransform);

    this->IntersectionMappers[i] = vtkPolyDataMapper2D::New();
    this->IntersectionMappers[i]->SetInput(this->IntersectionPlaneTransformFilters[i]->GetOutput());

    this->IntersectionActors[i] = vtkActor2D::New();
    this->IntersectionActors[i]->SetMapper(this->IntersectionMappers[i]);

    //this->GetRenderer()->AddActor2D(this->IntersectionActors[i]);
    }
}


//----------------------------------------------------------------------
void vtkAnnotationROIRepresentation2D::GetActors2D(vtkPropCollection *actors)
{
  actors->RemoveAllItems();
  //actors->AddItem(this->HexFace2D);
  int i;
  for (i=0; i<6; i++)
    {
    actors->AddItem(this->IntersectionActors[i]);
    }
  for (i=0; i<7; i++)
    {
    actors->AddItem(this->Handle2D[i]);
    }
}

//----------------------------------------------------------------------
void vtkAnnotationROIRepresentation2D::GetIntersectionActors(vtkPropCollection *actors)
{
  actors->RemoveAllItems();
  for (int i=0; i<6; i++)
    {
    actors->AddItem(this->IntersectionActors[i]);
    }
}


//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation2D::ReleaseGraphicsResources(vtkWindow *w)
{
  Superclass::ReleaseGraphicsResources(w);
  this->HexFace2D->ReleaseGraphicsResources(w);
  // render the handles
  int j;
  for (j=0; j<7; j++)
    {
    this->Handle2D[j]->ReleaseGraphicsResources(w);
    }
  for (j=0; j<6; j++)
    {
    this->IntersectionActors[j]->ReleaseGraphicsResources(w);
    }

}

//----------------------------------------------------------------------
int vtkAnnotationROIRepresentation2D::RenderOverlay(vtkViewport *vtkNotUsed(v))
{
  int count=0;
  return count;
}
//----------------------------------------------------------------------------
int vtkAnnotationROIRepresentation2D::RenderOpaqueGeometry(vtkViewport *vtkNotUsed(v))
{
  int count=0;
  return count;
}

//----------------------------------------------------------------------------
int vtkAnnotationROIRepresentation2D::RenderTranslucentPolygonalGeometry(vtkViewport *vtkNotUsed(v))
{
  int count=0;
  return count;
}

//----------------------------------------------------------------------------
int vtkAnnotationROIRepresentation2D::HasTranslucentPolygonalGeometry()
{
  int result=0;
  return result;
}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation2D::CreateDefaultProperties()
{
  for(int i=0;i<NUMBER_HANDLES;i++)
    {
      this->HandleProperties2D[i]=vtkProperty2D::New();
    }
  this->HandleProperties2D[0]->SetColor(1,0,1);
  this->HandleProperties2D[1]->SetColor(1,0,0);
  this->HandleProperties2D[2]->SetColor(1,1,1);
  this->HandleProperties2D[3]->SetColor(.89,.6,.07);
  this->HandleProperties2D[4]->SetColor(0,0,1);
  this->HandleProperties2D[5]->SetColor(0.2,0.2,0.2);
  this->HandleProperties2D[6]->SetColor(1,1,1);


  this->SelectedHandleProperty2D = vtkProperty2D::New();
  this->SelectedHandleProperty2D->SetColor(1,0,0);

  this->SelectedFaceProperty2D = vtkProperty2D::New();
  this->SelectedFaceProperty2D->SetColor(1,1,0);
  this->SelectedFaceProperty2D->SetOpacity(0.25);

  this->DefaultFaceProperty2D = vtkProperty2D::New();

}

//----------------------------------------------------------------------
void vtkAnnotationROIRepresentation2D::SetInteractionState(int state)
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
      handle = this->HighlightHandle(this->CurrentHandle2D);
      this->HighlightFace(handle);
      break;
    case vtkAnnotationROIRepresentation::Rotating:
      //this->HighlightOutline(0);
      //this->HighlightHandle(NULL);
      //this->HighlightFace(this->HexPicker->GetCellId());
      break;
    case vtkAnnotationROIRepresentation::Translating:
    case vtkAnnotationROIRepresentation::Scaling:
      this->HighlightHandle(this->Handle2D[6]);
      this->HighlightFace(-1);
      break;
    default:
      this->HighlightHandle(NULL);
      this->HighlightFace(-1);
    }
}

//----------------------------------------------------------------------------
int vtkAnnotationROIRepresentation2D::HighlightHandle(vtkProp *prop)
{

  for (int i = 0; i < NUMBER_HANDLES; i++)
    {
      this->Handle2D[i]->SetProperty(this->HandleProperties2D[i]);
    }
  this->CurrentHandle2D = NULL;
  if (prop)
    {
    this->CurrentHandle2D = static_cast<vtkActor2D *>(prop);
    }

  if ( this->CurrentHandle2D )
    {
    this->CurrentHandle2D->SetProperty(this->SelectedHandleProperty2D);
    for (int i=0; i<6; i++) //find attached face
      {
      if ( this->CurrentHandle2D == this->Handle2D[i] )
        {
        return i;
        }
      }
    }
  
  if ( this->CurrentHandle2D == this->Handle2D[6] )
    {
    return 6;
    }
  
  return -1;
}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation2D::HighlightFace(int cellId)
{
  for (int i=0; i<6; i++)
    {
    this->IntersectionActors[i]->SetProperty(this->DefaultFaceProperty2D);
    }

  if ( cellId >= 0 )
    {
    this->IntersectionActors[cellId]->SetProperty(this->SelectedFaceProperty2D);
    }
}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation2D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

#define VTK_AVERAGE(a,b,c) \
  c[0] = (a[0] + b[0])/2.0; \
  c[1] = (a[1] + b[1])/2.0; \
  c[2] = (a[2] + b[2])/2.0;

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation2D::PositionHandles()
{
  int i;
  double *pts =
     static_cast<vtkDoubleArray *>(this->Points->GetData())->GetPointer(0);
  double *p0 = pts;
  //double *p1 = pts + 3*1;
  //double *p2 = pts + 3*2;
  //double *p3 = pts + 3*3;
  //double *p4 = pts + 3*4;
  //double *p5 = pts + 3*5;
  double *p6 = pts + 3*6;
  //double *p7 = pts + 3*7;
  double x[3];

  double radius = this->ComputeHandleRadiusInWorldCoordinates(this->HandleSizeInPixels);

  int count=0;
  this->Points->GetData()->Modified();
  for (i=0; i<6; i++)
    {
    this->IntersectionFaces[i]->Modified();
    this->IntersectionCutters[i]->Update();
    this->IntersectionPlaneTransformFilters[i]->Update();

    if (this->IntersectionCutters[i]->GetOutput()->GetNumberOfLines() > 0)
      {
      double pi0[3];
      double pi1[3];
      this->IntersectionCutters[i]->GetOutput()->GetPoint(0, pi0);
      this->IntersectionCutters[i]->GetOutput()->GetPoint(1, pi1);
      VTK_AVERAGE(pi0,pi1,x);
      this->Points->SetPoint(8+i, x);
      this->HandleGeometry[i]->SetRadius(this->HandlesVisibility*radius);
      this->Handle2D[i]->SetVisibility(this->HandlesVisibility);
      count++;
      }
    else
      {
      this->HandleGeometry[i]->SetRadius(0);
      this->Handle2D[i]->SetVisibility(0);
      }
    }
    
  VTK_AVERAGE(p0,p6,x);
  this->Points->SetPoint(14, x);
  if (count)
    {
    this->HandleGeometry[6]->SetRadius(this->HandlesVisibility*radius);
    this->Handle2D[6]->SetVisibility(this->HandlesVisibility);
    }
  else
    {
    this->HandleGeometry[6]->SetRadius(0);
    this->Handle2D[6]->SetVisibility(0);
    }

  for (i = 0; i < 7; ++i)
    {
    this->HandleGeometry[i]->SetCenter(this->Points->GetPoint(8+i));
    this->HandleToPlaneTransformFilters[i]->Update();
    }

  this->Points->GetData()->Modified();

}

#undef VTK_AVERAGE


//----------------------------------------------------------------------
void vtkAnnotationROIRepresentation2D::WidgetInteraction(double e[2])
{
  // Convert events to appropriate coordinate systems
  vtkCamera *camera = this->Renderer->IsActiveCameraCreated() ? this->Renderer->GetActiveCamera() : NULL;
  if ( !camera )
    {
    return;
    }

  // Get transform from 2D image to world
  vtkSmartPointer<vtkMatrix4x4> XYtoWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  XYtoWorldMatrix->DeepCopy(this->GetIntersectionPlaneTransform()->GetMatrix());
  XYtoWorldMatrix->Invert();



  // Compute the two points defining the motion vector
  double point2D[4] = {e[0],e[1], 0, 1};
  double pickPoint[4] = {0, 0, 0, 1};
  double prevPickPoint[4] = {0, 0, 0, 1};

  XYtoWorldMatrix->MultiplyPoint(point2D, pickPoint);
  XYtoWorldMatrix->MultiplyPoint(this->LastEventPosition2D, prevPickPoint);

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
    double vpn[3];
    camera->GetViewPlaneNormal(vpn);

    this->Rotate(static_cast<int>(e[0]), static_cast<int>(e[1]), prevPickPoint, pickPoint, vpn);
    }

  // Store the start position
  this->LastEventPosition2D[0] = e[0];
  this->LastEventPosition2D[1] = e[1];
  this->LastEventPosition2D[2] = 0.0;
  this->LastEventPosition2D[3] = 1.0;
}


//----------------------------------------------------------------------
void vtkAnnotationROIRepresentation2D::StartWidgetInteraction(double e[2])
{
  Superclass::StartWidgetInteraction(e);
  // Store the start position
  this->LastEventPosition2D[0] = e[0];
  this->LastEventPosition2D[1] = e[1];
  this->LastEventPosition2D[2] = 0.0;
  this->LastEventPosition2D[3] = 1.0;
}


//----------------------------------------------------------------------------
int vtkAnnotationROIRepresentation2D::ComputeInteractionState(int X, int Y, int vtkNotUsed(modify))
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
  this->LastPicker2D = NULL;
  this->CurrentHandle2D = NULL;
  this->HandlePicker2D->Pick(X,Y,0.0,this->Renderer);
  path = this->HandlePicker2D->GetPath();
  if ( path != NULL )
    {
    this->ValidPick = 1;
    this->LastPicker2D = this->HandlePicker2D;
    this->CurrentHandle2D =
           reinterpret_cast<vtkActor2D *>(path->GetFirstNode()->GetViewProp());
    if ( this->CurrentHandle2D == this->Handle2D[0] )
      {
      this->InteractionState = vtkAnnotationROIRepresentation::MoveF0;
      }
    else if ( this->CurrentHandle2D == this->Handle2D[1] )
      {
      this->InteractionState = vtkAnnotationROIRepresentation::MoveF1;
      }
    else if ( this->CurrentHandle2D == this->Handle2D[2] )
      {
      this->InteractionState = vtkAnnotationROIRepresentation::MoveF2;
      }
    else if ( this->CurrentHandle2D == this->Handle2D[3] )
      {
      this->InteractionState = vtkAnnotationROIRepresentation::MoveF3;
      }
    else if ( this->CurrentHandle2D == this->Handle2D[4] )
      {
      this->InteractionState = vtkAnnotationROIRepresentation::MoveF4;
      }
    else if ( this->CurrentHandle2D == this->Handle2D[5] )
      {
      this->InteractionState = vtkAnnotationROIRepresentation::MoveF5;
      }
    else if ( this->CurrentHandle2D == this->Handle2D[6] )
      {
      this->InteractionState = vtkAnnotationROIRepresentation::Translating;
      }
    }

  return this->InteractionState;
}

//----------------------------------------------------------------------------
double vtkAnnotationROIRepresentation2D::ComputeHandleRadiusInWorldCoordinates(double radInPixels)
{
  /*
  double *center 
    = static_cast<vtkDoubleArray *>(this->Points->GetData())->GetPointer(3*14);

  double radius =
      this->vtkWidgetRepresentation::SizeHandlesInPixels(1.5,center);
  return radius;
  */

 
  // Get transform from 2D image to world
  vtkSmartPointer<vtkMatrix4x4> XYtoWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  XYtoWorldMatrix->DeepCopy(this->GetIntersectionPlaneTransform()->GetMatrix());
  XYtoWorldMatrix->Invert();
  double xyz0[4] = {0,0,0,1};
  double xyz1[4] = {radInPixels,radInPixels,0,1};
  double wxyz0[4] = {0,0,0,1};
  double wxyz1[4] = {0,0,0,1};

  XYtoWorldMatrix->MultiplyPoint(xyz0, wxyz0);
  XYtoWorldMatrix->MultiplyPoint(xyz1, wxyz1);

  double radius = 0;
  for (int i=0; i<3; i++)
    {
    radius += (wxyz1[i] - wxyz0[i])*(wxyz1[i] - wxyz0[i]);
    }
  return sqrt(radius/2);
}

//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation2D::SizeHandles()
{
  //double radius = this->ComputeHandleRadiusInWorldCoordinates(this->HandleSizeInPixels);
  for(int i=0; i<7; i++)
    {
    //this->HandleGeometry[i]->SetRadius(radius);
    this->HandleGeometry[i]->Modified();
    }
}
//----------------------------------------------------------------------------
void vtkAnnotationROIRepresentation2D::PrintIntersections(ostream& os)
{
  os << "PrintIntersections:\n";
  for (int i=0; i<6; i++)
    {
    double *pts = static_cast<vtkDoubleArray *>(this->Points->GetData())->GetPointer(0);
    vtkIdType ncpts;
    vtkIdType *cpts;
    this->IntersectionFaces[i]->GetCellPoints(0, ncpts, cpts);
    os << "   Face[" << i << "]=(" << pts[3*cpts[0]] << ", " << pts[3*cpts[0]+1] << ", " << pts[3*cpts[0]+2] << 
                             "), (" << pts[3*cpts[1]] << ", " << pts[3*cpts[1]+1] << ", " << pts[3*cpts[1]+2] << 
                             "), (" << pts[3*cpts[2]] << ", " << pts[3*cpts[2]+1] << ", " << pts[3*cpts[2]+2] << 
                             "), (" << pts[3*cpts[3]] << ", " << pts[3*cpts[3]+1] << ", " << pts[3*cpts[3]+2] << ")\n"; 

    if (this->IntersectionCutters[i]->GetOutput()->GetNumberOfLines())
      {
      float *fpts = static_cast<vtkFloatArray *>(this->IntersectionCutters[i]->GetOutput()->GetPoints()->GetData())->GetPointer(0);
      this->IntersectionCutters[i]->GetOutput()->GetLines()->GetCell(0, ncpts, cpts);
      os << "   Cutter[" << i <<"]=(" << fpts[3*cpts[0]] << ", " << fpts[3*cpts[0]+1] << ", " << fpts[3*cpts[0]+2] << 
                               "), (" << fpts[3*cpts[1]] << ", " << fpts[3*cpts[1]+1] << ", " << fpts[3*cpts[1]+2] << ")\n"; 
      }
    else
      {
      os << "   Cutter[" << i <<"]=null\n";
      }
    }
}
