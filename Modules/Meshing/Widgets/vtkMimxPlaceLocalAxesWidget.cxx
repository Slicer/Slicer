/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkMimxPlaceLocalAxesWidget.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkMimxPlaceLocalAxesWidget.h"

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
#include "vtkIdList.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPlanes.h"
#include "vtkDataSet.h"
#include "vtkDataSetMapper.h"
#include "vtkProperty.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkSphereSource.h"
#include "vtkTransform.h"
#include "vtkUnstructuredGrid.h"
#include "vtkGeometryFilter.h"
#include "vtkLocalAxesActor.h"

#include "vtkKWEntry.h"


vtkCxxRevisionMacro(vtkMimxPlaceLocalAxesWidget, "$Revision: 1.1 $");
vtkStandardNewMacro(vtkMimxPlaceLocalAxesWidget);

vtkMimxPlaceLocalAxesWidget::vtkMimxPlaceLocalAxesWidget()
{
  this->State = vtkMimxPlaceLocalAxesWidget::Start;
  this->EventCallbackCommand->SetCallback(vtkMimxPlaceLocalAxesWidget::ProcessEvents);
  
  // Enable/disable the translation, rotation, and scaling of the widget
//  this->TranslationEnabled = 1;
//  this->RotationEnabled = 1;
//  this->ScalingEnabled = 1;

  //Build the representation of the widget
  //int i;

  // Control orientation of normals
 /* this->InsideOut = 0;
  this->OutlineCellWires = 0;
  this->OutlineCursorWires = 1;*/

  // Construct the poly data representing the hex
  this->CellDataSet = vtkUnstructuredGrid::New();
  this->CellMapper = vtkDataSetMapper::New();
  this->CellMapper->SetInput(CellDataSet);
  this->CellActor = vtkActor::New();
  this->CellActor->SetMapper(this->CellMapper);
  
  // Construct initial points
  //this->Points = vtkPoints::New(VTK_DOUBLE);
  //this->Points->SetNumberOfPoints(15);//8 corners; 6 Cells; 1 center
  //this->HexDataSet->SetPoints(this->Points);
  
  // Construct connectivity for the Cells. These are used to perform
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
  this->HexDataSet->SetPolys(cells);
  cells->Delete();
  this->HexDataSet->BuildCells();*/
  
  // The Cell of the hexahedra
  //cells = vtkCellArray::New();
  //cells->Allocate(cells->EstimateSize(1,4));
  //cells->InsertNextCell(4,pts); //temporary, replaced later
  //this->HexCellDataSet = vtkDataSet::New();
  //this->HexCellDataSet->SetPoints(this->Points);
  //this->HexCellDataSet->SetPolys(cells);
  //this->HexCellMapper = vtkDataSetMapper::New();
  //this->HexCellMapper->SetInput(HexCellDataSet);
  //this->HexCell = vtkActor::New();
  //this->HexCell->SetMapper(this->HexCellMapper);
  //cells->Delete();

  //// Create the outline for the hex
  //this->OutlineDataSet = vtkDataSet::New();
  //this->OutlineDataSet->SetPoints(this->Points);
  //this->OutlineMapper = vtkDataSetMapper::New();
  //this->OutlineMapper->SetInput(this->OutlineDataSet);
  //this->HexOutline = vtkActor::New();
  //this->HexOutline->SetMapper(this->OutlineMapper);
  //cells = vtkCellArray::New();
  //cells->Allocate(cells->EstimateSize(15,2));
  //this->OutlineDataSet->SetLines(cells);
  //cells->Delete();

  //// Set up the initial properties
  //this->CreateDefaultProperties();

  //// Create the outline
  //this->GenerateOutline();

  //// Create the handles
  //this->Handle = new vtkActor* [7];
  //this->HandleMapper = new vtkDataSetMapper* [7];
  //this->HandleGeometry = new vtkSphereSource* [7];
  //for (i=0; i<7; i++)
  //  {
  //  this->HandleGeometry[i] = vtkSphereSource::New();
  //  this->HandleGeometry[i]->SetThetaResolution(16);
  //  this->HandleGeometry[i]->SetPhiResolution(8);
  //  this->HandleMapper[i] = vtkDataSetMapper::New();
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
  this->CellPicker = vtkCellPicker::New();
  this->CellPicker->SetTolerance(0.001);
 /* for (i=0; i<7; i++)
    {
    this->HandlePicker->AddPickList(this->Handle[i]);
    }*/
  this->CellPicker->AddPickList(this->CellActor);
  this->CellPicker->PickFromListOn();

  //this->HexPicker = vtkCellPicker::New();
  //this->HexPicker->SetTolerance(0.001);
  //this->HexPicker->AddPickList(HexActor);
  //this->HexPicker->PickFromListOn();
  //
  //this->CurrentHandle = NULL;

  //this->Transform = vtkTransform::New();
  this->InputActor = NULL;
//  this->PickedCells = vtkIdList::New();
  this->LocalAxesActor = NULL;
  this->PickedCell = -1;
  this->XMeshSeed = NULL;
  this->YMeshSeed = NULL;
  this->ZMeshSeed = NULL;
}

