/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxBoundingBoxWidget.cxx,v $
Language:  C++
Date:      $Date: 2007/05/17 16:30:26 $
Version:   $Revision: 1.7 $

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

#include "vtkMimxBoundingBoxWidget.h"

#include "vtkActor.h"
#include "vtkAssemblyNode.h"
#include "vtkAssemblyPath.h"
#include "vtkCallbackCommand.h"
#include "vtkCamera.h"
#include "vtkCellArray.h"
#include "vtkCellPicker.h"
#include "vtkCollection.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPlanes.h"
#include "vtkMimxModPointWidget.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkSphereSource.h"
#include "vtkTransform.h"

vtkCxxRevisionMacro(vtkMimxBoundingBoxWidget, "$Revision: 1.7 $");
vtkStandardNewMacro(vtkMimxBoundingBoxWidget);

class vtkBBWPWCallback : public vtkCommand
{
public:
        static vtkBBWPWCallback *New() 
        { return new vtkBBWPWCallback; }
        virtual void Execute(vtkObject *vtkNotUsed(caller), unsigned long, void*)
        {
                double x[3];
                this->PointWidget->GetPosition(x);
                this->BoundingBoxWidget->SetPoint(this->HandleNumber,x);
        }
        vtkBBWPWCallback():BoundingBoxWidget(0),PointWidget(0) {}
        vtkMimxBoundingBoxWidget  *BoundingBoxWidget;
        vtkMimxModPointWidget *PointWidget;
        int HandleNumber;
};

vtkMimxBoundingBoxWidget::vtkMimxBoundingBoxWidget()
{
        this->State = vtkMimxBoundingBoxWidget::Start;
        this->EventCallbackCommand->SetCallback(vtkMimxBoundingBoxWidget::ProcessEvents);

        //Build the representation of the widget
        int i;

        // Control orientation of normals
        this->InsideOut = 0;
        this->OutlineFaceWires = 0;
        this->OutlineCursorWires = 1;

        // Construct the poly data representing the hex
        this->HexPolyData = vtkPolyData::New();
        this->HexMapper = vtkPolyDataMapper::New();
        this->HexMapper->SetInput(HexPolyData);
        this->HexActor = vtkActor::New();
        this->HexActor->SetMapper(this->HexMapper);

        // Construct initial points
        this->Points = vtkPoints::New(VTK_DOUBLE);
        this->Points->SetNumberOfPoints(8);//8 corners; 6 faces; 1 center
        this->HexPolyData->SetPoints(this->Points);

        // Construct connectivity for the faces. These are used to perform
        // the picking.
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

        
        // Create the outline for the hex
        this->OutlinePolyData = vtkPolyData::New();
        this->OutlinePolyData->SetPoints(this->Points);
        this->OutlineMapper = vtkPolyDataMapper::New();
        this->OutlineMapper->SetInput(this->OutlinePolyData);
        this->HexOutline = vtkActor::New();
        this->HexOutline->SetMapper(this->OutlineMapper);
        cells = vtkCellArray::New();
        cells->Allocate(cells->EstimateSize(15,2));
        this->OutlinePolyData->SetLines(cells);
        cells->Delete();

        // Set up the initial properties
        this->CreateDefaultProperties();

        // Create the outline
        this->GenerateOutline();

        // Create the handles
        this->Handle = vtkCollection::New();
//      this->Handle = new vtkActor* [8];
        this->HandleMapper = vtkCollection::New();
//      this->HandleMapper = new vtkPolyDataMapper* [8];
        this->HandleGeometry = vtkCollection::New();
//      this->HandleGeometry = new vtkSphereSource* [8];
        this->PointWidget = vtkCollection::New();
        for (i=0; i<8; i++)
        {
                this->HandleGeometry->AddItem((vtkObject*) vtkSphereSource::New());
//              this->HandleGeometry[i] = vtkSphereSource::New();
                ((vtkSphereSource*)(this->HandleGeometry->GetItemAsObject(i)))->SetThetaResolution(16);
//              this->HandleGeometry[i]->SetThetaResolution(16);
                ((vtkSphereSource*)(this->HandleGeometry->GetItemAsObject(i)))->SetPhiResolution(8);
//              this->HandleGeometry[i]->SetPhiResolution(8);
                this->HandleMapper->AddItem((vtkObject*) vtkPolyDataMapper::New());
//              this->HandleMapper[i] = vtkPolyDataMapper::New();
                ((vtkPolyDataMapper*)(this->HandleMapper->GetItemAsObject(i)))->
                        SetInput(((vtkSphereSource*)(this->HandleGeometry->GetItemAsObject(i)))->
                        GetOutput());
//              this->HandleMapper[i]->SetInput(this->HandleGeometry[i]->GetOutput());
                this->Handle->AddItem((vtkObject*) vtkActor::New());
//              this->Handle[i] = vtkActor::New();
                ((vtkActor*) (this->Handle->GetItemAsObject(i)))->SetMapper
                        ((vtkPolyDataMapper*) this->HandleMapper->GetItemAsObject(i));
//              this->Handle[i]->SetMapper(this->HandleMapper[i]);
        }
        //      Pointwidget declarations
        for (i=0; i<8; i++)
        {
                this->PointWidget->AddItem((vtkObject*) vtkMimxModPointWidget::New());
                ((vtkMimxModPointWidget*) (this->PointWidget->GetItemAsObject(i)))->AllOff();
                ((vtkMimxModPointWidget*) (this->PointWidget->GetItemAsObject(i)))
                        ->SetHotSpotSize(0.5);
        }
        
        //      
        this->PWCallback = vtkBBWPWCallback::New();
        this->PWCallback->BoundingBoxWidget = this;
        this->PWCallback->PointWidget = (vtkMimxModPointWidget*) 
                (this->PointWidget->GetItemAsObject(0));

        // Define the point coordinates
        double bounds[6];
        bounds[0] = -0.5;
        bounds[1] = 0.5;
        bounds[2] = -0.5;
        bounds[3] = 0.5;
        bounds[4] = -0.5;
        bounds[5] = 0.5;
        // Points 8-14 are down by PositionHandles();
        this->PlaceWidget(bounds);

        //Manage the picking stuff
        this->HandlePicker = vtkCellPicker::New();
        this->HandlePicker->SetTolerance(0.001);
        for (i=0; i<8; i++)
        {
                this->HandlePicker->AddPickList(
                        (vtkActor*) this->Handle->GetItemAsObject(i));
        }
        this->HandlePicker->PickFromListOn();

        this->HexPicker = vtkCellPicker::New();
        this->HexPicker->SetTolerance(0.001);
        this->HexPicker->AddPickList(HexActor);
        this->HexPicker->PickFromListOn();

//      this->PointWidget1->AddObserver(vtkCommand::InteractionEvent,
//              this->PWCallback, 0.0);
        for(int i=0; i <8; i++)
        {
                (vtkMimxModPointWidget*) (this->PointWidget->GetItemAsObject(i))
                        ->AddObserver(vtkCommand::InteractionEvent,this->PWCallback, 0.0);
        }
        this->CurrentHandle = NULL;
}

