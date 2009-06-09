/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxSelectCellsWidgetFEMesh.cxx,v $
Language:  C++

Date:      $Date: 2008/08/10 00:48:34 $
Version:   $Revision: 1.12 $


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
#include "vtkMimxSelectCellsWidgetFEMesh.h"

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
#include "vtkGlyph3D.h"
#include "vtkHandleWidget.h"
#include "vtkIdFilter.h"
#include "vtkIdList.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInteractorObserver.h"
#include "vtkInteractorStyleRubberBandPick.h"
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
#include "vtkSphereSource.h"
#include "vtkVisibleCellSelector.h"
#include "vtkWidgetCallbackMapper.h" 
#include "vtkWidgetEvent.h"
#include "vtkWidgetEventTranslator.h"
#include "vtkCellPicker.h"


#include "vtkPolyDataWriter.h"

vtkCxxRevisionMacro(vtkMimxSelectCellsWidgetFEMesh, "$Revision: 1.12 $");
vtkStandardNewMacro(vtkMimxSelectCellsWidgetFEMesh);

//----------------------------------------------------------------------
vtkMimxSelectCellsWidgetFEMesh::vtkMimxSelectCellsWidgetFEMesh()
{
  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::LeftButtonPressEvent,
    vtkEvent::ControlModifier, 0, 1, NULL,
    vtkMimxSelectCellsWidgetFEMesh::CrtlLeftMouseButtonDown,
    this, vtkMimxSelectCellsWidgetFEMesh::CrtlLeftButtonDownCallback);

  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::MouseMoveEvent,
    vtkEvent::ControlModifier, 0, 1, NULL,
    vtkMimxSelectCellsWidgetFEMesh::CrtlLeftMouseButtonMove,
    this, vtkMimxSelectCellsWidgetFEMesh::CrtlMouseMoveCallback);

  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::LeftButtonReleaseEvent,
    vtkEvent::ControlModifier, 0, 1, NULL,
    vtkMimxSelectCellsWidgetFEMesh::CrtlLeftMouseButtonUp,
    this, vtkMimxSelectCellsWidgetFEMesh::CrtlLeftButtonUpCallback);

  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                          vtkMimxSelectCellsWidgetFEMesh::LeftMouseButtonUp,
                                          this, vtkMimxSelectCellsWidgetFEMesh::LeftButtonUpCallback);

  this->RubberBandStyle =  vtkInteractorStyleRubberBandPick::New();
  this->AreaPicker = vtkRenderedAreaPicker::New();
  this->Input = vtkUnstructuredGrid::New();
  this->InputActor = NULL;
  this->WidgetEvent = vtkMimxSelectCellsWidgetFEMesh::Start;
  this->Mapper = NULL;
  this->SelectedCellIds = vtkIdList::New();
  this->SelectedPointIds = vtkIdList::New();
  this->PickX0 = -1;    this->PickY0 = -1;      this->PickX1 = -1;      this->PickY1 = -1;
  this->PickStatus = 0;
  this->CellSelectionState = 3;
  this->ExtractedVolumeActor = NULL;
  this->ExtractedSurfaceActor = NULL;
  this->SingleCellActor = NULL;
  this->MeshType = MeshHexahedron;
}

//----------------------------------------------------------------------
vtkMimxSelectCellsWidgetFEMesh::~vtkMimxSelectCellsWidgetFEMesh()
{
  this->RubberBandStyle->Delete();
  this->AreaPicker->Delete();
  if(this->Mapper)
    this->Mapper->Delete();

  if(this->SelectedCellIds)
    this->SelectedCellIds->Delete();

  if(this->InputActor)
    {
    this->InputActor->Delete();
    }
  if(this->ExtractedVolumeActor)
    {
    this->ExtractedVolumeActor->Delete();
    }
  if(this->ExtractedSurfaceActor)
    this->ExtractedSurfaceActor->Delete();
  if(this->SingleCellActor)
    this->SingleCellActor->Delete();
  this->Input->Delete();
}

