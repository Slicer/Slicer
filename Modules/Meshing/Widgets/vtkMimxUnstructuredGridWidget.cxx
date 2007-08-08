/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxUnstructuredGridWidget.cxx,v $
Language:  C++
Date:      $Date: 2007/08/01 15:55:40 $
Version:   $Revision: 1.13 $

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

#include "vtkMimxUnstructuredGridWidget.h"

#include "vtkActor.h"
#include "vtkAssemblyNode.h"
#include "vtkAssemblyPath.h"
#include "vtkCallbackCommand.h"
#include "vtkCamera.h"
#include "vtkCellArray.h"
#include "vtkCellPicker.h"
#include "vtkCollection.h"
#include "vtkDoubleArray.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPlanes.h"
#include "vtkMimxModPointWidget.h"
#include "vtkUnstructuredGrid.h"
#include "vtkDataSetMapper.h"
#include "vtkProperty.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkSphereSource.h"
#include "vtkTransform.h"

vtkCxxRevisionMacro(vtkMimxUnstructuredGridWidget, "$Revision: 1.13 $");
vtkStandardNewMacro(vtkMimxUnstructuredGridWidget);

class vtkUGPWCallback : public vtkCommand
{
public:
  static vtkUGPWCallback *New() 
  { return new vtkUGPWCallback; }
  virtual void Execute(vtkObject *vtkNotUsed(caller), unsigned long, void*)
  {
    double x[3];
    this->PointWidget->GetPosition(x);
    this->UnstructuredGridWidget->SetPoint(this->HandleNumber,x);
  }
  vtkUGPWCallback():UnstructuredGridWidget(0),PointWidget(0) {}
  vtkMimxUnstructuredGridWidget  *UnstructuredGridWidget;
  vtkMimxModPointWidget *PointWidget;
  int HandleNumber;
};

vtkMimxUnstructuredGridWidget::vtkMimxUnstructuredGridWidget()
{
  this->State = vtkMimxUnstructuredGridWidget::Start;
  this->EventCallbackCommand->SetCallback(vtkMimxUnstructuredGridWidget::ProcessEvents);

  //Build the representation of the widget

  // Control orientation of normals
  this->InsideOut = 0;
  this->OutlineFaceWires = 0;
  this->OutlineCursorWires = 1;

  // Construct the poly data representing the hex
  this->HexMapper = vtkDataSetMapper::New();
  this->HexActor = vtkActor::New();

  // Create the outline for the hex
//  this->OutlineUnstructuredGrid = vtkUnstructuredGrid::New();
//  this->OutlineMapper = vtkDataSetMapper::New();
  this->HexOutline = vtkActor::New();

  // Set up the initial properties
  this->CreateDefaultProperties();

  // Create the handles
  this->Handle = vtkCollection::New();
//  this->Handle = new vtkActor* [8];
  this->HandleMapper = vtkCollection::New();
//  this->HandleMapper = new vtkDataSetMapper* [8];
  this->HandleGeometry = vtkCollection::New();
//  this->HandleGeometry = new vtkSphereSource* [8];
  this->PointWidget = vtkCollection::New();
  
  
  //  
  this->PWCallback = vtkUGPWCallback::New();
  this->PWCallback->UnstructuredGridWidget = this;

  

  //Manage the picking stuff
  this->HandlePicker = vtkCellPicker::New();
  this->HandlePicker->SetTolerance(0.001);
  

  this->HexPicker = vtkCellPicker::New();
  this->HexPicker->SetTolerance(0.001);
  this->HexPicker->AddPickList(HexActor);
  this->HexPicker->PickFromListOn();
  this->HandleSize = 1.0;  
  this->CurrentHandle = NULL;
  this->UGrid = NULL;
}

vtkMimxUnstructuredGridWidget::~vtkMimxUnstructuredGridWidget()
{
  this->HexActor->Delete();
  this->HexMapper->Delete();

  this->HexOutline->Delete();
//  this->OutlineMapper->Delete();
//  this->OutlineUnstructuredGrid->Delete();
  
  this->HandleGeometry->Delete();
  this->HandleMapper->Delete();
  this->Handle->Delete();

  this->HandlePicker->Delete();
  this->HexPicker->Delete();

  this->HandleProperty->Delete();
  this->SelectedHandleProperty->Delete();
  this->OutlineProperty->Delete();
  this->SelectedOutlineProperty->Delete();
}

