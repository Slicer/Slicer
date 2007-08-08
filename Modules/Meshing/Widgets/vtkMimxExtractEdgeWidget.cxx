/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkMimxExtractEdgeWidget.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkMimxExtractEdgeWidget.h"

#include "vtkActor.h"
#include "vtkAssemblyNode.h"
#include "vtkAssemblyPath.h"
#include "vtkCallbackCommand.h"
#include "vtkCamera.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCellPicker.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPlanes.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkSphereSource.h"
#include "vtkTransform.h"
#include "vtkUnstructuredGrid.h"

vtkCxxRevisionMacro(vtkMimxExtractEdgeWidget, "$Revision: 1.3 $");
vtkStandardNewMacro(vtkMimxExtractEdgeWidget);

vtkMimxExtractEdgeWidget::vtkMimxExtractEdgeWidget()
{
  this->State = vtkMimxExtractEdgeWidget::Start;
  this->EventCallbackCommand->SetCallback(vtkMimxExtractEdgeWidget::ProcessEvents);
  
  // Enable/disable the translation, rotation, and scaling of the widget
//  this->TranslationEnabled = 1;
//  this->RotationEnabled = 1;
//  this->ScalingEnabled = 1;

  //Build the representation of the widget
  //int i;

  // Control orientation of normals
 /* this->InsideOut = 0;
  this->OutlineFaceWires = 0;
  this->OutlineCursorWires = 1;*/

  // Construct the poly data representing the hex
  this->EdgePolyData = vtkPolyData::New();
  this->EdgeMapper = vtkPolyDataMapper::New();
  this->EdgeMapper->SetInput(EdgePolyData);
  this->EdgeActor = vtkActor::New();
  this->EdgeActor->SetMapper(this->EdgeMapper);
  
  // Construct initial points
  //this->Points = vtkPoints::New(VTK_DOUBLE);
  //this->Points->SetNumberOfPoints(15);//8 corners; 6 faces; 1 center
  //this->HexPolyData->SetPoints(this->Points);
  
  // Construct connectivity for the faces. These are used to perform
  // the picking.
 /* vtkIdType pts[4];
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
  this->HexPolyData->BuildCells();*/
  
  // The face of the hexahedra
  //cells = vtkCellArray::New();
  //cells->Allocate(cells->EstimateSize(1,4));
  //cells->InsertNextCell(4,pts); //temporary, replaced later
  //this->HexFacePolyData = vtkPolyData::New();
  //this->HexFacePolyData->SetPoints(this->Points);
  //this->HexFacePolyData->SetPolys(cells);
  //this->HexFaceMapper = vtkPolyDataMapper::New();
  //this->HexFaceMapper->SetInput(HexFacePolyData);
  //this->HexFace = vtkActor::New();
  //this->HexFace->SetMapper(this->HexFaceMapper);
  //cells->Delete();

  //// Create the outline for the hex
  //this->OutlinePolyData = vtkPolyData::New();
  //this->OutlinePolyData->SetPoints(this->Points);
  //this->OutlineMapper = vtkPolyDataMapper::New();
  //this->OutlineMapper->SetInput(this->OutlinePolyData);
  //this->HexOutline = vtkActor::New();
  //this->HexOutline->SetMapper(this->OutlineMapper);
  //cells = vtkCellArray::New();
  //cells->Allocate(cells->EstimateSize(15,2));
  //this->OutlinePolyData->SetLines(cells);
  //cells->Delete();

  //// Set up the initial properties
  //this->CreateDefaultProperties();

  //// Create the outline
  //this->GenerateOutline();

  //// Create the handles
  //this->Handle = new vtkActor* [7];
  //this->HandleMapper = new vtkPolyDataMapper* [7];
  //this->HandleGeometry = new vtkSphereSource* [7];
  //for (i=0; i<7; i++)
  //  {
  //  this->HandleGeometry[i] = vtkSphereSource::New();
  //  this->HandleGeometry[i]->SetThetaResolution(16);
  //  this->HandleGeometry[i]->SetPhiResolution(8);
  //  this->HandleMapper[i] = vtkPolyDataMapper::New();
  //  this->HandleMapper[i]->SetInput(this->HandleGeometry[i]->GetOutput());
  //  this->Handle[i] = vtkActor::New();
  //  this->Handle[i]->SetMapper(this->HandleMapper[i]);
  //  }
  //
  //// Define the point coordinates
  //double bounds[6];
  //bounds[0] = -0.5;
  //bounds[1] = 0.5;
  //bounds[2] = -0.5;
  //bounds[3] = 0.5;
  //bounds[4] = -0.5;
  //bounds[5] = 0.5;
  //// Points 8-14 are down by PositionHandles();
  //this->PlaceWidget(bounds);

  //Manage the picking stuff
  this->EdgePicker = vtkCellPicker::New();
  this->EdgePicker->SetTolerance(0.001);
 /* for (i=0; i<7; i++)
    {
    this->HandlePicker->AddPickList(this->Handle[i]);
    }*/
  this->EdgePicker->AddPickList(this->EdgeActor);
  this->EdgePicker->PickFromListOn();

  //this->HexPicker = vtkCellPicker::New();
  //this->HexPicker->SetTolerance(0.001);
  //this->HexPicker->AddPickList(HexActor);
  //this->HexPicker->PickFromListOn();
  //
  //this->CurrentHandle = NULL;

  //this->Transform = vtkTransform::New();
  this->InputActor = NULL;
}

