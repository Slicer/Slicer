/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxExtractCellWidget.cxx,v $
Language:  C++

Date:      $Date: 2008/08/06 21:29:12 $
Version:   $Revision: 1.10 $


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
#include "vtkMimxExtractCellWidget.h"

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
#include "vtkUnstructuredGrid.h"
#include "vtkDataSetMapper.h"
#include "vtkProperty.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkSphereSource.h"
#include "vtkTransform.h"
#include "vtkPolyDataMapper.h"
#include "vtkGeometryFilter.h"
#include "vtkPolyDataNormals.h"
#include "vtkPointData.h"
#include "vtkIntArray.h"
#include "vtkPointData.h"
#include "vtkLookupTable.h"
#include "vtkCellData.h"
#include "vtkWidgetCallbackMapper.h"
#include "vtkWidgetEvent.h"
#include "vtkEvent.h"
#include "vtkWidgetEventTranslator.h"
#include "vtkActorCollection.h"
#include "vtkProperty.h"

vtkCxxRevisionMacro(vtkMimxExtractCellWidget, "$Revision: 1.10 $");
vtkStandardNewMacro(vtkMimxExtractCellWidget);

//----------------------------------------------------------------------
vtkMimxExtractCellWidget::vtkMimxExtractCellWidget()
{
  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::LeftButtonPressEvent,
    vtkEvent::ShiftModifier, 0, 1, NULL,
    vtkMimxExtractCellWidget::ShiftLeftMouseButtonDown,
    this, vtkMimxExtractCellWidget::ShiftLeftButtonDownCallback);

  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::LeftButtonReleaseEvent,
    vtkEvent::ShiftModifier, 0, 1, NULL,
    vtkMimxExtractCellWidget::ShiftLeftMouseButtonUp,
    this, vtkMimxExtractCellWidget::ShiftLeftButtonUpCallback);

  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
                                          vtkMimxExtractCellWidget::LeftMouseButtonDown,
                                          this, vtkMimxExtractCellWidget::LeftButtonDownCallback);


  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                          vtkMimxExtractCellWidget::LeftMouseButtonUp,
                                          this, vtkMimxExtractCellWidget::LeftButtonUpCallback);

  this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonPressEvent,
                                          vtkMimxExtractCellWidget::RightMouseButtonDown,
                                          this, vtkMimxExtractCellWidget::RightButtonDownCallback);


  this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonReleaseEvent,
                                          vtkMimxExtractCellWidget::RightMouseButtonUp,
                                          this, vtkMimxExtractCellWidget::RightButtonUpCallback);

  this->State = vtkMimxExtractCellWidget::Start;
        
  this->UGrid = NULL;
  this->CompleteUGrid = NULL;
  this->InputActor = vtkActor::New();
  this->InputMapper = vtkDataSetMapper::New();
  this->InputActor->SetMapper(this->InputMapper);
  this->InputActor->GetProperty()->SetRepresentationToWireframe();
  this->PickedCellList = vtkIdList::New();
  this->CompletePickedCellList = vtkIdList::New();
  this->CellGeometry = vtkActorCollection::New();
  this->CellPicker = vtkCellPicker::New();
  this->CellPicker->SetTolerance(0.001);
}

//----------------------------------------------------------------------
vtkMimxExtractCellWidget::~vtkMimxExtractCellWidget()
{
  this->DeletePickedCells();
  this->PickedCellList->Delete();
  this->CompletePickedCellList->Delete();
  this->CellGeometry->Delete();
  this->CellPicker->Delete();
  this->InputMapper->Delete();
  this->InputActor->Delete();
}