vtkMimxBoundingBoxWidget::~vtkMimxBoundingBoxWidget()
{
        this->HexActor->Delete();
        this->HexMapper->Delete();
        this->HexPolyData->Delete();
        this->Points->Delete();

        this->HexOutline->Delete();
        this->OutlineMapper->Delete();
        this->OutlinePolyData->Delete();
        
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

void vtkMimxBoundingBoxWidget::SetEnabled(int enabling)
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
                for (int j=0; j<8; j++)
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
//              this->CurrentRenderer->RemoveActor(this->HexActor);
//              this->CurrentRenderer->RemoveActor(this->HexOutline);

                // turn off the handles
                for (int i=0; i<8; i++)
                {
                        this->CurrentRenderer->RemoveActor((vtkActor*)
                                this->Handle->GetItemAsObject(i));
                }

                this->CurrentHandle = NULL;
                this->InvokeEvent(vtkCommand::DisableEvent,NULL);
                this->CurrentRenderer = NULL;
        }

        this->Interactor->Render();
}

void vtkMimxBoundingBoxWidget::ProcessEvents(vtkObject* vtkNotUsed(object), 
                                                                 unsigned long event,
                                                                 void* clientdata, 
                                                                 void* vtkNotUsed(calldata))
{
        vtkMimxBoundingBoxWidget* self = reinterpret_cast<vtkMimxBoundingBoxWidget *>( clientdata );

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

void vtkMimxBoundingBoxWidget::PrintSelf(ostream& os, vtkIndent indent)
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

void vtkMimxBoundingBoxWidget::PositionHandles()
{
        for(int i=0; i <8; i++)
        {
                ((vtkSphereSource*) (this->HandleGeometry->GetItemAsObject(i)))->
                        SetCenter(this->Points->GetPoint(i));
        }
        this->HexPolyData->Modified();
//      this->GenerateOutline();
}


void vtkMimxBoundingBoxWidget::SizeHandles()
{
        float radius = this->vtk3DWidget::SizeHandles(1.5);
        for(int i=0; i<8; i++)
        {
                ((vtkSphereSource*) (this->HandleGeometry->GetItemAsObject(i)))
                        ->SetRadius(radius);
        }
}

int vtkMimxBoundingBoxWidget::HighlightHandle(vtkProp *prop)
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


void vtkMimxBoundingBoxWidget::HighlightOutline(int highlight)
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

void vtkMimxBoundingBoxWidget::OnLeftButtonDown()
{
        int X = this->Interactor->GetEventPosition()[0];
        int Y = this->Interactor->GetEventPosition()[1];

        // Okay, we can process this. Try to pick handles first;
        // if no handles picked, then pick the bounding box.
        vtkRenderer *ren = this->Interactor->FindPokedRenderer(X,Y);
        if ( ren != this->CurrentRenderer )
        {
                this->State = vtkMimxBoundingBoxWidget::Outside;
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
                this->State = vtkMimxBoundingBoxWidget::MovingHandle;
                this->HighlightHandle(path->GetFirstNode()->GetProp());
                this->EnablePointWidget();
                forward = this->ForwardEvent(vtkCommand::LeftButtonPressEvent);
        }
        
        this->EventCallbackCommand->SetAbortFlag(1);
        this->StartInteraction();
        if ( ! forward )
        {
                this->Interactor->Render();
        }
}

void vtkMimxBoundingBoxWidget::OnLeftButtonUp()
{
        if ( this->State == vtkMimxBoundingBoxWidget::Outside ||
                this->State == vtkMimxBoundingBoxWidget::Start )
        {
                return;
        }

        this->State = vtkMimxBoundingBoxWidget::Start;
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

void vtkMimxBoundingBoxWidget::OnMiddleButtonDown()
{
        int X = this->Interactor->GetEventPosition()[0];
        int Y = this->Interactor->GetEventPosition()[1];

        // Okay, we can process this. Try to pick handles first;
        // if no handles picked, then pick the bounding box.
        vtkRenderer *ren = this->Interactor->FindPokedRenderer(X,Y);
        if ( ren != this->CurrentRenderer )
        {
                this->State = vtkMimxBoundingBoxWidget::Outside;
                return;
        }

        vtkAssemblyPath *path;
        this->HandlePicker->Pick(X,Y,0.0,this->CurrentRenderer);
        path = this->HandlePicker->GetPath();
        if ( path != NULL )
        {
                this->State = vtkMimxBoundingBoxWidget::MovingHandle;
//              this->CurrentHandle = this->Handle[6];
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
                        this->State = vtkMimxBoundingBoxWidget::MovingHandle;
//                      this->CurrentHandle = this->Handle[6];
                        this->HighlightOutline(1);
                        this->HexPicker->GetPickPosition(this->LastPickPosition);
                        this->ValidPick = 1;
                }
                else
                {
                        this->State = vtkMimxBoundingBoxWidget::Outside;
                        return;
                }
        }

        this->EventCallbackCommand->SetAbortFlag(1);
        this->StartInteraction();
        this->InvokeEvent(vtkCommand::StartInteractionEvent, NULL);
        this->Interactor->Render();
}

void vtkMimxBoundingBoxWidget::OnMiddleButtonUp()
{
        if ( this->State == vtkMimxBoundingBoxWidget::Outside ||
                this->State == vtkMimxBoundingBoxWidget::Start )
        {
                return;
        }

        this->State = vtkMimxBoundingBoxWidget::Start;
        this->SizeHandles();

        this->EventCallbackCommand->SetAbortFlag(1);
        this->EndInteraction();
        this->InvokeEvent(vtkCommand::EndInteractionEvent, NULL);
        this->Interactor->Render();

}

void vtkMimxBoundingBoxWidget::OnRightButtonDown()
{
        int X = this->Interactor->GetEventPosition()[0];
        int Y = this->Interactor->GetEventPosition()[1];

        // Okay, we can process this. Try to pick handles first;
        // if no handles picked, then pick the bounding box.
        vtkRenderer *ren = this->Interactor->FindPokedRenderer(X,Y);
        if ( ren != this->CurrentRenderer )
        {
                this->State = vtkMimxBoundingBoxWidget::Outside;
                return;
        }

        vtkAssemblyPath *path;
        this->HandlePicker->Pick(X,Y,0.0,this->CurrentRenderer);
        path = this->HandlePicker->GetPath();
        if ( path != NULL )
        {
                this->State = vtkMimxBoundingBoxWidget::Scaling;
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
                        this->State = vtkMimxBoundingBoxWidget::Scaling;
                        this->HighlightOutline(1);
                        this->HexPicker->GetPickPosition(this->LastPickPosition);
                        this->ValidPick = 1;
                }
                else
                {
                        this->State = vtkMimxBoundingBoxWidget::Outside;
                        return;
                }
        }

        this->EventCallbackCommand->SetAbortFlag(1);
        this->StartInteraction();
        this->InvokeEvent(vtkCommand::StartInteractionEvent, NULL);
        this->Interactor->Render();
}