vtkMimxExtractEdgeWidget::~vtkMimxExtractEdgeWidget()
{
  this->EdgeActor->Delete();
  this->EdgeMapper->Delete();
  this->EdgePolyData->Delete();
  //this->Points->Delete();

  //this->HexFace->Delete();
  //this->HexFaceMapper->Delete();
  //this->HexFacePolyData->Delete();

  //this->HexOutline->Delete();
  //this->OutlineMapper->Delete();
  //this->OutlinePolyData->Delete();
  //
  //for (int i=0; i<7; i++)
  //  {
  //  this->HandleGeometry[i]->Delete();
  //  this->HandleMapper[i]->Delete();
  //  this->Handle[i]->Delete();
  //  }
  //delete [] this->Handle;
  //delete [] this->HandleMapper;
  //delete [] this->HandleGeometry;
  
  this->EdgePicker->Delete();
 /* this->HexPicker->Delete();

  this->Transform->Delete();
  
  this->HandleProperty->Delete();
  this->SelectedHandleProperty->Delete();
  this->FaceProperty->Delete();
  this->SelectedFaceProperty->Delete();
  this->OutlineProperty->Delete();
  this->SelectedOutlineProperty->Delete();*/
}

void vtkMimxExtractEdgeWidget::SetEnabled(int enabling)
{
  if ( ! this->Interactor )
    {
    vtkErrorMacro(<<"The interactor must be set prior to enabling/disabling widget");
    return;
    }

  if ( enabling ) //------------------------------------------------------------
    {
    vtkDebugMacro(<<"Enabling widget");

    if ( this->Enabled ) //already enabled, just return
      {
      return;
      }
    
    if ( ! this->CurrentRenderer )
      {
      this->SetCurrentRenderer(this->Interactor->FindPokedRenderer(
        this->Interactor->GetLastEventPosition()[0],
        this->Interactor->GetLastEventPosition()[1]));
      if (this->CurrentRenderer == NULL)
        {
        return;
        }
      }

    this->Enabled = 1;

    // listen to the following events
    vtkRenderWindowInteractor *i = this->Interactor;
    i->AddObserver(vtkCommand::MouseMoveEvent, this->EventCallbackCommand, 
                   this->Priority);
    i->AddObserver(vtkCommand::LeftButtonPressEvent, 
                   this->EventCallbackCommand, this->Priority);
    i->AddObserver(vtkCommand::LeftButtonReleaseEvent, 
                   this->EventCallbackCommand, this->Priority);
    i->AddObserver(vtkCommand::MiddleButtonPressEvent, 
                   this->EventCallbackCommand, this->Priority);
    i->AddObserver(vtkCommand::MiddleButtonReleaseEvent, 
                   this->EventCallbackCommand, this->Priority);
    i->AddObserver(vtkCommand::RightButtonPressEvent, 
                   this->EventCallbackCommand, this->Priority);
    i->AddObserver(vtkCommand::RightButtonReleaseEvent, 
                   this->EventCallbackCommand, this->Priority);

    // Add the various actors
    // Add the outline
    this->CurrentRenderer->AddActor(this->EdgeActor);
  if(this->InputActor)
    this->CurrentRenderer->RemoveActor(this->InputActor);
    //this->CurrentRenderer->AddActor(this->HexOutline);
    //this->HexActor->SetProperty(this->OutlineProperty);
    //this->HexOutline->SetProperty(this->OutlineProperty);

    // Add the hex face
    //this->CurrentRenderer->AddActor(this->HexFace);
    //this->HexFace->SetProperty(this->FaceProperty);

    // turn on the handles
  /*  for (int j=0; j<7; j++)
      {
      this->CurrentRenderer->AddActor(this->Handle[j]);
      this->Handle[j]->SetProperty(this->HandleProperty);
      }*/

    this->InvokeEvent(vtkCommand::EnableEvent,NULL);
    }

  else //disabling-------------------------------------------------------------
    {
    vtkDebugMacro(<<"Disabling widget");

    if ( ! this->Enabled ) //already disabled, just return
      {
      return;
      }
    
    this->Enabled = 0;

    // don't listen for events any more
    this->Interactor->RemoveObserver(this->EventCallbackCommand);

    // turn off the outline
    this->CurrentRenderer->RemoveActor(this->EdgeActor);
  if(this->InputActor)
    this->CurrentRenderer->AddActor(this->InputActor);

 //   this->CurrentRenderer->RemoveActor(this->HexOutline);

    // turn off the hex face
//    this->CurrentRenderer->RemoveActor(this->HexFace);

    // turn off the handles
 /*   for (int i=0; i<7; i++)
      {
      this->CurrentRenderer->RemoveActor(this->Handle[i]);
      }*/

    this->InvokeEvent(vtkCommand::DisableEvent,NULL);
    this->SetCurrentRenderer(NULL);
    }
  
  this->Interactor->Render();
}