//----------------------------------------------------------------------
void vtkMimxSelectCellsWidgetFEMesh::SetEnabled(int enabling)
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
    if(!this->Input)
      {
      vtkErrorMacro(<<"Input Should be Set");
      return;
      }
    vtkDataSetMapper *mapper = vtkDataSetMapper::New();
    mapper->SetInput(this->Input);
    if(!this->InputActor)
      this->InputActor = vtkActor::New();
    this->InputActor->SetMapper(mapper);

    int X=this->Interactor->GetEventPosition()[0];
    int Y=this->Interactor->GetEventPosition()[1];
  
    if ( ! this->CurrentRenderer )
      {
      this->SetCurrentRenderer(this->Interactor->FindPokedRenderer(X,Y));
  
      if (this->CurrentRenderer == NULL)
        {
        return;
        }
      this->CurrentRenderer->AddActor(this->InputActor);
      this->InputActor->PickableOn();
      mapper->Delete();

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
      this->InputActor->Delete();
      this->InputActor = NULL;
      }
    if(this->ExtractedSurfaceActor)
      {
      this->CurrentRenderer->RemoveActor(this->ExtractedSurfaceActor);
      this->ExtractedSurfaceActor->Delete();
      this->ExtractedSurfaceActor = NULL;
      }
    if(this->ExtractedVolumeActor)
      {
      this->CurrentRenderer->RemoveActor(this->ExtractedVolumeActor);
      this->ExtractedVolumeActor->Delete();
      this->ExtractedVolumeActor = NULL;
      }
    if(this->SingleCellActor)
      {
      this->CurrentRenderer->RemoveActor(this->SingleCellActor);
      this->SingleCellActor->Delete();
      this->SingleCellActor = NULL;
      }

    this->SelectedCellIds->Initialize();
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
void vtkMimxSelectCellsWidgetFEMesh::CrtlLeftButtonDownCallback(vtkAbstractWidget *w)
{
  vtkMimxSelectCellsWidgetFEMesh *self = reinterpret_cast<vtkMimxSelectCellsWidgetFEMesh*>(w);
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
    self->WidgetEvent = vtkMimxSelectCellsWidgetFEMesh::Outside;
    return;
    }

  self->WidgetEvent = vtkMimxSelectCellsWidgetFEMesh::CrtlLeftMouseButtonDown;

  self->RubberBandStyle->GetInteractor()->SetKeyCode('r');
  self->RubberBandStyle->OnChar();
  self->RubberBandStyle->OnLeftButtonDown();
  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  self->PickStatus = 1;
  self->Interactor->Render();
}
//----------------------------------------------------------------------
void vtkMimxSelectCellsWidgetFEMesh::CrtlLeftButtonUpCallback(vtkAbstractWidget *w)
{
  vtkMimxSelectCellsWidgetFEMesh *self = reinterpret_cast<vtkMimxSelectCellsWidgetFEMesh*>(w);
  int *size;
  size = self->Interactor->GetRenderWindow()->GetSize();

  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // cout <<X<<"  "<<Y<<endl;

  self->PickX1 = X;
  self->PickY1 = Y;

  if ( self->WidgetEvent == vtkMimxSelectCellsWidgetFEMesh::Outside ||
       self->WidgetEvent == vtkMimxSelectCellsWidgetFEMesh::Start )
    {
    return;
    }
  if(self->SingleCellActor)
    self->CurrentRenderer->RemoveActor(self->SingleCellActor);
  if(self->ExtractedVolumeActor)
    self->CurrentRenderer->RemoveActor(self->ExtractedVolumeActor);
  if(self->ExtractedSurfaceActor)
    self->CurrentRenderer->RemoveActor(self->ExtractedSurfaceActor);

  if(self->WidgetEvent == vtkMimxSelectCellsWidgetFEMesh::CrtlLeftMouseButtonDown)
    {
    self->WidgetEvent = vtkMimxSelectCellsWidgetFEMesh::Start;

    if(self->CellSelectionState != vtkMimxSelectCellsWidgetFEMesh::SelectSingleCell)
      {
      if(self->CellSelectionState == vtkMimxSelectCellsWidgetFEMesh::SelectVisibleCellsOnSurface)
        {
        vtkMimxSelectCellsWidgetFEMesh::SelectVisibleCellsOnSurfaceFunction(self);
        }
      if(self->CellSelectionState == vtkMimxSelectCellsWidgetFEMesh::SelectCellsOnSurface)
        {
        vtkMimxSelectCellsWidgetFEMesh::SelectCellsOnSurfaceFunction(self);
        }
      if(self->CellSelectionState == vtkMimxSelectCellsWidgetFEMesh::SelectCellsThrough)
        {
        vtkMimxSelectCellsWidgetFEMesh::SelectCellsThroughFunction(self);
        }
      if(self->CellSelectionState == vtkMimxSelectCellsWidgetFEMesh::ExtractSurface)
        {
        vtkMimxSelectCellsWidgetFEMesh::ExtractSurfaceFunction(self);
        }
      if(self->CellSelectionState == vtkMimxSelectCellsWidgetFEMesh::ExtractVisibleSurface)
        {
        vtkMimxSelectCellsWidgetFEMesh::ExtractVisibleSurfaceFunction(self);
        }
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
void vtkMimxSelectCellsWidgetFEMesh::CrtlMouseMoveCallback(vtkAbstractWidget *w)
{
  vtkMimxSelectCellsWidgetFEMesh *self = reinterpret_cast<vtkMimxSelectCellsWidgetFEMesh*>(w);
  if(self->CellSelectionState == vtkMimxSelectCellsWidgetFEMesh::SelectSingleCell)
    {
    self->SelectSingleCellFunction(self);
    }
  else
    {
    self->RubberBandStyle->OnMouseMove();
    }
}
//---------------------------------------------------------------------
void vtkMimxSelectCellsWidgetFEMesh::ComputeSelectedPointIds(
  vtkDataSet *ExtractedUGrid, vtkMimxSelectCellsWidgetFEMesh *self)
{
  int i;
  vtkPoints *polypoints;
  if(ExtractedUGrid->GetDataObjectType() == VTK_POLY_DATA)
    polypoints = vtkPolyData::SafeDownCast(ExtractedUGrid)->GetPoints();
  else
    polypoints = vtkUnstructuredGrid::SafeDownCast(ExtractedUGrid)->GetPoints();

  vtkPoints *inputpoints = self->Input->GetPoints();

  vtkPoints *newpts = vtkPoints::New();
  vtkPointLocator *locator = vtkPointLocator::New();
  locator->InitPointInsertion(newpts, self->Input->GetBounds());

  for (i=0; i< inputpoints->GetNumberOfPoints(); i++)
    {
    locator->InsertNextPoint(inputpoints->GetPoint(i));
    }
  if(self->SelectedPointIds)
    {
    self->SelectedPointIds->Initialize();
    }
  else{
  self->SelectedPointIds = vtkIdList::New();
  }

  for (i=0; i<polypoints->GetNumberOfPoints(); i++)
    {
    int location = locator->IsInsertedPoint(polypoints->GetPoint(i));
    if(location == -1)
      {
      vtkErrorMacro("Point sets do not match");
      locator->Delete();
      self->SelectedPointIds->Initialize();
      return;
      }
    else
      {
      self->SelectedPointIds->InsertNextId(location);
      }
    }
  newpts->Delete();
  locator->Delete();
}
//----------------------------------------------------------------------
void vtkMimxSelectCellsWidgetFEMesh::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);  
}
//-----------------------------------------------------------------------
void vtkMimxSelectCellsWidgetFEMesh::SelectVisibleCellsOnSurfaceFunction(
  vtkMimxSelectCellsWidgetFEMesh *self)
{
  int i;
  vtkGeometryFilter *fil = vtkGeometryFilter::New();
  fil->SetInput(self->Input);
  fil->Update();

  vtkPolyDataMapper *filmapper = vtkPolyDataMapper::New();
  vtkActor *filactor = vtkActor::New();
  filmapper->SetInput(fil->GetOutput());
  filactor->SetMapper(filmapper);
  filactor->PickableOn();
  self->CurrentRenderer->AddActor(filactor);
  if(self->InputActor)
    self->CurrentRenderer->RemoveActor(self->InputActor);
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
  if(extr->GetOutput()->GetNumberOfCells() < 1)
    {
    self->CurrentRenderer->RemoveActor(filactor);
    extr->Delete();
    fil->Delete();
    filmapper->Delete();
    filactor->Delete();
    select->Delete();
    res->Delete();
    self->CurrentRenderer->AddActor(self->InputActor);
    self->CurrentRenderer->Render();
    return;
    }
  self->ComputeSelectedCellIds(self, extr->GetOutput());
  select->Delete();
  res->Delete();
  extr->Delete();
  filmapper->Delete();
  fil->Delete();
  self->CurrentRenderer->RemoveActor(filactor);
  filactor->Delete();

  vtkSelection *selection = vtkSelection::New();
  vtkExtractSelection *ext = vtkExtractSelection::New();
  ext->SetInput(0, self->Input);
  ext->SetInput(1, selection);

  vtkIdTypeArray *globalids = vtkIdTypeArray::New();
  globalids->SetNumberOfComponents(1);
  globalids->SetName("GIds");
//      globalids->SetNumberOfTuples(self->Input->GetNumberOfCells());
  for (i=0; i<self->Input->GetNumberOfCells(); i++)
    {
    globalids->InsertNextValue(i);
    }
  self->Input->GetCellData()->AddArray(globalids);
  self->Input->GetCellData()->SetGlobalIds(globalids);

#if ( (VTK_MAJOR_VERSION >= 6) || ( VTK_MAJOR_VERSION == 5 && VTK_MINOR_VERSION >= 4 ) )
  selection->RemoveAllNodes();
  vtkSelectionNode *node = vtkSelectionNode::New();
  selection->AddNode(node);
  node->Delete();
  node->GetProperties()->Set(
    vtkSelectionNode::CONTENT_TYPE(), vtkSelectionNode::GLOBALIDS);
#else
  selection->Clear();
  selection->GetProperties()->Set(
    vtkSelection::CONTENT_TYPE(), vtkSelection::GLOBALIDS);
#endif
  vtkIdTypeArray *cellIds = vtkIdTypeArray::New();
  cellIds->SetNumberOfComponents(1);
  cellIds->SetNumberOfTuples(self->SelectedCellIds->GetNumberOfIds());
  for (i=0; i<self->SelectedCellIds->GetNumberOfIds(); i++)
    {
    cellIds->SetTuple1(i, self->SelectedCellIds->GetId(i));
    }
#if ( (VTK_MAJOR_VERSION >= 6) || ( VTK_MAJOR_VERSION == 5 && VTK_MINOR_VERSION >= 4 ) )
  selection->GetNode(0)->SetSelectionList(cellIds);
#else
  selection->SetSelectionList(cellIds);
#endif
  cellIds->Delete();
        
  ext->Update();

  self->Input->GetCellData()->RemoveArray("GIds");
  globalids->Delete();
  if(self->ExtractedVolumeActor)
    {
    self->CurrentRenderer->RemoveActor(self->ExtractedVolumeActor);
    self->ExtractedVolumeActor->Delete();
    }
  self->ExtractedVolumeActor = vtkActor::New();
  vtkDataSetMapper *mapper = vtkDataSetMapper::New();
  vtkUnstructuredGrid *UGrid = vtkUnstructuredGrid::SafeDownCast(ext->GetOutput());

  mapper->SetInput(UGrid);
  self->ExtractedVolumeActor->SetMapper(mapper);
  mapper->Delete();
  if(self->InputActor)
    self->CurrentRenderer->AddActor(self->InputActor);
  self->CurrentRenderer->AddActor(self->ExtractedVolumeActor);
  self->ExtractedVolumeActor->GetProperty()->SetColor(1.0, 0.0,0.0);
  self->ExtractedVolumeActor->GetProperty()->SetRepresentationToWireframe();
  selection->Delete();
  ext->Delete();
}
//-----------------------------------------------------------------------------
void vtkMimxSelectCellsWidgetFEMesh::SelectCellsOnSurfaceFunction(
  vtkMimxSelectCellsWidgetFEMesh *self)
{
  int i;
  vtkGeometryFilter *fil = vtkGeometryFilter::New();
  fil->SetInput(self->Input);
  fil->Update();

  vtkPolyDataMapper *filmapper = vtkPolyDataMapper::New();
  vtkActor *filactor = vtkActor::New();
  filmapper->SetInput(fil->GetOutput());
  filactor->SetMapper(filmapper);
  filactor->PickableOn();
  self->CurrentRenderer->AddActor(filactor);
  if(self->InputActor)
    self->CurrentRenderer->RemoveActor(self->InputActor);
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
  vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::SafeDownCast(
    Extract->GetOutput());
  if(ugrid->GetNumberOfCells() <1)
    {
    self->CurrentRenderer->RemoveActor(filactor);
    Extract->Delete();
    fil->Delete();
    filmapper->Delete();
    filactor->Delete();
    self->CurrentRenderer->AddActor(self->InputActor);
    self->CurrentRenderer->Render();
    return;
    }

  self->ComputeSelectedPointIds(ugrid, self);
  self->ComputeSelectedCellIds(self, ugrid);
  Extract->Delete();
  filmapper->Delete();
  fil->Delete();
  self->CurrentRenderer->RemoveActor(filactor);
  filactor->Delete();

  vtkSelection *selection = vtkSelection::New();
  vtkExtractSelection *ext = vtkExtractSelection::New();
  ext->SetInput(0, self->Input);
  ext->SetInput(1, selection);

  vtkIdTypeArray *globalids = vtkIdTypeArray::New();
  globalids->SetNumberOfComponents(1);
  globalids->SetName("GIds");
  //      globalids->SetNumberOfTuples(self->Input->GetNumberOfCells());
  for (i=0; i<self->Input->GetNumberOfCells(); i++)
    {
    globalids->InsertNextValue(i);
    }
  self->Input->GetCellData()->AddArray(globalids);
  self->Input->GetCellData()->SetGlobalIds(globalids);
#if ( (VTK_MAJOR_VERSION >= 6) || ( VTK_MAJOR_VERSION == 5 && VTK_MINOR_VERSION >= 4 ) )
  selection->RemoveAllNodes();
  vtkSelectionNode *node = vtkSelectionNode::New();
  selection->AddNode(node);
  node->Delete();
  node->GetProperties()->Set(
    vtkSelectionNode::CONTENT_TYPE(), vtkSelectionNode::GLOBALIDS);
#else
  selection->Clear();
  selection->GetProperties()->Set(
    vtkSelection::CONTENT_TYPE(), vtkSelection::GLOBALIDS);
#endif
  vtkIdTypeArray *cellIds = vtkIdTypeArray::New();
  cellIds->SetNumberOfComponents(1);
  cellIds->SetNumberOfTuples(self->SelectedCellIds->GetNumberOfIds());
  for (i=0; i<self->SelectedCellIds->GetNumberOfIds(); i++)
    {
    cellIds->SetTuple1(i, self->SelectedCellIds->GetId(i));
    }
#if ( (VTK_MAJOR_VERSION >= 6) || ( VTK_MAJOR_VERSION == 5 && VTK_MINOR_VERSION >= 4 ) )
  selection->GetNode(0)->SetSelectionList(cellIds);
#else
  selection->SetSelectionList(cellIds);
#endif
  cellIds->Delete();

  ext->Update();
  vtkUnstructuredGrid *extugrid = vtkUnstructuredGrid::SafeDownCast(ext->GetOutput());    
        
  self->Input->GetCellData()->RemoveArray("GIds");
  globalids->Delete();
  if(self->ExtractedVolumeActor)
    {
    self->CurrentRenderer->RemoveActor(self->ExtractedVolumeActor);
    self->ExtractedVolumeActor->Delete();
    }
  self->ExtractedVolumeActor = vtkActor::New();
  vtkDataSetMapper *mapper = vtkDataSetMapper::New();
  mapper->SetInput(extugrid);
  self->ExtractedVolumeActor->SetMapper(mapper);
  mapper->Delete();
  if(self->InputActor)
    self->CurrentRenderer->AddActor(self->InputActor);
  self->CurrentRenderer->AddActor(self->ExtractedVolumeActor);
  self->ExtractedVolumeActor->GetProperty()->SetColor(1.0, 0.0,0.0);
  self->ExtractedVolumeActor->GetProperty()->SetLineWidth(2);
  self->ExtractedVolumeActor->GetProperty()->SetRepresentationToWireframe();
  selection->Delete();

  //vtkUnstructuredGridWriter *writer = vtkUnstructuredGridWriter::New();
  //writer->SetInput(ext->GetOutput());
  //writer->SetFileName("Extract.vtk");
  //writer->Write();
  //writer->Delete();

  ext->Delete();
}
//-----------------------------------------------------------------------------
void vtkMimxSelectCellsWidgetFEMesh::SelectCellsThroughFunction(
  vtkMimxSelectCellsWidgetFEMesh *self)
{
  int i;
  double x0 = self->PickX0;
  double y0 = self->PickY0;
  double x1 = self->PickX1;
  double y1 = self->PickY1;
  self->AreaPicker->AreaPick(static_cast<int>(x0), static_cast<int>(y0), 
                             static_cast<int>(x1), static_cast<int>(y1), self->CurrentRenderer);
  vtkExtractSelectedFrustum *Extract = vtkExtractSelectedFrustum::New();
  Extract->SetInput(self->Input);
//      Extract->PassThroughOff();
  Extract->SetFrustum(self->AreaPicker->GetFrustum());
  Extract->Update();
  vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::SafeDownCast(
    Extract->GetOutput());
  if(ugrid->GetNumberOfCells() <1)
    {
    Extract->Delete();
    return;
    }
  vtkIdTypeArray *idarray = vtkIdTypeArray::SafeDownCast(
    ugrid->GetCellData()->GetArray("vtkOriginalCellIds"));
  if(idarray)
    {
    self->SelectedCellIds->Initialize();
    self->SelectedCellIds->SetNumberOfIds(idarray->GetNumberOfTuples());
    for(i=0; i<idarray->GetNumberOfTuples(); i++)
      {
      self->SelectedCellIds->SetId(i, idarray->GetValue(i));
      }
    }
  if(self->ExtractedVolumeActor)
    {
    self->CurrentRenderer->RemoveActor(self->ExtractedVolumeActor);
    self->ExtractedVolumeActor->Delete();
    }
  self->ExtractedVolumeActor = vtkActor::New();
  vtkDataSetMapper *mapper = vtkDataSetMapper::New();
  mapper->SetInput(ugrid);
  self->ExtractedVolumeActor->SetMapper(mapper);
  mapper->Delete();
  if(self->InputActor)
    self->CurrentRenderer->AddActor(self->InputActor);
  self->CurrentRenderer->AddActor(self->ExtractedVolumeActor);
  self->ExtractedVolumeActor->GetProperty()->SetColor(1.0, 0.0,0.0);
  self->ExtractedVolumeActor->GetProperty()->SetRepresentationToWireframe();
  self->ExtractedVolumeActor->GetProperty()->SetLineWidth(2);
}
//-----------------------------------------------------------------------------
void vtkMimxSelectCellsWidgetFEMesh::SelectSingleCellFunction(
  vtkMimxSelectCellsWidgetFEMesh *self)
{
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // Okay, we can process this. Try to pick handles first;
  // if no handles picked, then pick the bounding box.
  if (!self->CurrentRenderer || !self->CurrentRenderer->IsInViewport(X, Y))
    {
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
      self->SelectedCellIds->Initialize();
      self->SelectedCellIds->SetNumberOfIds(1);
      self->SelectedCellIds->SetId(0, PickedCell);
      if(self->SingleCellActor)
        {
        self->CurrentRenderer->RemoveActor(self->SingleCellActor);      
        self->SingleCellActor->Delete();
        }
      self->SingleCellActor = vtkActor::New();
      vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
      vtkIdList *ptids = vtkIdList::New();
      self->Input->GetCellPoints(PickedCell,ptids);
      vtkPoints *points = vtkPoints::New();
      for (i=0; i<ptids->GetNumberOfIds(); i++)
        {
        points->InsertNextPoint(self->Input->GetPoint(ptids->GetId(i)));
        ptids->SetId(i,i);
        }
      ugrid->Allocate(1,1);
      ugrid->InsertNextCell(self->Input->GetCellType(PickedCell), ptids);
      ugrid->SetPoints(points);
      points->Delete();
      vtkDataSetMapper *mapper = vtkDataSetMapper::New();
      mapper->SetInput(ugrid);
      ugrid->Delete();
      self->SingleCellActor->SetMapper(mapper);
      mapper->Delete();
      ptids->Delete();
      self->SingleCellActor->GetProperty()->SetColor(1.0,0.66,0.33);
      self->SingleCellActor->GetProperty()->SetLineWidth(
        self->InputActor->GetProperty()->GetLineWidth()*2.0);
      self->SingleCellActor->GetProperty()->SetRepresentationToWireframe();
      self->CurrentRenderer->AddActor(self->SingleCellActor); 
      }
    }
  self->Interactor->Render();
  CellPicker->Delete();
}
//--------------------------------------------------------------------------------------
void vtkMimxSelectCellsWidgetFEMesh::ComputeSelectedCellIds(
  vtkMimxSelectCellsWidgetFEMesh *Self, vtkDataSet *DataSet)
{
  Self->ComputeSelectedPointIds(DataSet, Self);
  if(DataSet->GetDataObjectType() == VTK_POLY_DATA)
    {
    int i, CellNum;
    vtkPolyData *PolyData = vtkPolyData::SafeDownCast(DataSet);
    Self->SelectedCellIds->Initialize();
    for (i=0; i<PolyData->GetNumberOfCells(); i++)
      {
      vtkIdList *PtIds = PolyData->GetCell(i)->GetPointIds();
      CellNum = Self->ComputeOriginalCellNumber(Self, PtIds);
      if(CellNum == -1)
        {
        Self->SelectedCellIds->Initialize();
        vtkErrorMacro("Cells chosen do not belong to the original set");
        return;
        }
      else
        {
        Self->SelectedCellIds->InsertUniqueId(CellNum);
        }
      }       
    }
  else{
  int i, j, CellNum;
  vtkUnstructuredGrid *UGrid = vtkUnstructuredGrid::SafeDownCast(DataSet);
  Self->SelectedCellIds->Initialize();
  for (i=0; i<UGrid->GetNumberOfCells(); i++)
    {
    vtkIdList *PtIds = UGrid->GetCell(i)->GetPointIds();
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
      Self->SelectedCellIds->Initialize();
      vtkErrorMacro("Cells chosen do not belong to the original set");
      return;
      }
    else
      {
      Self->SelectedCellIds->InsertUniqueId(CellNum);
      }
    }       

  }
}
//---------------------------------------------------------------------------------------
int vtkMimxSelectCellsWidgetFEMesh::ComputeOriginalCellNumber(
  vtkMimxSelectCellsWidgetFEMesh *self, vtkIdList *PtIds)
{
  int i,j, k;
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
  return -1;
}
//----------------------------------------------------------------------------------------
void vtkMimxSelectCellsWidgetFEMesh::ExtractVisibleSurfaceFunction(vtkMimxSelectCellsWidgetFEMesh *self)
{
  vtkGeometryFilter *fil = vtkGeometryFilter::New();
  fil->SetInput(self->Input);
  fil->Update();

  vtkCleanPolyData *clean = vtkCleanPolyData::New();
  clean->SetInput(fil->GetOutput());
  clean->Update();
  vtkPolyDataMapper *cleanpolydatamapper = vtkPolyDataMapper::New();
  vtkActor *cleanactor = vtkActor::New();
  cleanpolydatamapper->SetInput(clean->GetOutput());
  cleanactor->SetMapper(cleanpolydatamapper);
  cleanactor->PickableOn();
  self->CurrentRenderer->AddActor(cleanactor);
  if(self->InputActor)
    self->CurrentRenderer->RemoveActor(self->InputActor);
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

  if(self->ExtractedSurfaceActor)
    {
    self->CurrentRenderer->RemoveActor(self->ExtractedSurfaceActor);
    self->ExtractedSurfaceActor->Delete();
    }
#if ( (VTK_MAJOR_VERSION >= 6) || ( VTK_MAJOR_VERSION == 5 && VTK_MINOR_VERSION >= 4 ) )
  vtkSelectionNode *cellids = res->GetNode(0);
#else
  vtkSelection *cellids = res->GetChild(0);
#endif
  vtkExtractSelectedPolyDataIds *extr = vtkExtractSelectedPolyDataIds::New();
  if (cellids)
    {
    extr->SetInput(0, clean->GetOutput());
#if ( (VTK_MAJOR_VERSION >= 6) || ( VTK_MAJOR_VERSION == 5 && VTK_MINOR_VERSION >= 4 ) )
    extr->SetInput(1, res);
#else
    extr->SetInput(1, cellids);
#endif
    extr->Update();
    self->ExtractedSurfaceActor = vtkActor::New();
    vtkPolyDataMapper *mapper = vtkPolyDataMapper::New();
    mapper->SetInput(extr->GetOutput());
    self->ExtractedSurfaceActor->SetMapper(mapper);
    mapper->Delete();
    }
  select->Delete();
  res->Delete();
  extr->Delete();
  fil->Delete();
  clean->Delete();
  cleanpolydatamapper->Delete();
  if(self->ExtractedSurfaceActor)
    {
    self->ExtractedSurfaceActor->GetProperty()->SetColor(1.0, 0.0,0.0);
    self->ExtractedSurfaceActor->GetProperty()->SetRepresentationToWireframe();
    self->ExtractedSurfaceActor->GetProperty()->SetLineWidth(2);
    self->CurrentRenderer->AddActor(self->ExtractedSurfaceActor);
    }
  if(self->InputActor)
    self->CurrentRenderer->AddActor(self->InputActor);
  self->CurrentRenderer->RemoveActor(cleanactor);
  cleanactor->Delete();
}
//----------------------------------------------------------------------------------------
void vtkMimxSelectCellsWidgetFEMesh::ExtractSurfaceFunction(vtkMimxSelectCellsWidgetFEMesh *self)
{
  vtkGeometryFilter *fil = vtkGeometryFilter::New();
  fil->SetInput(self->Input);
  fil->Update();

  vtkCleanPolyData *clean = vtkCleanPolyData::New();
  clean->SetInput(fil->GetOutput());
  clean->Update();
  vtkPolyDataMapper *cleanpolydatamapper = vtkPolyDataMapper::New();
  vtkActor *cleanactor = vtkActor::New();
  cleanpolydatamapper->SetInput(clean->GetOutput());
  cleanactor->SetMapper(cleanpolydatamapper);
  cleanactor->PickableOn();
  self->CurrentRenderer->AddActor(cleanactor);
  if(self->InputActor)
    self->CurrentRenderer->RemoveActor(self->InputActor);
  self->CurrentRenderer->Render();

  double x0 = self->PickX0;
  double y0 = self->PickY0;
  double x1 = self->PickX1;
  double y1 = self->PickY1;

  self->AreaPicker->AreaPick(static_cast<int>(x0), static_cast<int>(y0), 
                             static_cast<int>(x1), static_cast<int>(y1), self->CurrentRenderer);
  vtkExtractSelectedFrustum *Extract = vtkExtractSelectedFrustum::New();
  Extract->SetInput(clean->GetOutput());
//      Extract->PassThroughOff();
  Extract->SetFrustum(self->AreaPicker->GetFrustum());
  Extract->Update();
  vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::SafeDownCast(
    Extract->GetOutput());  
  vtkGeometryFilter *fil1 = vtkGeometryFilter::New();
  fil1->SetInput(ugrid);
  fil1->Update();
  vtkCleanPolyData *clean1 = vtkCleanPolyData::New();
  clean1->SetInput(fil1->GetOutput());
  clean1->Update();
  //vtkPolyDataWriter *writer = vtkPolyDataWriter::New();
  //writer->SetInput(fil1->GetOutput());
  //writer->SetFileName("Extract.vtk");
  //writer->Write();
  //writer->Delete();
  if(self->ExtractedSurfaceActor)
    {
    self->CurrentRenderer->RemoveActor(self->ExtractedSurfaceActor);
    self->ExtractedSurfaceActor->Delete();
    }
  self->ExtractedSurfaceActor = vtkActor::New();
  vtkPolyDataMapper *mapper = vtkPolyDataMapper::New();
  mapper->SetInput(clean1->GetOutput());
  self->ExtractedSurfaceActor->SetMapper(mapper);
  mapper->Delete();
  if(self->InputActor)
    self->CurrentRenderer->AddActor(self->InputActor);
  self->CurrentRenderer->AddActor(self->ExtractedSurfaceActor);
  self->CurrentRenderer->RemoveActor(cleanactor);
  self->ExtractedSurfaceActor->GetProperty()->SetColor(1.0, 0.0,0.0);
  self->ExtractedSurfaceActor->GetProperty()->SetRepresentationToWireframe();
  self->ExtractedSurfaceActor->GetProperty()->SetLineWidth(2);
  fil1->Delete();
  Extract->Delete();
  cleanactor->Delete();
  cleanpolydatamapper->Delete();
  clean->Delete();
  fil->Delete();
  clean1->Delete();
}
//-------------------------------------------------------------------------------------------
void vtkMimxSelectCellsWidgetFEMesh::LeftButtonUpCallback(vtkAbstractWidget* w)
{
  vtkMimxSelectCellsWidgetFEMesh *Self = 
    reinterpret_cast<vtkMimxSelectCellsWidgetFEMesh*>(w);
  if(Self->PickStatus)
    {
    Self->CrtlLeftButtonUpCallback(w);
    }
}
//-------------------------------------------------------------------------------------------
void vtkMimxSelectCellsWidgetFEMesh::SetInput(vtkUnstructuredGrid *Ugrid)
{
  this->Input->DeepCopy(Ugrid);
  this->Input->GetCellData()->Initialize();
  //std::cout  << "Set Input" <<std::endl;
  /* Update mesh Type */
  //for (int i=0;i<Ugrid->GetNumberOfCells();i++)
  //{
  //  std::cout << "Cell Type : " << Ugrid->GetCellType(i) << std::endl;
  //}
}
//---------------------------------------------------------------------------------------------
