/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxCreateElementSetWidgetFEMesh.cxx,v $
Language:  C++
Date:      $Date: 2008/08/14 05:01:51 $
Version:   $Revision: 1.30 $

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
#include "vtkMimxCreateElementSetWidgetFEMesh.h"

#include "vtkActor.h"
#include "vtkAssemblyPath.h"
#include "vtkCallbackCommand.h"
#include "vtkCamera.h"
#include "vtkCell.h"
#include "vtkCellData.h"
#include "vtkCellLocator.h"
#include "vtkCleanPolyData.h"
#include "vtkCommand.h"
#include "vtkDataSet.h"
#include "vtkDataSetMapper.h"
#include "vtkExtractSelectedFrustum.h"
#include "vtkExtractSelection.h"
#include "vtkExtractSelectedIds.h"
#include "vtkExtractSelectedPolyDataIds.h"
#include "vtkEvent.h"
#include "vtkGarbageCollector.h"
#include "vtkGeometryFilter.h"
#include "vtkHandleWidget.h"
#include "vtkIdFilter.h"
#include "vtkIdList.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInteractorObserver.h"
#include "vtkInteractorStyleRubberBandPick.h"
#include "vtkLookupTable.h"
#include "vtkObjectFactory.h"
#include "vtkPointLocator.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRenderedAreaPicker.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkRendererCollection.h"
#include "vtkRenderWindow.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyDataSource.h"
#include "vtkSelection.h"
#if ( (VTK_MAJOR_VERSION >= 6) || ( VTK_MAJOR_VERSION == 5 && VTK_MINOR_VERSION >= 4 ) )
#include "vtkSelectionNode.h"
#endif
#include "vtkVisibleCellSelector.h"
#include "vtkWidgetCallbackMapper.h" 
#include "vtkWidgetEvent.h"
#include "vtkWidgetEventTranslator.h"
#include "vtkCellPicker.h"
#include "vtkGeometryFilter.h"
#include "vtkMimxUnstructuredToStructuredGrid.h"
#include "vtkMimxExtractStructuredGridFace.h"
#include "vtkStructuredGridGeometryFilter.h"
#include "vtkUnstructuredGrid.h"
#include "vtkExtractCells.h"
#include "vtkPointData.h"
#include "vtkCellType.h"
#include "vtkUnstructuredGridWriter.h"
#include "vtkPolyDataWriter.h"
#include "vtkStructuredGridWriter.h"
#include "vtkMergeCells.h"

vtkCxxRevisionMacro(vtkMimxCreateElementSetWidgetFEMesh, "$Revision: 1.30 $");
vtkStandardNewMacro(vtkMimxCreateElementSetWidgetFEMesh);

//----------------------------------------------------------------------
vtkMimxCreateElementSetWidgetFEMesh::vtkMimxCreateElementSetWidgetFEMesh()
{
  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::LeftButtonPressEvent,
    vtkEvent::ControlModifier, 0, 1, NULL,
    vtkMimxCreateElementSetWidgetFEMesh::CrtlLeftMouseButtonDown,
    this, vtkMimxCreateElementSetWidgetFEMesh::CrtlLeftButtonDownCallback);

  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::MouseMoveEvent,
    vtkEvent::ControlModifier, 0, 1, NULL,
    vtkMimxCreateElementSetWidgetFEMesh::CrtlLeftMouseButtonMove,
    this, vtkMimxCreateElementSetWidgetFEMesh::CrtlMouseMoveCallback);

  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::MouseMoveEvent,
    vtkEvent::ShiftModifier, 0, 1, NULL,
    vtkMimxCreateElementSetWidgetFEMesh::ShiftMouseMove,
    this, vtkMimxCreateElementSetWidgetFEMesh::ShiftMouseMoveCallback);

  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::LeftButtonReleaseEvent,
    vtkEvent::ControlModifier, 0, 1, NULL,
    vtkMimxCreateElementSetWidgetFEMesh::CrtlLeftMouseButtonUp,
    this, vtkMimxCreateElementSetWidgetFEMesh::CrtlLeftButtonUpCallback);

  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                          vtkMimxCreateElementSetWidgetFEMesh::LeftMouseButtonUp,
                                          this, vtkMimxCreateElementSetWidgetFEMesh::LeftButtonUpCallback);

  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
                                          vtkMimxCreateElementSetWidgetFEMesh::LeftMouseButtonDown,
                                          this, vtkMimxCreateElementSetWidgetFEMesh::LeftButtonDownCallback);

  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::LeftButtonPressEvent,
    vtkEvent::ShiftModifier, 0, 1, NULL,
    vtkMimxCreateElementSetWidgetFEMesh::ShiftLeftMouseButtonDown,
    this, vtkMimxCreateElementSetWidgetFEMesh::ShiftLeftButtonDownCallback);

  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::LeftButtonReleaseEvent,
    vtkEvent::ShiftModifier, 0, 1, NULL,
    vtkMimxCreateElementSetWidgetFEMesh::ShiftLeftMouseButtonUp,
    this, vtkMimxCreateElementSetWidgetFEMesh::ShiftLeftButtonUpCallback);

  this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonReleaseEvent,
                                          vtkMimxCreateElementSetWidgetFEMesh::RightMouseButtonUp,
                                          this, vtkMimxCreateElementSetWidgetFEMesh::RightButtonUpCallback);

  this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonPressEvent,
                                          vtkMimxCreateElementSetWidgetFEMesh::RightMouseButtonDown,
                                          this, vtkMimxCreateElementSetWidgetFEMesh::RightButtonDownCallback);

  this->CallbackMapper->SetCallbackMethod(vtkCommand::MouseMoveEvent,
                                          vtkMimxCreateElementSetWidgetFEMesh::MouseMove,
                                          this, vtkMimxCreateElementSetWidgetFEMesh::MouseMoveCallback);

  this->RubberBandStyle =  vtkInteractorStyleRubberBandPick::New();
  this->AreaPicker = vtkRenderedAreaPicker::New();
  //this->Input = vtkUnstructuredGrid::New();
  this->Input = vtkUnstructuredGrid::New();
  this->InputMapper = vtkDataSetMapper::New();
  this->InputMapper->SetInput(this->Input);
  this->InputActor = vtkActor::New();
  this->InputActor->SetMapper(this->InputMapper);
  this->WidgetEvent = vtkMimxCreateElementSetWidgetFEMesh::Start;
  this->ExtractedCellIds = vtkIdList::New();
  this->SelectedPointIds = vtkIdList::New();
  this->DeleteCellIds = vtkIdList::New();
  this->PickX0 = -1;      this->PickY0 = -1;      this->PickX1 = -1;      this->PickY1 = -1;
  this->CellSelectionState = 1;
  this->BooleanState = 0;
  this->EditMeshState = 0;
  this->DimensionState = 0;
  //vtkDataSetMapper *mapper = vtkDataSetMapper::New();
  //mapper->SetInput(this->Input);
  //this->ExtractedActor = vtkActor::New() ;
  //this->ExtractedActor->SetMapper(mapper);
  //mapper->Delete();
  this->PointLocator = NULL;
  this->LocatorPoints = NULL;
  this->InputLocator = NULL;
  this->InputPoints = NULL;
  this->PickStatus = 0;
  this->SurfaceFilter = vtkGeometryFilter::New();
  this->SurfaceMapper = vtkPolyDataMapper::New();
  this->SurfaceActor = vtkActor::New();
  this->SurfaceActor->SetMapper(this->SurfaceMapper);

  this->ExtractedCells = vtkExtractCells::New();
  ExtractedCells->SetInput(this->Input);
  ExtractedCells->SetCellList(this->ExtractedCellIds);
  this->ExtractedGrid = ExtractedCells->GetOutput();
  this->ExtractedMapper = vtkDataSetMapper::New();
  this->ExtractedMapper->SetInput(this->ExtractedGrid);
  this->ExtractedActor = vtkActor::New();
  this->ExtractedActor->SetMapper(ExtractedMapper);
  this->ExtractedSurfaceFilter = vtkGeometryFilter::New();
  this->ExtractedSurfaceMapper = vtkPolyDataMapper::New();
  this->ExtractedSurfaceActor = vtkActor::New();
  this->ExtractedSurfaceFilter->SetInput(this->ExtractedGrid);
  this->ExtractedSurfaceMapper->SetInput(this->ExtractedSurfaceFilter->GetOutput());
  this->ExtractedSurfaceActor->SetMapper(this->ExtractedSurfaceMapper);
  this->ExtractedActor->GetProperty()->SetColor(0.0,1.0,0.0);
  this->ExtractedActor->GetProperty()->SetRepresentationToWireframe();
  //  this->ExtractedActor->GetProperty()->SetLineWidth(2.0);

  this->SelectedCellIds = vtkIdList::New();
  this->SelectedCells = vtkExtractCells::New();
  this->SelectedCells->SetInput(this->Input);
  this->SelectedCells->SetCellList(this->SelectedCellIds);
  this->SelectedGrid = this->SelectedCells->GetOutput();
  this->SelectedMapper = vtkDataSetMapper::New();
  this->SelectedMapper->SetInput(this->SelectedCells->GetOutput());
  this->SelectedActor = vtkActor::New();
  this->SelectedActor->SetMapper(this->SelectedMapper);
  this->SelectedActor->GetProperty()->SetColor(1.0,0.0,0.0);
  this->SelectedActor->GetProperty()->SetRepresentationToWireframe();
  this->LineWidth = 1.0;
  this->InputVolume = NULL;
  this->MeshType = VTK_HEXAHEDRON;
}

//----------------------------------------------------------------------
vtkMimxCreateElementSetWidgetFEMesh::~vtkMimxCreateElementSetWidgetFEMesh()
{
  this->RubberBandStyle->Delete();
  this->AreaPicker->Delete();

  if(this->ExtractedCellIds)
    this->ExtractedCellIds->Delete();

  if (this->SelectedPointIds)
    {
    this->SelectedPointIds->Delete();
    }
  this->DeleteCellIds->Delete();
  if(this->ExtractedActor)
    {
    this->ExtractedActor->Delete();
    }
  //if(this->ExtractedActor)
  //      this->ExtractedActor->Delete();
  if(this->PointLocator)
    this->PointLocator->Delete();
  if(this->LocatorPoints)
    this->LocatorPoints->Delete();  
  if(this->InputLocator)
    this->InputLocator->Delete();
  if(this->InputPoints)
    this->InputPoints->Delete();
  this->SurfaceFilter->Delete();
  //this->SurfaceActor->Delete();
  //this->SurfaceMapper->Delete();
  this->ExtractedCells->Delete();
  this->Input->Delete();
  this->InputMapper->Delete();
  this->InputActor->Delete();
  this->ExtractedSurfaceFilter->Delete();
  this->ExtractedSurfaceMapper->Delete();
  this->ExtractedSurfaceActor->Delete();
  this->ExtractedMapper->Delete();

  this->SelectedCellIds->Delete();
  this->SelectedCells->Delete();
  this->SelectedMapper->Delete();
  this->SelectedActor->Delete();
}