void vtkMimxExtractEdgeWidget::ProcessEvents(vtkObject* vtkNotUsed(object), 
                                 unsigned long event,
                                 void* clientdata, 
                                 void* vtkNotUsed(calldata))
{
  vtkMimxExtractEdgeWidget* self = reinterpret_cast<vtkMimxExtractEdgeWidget *>( clientdata );

  //okay, let's do the right thing
  switch(event)
    {
    case vtkCommand::LeftButtonPressEvent:
      self->OnLeftButtonDown();
      break;
    case vtkCommand::LeftButtonReleaseEvent:
      self->OnLeftButtonUp();
      break;
    case vtkCommand::MiddleButtonPressEvent:
      self->OnMiddleButtonDown();
      break;
    case vtkCommand::MiddleButtonReleaseEvent:
      self->OnMiddleButtonUp();
      break;
    case vtkCommand::RightButtonPressEvent:
      self->OnRightButtonDown();
      break;
    case vtkCommand::RightButtonReleaseEvent:
      self->OnRightButtonUp();
      break;
    case vtkCommand::MouseMoveEvent:
      self->OnMouseMove();
      break;
    }
}

void vtkMimxExtractEdgeWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent); 
}

void vtkMimxExtractEdgeWidget::OnLeftButtonDown()
{
  int X = this->Interactor->GetEventPosition()[0];
  int Y = this->Interactor->GetEventPosition()[1];

  // Okay, we can process this. Try to pick handles first;
  // if no handles picked, then pick the bounding box.
  if (!this->CurrentRenderer || !this->CurrentRenderer->IsInViewport(X, Y))
    {
    this->State = vtkMimxExtractEdgeWidget::Outside;
    return;
    }
  
  vtkAssemblyPath *path;
  this->EdgePicker->Pick(X,Y,0.0,this->CurrentRenderer);
  path = this->EdgePicker->GetPath();
  if ( path != NULL )
  {
    this->State = vtkMimxExtractEdgeWidget::Moving;
    vtkIdType PickedCell = this->EdgePicker->GetCellId();
    if(PickedCell != -1)
    {
 /*     if(this->EdgePolyData->GetCellData()->GetScalars())
      {
        this->EdgePolyData->GetCellData()->GetScalars()->Delete();
      }*/
      vtkIntArray *intarray = vtkIntArray::New();
      intarray->SetNumberOfValues(this->EdgePolyData->GetNumberOfCells());
      this->EdgePolyData->GetCellData()->SetScalars(intarray);
      for(int i=0; i < this->EdgePolyData->GetNumberOfCells(); i++)  intarray->SetValue(i,0.0);
       intarray->SetValue(PickedCell,1.0);
      intarray->Delete();
      vtkIdType t;
      this->EdgePolyData->GetCellPoints(PickedCell,t,this->EdgePoints);
    }
  }
  else
  {
    this->State = vtkMimxExtractEdgeWidget::Outside;
    return;
  }
  this->EventCallbackCommand->SetAbortFlag(1);
  this->StartInteraction();
  this->InvokeEvent(vtkCommand::StartInteractionEvent, NULL);
  this->Interactor->Render();
}

