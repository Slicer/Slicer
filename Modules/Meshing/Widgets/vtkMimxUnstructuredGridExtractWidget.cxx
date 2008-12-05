/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxUnstructuredGridExtractWidget.cxx,v $
Language:  C++
Date:      $Date: 2007/05/17 16:30:26 $
Version:   $Revision: 1.15 $

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

#include "vtkMimxUnstructuredGridExtractWidget.h"

#include "vtkActor.h"
#include "vtkAssemblyNode.h"
#include "vtkAssemblyPath.h"
#include "vtkCallbackCommand.h"
#include "vtkCamera.h"
#include "vtkCellArray.h"
#include "vtkCellPicker.h"
#include "vtkCollection.h"
#include "vtkDataSetMapper.h"
#include "vtkDoubleArray.h"
#include "vtkObjectFactory.h"
#include "vtkUnstructuredGrid.h"
#include "vtkProperty.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkExtractEdges.h"
#include "vtkPointPicker.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyDataWriter.h"
#include "vtkCellData.h"
#include "vtkFloatArray.h"
#include "vtkUnstructuredGridWriter.h"
#include "vtkCell.h"
#include "vtkIdList.h"
#include "vtkSphereSource.h"
#include "vtkGeometryFilter.h"

#include "vtkPolyDataWriter.h"

vtkCxxRevisionMacro(vtkMimxUnstructuredGridExtractWidget, "$Revision: 1.15 $");
vtkStandardNewMacro(vtkMimxUnstructuredGridExtractWidget);

// Callback command for picked entity in an unstructured grid
// entity could be a vertex, edge, face or a cell
class vtkUGEWCallback : public vtkCommand
{
public:
        static vtkUGEWCallback *New() 
        { return new vtkUGEWCallback; }
        virtual void Execute(vtkObject *caller, unsigned long, void*)
        {
                pickedcell = cellpicker->GetCellId();
                pickedpoint = pointpicker->GetPointId();
        }
        vtkUGEWCallback():UnstructuredGridWidget(0) {}
        vtkMimxUnstructuredGridExtractWidget  *UnstructuredGridWidget;
        vtkCellPicker* cellpicker;
        vtkPointPicker *pointpicker;
        vtkIdType pickedcell;
        vtkIdType pickedpoint;
};

vtkMimxUnstructuredGridExtractWidget::vtkMimxUnstructuredGridExtractWidget()
{
        this->State = vtkMimxUnstructuredGridExtractWidget::Start;
        this->EventCallbackCommand->SetCallback(vtkMimxUnstructuredGridExtractWidget::ProcessEvents);

        // Construct the poly data representing the hex
        this->PolyDataMapper = vtkPolyDataMapper::New();
        this->PolyDataActor = vtkActor::New();
        this->PolyData = vtkPolyData::New();
        // for 3D solid unstructured grid
        this->UGridSet = vtkUnstructuredGrid::New();
        this->UGridMapperSet = vtkDataSetMapper::New();
        this->UGridActorSet = vtkActor::New();

        // Set up the initial properties
//      this->CreateDefaultProperties();

        // Create the handles
        this->EWCallback = vtkUGEWCallback::New();
        this->EWCallback->UnstructuredGridWidget = this;

        this->CellPicker = vtkCellPicker::New();
        this->CellPicker->SetTolerance(0.00001);

        this->PointPicker = vtkPointPicker::New();
        this->EWCallback->cellpicker = this->CellPicker;
        this->EWCallback->pointpicker = this->PointPicker;
        this->PointsList = NULL;
        this->CellIdList = vtkIdList::New();
        this->PointIdList = vtkIdList::New();
        // for picked point indication
        this->Handle = vtkCollection::New();
        this->HandleMapper = vtkCollection::New();
        this->HandleGeometry = vtkCollection::New();

        this->HandleSize = 1.0;
        // highlight details
        this->PolyDataHighLight = vtkPolyData::New();
        this->PolyDataHighLightMapper = vtkPolyDataMapper::New();
        this->PolyDataHighLightActor = vtkActor::New();

        this->UGridHighLight = vtkUnstructuredGrid::New();
        this->UGridHighLightMapper = vtkDataSetMapper::New();
        this->UGridHighLightActor = vtkActor::New();
}