//----------------------------------------------------------------------
void vtkMimxCreateElementSetWidgetFEMesh::SetEnabled(int enabling)
{  
  if ( enabling ) //----------------
    {
    vtkDebugMacro(<<"Enabling widget");

    if ( this->Enabled ) //already enabled, just return
      {
      return;
      }

    if ( ! this->Interactor )
      {
      vtkErrorMacro(<<"The interactor must be set prior to enabling the widget");
      return;
      }
    //if(!this->Input)
    //{
    //      vtkErrorMacro(<<"Input Should be Set");
    //      return;
    //}

    int X=this->Interactor->GetEventPosition()[0];
    int Y=this->Interactor->GetEventPosition()[1];

    if ( ! this->CurrentRenderer )
      {
      this->SetCurrentRenderer(this->Interactor->FindPokedRenderer(X,Y));

      if (this->CurrentRenderer == NULL)
        {
        return;
        }

      if(this->InputActor)
        {
        this->CurrentRenderer->AddActor(this->InputActor);
        }
      if (this->SelectedActor)
        {
        this->CurrentRenderer->AddActor(this->SelectedActor);
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

    if ( !this->Interactor )
      {
      vtkErrorMacro(<<"The interactor must be set prior to enabling/disabling widget");
      return;
      }

    this->Interactor->SetInteractorStyle(this->RubberBandStyle);
    //      this->RubberBandStyle->SetEnabled(1);

    this->Interactor->SetPicker(this->AreaPicker);
    }

  else //disabling------------------
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

    //      this->RubberBandStyle->OnChar();
    if (this->InputActor)
      {
      this->CurrentRenderer->RemoveActor(this->InputActor);
      }
    if(this->ExtractedActor)
      {
      this->CurrentRenderer->RemoveActor(this->ExtractedActor);
      }
    if(this->SelectedActor)
      this->CurrentRenderer->RemoveActor(this->SelectedActor);
    //if (this->ExtractedActor)
    //{
    //      this->CurrentRenderer->RemoveActor(this->ExtractedActor);
    //}
    //this->CurrentRenderer->RemoveActor(this->ExtractedActor);
    if(this->SurfaceActor)
      this->CurrentRenderer->RemoveActor(this->SurfaceActor);
    if(this->ExtractedSurfaceActor)
      this->CurrentRenderer->RemoveActor(this->ExtractedSurfaceActor);
    this->SelectedCellIds->Initialize();
    this->ExtractedCellIds->Initialize();
    this->SelectedPointIds->Initialize();
    this->DeleteCellIds->Initialize();
    this->InvokeEvent(vtkCommand::DisableEvent,NULL);
    this->SetCurrentRenderer(NULL);
    }

  // Should only render if there is no parent
  if ( this->Interactor && !this->Parent )
    {
    this->Interactor->Render();
    }
}
//--------------------------------------------------------------------------------------
void vtkMimxCreateElementSetWidgetFEMesh::CrtlLeftButtonDownCallback(vtkAbstractWidget *w)
{
  vtkMimxCreateElementSetWidgetFEMesh *self = reinterpret_cast<vtkMimxCreateElementSetWidgetFEMesh*>(w);
  //if(self->CellSelectionState == vtkMimxCreateElementSetWidgetFEMesh::SelectMultipleCells)      return;
  if(self->CellSelectionState == vtkMimxCreateElementSetWidgetFEMesh::SelectIndividualCell
     || self->CellSelectionState == vtkMimxCreateElementSetWidgetFEMesh::SelectMultipleCells)        
    {
    self->WidgetEvent = vtkMimxCreateElementSetWidgetFEMesh::CrtlLeftMouseButtonDown;
    self->EventCallbackCommand->SetAbortFlag(1);
    self->StartInteraction();
    self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
    self->Interactor->Render();
    return;
    }
  self->PickStatus = 1;
  if(self->RubberBandStyle->GetEnabled())
    {
    self->RubberBandStyle->SetEnabled(0);
    self->RubberBandStyle->OnChar();
    }
  self->RubberBandStyle->SetEnabled(1);
  int *size;
  size = self->Interactor->GetRenderWindow()->GetSize();
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];
  //      cout <<X<<"  "<<Y<<endl;
  self->PickX0 = X;
  self->PickY0 = Y;
  // Okay, make sure that the pick is in the current renderer
  if ( !self->CurrentRenderer || !self->CurrentRenderer->IsInViewport(X, Y) )
    {
    self->WidgetEvent = vtkMimxCreateElementSetWidgetFEMesh::Outside;
    return;
    }

  self->WidgetEvent = vtkMimxCreateElementSetWidgetFEMesh::CrtlLeftMouseButtonDown;
  if(self->Input->GetCellData()->GetArray("Mimx_Scalars"))
    {
    self->Input->GetCellData()->RemoveArray("Mimx_Scalars");
    self->Input->Modified();
    }
  self->RubberBandStyle->GetInteractor()->SetKeyCode('r');
  self->RubberBandStyle->OnChar();
  self->RubberBandStyle->OnLeftButtonDown();
  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  self->Interactor->Render();
}
//----------------------------------------------------------------------
void vtkMimxCreateElementSetWidgetFEMesh::CrtlLeftButtonUpCallback(vtkAbstractWidget *w)
{
  vtkMimxCreateElementSetWidgetFEMesh *self = reinterpret_cast<vtkMimxCreateElementSetWidgetFEMesh*>(w);
  //if(self->CellSelectionState == vtkMimxCreateElementSetWidgetFEMesh::SelectMultipleCells)      return;

  int *size;
  size = self->Interactor->GetRenderWindow()->GetSize();

  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // cout <<X<<"  "<<Y<<endl;

  self->PickX1 = X;
  self->PickY1 = Y;

  if ( self->WidgetEvent == vtkMimxCreateElementSetWidgetFEMesh::Outside ||
       self->WidgetEvent == vtkMimxCreateElementSetWidgetFEMesh::Start )
    {
    return;
    }

  if(self->ExtractedActor)
    self->CurrentRenderer->RemoveActor(self->ExtractedActor);
  //if(self->WidgetEvent != vtkMimxCreateElementSetWidgetFEMesh::CrtlLeftMouseButtonDown)
  //{
  // int x = 0;
  //}
  if(self->WidgetEvent == vtkMimxCreateElementSetWidgetFEMesh::CrtlLeftMouseButtonDown)
    {
    self->WidgetEvent = vtkMimxCreateElementSetWidgetFEMesh::Start;

    if(self->CellSelectionState == vtkMimxCreateElementSetWidgetFEMesh::SelectVisibleCellsOnSurface)
      {
      vtkMimxCreateElementSetWidgetFEMesh::SelectVisibleCellsOnSurfaceFunction(self);
      }
    if(self->CellSelectionState == vtkMimxCreateElementSetWidgetFEMesh::SelectCellsOnSurface)
      {
      vtkMimxCreateElementSetWidgetFEMesh::SelectCellsOnSurfaceFunction(self);
      }
    if(self->CellSelectionState == vtkMimxCreateElementSetWidgetFEMesh::SelectCellsThrough)
      {
      vtkMimxCreateElementSetWidgetFEMesh::SelectCellsThroughFunction(self);
      }
    if (self->CellSelectionState == vtkMimxCreateElementSetWidgetFEMesh::SelectIndividualCell)
      {
      if(self->DimensionState)
        {
        vtkMimxCreateElementSetWidgetFEMesh::ExtractElementsBelongingToAFaceSurface(self);
        }
      else
        {
        vtkMimxCreateElementSetWidgetFEMesh::ExtractElementsBelongingToAFace(self);
        }
      return;
      }
    if (self->CellSelectionState == vtkMimxCreateElementSetWidgetFEMesh::SelectMultipleCells)
      {
      vtkMimxCreateElementSetWidgetFEMesh::SelectIndividualCellFunction(self);
      return;
      }
    }
  self->RubberBandStyle->GetInteractor()->SetKeyCode('r');
  self->RubberBandStyle->OnChar();
  self->RubberBandStyle->OnLeftButtonUp();
  self->RubberBandStyle->SetEnabled(0);
  self->EventCallbackCommand->SetAbortFlag(1);
  self->EndInteraction();
  self->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
  self->PickStatus = 0;
  self->Interactor->Render();
}
//----------------------------------------------------------------------
void vtkMimxCreateElementSetWidgetFEMesh::CrtlMouseMoveCallback(vtkAbstractWidget *w)
{
  vtkMimxCreateElementSetWidgetFEMesh *self = reinterpret_cast<vtkMimxCreateElementSetWidgetFEMesh*>(w);
  //if(self->CellSelectionState == vtkMimxCreateElementSetWidgetFEMesh::SelectMultipleCells)      return;

  if(self->CellSelectionState != vtkMimxCreateElementSetWidgetFEMesh::SelectIndividualCell
     && self->CellSelectionState != vtkMimxCreateElementSetWidgetFEMesh::SelectMultipleCells)
    {
    self->RubberBandStyle->OnMouseMove();
    }
  else{
  self->SetCellSelectionState(self->CellSelectionState);
  if(self->CellSelectionState == vtkMimxCreateElementSetWidgetFEMesh::SelectIndividualCell)
    {
    self->SelectIndividualSurfaceCellFunction(self);
    }
  //else{
  // self->SelectIndividualCellFunction(self);
  //}
  self->Interactor->Render();
  }
}
//---------------------------------------------------------------------
void vtkMimxCreateElementSetWidgetFEMesh::ComputeSelectedPointIds(
  vtkDataSet *SelectedUGrid, vtkMimxCreateElementSetWidgetFEMesh *self)
{
  int i;
  vtkPoints *polypoints ;
  if(SelectedUGrid->GetDataObjectType() == VTK_POLY_DATA)
    polypoints = vtkPolyData::SafeDownCast(SelectedUGrid)->GetPoints();
  else
    polypoints = vtkUnstructuredGrid::SafeDownCast(SelectedUGrid)->GetPoints();

  if(self->SelectedPointIds)
    {
    self->SelectedPointIds->Initialize();
    }
  else{
  self->SelectedPointIds = vtkIdList::New();
  }

  for (i=0; i<polypoints->GetNumberOfPoints(); i++)
    {
    int location = PointLocator->IsInsertedPoint(polypoints->GetPoint(i));
    if(location == -1)
      {
      vtkErrorMacro("Point sets do not match");
      self->SelectedPointIds->Initialize();
      return;
      }
    else
      {
      self->SelectedPointIds->InsertNextId(location);
      }
    }
}
//----------------------------------------------------------------------
void vtkMimxCreateElementSetWidgetFEMesh::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);  
}
//-----------------------------------------------------------------------
void vtkMimxCreateElementSetWidgetFEMesh::SelectVisibleCellsOnSurfaceFunction(
  vtkMimxCreateElementSetWidgetFEMesh *self)
{
  vtkGeometryFilter *fil = vtkGeometryFilter::New();
  if(self->EditMeshState == vtkMimxCreateElementSetWidgetFEMesh::FullMesh)
    fil->SetInput(self->Input);
  else
    fil->SetInput(self->SelectedCells->GetOutput());

  fil->Update();

  vtkPolyDataMapper *filmapper = vtkPolyDataMapper::New();
  vtkActor *filactor = vtkActor::New();
  filmapper->SetInput(fil->GetOutput());
  filactor->SetMapper(filmapper);
  filactor->PickableOn();
  self->CurrentRenderer->AddActor(filactor);
  self->CurrentRenderer->Render();

  vtkVisibleCellSelector *select = vtkVisibleCellSelector::New();
  select->SetRenderer(self->CurrentRenderer);

  double x0 = self->PickX0;
  double y0 = self->PickY0;
  double x1 = self->PickX1;
  double y1 = self->PickY1;

  select->SetRenderPasses(0,1,0,1,1);
  select->SetArea(static_cast<int>(x0),static_cast<int>(y1),static_cast<int>(x1),
                  static_cast<int>(y0));
  select->Select();

  vtkSelection *res = vtkSelection::New();
  select->GetSelectedIds(res);
#if ( (VTK_MAJOR_VERSION >= 6) || ( VTK_MAJOR_VERSION == 5 && VTK_MINOR_VERSION >= 4 ) )
  vtkSelectionNode *cellidssel = res->GetNode(0);
#else
  vtkSelection *cellidssel = res->GetChild(0);
#endif
  vtkExtractSelectedPolyDataIds *extr = vtkExtractSelectedPolyDataIds::New();
  if (cellidssel)
    {
    extr->SetInput(0, fil->GetOutput());
#if ( (VTK_MAJOR_VERSION >= 6) || ( VTK_MAJOR_VERSION == 5 && VTK_MINOR_VERSION >= 4 ) )
    extr->SetInput(1, res);
#else
    extr->SetInput(1, cellidssel);
#endif
    extr->Update();
    }

  if (extr->GetOutput()->GetNumberOfCells() < 1)
    {
    select->Delete();
    res->Delete();
    extr->Delete();
    filmapper->Delete();
    fil->Delete();
    self->CurrentRenderer->RemoveActor(filactor);
    filactor->Delete();
    return;
    }
  //vtkMergeCells *mergecells = vtkMergeCells::New();
  //vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();

  //mergecells->SetUnstructuredGrid(ugrid);
  //mergecells->SetTotalNumberOfDataSets(1);
  //mergecells->SetTotalNumberOfCells(extr->GetOutput()->GetNumberOfCells());
  //mergecells->SetTotalNumberOfPoints(extr->GetOutput()->GetNumberOfPoints());
  //mergecells->MergeDuplicatePointsOn();
  //mergecells->MergeDataSet(extr->GetOutput());
  //mergecells->Finish();
  //mergecells->Delete();
  //self->ComputeExtractedCellIdsSurface(self, ugrid);
  self->ComputeExtractedCellIdsSurface(self, extr->GetOutput());
  select->Delete();
  res->Delete();
  extr->Delete();
  filmapper->Delete();
  fil->Delete();
  self->CurrentRenderer->RemoveActor(filactor);
  filactor->Delete();
  //ugrid->Delete();
  //      vtkSelection *selection = vtkSelection::New();
  //      vtkExtractSelection *ext = vtkExtractSelection::New();
  //      ext->SetInput(0, self->Input);
  //      ext->SetInput(1, selection);
  //
  //      vtkIdTypeArray *globalids = vtkIdTypeArray::New();
  //      globalids->SetNumberOfComponents(1);
  //      globalids->SetName("GIds");
  ////    globalids->SetNumberOfTuples(self->Input->GetNumberOfCells());
  //      for (i=0; i<self->Input->GetNumberOfCells(); i++)
  //      {
  //              globalids->InsertNextValue(i);
  //      }
  //      self->Input->GetCellData()->AddArray(globalids);
  //      self->Input->GetCellData()->SetGlobalIds(globalids);
  //
  //      selection->Clear();
  //      selection->GetProperties()->Set(
  //              vtkSelection::CONTENT_TYPE(), vtkSelection::GLOBALIDS);
  //      vtkIdTypeArray *cellIds = vtkIdTypeArray::New();
  //      cellIds->SetNumberOfComponents(1);
  //      cellIds->SetNumberOfTuples(self->ExtractedCellIds->GetNumberOfIds());
  //      for (i=0; i<self->ExtractedCellIds->GetNumberOfIds(); i++)
  //      {
  //              cellIds->SetTuple1(i, self->ExtractedCellIds->GetId(i));
  //      }
  //      selection->SetSelectionList(cellIds);
  //      cellIds->Delete();
  //      
  //      ext->Update();
  //
  //      self->Input->GetCellData()->RemoveArray("GIds");
  //      globalids->Delete();
  //      if(self->ExtractedActor)
  //      {
  //              self->CurrentRenderer->RemoveActor(self->ExtractedActor);
  //              self->ExtractedActor->Delete();
  //      }
  //      self->ExtractedActor = vtkActor::New();
  //      vtkDataSetMapper *mapper = vtkDataSetMapper::New();
  //      mapper->SetInput(ext->GetOutput());
  //      self->ExtractedActor->SetMapper(mapper);
  //      mapper->Delete();
  //      if(self->InputActor)
  //              self->CurrentRenderer->AddActor(self->InputActor);
  //      self->CurrentRenderer->AddActor(self->ExtractedActor);
  //      self->ExtractedActor->GetProperty()->SetColor(1.0, 0.0,0.0);
  //      self->ExtractedActor->GetProperty()->SetRepresentationToWireframe();
  //      selection->Delete();
  //      ext->Delete();
}
//-----------------------------------------------------------------------------
void vtkMimxCreateElementSetWidgetFEMesh::SelectCellsOnSurfaceFunction(
  vtkMimxCreateElementSetWidgetFEMesh *self)
{
  vtkGeometryFilter *fil = vtkGeometryFilter::New();
  if(self->EditMeshState == vtkMimxCreateElementSetWidgetFEMesh::FullMesh)
    fil->SetInput(self->Input);
  else
    fil->SetInput(self->SelectedCells->GetOutput());

  fil->Update();
  //vtkPolyDataWriter *writerp = vtkPolyDataWriter::New();
  //writerp->SetInput(fil->GetOutput());
  //writerp->SetFileName("Selected.vtk");
  //writerp->Write();
  //writerp->Delete();

  vtkPolyDataMapper *filmapper = vtkPolyDataMapper::New();
  vtkActor *filactor = vtkActor::New();
  filmapper->SetInput(fil->GetOutput());
  filactor->SetMapper(filmapper);
  filactor->PickableOn();
  self->CurrentRenderer->AddActor(filactor);

  if(self->EditMeshState == vtkMimxCreateElementSetWidgetFEMesh::FullMesh)
    {       if(self->InputActor)
      self->CurrentRenderer->RemoveActor(self->InputActor);}
  else
    {
    self->CurrentRenderer->RemoveActor(self->ExtractedActor);
    }

  self->CurrentRenderer->Render();

  double x0 = self->PickX0;
  double y0 = self->PickY0;
  double x1 = self->PickX1;
  double y1 = self->PickY1;
  self->AreaPicker->AreaPick(static_cast<int>(x0), static_cast<int>(y0), 
                             static_cast<int>(x1), static_cast<int>(y1), self->CurrentRenderer);
  vtkExtractSelectedFrustum *Extract = vtkExtractSelectedFrustum::New();
  Extract->SetInput(fil->GetOutput());
  //      Extract->PassThroughOff();
  Extract->SetFrustum(self->AreaPicker->GetFrustum());
  Extract->Update();
  vtkUnstructuredGrid *exugrid = vtkUnstructuredGrid::SafeDownCast(
    Extract->GetOutput());  
  if (exugrid->GetNumberOfCells() < 1)
    {
    Extract->Delete();
    filmapper->Delete();
    fil->Delete();
    self->CurrentRenderer->RemoveActor(filactor);
    filactor->Delete();
    if(self->EditMeshState == vtkMimxCreateElementSetWidgetFEMesh::FullMesh)
      {       if(self->InputActor)
        self->CurrentRenderer->AddActor(self->InputActor);}
    else
      {
      self->CurrentRenderer->AddActor(self->ExtractedActor);
      }
    self->CurrentRenderer->Render();
    return;
    }
  vtkMergeCells *mergecells = vtkMergeCells::New();
  vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();

  mergecells->SetUnstructuredGrid(ugrid);
  mergecells->SetTotalNumberOfDataSets(1);
  mergecells->SetTotalNumberOfCells(exugrid->GetNumberOfCells());
  mergecells->SetTotalNumberOfPoints(exugrid->GetNumberOfPoints());
  mergecells->MergeDuplicatePointsOn();
  mergecells->MergeDataSet(exugrid);
  mergecells->Finish();
  mergecells->Delete();

  //vtkUnstructuredGridWriter *writer = vtkUnstructuredGridWriter::New();
  //writer->SetInput(ugrid);
  //writer->SetFileName("Surface.vtk");
  //writer->Write();
  //writer->Delete();
  self->ComputeExtractedCellIdsSurface(self, ugrid);
  ugrid->Delete();

  Extract->Delete();
  filmapper->Delete();
  fil->Delete();
  self->CurrentRenderer->RemoveActor(filactor);
  filactor->Delete();
  if(self->EditMeshState == vtkMimxCreateElementSetWidgetFEMesh::FullMesh)
    {
    if(self->InputActor)
      self->CurrentRenderer->AddActor(self->InputActor);}
  else
    {
    self->CurrentRenderer->AddActor(self->ExtractedActor);
    }
  self->CurrentRenderer->Render();
  //vtkSelection *selection = vtkSelection::New();
  //vtkExtractSelection *ext = vtkExtractSelection::New();
  //ext->SetInput(0, self->Input);
  //ext->SetInput(1, selection);

  //vtkIdTypeArray *globalids = vtkIdTypeArray::New();
  //globalids->SetNumberOfComponents(1);
  //globalids->SetName("GIds");
  ////    globalids->SetNumberOfTuples(self->Input->GetNumberOfCells());
  //for (i=0; i<self->Input->GetNumberOfCells(); i++)
  //{
  //      globalids->InsertNextValue(i);
  //}
  //self->Input->GetCellData()->AddArray(globalids);
  //self->Input->GetCellData()->SetGlobalIds(globalids);

  //selection->Clear();
  //selection->GetProperties()->Set(
  //      vtkSelection::CONTENT_TYPE(), vtkSelection::GLOBALIDS);
  //vtkIdTypeArray *cellIds = vtkIdTypeArray::New();
  //cellIds->SetNumberOfComponents(1);
  //cellIds->SetNumberOfTuples(self->ExtractedCellIds->GetNumberOfIds());
  //for (i=0; i<self->ExtractedCellIds->GetNumberOfIds(); i++)
  //{
  //      cellIds->SetTuple1(i, self->ExtractedCellIds->GetId(i));
  //}
  //selection->SetSelectionList(cellIds);
  //cellIds->Delete();

  //ext->Update();

  //self->Input->GetCellData()->RemoveArray("GIds");
  //globalids->Delete();
  //if(self->ExtractedActor)
  //{
  //      self->CurrentRenderer->RemoveActor(self->ExtractedActor);
  //      self->ExtractedActor->Delete();
  //}
  //self->ExtractedActor = vtkActor::New();
  //vtkDataSetMapper *mapper = vtkDataSetMapper::New();
  //mapper->SetInput(ext->GetOutput());
  //self->ExtractedActor->SetMapper(mapper);
  //mapper->Delete();
  //if(self->InputActor)
  //      self->CurrentRenderer->AddActor(self->InputActor);
  //self->CurrentRenderer->AddActor(self->ExtractedActor);
  //self->ExtractedActor->GetProperty()->SetColor(1.0, 0.0,0.0);
  //self->ExtractedActor->GetProperty()->SetLineWidth(2);
  //self->ExtractedActor->GetProperty()->SetRepresentationToWireframe();
  //selection->Delete();
  //ext->Delete();
}
//-----------------------------------------------------------------------------
void vtkMimxCreateElementSetWidgetFEMesh::SelectCellsThroughFunction(
  vtkMimxCreateElementSetWidgetFEMesh *self)
{
  double x0 = self->PickX0;
  double y0 = self->PickY0;
  double x1 = self->PickX1;
  double y1 = self->PickY1;
  self->AreaPicker->AreaPick(static_cast<int>(x0), static_cast<int>(y0), 
                             static_cast<int>(x1), static_cast<int>(y1), self->CurrentRenderer);
  vtkExtractSelectedFrustum *Extract = vtkExtractSelectedFrustum::New();
  if(self->EditMeshState == vtkMimxCreateElementSetWidgetFEMesh::FullMesh)
    {       
    Extract->SetInput(self->Input);
    }
  else
    {
    Extract->SetInput(self->SelectedCells->GetOutput());
    }

  Extract->SetFrustum(self->AreaPicker->GetFrustum());
  Extract->Update();
  vtkUnstructuredGrid *exugrid = vtkUnstructuredGrid::SafeDownCast(
    Extract->GetOutput());  
  if(exugrid->GetNumberOfCells() <1)
    {
    Extract->Delete();
    return;
    }
  self->ComputeExtractedCellIds(self, exugrid);
  Extract->Delete();
}
//-----------------------------------------------------------------------------
void vtkMimxCreateElementSetWidgetFEMesh::SelectIndividualCellFunction(
  vtkMimxCreateElementSetWidgetFEMesh *self)
{
  //      int *size;
  //      size = self->Interactor->GetRenderWindow()->GetSize();
  //      int X = self->Interactor->GetEventPosition()[0];
  //      int Y = self->Interactor->GetEventPosition()[1];
  //
  //      // Okay, make sure that the pick is in the current renderer
  //      if ( !self->CurrentRenderer || !self->CurrentRenderer->IsInViewport(X, Y) )
  //      {
  //              self->WidgetEvent = vtkMimxCreateElementSetWidgetFEMesh::Outside;
  //              return;
  //      }
  //
  ////    self->WidgetEvent = vtkMimxCreateElementSetWidgetFEMesh::ShiftMouseMove;
  //
  //      int i;
  //      vtkAssemblyPath *path;
  //      vtkCellPicker *CellPicker = vtkCellPicker::New();
  //      CellPicker->SetTolerance(0.01);
  //      vtkPolyData *Surface;
  //      if(self->EditMeshState == vtkMimxCreateElementSetWidgetFEMesh::FullMesh)
  //      {
  //              CellPicker->AddPickList(self->SurfaceActor);
  //              Surface = self->SurfaceFilter->GetOutput();
  //      }
  //      else
  //      {
  //              CellPicker->AddPickList(self->ExtractedSurfaceActor);
  //              Surface = self->ExtractedSurfaceFilter->GetOutput();
  //      }
  //      Surface->GetCellData()->Initialize();
  //      CellPicker->PickFromListOn();
  //      CellPicker->Pick(X,Y,0.0,self->CurrentRenderer);
  //      path = CellPicker->GetPath();
  //      
  //      if ( path != NULL )
  //      {
  //              vtkIdType PickedCell = CellPicker->GetCellId();
  //              if(PickedCell != -1)
  //              {
  //                      vtkLookupTable *lut = vtkLookupTable::New();
  //                      lut->SetNumberOfColors(2);
  //                      lut->Build();
  //                      lut->SetTableValue(0, 1.0, 1.0, 1.0, 1.0);
  //                      lut->SetTableValue(1, 1.0, 0.0, 0.0, 1.0);
  //                      lut->SetTableRange(0,1);
  //                      self->SurfaceActor->GetMapper()->SetLookupTable(lut);
  //                      self->SurfaceActor->GetMapper()->SetScalarRange(0,1);
  //                      lut->Delete();
  //                      vtkIntArray *intarray = vtkIntArray::New();
  //                      intarray->SetNumberOfValues(Surface->GetNumberOfCells());
  //                      for (i=0; i<Surface->GetNumberOfCells(); i++)
  //                      {
  //                              intarray->SetValue(i, 0);
  //                      }
  //                      intarray->SetValue(PickedCell, 1);
  //                      intarray->SetName("Mimx_Scalars");
  //                      Surface->GetCellData()->SetScalars(intarray);
  //                      intarray->Delete();
  //                      Surface->Modified();
  //                      vtkPolyDataWriter *writer = vtkPolyDataWriter::New();
  //                      writer->SetInput(Surface);
  //                      writer->SetFileName("Surface.vtk");
  //                      writer->Write();
  //                      writer->Delete();
  //                      self->ExtractedSurfaceActor->GetProperty()->SetColor(1.0, 1.0, 1.0);
  //              }
  //      }
  //      self->Interactor->Render();
  //      CellPicker->Delete();

  int *size;
  size = self->Interactor->GetRenderWindow()->GetSize();
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // Okay, make sure that the pick is in the current renderer
  if ( !self->CurrentRenderer || !self->CurrentRenderer->IsInViewport(X, Y) )
    {
    self->WidgetEvent = vtkMimxCreateElementSetWidgetFEMesh::Outside;
    return;
    }

  //      self->WidgetEvent = vtkMimxCreateElementSetWidgetFEMesh::ShiftMouseMove;

  vtkAssemblyPath *path;
  vtkCellPicker *CellPicker = vtkCellPicker::New();
  CellPicker->SetTolerance(0.001);
  vtkUnstructuredGrid *Ugrid;
  if(self->EditMeshState == vtkMimxCreateElementSetWidgetFEMesh::FullMesh)
    {
    CellPicker->AddPickList(self->InputActor);
    Ugrid = self->Input;
    }
  else
    {
    CellPicker->AddPickList(self->SelectedActor);
    Ugrid = self->SelectedCells->GetOutput();
    }
  CellPicker->PickFromListOn();
  CellPicker->Pick(X,Y,0.0,self->CurrentRenderer);
  path = CellPicker->GetPath();

  if ( path != NULL )
    {
    vtkIdType PickedCell = CellPicker->GetCellId();
    if(PickedCell != -1)
      {
      self->ExtractedCellIds->Initialize();
      if(self->EditMeshState == vtkMimxCreateElementSetWidgetFEMesh::FullMesh)
        {
        self->ExtractedCellIds->InsertUniqueId(PickedCell);
        }
      else{
      vtkIntArray *intarray = vtkIntArray::SafeDownCast(
        self->SelectedCells->GetOutput()->GetCellData()->GetArray("mimx_Original_Cell_Ids"));
      if(intarray)
        self->ExtractedCellIds->InsertUniqueId(intarray->GetValue(PickedCell));
      }
      self->ExtractedCells->SetInput(self->Input);
      self->ExtractedCells->SetCellList(self->ExtractedCellIds);
      self->ExtractedCells->Modified();
      self->ExtractedCells->Update();
      self->ExtractedCells->GetOutput()->GetCellData()->RemoveArray("vtkOriginalCellIds");
      self->CurrentRenderer->AddActor(self->ExtractedActor);          
      }
    }
  self->Interactor->Render();
  CellPicker->Delete();
}
//--------------------------------------------------------------------------------------
void vtkMimxCreateElementSetWidgetFEMesh::ComputeExtractedCellIds(
  vtkMimxCreateElementSetWidgetFEMesh *Self, vtkDataSet *DataSet)
{
  //vtkUnstructuredGridWriter *writer = vtkUnstructuredGridWriter::New();
  //writer->SetInput(DataSet);
  //writer->SetFileName("Extract.vtk");
  //writer->Write();
  //writer->Delete();
  //Self->ExtractedCellIds->Initialize();
  //Self->DeleteCellIds->Initialize();
  // first compute the cells selected
  vtkIntArray *originalids = vtkIntArray::SafeDownCast(
    DataSet->GetCellData()->GetArray("mimx_Original_Cell_Ids"));
  Self->ExtractedCellIds->Initialize();   
  int i, j, CellNum;
  if(originalids)
    {
    for (i=0; i<originalids->GetNumberOfTuples(); i++)
      {
      if(Self->EditMeshState == vtkMimxCreateElementSetWidgetFEMesh::SelectedMesh
         && Self->SelectedCellIds->GetNumberOfIds())
        {
        int location = Self->SelectedCellIds->IsId(originalids->GetValue(i));
        if(location != -1)
          {
          Self->ExtractedCellIds->InsertUniqueId(originalids->GetValue(i));
          }
        }
      else{
      Self->ExtractedCellIds->InsertUniqueId(originalids->GetValue(i));
      }
      }       
    }
  else
    {
    for (i=0; i<DataSet->GetNumberOfCells(); i++)
      {
      Self->ComputeSelectedPointIds(DataSet, Self);
      vtkIdList *PtIds = DataSet->GetCell(i)->GetPointIds();
      vtkIdList *OrIds = vtkIdList::New();
      OrIds->SetNumberOfIds(PtIds->GetNumberOfIds());
      for (j=0; j<PtIds->GetNumberOfIds(); j++)
        {
        OrIds->SetId(j, Self->SelectedPointIds->GetId(PtIds->GetId(j)));
        }
      CellNum = Self->ComputeOriginalCellNumber(Self, OrIds);
      OrIds->Delete();
      if(CellNum == -1)
        {
        Self->ExtractedCellIds->Initialize();
        vtkErrorMacro("Cells chosen do not belong to the original set");
        return;
        }
      else
        {
        if(Self->EditMeshState == vtkMimxCreateElementSetWidgetFEMesh::SelectedMesh
           && Self->SelectedCellIds->GetNumberOfIds())
          {
          int location = Self->SelectedCellIds->IsId(CellNum);
          if(location != -1)
            {
            Self->ExtractedCellIds->InsertUniqueId(CellNum);
            }
          }
        else{
        Self->ExtractedCellIds->InsertUniqueId(CellNum);
        }
        }
      }
    }
  Self->ExtractedCells->SetInput(Self->Input);
  Self->ExtractedCells->SetCellList(Self->ExtractedCellIds);
  Self->ExtractedCells->Modified();
  Self->ExtractedCells->Update();
  Self->ExtractedCells->GetOutput()->GetCellData()->RemoveArray("vtkOriginalCellIds");
  Self->CurrentRenderer->AddActor(Self->ExtractedActor);
  //vtkUnstructuredGridWriter *writer = vtkUnstructuredGridWriter::New();
  //writer->SetInput(Self->ExtractedCells->GetOutput());
  //writer->SetFileName("ExtractUGrid.vtk");
  //writer->Write();
  //writer->Delete();
  // create a new selected element set based on computed selected cells
  //Self->ExtractedGrid->Initialize();
  //Self->ExtractedGrid->Allocate(DataSet->GetNumberOfCells(), DataSet->GetNumberOfCells());
  //vtkPointLocator *locator = vtkPointLocator::New();
  //vtkPoints *points = vtkPoints::New();
  //points->Allocate(DataSet->GetNumberOfPoints()*4);
  //locator->InitPointInsertion(points, Self->Input->GetPoints()->GetBounds());
  //vtkIdList *idlist;
  //double x[3];
  //vtkIdType tempid;
  //int numIds = Self->ExtractedCellIds->GetNumberOfIds();
  //Self->ExtractedGrid->Initialize();
  //Self->ExtractedGrid->Allocate(numIds, numIds);
  //for (i=0; i<numIds; i++)
  //{
  //      idlist = vtkIdList::New();
  //      idlist->DeepCopy(Self->Input->GetCell(Self->ExtractedCellIds->GetId(i))->GetPointIds());
  //      for(int j=0; j <8; j++)
  //      {
  //              Self->Input->GetPoints()->GetPoint(idlist->GetId(j),x);          
  //              locator->InsertUniquePoint(x,tempid);
  //              idlist->SetId(j,tempid);
  //      }
  //      Self->ExtractedGrid->InsertNextCell(12, idlist);
  //      idlist->Delete();
  //}

  //points->Squeeze();
  //Self->ExtractedGrid->SetPoints(points);
  //Self->ExtractedGrid->Squeeze();
  //points->Delete();
  //locator->Delete();
  //Self->ExtractedGrid->Modified();
  //Self->CurrentRenderer->AddActor(Self->ExtractedActor); 
  Self->Interactor->Render();

}
//---------------------------------------------------------------------------------------
void vtkMimxCreateElementSetWidgetFEMesh::ComputeExtractedCellIdsSurface(
  vtkMimxCreateElementSetWidgetFEMesh *Self, vtkDataSet *DataSet)
{
  //vtkUnstructuredGridWriter *writer = vtkUnstructuredGridWriter::New();
  //writer->SetInput(DataSet);
  //writer->SetFileName("Extract.vtk");
  //writer->Write();
  //writer->Delete();
  //Self->ExtractedCellIds->Initialize();
  //Self->DeleteCellIds->Initialize();
  // first compute the cells selected
  vtkIntArray *originalids = vtkIntArray::SafeDownCast(
    DataSet->GetCellData()->GetArray("mimx_Original_Cell_Ids"));

  int i, j, CellNum;

  Self->ExtractedCellIds->Initialize();   

  if(originalids && !Self->DimensionState)
    {
    for (i=0; i<DataSet->GetNumberOfCells(); i++)
      {
      CellNum = originalids->GetValue(i);
      if(Self->EditMeshState == vtkMimxCreateElementSetWidgetFEMesh::SelectedMesh
         && Self->SelectedCellIds->GetNumberOfIds())
        {
        int location = Self->SelectedCellIds->IsId(CellNum);
        if(location != -1)
          {
          Self->ExtractedCellIds->InsertUniqueId(CellNum);
          }
        }
      else{
      Self->ExtractedCellIds->InsertUniqueId(CellNum);
      }
      }

    }
  else
    {
    Self->ComputeSelectedPointIds(DataSet, Self);
    for (i=0; i<DataSet->GetNumberOfCells(); i++)
      {
      vtkIdList *PtIds = DataSet->GetCell(i)->GetPointIds();
      vtkIdList *OrIds = vtkIdList::New();
      OrIds->SetNumberOfIds(PtIds->GetNumberOfIds());
      for (j=0; j<PtIds->GetNumberOfIds(); j++)
        {
        OrIds->SetId(j, Self->SelectedPointIds->GetId(PtIds->GetId(j)));
        }
      if(Self->EditMeshState == vtkMimxCreateElementSetWidgetFEMesh::SelectedMesh)
        {
        CellNum = Self->ComputeOriginalCellNumberFromSelectedSurfaceMesh(Self,OrIds);
        }
      else
        {
        CellNum = Self->ComputeOriginalCellNumber(Self, OrIds);
        }
      OrIds->Delete();
      if(CellNum == -1)
        {
        Self->ExtractedCellIds->Initialize();
        vtkErrorMacro("Cells chosen do not belong to the original set");
        return;
        }
      else
        {
        if(Self->EditMeshState == vtkMimxCreateElementSetWidgetFEMesh::SelectedMesh
           && Self->SelectedCellIds->GetNumberOfIds())
          {
          int location = Self->SelectedCellIds->IsId(CellNum);
          if(location != -1)
            {
            Self->ExtractedCellIds->InsertUniqueId(CellNum);
            }
          }
        else{
        Self->ExtractedCellIds->InsertUniqueId(CellNum);
        }
        }
      }
    }
  Self->ExtractedCells->SetInput(Self->Input);
  Self->ExtractedCells->SetCellList(Self->ExtractedCellIds);
  Self->ExtractedCells->Modified();
  Self->ExtractedCells->Update();
  Self->ExtractedCells->GetOutput()->GetCellData()->RemoveArray("vtkOriginalCellIds");
  Self->CurrentRenderer->AddActor(Self->ExtractedActor);
  //writer = vtkUnstructuredGridWriter::New();
  //writer->SetInput(Self->ExtractedGrid);
  //writer->SetFileName("ExtractUGrid.vtk");
  //writer->Write();
  //writer->Delete();
  // create a new selected element set based on computed selected cells
  //Self->ExtractedGrid->Initialize();
  //Self->ExtractedGrid->Allocate(DataSet->GetNumberOfCells(), DataSet->GetNumberOfCells());
  //vtkPointLocator *locator = vtkPointLocator::New();
  //vtkPoints *points = vtkPoints::New();
  //points->Allocate(DataSet->GetNumberOfPoints()*4);
  //locator->InitPointInsertion(points, Self->Input->GetPoints()->GetBounds());
  //vtkIdList *idlist;
  //double x[3];
  //vtkIdType tempid;
  //int numIds = Self->ExtractedCellIds->GetNumberOfIds();
  //Self->ExtractedGrid->Initialize();
  //Self->ExtractedGrid->Allocate(numIds, numIds);
  //for (i=0; i<numIds; i++)
  //{
  //      idlist = vtkIdList::New();
  //      idlist->DeepCopy(Self->Input->GetCell(Self->ExtractedCellIds->GetId(i))->GetPointIds());
  //      for(int j=0; j <8; j++)
  //      {
  //              Self->Input->GetPoints()->GetPoint(idlist->GetId(j),x);          
  //              locator->InsertUniquePoint(x,tempid);
  //              idlist->SetId(j,tempid);
  //      }
  //      Self->ExtractedGrid->InsertNextCell(12, idlist);
  //      idlist->Delete();
  //}

  //points->Squeeze();
  //Self->ExtractedGrid->SetPoints(points);
  //Self->ExtractedGrid->Squeeze();
  //points->Delete();
  //locator->Delete();
  //Self->ExtractedGrid->Modified();
  //Self->CurrentRenderer->AddActor(Self->ExtractedActor); 
  Self->Interactor->Render();

}
//---------------------------------------------------------------------------------------
int vtkMimxCreateElementSetWidgetFEMesh::ComputeOriginalCellNumber(
  vtkMimxCreateElementSetWidgetFEMesh *self, vtkIdList *PtIds)
{
  int i,j, k;

  //      if ((PtIds->GetNumberOfIds() == 8) /*|| (PtIds->GetNumberOfIds() == 4)*/)

  int numPtIds = 0;
  switch (this->MeshType)
    {
    case VTK_HEXAHEDRON: numPtIds = 8; break;
    case VTK_TETRA: numPtIds = 4; break;
    case VTK_QUAD: numPtIds = 4; break;
    case VTK_TRIANGLE: numPtIds = 3; break;
    }

  //      if (PtIds->GetNumberOfIds() == numPtIds) 
  int surface = 0;
  if(this->MeshType == VTK_QUAD || this->MeshType == VTK_TRIANGLE)        surface = 1;
  if(surface || numPtIds == PtIds->GetNumberOfIds())
    {
    //if(PtIds->GetNumberOfIds() == 8)
    //{
    for (i=0; i<self->Input->GetNumberOfCells(); i++)
      {
      vtkIdList *InputIds = self->Input->GetCell(i)->GetPointIds();
      int NumIds = InputIds->GetNumberOfIds();
      bool status = true;
      for (k=0; k<NumIds; k++)
        {
        if(PtIds->IsId(InputIds->GetId(k)) == -1)
          {
          status = false;
          break;
          }
        }
      if(status)
        return i;
      status = true;
      }
    }
  else
    {
    for (i=0; i<self->Input->GetNumberOfCells(); i++)
      {
      vtkCell *Cell = self->Input->GetCell(i);
      int NumFaces = Cell->GetNumberOfFaces();
      bool status = true;
      for (j=0; j<NumFaces; j++)
        {
        vtkIdList *InputIds = Cell->GetFace(j)->GetPointIds();
        int NumIds = InputIds->GetNumberOfIds();
        for (k=0; k<NumIds; k++)
          {
          if(PtIds->IsId(InputIds->GetId(k)) == -1)
            {
            status = false;
            break;
            }
          }
        if(status)
          return i;
        status = true;
        }
      }
    }
  return -1;
}
//----------------------------------------------------------------------------------------
void vtkMimxCreateElementSetWidgetFEMesh::AcceptSelectedMesh(
  vtkMimxCreateElementSetWidgetFEMesh *vtkNotUsed(Self))
{
  //if(!Self->ExtractedGrid)      return;

  //if(Self->WidgetEvent == vtkMimxCreateElementSetWidgetFEMesh::ShiftLeftMouseButtonDown)
  //{
  //      if(Self->DeleteCellIds->GetNumberOfIds())
  //      {
  //              Self->WidgetEvent = vtkMimxCreateElementSetWidgetFEMesh::Start;
  //              Self->DeleteExtractedCells(Self);
  //              vtkUnstructuredGrid *tempgrid = vtkUnstructuredGrid::New();
  //              tempgrid->DeepCopy(Self->ExtractedGrid);
  //              Self->ComputeExtractedCellIds(Self, tempgrid);
  //              tempgrid->Delete();
  //      }
  //}

  //if(!Self->ExtractedGrid->GetNumberOfCells())  return;
  //if(!Self->ExtractedGrid->GetNumberOfPoints()) return;
  //if(Self->ExtractedActor)
  //{
  //      Self->CurrentRenderer->RemoveActor(Self->ExtractedActor);
  //}
  ////Self->Input->Initialize();
  ////Self->Input->DeepCopy(Self->ExtractedGrid);
  ////Self->Input->Modified();
  ////
  //if(Self->InputLocator)
  //      Self->InputLocator->Initialize();
  //else
  //      Self->InputLocator = vtkPointLocator::New();
  //if(Self->InputPoints)
  //      Self->InputPoints->Initialize();
  //else
  //      Self->InputPoints = vtkPoints::New();
  //Self->InputLocator->InitPointInsertion(Self->InputPoints, Self->Input->GetBounds());
  //for (int i=0; i<Self->Input->GetNumberOfPoints(); i++)
  //      Self->InputLocator->InsertNextPoint(Self->Input->GetPoint(i));
  ////
  //Self->CurrentRenderer->AddActor(Self->ExtractedActor); 
  //if(Self->ExtractedActor)
  //      Self->CurrentRenderer->RemoveActor(Self->ExtractedActor); 
  //if(Self->SurfaceActor)
  //      Self->CurrentRenderer->RemoveActor(Self->SurfaceActor);
  //Self->ExtractedGrid->Initialize();
  //Self->ExtractedGrid->Modified();
  //Self->Interactor->Render();
}
//-------------------------------------------------------------------------------------------------
void vtkMimxCreateElementSetWidgetFEMesh::SetInput(vtkUnstructuredGrid *input)
{
  //      if(this->Input)
  //              this->Input->Initialize();
  //      else
  //              this->Input = vtkUnstructuredGrid::New();
  //      this->Input->DeepCopy(input);
  //      vtkDataSetMapper::SafeDownCast(
  //              this->ExtractedActor->GetMapper())->SetInput(this->Input);
  //      this->Input->Modified();
  //      //
  ////    this->CurrentRenderer->AddActor(this->ExtractedActor); 
  //      this->ExtractedActor->PickableOn();

  this->Input->Initialize();
  if(this->DimensionState)
    {
    this->InputVolume = input;
    vtkGeometryFilter *fil = vtkGeometryFilter::New();
    fil->SetInput(input);
    fil->Update();
    vtkMergeCells* mergecells = vtkMergeCells::New();
    mergecells->SetUnstructuredGrid(this->Input);
    mergecells->SetTotalNumberOfDataSets(1);
    mergecells->SetTotalNumberOfCells(fil->GetOutput()->GetNumberOfCells());
    mergecells->SetTotalNumberOfPoints(fil->GetOutput()->GetNumberOfPoints());
    mergecells->SetMergeDuplicatePoints(0);
    mergecells->MergeDataSet(fil->GetOutput());
    mergecells->Finish();
    fil->Delete();
    mergecells->Delete();
    this->Input->GetCellData()->Initialize();
    this->Input->GetPointData()->Initialize();
    int numCells = this->Input->GetNumberOfCells();
    int i;
    vtkIntArray *intarray = vtkIntArray::New();
    intarray->SetNumberOfValues(numCells);
    for (i=0; i<numCells; i++)
      {
      intarray->SetValue(i,i);
      }
    intarray->SetName("mimx_Original_Cell_Ids");
    this->Input->GetCellData()->AddArray(intarray);
    intarray->Delete();

    int numPoints = this->Input->GetNumberOfPoints();
    vtkIntArray *pointarray = vtkIntArray::New();
    pointarray->SetNumberOfValues(numPoints);
    for (i=0; i<numPoints; i++)
      {
      pointarray->SetValue(i,i);
      }
    pointarray->SetName("mimx_Original_Point_Ids");
    this->Input->GetPointData()->AddArray(pointarray);
    pointarray->Delete();

    }
  else
    {
    this->Input->DeepCopy(input);
    this->Input->GetCellData()->Initialize();
    this->Input->GetPointData()->Initialize();
    if(input->GetCellData()->GetArray("Bounding_Box_Number"))
      {
      this->Input->GetCellData()->AddArray(
        input->GetCellData()->GetArray("Bounding_Box_Number"));
      }
    if(input->GetCellData()->GetArray("Face_Numbers"))
      {
      this->Input->GetCellData()->AddArray(
        input->GetCellData()->GetArray("Face_Numbers"));
      }
    int numCells = this->Input->GetNumberOfCells();
    int i;
    vtkIntArray *intarray = vtkIntArray::New();
    intarray->SetNumberOfValues(numCells);
    for (i=0; i<numCells; i++)
      {
      intarray->SetValue(i,i);
      }
    intarray->SetName("mimx_Original_Cell_Ids");
    this->Input->GetCellData()->AddArray(intarray);
    intarray->Delete();

    int numPoints = this->Input->GetNumberOfPoints();
    vtkIntArray *pointarray = vtkIntArray::New();
    pointarray->SetNumberOfValues(numPoints);
    for (i=0; i<numPoints; i++)
      {
      pointarray->SetValue(i,i);
      }
    pointarray->SetName("mimx_Original_Point_Ids");
    this->Input->GetPointData()->AddArray(pointarray);
    pointarray->Delete();

    vtkIdList *deleteids = vtkIdList::New();

    vtkCellTypes *cellTypes = vtkCellTypes::New();
    this->Input->GetCellTypes(cellTypes);
    if(cellTypes->GetNumberOfTypes() == 1)
      {
      for (i=0; i<numCells; i++)
        {
        deleteids->InsertNextId(i);
        }
      }
    else
      {
      for (i=0; i<numCells; i++)
        {
        if(this->Input->GetCellType(i) != VTK_TRIANGLE && 
           this->Input->GetCellType(i) != VTK_QUAD)
          {
          deleteids->InsertNextId(i);
          }
        }
      }
    cellTypes->Delete();
    vtkExtractCells *extractcells = vtkExtractCells::New();
    extractcells->SetInput(this->Input);
    extractcells->SetCellList(deleteids);
    extractcells->Update();
    this->Input->DeepCopy(extractcells->GetOutput());
    deleteids->Delete();
    extractcells->Delete();
    }

  if (this->Input->GetNumberOfCells() > 0 )
    this->MeshType = this->Input->GetCellType( 0 );

  this->Input->Modified();
  this->InputMapper->SetInput(this->Input);
  this->InputMapper->Modified();
  this->LineWidth = this->InputActor->GetProperty()->GetLineWidth();
  this->SelectedActor->GetProperty()->SetLineWidth(this->LineWidth*2.0);
  this->ExtractedActor->GetProperty()->SetLineWidth(this->LineWidth*2.0);
  //
  this->SurfaceFilter->SetInput(this->Input);
  this->SurfaceFilter->Update();
  this->SurfaceMapper->SetInput(this->SurfaceFilter->GetOutput());
  this->SurfaceMapper->Modified();
  //
  this->SelectedCellIds->Initialize();
  //numCells = this->Input->GetNumberOfCells();
  //for (i=0; i<numCells; i++)
  //{
  //      this->SelectedCellIds->InsertNextId(i);
  //}
  //this->SelectedCells->SetInput(this->Input);
  //this->SelectedCells->SetCellList(this->SelectedCellIds);
  //this->SelectedCells->Modified();
  //this->SelectedCells->Update();
  // build the locator
  if(this->PointLocator)
    this->PointLocator->Initialize();
  else
    this->PointLocator = vtkPointLocator::New();
  if(this->LocatorPoints)
    this->LocatorPoints->Initialize();
  else
    this->LocatorPoints = vtkPoints::New();
  this->PointLocator->InitPointInsertion(this->LocatorPoints, input->GetBounds());
  int i;
  for (i=0; i<input->GetNumberOfPoints(); i++)
    this->PointLocator->InsertNextPoint(input->GetPoint(i));
  //
  if(this->InputLocator)
    this->InputLocator->Initialize();
  else
    this->InputLocator = vtkPointLocator::New();
  if(this->InputPoints)
    this->InputPoints->Initialize();
  else
    this->InputPoints = vtkPoints::New();
  this->InputLocator->InitPointInsertion(this->InputPoints, input->GetBounds());
  for (i=0; i<input->GetNumberOfPoints(); i++)
    this->InputLocator->InsertNextPoint(input->GetPoint(i));
  //
  //      this->SelectedCellIds->Initialize();
  this->ExtractedCellIds->Initialize();
  this->SelectedPointIds->Initialize();
  this->DeleteCellIds->Initialize();
}
//--------------------------------------------------------------------------------------------
//void vtkMimxCreateElementSetWidgetFEMesh::CrtlRightButtonDownCallback(vtkAbstractWidget *w)
//{
//      vtkMimxCreateElementSetWidgetFEMesh *self = reinterpret_cast<vtkMimxCreateElementSetWidgetFEMesh*>(w);
//      int *size;
//      size = self->Interactor->GetRenderWindow()->GetSize();
//      int X = self->Interactor->GetEventPosition()[0];
//      int Y = self->Interactor->GetEventPosition()[1];
//      //      cout <<X<<"  "<<Y<<endl;
//      self->PickX0 = X;
//      self->PickY0 = Y;
//      // Okay, make sure that the pick is in the current renderer
//      if ( !self->CurrentRenderer || !self->CurrentRenderer->IsInViewport(X, Y) )
//      {
//              self->WidgetEvent = vtkMimxCreateElementSetWidgetFEMesh::Outside;
//              return;
//      }
//      self->AcceptSelectedMesh(self);
//}
//-----------------------------------------------------------------------------------------------
void vtkMimxCreateElementSetWidgetFEMesh::ShiftLeftButtonDownCallback(
  vtkAbstractWidget *vtkNotUsed(w))
{
  //vtkMimxCreateElementSetWidgetFEMesh *self = reinterpret_cast<vtkMimxCreateElementSetWidgetFEMesh*>(w);
  //self->EventCallbackCommand->SetAbortFlag(1);
  //self->StartInteraction();
  //self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  //self->WidgetEvent = vtkMimxCreateElementSetWidgetFEMesh::ShiftLeftMouseButtonDown;
  //self->Interactor->Render();
}
//-----------------------------------------------------------------------------------
void vtkMimxCreateElementSetWidgetFEMesh::ShiftLeftButtonUpCallback(
  vtkAbstractWidget *vtkNotUsed(w))
{
  //vtkMimxCreateElementSetWidgetFEMesh *self = reinterpret_cast<vtkMimxCreateElementSetWidgetFEMesh*>(w);
  //if(self->CellSelectionState != vtkMimxCreateElementSetWidgetFEMesh::SelectMultipleCells)      return;
  //
  //vtkIntArray *scalararray = vtkIntArray::SafeDownCast(
  //      self->Input->GetCellData()->GetArray("Mimx_Scalars"));
  //int i;

  //int numCells = self->Input->GetNumberOfCells();
  //if(!scalararray)
  //{
  //      scalararray = vtkIntArray::New();
  //      scalararray->SetNumberOfValues(numCells);
  //      scalararray->SetName("Mimx_Scalars");
  //      self->Input->GetCellData()->AddArray(scalararray);
  //      for (i=0; i<numCells; i++)
  //      {
  //              scalararray->SetValue(i, 0);
  //      }
  //}

  //int *size;
  //size = self->Interactor->GetRenderWindow()->GetSize();
  //int X = self->Interactor->GetEventPosition()[0];
  //int Y = self->Interactor->GetEventPosition()[1];

  //// Okay, make sure that the pick is in the current renderer
  //if ( !self->CurrentRenderer || !self->CurrentRenderer->IsInViewport(X, Y) )
  //{
  //      self->WidgetEvent = vtkMimxCreateElementSetWidgetFEMesh::Outside;
  //      return;
  //}

  //vtkAssemblyPath *path;
  //vtkCellPicker *CellPicker = vtkCellPicker::New();
  //CellPicker->SetTolerance(0.01);
  //CellPicker->AddPickList(self->ExtractedActor);
  //CellPicker->PickFromListOn();
  //CellPicker->Pick(X,Y,0.0,self->CurrentRenderer);
  //path = CellPicker->GetPath();
  //if ( path != NULL )
  //{
  //      vtkIdType PickedCell = CellPicker->GetCellId();
  //      if(PickedCell != -1)
  //      {
  //              int location = self->DeleteCellIds->IsId(PickedCell);
  //              if(location == -1)
  //              {
  //                      self->DeleteCellIds->InsertNextId(PickedCell);
  //              }
  //              else
  //              {
  //                      self->DeleteCellIds->DeleteId(PickedCell);
  //              }
  //              vtkLookupTable *lut = vtkLookupTable::New();
  //              lut->SetNumberOfColors(2);
  //              lut->Build();
  //              lut->SetTableValue(0, 1.0, 1.0, 1.0, 1.0);
  //              lut->SetTableValue(1, 1.0, 0.0, 0.0, 1.0);
  //              lut->SetTableRange(0,1);
  //              self->ExtractedActor->GetMapper()->SetLookupTable(lut);
  //              self->ExtractedActor->GetMapper()->SetScalarRange(0,1);
  //              lut->Delete();
  //              for (i=0; i<numCells; i++)
  //              {
  //                      scalararray->SetValue(i,0);
  //                      if(self->DeleteCellIds->IsId(i) != -1)
  //                      {
  //                              scalararray->SetValue(i,1);
  //                      }
  //              }
  //              self->Input->GetCellData()->SetScalars(scalararray);
  //              self->Input->Modified();
  //      }
  //}
  //self->Interactor->Render();
  //CellPicker->Delete();
}
//------------------------------------------------------------------------------------
void vtkMimxCreateElementSetWidgetFEMesh::DeleteExtractedCells(
  vtkMimxCreateElementSetWidgetFEMesh *vtkNotUsed(Self))
{
  // create a new selected element set based on computed selected cells
  //Self->ExtractedGrid->Initialize();
  //vtkPointLocator *locator = vtkPointLocator::New();
  //vtkPoints *points = vtkPoints::New();
  //points->Allocate(Self->Input->GetNumberOfPoints());
  //locator->InitPointInsertion(points, Self->Input->GetPoints()->GetBounds());
  //vtkIdList *idlist;
  //int i;
  //double x[3];
  //vtkIdType tempid;
  //int numIds = Self->Input->GetNumberOfCells();
  //Self->ExtractedGrid->Allocate(numIds, numIds);
  //for (i=0; i<numIds; i++)
  //{
  //      if(Self->DeleteCellIds->IsId(i) == -1)
  //      {
  //              idlist = vtkIdList::New();
  //              idlist->DeepCopy(Self->Input->GetCell(i)->GetPointIds());
  //              for(int j=0; j <idlist->GetNumberOfIds(); j++)
  //              {
  //                      Self->Input->GetPoints()->GetPoint(idlist->GetId(j),x);          
  //                      locator->InsertUniquePoint(x,tempid);
  //                      idlist->SetId(j,tempid);
  //              }
  //              Self->ExtractedGrid->InsertNextCell(12, idlist);
  //              idlist->Delete();
  //      }
  //}

  //points->Squeeze();
  //Self->ExtractedGrid->SetPoints(points);
  //Self->ExtractedGrid->Squeeze();
  //points->Delete();
  //locator->Delete();
  //Self->ExtractedGrid->Modified();      
}
//-------------------------------------------------------------------------------------------
int vtkMimxCreateElementSetWidgetFEMesh::DoesCellBelong(
  int vtkNotUsed(CellNum), vtkMimxCreateElementSetWidgetFEMesh *vtkNotUsed(Self))
{
  //int i, j;
  //vtkIdList *OrIds = Self->Input->GetCell(CellNum)->GetPointIds();
  //vtkIdList *SelectedIds = vtkIdList::New();
  //SelectedIds->SetNumberOfIds(OrIds->GetNumberOfIds());

  //for (i = 0; i < OrIds->GetNumberOfIds(); i++)
  //{
  //      int location = Self->InputLocator->IsInsertedPoint(
  //              Self->Input->GetPoint(OrIds->GetId(i)));
  //      if( location == -1)
  //      {
  //              SelectedIds->Delete();
  //              return 0;
  //      }
  //      else
  //      {
  //              SelectedIds->SetId(i, location);
  //      }
  //}
  //// loop through all the cells in the Self->Input to locate the cell connectivity
  //// if found return 1 else return 0;
  //bool status;
  //for (i=0; i< Self->Input->GetNumberOfCells(); i++)
  //{
  //      status = true;
  //      vtkIdList *InputIds = Self->Input->GetCell(i)->GetPointIds();
  //      for (j=0; j<InputIds->GetNumberOfIds(); j++)
  //      {
  //              if(SelectedIds->IsId(InputIds->GetId(j)) == -1)
  //              {
  //                      status = false;
  //                      break;
  //              }
  //      }
  //      if(status)
  //      {
  //              SelectedIds->Delete();
  //              return 1;
  //      }
  //}
  //SelectedIds->Delete();
  //return 0;
  return 1;
}
//-------------------------------------------------------------------------------------------
void vtkMimxCreateElementSetWidgetFEMesh::LeftButtonUpCallback(vtkAbstractWidget* w)
{
  vtkMimxCreateElementSetWidgetFEMesh *Self = 
    reinterpret_cast<vtkMimxCreateElementSetWidgetFEMesh*>(w);

  if(Self->PickStatus)
    {
    Self->CrtlLeftButtonUpCallback(w);
    return;
    }
  //if(Self->CellSelectionState == 
  //      vtkMimxCreateElementSetWidgetFEMesh::SelectMultipleCells)
  //{
  //      Self->ShiftLeftButtonUpCallback(w);
  //}
}
//--------------------------------------------------------------------------------------------
void vtkMimxCreateElementSetWidgetFEMesh::SetCellSelectionState(int Selection)
{
  this->CellSelectionState = Selection;
  if(     this->CellSelectionState == vtkMimxCreateElementSetWidgetFEMesh::SelectIndividualCell)
    {
    if(this->EditMeshState == vtkMimxCreateElementSetWidgetFEMesh::FullMesh)
      {
      this->CurrentRenderer->AddActor(this->SurfaceActor);
      this->CurrentRenderer->RemoveActor(this->InputActor);
      }
    else
      {
      this->CurrentRenderer->AddActor(this->ExtractedSurfaceActor);
      this->CurrentRenderer->RemoveActor(this->ExtractedActor);
      }
    }
  else
    {
    if(this->EditMeshState == vtkMimxCreateElementSetWidgetFEMesh::FullMesh)
      {
      this->CurrentRenderer->RemoveActor(this->SurfaceActor);
      this->CurrentRenderer->AddActor(this->InputActor);
      }
    else
      {
      this->CurrentRenderer->RemoveActor(this->ExtractedSurfaceActor);
      this->CurrentRenderer->AddActor(this->ExtractedActor);
      }
    }
  this->ExtractedCellIds->Initialize();
  this->ExtractedCells->Update();
  this->CurrentRenderer->Render();
}
//---------------------------------------------------------------------------------------------
void vtkMimxCreateElementSetWidgetFEMesh::ShiftMouseMoveCallback(
  vtkAbstractWidget *vtkNotUsed(w))
{
  //vtkMimxCreateElementSetWidgetFEMesh *self = reinterpret_cast<vtkMimxCreateElementSetWidgetFEMesh*>(w);
  //if(self->CellSelectionState == vtkMimxCreateElementSetWidgetFEMesh::SelectIndividualCell)
  //{
  //      self->SelectIndividualCellFunction(self);
  //}
}
//-------------------------------------------------------------------------------------------------
int vtkMimxCreateElementSetWidgetFEMesh::GetCellNumGivenFaceIds(
  vtkIdList *PtIds, vtkMimxCreateElementSetWidgetFEMesh *Self)
{
  int i,j,k;
  int numcells = Self->Input->GetNumberOfCells();
  for (i=0; i<numcells; i++)
    {
    vtkCell *cell = Self->Input->GetCell(i);
    int numfaces = cell->GetNumberOfFaces();
    for (j=0; j<numfaces; j++)
      {
      vtkCell *face = cell->GetFace(j);
      vtkIdList *faceids = face->GetPointIds();
      bool status = true;
      for (k=0; k<faceids->GetNumberOfIds(); k++)
        {
        if(PtIds->IsId(faceids->GetId(k)) == -1)
          {
          status = false;
          break;
          }
        }
      if(status)      
        {
        return i;
        }
      }
    }
  return -1;
}
//---------------------------------------------------------------------------------------------
int vtkMimxCreateElementSetWidgetFEMesh::GetFaceNumGivenCellNumFaceIds(
  int CellNum, vtkIdList *PtIds, vtkPolyData *aSurface, vtkMimxCreateElementSetWidgetFEMesh *Self)
{
  int i,k;
  int x;
  for(i=0; i<PtIds->GetNumberOfIds(); i++)
    {
    x = PtIds->GetId(i);
    }
  vtkUnstructuredGrid *bbox = vtkUnstructuredGrid::New();
  vtkDataArray *dataarray = Self->Input->GetFieldData()->GetArray("Mesh_Seed");
  bbox->GetCellData()->AddArray(dataarray);
  bbox->Allocate(dataarray->GetNumberOfTuples(), dataarray->GetNumberOfTuples());
  for(i=0; i< dataarray->GetNumberOfTuples(); i++)
    {
    vtkIdList *idlist = vtkIdList::New();
    switch (this->MeshType)
      {
      case VTK_HEXAHEDRON:
        idlist->SetNumberOfIds(8);
        bbox->InsertNextCell(VTK_HEXAHEDRON, idlist);
        break;
      case VTK_TETRA: 
        idlist->SetNumberOfIds(4);
        bbox->InsertNextCell(VTK_TETRA, idlist);
        break;
      case VTK_QUAD:
        idlist->SetNumberOfIds(4);
        bbox->InsertNextCell(VTK_QUAD, idlist);
        break;
      case VTK_TRIANGLE:
        idlist->SetNumberOfIds(3);
        bbox->InsertNextCell(VTK_TRIANGLE, idlist);
        break;
      default:
        idlist->SetNumberOfIds(8);
        bbox->InsertNextCell(VTK_HEXAHEDRON, idlist);
        break;
      }
    idlist->SetNumberOfIds(8);
    bbox->InsertNextCell(12, idlist);
    idlist->Delete();
    }
  vtkMimxUnstructuredToStructuredGrid *utosgrid = vtkMimxUnstructuredToStructuredGrid::New();
  utosgrid->SetInput(Self->Input);
  utosgrid->SetBoundingBox(bbox);
  utosgrid->SetStructuredGridNum(CellNum);
  utosgrid->Update();
  vtkStructuredGrid *sgrid = utosgrid->GetOutput();
  //vtkStructuredGridWriter *swriter = vtkStructuredGridWriter::New();
  //swriter->SetInput(sgrid);
  //swriter->SetFileName("SGridSolid.vtk");
  //swriter->Write();
  //swriter->Delete();
  bbox->Delete();
  for (i=0; i<6; i++)
    {
    vtkMimxExtractStructuredGridFace *exface =
      vtkMimxExtractStructuredGridFace::New();
    exface->SetInput(sgrid);
    exface->SetFaceNum(i);
    exface->Update();
    //swriter = vtkStructuredGridWriter::New();
    //swriter->SetInput(exface->GetOutput());
    //swriter->SetFileName("SGridFace.vtk");
    //swriter->Write();
    //swriter->Delete();

    vtkStructuredGrid *facesgrid = exface->GetOutput();
    vtkIntArray *intarray = vtkIntArray::SafeDownCast(
      facesgrid->GetPointData()->GetArray("mimx_Original_Point_Ids"));
    if(!intarray)   return -1;
    //vtkPointLocator *localPointLocator = vtkPointLocator::New();
    //vtkPoints *localPoints = vtkPoints::New();
    //localPointLocator->InitPointInsertion(localPoints, exface->GetOutput()->GetBounds());
    //for (j=0; j<facesgrid->GetNumberOfPoints(); j++)
    //      localPointLocator->InsertNextPoint(facesgrid->GetPoint(j));

    //// check if the points list from pointids match
    //bool status = true;
    //for (k=0; k<PtIds->GetNumberOfIds(); k++)
    //{
    //      int location = localPointLocator->IsInsertedPoint(
    //              Self->Input->GetPoint(PtIds->GetId(k)));
    //      if(location == -1)
    //      {
    //              status = false;
    //              break;
    //      }
    //}
    //localPointLocator->Delete();
    //localPoints->Delete();

    // check if the points list from pointids match
    vtkIdList *templist = vtkIdList::New();
    for (k=0; k<intarray->GetNumberOfTuples(); k++)
      {
      templist->InsertUniqueId(intarray->GetValue(k));
      }
    bool status = true;
    for (k = 0; k < PtIds->GetNumberOfIds(); k++)
      {
      int location = templist->IsId(PtIds->GetId(k));
      if(location == -1)
        {
        status = false;
        break;
        }
      }

    if(status)
      {
      //vtkStructuredGridWriter *writer = vtkStructuredGridWriter::New();
      //writer->SetFileName("Structured.vtk");
      //writer->SetInput(exface->GetOutput());
      //writer->Write();
      //writer->Delete();
      vtkStructuredGridGeometryFilter *fil = vtkStructuredGridGeometryFilter::New();
      fil->SetInput(exface->GetOutput());
      fil->Update();
      aSurface->DeepCopy(fil->GetOutput());
      fil->Delete();
      exface->Delete();
      utosgrid->Delete();
      //vtkPolyDataWriter *pwriter = vtkPolyDataWriter::New();
      //pwriter->SetFileName("Surface.vtk");
      //pwriter->SetInput(Surface);
      //pwriter->Write();
      //pwriter->Delete();
      return i;
      }
    exface->Delete();
    }
  utosgrid->Delete();
  return -1;
}
//----------------------------------------------------------------------------------------------
void vtkMimxCreateElementSetWidgetFEMesh::ExtractElementsBelongingToAFace(
  vtkMimxCreateElementSetWidgetFEMesh *self)
{
  if(self->EditMeshState == vtkMimxCreateElementSetWidgetFEMesh::SelectedMesh)
    return;
  vtkPolyData *polydata = self->SurfaceFilter->GetOutput();
  vtkIntArray *scalararray = vtkIntArray::SafeDownCast(
    polydata->GetCellData()->GetArray("Mimx_Scalars"));
  if(!scalararray)        return;
  int i;
  for (i=0; i<polydata->GetNumberOfCells(); i++)
    {
    if(scalararray->GetValue(i))    break;
    }
  int selectedCell = i;
  vtkIntArray *bboxArray = vtkIntArray::SafeDownCast(
    self->Input->GetCellData()->GetArray("Bounding_Box_Number"));
  vtkIntArray *faceArray = vtkIntArray::SafeDownCast(
    self->Input->GetCellData()->GetArray("Face_Numbers"));

  if(bboxArray && faceArray)
    {
    int bboxNum = bboxArray->GetValue(selectedCell);
    int faceNum = faceArray->GetValue(selectedCell);
    self->ExtractedCellIds->Initialize();

    for (i=0; i<self->Input->GetNumberOfCells(); i++)
      {
      int currBboxNum = bboxArray->GetValue(i);
      int cuurFaceNum = faceArray->GetValue(i);
      if(bboxNum == currBboxNum && faceNum == cuurFaceNum)
        {
        self->ExtractedCellIds->InsertNextId(i);
        }
      }
    self->ExtractedCells->SetInput(self->Input);
    self->ExtractedCells->SetCellList(self->ExtractedCellIds);
    self->ExtractedCells->Modified();
    self->ExtractedCells->Update();
    self->ExtractedCells->GetOutput()->GetCellData()->RemoveArray("vtkOriginalCellIds");
    self->CurrentRenderer->AddActor(self->ExtractedActor);
    self->EventCallbackCommand->SetAbortFlag(1);
    self->EndInteraction();
    self->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
    self->PickStatus = 0;
    self->Interactor->Render();
    return ;
    }
  vtkIdList *ptids = vtkIdList::New();
  polydata->GetCellPoints(i, ptids);
  int cellnum;
  vtkIntArray *intarray = vtkIntArray::SafeDownCast(polydata->GetCellData()->GetArray("mimx_Original_Cell_Ids"));
  if(intarray)    cellnum = intarray->GetValue(i);
  else    cellnum = self->GetCellNumGivenFaceIds(ptids, self);
  if(cellnum == -1)       return;
  int dim[3];
  vtkIntArray *meshseed = vtkIntArray::SafeDownCast(
    self->Input->GetFieldData()->GetArray("Mesh_Seed"));
  if(!meshseed)   return;
  int StartEleNum = 0;
  int EndEleNum = 0;
  int bblocknum = -1;
  meshseed->GetTupleValue(0, dim);
  for (i=0; i<meshseed->GetNumberOfTuples(); i++)
    {
    meshseed->GetTupleValue(i, dim);
    EndEleNum = EndEleNum + (dim[0]-1)*(dim[1]-1)*(dim[2]-1)-1;
    if(cellnum >= StartEleNum && cellnum <= EndEleNum)
      {
      bblocknum = i;
      break;
      }
    EndEleNum ++;
    StartEleNum = EndEleNum;
    }
  if(bblocknum == -1)
    {
    ptids->Delete();
    return;
    }
  vtkPolyData *surface = vtkPolyData::New();
  int FaceNum =  self->GetFaceNumGivenCellNumFaceIds(bblocknum, ptids, surface, self);
  if(FaceNum == -1)
    {
    ptids->Delete();
    surface->Delete();
    return;
    }
  //vtkPolyDataWriter *writer = vtkPolyDataWriter::New();
  //writer->SetFileName("Extract.vtk");
  //writer->SetInput(surface);
  //writer->Write();
  //writer->Delete();
  self->ComputeExtractedCellIdsSurface(self, surface);
  surface->Delete();
  ptids->Delete();
  self->EventCallbackCommand->SetAbortFlag(1);
  self->EndInteraction();
  self->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
  self->PickStatus = 0;
  self->Interactor->Render();
}
//----------------------------------------------------------------------------------------------
void vtkMimxCreateElementSetWidgetFEMesh::SelectMultipleCellFunction(
  vtkMimxCreateElementSetWidgetFEMesh *self)
{
  int *size;
  size = self->Interactor->GetRenderWindow()->GetSize();
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // Okay, make sure that the pick is in the current renderer
  if ( !self->CurrentRenderer || !self->CurrentRenderer->IsInViewport(X, Y) )
    {
    self->WidgetEvent = vtkMimxCreateElementSetWidgetFEMesh::Outside;
    return;
    }
  int i;
  vtkAssemblyPath *path;
  vtkCellPicker *CellPicker = vtkCellPicker::New();
  CellPicker->SetTolerance(0.01);
  CellPicker->AddPickList(self->InputActor);
  CellPicker->PickFromListOn();
  CellPicker->Pick(X,Y,0.0,self->CurrentRenderer);
  path = CellPicker->GetPath();
  if ( path != NULL )
    {
    vtkIdType PickedCell = CellPicker->GetCellId();
    if(PickedCell != -1)
      {
      vtkLookupTable *lut = vtkLookupTable::New();
      lut->SetNumberOfColors(2);
      lut->Build();
      lut->SetTableValue(0, 1.0, 1.0, 1.0, 1.0);
      lut->SetTableValue(1, 1.0, 0.0, 0.0, 1.0);
      lut->SetTableRange(0,1);
      self->InputActor->GetMapper()->SetLookupTable(lut);
      self->InputActor->GetMapper()->SetScalarRange(0,1);
      lut->Delete();
      vtkIntArray *intarray = vtkIntArray::New();
      intarray->SetNumberOfValues(self->Input->GetNumberOfCells());
      for (i=0; i<self->Input->GetNumberOfCells(); i++)
        {
        intarray->SetValue(i, 0);
        }
      intarray->SetValue(PickedCell, 1);
      intarray->SetName("Mimx_Scalars");
      self->Input->GetCellData()->SetScalars(intarray);
      intarray->Delete();
      self->Input->Modified();
      }
    }
  self->Interactor->Render();
  CellPicker->Delete();
}
//---------------------------------------------------------------------------------------------------
void vtkMimxCreateElementSetWidgetFEMesh::AddDeleteSelectedElement(vtkMimxCreateElementSetWidgetFEMesh *Self)
{

  //vtkPolyData *polydata;
  //if(Self->EditMeshState == vtkMimxCreateElementSetWidgetFEMesh::FullMesh)
  //      polydata = Self->SurfaceFilter->GetOutput();
  //else
  //      polydata = Self->ExtractedSurfaceFilter->GetOutput();

  //vtkIntArray *scalararray = vtkIntArray::SafeDownCast(
  //      polydata->GetCellData()->GetArray("Mimx_Scalars"));
  //if(!scalararray)      return;
  //int i;
  //for (i=0; i<polydata->GetNumberOfCells(); i++)
  //{
  //      if(scalararray->GetValue(i))    break;
  //}
  //int cellnum;
  //if(Self->EditMeshState == vtkMimxCreateElementSetWidgetFEMesh::FullMesh)
  //      cellnum = Self->GetCellNumGivenFaceIds(polydata->GetCell(i)->GetPointIds(), Self);
  //else
  //      cellnum = Self->GetCellNumGivenFaceIdsSelectedMesh(polydata->GetCell(i)->GetPointIds(), Self);
  //if(Self->BooleanState == vtkMimxCreateElementSetWidgetFEMesh::Add)
  //      Self->ExtractedCellIds->InsertUniqueId(cellnum);
  //else
  //      Self->ExtractedCellIds->DeleteId(cellnum);
  //Self->ExtractedCells->SetInput(Self->Input);
  //Self->ExtractedCells->SetCellList(Self->ExtractedCellIds);
  //Self->ExtractedCells->Modified();
  //Self->ExtractedCells->Update();
  //Self->CurrentRenderer->AddActor(Self->ExtractedActor);


  //Self->EventCallbackCommand->SetAbortFlag(1);
  //Self->EndInteraction();
  //Self->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
  //Self->PickStatus = 0;
  //Self->Interactor->Render();

  vtkUnstructuredGrid *Ugrid;
  if(Self->EditMeshState == vtkMimxCreateElementSetWidgetFEMesh::FullMesh)
    Ugrid = Self->Input;
  else
    Ugrid = Self->SelectedCells->GetOutput();

  vtkIntArray *scalararray = vtkIntArray::SafeDownCast(
    Ugrid->GetCellData()->GetArray("Mimx_Scalars"));
  if(!scalararray)        return;
  int i;
  for (i=0; i<Ugrid->GetNumberOfCells(); i++)
    {
    if(scalararray->GetValue(i))    break;
    }
  int cellnum;
  if(Self->EditMeshState == vtkMimxCreateElementSetWidgetFEMesh::FullMesh)
    {
    cellnum = i;
    Self->InputMapper->SetScalarVisibility(0);
    }
  else
    {
    cellnum = scalararray->GetValue(i);
    Self->SelectedMapper->SetScalarVisibility(0);
    }
  if(Self->BooleanState == vtkMimxCreateElementSetWidgetFEMesh::Add)
    Self->SelectedCellIds->InsertUniqueId(cellnum);
  else
    Self->SelectedCellIds->DeleteId(cellnum);
  Ugrid->GetCellData()->RemoveArray("Mimx_Scalars");
  Self->SelectedCells->SetInput(Self->Input);
  Self->SelectedCells->SetCellList(Self->SelectedCellIds);
  Self->SelectedCells->Modified();
  Self->SelectedCells->Update();
  Self->SelectedCells->GetOutput()->GetCellData()->RemoveArray("vtkOriginalCellIds");
  Self->CurrentRenderer->AddActor(Self->SelectedActor);
  Self->EventCallbackCommand->SetAbortFlag(1);
  Self->EndInteraction();
  Self->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
  Self->PickStatus = 0;
  Self->Interactor->Render();

}
//---------------------------------------------------------------------------------------------------------
void vtkMimxCreateElementSetWidgetFEMesh::LeftButtonDownCallback(vtkAbstractWidget* w)
{
  vtkMimxCreateElementSetWidgetFEMesh *self = 
    reinterpret_cast<vtkMimxCreateElementSetWidgetFEMesh*>(w);

  if(self->PickStatus)
    {
    self->RubberBandStyle->GetInteractor()->SetKeyCode('r');
    self->RubberBandStyle->OnChar();
    self->RubberBandStyle->OnLeftButtonUp();
    self->RubberBandStyle->SetEnabled(0);
    self->PickStatus = 0;
    return;
    }
}
//--------------------------------------------------------------------------------------------
void vtkMimxCreateElementSetWidgetFEMesh::SetEditMeshState(int aMeshType)
{
  this->EditMeshState = aMeshType;
  if(aMeshType == vtkMimxCreateElementSetWidgetFEMesh::FullMesh)
    {
    this->SelectedActor->GetProperty()->SetRepresentationToWireframe();
    this->SelectedActor->GetProperty()->SetColor(1.0, 0.0, 0.0);
    this->CurrentRenderer->AddActor(this->InputActor);
    this->CurrentRenderer->AddActor(this->SurfaceActor);
    //              this->CurrentRenderer->RemoveActor(this->SelectedActor);
    //              this->BooleanState = vtkMimxCreateElementSetWidgetFEMesh::Add;
    }
  else
    {
    this->SelectedActor->GetProperty()->SetRepresentationToSurface();
    this->SelectedActor->GetProperty()->SetColor(1.0, 1.0, 1.0);
    this->CurrentRenderer->RemoveActor(this->InputActor);
    this->CurrentRenderer->RemoveActor(this->SurfaceActor);
    //              this->BooleanState = vtkMimxCreateElementSetWidgetFEMesh::Subtract;
    }
  this->CurrentRenderer->Render();
}
//-----------------------------------------------------------------------------------------------------
int vtkMimxCreateElementSetWidgetFEMesh::GetCellNumGivenFaceIdsSelectedMesh(
  vtkIdList *PtIds, vtkMimxCreateElementSetWidgetFEMesh *Self)
{
  vtkPolyData *aSurface = Self->ExtractedSurfaceFilter->GetOutput();
  vtkIntArray *intarray = vtkIntArray::SafeDownCast(
    aSurface->GetPointData()->GetArray("mimx_Original_Point_Ids"));
  if(!intarray)   return -1;
  vtkIdList *idlist = vtkIdList::New();
  idlist->SetNumberOfIds(PtIds->GetNumberOfIds());
  int i;

  for (i=0; i<PtIds->GetNumberOfIds(); i++)
    {
    idlist->SetId(i, intarray->GetValue(PtIds->GetId(i)));
    }
  int cellnum = Self->GetCellNumGivenFaceIds(idlist, Self);
  idlist->Delete();
  return cellnum;
}
//-------------------------------------------------------------------------------------------------------
void vtkMimxCreateElementSetWidgetFEMesh::RightButtonDownCallback(vtkAbstractWidget* w)
{
  vtkMimxCreateElementSetWidgetFEMesh *self = 
    reinterpret_cast<vtkMimxCreateElementSetWidgetFEMesh*>(w);

  int *size;
  size = self->Interactor->GetRenderWindow()->GetSize();
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  self->SurfaceActor->GetMapper()->SetScalarVisibility(0);

  // Okay, make sure that the pick is in the current renderer
  if ( !self->CurrentRenderer || !self->CurrentRenderer->IsInViewport(X, Y) )
    {
    self->WidgetEvent = vtkMimxCreateElementSetWidgetFEMesh::Outside;
    self->EventCallbackCommand->SetAbortFlag(1);
    self->EndInteraction();
    self->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
    self->Interactor->Render();
    return;
    }

  //      self->WidgetEvent = vtkMimxCreateElementSetWidgetFEMesh::ShiftMouseMove;

  int i;
  vtkAssemblyPath *path;
  vtkCellPicker *CellPicker = vtkCellPicker::New();
  CellPicker->SetTolerance(0.01);
  vtkUnstructuredGrid *Ugrid;

  CellPicker->AddPickList(self->ExtractedActor);
  Ugrid = self->ExtractedCells->GetOutput();

  CellPicker->PickFromListOn();
  CellPicker->Pick(X,Y,0.0,self->CurrentRenderer);
  path = CellPicker->GetPath();

  //std::cout << "Boolean State : " << self->BooleanState << std::endl;

  if ( path != NULL )
    {
    vtkIdType PickedCell = CellPicker->GetCellId();
    if(PickedCell != -1)
      {
      int numSelectedIds = self->SelectedCellIds->GetNumberOfIds();
      int numIds = self->ExtractedCellIds->GetNumberOfIds();
      if(!numSelectedIds)
        {
        if(self->BooleanState == vtkMimxCreateElementSetWidgetFEMesh::Add)
          {
          self->SelectedCellIds->Initialize();
          self->SelectedCellIds->DeepCopy(self->ExtractedCellIds);
          }
        else
          {
          for (i=0; i<self->Input->GetNumberOfCells(); i++)
            {
            self->SelectedCellIds->InsertNextId(i);
            }
          for (i=0; i<numIds; i++)
            {
            self->SelectedCellIds->DeleteId(self->ExtractedCellIds->GetId(i));
            }
          }
        }
      else
        {

        if(self->EditMeshState == vtkMimxCreateElementSetWidgetFEMesh::SelectedMesh)

          {
          if(self->BooleanState == vtkMimxCreateElementSetWidgetFEMesh::Add)
            {
            self->SelectedCellIds->Initialize();
            self->SelectedCellIds->DeepCopy(self->ExtractedCellIds);
            }
          else
            {
            for (i=0; i<numIds; i++)
              {
              self->SelectedCellIds->DeleteId(self->ExtractedCellIds->GetId(i));
              }
            }
          }
        else
          {
          if(self->BooleanState == vtkMimxCreateElementSetWidgetFEMesh::Add)
            {
            for (i=0; i<numIds;i++)
              {
              self->SelectedCellIds->InsertUniqueId(self->ExtractedCellIds->GetId(i));
              }
            }
          else
            {
            for (i=0; i<numIds; i++)
              {
              self->SelectedCellIds->DeleteId(self->ExtractedCellIds->GetId(i));
              }

            }
          }
        }
      self->ExtractedCellIds->Initialize();
      self->ExtractedCells->SetInput(self->Input);
      self->ExtractedCells->SetCellList(self->ExtractedCellIds);
      self->ExtractedCells->Modified();
      self->ExtractedCells->Update();
      self->ExtractedCells->GetOutput()->GetCellData()->RemoveArray("vtkOriginalCellIds");

      self->SelectedCells->SetInput(self->Input);
      self->SelectedCells->SetCellList(self->SelectedCellIds);
      self->SelectedCells->Modified();
      self->SelectedCells->Update();
      self->SelectedCells->GetOutput()->GetCellData()->RemoveArray("vtkOriginalCellIds");
      self->CurrentRenderer->AddActor(self->SelectedActor);
      self->Interactor->Render();
      }
    }
  CellPicker->Delete();
  //self->EventCallbackCommand->SetAbortFlag(1);
  //self->EndInteraction();
  //self->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
}
//--------------------------------------------------------------------------------------------------------
void vtkMimxCreateElementSetWidgetFEMesh::RightButtonUpCallback(
  vtkAbstractWidget* vtkNotUsed(w))
{
}
//--------------------------------------------------------------------------------------------------------
void vtkMimxCreateElementSetWidgetFEMesh::MouseMoveCallback(vtkAbstractWidget* w)
{
  vtkMimxCreateElementSetWidgetFEMesh *self = 
    reinterpret_cast<vtkMimxCreateElementSetWidgetFEMesh*>(w);
  if(self->PickStatus)
    {
    self->CrtlLeftButtonUpCallback(w);
    return;
    }
  //self->EventCallbackCommand->SetAbortFlag(1);
  //self->InvokeEvent(vtkCommand::InteractionEvent,NULL);

  //self->Interactor->Render();

}
//--------------------------------------------------------------------------------------------------------
void vtkMimxCreateElementSetWidgetFEMesh::SelectIndividualSurfaceCellFunction(vtkMimxCreateElementSetWidgetFEMesh *self)
{
  if(self->EditMeshState == vtkMimxCreateElementSetWidgetFEMesh::SelectedMesh)
    return;
  int *size;
  size = self->Interactor->GetRenderWindow()->GetSize();
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // Okay, make sure that the pick is in the current renderer
  if ( !self->CurrentRenderer || !self->CurrentRenderer->IsInViewport(X, Y) )
    {
    self->WidgetEvent = vtkMimxCreateElementSetWidgetFEMesh::Outside;
    return;
    }

  //      self->WidgetEvent = vtkMimxCreateElementSetWidgetFEMesh::ShiftMouseMove;

  int i;
  vtkAssemblyPath *path;
  vtkCellPicker *CellPicker = vtkCellPicker::New();
  CellPicker->SetTolerance(0.01);
  vtkPolyData *Surface;
  CellPicker->AddPickList(self->SurfaceActor);
  Surface = self->SurfaceFilter->GetOutput();
  CellPicker->PickFromListOn();
  CellPicker->Pick(X,Y,0.0,self->CurrentRenderer);
  path = CellPicker->GetPath();
  self->SurfaceActor->GetMapper()->SetScalarVisibility(1);
  if ( path != NULL )
    {
    vtkIdType PickedCell = CellPicker->GetCellId();
    if(PickedCell != -1)
      {
      vtkLookupTable *lut = vtkLookupTable::New();
      lut->SetNumberOfColors(2);
      lut->Build();
      lut->SetTableValue(0, 1.0, 1.0, 1.0, 1.0);
      lut->SetTableValue(1, 0.0, 1.0, 0.0, 1.0);
      lut->SetTableRange(0,1);
      self->SurfaceActor->GetMapper()->SetLookupTable(lut);
      self->SurfaceActor->GetMapper()->SetScalarRange(0,1);
      lut->Delete();
      vtkIntArray *intarray = vtkIntArray::New();
      intarray->SetNumberOfValues(Surface->GetNumberOfCells());
      for (i=0; i<Surface->GetNumberOfCells(); i++)
        {
        intarray->SetValue(i, 0);
        }
      intarray->SetValue(PickedCell, 1);
      intarray->SetName("Mimx_Scalars");
      Surface->GetCellData()->SetScalars(intarray);
      intarray->Delete();
      Surface->Modified();
      }
    }
  self->Interactor->Render();
  CellPicker->Delete();
}
//----------------------------------------------------------------------------------------------------
void vtkMimxCreateElementSetWidgetFEMesh::ClearSelections()
{
  if(this->Input)
    {
    this->ExtractedCellIds->Initialize();
    this->ExtractedCells->SetInput(this->Input);
    this->ExtractedCells->SetCellList(this->ExtractedCellIds);
    this->ExtractedCells->Modified();
    this->ExtractedCells->Update();
    this->ExtractedCells->GetOutput()->GetCellData()->RemoveArray("vtkOriginalCellIds");    
    //if(this->EditMeshState == vtkMimxCreateElementSetWidgetFEMesh::FullMesh)
    //{
    this->SelectedCellIds->Initialize();
    //int i;
    //int numCells = this->Input->GetNumberOfCells();
    //for (i=0; i<numCells; i++)
    //{
    //      this->SelectedCellIds->InsertNextId(i);
    //}
    this->SelectedCells->SetInput(this->Input);
    this->SelectedCells->SetCellList(this->SelectedCellIds);
    this->SelectedCells->Modified();
    this->SelectedCells->Update();
    this->SelectedCells->GetOutput()->GetCellData()->RemoveArray("vtkOriginalCellIds");
    //}
    this->Interactor->Render();
    }
}
//----------------------------------------------------------------------------------------------------
void vtkMimxCreateElementSetWidgetFEMesh::SetInputOpacity(double Val)
{
  this->InputActor->GetProperty()->SetOpacity(Val);
  this->SurfaceActor->GetProperty()->SetOpacity(Val);
  if(Val == 1.0)
    {
    this->EditMeshState = vtkMimxCreateElementSetWidgetFEMesh::FullMesh;
    }
  else
    {
    this->EditMeshState = vtkMimxCreateElementSetWidgetFEMesh::SelectedMesh;
    }
  if(Val == 0.0)
    {
    this->SelectedActor->GetProperty()->SetColor(1.0, 1.0, 1.0);
    this->SelectedActor->GetProperty()->SetRepresentationToSurface();
    }
  else
    {
    this->SelectedActor->GetProperty()->SetColor(1.0, 0.0, 0.0);
    this->SelectedActor->GetProperty()->SetRepresentationToWireframe();
    }

}
//-------------------------------------------------------------------------------------------------
int vtkMimxCreateElementSetWidgetFEMesh::ComputeOriginalCellNumberFromSelectedSurfaceMesh(
  vtkMimxCreateElementSetWidgetFEMesh *self, vtkIdList *PtIds)
{
  int i, j, k;

  vtkUnstructuredGrid *sel = self->SelectedCells->GetOutput();
  for (i=0; i<sel->GetNumberOfCells(); i++)
    {
    vtkCell *Cell = sel->GetCell(i);
    int NumFaces = Cell->GetNumberOfFaces();
    bool status = true;
    for (j=0; j<NumFaces; j++)
      {
      vtkIdList *InputIds = Cell->GetFace(j)->GetPointIds();
      int NumIds = InputIds->GetNumberOfIds();
      vtkIdList *orIds = vtkIdList::New();
      orIds->SetNumberOfIds(NumIds);
      vtkIntArray *pointarray = vtkIntArray::SafeDownCast(
        sel->GetPointData()->GetArray("mimx_Original_Point_Ids"));
      for (k=0; k<NumIds; k++)
        {
        orIds->SetId(k, pointarray->GetValue(InputIds->GetId(k)));
        }
      for (k=0; k<NumIds; k++)
        {
        if(PtIds->IsId(orIds->GetId(k)) == -1)
          {
          status = false;
          break;
          }
        }
      orIds->Delete();
      if(status)
        {
        vtkIntArray *intarray = vtkIntArray::SafeDownCast(
          sel->GetCellData()->GetArray("mimx_Original_Cell_Ids"));
        if(intarray)
          return intarray->GetValue(i);
        }
      status = true;
      }
    }

  return -1;
}
//--------------------------------------------------------------------------------------------------
void vtkMimxCreateElementSetWidgetFEMesh::ExtractElementsBelongingToAFaceSurface(
  vtkMimxCreateElementSetWidgetFEMesh *self)
{
  if(self->EditMeshState == vtkMimxCreateElementSetWidgetFEMesh::SelectedMesh)
    return;
  vtkPolyData *polydata = self->SurfaceFilter->GetOutput();
  vtkIntArray *scalararray = vtkIntArray::SafeDownCast(
    polydata->GetCellData()->GetArray("Mimx_Scalars"));
  if(!scalararray)        return;
  int i;
  for (i=0; i<polydata->GetNumberOfCells(); i++)
    {
    if(scalararray->GetValue(i))    break;
    }
  vtkIdList *ptids = vtkIdList::New();
  polydata->GetCellPoints(i, ptids);
  int cellnum = self->GetCellNumGivenFaceIdsSurface(ptids, self);
  if(cellnum == -1)       return;
  int dim[3];
  vtkIntArray *meshseed = vtkIntArray::SafeDownCast(
    self->InputVolume->GetFieldData()->GetArray("Mesh_Seed"));
  if(!meshseed)   return;
  int StartEleNum = 0;
  int EndEleNum = 0;
  int bblocknum = -1;
  meshseed->GetTupleValue(0, dim);
  for (i=0; i<meshseed->GetNumberOfTuples(); i++)
    {
    meshseed->GetTupleValue(i, dim);
    EndEleNum = EndEleNum + (dim[0]-1)*(dim[1]-1)*(dim[2]-1)-1;
    if(cellnum >= StartEleNum && cellnum <= EndEleNum)
      {
      bblocknum = i;
      break;
      }
    EndEleNum ++;
    StartEleNum = EndEleNum;
    }
  if(bblocknum == -1)
    {
    ptids->Delete();
    return;
    }
  vtkPolyData *surface = vtkPolyData::New();
  int FaceNum =  self->GetFaceNumGivenCellNumFaceIdsSurface(bblocknum, ptids, surface, self);
  if(FaceNum == -1)
    {
    ptids->Delete();
    surface->Delete();
    return;
    }
  //vtkPolyDataWriter *writer = vtkPolyDataWriter::New();
  //writer->SetFileName("Extract.vtk");
  //writer->SetInput(surface);
  //writer->Write();
  //writer->Delete();
  self->ComputeExtractedCellIdsSurface(self, surface);
  surface->Delete();
  ptids->Delete();
  self->EventCallbackCommand->SetAbortFlag(1);
  self->EndInteraction();
  self->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
  self->PickStatus = 0;
  self->Interactor->Render();
}
//----------------------------------------------------------------------------------------------
int vtkMimxCreateElementSetWidgetFEMesh::GetCellNumGivenFaceIdsSurface(
  vtkIdList *PtIds, vtkMimxCreateElementSetWidgetFEMesh *Self)
{
  int i,j,k;
  int numcells = Self->InputVolume->GetNumberOfCells();
  for (i=0; i<numcells; i++)
    {
    vtkCell *cell = Self->InputVolume->GetCell(i);
    int numfaces = cell->GetNumberOfFaces();
    for (j=0; j<numfaces; j++)
      {
      vtkCell *face = cell->GetFace(j);
      vtkIdList *faceids = face->GetPointIds();
      bool status = true;
      for (k=0; k<faceids->GetNumberOfIds(); k++)
        {
        if(PtIds->IsId(faceids->GetId(k)) == -1)
          {
          status = false;
          break;
          }
        }
      if(status)      
        {
        return i;
        }
      }
    }
  return -1;
}
//---------------------------------------------------------------------------------------------
int vtkMimxCreateElementSetWidgetFEMesh::GetFaceNumGivenCellNumFaceIdsSurface(
  int CellNum, vtkIdList *PtIds, vtkPolyData *aSurface, vtkMimxCreateElementSetWidgetFEMesh *Self)
{
  int i,j,k;
  vtkUnstructuredGrid *bbox = vtkUnstructuredGrid::New();
  vtkDataArray *dataarray = Self->InputVolume->GetFieldData()->GetArray("Mesh_Seed");
  bbox->GetCellData()->AddArray(dataarray);
  bbox->Allocate(dataarray->GetNumberOfTuples(), dataarray->GetNumberOfTuples());
  for(i=0; i< dataarray->GetNumberOfTuples(); i++)
    {
    vtkIdList *idlist = vtkIdList::New();
    switch (this->MeshType)
      {
      case VTK_HEXAHEDRON:
        idlist->SetNumberOfIds(8);
        bbox->InsertNextCell(VTK_HEXAHEDRON, idlist);
        break;
      case VTK_TETRA: 
        idlist->SetNumberOfIds(4);
        bbox->InsertNextCell(VTK_TETRA, idlist);
        break;
      case VTK_QUAD:
        idlist->SetNumberOfIds(4);
        bbox->InsertNextCell(VTK_QUAD, idlist);
        break;
      case VTK_TRIANGLE:
        idlist->SetNumberOfIds(3);
        bbox->InsertNextCell(VTK_TRIANGLE, idlist);
        break;
      default:
        idlist->SetNumberOfIds(8);
        bbox->InsertNextCell(VTK_HEXAHEDRON, idlist);
        break;
      }
    idlist->SetNumberOfIds(8);
    bbox->InsertNextCell(12, idlist);
    idlist->Delete();
    }
  vtkMimxUnstructuredToStructuredGrid *utosgrid = vtkMimxUnstructuredToStructuredGrid::New();
  utosgrid->SetInput(Self->InputVolume);
  utosgrid->SetBoundingBox(bbox);
  utosgrid->SetStructuredGridNum(CellNum);
  utosgrid->Update();
  vtkStructuredGrid *sgrid = utosgrid->GetOutput();
  bbox->Delete();
  for (i=0; i<6; i++)
    {
    vtkMimxExtractStructuredGridFace *exface =
      vtkMimxExtractStructuredGridFace::New();
    exface->SetInput(sgrid);
    exface->SetFaceNum(i);
    exface->Update();
    vtkStructuredGrid *facesgrid = exface->GetOutput();
    vtkPointLocator *localPointLocator = vtkPointLocator::New();
    vtkPoints *localPoints = vtkPoints::New();
    localPointLocator->InitPointInsertion(localPoints, Self->InputVolume->GetBounds());
    for (j=0; j<facesgrid->GetNumberOfPoints(); j++)
      localPointLocator->InsertNextPoint(facesgrid->GetPoint(j));

    // check if the points list from pointids match
    bool status = true;
    for (k=0; k<PtIds->GetNumberOfIds(); k++)
      {
      int location = localPointLocator->IsInsertedPoint(
        Self->InputVolume->GetPoint(PtIds->GetId(k)));
      if(location == -1)
        {
        status = false;
        break;
        }
      }
    localPointLocator->Delete();
    localPoints->Delete();

    if(status)
      {
      //vtkStructuredGridWriter *writer = vtkStructuredGridWriter::New();
      //writer->SetFileName("Structured.vtk");
      //writer->SetInput(exface->GetOutput());
      //writer->Write();
      //writer->Delete();
      vtkStructuredGridGeometryFilter *fil = vtkStructuredGridGeometryFilter::New();
      fil->SetInput(exface->GetOutput());
      fil->Update();
      aSurface->DeepCopy(fil->GetOutput());
      fil->Delete();
      exface->Delete();
      utosgrid->Delete();
      return i;
      }
    exface->Delete();
    }
  utosgrid->Delete();
  return -1;
}
//----------------------------------------------------------------------------------------------