vtkMimxPlaceLocalAxesWidget::~vtkMimxPlaceLocalAxesWidget()
{
  this->CellActor->Delete();
  this->CellMapper->Delete();
  this->CellDataSet->Delete();
  //this->Points->Delete();

  //this->HexCell->Delete();
  //this->HexCellMapper->Delete();
  //this->HexCellDataSet->Delete();

  //this->HexOutline->Delete();
  //this->OutlineMapper->Delete();
  //this->OutlineDataSet->Delete();
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
  
  this->CellPicker->Delete();
 /* this->HexPicker->Delete();

  this->Transform->Delete();
  
  this->HandleProperty->Delete();
  this->SelectedHandleProperty->Delete();
  this->CellProperty->Delete();
  this->SelectedCellProperty->Delete();
  this->OutlineProperty->Delete();
  this->SelectedOutlineProperty->Delete();*/
//  this->PickedCells->Delete();
  if(this->LocalAxesActor)
    this->LocalAxesActor->Delete();
}

void vtkMimxPlaceLocalAxesWidget::SetEnabled(int enabling)
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
  this->PickedCell = -1;

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
  this->CellActor->GetProperty()->SetLineWidth(3.0);
  this->CellActor->GetProperty()->SetRepresentationToWireframe();
    this->CurrentRenderer->AddActor(this->CellActor);
  if(this->InputActor)
    this->CurrentRenderer->RemoveActor(this->InputActor);
    //this->CurrentRenderer->AddActor(this->HexOutline);
    //this->HexActor->SetProperty(this->OutlineProperty);
    //this->HexOutline->SetProperty(this->OutlineProperty);

    // Add the hex Cell
    //this->CurrentRenderer->AddActor(this->HexCell);
    //this->HexCell->SetProperty(this->CellProperty);

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
    this->CurrentRenderer->RemoveActor(this->CellActor);
  if(this->InputActor)
    this->CurrentRenderer->AddActor(this->InputActor);
  if(this->LocalAxesActor)
    this->CurrentRenderer->RemoveViewProp(this->LocalAxesActor);
 //   this->CurrentRenderer->RemoveActor(this->HexOutline);

    // turn off the hex Cell
//    this->CurrentRenderer->RemoveActor(this->HexCell);

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

void vtkMimxPlaceLocalAxesWidget::ProcessEvents(vtkObject* vtkNotUsed(object), 
                                 unsigned long event,
                                 void* clientdata, 
                                 void* vtkNotUsed(calldata))
{
  vtkMimxPlaceLocalAxesWidget* self = reinterpret_cast<vtkMimxPlaceLocalAxesWidget *>( clientdata );

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

void vtkMimxPlaceLocalAxesWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent); 
}

