/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxExtractMultipleFaceWidget.cxx,v $
Language:  C++

Date:      $Date: 2008/07/06 20:30:13 $
Version:   $Revision: 1.2 $


 Musculoskeletal Imaging, Modelling and Experimentation (MIMX)
 Center for Computer Aided Design
 The University of Iowa
 Iowa City, IA 52242
 http://www.ccad.uiowa.edu/mimx/
 
Copyright (c) The University of Iowa. All rights reserved.
See MIMXCopyright.txt or http://www.ccad.uiowa.edu/mimx/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "vtkMimxExtractMultipleFaceWidget.h"

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
#include "vtkIntArray.h"
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
#include "vtkGeometryFilter.h"

vtkCxxRevisionMacro(vtkMimxExtractMultipleFaceWidget, "$Revision: 1.2 $");
vtkStandardNewMacro(vtkMimxExtractMultipleFaceWidget);

vtkMimxExtractMultipleFaceWidget::vtkMimxExtractMultipleFaceWidget()
{
  this->State = vtkMimxExtractMultipleFaceWidget::Start;
  this->EventCallbackCommand->SetCallback(vtkMimxExtractMultipleFaceWidget::ProcessEvents);
  
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
  this->FacePolyData = vtkPolyData::New();
  this->FaceMapper = vtkPolyDataMapper::New();
  this->FaceMapper->SetInput(FacePolyData);
  this->FaceActor = vtkActor::New();
  this->FaceActor->SetMapper(this->FaceMapper);
  
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
  this->FacePicker = vtkCellPicker::New();
  this->FacePicker->SetTolerance(0.001);
  /* for (i=0; i<7; i++)
    {
    this->HandlePicker->AddPickList(this->Handle[i]);
    }*/
  this->FacePicker->AddPickList(this->FaceActor);
  this->FacePicker->PickFromListOn();

  //this->HexPicker = vtkCellPicker::New();
  //this->HexPicker->SetTolerance(0.001);
  //this->HexPicker->AddPickList(HexActor);
  //this->HexPicker->PickFromListOn();
  //
  //this->CurrentHandle = NULL;

  //this->Transform = vtkTransform::New();
  this->InputActor = NULL;
  this->FacePoints = vtkIntArray::New();
  this->FacePoints->SetNumberOfComponents(4);
  this->FaceScalars = vtkIntArray::New();
}

vtkMimxExtractMultipleFaceWidget::~vtkMimxExtractMultipleFaceWidget()
{
  this->FaceActor->Delete();
  this->FaceMapper->Delete();
  this->FacePolyData->Delete();
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
  
  this->FacePicker->Delete();
  this->FacePoints->Delete();
  /* this->HexPicker->Delete();

  this->Transform->Delete();
  
  this->HandleProperty->Delete();
  this->SelectedHandleProperty->Delete();
  this->FaceProperty->Delete();
  this->SelectedFaceProperty->Delete();
  this->OutlineProperty->Delete();
  this->SelectedOutlineProperty->Delete();*/
  this->FaceScalars->Delete();
}

void vtkMimxExtractMultipleFaceWidget::SetEnabled(int enabling)
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
    this->CurrentRenderer->AddActor(this->FaceActor);
    if(this->InputActor)
      this->CurrentRenderer->RemoveActor(this->InputActor);

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
    this->CurrentRenderer->RemoveActor(this->FaceActor);
    if(this->InputActor)
      this->CurrentRenderer->AddActor(this->InputActor);
 
    this->InvokeEvent(vtkCommand::DisableEvent,NULL);
    this->SetCurrentRenderer(NULL);

    }
  
  this->Interactor->Render();
}

void vtkMimxExtractMultipleFaceWidget::ProcessEvents(vtkObject* vtkNotUsed(object), 
                                                     unsigned long event,
                                                     void* clientdata, 
                                                     void* vtkNotUsed(calldata))
{
  vtkMimxExtractMultipleFaceWidget* self = reinterpret_cast<vtkMimxExtractMultipleFaceWidget *>( clientdata );

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

void vtkMimxExtractMultipleFaceWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent); 
}