void vtkMimxUnstructuredGridWidget::SetEnabled(int enabling)
{
  if ( ! this->Interactor )
  {
    vtkErrorMacro(<<"The interactor must be set prior to enabling/disabling widget");
    return;
  }

  if ( enabling ) //------------------------------------------------------------
  {
    vtkDebugMacro(<<"Enabling widget");

    if ( this->Enabled || !this->UGrid) //already enabled, just return
    {
      return;
    }

    if ( ! this->CurrentRenderer )
    {
      this->CurrentRenderer = this->Interactor->FindPokedRenderer(
        this->Interactor->GetLastEventPosition()[0],
        this->Interactor->GetLastEventPosition()[1]);
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
    this->CurrentRenderer->AddActor(this->HexActor);
    this->CurrentRenderer->AddActor(this->HexOutline);
    this->HexActor->SetProperty(this->OutlineProperty);
    this->HexOutline->SetProperty(this->OutlineProperty);

    // turn on the handles
    for (int j=0; j<this->GetUGrid()->GetNumberOfPoints(); j++)
    {
      this->CurrentRenderer->AddActor((vtkActor*) this->Handle->GetItemAsObject(j));
      ((vtkActor*) (this->Handle->GetItemAsObject(j)))->SetProperty(this->HandleProperty);
    }
    this->PositionHandles();
    this->SizeHandles();
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
    this->CurrentRenderer->RemoveActor(this->HexActor);
    this->CurrentRenderer->RemoveActor(this->HexOutline);

    // turn off the handles
    for (int i=0; i<this->GetUGrid()->GetNumberOfPoints(); i++)
    {
      this->CurrentRenderer->RemoveActor((vtkActor*)
        this->Handle->GetItemAsObject(i));
    }

    this->CurrentHandle = NULL;
    this->InvokeEvent(vtkCommand::DisableEvent,NULL);
    this->CurrentRenderer = NULL;
//    this->UGrid = NULL;
  }

  this->Interactor->Render();
}

void vtkMimxUnstructuredGridWidget::Execute()
{
  this->SetEnabled(0);
if(this->UGrid)
{
  this->Initialize();
  }
this->HexMapper->SetInput(this->UGrid);
this->HexActor->SetMapper(this->HexMapper);

    for (int i=0; i<this->UGrid->GetNumberOfPoints(); i++)
    {
      this->HandleGeometry->AddItem((vtkObject*) vtkSphereSource::New());
      ((vtkSphereSource*)(this->HandleGeometry->GetItemAsObject(i)))->SetThetaResolution(16);
      ((vtkSphereSource*)(this->HandleGeometry->GetItemAsObject(i)))->SetPhiResolution(8);
      this->HandleMapper->AddItem((vtkObject*) vtkDataSetMapper::New());
      ((vtkDataSetMapper*)(this->HandleMapper->GetItemAsObject(i)))->
        SetInput(((vtkSphereSource*)(this->HandleGeometry->GetItemAsObject(i)))->
        GetOutput());
      this->Handle->AddItem((vtkObject*) vtkActor::New());
      ((vtkActor*) (this->Handle->GetItemAsObject(i)))->SetMapper
        ((vtkDataSetMapper*) this->HandleMapper->GetItemAsObject(i));
    }
    //  Pointwidget declarations
    for (int i=0; i < this->UGrid->GetNumberOfPoints(); i++)
    {
      this->PointWidget->AddItem((vtkObject*) vtkMimxModPointWidget::New());
      ((vtkMimxModPointWidget*) (this->PointWidget->GetItemAsObject(i)))->AllOff();
      ((vtkMimxModPointWidget*) (this->PointWidget->GetItemAsObject(i)))
        ->SetHotSpotSize(0.5);
    }

    this->PWCallback->PointWidget = (vtkMimxModPointWidget*) 
      (this->PointWidget->GetItemAsObject(0));

    for (int i=0; i < this->UGrid->GetNumberOfPoints(); i++)
    {
      this->HandlePicker->AddPickList(
        (vtkActor*) this->Handle->GetItemAsObject(i));
    }
    this->HandlePicker->PickFromListOn();

    for(int i=0; i < this->UGrid->GetNumberOfPoints(); i++)
    {
      (vtkMimxModPointWidget*) (this->PointWidget->GetItemAsObject(i))
        ->AddObserver(vtkCommand::InteractionEvent,this->PWCallback, 0.0);
    }
    this->PositionHandles();
    this->SizeHandles();
    this->SetEnabled(1);
}
void vtkMimxUnstructuredGridWidget::ProcessEvents(vtkObject* vtkNotUsed(object), 
                 unsigned long event,
                 void* clientdata, 
                 void* vtkNotUsed(calldata))
{
  vtkMimxUnstructuredGridWidget* self = reinterpret_cast<vtkMimxUnstructuredGridWidget *>( clientdata );

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

void vtkMimxUnstructuredGridWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  if ( this->HandleProperty )
  {
    os << indent << "Handle Property: " << this->HandleProperty << "\n";
  }
  else
  {
    os << indent << "Handle Property: (none)\n";
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

  os << indent << "Outline Face Wires: " << (this->OutlineFaceWires ? "On\n" : "Off\n");
  os << indent << "Outline Cursor Wires: " << (this->OutlineCursorWires ? "On\n" : "Off\n");
  os << indent << "Inside Out: " << (this->InsideOut ? "On\n" : "Off\n");
}

void vtkMimxUnstructuredGridWidget::PositionHandles()
{
  for(int i=0; i < this->UGrid->GetNumberOfPoints(); i++)
  {
    double x[3];
    this->UGrid->GetPoint(i,x);
//    cout <<i<<" "<<x[0]<<"  "<<x[1]<<"  "<<x[2]<<endl;
    ((vtkSphereSource*) (this->HandleGeometry->GetItemAsObject(i)))->
      SetCenter(this->UGrid->GetPoint(i));
  }
}


void vtkMimxUnstructuredGridWidget::SizeHandles()
{
  for(int i=0; i< this->UGrid->GetNumberOfPoints(); i++)
  {
    ((vtkSphereSource*) (this->HandleGeometry->GetItemAsObject(i)))
      ->SetRadius(this->HandleSize);
  }
}

int vtkMimxUnstructuredGridWidget::HighlightHandle(vtkProp *prop)
{
  // first unhighlight anything picked
  if ( this->CurrentHandle )
  {
    this->CurrentHandle->SetProperty(this->HandleProperty);
  }

  // set the current handle
  this->CurrentHandle = (vtkActor *)prop;

  // find the current handle
  if ( this->CurrentHandle )
  {
    this->ValidPick = 1;
    this->HandlePicker->GetPickPosition(this->LastPickPosition);
    this->CurrentHandle->SetProperty(this->SelectedHandleProperty);
    return 1;
  }
  else
  {
    return -1;
  }
}


void vtkMimxUnstructuredGridWidget::HighlightOutline(int highlight)
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

void vtkMimxUnstructuredGridWidget::OnLeftButtonDown()
{
  int X = this->Interactor->GetEventPosition()[0];
  int Y = this->Interactor->GetEventPosition()[1];

  // Okay, we can process this. Try to pick handles first;
  // if no handles picked, then pick the bounding box.
  vtkRenderer *ren = this->Interactor->FindPokedRenderer(X,Y);
  if ( ren != this->CurrentRenderer )
  {
    this->State = vtkMimxUnstructuredGridWidget::Outside;
    return;
  }

  int forward=0;

  
  // Okay, we can process this. Try to pick handles first;
  // if no handles picked, then try to pick the line.
  vtkAssemblyPath *path;
  this->HandlePicker->Pick(X,Y,0.0,this->CurrentRenderer);
  path = this->HandlePicker->GetPath();
  if ( path != NULL )
  {
    this->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
    this->State = vtkMimxUnstructuredGridWidget::MovingHandle;
    this->HighlightHandle(path->GetFirstNode()->GetViewProp());
    this->EnablePointWidget();
    forward = this->ForwardEvent(vtkCommand::LeftButtonPressEvent);
  }
  else
  {
    this->State = vtkMimxUnstructuredGridWidget::Outside;
    return;
  }
  this->EventCallbackCommand->SetAbortFlag(1);
  this->StartInteraction();
  if ( ! forward )
  {
    this->Interactor->Render();
  }
}

void vtkMimxUnstructuredGridWidget::OnLeftButtonUp()
{
  if ( this->State == vtkMimxUnstructuredGridWidget::Outside ||
    this->State == vtkMimxUnstructuredGridWidget::Start )
  {
    return;
  }

  this->State = vtkMimxUnstructuredGridWidget::Start;
  this->HighlightHandle(NULL);

  this->SizeHandles();

  int forward = this->ForwardEvent(vtkCommand::LeftButtonReleaseEvent);
  this->DisablePointWidget();

  this->EventCallbackCommand->SetAbortFlag(1);
  this->EndInteraction();
  this->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
  if ( ! forward )
  {
    this->Interactor->Render();
  }

}

void vtkMimxUnstructuredGridWidget::OnMiddleButtonDown()
{
  int X = this->Interactor->GetEventPosition()[0];
  int Y = this->Interactor->GetEventPosition()[1];

  // Okay, we can process this. Try to pick handles first;
  // if no handles picked, then pick the bounding box.
  vtkRenderer *ren = this->Interactor->FindPokedRenderer(X,Y);
  if ( ren != this->CurrentRenderer )
  {
    this->State = vtkMimxUnstructuredGridWidget::Outside;
    return;
  }

  vtkAssemblyPath *path;
  this->HandlePicker->Pick(X,Y,0.0,this->CurrentRenderer);
  path = this->HandlePicker->GetPath();
  if ( path != NULL )
  {
    this->State = vtkMimxUnstructuredGridWidget::MovingHandle;
//    this->CurrentHandle = this->Handle[6];
    this->HighlightOutline(1);
    this->HandlePicker->GetPickPosition(this->LastPickPosition);
    this->ValidPick = 1;
  }
  else
  {
    this->HexPicker->Pick(X,Y,0.0,this->CurrentRenderer);
    path = this->HexPicker->GetPath();
    if ( path != NULL )
    {
      this->State = vtkMimxUnstructuredGridWidget::MovingHandle;
//      this->CurrentHandle = this->Handle[6];
      this->HighlightOutline(1);
      this->HexPicker->GetPickPosition(this->LastPickPosition);
      this->ValidPick = 1;
    }
    else
    {
      this->State = vtkMimxUnstructuredGridWidget::Outside;
      return;
    }
  }

  this->EventCallbackCommand->SetAbortFlag(1);
  this->StartInteraction();
  this->InvokeEvent(vtkCommand::StartInteractionEvent, NULL);
  this->Interactor->Render();
}

void vtkMimxUnstructuredGridWidget::OnMiddleButtonUp()
{
  if ( this->State == vtkMimxUnstructuredGridWidget::Outside ||
    this->State == vtkMimxUnstructuredGridWidget::Start )
  {
    return;
  }

  this->State = vtkMimxUnstructuredGridWidget::Start;
  this->SizeHandles();

  this->EventCallbackCommand->SetAbortFlag(1);
  this->EndInteraction();
  this->InvokeEvent(vtkCommand::EndInteractionEvent, NULL);
  this->Interactor->Render();

}

void vtkMimxUnstructuredGridWidget::OnRightButtonDown()
{
/*  int X = this->Interactor->GetEventPosition()[0];
  int Y = this->Interactor->GetEventPosition()[1];

  // Okay, we can process this. Try to pick handles first;
  // if no handles picked, then pick the bounding box.
  vtkRenderer *ren = this->Interactor->FindPokedRenderer(X,Y);
  if ( ren != this->CurrentRenderer )
  {
    this->State = vtkMimxUnstructuredGridWidget::Outside;
    return;
  }

  //int forward=0;


  //// Okay, we can process this. Try to pick handles first;
  //// if no handles picked, then try to pick the line.
  //vtkAssemblyPath *path;
  //this->HandlePicker->Pick(X,Y,0.0,this->CurrentRenderer);
  //path = this->HandlePicker->GetPath();
  //if ( path != NULL )
  //{
    this->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
    this->State = vtkMimxUnstructuredGridWidget::Translate;
    //forward = this->ForwardEvent(vtkCommand::RightButtonPressEvent);
  //}

  this->EventCallbackCommand->SetAbortFlag(1);
  this->StartInteraction();
  //if ( ! forward )
  //{
    this->Interactor->Render();
  //}*/
}

void vtkMimxUnstructuredGridWidget::OnRightButtonUp()
{
/*  if ( this->State == vtkMimxUnstructuredGridWidget::Outside ||
    this->State == vtkMimxUnstructuredGridWidget::Start )
  {
    return;
  }

  this->State = vtkMimxUnstructuredGridWidget::Start;


  //int forward = this->ForwardEvent(vtkCommand::RightButtonReleaseEvent);

  this->EventCallbackCommand->SetAbortFlag(1);
  this->EndInteraction();
  this->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
  //if ( ! forward )
  //{
    //this->Interactor->Render();
  //}*/
}

void vtkMimxUnstructuredGridWidget::OnMouseMove()
{
  // See whether we're active
  if ( this->State == vtkMimxUnstructuredGridWidget::Outside || 
    this->State == vtkMimxUnstructuredGridWidget::Start )
  {
    return;
  }

  int X = this->Interactor->GetEventPosition()[0];
  int Y = this->Interactor->GetEventPosition()[1];

  // Do different things depending on state
  // Calculations everybody does
  double focalPoint[4], pickPoint[4], prevPickPoint[4];
  double z;

  vtkRenderer *renderer = this->Interactor->FindPokedRenderer(X,Y);
  vtkCamera *camera = renderer->GetActiveCamera();
  if ( !camera )
  {
    return;
  }

  // Compute the two points defining the motion vector
  this->ComputeWorldToDisplay(this->LastPickPosition[0], this->LastPickPosition[1],
    this->LastPickPosition[2], focalPoint);
  z = focalPoint[2];
  this->ComputeDisplayToWorld(double(this->Interactor->GetLastEventPosition()[0]),
    double(this->Interactor->GetLastEventPosition()[1]),
    z, prevPickPoint);
  this->ComputeDisplayToWorld(double(X), double(Y), z, pickPoint);

  // Process the motion
  int forward=0;
  if ( this->State == vtkMimxUnstructuredGridWidget::MovingHandle )
  {
    forward = this->ForwardEvent(vtkCommand::MouseMoveEvent);
  }
  else
  {
    this->TranslateProp(prevPickPoint, pickPoint);
  }
  this->EventCallbackCommand->SetAbortFlag(1);
  this->InvokeEvent(vtkCommand::InteractionEvent,NULL);
  if(this->State == vtkMimxUnstructuredGridWidget::MovingHandle)
  {
    if ( ! forward )
    {
      this->Interactor->Render();
    }
  }
  else{
    this->Interactor->Render();
  }
}

void vtkMimxUnstructuredGridWidget::CreateDefaultProperties()
{
  // Handle properties
  this->HandleProperty = vtkProperty::New();
  this->HandleProperty->SetColor(0,1,0);

  this->SelectedHandleProperty = vtkProperty::New();
  this->SelectedHandleProperty->SetColor(1,0,0);  

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

void vtkMimxUnstructuredGridWidget::PlaceWidget(double bds[6])
{
}


void vtkMimxUnstructuredGridWidget::GenerateOutline()
{
  // Now the outline lines
/*  if ( ! this->OutlineFaceWires && ! this->OutlineCursorWires )
  {
    return;
  }

  vtkIdType pts[2];
  vtkCellArray *cells = this->OutlineUnstructuredGrid->GetLines();
  cells->Reset();

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
  this->OutlineUnstructuredGrid->Modified();
  if ( this->OutlineProperty) 
  {
    this->OutlineProperty->SetRepresentationToWireframe();
    this->SelectedOutlineProperty->SetRepresentationToWireframe();
  }*/
}

int vtkMimxUnstructuredGridWidget::ForwardEvent(unsigned long event)
{
  if ( ! this->CurrentPointWidget )
  {
    return 0;
  }

  this->CurrentPointWidget->ProcessEvents(this,event,
    this->CurrentPointWidget,NULL);

  return 1;
}

void vtkMimxUnstructuredGridWidget::EnablePointWidget()
{
  // Set up the point widgets
  double x[3];
  if ( this->CurrentHandle ) //picking the handles
  {
    bool status = false;
    int count = 0;
    do {
      if ( this->CurrentHandle == (vtkActor*) 
        this->Handle->GetItemAsObject(count) )
      {
        this->CurrentPointWidget = (vtkMimxModPointWidget*) 
          (this->PointWidget->GetItemAsObject(count));
        this->UGrid->GetPoint(count,x);
        status = true;
        PWCallback->HandleNumber = count;
        PWCallback->PointWidget = this->CurrentPointWidget;
      }
      count++;
    } while(count < this->UGrid->GetNumberOfPoints() && status == false);

  }
  
  double bounds[6];
  for (int i=0; i<3; i++)
  {
    bounds[2*i] = x[i] - 0.1*this->InitialLength;
    bounds[2*i+1] = x[i] + 0.1*this->InitialLength;
  }
    bounds[0] = 0; bounds[2] = 0; bounds[4] = 0;
  bounds[1] = 1; bounds[3] = 3; bounds[5] = 5;
  // Note: translation mode is disabled and enabled to control
  // the proper positioning of the bounding box.
  this->CurrentPointWidget->SetInteractor(this->Interactor);
  this->CurrentPointWidget->TranslationModeOff();
  this->CurrentPointWidget->SetPlaceFactor(1.0);
  this->CurrentPointWidget->PlaceWidget(bounds);
  this->CurrentPointWidget->TranslationModeOn();
  this->CurrentPointWidget->SetPosition(x);
  this->CurrentPointWidget->On();
}

void vtkMimxUnstructuredGridWidget::DisablePointWidget()
{
  this->CurrentPointWidget->Off();
  this->CurrentPointWidget = NULL;
}

void vtkMimxUnstructuredGridWidget::SetPoint(int i, double x[3])
{
//  double x1[3];
  this->UGrid->GetPoints()->SetPoint(i,x);
  this->UGrid->Modified();
  ((vtkSphereSource*) (this->HandleGeometry->GetItemAsObject(i)))->SetCenter(x);
  ((vtkSphereSource*) (this->HandleGeometry->GetItemAsObject(i)))->Update();
}

void vtkMimxUnstructuredGridWidget::Initialize()
{
  this->HandleGeometry->InitTraversal();
  this->HandleMapper->InitTraversal();
  this->Handle->InitTraversal();
  this->PointWidget->InitTraversal();
  vtkIdType numPts = this->HandleGeometry->GetNumberOfItems();
  while (numPts !=0) {
    this->HandleGeometry->GetNextItemAsObject()->Delete();
    this->HandleMapper->GetNextItemAsObject()->Delete();
    this->Handle->GetNextItemAsObject()->Delete();
    this->PointWidget->GetNextItemAsObject()->Delete();
    numPts--;
  }
  this->HandleGeometry->RemoveAllItems();
  this->HandleMapper->RemoveAllItems();
  this->Handle->RemoveAllItems();
  this->PointWidget->RemoveAllItems();
  this->HandlePicker->InitializePickList();

}

void vtkMimxUnstructuredGridWidget::TranslateProp(double p1[3], double p2[3])
{
  double x[3];
  for(int i=0; i <this->UGrid->GetNumberOfPoints(); i++)
  {
    this->UGrid->GetPoint(i,x);
    for(int j=0; j<3; j++)  x[j] = x[j] + p2[j]-p1[j];
    this->UGrid->GetPoints()->SetPoint(i,x);
  }
  this->UGrid->Modified();
  this->PositionHandles();
}