void vtkMimxPlaceLocalAxesWidget::OnLeftButtonDown()
{
  int X = this->Interactor->GetEventPosition()[0];
  int Y = this->Interactor->GetEventPosition()[1];

  // Okay, we can process this. Try to pick handles first;
  // if no handles picked, then pick the bounding box.
  if (!this->CurrentRenderer || !this->CurrentRenderer->IsInViewport(X, Y))
    {
    this->State = vtkMimxPlaceLocalAxesWidget::Outside;
    return;
    }
  
  vtkAssemblyPath *path;
  this->CellPicker->Pick(X,Y,0.0,this->CurrentRenderer);
  path = this->CellPicker->GetPath();
  if ( path != NULL )
  {
    this->State = vtkMimxPlaceLocalAxesWidget::Moving;
    this->PickedCell = this->CellPicker->GetCellId();
//  this->PickedCells->SetNumberOfIds(1);
//  this->PickedCells->SetId(0,PickedCell);
    if(this->PickedCell != -1)
    {
 /*     if(this->CellDataSet->GetCellData()->GetScalars())
      {
        this->CellDataSet->GetCellData()->GetScalars()->Delete();
      }*/
      vtkIntArray *intarray = vtkIntArray::New();
      intarray->SetNumberOfValues(this->CellDataSet->GetNumberOfCells());
      this->CellDataSet->GetCellData()->SetScalars(intarray);
      for(int i=0; i < this->CellDataSet->GetNumberOfCells(); i++)  intarray->SetValue(i,0.0);
       intarray->SetValue(this->PickedCell,1.0);
      intarray->Delete();
    if(this->LocalAxesActor)
    {
      this->CurrentRenderer->RemoveViewProp(this->LocalAxesActor);
    this->LocalAxesActor->Delete();
    this->LocalAxesActor = NULL;
    }
    this->LocalAxesActor = vtkLocalAxesActor::New();
    vtkIdList *ptids = vtkIdList::New();
    this->UGrid->GetCellPoints(this->PickedCell, ptids);
    this->LocalAxesActor->SetOrigin(UGrid->GetPoint(ptids->GetId(0)));
    vtkPoints *points = vtkPoints::New();
    points->SetNumberOfPoints(3);
    points->SetPoint(0, UGrid->GetPoint(ptids->GetId(3)));
    points->SetPoint(1, UGrid->GetPoint(ptids->GetId(4)));
    points->SetPoint(2, UGrid->GetPoint(ptids->GetId(1)));
    this->LocalAxesActor->SetAxesPoints(points);
    points->Delete();
    ptids->Delete();
    this->CurrentRenderer->AddViewProp(this->LocalAxesActor);
    vtkIntArray *meshseed = vtkIntArray::SafeDownCast(
      this->UGrid->GetCellData()->GetArray("Mesh_Seed"));
    if(meshseed)
    {
      if(this->XMeshSeed && this->YMeshSeed && this->ZMeshSeed)
      {
        int dim[3];
      meshseed->GetTupleValue(this->PickedCell, dim);
      this->XMeshSeed->GetWidget()->SetValueAsInt(dim[0]);
      this->YMeshSeed->GetWidget()->SetValueAsInt(dim[1]);
      this->ZMeshSeed->GetWidget()->SetValueAsInt(dim[2]);
      this->XMeshSeed->SetEnabled(1);
      this->YMeshSeed->SetEnabled(1);
      this->ZMeshSeed->SetEnabled(1);
      }
    }

  }
  }
  else
  {
    this->State = vtkMimxPlaceLocalAxesWidget::Outside;
    return;
  }
  this->EventCallbackCommand->SetAbortFlag(1);
  this->StartInteraction();
  this->InvokeEvent(vtkCommand::StartInteractionEvent, NULL);
  this->Interactor->Render();
}

void vtkMimxPlaceLocalAxesWidget::OnLeftButtonUp()
{
  if ( this->State == vtkMimxPlaceLocalAxesWidget::Outside ||
       this->State == vtkMimxPlaceLocalAxesWidget::Start )
    {
    return;
    }

  this->State = vtkMimxPlaceLocalAxesWidget::Start;
 /* this->HighlightCell(this->HighlightHandle(NULL));
  this->SizeHandles();*/

  this->EventCallbackCommand->SetAbortFlag(1);
  this->EndInteraction();
  this->InvokeEvent(vtkCommand::EndInteractionEvent, NULL);
  this->Interactor->Render();
  
}

void vtkMimxPlaceLocalAxesWidget::OnMiddleButtonDown()
{
  //int X = this->Interactor->GetEventPosition()[0];
  //int Y = this->Interactor->GetEventPosition()[1];

  //// Okay, we can process this. Try to pick handles first;
  //// if no handles picked, then pick the bounding box.
  //if (!this->CurrentRenderer || !this->CurrentRenderer->IsInViewport(X, Y))
  //  {
  //  this->State = vtkMimxPlaceLocalAxesWidget::Outside;
  //  return;
  //  }
  //
  //vtkAssemblyPath *path;
  //this->HandlePicker->Pick(X,Y,0.0,this->CurrentRenderer);
  //path = this->HandlePicker->GetPath();
  //if ( path != NULL )
  //  {
  //  this->State = vtkMimxPlaceLocalAxesWidget::Moving;
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
  //    this->State = vtkMimxPlaceLocalAxesWidget::Moving;
  //    this->CurrentHandle = this->Handle[6];
  //    this->HighlightOutline(1);
  //    this->HexPicker->GetPickPosition(this->LastPickPosition);
  //    this->ValidPick = 1;
  //    }
  //  else
  //    {
  //    this->HighlightCell(this->HighlightHandle(NULL));
  //    this->State = vtkMimxPlaceLocalAxesWidget::Outside;
  //    return;
  //    }
  //  }
  //
  //this->EventCallbackCommand->SetAbortFlag(1);
  //this->StartInteraction();
  //this->InvokeEvent(vtkCommand::StartInteractionEvent, NULL);
  //this->Interactor->Render();
}

void vtkMimxPlaceLocalAxesWidget::OnMiddleButtonUp()
{
  /*if ( this->State == vtkMimxPlaceLocalAxesWidget::Outside ||
       this->State == vtkMimxPlaceLocalAxesWidget::Start )
    {
    return;
    }

  this->State = vtkMimxPlaceLocalAxesWidget::Start;
  this->HighlightCell(this->HighlightHandle(NULL));
  this->SizeHandles();

  this->EventCallbackCommand->SetAbortFlag(1);
  this->EndInteraction();
  this->InvokeEvent(vtkCommand::EndInteractionEvent, NULL);
  this->Interactor->Render();*/
  
}