//----------------------------------------------------------------------
void vtkMimxExtractCellWidget::SetEnabled(int enabling)
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
    // We're ready to enable
    this->Enabled = 1;
    this->CurrentRenderer->AddActor(this->InputActor);
    // listen for the events found in the EventTranslator
    if ( ! this->Parent )
      {
      this->EventTranslator->AddEventsToInteractor(this->Interactor,
                                                   this->EventCallbackCommand,this->Priority);
      }
    else
      {
      this->EventTranslator->AddEventsToParent(this->Parent,
                                               this->EventCallbackCommand,this->Priority);
      }
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
    if ( ! this->Parent )
      {
      this->Interactor->RemoveObserver(this->EventCallbackCommand);
      }
    else
      {
      this->Parent->RemoveObserver(this->EventCallbackCommand);
      }

    this->CurrentRenderer->RemoveActor(this->InputActor);
    this->InvokeEvent(vtkCommand::DisableEvent,NULL);
    this->DeletePickedCells();
    this->CurrentRenderer = NULL;
    }
  if ( this->Interactor && !this->Parent )
    {
    this->Interactor->Render();
    }
}
//--------------------------------------------------------------------------------------
void vtkMimxExtractCellWidget::ShiftLeftButtonDownCallback(vtkAbstractWidget *w)
{
  vtkMimxExtractCellWidget *Self = reinterpret_cast<vtkMimxExtractCellWidget*>(w);

  int X = Self->Interactor->GetEventPosition()[0];
  int Y = Self->Interactor->GetEventPosition()[1];

  // Okay, we can process Self. Try to pick handles first;
  // if no handles picked, then pick the bounding box.
  vtkRenderer *ren = Self->Interactor->FindPokedRenderer(X,Y);
  if ( ren != Self->CurrentRenderer )
    {
    Self->State = vtkMimxExtractCellWidget::Outside;
    return;
    }


  Self->State = vtkMimxExtractCellWidget::Start;
  // Okay, we can process Self. Try to pick handles first;
  // if no handles picked, then try to pick the line.
  vtkAssemblyPath *path;
  Self->CellPicker->AddPickList(Self->InputActor);
  Self->CellPicker->PickFromListOn();
  Self->CellPicker->Pick(X,Y,0.0, Self->CurrentRenderer);
  path = Self->CellPicker->GetPath();
  if(path !=NULL)
    {
    int PickedCell = Self->CellPicker->GetCellId();
    if(PickedCell > -1)
      {
      Self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
      // hide handles so that you can place a quad at the face picked
      int location = Self->PickedCellList->IsId(PickedCell);
      int i;
      if(location != -1)
        {
        vtkActor *curactor = vtkActor::SafeDownCast(
          Self->CellGeometry->GetItemAsObject(location));
        Self->CurrentRenderer->RemoveActor(curactor);
        curactor->Delete();
        Self->CellGeometry->RemoveItem(location);

        vtkIdList *IdList = vtkIdList::New();
        IdList->DeepCopy(Self->PickedCellList);
        Self->PickedCellList->Initialize();
        for (i=0; i<IdList->GetNumberOfIds(); i++)
          if(i != location)
            Self->PickedCellList->InsertNextId(IdList->GetId(i));

        IdList->Initialize();
        IdList->DeepCopy(Self->CompletePickedCellList);
        Self->CompletePickedCellList->Initialize();
        for (i=0; i<IdList->GetNumberOfIds(); i++)
          if(i != location)
            Self->CompletePickedCellList->InsertNextId(IdList->GetId(i));

        IdList->Delete();
        }
      else
        {
        Self->PickedCellList->InsertNextId(PickedCell);
        vtkIntArray *cellarray = vtkIntArray::SafeDownCast(
          Self->UGrid->GetCellData()->GetArray("Original_Cell_Ids"));
        if(cellarray)
          {
          Self->CompletePickedCellList->InsertNextId(cellarray->GetValue(PickedCell));
          }
        vtkDataSetMapper *selectedCellMapper = vtkDataSetMapper::New();
        vtkActor *selectedCellActor = vtkActor::New();

        vtkIdList *cellIds = vtkIdList::New();
        Self->UGrid->GetCellPoints(PickedCell, cellIds);

        vtkUnstructuredGrid *selectedCellGrid = vtkUnstructuredGrid::New();
        selectedCellGrid->SetPoints(Self->UGrid->GetPoints());
        selectedCellGrid->InsertNextCell(
          Self->UGrid->GetCellType(PickedCell), cellIds);
        selectedCellMapper->SetInput(selectedCellGrid);
        selectedCellActor->SetMapper(selectedCellMapper);
        selectedCellActor->GetProperty()->SetColor(0.0,1.0,0.0);
        selectedCellActor->GetProperty()->SetRepresentationToSurface();
        Self->CurrentRenderer->AddActor(selectedCellActor);
        Self->CellGeometry->AddItem(selectedCellActor);
        selectedCellGrid->Delete();
        selectedCellMapper->Delete();
        }
      }
    }
  else
    {
    Self->State = vtkMimxExtractCellWidget::Outside;
    return;
    }
  Self->EventCallbackCommand->SetAbortFlag(1);
  Self->StartInteraction();
  Self->Interactor->Render();
}
//-------------------------------------------------------------------------
void vtkMimxExtractCellWidget::ShiftLeftButtonUpCallback(vtkAbstractWidget *w)
{
  vtkMimxExtractCellWidget *Self = reinterpret_cast<vtkMimxExtractCellWidget*>(w);
  Self->State = vtkMimxExtractCellWidget::Start;
  //Self->EventCallbackCommand->SetAbortFlag(1);
  Self->EndInteraction();
  Self->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
  Self->Interactor->Render();
}
//-----------------------------------------------------------------------------------
void vtkMimxExtractCellWidget::Initialize()
{
  if(this->UGrid)
    {
    this->DeletePickedCells();
    this->CellPicker->InitializePickList();
    this->InputMapper->SetInput(this->UGrid);
    this->InputMapper->Modified();
    this->CellPicker->AddPickList(this->InputActor);
    this->CellPicker->PickFromListOn();
    }
}
//------------------------------------------------------------------------------------------
void vtkMimxExtractCellWidget::SetInput(vtkDataSet *Input)
{
  this->UGrid = vtkUnstructuredGrid::SafeDownCast(Input);
  this->Initialize();
}
//-------------------------------------------------------------------------------------------
void vtkMimxExtractCellWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//-----------------------------------------------------------------------------------------------
void vtkMimxExtractCellWidget::DeletePickedCells()
{
  int numitems = this->CellGeometry->GetNumberOfItems();
  if(numitems)
    {
    int i;
    for (i=0; i<this->CellGeometry->GetNumberOfItems(); i++)
      {
      vtkActor *curactor = vtkActor::SafeDownCast(
        this->CellGeometry->GetItemAsObject(i));
      this->CurrentRenderer->RemoveActor(curactor);
      curactor->Delete();
      }
    do 
      {
      this->CellGeometry->RemoveItem(0);
      } 
    while(this->CellGeometry->GetNumberOfItems() != 0);
    }
  this->PickedCellList->Initialize();
  this->CompletePickedCellList->Initialize();
}
//-------------------------------------------------------------------------------------------------
void vtkMimxExtractCellWidget::LeftButtonDownCallback(vtkAbstractWidget *w)
{
  vtkMimxExtractCellWidget *Self = reinterpret_cast<vtkMimxExtractCellWidget*>(w);

  int X = Self->Interactor->GetEventPosition()[0];
  int Y = Self->Interactor->GetEventPosition()[1];

  // Okay, we can process Self. Try to pick handles first;
  // if no handles picked, then pick the bounding box.
  vtkRenderer *ren = Self->Interactor->FindPokedRenderer(X,Y);
  if ( ren != Self->CurrentRenderer )
    {
    Self->State = vtkMimxExtractCellWidget::Outside;
    return;
    }


  Self->State = vtkMimxExtractCellWidget::Start;
  // Okay, we can process Self. Try to pick handles first;
  // if no handles picked, then try to pick the line.
  vtkAssemblyPath *path;
  Self->CellPicker->Pick(X,Y,0.0, Self->CurrentRenderer);
  path = Self->CellPicker->GetPath();
  if(path !=NULL)
    {
    int PickedCell = Self->CellPicker->GetCellId();
    if(PickedCell > -1)
      {
      //if(Self->CellGeometry->GetNumberOfItems() > 1)
      //{
      Self->DeletePickedCells();
      //}
      Self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
      // hide handles so that you can place a quad at the face picked
      //int location = Self->PickedCellList->IsId(PickedCell);
      //Self->DeletePickedCells();
      //if(location == -1)
      //{
      Self->PickedCellList->InsertNextId(PickedCell);
      vtkIntArray *cellarray = vtkIntArray::SafeDownCast(
        Self->UGrid->GetCellData()->GetArray("Original_Cell_Ids"));
      if(cellarray)
        {
        Self->CompletePickedCellList->InsertNextId(cellarray->GetValue(PickedCell));
        }
      vtkDataSetMapper *selectedCellMapper = vtkDataSetMapper::New();
      vtkActor *selectedCellActor = vtkActor::New();

      vtkIdList *cellIds = vtkIdList::New();
      Self->UGrid->GetCellPoints(PickedCell, cellIds);

      vtkUnstructuredGrid *selectedCellGrid = vtkUnstructuredGrid::New();
      selectedCellGrid->SetPoints(Self->UGrid->GetPoints());
      selectedCellGrid->InsertNextCell(
        Self->UGrid->GetCellType(PickedCell), cellIds);
      selectedCellMapper->SetInput(selectedCellGrid);
      selectedCellActor->SetMapper(selectedCellMapper);
      selectedCellActor->GetProperty()->SetColor(0.0,1.0,0.0);
      selectedCellActor->GetProperty()->SetRepresentationToSurface();
      Self->CurrentRenderer->AddActor(selectedCellActor);
      Self->CellGeometry->AddItem(selectedCellActor);
      selectedCellGrid->Delete();
      selectedCellMapper->Delete();
      //}
      }
    }
  else
    {
    Self->State = vtkMimxExtractCellWidget::Outside;
    return;
    }
  Self->EventCallbackCommand->SetAbortFlag(1);
  Self->StartInteraction();
  Self->Interactor->Render();
}
//----------------------------------------------------------------------
void vtkMimxExtractCellWidget::LeftButtonUpCallback(vtkAbstractWidget *w)
{
  vtkMimxExtractCellWidget *Self = reinterpret_cast<vtkMimxExtractCellWidget*>(w);

  //Self->EventCallbackCommand->SetAbortFlag(1);
  Self->EndInteraction();
  Self->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);

  Self->Interactor->Render();
}
//---------------------------------------------------------------------------------------
void vtkMimxExtractCellWidget::RightButtonDownCallback(vtkAbstractWidget *w)
{
  vtkMimxExtractCellWidget *Self = reinterpret_cast<vtkMimxExtractCellWidget*>(w);
  int X = Self->Interactor->GetEventPosition()[0];
  int Y = Self->Interactor->GetEventPosition()[1];

  // Okay, we can process this. Try to pick handles first;
  // if no handles picked, then pick the bounding box.
  if (!Self->CurrentRenderer || !Self->CurrentRenderer->IsInViewport(X, Y))
    {
    Self->State = vtkMimxExtractCellWidget::Outside;
    return;
    }
  
  vtkAssemblyPath *path;
  Self->CellPicker->Pick(X,Y,0.0, Self->CurrentRenderer);
  path = Self->CellPicker->GetPath();
  if(path !=NULL)
    {
    int PickedCell = Self->CellPicker->GetCellId();
    if(PickedCell > -1)
      {
      Self->InvokeEvent(vtkCommand::RightButtonPressEvent, NULL);
      Self->InvokeEvent(vtkCommand::RightButtonReleaseEvent, NULL);
      Self->EventCallbackCommand->SetAbortFlag(1);
      Self->StartInteraction();
      Self->InvokeEvent(vtkCommand::StartInteractionEvent, NULL);
      Self->Interactor->Render();
      return;
      }
    }
  else
    {
    return;
    }

  Self->EventCallbackCommand->SetAbortFlag(1);
  Self->StartInteraction();
  Self->InvokeEvent(vtkCommand::StartInteractionEvent, NULL);
  Self->Interactor->Render();
}
//----------------------------------------------------------------------------------------------------
void vtkMimxExtractCellWidget::RightButtonUpCallback(vtkAbstractWidget *w)
{
  vtkMimxExtractCellWidget *Self = reinterpret_cast<vtkMimxExtractCellWidget*>(w);
  if ( Self->State == vtkMimxExtractCellWidget::Outside ||
       Self->State == vtkMimxExtractCellWidget::Start )
    {
    return;
    }

  Self->State = vtkMimxExtractCellWidget::Start;

  Self->EventCallbackCommand->SetAbortFlag(1);
  Self->EndInteraction();
  Self->InvokeEvent(vtkCommand::EndInteractionEvent, NULL);
  Self->Interactor->Render();
}
//----------------------------------------------------------------------------------------------------