void vtkMimxExtractMultipleFaceWidget::OnLeftButtonDown()
{
  int X = this->Interactor->GetEventPosition()[0];
  int Y = this->Interactor->GetEventPosition()[1];

  // Okay, we can process this. Try to pick handles first;
  // if no handles picked, then pick the bounding box.
  if (!this->CurrentRenderer || !this->CurrentRenderer->IsInViewport(X, Y))
    {
    this->State = vtkMimxExtractMultipleFaceWidget::Outside;
    return;
    }
  
  vtkAssemblyPath *path;
  this->FacePicker->Pick(X,Y,0.0,this->CurrentRenderer);
  path = this->FacePicker->GetPath();
  if ( path != NULL )
    {
    this->State = vtkMimxExtractMultipleFaceWidget::Moving;
    vtkIdType PickedCell = this->FacePicker->GetCellId();
    vtkIntArray *intarray = vtkIntArray::SafeDownCast(
      this->FacePolyData->GetCellData()->GetScalars());
    if(PickedCell != -1)
      {
      if(intarray->GetValue(PickedCell))
        {
        intarray->SetValue(PickedCell, 0);
        }
      else
        {
        intarray->SetValue(PickedCell, 1);
        }
//        this->FacePolyData->GetCellPoints(PickedCell,this->FacePoints);
      //
      this->FacePoints->Initialize();
      for (int i=0; i<this->FacePolyData->GetNumberOfCells(); i++)
        {
        if(intarray->GetValue(i))
          {
          vtkIdList *idlist = vtkIdList::New();
          this->FacePolyData->GetCellPoints(i,idlist);
          int ptlist[4];
          for (int j=0; j<4; j++)
            {
            ptlist[j] = idlist->GetId(j);
            }
          this->FacePoints->InsertNextTupleValue(ptlist);
          }
        }
      this->FacePolyData->Modified();
      }
    }
  else
    {
    this->State = vtkMimxExtractMultipleFaceWidget::Outside;
    return;
    }
  this->EventCallbackCommand->SetAbortFlag(1);
  this->StartInteraction();
  this->InvokeEvent(vtkCommand::StartInteractionEvent, NULL);
  this->Interactor->Render();
}

void vtkMimxExtractMultipleFaceWidget::OnLeftButtonUp()
{
  if ( this->State == vtkMimxExtractMultipleFaceWidget::Outside ||
       this->State == vtkMimxExtractMultipleFaceWidget::Start )
    {
    return;
    }

  this->State = vtkMimxExtractMultipleFaceWidget::Start;
  /* this->HighlightFace(this->HighlightHandle(NULL));
  this->SizeHandles();*/

  //this->EventCallbackCommand->SetAbortFlag(1);
  this->EndInteraction();
  this->InvokeEvent(vtkCommand::EndInteractionEvent, NULL);
  this->Interactor->Render();
  
}

void vtkMimxExtractMultipleFaceWidget::OnMiddleButtonDown()
{
  //int X = this->Interactor->GetEventPosition()[0];
  //int Y = this->Interactor->GetEventPosition()[1];

  //// Okay, we can process this. Try to pick handles first;
  //// if no handles picked, then pick the bounding box.
  //if (!this->CurrentRenderer || !this->CurrentRenderer->IsInViewport(X, Y))
  //  {
  //  this->State = vtkMimxExtractMultipleFaceWidget::Outside;
  //  return;
  //  }
  //
  //vtkAssemblyPath *path;
  //this->HandlePicker->Pick(X,Y,0.0,this->CurrentRenderer);
  //path = this->HandlePicker->GetPath();
  //if ( path != NULL )
  //  {
  //  this->State = vtkMimxExtractMultipleFaceWidget::Moving;
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
  //    this->State = vtkMimxExtractMultipleFaceWidget::Moving;
  //    this->CurrentHandle = this->Handle[6];
  //    this->HighlightOutline(1);
  //    this->HexPicker->GetPickPosition(this->LastPickPosition);
  //    this->ValidPick = 1;
  //    }
  //  else
  //    {
  //    this->HighlightFace(this->HighlightHandle(NULL));
  //    this->State = vtkMimxExtractMultipleFaceWidget::Outside;
  //    return;
  //    }
  //  }
  //
  //this->EventCallbackCommand->SetAbortFlag(1);
  //this->StartInteraction();
  //this->InvokeEvent(vtkCommand::StartInteractionEvent, NULL);
  //this->Interactor->Render();
}

void vtkMimxExtractMultipleFaceWidget::OnMiddleButtonUp()
{
  /*if ( this->State == vtkMimxExtractMultipleFaceWidget::Outside ||
       this->State == vtkMimxExtractMultipleFaceWidget::Start )
    {
    return;
    }

  this->State = vtkMimxExtractMultipleFaceWidget::Start;
  this->HighlightFace(this->HighlightHandle(NULL));
  this->SizeHandles();

  this->EventCallbackCommand->SetAbortFlag(1);
  this->EndInteraction();
  this->InvokeEvent(vtkCommand::EndInteractionEvent, NULL);
  this->Interactor->Render();*/
  
}