vtkMimxUnstructuredGridExtractWidget::~vtkMimxUnstructuredGridExtractWidget()
{
        this->PolyDataActor->Delete();
        this->PolyDataMapper->Delete();

        if(this->UGridSet)
                this->UGridSet->Delete();
        this->UGridActorSet->Delete();
        this->UGridMapperSet->Delete();

        this->PolyData->Delete();
        this->CellPicker->Delete();
        this->PointPicker->Delete();
        this->CellIdList->Delete();
        this->PointIdList->Delete();
        this->HandleGeometry->Delete();
        this->HandleMapper->Delete();
        this->Handle->Delete();

        this->PolyDataHighLight->Delete();
        this->PolyDataHighLightActor->Delete();
        this->PolyDataHighLightMapper->Delete();

        this->UGridHighLight->Delete();
        this->UGridHighLightActor->Delete();
        this->UGridHighLightMapper->Delete();
}

void vtkMimxUnstructuredGridExtractWidget::SetEnabled(int enabling)
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
                this->Initialize();
                
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
                this->CurrentRenderer->RemoveActor(this->UGridActor);
                if(this->State == 4 || this->State == 5)        
                {
                        this->UGridActorSet->GetProperty()->SetColor
                                (this->UGridActorSet->GetProperty()->GetColor());
                        this->CurrentRenderer->AddActor(this->UGridActorSet);
                }
                else
                {
                        if( this->State == 1)
                        {
                                for (int i=0; i < this->Handle->GetNumberOfItems(); i++)
                                {
                                        this->CurrentRenderer->AddActor((vtkActor*) this->Handle->GetItemAsObject(i));
                                }
                                this->SizeHandles();
                        }
                        this->CurrentRenderer->AddActor(this->PolyDataActor);
                        this->CellIdList->Initialize();
                }
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
                this->CurrentRenderer->AddActor(this->UGridActor);
                if(this->State ==4 || this->State == 5) 
                {
                        //if(this->UGridSet->GetCellData()->GetScalars())       
                        //      this->UGridSet->GetCellData()->GetScalars()->Initialize();
                        //this->UGridSet->Modified();
                        this->CurrentRenderer->RemoveActor(this->UGridActorSet);
                        this->CellPicker->DeletePickList(this->UGridActorSet);
                        this->CurrentRenderer->RemoveActor(this->UGridHighLightActor);
                }
                else
                {
                        //if(this->PolyData->GetCellData()->GetScalars())       
                        //      this->PolyData->GetCellData()->GetScalars()->Initialize();
                        //this->PolyData->Modified();
                        this->CurrentRenderer->RemoveActor(this->PolyDataActor);
                        this->CellPicker->DeletePickList(this->PolyDataActor);
                        if( this->State == 1)
                        {
                                for (int i=0; i < this->Handle->GetNumberOfItems(); i++)
                                {
                                        this->CurrentRenderer->RemoveActor((vtkActor*) this->Handle->GetItemAsObject(i));
                                }
                        }
                        this->CurrentRenderer->RemoveActor(this->PolyDataHighLightActor);
                }
                this->InvokeEvent(vtkCommand::DisableEvent,NULL);
                this->CurrentRenderer = NULL;
        }
        this->Interactor->Render();
}