void vtkMimxExtractEdgeWidget::OnLeftButtonUp()
{
  if ( this->State == vtkMimxExtractEdgeWidget::Outside ||
       this->State == vtkMimxExtractEdgeWidget::Start )
    {
    return;
    }

  this->State = vtkMimxExtractEdgeWidget::Start;
 /* this->HighlightFace(this->HighlightHandle(NULL));
  this->SizeHandles();*/

  this->EventCallbackCommand->SetAbortFlag(1);
  this->EndInteraction();
  this->InvokeEvent(vtkCommand::EndInteractionEvent, NULL);
  this->Interactor->Render();
  
}

void vtkMimxExtractEdgeWidget::OnMiddleButtonDown()
{
  //int X = this->Interactor->GetEventPosition()[0];
  //int Y = this->Interactor->GetEventPosition()[1];

  //// Okay, we can process this. Try to pick handles first;
  //// if no handles picked, then pick the bounding box.
  //if (!this->CurrentRenderer || !this->CurrentRenderer->IsInViewport(X, Y))
  //  {
  //  this->State = vtkMimxExtractEdgeWidget::Outside;
  //  return;
  //  }
  //
  //vtkAssemblyPath *path;
  //this->HandlePicker->Pick(X,Y,0.0,this->CurrentRenderer);
  //path = this->HandlePicker->GetPath();
  //if ( path != NULL )
  //  {
  //  this->State = vtkMimxExtractEdgeWidget::Moving;
  //  this->CurrentHandle = this->Handle[6];
  //  this->HighlightOutline(1);
  //  this->HandlePicker->GetPickPosition(this->LastPickPosition);
  //  this->ValidPick = 1;
  //  }
  //else
  //  {
  //  this->HexPicker->Pick(X,Y,0.0,this->CurrentRenderer);
  //  path = this->HexPicker->GetPath();
  //  if ( path != NULL )
  //    {
  //    this->State = vtkMimxExtractEdgeWidget::Moving;
  //    this->CurrentHandle = this->Handle[6];
  //    this->HighlightOutline(1);
  //    this->HexPicker->GetPickPosition(this->LastPickPosition);
  //    this->ValidPick = 1;
  //    }
  //  else
  //    {
  //    this->HighlightFace(this->HighlightHandle(NULL));
  //    this->State = vtkMimxExtractEdgeWidget::Outside;
  //    return;
  //    }
  //  }
  //
  //this->EventCallbackCommand->SetAbortFlag(1);
  //this->StartInteraction();
  //this->InvokeEvent(vtkCommand::StartInteractionEvent, NULL);
  //this->Interactor->Render();
}

void vtkMimxExtractEdgeWidget::OnMiddleButtonUp()
{
  /*if ( this->State == vtkMimxExtractEdgeWidget::Outside ||
       this->State == vtkMimxExtractEdgeWidget::Start )
    {
    return;
    }

  this->State = vtkMimxExtractEdgeWidget::Start;
  this->HighlightFace(this->HighlightHandle(NULL));
  this->SizeHandles();

  this->EventCallbackCommand->SetAbortFlag(1);
  this->EndInteraction();
  this->InvokeEvent(vtkCommand::EndInteractionEvent, NULL);
  this->Interactor->Render();*/
  
}