void vtkMimxBoundingBoxWidget::OnRightButtonUp()
{
        if ( this->State == vtkMimxBoundingBoxWidget::Outside )
        {
                return;
        }

        this->State = vtkMimxBoundingBoxWidget::Start;
        this->HighlightOutline(0);
        this->SizeHandles();

        this->EventCallbackCommand->SetAbortFlag(1);
        this->EndInteraction();
        this->InvokeEvent(vtkCommand::EndInteractionEvent, NULL);
        this->Interactor->Render();
}

void vtkMimxBoundingBoxWidget::OnMouseMove()
{
        // See whether we're active
        if ( this->State == vtkMimxBoundingBoxWidget::Outside || 
                this->State == vtkMimxBoundingBoxWidget::Start )
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
        if ( this->State == vtkMimxBoundingBoxWidget::MovingHandle )
        {
                forward = this->ForwardEvent(vtkCommand::MouseMoveEvent);
        }
        this->EventCallbackCommand->SetAbortFlag(1);
        this->InvokeEvent(vtkCommand::InteractionEvent,NULL);
        if ( ! forward )
        {
                this->Interactor->Render();
        }
}

void vtkMimxBoundingBoxWidget::CreateDefaultProperties()
{
        // Handle properties
        this->HandleProperty = vtkProperty::New();
        this->HandleProperty->SetColor(1,1,1);

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

void vtkMimxBoundingBoxWidget::PlaceWidget(double bds[6])
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
        this->SizeHandles();
}