void vtkMimxUnstructuredGridExtractWidget::ProcessEvents(vtkObject* vtkNotUsed(object), 
                                                                 unsigned long event,
                                                                 void* clientdata, 
                                                                 void* vtkNotUsed(calldata))
{
        vtkMimxUnstructuredGridExtractWidget* self = reinterpret_cast<vtkMimxUnstructuredGridExtractWidget *>( clientdata );

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

void vtkMimxUnstructuredGridExtractWidget::PrintSelf(ostream& os, vtkIndent indent)
{
        this->Superclass::PrintSelf(os,indent);

}


void vtkMimxUnstructuredGridExtractWidget::OnLeftButtonDown()
{
        int X = this->Interactor->GetEventPosition()[0];
        int Y = this->Interactor->GetEventPosition()[1];
        this->PointsList = NULL;
        
        vtkRenderer *ren = this->Interactor->FindPokedRenderer(X,Y);
        if ( ren != this->CurrentRenderer )
        {
                return;
        }

        int forward=0;

        vtkAssemblyPath *path;

        if(this->State != 1)
        {
        this->CellPicker->Pick(X,Y,0.0,this->CurrentRenderer);
        path = this->CellPicker->GetPath();
        vtkIdType pick = this->CellPicker->GetSubId();
        // check if an entity is picked
        if ( path != NULL )
        {
                this->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);

                vtkIdType pick = this->CellPicker->GetCellId();
        }

        
        this->EventCallbackCommand->SetAbortFlag(1);
        this->StartInteraction();
        if ( ! forward )
        {
                this->Interactor->Render();
        }

        // if the picked entity is valid
        if(this->EWCallback->pickedcell != -1)
        {
                vtkIdType t = 0;
                if(this->State != 4 && this->State != 5 )
                {
                        if(this->State == 7)
                        {
                                if (this->CellIdList->GetNumberOfIds() == 0)
                                {
                                        this->CellIdList->InsertNextId(this->EWCallback->pickedcell);
                                        this->PolyData->GetCellPoints
                                                (this->EWCallback->pickedcell,t,this->PointsList);
                                        vtkFloatArray* floatarray = vtkFloatArray::New();
                                        floatarray->SetNumberOfValues(this->PolyData->GetNumberOfCells());
                                        for(int i=0; i < this->PolyData->GetNumberOfCells(); i++)
                                                floatarray->SetValue(i,0.0);
                                        floatarray->SetValue(this->EWCallback->pickedcell,1.0);
                                        this->PolyData->GetCellData()->SetScalars(floatarray);
                                        floatarray->Delete();
                                }
                                else
                                {
                                        if(this->CellIdList->IsId(this->EWCallback->pickedcell) != -1)
                                        {
                                                this->CellIdList->DeleteId(this->EWCallback->pickedcell);
                                                vtkFloatArray* floatarray = vtkFloatArray::SafeDownCast(this->PolyData->GetCellData()->GetScalars());
                                                floatarray->SetValue(this->EWCallback->pickedcell,0.0);
                                                this->PolyData->GetCellData()->SetScalars(floatarray);
                                                this->PolyData->Modified();
                                        }
                                        else{
                                                this->CellIdList->InsertNextId(this->EWCallback->pickedcell);
                                                vtkFloatArray* floatarray = vtkFloatArray::SafeDownCast(this->PolyData->GetCellData()->GetScalars());
                                                floatarray->SetValue(this->EWCallback->pickedcell,1.0);
                                                this->PolyData->GetCellData()->SetScalars(floatarray);  
                                        this->PolyData->Modified();
                                        }
                                }
                        }
                        else
                        {
                                this->PolyData->GetCellPoints
                                        (this->EWCallback->pickedcell,t,this->PointsList);
                                //vtkFloatArray* floatarray = vtkFloatArray::New();
                                //floatarray->SetNumberOfValues(this->PolyData->GetNumberOfCells());
                                //for(int i=0; i < this->PolyData->GetNumberOfCells(); i++)
                                //      floatarray->SetValue(i,0.0);
                                //floatarray->SetValue(this->EWCallback->pickedcell,1.0);
                                //this->PolyData->GetCellData()->SetScalars(floatarray);
                                //floatarray->Delete();
                                this->CurrentRenderer->RemoveActor(this->PolyDataHighLightActor);
                                this->PolyDataHighLight->Delete();
                                this->PolyDataHighLight = vtkPolyData::New();
//                              this->PolyDataHighLight->Modified();
                                vtkCellArray *cellarray = vtkCellArray::New();
                                cellarray->SetNumberOfCells(1);
                                vtkIdList *cellids;
                                vtkCell *cell = this->PolyData->GetCell(this->EWCallback->pickedcell);
                                cellids = cell->GetPointIds();
                                cellarray->InsertNextCell(cellids->GetNumberOfIds());
                                vtkPoints *points = vtkPoints::New();
                                points->SetNumberOfPoints(cellids->GetNumberOfIds());
                                for (int i=0; i<cellids->GetNumberOfIds(); i++) {
                                        cellarray->InsertCellPoint(i);
                                        points->InsertPoint(i, this->PolyData->GetPoint(cellids->GetId(i)));
                                }
                                if(this->State == 2)
                                        this->PolyDataHighLight->SetLines(cellarray);
                                else
                                        this->PolyDataHighLight->SetPolys(cellarray);
                                PolyDataHighLight->SetPoints(points);
                                cellarray->Delete();
                                points->Delete();
                                this->PolyDataHighLightMapper->Delete();
                                this->PolyDataHighLightMapper = vtkPolyDataMapper::New();
                                this->PolyDataHighLightMapper->SetInput(PolyDataHighLight);
                                this->PolyDataHighLightActor->Delete();
                                this->PolyDataHighLightActor = vtkActor::New();
                                this->PolyDataHighLightActor->SetMapper(PolyDataHighLightMapper);
                                this->PolyDataHighLightActor->GetProperty()->SetLineWidth(
                                        this->PolyDataActor->GetProperty()->GetLineWidth()*2.0);
                                //double color[3];
                                //this->PolyDataActor->GetProperty()->GetColor(color);
                                this->PolyDataHighLightActor->GetProperty()->SetColor(1.0, 0.0, 0.0);
                                this->CurrentRenderer->AddActor(this->PolyDataHighLightActor);
                                this->CurrentRenderer->Render();
                        }
                }
                else
                {
                        this->UGridSet->GetCellPoints(
                                this->EWCallback->pickedcell,t,this->PointsList);
        /*              vtkFloatArray* floatarray = vtkFloatArray::New();
                        floatarray->SetNumberOfValues(this->UGridSet->GetNumberOfCells());
                        for(int i=0; i < this->UGridSet->GetNumberOfCells(); i++)
                                floatarray->SetValue(i,0.0);
                        floatarray->SetValue(this->EWCallback->pickedcell,1.0);
                        this->UGridSet->GetCellData()->SetScalars(floatarray);
                        floatarray->Delete();*/
                        this->CurrentRenderer->RemoveActor(this->UGridHighLightActor);
                        this->UGridHighLight->Delete();
                        this->UGridHighLight = vtkUnstructuredGrid::New();
//                      this->UGridHighLight->Modified();
                        vtkCell *cell = this->UGrid->GetCell(this->EWCallback->pickedcell);
                        vtkIdList *cellids = cell->GetPointIds();
                        vtkPoints *points = vtkPoints::New();
                        points->SetNumberOfPoints(cellids->GetNumberOfIds());
                        int i;
                        for (i=0; i<cellids->GetNumberOfIds(); i++) {
                                points->InsertPoint(i, this->UGrid->GetPoint(cellids->GetId(i)));
                        }
                        vtkIdList *ptids = vtkIdList::New();
                        ptids->SetNumberOfIds(cellids->GetNumberOfIds());
                        for (i=0; i<cellids->GetNumberOfIds(); i++) {
                                ptids->SetId(i,i);
                        }
                        this->UGridHighLight->Allocate(1,1);
                        this->UGridHighLight->InsertNextCell(cell->GetCellType(), ptids);
                        this->UGridHighLight->SetPoints(points);
                        points->Delete();
                        ptids->Delete();
                        this->UGridHighLight->Squeeze();
                        this->UGridHighLightMapper->Delete();
                        this->UGridHighLightMapper = vtkDataSetMapper::New();
                        this->UGridHighLightMapper->SetInput(UGridHighLight);
                        this->UGridHighLightActor->Delete();
                        this->UGridHighLightActor = vtkActor::New();
                        this->UGridHighLightActor->SetMapper(UGridHighLightMapper);
                        this->UGridHighLightActor->GetProperty()->SetLineWidth(
                                this->UGridActorSet->GetProperty()->GetLineWidth()*2.0);
                        this->UGridHighLightActor->GetProperty()->SetColor(1.0, 0.0, 0.0);
                        this->CurrentRenderer->AddActor(this->UGridHighLightActor);
                        this->CurrentRenderer->Render();
                }
        }
        }
        else{
                this->PointPicker->Pick(X,Y,0.0,this->CurrentRenderer);
                path = this->PointPicker->GetPath();
                vtkIdType pick = this->CellPicker->GetSubId();
                // check if an entity is picked
                if ( path != NULL ){
                        if(this->EWCallback->pickedpoint != -1)
                        {
                                if(this->PointIdList->GetNumberOfIds() > 3)
                                {
                                        vtkErrorMacro("Max 4 points can be picked");
                                }
                                else{
                                vtkIdType *cells;
                                unsigned short ncells;
                                this->PolyData->GetPointCells(this->EWCallback->pickedpoint, ncells, cells);
                                if(ncells > 0)
                                {
                                        if(this->PointIdList->IsId(this->EWCallback->pickedpoint) == -1)
                                        {
                                                this->PointIdList->InsertNextId(this->EWCallback->pickedpoint);
                                this->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);

                                this->HandleGeometry->AddItem((vtkObject*) vtkSphereSource::New());
                                ((vtkSphereSource*)(this->HandleGeometry->GetItemAsObject(this->HandleGeometry->GetNumberOfItems()-1)))->SetThetaResolution(16);
                                ((vtkSphereSource*)(this->HandleGeometry->GetItemAsObject(this->HandleGeometry->GetNumberOfItems()-1)))->SetPhiResolution(8);
                                this->HandleMapper->AddItem((vtkObject*) vtkDataSetMapper::New());
                                
                                ((vtkDataSetMapper*)(this->HandleMapper->GetItemAsObject(this->HandleMapper->GetNumberOfItems()-1)))->SetInput(((vtkSphereSource*)(this->HandleGeometry->GetItemAsObject(this->HandleMapper->GetNumberOfItems()-1)))->
                                        GetOutput());

                                this->Handle->AddItem((vtkObject*) vtkActor::New());
                                ((vtkActor*) (this->Handle->GetItemAsObject(this->Handle->GetNumberOfItems()-1)))->SetMapper
                                        ((vtkDataSetMapper*) (this->HandleMapper->GetItemAsObject(this->HandleMapper->GetNumberOfItems()-1)));
                                ((vtkSphereSource*) (this->HandleGeometry->GetItemAsObject(this->HandleGeometry->GetNumberOfItems()-1)))->SetCenter(this->PolyData->GetPoint(this->EWCallback->pickedpoint));

                                ((vtkSphereSource*) (this->HandleGeometry->GetItemAsObject(this->HandleGeometry->GetNumberOfItems()-1)))->SetRadius(this->HandleSize);
                                
                                ((vtkActor*) (this->Handle->GetItemAsObject(this->Handle->GetNumberOfItems()-1)))->GetProperty()->SetColor(0.0, 1.0, 0.0);

                                this->CurrentRenderer->AddActor((vtkActor*) this->Handle->GetItemAsObject(this->Handle->GetNumberOfItems()-1));
                                this->Interactor->Render();
                                        }
                                        }
                                        else
                                        {
                                                this->CurrentRenderer->RemoveActor((vtkActor*) this->Handle->GetItemAsObject(this->PointIdList->IsId(this->EWCallback->pickedpoint)));
                                                this->HandleGeometry->RemoveItem(this->PointIdList->IsId(this->EWCallback->pickedpoint));
                                                this->HandleMapper->RemoveItem(this->PointIdList->IsId(this->EWCallback->pickedpoint));
                                                this->Handle->RemoveItem(this->PointIdList->IsId(this->EWCallback->pickedpoint));
                                                this->PointIdList->DeleteId(this->EWCallback->pickedpoint);
                                        }
                                }
                        }
                }
        }

}