void vtkMimxExtractMultipleFaceWidget::OnRightButtonDown()
{
//  int X = this->Interactor->GetEventPosition()[0];
//  int Y = this->Interactor->GetEventPosition()[1];
//
//  // Okay, we can process this. Try to pick handles first;
//  // if no handles picked, then pick the bounding box.
//  if (!this->CurrentRenderer || !this->CurrentRenderer->IsInViewport(X, Y))
//    {
//    this->State = vtkMimxExtractMultipleFaceWidget::Outside;
//    return;
//    }
//  
//  vtkAssemblyPath *path;
//  this->HandlePicker->Pick(X,Y,0.0,this->CurrentRenderer);
//  path = this->HandlePicker->GetPath();
//  if ( path != NULL )
//    {
//    this->State = vtkMimxExtractMultipleFaceWidget::Scaling;
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
//      this->State = vtkMimxExtractMultipleFaceWidget::Scaling;
//      this->HighlightOutline(1);
//      this->HexPicker->GetPickPosition(this->LastPickPosition);
//      this->ValidPick = 1;
//      }
//    else
//      {
//      this->State = vtkMimxExtractMultipleFaceWidget::Outside;
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
//void vtkMimxExtractMultipleFaceWidget::OnRightButtonUp()
//{
//  if ( this->State == vtkMimxExtractMultipleFaceWidget::Outside )
//    {
//    return;
//    }
//
//  this->State = vtkMimxExtractMultipleFaceWidget::Start;
//  this->HighlightOutline(0);
//  this->SizeHandles();
//  
//  this->EventCallbackCommand->SetAbortFlag(1);
//  this->EndInteraction();
//  this->InvokeEvent(vtkCommand::EndInteractionEvent, NULL);
//  this->Interactor->Render();
}

void vtkMimxExtractMultipleFaceWidget::OnMouseMove()
{
  //// See whether we're active
  //if ( this->State == vtkMimxExtractMultipleFaceWidget::Outside || 
  //     this->State == vtkMimxExtractMultipleFaceWidget::Start )
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
  //if ( this->State == vtkMimxExtractMultipleFaceWidget::Moving )
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
  //else if ( this->ScalingEnabled && this->State == vtkMimxExtractMultipleFaceWidget::Scaling )
  //  {
  //  this->Scale(prevPickPoint, pickPoint, X, Y);
  //  }

  //// Interact, if desired
  //this->EventCallbackCommand->SetAbortFlag(1);
  //this->InvokeEvent(vtkCommand::InteractionEvent, NULL);
  //this->Interactor->Render();
}

void vtkMimxExtractMultipleFaceWidget::ExtractFace()
{
  vtkGeometryFilter* geofil = vtkGeometryFilter::New();
  geofil->SetInput(this->UGrid);
  geofil->Update();
  this->FacePolyData->SetPoints(geofil->GetOutput()->GetPoints());
  this->FacePolyData->SetPolys(geofil->GetOutput()->GetPolys());
  this->FaceScalars->Initialize();
  this->FaceScalars->SetNumberOfValues(this->FacePolyData->GetNumberOfCells());
  for (int i=0; i<this->FacePolyData->GetNumberOfCells(); i++)
    {
    this->FaceScalars->SetValue(i, 0);
    }
  this->FacePolyData->GetCellData()->SetScalars(this->FaceScalars);
  this->FacePolyData->Modified();
  this->FacePolyData->BuildLinks();
  this->FaceActor->GetProperty()->SetLineWidth(3);
  geofil->Delete();
}

void vtkMimxExtractMultipleFaceWidget::Initialize()
{
  if(this->UGrid)
    {
    // if the input data changes
    if(this->FacePolyData->GetPoints())
      this->FacePolyData->GetPoints()->Delete();
    if (this->FacePolyData->GetPolys())
      this->FacePolyData->GetPolys()->Delete();
    this->FacePolyData->Initialize();
    this->ExtractFace();
    this->FacePicker->AddPickList(this->FaceActor);
    this->FacePicker->PickFromListOn();
    }
}

void vtkMimxExtractMultipleFaceWidget::OnRightButtonUp()
{

}

void vtkMimxExtractMultipleFaceWidget::PlaceWidget(double vtkNotUsed(bounds)[6])
{

}
void vtkMimxExtractMultipleFaceWidget::SetInput(vtkDataSet *aInput)
{
  this->UGrid = vtkUnstructuredGrid::SafeDownCast(aInput);
  this->Initialize();
  this->InputActor = NULL;
}