void vtkMimxBoundingBoxWidget::GetPolyData(vtkPolyData *pd)
{
        pd->SetPoints(this->HexPolyData->GetPoints());
        pd->SetPolys(this->HexPolyData->GetPolys());
}

void vtkMimxBoundingBoxWidget::GenerateOutline()
{
        // Now the outline lines
        if ( ! this->OutlineFaceWires && ! this->OutlineCursorWires )
        {
                return;
        }

        vtkIdType pts[2];
        vtkCellArray *cells = this->OutlinePolyData->GetLines();
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
        this->OutlinePolyData->Modified();
        if ( this->OutlineProperty) 
        {
                this->OutlineProperty->SetRepresentationToWireframe();
                this->SelectedOutlineProperty->SetRepresentationToWireframe();
        }
}

int vtkMimxBoundingBoxWidget::ForwardEvent(unsigned long event)
{
        if ( ! this->CurrentPointWidget )
        {
                return 0;
        }
//VAM Re-enable
        this->CurrentPointWidget->ProcessEvents(this,event,
                this->CurrentPointWidget,NULL);
       // this->CurrentPointWidget->InvokeEvent(event);

        return 1;
}

void vtkMimxBoundingBoxWidget::EnablePointWidget()
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
                                this->Points->GetPoint(count,x);
                                status = true;
                                PWCallback->HandleNumber = count;
                                PWCallback->PointWidget = this->CurrentPointWidget;
                        }
                        count++;
                } while(count < 8 && status == false);

        }
        
        double bounds[6];
        for (int i=0; i<3; i++)
        {
                bounds[2*i] = x[i] - 0.1*this->InitialLength;
                bounds[2*i+1] = x[i] + 0.1*this->InitialLength;
        }

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

void vtkMimxBoundingBoxWidget::DisablePointWidget()
{
        this->CurrentPointWidget->Off();
        this->CurrentPointWidget = NULL;
}

void vtkMimxBoundingBoxWidget::SetPoint(int i, double x[3])
{
        this->Points->SetPoint(i,x);
        this->HexPolyData->SetPoints(this->Points);
        this->HexPolyData->Modified();
        ((vtkSphereSource*) (this->HandleGeometry->GetItemAsObject(i)))->SetCenter(x);
        ((vtkSphereSource*) (this->HandleGeometry->GetItemAsObject(i)))->Update();
}