void vtkMimxUnstructuredGridExtractWidget::OnLeftButtonUp()
{
        this->EventCallbackCommand->SetAbortFlag(1);
        this->EndInteraction();
        this->InvokeEvent(vtkCommand::EndInteractionEvent, NULL);
        this->Interactor->Render();
}

void vtkMimxUnstructuredGridExtractWidget::OnMiddleButtonDown()
{
}

void vtkMimxUnstructuredGridExtractWidget::OnMiddleButtonUp()
{
}

void vtkMimxUnstructuredGridExtractWidget::OnRightButtonDown()
{
}

void vtkMimxUnstructuredGridExtractWidget::OnRightButtonUp()
{
}

void vtkMimxUnstructuredGridExtractWidget::OnMouseMove()
{
        //if(this->State != 1)
        //{
        //      this->CurrentRenderer->RemoveActor(this->PolyDataHighLightActor);
        //      this->CurrentRenderer->RemoveActor(this->UGridHighLightActor);

        //      this->CellPicker->AddObserver(vtkCommand::MouseMoveEvent,this->EWCallback, 0.0);
        //      this->Interactor->SetPicker(this->CellPicker);
        //      this->CellPicker->SetTolerance(0.01);

        //      if(this->State == 4 || this->State == 5)        
        //      {
        //              this->CellPicker->AddPickList(this->UGridActorSet);
        //      }
        //      else
        //      {
        //              this->CellPicker->AddPickList(this->PolyDataActor);
        //      }
        //      int X = this->Interactor->GetEventPosition()[0];
        //      int Y = this->Interactor->GetEventPosition()[1];
        //      this->PointsList = NULL;

        //      vtkRenderer *ren = this->Interactor->FindPokedRenderer(X,Y);
        //      if ( ren != this->CurrentRenderer )
        //      {
        //              return;
        //      }

        //      int forward=0;

        //      vtkAssemblyPath *path;

        //      if(this->State != 1)
        //      {
        //              this->CellPicker->Pick(X,Y,0.0,this->CurrentRenderer);
        //              path = this->CellPicker->GetPath();
        //              vtkIdType pick = this->CellPicker->GetSubId();
        //              // check if an entity is picked
        //              if ( path != NULL )
        //              {
        //                      this->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);

        //                      vtkIdType pick = this->CellPicker->GetCellId();
        //              }


        //              this->EventCallbackCommand->SetAbortFlag(1);
        //              this->StartInteraction();
        //              if ( ! forward )
        //              {
        //                      this->Interactor->Render();
        //              }

        //              // if the picked entity is valid
        //              if(this->EWCallback->pickedcell != -1)
        //              {
        //                      vtkIdType t = 0;
        //                      if(this->State != 4 && this->State != 5 )
        //                      {
        //                              this->PolyDataHighLight->Initialize();
        //                              this->PolyDataHighLight->Modified();
        //                              vtkCellArray *cellarray = vtkCellArray::New();
        //                              cellarray->SetNumberOfCells(1);
        //                              vtkIdList *cellids;
        //                              vtkCell *cell = this->PolyData->GetCell(this->EWCallback->pickedcell);
        //                              cellids = cell->GetPointIds();
        //                              cellarray->InsertNextCell(cellids->GetNumberOfIds());
        //                              vtkPoints *points = vtkPoints::New();
        //                              points->SetNumberOfPoints(cellids->GetNumberOfIds());
        //                              for (int i=0; i<cellids->GetNumberOfIds(); i++) {
        //                                      cellarray->InsertCellPoint(cellids->GetId(i));
        //                                      points->InsertPoint(i, this->PolyData->GetPoint(cellids->GetId(i)));
        //                              }
        //                              if(this->State == 2)
        //                                      this->PolyDataHighLight->SetLines(cellarray);
        //                              else
        //                                      this->PolyDataHighLight->SetPolys(cellarray);
        //                              PolyDataHighLight->SetPoints(points);
        //                              cellarray->Delete();
        //                              points->Delete();
        //                              this->PolyDataHighLightMapper->SetInput(PolyDataHighLight);
        //                              this->PolyDataHighLightActor->SetMapper(PolyDataHighLightMapper);
        //                              this->PolyDataHighLightActor->GetProperty()->SetLineWidth(
        //                                      this->PolyDataActor->GetProperty()->GetLineWidth()*1.5);

        //                              this->CurrentRenderer->AddActor(this->PolyDataHighLightActor);
        //                              this->CurrentRenderer->Render();
        //                      }
        //                      else
        //                      {
        //                              this->UGridHighLight->Initialize();
        //                              this->UGridHighLight->Modified();
        //                      }
        //              }
        //      }
        //}

}