void vtkMimxExtractEdgeWidget::OnRightButtonDown()
{
//  int X = this->Interactor->GetEventPosition()[0];
//  int Y = this->Interactor->GetEventPosition()[1];
//
//  // Okay, we can process this. Try to pick handles first;
//  // if no handles picked, then pick the bounding box.
//  if (!this->CurrentRenderer || !this->CurrentRenderer->IsInViewport(X, Y))
//    {
//    this->State = vtkMimxExtractEdgeWidget::Outside;
//    return;
//    }
//  
//  vtkAssemblyPath *path;
//  this->HandlePicker->Pick(X,Y,0.0,this->CurrentRenderer);
//  path = this->HandlePicker->GetPath();
//  if ( path != NULL )
//    {
//    this->State = vtkMimxExtractEdgeWidget::Scaling;
//    this->HighlightOutline(1);
//    this->HandlePicker->GetPickPosition(this->LastPickPosition);
//    this->ValidPick = 1;
//    }
//  else
//    {
//    this->HexPicker->Pick(X,Y,0.0,this->CurrentRenderer);
//    path = this->HexPicker->GetPath();
//    if ( path != NULL )
//      {
//      this->State = vtkMimxExtractEdgeWidget::Scaling;
//      this->HighlightOutline(1);
//      this->HexPicker->GetPickPosition(this->LastPickPosition);
//      this->ValidPick = 1;
//      }
//    else
//      {
//      this->State = vtkMimxExtractEdgeWidget::Outside;
//      return;
//      }
//    }
//  
//  this->EventCallbackCommand->SetAbortFlag(1);
//  this->StartInteraction();
//  this->InvokeEvent(vtkCommand::StartInteractionEvent, NULL);
//  this->Interactor->Render();
//}
//
//void vtkMimxExtractEdgeWidget::OnRightButtonUp()
//{
//  if ( this->State == vtkMimxExtractEdgeWidget::Outside )
//    {
//    return;
//    }
//
//  this->State = vtkMimxExtractEdgeWidget::Start;
//  this->HighlightOutline(0);
//  this->SizeHandles();
//  
//  this->EventCallbackCommand->SetAbortFlag(1);
//  this->EndInteraction();
//  this->InvokeEvent(vtkCommand::EndInteractionEvent, NULL);
//  this->Interactor->Render();
}

void vtkMimxExtractEdgeWidget::OnMouseMove()
{
  //// See whether we're active
  //if ( this->State == vtkMimxExtractEdgeWidget::Outside || 
  //     this->State == vtkMimxExtractEdgeWidget::Start )
  //  {
  //  return;
  //  }
  //
  //int X = this->Interactor->GetEventPosition()[0];
  //int Y = this->Interactor->GetEventPosition()[1];

  //// Do different things depending on state
  //// Calculations everybody does
  //double focalPoint[4], pickPoint[4], prevPickPoint[4];
  //double z, vpn[3];

  //vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
  //if ( !camera )
  //  {
  //  return;
  //  }

  //// Compute the two points defining the motion vector
  //this->ComputeWorldToDisplay(this->LastPickPosition[0], this->LastPickPosition[1],
  //                            this->LastPickPosition[2], focalPoint);
  //z = focalPoint[2];
  //this->ComputeDisplayToWorld(double(this->Interactor->GetLastEventPosition()[0]),
  //                            double(this->Interactor->GetLastEventPosition()[1]),
  //                            z, prevPickPoint);
  //this->ComputeDisplayToWorld(double(X), double(Y), z, pickPoint);

  //// Process the motion
  //if ( this->State == vtkMimxExtractEdgeWidget::Moving )
  //  {
  //  // Okay to process
  //  if ( this->CurrentHandle )
  //    {
  //    if ( this->RotationEnabled && this->CurrentHandle == this->HexFace )
  //      {
  //      camera->GetViewPlaneNormal(vpn);
  //      this->Rotate(X, Y, prevPickPoint, pickPoint, vpn);
  //      }
  //    else if ( this->TranslationEnabled && this->CurrentHandle == this->Handle[6] )
  //      {
  //      this->Translate(prevPickPoint, pickPoint);
  //      }
  //    else if ( this->TranslationEnabled && this->ScalingEnabled ) 
  //      {
  //      if ( this->CurrentHandle == this->Handle[0] )
  //        {
  //        this->MoveMinusXFace(prevPickPoint, pickPoint);
  //        }
  //      else if ( this->CurrentHandle == this->Handle[1] )
  //        {
  //        this->MovePlusXFace(prevPickPoint, pickPoint);
  //        }
  //      else if ( this->CurrentHandle == this->Handle[2] )
  //        {
  //        this->MoveMinusYFace(prevPickPoint, pickPoint);
  //        }
  //      else if ( this->CurrentHandle == this->Handle[3] )
  //        {
  //        this->MovePlusYFace(prevPickPoint, pickPoint);
  //        }
  //      else if ( this->CurrentHandle == this->Handle[4] )
  //        {
  //        this->MoveMinusZFace(prevPickPoint, pickPoint);
  //        }
  //      else if ( this->CurrentHandle == this->Handle[5] )
  //        {
  //        this->MovePlusZFace(prevPickPoint, pickPoint);
  //        }
  //      }
  //    }
  //  }
  //else if ( this->ScalingEnabled && this->State == vtkMimxExtractEdgeWidget::Scaling )
  //  {
  //  this->Scale(prevPickPoint, pickPoint, X, Y);
  //  }

  //// Interact, if desired
  //this->EventCallbackCommand->SetAbortFlag(1);
  //this->InvokeEvent(vtkCommand::InteractionEvent, NULL);
  //this->Interactor->Render();
}

