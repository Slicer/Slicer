/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxExtractFaceWidget.cxx,v $
Language:  C++
Date:      $Date: 2008/06/28 01:24:43 $
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
#include "vtkMimxExtractFaceWidget.h"

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

vtkCxxRevisionMacro(vtkMimxExtractFaceWidget, "$Revision: 1.15 $");
vtkStandardNewMacro(vtkMimxExtractFaceWidget);

//----------------------------------------------------------------------
vtkMimxExtractFaceWidget::vtkMimxExtractFaceWidget()
{
  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::LeftButtonPressEvent,
    vtkEvent::ShiftModifier, 0, 1, NULL,
    vtkMimxExtractFaceWidget::ShiftLeftMouseButtonDown,
    this, vtkMimxExtractFaceWidget::ShiftLeftButtonDownCallback);

  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::LeftButtonReleaseEvent,
    vtkEvent::ShiftModifier, 0, 1, NULL,
    vtkMimxExtractFaceWidget::ShiftLeftMouseButtonUp,
    this, vtkMimxExtractFaceWidget::ShiftLeftButtonUpCallback);

  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
                                          vtkMimxExtractFaceWidget::LeftMouseButtonDown,
                                          this, vtkMimxExtractFaceWidget::LeftButtonDownCallback);


  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                          vtkMimxExtractFaceWidget::LeftMouseButtonUp,
                                          this, vtkMimxExtractFaceWidget::LeftButtonUpCallback);

  this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonPressEvent,
                                          vtkMimxExtractFaceWidget::RightMouseButtonDown,
                                          this, vtkMimxExtractFaceWidget::RightButtonDownCallback);


  this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonReleaseEvent,
                                          vtkMimxExtractFaceWidget::RightMouseButtonUp,
                                          this, vtkMimxExtractFaceWidget::RightButtonUpCallback);

  this->State = vtkMimxExtractFaceWidget::Start;
        
  this->UGrid = NULL;
  this->CompleteUGrid = NULL;
  // Construct the poly data representing the hex
  this->GeometryFilter = vtkGeometryFilter::New();
  this->FacePolyData = GeometryFilter->GetOutput();
  this->FaceMapper = vtkPolyDataMapper::New();
  this->FaceMapper->SetInput(FacePolyData);
  this->FaceActor = vtkActor::New();
  this->FaceActor->SetMapper(this->FaceMapper);
  this->FaceActor->GetProperty()->SetRepresentationToWireframe();

  this->SelectedFaceMapper = NULL;
  this->SelectedFaceActor = NULL;
  //Manage the picking stuff
  this->FacePicker = vtkCellPicker::New();
  this->FacePicker->SetTolerance(0.001);
  //this->FacePicker->AddPickList(this->FaceActor);
  //this->FacePicker->PickFromListOn();
  this->InputActor = NULL;
  this->PickedCellList = vtkIdList::New();
  this->PickedFaceList = vtkIdList::New();
  this->CompletePickedCellList = vtkIdList::New();
  this->CompletePickedFaceList = vtkIdList::New();
  this->SurfaceCellList = vtkIdList::New();

  this->FaceGeometry = vtkActorCollection::New();
  this->InteriorFaceGeometry = vtkActorCollection::New();
}

//----------------------------------------------------------------------
vtkMimxExtractFaceWidget::~vtkMimxExtractFaceWidget()
{
  this->RemoveHighlightedFaces(this);
  this->FacePicker->Delete();
  this->FaceGeometry->Delete();
  this->InteriorFaceGeometry->Delete();
  this->FacePolyData->Delete();
//      this->FaceMapper->Delete();
//      this->FaceActor->SetMapper(NULL);
  this->FaceActor->Delete();
  this->PickedCellList->Delete();
  this->PickedFaceList->Delete();
  this->CompletePickedCellList->Delete();
  this->CompletePickedFaceList->Delete();
  this->SurfaceCellList->Delete();
  if (this->SelectedFaceActor)
    this->SelectedFaceActor->Delete();
  if (this->SelectedFaceMapper)
    this->SelectedFaceMapper->Delete();
  this->GeometryFilter->Delete();
}