int vtkMimxUnstructuredGridExtractWidget::ForwardEvent(unsigned long event)
{
        this->ProcessEvents(this,event,
                this,NULL);
        return 1;
}

void vtkMimxUnstructuredGridExtractWidget::Initialize()
{
        if(this->UGrid)
        {
                // if the input data changes
                this->PolyData->Initialize();
                this->PolyData->Delete();
                this->PolyData = vtkPolyData::New();
                this->UGridSet->Delete();
                this->UGridSet = vtkUnstructuredGrid::New();

                if(this->State == 2)    this->ExtractEdge();
                if(this->State == 3 || this->State == 7 || this->State == 1)    this->ExtractFace();
                if(this->State == 4 || this->State == 5)        {
                        this->UGridSet->DeepCopy(this->UGrid);
                        this->UGridMapperSet->Delete();
                        this->UGridMapperSet = vtkDataSetMapper::New();
                        this->UGridMapperSet->SetInput(this->UGridSet);
                        this->UGridActorSet->Delete();
                        this->UGridActorSet = vtkActor::New();
                        this->UGridActorSet->SetMapper(this->UGridMapperSet);
                        this->UGridActorSet->GetProperty()->SetRepresentationToWireframe();
                        this->UGridActorSet->GetProperty()->SetLineWidth(3.0);
                }
                this->CellPicker->AddObserver(vtkCommand::EndPickEvent,this->EWCallback, 0.0);
                this->CellPicker->AddObserver(vtkCommand::MouseMoveEvent,this->EWCallback, 0.0);

                this->PointPicker->AddObserver(vtkCommand::EndPickEvent,this->EWCallback, 0.0);
                if(this->State != 1)
                {this->Interactor->SetPicker(this->CellPicker);
                this->CellPicker->SetTolerance(0.01);}
                else{this->Interactor->SetPicker(this->PointPicker);
                this->PointPicker->SetTolerance(0.01);}

                if(this->State == 4 || this->State == 5)        
                {
                        this->CellPicker->AddPickList(this->UGridActorSet);}
                else
                {
                        if(this->State == 1)
                        {
                                this->PointPicker->AddPickList(this->PolyDataActor);
                        }
                        else{this->CellPicker->AddPickList(this->PolyDataActor);}
                }
                if(this->State == 1)
                {

                        this->PointPicker->PickFromListOn();
                }
                else
                        this->CellPicker->PickFromListOn();
        }
}