void vtkMimxExtractEdgeWidget::ExtractEdge()
{
  // the unstructured grid is converted to polydata with each edge 
  // stored as a cell in polydata
  vtkIdType* pts=0;
  vtkIdType t=0;
  this->UGrid->GetCells()->InitTraversal();
  vtkPoints* points = vtkPoints::New();
  points->SetNumberOfPoints(this->UGrid->GetNumberOfPoints());
  for(int i=0; i <this->UGrid->GetNumberOfPoints(); i++)
    points->SetPoint(i,this->UGrid->GetPoint(i));
  this->EdgePolyData->SetPoints(points);
  vtkCell* cell;
  vtkCellArray* edgelist = vtkCellArray::New();
  edgelist->InitTraversal();
  for(int i=0; i < this->UGrid->GetNumberOfCells(); i++)
  {
    if(i >0)
    {
      cell = this->UGrid->GetCell(i);
      // loop through all the edges in the hexahedron cell
      for(int j=0; j < cell->GetNumberOfEdges(); j++)
      {
        vtkCell* edgecell = cell->GetEdge(j);
        vtkIdList* pointlist = edgecell->GetPointIds();
        vtkIdType pt1 = pointlist->GetId(0);
        vtkIdType pt2 = pointlist->GetId(1);
        bool status = false;
        edgelist->InitTraversal();
        edgelist->GetNextCell(t,pts);
        do {
          // check if the edge is already present in the edge list
          if((pts[0] == pt1 && pts[1] == pt2) ||
            (pts[0] == pt2 && pts[1] == pt1))
          {
            status = true;
          }
        } while(!status && edgelist->GetNextCell(t,pts));
        if(!status)  
        {edgelist->InsertNextCell(2);
        edgelist->InsertCellPoint(pt1);
        edgelist->InsertCellPoint(pt2);
        }
      }
    }
    else
    {
      cell = this->UGrid->GetCell(i);
      for(int j=0; j < cell->GetNumberOfEdges(); j++)
      {
        vtkCell* edgecell = cell->GetEdge(j);
        vtkIdList* pointlist = edgecell->GetPointIds();
        vtkIdType pt1 = pointlist->GetId(0);
        vtkIdType pt2 = pointlist->GetId(1);
        bool status = false;
        edgelist->InsertNextCell(2);
        edgelist->InsertCellPoint(pt1);
        edgelist->InsertCellPoint(pt2);
      }
    }
  }
  this->EdgePolyData->SetLines(edgelist);
  this->EdgePolyData->Modified();
  this->EdgeActor->GetProperty()->SetLineWidth(3);
  edgelist->Delete();
  points->Delete();
}

void vtkMimxExtractEdgeWidget::Initialize()
{
  if(this->UGrid)
  {
    // if the input data changes
    if(this->EdgePolyData->GetPoints())
      this->EdgePolyData->GetPoints()->Delete();
    if (this->EdgePolyData->GetPolys())
      this->EdgePolyData->GetPolys()->Delete();
    this->EdgePolyData->Initialize();
    this->ExtractEdge();
    this->EdgePicker->AddPickList(this->EdgeActor);
    this->EdgePicker->PickFromListOn();
  }
}

void vtkMimxExtractEdgeWidget::OnRightButtonUp()
{

}

void vtkMimxExtractEdgeWidget::PlaceWidget(double bounds[6])
{

}
void vtkMimxExtractEdgeWidget::SetInput(vtkDataSet *Input)
{
  this->UGrid = vtkUnstructuredGrid::SafeDownCast(Input);
  this->Initialize();
  this->InputActor = NULL;
}