//----------------------------------------------------------------------
void vtkMimxExtractFaceWidget::SetEnabled(int enabling)
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
    // Add the various actors
    this->CurrentRenderer->AddActor(this->FaceActor);
    if(this->InputActor)
      this->CurrentRenderer->RemoveActor(this->InputActor);
    this->ShowInteriorHighlightedFaces();           
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

    this->InvokeEvent(vtkCommand::DisableEvent,NULL);
    this->RemoveHighlightedFaces(this);
    this->CurrentRenderer->RemoveActor(this->FaceActor);
    if(this->InputActor)
      this->CurrentRenderer->AddActor(this->InputActor);
    this->DeleteInteriorHighlightedFaces();
    this->CurrentRenderer = NULL;

    }

  if ( this->Interactor && !this->Parent )
    {
    this->Interactor->Render();
    }
}
//--------------------------------------------------------------------------------------
void vtkMimxExtractFaceWidget::ShiftLeftButtonDownCallback(vtkAbstractWidget *w)
{
  vtkMimxExtractFaceWidget *Self = reinterpret_cast<vtkMimxExtractFaceWidget*>(w);

  int X = Self->Interactor->GetEventPosition()[0];
  int Y = Self->Interactor->GetEventPosition()[1];

  // Okay, we can process Self. Try to pick handles first;
  // if no handles picked, then pick the bounding box.
  vtkRenderer *ren = Self->Interactor->FindPokedRenderer(X,Y);
  if ( ren != Self->CurrentRenderer )
    {
    Self->State = vtkMimxExtractFaceWidget::Outside;
    return;
    }


  Self->State = vtkMimxExtractFaceWidget::Start;
  // Okay, we can process Self. Try to pick handles first;
  // if no handles picked, then try to pick the line.
  vtkAssemblyPath *path;
  Self->FacePicker->Pick(X,Y,0.0, Self->CurrentRenderer);
  path = Self->FacePicker->GetPath();
  if(path !=NULL)
    {
    int PickedCell = Self->FacePicker->GetCellId();
    if(PickedCell > -1)
      {
      Self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
      // hide handles so that you can place a quad at the face picked
      Self->State = vtkMimxExtractFaceWidget::PickMultipleFace;
      int location = Self->SurfaceCellList->IsId(PickedCell);
      int i;
      if(location != -1)
        {
        vtkActor *curactor = vtkActor::SafeDownCast(
          Self->FaceGeometry->GetItemAsObject(location));
        Self->CurrentRenderer->RemoveActor(curactor);
        curactor->Delete();
        Self->FaceGeometry->RemoveItem(location);

        vtkIdList *IdList = vtkIdList::New();
        IdList->DeepCopy(Self->PickedCellList);
        Self->PickedCellList->Initialize();
        for (i=0; i<IdList->GetNumberOfIds(); i++)
          if(i != location)
            Self->PickedCellList->InsertNextId(IdList->GetId(i));

        IdList->Initialize();
        IdList->DeepCopy(Self->PickedFaceList);
        Self->PickedFaceList->Initialize();
        for (i=0; i<IdList->GetNumberOfIds(); i++)
          if(i != location)
            Self->PickedFaceList->InsertNextId(IdList->GetId(i));

        IdList->Initialize();
        IdList->DeepCopy(Self->CompletePickedCellList);
        Self->CompletePickedCellList->Initialize();
        for (i=0; i<IdList->GetNumberOfIds(); i++)
          if(i != location)
            Self->CompletePickedCellList->InsertNextId(IdList->GetId(i));

        IdList->Initialize();
        IdList->DeepCopy(Self->CompletePickedFaceList);
        Self->CompletePickedFaceList->Initialize();
        for (i=0; i<IdList->GetNumberOfIds(); i++)
          if(i != location)
            Self->CompletePickedFaceList->InsertNextId(IdList->GetId(i));

        Self->SurfaceCellList->DeleteId(PickedCell);
        IdList->Delete();
        }
      else
        {
        // check if the latest cell picked is valid
        // if it is valid, add the same to the list
        int CellFace;
        int CompleteCell, CompleteFace;
        int CellNum = Self->GetInputPickedCellAndFace(PickedCell, CellFace);
        if(Self->GetInputPickedCompleteFace(CellNum, CellFace, CompleteCell, CompleteFace))
          {
          bool status = true;
          for (i=0; i<Self->PickedCellList->GetNumberOfIds(); i++)
            {
            if(Self->PickedCellList->GetId(i) == CellNum && 
               Self->PickedFaceList->GetId(i) == CellFace)
              {
              Self->PickedCellList->SetId(i, -1);
              Self->PickedFaceList->SetId(i, -1);
              Self->CompletePickedCellList->SetId(i, -1);
              Self->CompletePickedFaceList->SetId(i, -1);
              status = false;
              break;
              }
            }
          if(status)
            {
            Self->FaceGeometry->AddItem(vtkActor::New());
            int curnum = Self->FaceGeometry->GetNumberOfItems()-1;
            vtkActor *curactor = vtkActor::SafeDownCast(
              Self->FaceGeometry->GetItemAsObject(curnum));
            vtkCellArray *cellarray = vtkCellArray::New();
            vtkPolyData *surfacepolydata = Self->FaceMapper->GetInput();
            cellarray->InsertNextCell(surfacepolydata->GetCell(Self->FacePicker->GetCellId()));
            vtkPolyData *polydata = vtkPolyData::New();
            polydata->SetPoints(Self->UGrid->GetPoints());
            polydata->SetPolys(cellarray);
            cellarray->Delete();
            vtkPolyDataMapper *mapper = vtkPolyDataMapper::New();
            mapper->SetInput(polydata);
            polydata->Delete();
            curactor->SetMapper(mapper);
            mapper->Delete();
            curactor->GetProperty()->SetColor(0.0,1.0,0.0);
            Self->CurrentRenderer->AddActor(curactor);
            Self->FacePicker->GetPickPosition(Self->LastPickPosition);
            Self->PickedCellList->InsertNextId(CellNum);
            Self->PickedFaceList->InsertNextId(CellFace);
            Self->CompletePickedCellList->InsertNextId(CompleteCell);
            Self->CompletePickedFaceList->InsertNextId(CompleteFace);
            Self->SurfaceCellList->InsertNextId(PickedCell);
            }
          else
            {
            vtkIdList *IdList = vtkIdList::New();
            IdList->DeepCopy(Self->PickedCellList);
            Self->PickedCellList->Initialize();
            for (i=0; i<IdList->GetNumberOfIds(); i++)
              if(IdList->GetId(i) != -1)
                Self->PickedCellList->InsertNextId(IdList->GetId(i));

            IdList->Initialize();
            IdList->DeepCopy(Self->PickedFaceList);
            Self->PickedFaceList->Initialize();
            for (i=0; i<IdList->GetNumberOfIds(); i++)
              if(IdList->GetId(i) != -1)
                Self->PickedFaceList->InsertNextId(IdList->GetId(i));

            IdList->Initialize();
            IdList->DeepCopy(Self->CompletePickedCellList);
            Self->CompletePickedCellList->Initialize();
            for (i=0; i<IdList->GetNumberOfIds(); i++)
              if(IdList->GetId(i) != -1)
                Self->CompletePickedCellList->InsertNextId(IdList->GetId(i));

            IdList->Initialize();
            IdList->DeepCopy(Self->CompletePickedFaceList);
            Self->CompletePickedFaceList->Initialize();
            for (i=0; i<IdList->GetNumberOfIds(); i++)
              if(IdList->GetId(i) != -1)
                Self->CompletePickedFaceList->InsertNextId(IdList->GetId(i));

            Self->SurfaceCellList->DeleteId(PickedCell);
            IdList->Delete();
            }
          }
        else
          {
          // cannot user vtkwarning macro in a static class
//                              vtkWarningMacro("Face picked is interior to the full/complete mesh");
          }
        }
      }
    }
  else
    {
    Self->State = vtkMimxExtractFaceWidget::Outside;
    return;
    }
  Self->EventCallbackCommand->SetAbortFlag(1);
  Self->StartInteraction();
  Self->Interactor->Render();
}
//-------------------------------------------------------------------------
void vtkMimxExtractFaceWidget::ShiftLeftButtonUpCallback(vtkAbstractWidget *w)
{
  vtkMimxExtractFaceWidget *Self = reinterpret_cast<vtkMimxExtractFaceWidget*>(w);
  Self->State = vtkMimxExtractFaceWidget::Start;
  //Self->EventCallbackCommand->SetAbortFlag(1);
  Self->EndInteraction();
  Self->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
  Self->Interactor->Render();
}
//------------------------------------------------------------------------------------------
void vtkMimxExtractFaceWidget::ExtractFace()
{
  this->GeometryFilter->SetInput(this->UGrid);
  this->GeometryFilter->Update();
  this->FacePolyData = this->GeometryFilter->GetOutput();
  this->FacePolyData->Modified();
  this->FacePolyData->BuildLinks();
}
//-----------------------------------------------------------------------------------
void vtkMimxExtractFaceWidget::Initialize()
{
  if(this->UGrid)
    {
    this->DeleteInteriorHighlightedFaces();
    // if the input data changes
    //if(this->FacePolyData->GetPoints())
    //      this->FacePolyData->GetPoints()->Initialize();
    //if (this->FacePolyData->GetPolys())
    //      this->FacePolyData->GetPolys()->Initialize();
    //this->FacePolyData->Initialize();
    this->ExtractFace();
    this->FacePicker->InitializePickList();
    this->FacePicker->AddPickList(this->FaceActor);
    this->FacePicker->PickFromListOn();
    this->RemoveHighlightedFaces(this);
    this->ComputeInteriorHighlightedFaces();
    }
}
//------------------------------------------------------------------------------------------
void vtkMimxExtractFaceWidget::SetInput(vtkDataSet *Input)
{
  this->UGrid = vtkUnstructuredGrid::SafeDownCast(Input);
  this->Initialize();
  this->InputActor = NULL;
}
//-------------------------------------------------------------------------------------------
int vtkMimxExtractFaceWidget::GetInputPickedCellAndFace(
  int PickedFace, int &CellFace)
{
  // check to which cell and face it belongs
  int i,j, k;
  vtkIdList *surfacepointlist;
  vtkIdList *cellfacepointlist;
  // get pointids of the picked cell in the surface
  surfacepointlist = this->FacePolyData->GetCell(PickedFace)->GetPointIds();
  for(i=0; i<this->UGrid->GetNumberOfCells(); i++)
    {
    // loop through all the faces of the list
    vtkCell *solidcell = this->UGrid->GetCell(i);
    for (j=0; j< solidcell->GetNumberOfFaces(); j++)
      {
      cellfacepointlist = solidcell->GetFace(j)->GetPointIds();
      bool status = true;
      for (k=0; k< surfacepointlist->GetNumberOfIds(); k++)
        {
        if(cellfacepointlist->IsId(surfacepointlist->GetId(k)) == -1)
          {
          status = false;
          break;
          }
        }
      if(status)
        {
        CellFace = j;
        return i;
        }
      }
    }
  return -1;
}
//-----------------------------------------------------------------------------------------------
int vtkMimxExtractFaceWidget::GetInputPickedCompleteFace(
  int CellNum, int CellFace, int &CompleteCell, int &CompleteFace)
{
  int i, j, k;
  // check if the given face is interior to the complete mesh
  // if yes return -1
  vtkGeometryFilter *fil = vtkGeometryFilter::New();
  fil->SetInput(this->CompleteUGrid);
  fil->Update();
  vtkPolyData *compsurfpoly = fil->GetOutput();
  compsurfpoly->BuildLinks();
  //
  vtkIntArray *completearray = vtkIntArray::SafeDownCast(
    this->UGrid->GetPointData()->GetArray("Original_Point_Ids"));
  if(!completearray)
    {
    return 0;
    }

  vtkIdList *partialfaceidlist = 
    this->UGrid->GetCell(CellNum)->GetFace(CellFace)->GetPointIds();

  vtkIdList *actualidlist = vtkIdList::New();

  for (i=0; i<partialfaceidlist->GetNumberOfIds(); i++)
    {
    actualidlist->InsertNextId(completearray->GetValue(partialfaceidlist->GetId(i)));       
    }
  vtkIdList *completecellidlist;

  bool status = true;
  for (i=0; i<compsurfpoly->GetNumberOfCells(); i++)
    {
    status = true;
    completecellidlist = compsurfpoly->GetCell(i)->GetPointIds();
    for (j=0; j< actualidlist->GetNumberOfIds(); j++)
      {
      if(completecellidlist->IsId(actualidlist->GetId(j)) == -1)
        {
        status = false;
        break;
        }
      }
    if(status)
      {
      break;
      }
    }
  if (!status)
    {
    fil->Delete();
    actualidlist->Delete();
    return 0;
    }
  //      vtkIdList *completefaceidlist;
  for (i=0; i<this->CompleteUGrid->GetNumberOfCells(); i++)
    {
    vtkCell *solidcell = this->CompleteUGrid->GetCell(i);
    for (j=0; j<solidcell->GetNumberOfFaces(); j++)
      {
      status = true;
      completecellidlist = solidcell->GetFace(j)->GetPointIds();
      for (k=0; k< actualidlist->GetNumberOfIds(); k++)
        {
        if(completecellidlist->IsId(actualidlist->GetId(k)) == -1)
          {
          status = false;
          break;
          }
        }
      if(status)
        {
        status = true;
        for (k=0; k< actualidlist->GetNumberOfIds(); k++)
          {
          vtkIdType *cells;
          unsigned short ncells;
          compsurfpoly->GetPointCells(actualidlist->GetId(k), ncells, cells);
          if(!ncells)
            {
            status = false;
            break;
            }
          }
        if(status)
          {
          CompleteFace  = j;
          CompleteCell = i;
          fil->Delete();
          actualidlist->Delete();
          return 1;
          }
        }
      }
    }
  fil->Delete();
  actualidlist->Delete();
  return 0;
}
//-------------------------------------------------------------------------------------------
void vtkMimxExtractFaceWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//-----------------------------------------------------------------------------------------------
void vtkMimxExtractFaceWidget::RemoveHighlightedFaces(vtkMimxExtractFaceWidget *Self)
{
  int numitems = Self->FaceGeometry->GetNumberOfItems();
  if(numitems)
    {
    int i;
    for (i=0; i<Self->FaceGeometry->GetNumberOfItems(); i++)
      {
      vtkActor *curactor = vtkActor::SafeDownCast(
        Self->FaceGeometry->GetItemAsObject(i));
      Self->CurrentRenderer->RemoveActor(curactor);
      curactor->Delete();
      }
    do 
      {
      Self->FaceGeometry->RemoveItem(0);
      }
    while(Self->FaceGeometry->GetNumberOfItems() != 0);
    }
  this->PickedCellList->Initialize();
  this->PickedFaceList->Initialize();
  this->CompletePickedCellList->Initialize();
  this->CompletePickedFaceList->Initialize();
  this->SurfaceCellList->Initialize();
}
//-------------------------------------------------------------------------------------------------
void vtkMimxExtractFaceWidget::ShowInteriorHighlightedFaces()
{
  int numitems = this->InteriorFaceGeometry->GetNumberOfItems();
  int i;

  for (i=0; i<numitems; i++)
    {
    vtkActor *curactor = vtkActor::SafeDownCast(
      this->InteriorFaceGeometry->GetItemAsObject(i));
    this->CurrentRenderer->AddActor(curactor);
    }
}
//-----------------------------------------------------------------------------------------------
void vtkMimxExtractFaceWidget::HideInteriorHighlightedFaces()
{
  int numitems = this->InteriorFaceGeometry->GetNumberOfItems();
  int i;

  for (i=0; i<numitems; i++)
    {
    vtkActor *curactor = vtkActor::SafeDownCast(
      this->InteriorFaceGeometry->GetItemAsObject(i));
    this->CurrentRenderer->RemoveActor(curactor);
    }
}
//-----------------------------------------------------------------------------------------------
void vtkMimxExtractFaceWidget::DeleteInteriorHighlightedFaces()
{
  this->HideInteriorHighlightedFaces();
  int numitems = this->InteriorFaceGeometry->GetNumberOfItems();
  if(numitems)
    {
    int i;
    for (i=0; i<this->InteriorFaceGeometry->GetNumberOfItems(); i++)
      {
      vtkActor *curactor = vtkActor::SafeDownCast(
        this->InteriorFaceGeometry->GetItemAsObject(i));
      curactor->Delete();
      }
    do 
      {
      this->InteriorFaceGeometry->RemoveItem(0);
      }
    while(this->InteriorFaceGeometry->GetNumberOfItems() != 0);
    }
}
//-----------------------------------------------------------------------------------------------
void vtkMimxExtractFaceWidget::ComputeInteriorHighlightedFaces()
{
  int i,CellFace;
  int CompleteCell, CompleteFace;
  int numCells = this->FacePolyData->GetNumberOfCells();

  for (i=0; i<numCells; i++)
    {
    int CellNum = this->GetInputPickedCellAndFace(i, CellFace);
    if(!this->GetInputPickedCompleteFace(CellNum, CellFace, CompleteCell, CompleteFace))
      {
      this->InteriorFaceGeometry->AddItem(vtkActor::New());
      int curnum = this->InteriorFaceGeometry->GetNumberOfItems()-1;
      vtkActor *curactor = vtkActor::SafeDownCast(
        this->InteriorFaceGeometry->GetItemAsObject(curnum));

      vtkCellArray *cellarray = vtkCellArray::New();
      cellarray->InsertNextCell(this->FacePolyData->GetCell(i));

      vtkPolyData *polydata = vtkPolyData::New();
      polydata->SetPoints(this->FacePolyData->GetPoints());
      polydata->SetPolys(cellarray);
      cellarray->Delete();
      vtkPolyDataMapper *mapper = vtkPolyDataMapper::New();
      mapper->SetInput(polydata);
      polydata->Delete();
      curactor->SetMapper(mapper);
      mapper->Delete();
      curactor->GetProperty()->SetColor(1.0,0.0,0.0);
      }
    }
}
//------------------------------------------------------------------------------------------------
void vtkMimxExtractFaceWidget::LeftButtonDownCallback(vtkAbstractWidget *w)
{
  vtkMimxExtractFaceWidget *Self = reinterpret_cast<vtkMimxExtractFaceWidget*>(w);

  int X = Self->Interactor->GetEventPosition()[0];
  int Y = Self->Interactor->GetEventPosition()[1];

  // Okay, we can process Self. Try to pick handles first;
  // if no handles picked, then pick the bounding box.
  vtkRenderer *ren = Self->Interactor->FindPokedRenderer(X,Y);
  if ( ren != Self->CurrentRenderer )
    {
    Self->State = vtkMimxExtractFaceWidget::Outside;
    return;
    }


  Self->State = vtkMimxExtractFaceWidget::Start;
  // Okay, we can process Self. Try to pick handles first;
  // if no handles picked, then try to pick the line.
  vtkAssemblyPath *path;
  Self->FacePicker->Pick(X,Y,0.0, Self->CurrentRenderer);
  path = Self->FacePicker->GetPath();
  if(path !=NULL)
    {
    int PickedCell = Self->FacePicker->GetCellId();
    if(PickedCell > -1)
      {
      Self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
      // hide handles so that you can place a quad at the face picked
      Self->State = vtkMimxExtractFaceWidget::PickMultipleFace;
      Self->RemoveHighlightedFaces(Self);
      // check if the latest cell picked is valid
      // if it is valid, add the same to the list
      int CellFace;
      int CompleteCell, CompleteFace;
      int CellNum = Self->GetInputPickedCellAndFace(PickedCell, CellFace);
      if(Self->GetInputPickedCompleteFace(CellNum, CellFace, CompleteCell, CompleteFace))
        {
        Self->FaceGeometry->AddItem(vtkActor::New());
        int curnum = Self->FaceGeometry->GetNumberOfItems()-1;
        vtkActor *curactor = vtkActor::SafeDownCast(
          Self->FaceGeometry->GetItemAsObject(curnum));
        vtkCellArray *cellarray = vtkCellArray::New();
        vtkPolyData *surfacepolydata = Self->FaceMapper->GetInput();
        cellarray->InsertNextCell(surfacepolydata->GetCell(Self->FacePicker->GetCellId()));
        vtkPolyData *polydata = vtkPolyData::New();
        polydata->SetPoints(Self->UGrid->GetPoints());
        polydata->SetPolys(cellarray);
        cellarray->Delete();
        vtkPolyDataMapper *mapper = vtkPolyDataMapper::New();
        mapper->SetInput(polydata);
        polydata->Delete();
        curactor->SetMapper(mapper);
        mapper->Delete();
        curactor->GetProperty()->SetColor(0.0,1.0,0.0);
        Self->CurrentRenderer->AddActor(curactor);
        Self->FacePicker->GetPickPosition(Self->LastPickPosition);
        Self->PickedCellList->InsertNextId(CellNum);
        Self->PickedFaceList->InsertNextId(CellFace);
        Self->CompletePickedCellList->InsertNextId(CompleteCell);
        Self->CompletePickedFaceList->InsertNextId(CompleteFace);
        Self->SurfaceCellList->InsertNextId(PickedCell);
        }
      }
    }
  else
    {
    Self->State = vtkMimxExtractFaceWidget::Outside;
    return;
    }
  Self->EventCallbackCommand->SetAbortFlag(1);
  Self->StartInteraction();
  Self->Interactor->Render();
}
//----------------------------------------------------------------------
void vtkMimxExtractFaceWidget::LeftButtonUpCallback(vtkAbstractWidget *w)
{
  vtkMimxExtractFaceWidget *Self = reinterpret_cast<vtkMimxExtractFaceWidget*>(w);

  //Self->EventCallbackCommand->SetAbortFlag(1);
  Self->EndInteraction();
  Self->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);

  Self->Interactor->Render();
}
//---------------------------------------------------------------------------------------
void vtkMimxExtractFaceWidget::RightButtonDownCallback(vtkAbstractWidget *w)
{
  vtkMimxExtractFaceWidget *Self = reinterpret_cast<vtkMimxExtractFaceWidget*>(w);
  int X = Self->Interactor->GetEventPosition()[0];
  int Y = Self->Interactor->GetEventPosition()[1];

  // Okay, we can process this. Try to pick handles first;
  // if no handles picked, then pick the bounding box.
  if (!Self->CurrentRenderer || !Self->CurrentRenderer->IsInViewport(X, Y))
    {
    Self->State = vtkMimxExtractFaceWidget::Outside;
    return;
    }
  
  vtkAssemblyPath *path;
  Self->FacePicker->Pick(X,Y,0.0, Self->CurrentRenderer);
  path = Self->FacePicker->GetPath();
  if(path !=NULL)
    {
    int PickedCell = Self->FacePicker->GetCellId();
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
void vtkMimxExtractFaceWidget::RightButtonUpCallback(vtkAbstractWidget *w)
{
  vtkMimxExtractFaceWidget *Self = reinterpret_cast<vtkMimxExtractFaceWidget*>(w);
  if ( Self->State == vtkMimxExtractFaceWidget::Outside ||
       Self->State == vtkMimxExtractFaceWidget::Start )
    {
    return;
    }

  Self->State = vtkMimxExtractFaceWidget::Start;

  Self->EventCallbackCommand->SetAbortFlag(1);
  Self->EndInteraction();
  Self->InvokeEvent(vtkCommand::EndInteractionEvent, NULL);
  Self->Interactor->Render();
}
//----------------------------------------------------------------------------------------------------