void vtkMimxUnstructuredGridExtractWidget::ExtractEdge()
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
        this->PolyData->SetPoints(points);
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
        this->PolyData->SetLines(edgelist);
        this->PolyDataMapper->Delete();
        this->PolyDataMapper = vtkPolyDataMapper::New();
        this->PolyDataMapper->SetInput(this->PolyData);
        this->PolyDataActor->Delete();
        this->PolyDataActor = vtkActor::New();
        this->PolyDataActor->SetMapper(this->PolyDataMapper);
        this->PolyDataActor->GetProperty()->SetLineWidth(3);
        //this->CellPicker->AddPickList(this->PolyDataActor);
        //this->CellPicker->PickFromListOn();
        edgelist->Delete();
        points->Delete();
        vtkPolyDataWriter *writer = vtkPolyDataWriter::New();
        writer->SetInput(this->PolyData);
        writer->SetFileName("edge.vtk");
        writer->Write();
}

void vtkMimxUnstructuredGridExtractWidget::ExtractFace()
{
        vtkGeometryFilter* geofil = vtkGeometryFilter::New();
        geofil->SetInput(this->UGrid);
        geofil->Update();
//      geofil->GetOutput()->BuildLinks();
        this->PolyData->SetPoints(geofil->GetOutput()->GetPoints());
        this->PolyData->SetPolys(geofil->GetOutput()->GetPolys());
        this->PolyData->Modified();
        this->PolyData->BuildLinks();
        this->PolyDataMapper->Delete();
        this->PolyDataMapper = vtkPolyDataMapper::New();
        this->PolyDataMapper->SetInput(this->PolyData);
        this->PolyDataActor->Delete();
        this->PolyDataActor = vtkActor::New();
        this->PolyDataActor->SetMapper(this->PolyDataMapper);
        this->PolyDataActor->GetProperty()->SetLineWidth(3);
        this->PolyDataActor->GetProperty()->SetRepresentationToWireframe();
        geofil->Delete();
}

void vtkMimxUnstructuredGridExtractWidget::DeleteHandles()
{
        vtkIdType numPts = this->HandleGeometry->GetNumberOfItems();
        this->HandleGeometry->InitTraversal();
        this->HandleMapper->InitTraversal();
        this->Handle->InitTraversal();
        while (numPts !=0) {
                this->HandleGeometry->GetNextItemAsObject()->Delete();
                this->HandleMapper->GetNextItemAsObject()->Delete();
                this->Handle->GetNextItemAsObject()->Delete();
                numPts--;
        }
        this->HandleGeometry->RemoveAllItems();
        this->HandleMapper->RemoveAllItems();
        this->Handle->RemoveAllItems();
}

void vtkMimxUnstructuredGridExtractWidget::SizeHandles()
{
        for(int i=0; i< this->Handle->GetNumberOfItems(); i++)
        {
                ((vtkSphereSource*) (this->HandleGeometry->GetItemAsObject(i)))
                        ->SetRadius(this->HandleSize);
        }
}
vtkIdType vtkMimxUnstructuredGridExtractWidget::ReturnPickedCell()
{
        return this->EWCallback->pickedcell;

}