void vtkMimxPlaceLocalAxesWidget::OnRightButtonDown()
{
//  int X = this->Interactor->GetEventPosition()[0];
//  int Y = this->Interactor->GetEventPosition()[1];
//
//  // Okay, we can process this. Try to pick handles first;
//  // if no handles picked, then pick the bounding box.
//  if (!this->CurrentRenderer || !this->CurrentRenderer->IsInViewport(X, Y))
//    {
//    this->State = vtkMimxPlaceLocalAxesWidget::Outside;
//    return;
//    }
//  
//  vtkAssemblyPath *path;
//  this->HandlePicker->Pick(X,Y,0.0,this->CurrentRenderer);
//  path = this->HandlePicker->GetPath();
//  if ( path != NULL )
//    {
//    this->State = vtkMimxPlaceLocalAxesWidget::Scaling;
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
//      this->State = vtkMimxPlaceLocalAxesWidget::Scaling;
//      this->HighlightOutline(1);
//      this->HexPicker->GetPickPosition(this->LastPickPosition);
//      this->ValidPick = 1;
//      }
//    else
//      {
//      this->State = vtkMimxPlaceLocalAxesWidget::Outside;
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
//void vtkMimxPlaceLocalAxesWidget::OnRightButtonUp()
//{
//  if ( this->State == vtkMimxPlaceLocalAxesWidget::Outside )
//    {
//    return;
//    }
//
//  this->State = vtkMimxPlaceLocalAxesWidget::Start;
//  this->HighlightOutline(0);
//  this->SizeHandles();
//  
//  this->EventCallbackCommand->SetAbortFlag(1);
//  this->EndInteraction();
//  this->InvokeEvent(vtkCommand::EndInteractionEvent, NULL);
//  this->Interactor->Render();
}

void vtkMimxPlaceLocalAxesWidget::OnMouseMove()
{
  //// See whether we're active
  //if ( this->State == vtkMimxPlaceLocalAxesWidget::Outside || 
  //     this->State == vtkMimxPlaceLocalAxesWidget::Start )
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
  //if ( this->State == vtkMimxPlaceLocalAxesWidget::Moving )
  //  {
  //  // Okay to process
  //  if ( this->CurrentHandle )
  //    {
  //    if ( this->RotationEnabled && this->CurrentHandle == this->HexCell )
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
  //        this->MoveMinusXCell(prevPickPoint, pickPoint);
  //        }
  //      else if ( this->CurrentHandle == this->Handle[1] )
  //        {
  //        this->MovePlusXCell(prevPickPoint, pickPoint);
  //        }
  //      else if ( this->CurrentHandle == this->Handle[2] )
  //        {
  //        this->MoveMinusYCell(prevPickPoint, pickPoint);
  //        }
  //      else if ( this->CurrentHandle == this->Handle[3] )
  //        {
  //        this->MovePlusYCell(prevPickPoint, pickPoint);
  //        }
  //      else if ( this->CurrentHandle == this->Handle[4] )
  //        {
  //        this->MoveMinusZCell(prevPickPoint, pickPoint);
  //        }
  //      else if ( this->CurrentHandle == this->Handle[5] )
  //        {
  //        this->MovePlusZCell(prevPickPoint, pickPoint);
  //        }
  //      }
  //    }
  //  }
  //else if ( this->ScalingEnabled && this->State == vtkMimxPlaceLocalAxesWidget::Scaling )
  //  {
  //  this->Scale(prevPickPoint, pickPoint, X, Y);
  //  }

  //// Interact, if desired
  //this->EventCallbackCommand->SetAbortFlag(1);
  //this->InvokeEvent(vtkCommand::InteractionEvent, NULL);
  //this->Interactor->Render();
}

void vtkMimxPlaceLocalAxesWidget::Initialize()
{
  if(this->UGrid)
  {
    // if the input data changes
    this->CellDataSet->SetPoints(this->UGrid->GetPoints());
  this->CellDataSet->SetCells(12, this->UGrid->GetCells());
  this->CellActor->Modified();
    this->CellPicker->AddPickList(this->CellActor);
    this->CellPicker->PickFromListOn();
  }
}

void vtkMimxPlaceLocalAxesWidget::OnRightButtonUp()
{

}

void vtkMimxPlaceLocalAxesWidget::PlaceWidget(double bounds[6])
{

}
void vtkMimxPlaceLocalAxesWidget::SetInput(vtkDataSet *Input)
{
  this->UGrid = vtkUnstructuredGrid::SafeDownCast(Input);
  this->Initialize();
  this->InputActor = NULL;
}
