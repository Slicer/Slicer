/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxSelectPointsWidget.cxx,v $
Language:  C++
Date:      $Date: 2008/08/14 05:01:52 $
Version:   $Revision: 1.18 $

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
#include "vtkMimxSelectPointsWidget.h"

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
#include "vtkExtractSelectedPolyDataIds.h"
#include "vtkEvent.h"
#include "vtkGarbageCollector.h"
#include "vtkGeometryFilter.h"
#include "vtkGlyph3D.h"
#include "vtkHandleWidget.h"
#include "vtkIdFilter.h"
#include "vtkIdList.h"
#include "vtkIntArray.h"
#include "vtkInteractorObserver.h"
#include "vtkInteractorStyleRubberBandPick.h"
#include "vtkMath.h"
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
#include "vtkSphereSource.h"
#include "vtkStructuredGrid.h"
#include "vtkVisibleCellSelector.h"
#include "vtkSelectVisiblePoints.h"
#include "vtkWidgetCallbackMapper.h" 
#include "vtkWidgetEvent.h"
#include "vtkWidgetEventTranslator.h"
#include "vtkPointPicker.h"
#include "vtkPointLocator.h"
#include "vtkFieldData.h"
#include "vtkPointData.h"
#include "vtkSelectVisiblePoints.h"
#include "vtkCellPicker.h"
#include "vtkExtractCells.h"

//#include "vtkStructuredGridWriter.h"
#include "vtkPolyDataWriter.h"

#include "vtkMimxUnstructuredToStructuredGrid.h"
#include "vtkMimxExtractStructuredGridFace.h"

vtkCxxRevisionMacro(vtkMimxSelectPointsWidget, "$Revision: 1.18 $");
vtkStandardNewMacro(vtkMimxSelectPointsWidget);

//----------------------------------------------------------------------
vtkMimxSelectPointsWidget::vtkMimxSelectPointsWidget()
{
  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::LeftButtonPressEvent,
    vtkEvent::ControlModifier, 0, 1, NULL,
    vtkMimxSelectPointsWidget::CrtlLeftMouseButtonDown,
    this, vtkMimxSelectPointsWidget::CrtlLeftButtonDownCallback);

  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::MouseMoveEvent,
    vtkEvent::ControlModifier, 0, 1, NULL,
    vtkMimxSelectPointsWidget::CrtlLeftMouseButtonMove,
    this, vtkMimxSelectPointsWidget::CrtlMouseMoveCallback);

  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::LeftButtonReleaseEvent,
    vtkEvent::ControlModifier, 0, 1, NULL,
    vtkMimxSelectPointsWidget::CrtlLeftMouseButtonUp,
    this, vtkMimxSelectPointsWidget::CrtlLeftButtonUpCallback);

  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::LeftButtonPressEvent,
    vtkEvent::ShiftModifier, 0, 1, NULL,
    vtkMimxSelectPointsWidget::ShiftLeftMouseButtonDown,
    this, vtkMimxSelectPointsWidget::ShiftLeftButtonDownCallback);

  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::LeftButtonReleaseEvent,
    vtkEvent::ShiftModifier, 0, 1, NULL,
    vtkMimxSelectPointsWidget::ShiftLeftMouseButtonUp,
    this, vtkMimxSelectPointsWidget::ShiftLeftButtonUpCallback);

  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::MouseMoveEvent,
    vtkEvent::ShiftModifier, 0, 1, NULL,
    vtkMimxSelectPointsWidget::ShiftLeftMouseButtonMove,
    this, vtkMimxSelectPointsWidget::ShiftMouseMoveCallback);

  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                          vtkMimxSelectPointsWidget::LeftMouseButtonUp,
                                          this, vtkMimxSelectPointsWidget::LeftButtonUpCallback);

  this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonReleaseEvent,
                                          vtkMimxSelectPointsWidget::RightMouseButtonUp,
                                          this, vtkMimxSelectPointsWidget::RightButtonUpCallback);

  this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonPressEvent,
                                          vtkMimxSelectPointsWidget::RightMouseButtonDown,
                                          this, vtkMimxSelectPointsWidget::RightButtonDownCallback);

  this->WidgetEvent = vtkMimxSelectPointsWidget::Start;

  this->Input = vtkUnstructuredGrid::New();
  this->InputMapper = vtkDataSetMapper::New();
  this->InputMapper->SetInput(this->Input);
  this->InputActor = vtkActor::New();
  this->InputActor->SetMapper(this->InputMapper);

  this->SelectedPointSet =  vtkUnstructuredGrid::New();
  this->SelectedSphere =  vtkSphereSource::New();
  this->SelectedSphere->SetThetaResolution(6);
  this->SelectedSphere->SetPhiResolution(6);
  this->SelectedGlyph = vtkGlyph3D::New();
  this->SelectedGlyph->SetSource(this->SelectedSphere->GetOutput());
  this->SelectedGlyph->SetInput(this->SelectedPointSet);
  this->SelectedGlyphMapper = vtkPolyDataMapper::New();
  this->SelectedGlyphMapper->SetInput(this->SelectedGlyph->GetOutput());
  this->SelectedGlyphActor = vtkActor::New();
  this->SelectedGlyphActor->SetMapper(this->SelectedGlyphMapper);
  this->SelectedGlyphActor->GetProperty()->SetColor(1.0, 0.0, 0.0);

  this->ExtractedPointSet =  vtkUnstructuredGrid::New();
  this->ExtractedSphere =  vtkSphereSource::New();
  this->ExtractedSphere->SetThetaResolution(6);
  this->ExtractedSphere->SetPhiResolution(6);
  this->ExtractedGlyph = vtkGlyph3D::New();
  this->ExtractedGlyph->SetSource(this->ExtractedSphere->GetOutput());
  this->ExtractedGlyph->SetInput(this->ExtractedPointSet);
  this->ExtractedGlyphMapper = vtkPolyDataMapper::New();
  this->ExtractedGlyphMapper->SetInput(this->ExtractedGlyph->GetOutput());
  this->ExtractedGlyphActor = vtkActor::New();
  this->ExtractedGlyphActor->SetMapper(this->ExtractedGlyphMapper);
  this->ExtractedGlyphActor->GetProperty()->SetColor(0.0, 1.0, 0.0);

  this->SingleSphere = vtkSphereSource::New();
  this->SingleSphereMapper = vtkPolyDataMapper::New();
  this->SingleSphereMapper->SetInput(this->SingleSphere->GetOutput());
  this->SingleSphereActor = vtkActor::New();
  this->SingleSphereActor->SetMapper(this->SingleSphereMapper);
  this->SingleSphereActor->GetProperty()->SetColor(0.0, 1.0, 0.0);

  this->RubberBandStyle =  vtkInteractorStyleRubberBandPick::New();
  this->AreaPicker = vtkRenderedAreaPicker::New();

  this->SelectedPointIds = vtkIdList::New();
  this->ExtractedPointIds = vtkIdList::New();

  this->SurfaceActor = vtkActor::New();
  this->SurfaceMapper = vtkPolyDataMapper::New();
  this->SurfaceActor->SetMapper(this->SurfaceMapper);
  this->PickX0 = -1;    this->PickY0 = -1;      this->PickX1 = -1;      this->PickY1 = -1;
  this->PointSelectionState = 4;
  this->PickStatus = 0;

  this->BooleanState = vtkMimxSelectPointsWidget::Add;
  this->EditMeshState = vtkMimxSelectPointsWidget::Full;
  this->InputSurfaceFilter = vtkGeometryFilter::New();
  this->InputCleanPolyData = vtkCleanPolyData::New();
  this->InputCleanPolyData->SetInput(this->InputSurfaceFilter->GetOutput());
  this->PointLocator = vtkPointLocator::New();
  this->LocatorPoints = vtkPoints::New();
}

//----------------------------------------------------------------------
vtkMimxSelectPointsWidget::~vtkMimxSelectPointsWidget()
{
  this->Input->Delete();
  this->InputMapper->Delete();
                
  this->InputActor->Delete();

  this->SelectedPointSet->Delete();
  this->SelectedSphere->Delete();
  this->SelectedGlyph->Delete();
  this->SelectedGlyphMapper->Delete();
  this->SelectedGlyphActor->Delete();

  this->ExtractedPointSet->Delete();
  this->ExtractedSphere->Delete();
  this->ExtractedGlyph->Delete();
  this->ExtractedGlyphMapper->Delete();
  this->ExtractedGlyphActor->Delete();

  this->RubberBandStyle->Delete();
  this->AreaPicker->Delete();

  this->SelectedPointIds->Delete();
  this->ExtractedPointIds->Delete();
  this->SurfaceActor->Delete();
  this->SurfaceMapper->Delete();
  this->InputSurfaceFilter->Delete();
  this->InputCleanPolyData->Delete();
  this->PointLocator->Delete();
  this->LocatorPoints->Delete();
}

//----------------------------------------------------------------------
void vtkMimxSelectPointsWidget::SetEnabled(int enabling)
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

    int X=this->Interactor->GetEventPosition()[0];
    int Y=this->Interactor->GetEventPosition()[1];
  
    if ( ! this->CurrentRenderer )
      {
      this->SetCurrentRenderer(this->Interactor->FindPokedRenderer(X,Y));
  
      if (this->CurrentRenderer == NULL)
        {
        return;
        }
      }
    this->SelectedPointSet->Initialize();
    this->ExtractedPointSet->Initialize();
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
    this->CurrentRenderer->AddActor(this->InputActor);

    if ( !this->Interactor )
      {
      vtkErrorMacro(<<"The interactor must be set prior to enabling/disabling widget");
      return;
      }

    this->Interactor->SetInteractorStyle(this->RubberBandStyle);

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
        
    this->CurrentRenderer->RemoveActor(this->SelectedGlyphActor);
    this->CurrentRenderer->RemoveActor(this->ExtractedGlyphActor);
    this->CurrentRenderer->RemoveActor(this->SingleSphereActor);
    this->CurrentRenderer->RemoveActor(this->InputActor);
    this->CurrentRenderer->RemoveActor(this->SurfaceActor);
    this->ExtractedPointIds->Initialize();
    this->SelectedPointIds->Initialize();
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
void vtkMimxSelectPointsWidget::CrtlLeftButtonDownCallback(vtkAbstractWidget *w)
{
  vtkMimxSelectPointsWidget *self = reinterpret_cast<vtkMimxSelectPointsWidget*>(w);
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
    self->WidgetEvent = vtkMimxSelectPointsWidget::Outside;
    return;
    }

  self->WidgetEvent = vtkMimxSelectPointsWidget::CrtlLeftMouseButtonDown;

  self->RubberBandStyle->GetInteractor()->SetKeyCode('r');
  self->RubberBandStyle->OnChar();
  self->RubberBandStyle->OnLeftButtonDown();
  // Okay, we can process this. Try to pick handles first;
  // if no handles picked, then try to pick the line.
  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  self->PickStatus = 1;
  self->CurrentRenderer->RemoveActor(self->SingleSphereActor);
  self->Interactor->Render();
}
//----------------------------------------------------------------------
void vtkMimxSelectPointsWidget::CrtlLeftButtonUpCallback(vtkAbstractWidget *w)
{
  vtkMimxSelectPointsWidget *self = reinterpret_cast<vtkMimxSelectPointsWidget*>(w);
  int *size;
  size = self->Interactor->GetRenderWindow()->GetSize();

  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // cout <<X<<"  "<<Y<<endl;

  self->PickX1 = X;
  self->PickY1 = Y;

  if ( self->WidgetEvent == vtkMimxSelectPointsWidget::Outside ||
       self->WidgetEvent == vtkMimxSelectPointsWidget::Start )
    {
    return;
    }

  if(self->WidgetEvent == vtkMimxSelectPointsWidget::CrtlLeftMouseButtonDown)
    {
    self->WidgetEvent = vtkMimxSelectPointsWidget::Start;

    //if(self->PointSelectionState != vtkMimxSelectPointsWidget::SelectSinglePoint )
    //{
    if(self->PointSelectionState == vtkMimxSelectPointsWidget::SelectVisiblePointsOnSurface)
      {
      vtkMimxSelectPointsWidget::SelectVisiblePointsOnSurfaceFunction(self);
      }
    if(self->PointSelectionState == vtkMimxSelectPointsWidget::SelectPointsOnSurface)
      {
      vtkMimxSelectPointsWidget::SelectPointsOnSurfaceFunction(self);
      }
    if(self->PointSelectionState == vtkMimxSelectPointsWidget::SelectPointsThrough)
      {
      vtkMimxSelectPointsWidget::SelectPointsThroughFunction(self);
      }
    if(self->PointSelectionState == vtkMimxSelectPointsWidget::SelectPointsBelongingToAFace)
      {
      vtkMimxSelectPointsWidget::SelectPointsOnAFaceFunction(self);
      self->CurrentRenderer->RemoveActor(self->SingleSphereActor);
      }
    if(self->PointSelectionState == vtkMimxSelectPointsWidget::SelectSinglePoint)
      {
      self->SelectSinglePointFunction(self);
      }
    //}
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
void vtkMimxSelectPointsWidget::CrtlMouseMoveCallback(vtkAbstractWidget *w)
{
  vtkMimxSelectPointsWidget *self = reinterpret_cast<vtkMimxSelectPointsWidget*>(w);
  /*if(self->PointSelectionState == vtkMimxSelectPointsWidget::SelectSinglePoint)
  {
        self->SelectSinglePointFunction(self);
  }
  else */if(self->PointSelectionState == vtkMimxSelectPointsWidget::SelectPointsBelongingToAFace)
    {
    self->SelectSinglePointBelongingToAFaceFunction(self);
    }
  else
    {
    if(self->PointSelectionState != vtkMimxSelectPointsWidget::SelectSinglePoint)
      self->RubberBandStyle->OnMouseMove();
    }
}
//---------------------------------------------------------------------
void vtkMimxSelectPointsWidget::ComputeSelectedPointIds(
  vtkDataSet *ExtractedUGrid, vtkMimxSelectPointsWidget *self)
{
  int i;
  vtkPoints *polypoints = NULL;
  if(ExtractedUGrid->GetDataObjectType() == VTK_POLY_DATA)
    polypoints = vtkPolyData::SafeDownCast(ExtractedUGrid)->GetPoints();

  if (ExtractedUGrid->GetDataObjectType() == VTK_STRUCTURED_GRID)
    polypoints = vtkStructuredGrid::SafeDownCast(ExtractedUGrid)->GetPoints();

  if (ExtractedUGrid->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
    polypoints = vtkUnstructuredGrid::SafeDownCast(ExtractedUGrid)->GetPoints();

  vtkPoints *inputpoints = self->Input->GetPoints();
    
  vtkPoints *newpts = vtkPoints::New();
  vtkPointLocator *locator = vtkPointLocator::New();
  locator->InitPointInsertion(newpts, self->Input->GetBounds());
        
  for (i=0; i< inputpoints->GetNumberOfPoints(); i++)
    {
    locator->InsertNextPoint(inputpoints->GetPoint(i));
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
void vtkMimxSelectPointsWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);  
}
//-----------------------------------------------------------------------
void vtkMimxSelectPointsWidget::SelectVisiblePointsOnSurfaceFunction(
  vtkMimxSelectPointsWidget *self)
{
  if(self->EditMeshState == vtkMimxSelectPointsWidget::Partial)   return;

  double x0 = self->PickX0;
  double y0 = self->PickY0;
  double x1 = self->PickX1;
  double y1 = self->PickY1;

  self->CurrentRenderer->AddActor(self->InputActor);
  self->CurrentRenderer->Render();

  vtkSelectVisiblePoints *select = vtkSelectVisiblePoints::New();
  select->SetInput(self->SurfaceMapper->GetInput());
  select->SetRenderer(self->CurrentRenderer);
  select->SelectionWindowOn();
  int xmin, ymin, xmax, ymax;
  if(x0 <= x1)
    {
    xmin = static_cast<int>(x0);
    xmax = static_cast<int>(x1);
    }
  else
    {
    xmin = static_cast<int>(x1);
    xmax = static_cast<int>(x0);
    }

  if(y0 <= y1)
    {
    ymin = static_cast<int>(y0);
    ymax = static_cast<int>(y1);
    }
  else
    {
    ymin = static_cast<int>(y1);
    ymax = static_cast<int>(y0);
    }
  //select->SetSelection(static_cast<int>(x0),static_cast<int>(x1),
  //      static_cast<int>(y1),static_cast<int>(y0));
  select->SetSelection( xmin, xmax, ymin, ymax);
  select->Update();

  //vtkPolyDataWriter *writer = vtkPolyDataWriter::New();
  //writer->SetInput(select->GetOutput());
  //writer->SetFileName("Extract.vtk");
  //writer->Write();
  //writer->Delete();
  //vtkVisibleCellSelector *select = vtkVisibleCellSelector::New();
  //select->SetRenderer(self->CurrentRenderer);

  //double x0 = self->PickX0;
  //double y0 = self->PickY0;
  //double x1 = self->PickX1;
  //double y1 = self->PickY1;

  //select->SetRenderPasses(0,1,0,1,1);
  //select->SetArea(static_cast<int>(x0),static_cast<int>(y1),static_cast<int>(x1),
  //      static_cast<int>(y0));
  //select->Select();

  //vtkSelection *res = vtkSelection::New();
  //select->GetSelectedIds(res);

  //vtkSelection *cellids = res->GetChild(0);
  //vtkExtractSelectedPolyDataIds *extr = vtkExtractSelectedPolyDataIds::New();
  //if (cellids)
  //{
  //      extr->SetInput(0, clean->GetOutput());
  //      extr->SetInput(1, cellids);
  //      extr->Update();
  //}
  //vtkCleanPolyData *cleanextr = vtkCleanPolyData::New();
  //cleanextr->SetInput(extr->GetOutput());
  //cleanextr->Update();


  //self->AreaPicker->AreaPick(static_cast<int>(x0), static_cast<int>(y0), 
  //      static_cast<int>(x1), static_cast<int>(y1), self->CurrentRenderer);

  //vtkPointSet *pointSet = vtkUnstructuredGrid::New();
  //pointSet->SetPoints(cleanextr->GetOutput()->GetPoints());

  //vtkExtractSelectedFrustum *Extract = vtkExtractSelectedFrustum::New();
  //Extract->SetInput(select->GetOutput());
  //Extract->SetFrustum(self->AreaPicker->GetFrustum());
  //Extract->Update();

  //vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::SafeDownCast(
  //      Extract->GetOutput());
  //vtkPolyDataWriter *writer = vtkPolyDataWriter::New();
  //writer->SetInput(extr->GetOutput());
  //writer->SetFileName("Extract.vtk");
  //writer->Write();
  //writer->Delete();

  self->UpdateExtractedGlyphDisplay(select->GetOutput(), self);
   
  //Extract->Delete();
  select->Delete();
  self->CurrentRenderer->Render();
}
//-----------------------------------------------------------------------------
void vtkMimxSelectPointsWidget::SelectPointsOnSurfaceFunction(
  vtkMimxSelectPointsWidget *self)
{
  if(self->EditMeshState == vtkMimxSelectPointsWidget::Partial)   return;

  double x0 = self->PickX0;
  double y0 = self->PickY0;
  double x1 = self->PickX1;
  double y1 = self->PickY1;
  self->AreaPicker->AreaPick(static_cast<int>(x0), static_cast<int>(y0), 
                             static_cast<int>(x1), static_cast<int>(y1), self->CurrentRenderer);

  vtkPointSet *pointSet = vtkUnstructuredGrid::New();
  pointSet->SetPoints(self->SurfaceMapper->GetInput()->GetPoints());
  vtkExtractSelectedFrustum *Extract = vtkExtractSelectedFrustum::New();
  Extract->SetInput(pointSet);
//      Extract->PassThroughOff();
  Extract->SetFrustum(self->AreaPicker->GetFrustum());
  Extract->Update();
  vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::SafeDownCast(
    Extract->GetOutput());
  self->UpdateExtractedGlyphDisplay(ugrid, self);

  pointSet->Delete();
  Extract->Delete();
}
//-----------------------------------------------------------------------------
void vtkMimxSelectPointsWidget::SelectPointsThroughFunction(
  vtkMimxSelectPointsWidget *self)
{
  double x0 = self->PickX0;
  double y0 = self->PickY0;
  double x1 = self->PickX1;
  double y1 = self->PickY1;

  self->AreaPicker->AreaPick(static_cast<int>(x0), static_cast<int>(y0), 
                             static_cast<int>(x1), static_cast<int>(y1), self->CurrentRenderer);

  vtkPointSet *pointSet = vtkUnstructuredGrid::New();
  pointSet->SetPoints(self->Input->GetPoints());
  pointSet->GetPointData()->AddArray(
    self->Input->GetPointData()->GetArray("mimx_Original_Point_Ids"));

  vtkExtractSelectedFrustum *Extract = vtkExtractSelectedFrustum::New();
  Extract->SetInput(pointSet);
  Extract->SetFrustum(self->AreaPicker->GetFrustum());
  Extract->Update();

  vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::SafeDownCast(
    Extract->GetOutput());
  self->UpdateExtractedGlyphDisplay(ugrid, self);

  pointSet->Delete();
  Extract->Delete();
}
//-----------------------------------------------------------------------------
void vtkMimxSelectPointsWidget::SelectSinglePointFunction(
  vtkMimxSelectPointsWidget *self)
{
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // Okay, we can process this. Try to pick handles first;
  // if no handles picked, then pick the bounding box.
  if (!self->CurrentRenderer || !self->CurrentRenderer->IsInViewport(X, Y))
    {
    return;
    }

  vtkAssemblyPath *path;
  vtkPointPicker *PointPicker = vtkPointPicker::New();
  PointPicker->SetTolerance(0.01);
  if(self->EditMeshState == vtkMimxSelectPointsWidget::Full)
    PointPicker->AddPickList(self->SurfaceActor);
  else
    PointPicker->AddPickList(self->SelectedGlyphActor);
  PointPicker->PickFromListOn();
  PointPicker->Pick(X,Y,0.0,self->CurrentRenderer);
  path = PointPicker->GetPath();
  if ( path != NULL )
    {
    vtkIdType PickedPoint = PointPicker->GetPointId();
    if(PickedPoint != -1)
      {
      vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
      vtkPoints *points = vtkPoints::New();
      points->SetNumberOfPoints(0);
                        
      if(self->EditMeshState == vtkMimxSelectPointsWidget::Full)
        {
        vtkIntArray *intarray = vtkIntArray::SafeDownCast(
          self->SurfaceMapper->GetInput()
          ->GetPointData()->GetArray("mimx_Original_Point_Ids"));

        if(intarray)
          {
          points->InsertNextPoint(self->Input->GetPoint(
                                    intarray->GetValue(PickedPoint)));
          }
        }
      else
        {
        vtkPointLocator *locator = vtkPointLocator::New();
        locator->SetDataSet(self->SelectedPointSet);
        locator->BuildLocator();
        int poinum = locator->FindClosestPoint(self->SelectedGlyphActor->
                                               GetMapper()->GetInput()->GetPoint(PickedPoint));
        if(poinum != -1)
          {
          points->InsertNextPoint(self->SelectedPointSet->GetPoint(poinum));
          }
        }
      ugrid->SetPoints(points);
      self->UpdateExtractedGlyphDisplay(ugrid, self);
      ugrid->Delete();
      points->Delete();
      }
    }
  self->Interactor->Render();
  PointPicker->Delete();
}
//--------------------------------------------------------------------------------------
void vtkMimxSelectPointsWidget::ComputeSelectedPointIds(vtkMimxSelectPointsWidget *self)
{

  int i;

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

  int location = locator->IsInsertedPoint(
    self->SingleSphere->GetCenter());
  if(location == -1)
    {
    vtkErrorMacro("Invalid Sphere Location");
    locator->Delete();
    self->SelectedPointIds->Initialize();
    return;
    }
  else
    {
    self->SelectedPointIds->InsertNextId(location);
    }
  newpts->Delete();
  locator->Delete();
}
//----------------------------------------------------------------------------------------
void vtkMimxSelectPointsWidget::ShiftLeftButtonUpCallback(vtkAbstractWidget *vtkNotUsed(w))
{
  //vtkMimxSelectPointsWidget *self = reinterpret_cast<vtkMimxSelectPointsWidget*>(w);

  //if ( self->WidgetEvent == vtkMimxSelectPointsWidget::Outside ||
  //      self->WidgetEvent == vtkMimxSelectPointsWidget::Start )
  //{
  //      return;
  //}

  //if(self->WidgetEvent == vtkMimxSelectPointsWidget::ShiftLeftMouseButtonDown)
  //{
  //      self->WidgetEvent = vtkMimxSelectPointsWidget::Start;

  //              if(self->GlyphActor)
  //              {
  //                      self->CurrentRenderer->RemoveActor(self->GlyphActor);
  //                      self->GlyphActor->Delete(); 
  //              }
  //              self->GlyphActor = vtkActor::New();

  //              if(self->PointSelectionState == vtkMimxSelectPointsWidget::SelectPointsBelongingToAFace)
  //              {
  //                      vtkMimxSelectPointsWidget::SelectPointsOnAFaceFunction(self);
  //              }
  //      }
  //self->EndInteraction();
  //self->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
  //self->Interactor->Render();
}
//----------------------------------------------------------------------
void vtkMimxSelectPointsWidget::ShiftMouseMoveCallback(vtkAbstractWidget *vtkNotUsed(w))
{
  //vtkMimxSelectPointsWidget *self = reinterpret_cast<vtkMimxSelectPointsWidget*>(w);
  //if(self->PointSelectionState == vtkMimxSelectPointsWidget::SelectSinglePoint ||
  //      self->PointSelectionState == vtkMimxSelectPointsWidget::SelectPointsBelongingToAFace)
  //{
  //      self->SelectSinglePointFunction(self);
  //}
}
//---------------------------------------------------------------------
void vtkMimxSelectPointsWidget::SelectPointsOnAFaceFunction(
  vtkMimxSelectPointsWidget *self)
{
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // Okay, we can process this. Try to pick handles first;
  // if no handles picked, then pick the bounding box.
  if (!self->CurrentRenderer || !self->CurrentRenderer->IsInViewport(X, Y))
    {
    return;
    }

  if(self->EditMeshState == vtkMimxSelectPointsWidget::Partial)   return;

  if(!self->Input->GetFieldData()->GetArray("Mesh_Seed"))
    {
//              vtkErrorMacro("Mesh Seed data should be present for the input mesh");
    return ;
    }
  vtkIntArray *bboxArray = vtkIntArray::SafeDownCast(
    self->Input->GetCellData()->GetArray("Bounding_Box_Number"));
  vtkIntArray *faceArray = vtkIntArray::SafeDownCast(
    self->Input->GetCellData()->GetArray("Face_Numbers"));

  if(bboxArray && faceArray)
    {
    int location = self->PointLocator->IsInsertedPoint(self->SingleSphere->GetCenter());
    if(location == -1)      return;
    vtkIdList *cellIds = vtkIdList::New();
    self->Input->GetPointCells(location, cellIds);
    if(!cellIds->GetNumberOfIds())
      {
      cellIds->Delete();
      return;
      }
    int selectedCell = cellIds->GetId(0);

    int bboxNum = bboxArray->GetValue(selectedCell);
    int faceNum = faceArray->GetValue(selectedCell);
                
    cellIds->Initialize();
    int i;
    for (i=0; i<self->Input->GetNumberOfCells(); i++)
      {
      int currBboxNum = bboxArray->GetValue(i);
      int cuurFaceNum = faceArray->GetValue(i);
      if(bboxNum == currBboxNum && faceNum == cuurFaceNum)
        {
        cellIds->InsertNextId(i);
        }
      }
    vtkExtractCells *exCells = vtkExtractCells::New();
    exCells->SetInput(self->Input);
    exCells->SetCellList(cellIds);
    exCells->Update();
    self->UpdateExtractedGlyphDisplay(exCells->GetOutput(), self);
    cellIds->Delete();
    exCells->Delete();
    return;
    }
  // create a building block with just the mesh seeds.
  vtkUnstructuredGrid *bblock = vtkUnstructuredGrid::New();
  vtkIntArray *intarray = vtkIntArray::New();
  intarray->SetNumberOfComponents(3);
  vtkIntArray *meshseed = vtkIntArray::SafeDownCast(
    self->Input->GetFieldData()->GetArray("Mesh_Seed"));

  int i,j, dim[3];
  // create a copy of mesh seed values with no junk values.
  for (i=0; i<meshseed->GetNumberOfTuples(); i++)
    {
    meshseed->GetTupleValue(i, dim);
    intarray->InsertNextTupleValue(dim);
    }
  intarray->SetName("Mesh_Seed");
  bblock->GetCellData()->AddArray(intarray);
  bblock->Allocate(intarray->GetNumberOfTuples(), 
                   intarray->GetNumberOfTuples());
  for(i=0; i< intarray->GetNumberOfTuples(); i++)
    {
    vtkIdList *idlist = vtkIdList::New();
    idlist->SetNumberOfIds(8);
    bblock->InsertNextCell(12, idlist);
    idlist->Delete();
    }
  self->ExtractedPointIds->Initialize();
  for (i=0; i<intarray->GetNumberOfTuples();i++)
    {
    vtkMimxUnstructuredToStructuredGrid *utosgrid = 
      vtkMimxUnstructuredToStructuredGrid::New();
    utosgrid->SetInput(self->Input);
    utosgrid->SetBoundingBox(bblock);
    utosgrid->SetStructuredGridNum(i);
    utosgrid->Update();
    vtkStructuredGrid *solid = vtkStructuredGrid::New();
    solid->DeepCopy(utosgrid->GetOutput());
    if(self->CheckGivenPointBelongsToStructuredGrid(utosgrid->GetOutput(), 
                                                    self->SingleSphere->GetCenter(), self->Input->GetBounds()))
      {
      for (j=0; j<6; j++)
        {
        vtkMimxExtractStructuredGridFace *exface =
          vtkMimxExtractStructuredGridFace::New();
        exface->SetInput(utosgrid->GetOutput());
        exface->SetFaceNum(j);
        exface->Update();
        vtkStructuredGrid *exfacegrid = vtkStructuredGrid::New();
        exfacegrid->DeepCopy(exface->GetOutput());
        if(self->CheckGivenPointBelongsToStructuredGrid(exfacegrid, 
                                                        self->SingleSphere->GetCenter(), self->Input->GetBounds()))
          {
          if(self->DoAllPointsLieOnSurface(exfacegrid, 
                                           self->SurfaceMapper->GetInput(), self->Input->GetBounds()))
            {
            vtkStructuredGrid *sgrid = vtkStructuredGrid::New();
            sgrid->DeepCopy(exface->GetOutput());
            self->UpdateExtractedGlyphDisplay(sgrid, self);
            exface->Delete();
            utosgrid->Delete();
            bblock->Delete();
            intarray->Delete();
            sgrid->Delete();
            return;
            }
          }
        exface->Delete();
        exfacegrid->Delete();
        }
      }
    utosgrid->Delete();
    solid->Delete();
    }
  bblock->Delete();
  intarray->Delete();
}
//--------------------------------------------------------------------------------------
int vtkMimxSelectPointsWidget::CheckGivenPointBelongsToStructuredGrid(
  vtkStructuredGrid *SGrid, double x[3], double *bounds)
{
  int i;
  vtkPoints *newpts = vtkPoints::New();
  vtkPointLocator *locator = vtkPointLocator::New();
  locator->InitPointInsertion(newpts, bounds);
        
  for (i=0; i< SGrid->GetNumberOfPoints(); i++)
    {
    locator->InsertNextPoint(SGrid->GetPoint(i));
    }
  int location = locator->IsInsertedPoint(x);
  if(location == -1)
    {
    locator->Initialize();
    locator->Delete();
    newpts->Delete();
    return 0;
    }
  locator->Initialize();
  locator->Delete();
  newpts->Delete();
  return 1;       
}
//--------------------------------------------------------------------------------------
int vtkMimxSelectPointsWidget::DoAllPointsLieOnSurface(
  vtkStructuredGrid *SGrid, vtkPolyData *Surface, double *bounds)
{
  int i;
  vtkPoints *newpts = vtkPoints::New();
  vtkPointLocator *locator = vtkPointLocator::New();
  locator->InitPointInsertion(newpts, bounds);

  for (i=0; i< Surface->GetNumberOfPoints(); i++)
    {
    locator->InsertNextPoint(Surface->GetPoint(i));
    }
  for (i=0; i<SGrid->GetNumberOfPoints(); i++)
    {
    int location = locator->IsInsertedPoint(SGrid->GetPoint(i));
    if(location == -1)
      {
      locator->Initialize();
      locator->Delete();
      newpts->Delete();
      return 0;
      }       
    }
  locator->Initialize();
  locator->Delete();
  newpts->Delete();
  return 1;       
}
//--------------------------------------------------------------------------------------
void vtkMimxSelectPointsWidget::ShiftLeftButtonDownCallback(vtkAbstractWidget *vtkNotUsed(w))
{
  //vtkMimxSelectPointsWidget *self = reinterpret_cast<vtkMimxSelectPointsWidget*>(w);
  //int X = self->Interactor->GetEventPosition()[0];
  //int Y = self->Interactor->GetEventPosition()[1];
  ////    cout <<X<<"  "<<Y<<endl;
  //// Okay, make sure that the pick is in the current renderer
  //if ( !self->CurrentRenderer || !self->CurrentRenderer->IsInViewport(X, Y) )
  //{
  //      self->WidgetEvent = vtkMimxSelectPointsWidget::Outside;
  //      return;
  //}
  //self->WidgetEvent = vtkMimxSelectPointsWidget::ShiftLeftMouseButtonDown;
  //self->EventCallbackCommand->SetAbortFlag(1);
  //self->StartInteraction();
  //self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  //self->Interactor->Render();
}
//----------------------------------------------------------------------
void vtkMimxSelectPointsWidget::LeftButtonUpCallback(vtkAbstractWidget* w)
{
  vtkMimxSelectPointsWidget *Self = 
    reinterpret_cast<vtkMimxSelectPointsWidget*>(w);
  if(Self->PickStatus)
    {
    Self->CrtlLeftButtonUpCallback(w);
    }
}
//-------------------------------------------------------------------------------------------
void vtkMimxSelectPointsWidget::SetSphereRadius(double vtkNotUsed(Radius))
{
  /*this->Sphere->SetRadius(Radius);
        this->Sphere->Update();*/
}
//--------------------------------------------------------------------------------------------
double vtkMimxSelectPointsWidget::GetSphereRadius()
{
  //return this->Sphere->GetRadius();
  return 1.0;
}
//--------------------------------------------------------------------------------------------
double vtkMimxSelectPointsWidget::ComputeSphereRadius(vtkDataSet *DataSet)
{
  double cumdist = 0.0;
  int count = 0;
  double radius = 0.0;
  if(DataSet->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
    {
    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::SafeDownCast(DataSet);
    int i,j;
    for (i=0; i<ugrid->GetNumberOfCells(); i++)
      {
      vtkCell *cell = ugrid->GetCell(i);
      for (j=0; j<cell->GetNumberOfEdges(); j++)
        {
        vtkCell *edge = cell->GetEdge(j);
        vtkIdList *ptids = edge->GetPointIds();
        int pt1 = ptids->GetId(0);
        int pt2 = ptids->GetId(1);
        double p1[3], p2[3];
        ugrid->GetPoint(pt1, p1);       ugrid->GetPoint(pt2, p2);
        cumdist = cumdist + sqrt(vtkMath::Distance2BetweenPoints(p1, p2));
        count ++;
        }
      }
    radius = 0.25*cumdist/count;
    return radius;
    }
  else
    {
    vtkPolyData *polydata = vtkPolyData::SafeDownCast(DataSet);
    int i,j;
    for (i=0; i<polydata->GetNumberOfCells(); i++)
      {
      vtkCell *cell = polydata->GetCell(i);
      for (j=0; j<cell->GetNumberOfEdges(); j++)
        {
        vtkCell *edge = cell->GetEdge(j);
        vtkIdList *ptids = edge->GetPointIds();
        int pt1 = ptids->GetId(0);
        int pt2 = ptids->GetId(1);
        double p1[3], p2[3];
        polydata->GetPoint(pt1, p1);    polydata->GetPoint(pt2, p2);
        cumdist = cumdist + sqrt(vtkMath::Distance2BetweenPoints(p1, p2));
        count ++;
        }
      }
    radius = 0.25*cumdist/count;    
    return radius;
    }
}
//---------------------------------------------------------------------------------------------
void vtkMimxSelectPointsWidget::SetInput(vtkUnstructuredGrid *Ugrid)
{
  this->Input->DeepCopy(Ugrid);
  this->Input->GetCellData()->Initialize();
  if(Ugrid->GetCellData()->GetArray("Bounding_Box_Number"))
    {
    this->Input->GetCellData()->AddArray(
      Ugrid->GetCellData()->GetArray("Bounding_Box_Number"));
    }
  if(Ugrid->GetCellData()->GetArray("Face_Numbers"))
    {
    this->Input->GetCellData()->AddArray(
      Ugrid->GetCellData()->GetArray("Face_Numbers"));
    }
  int i;

  int numPoints = this->Input->GetNumberOfPoints();
  vtkIntArray *pointarray = vtkIntArray::New();
  pointarray->SetNumberOfValues(numPoints);
  this->PointLocator->Initialize();
  this->LocatorPoints->Initialize();
  this->PointLocator->InitPointInsertion(this->LocatorPoints, this->Input->GetBounds());
  for (i=0; i<numPoints; i++)
    {
    this->PointLocator->InsertNextPoint(this->Input->GetPoint(i));
    pointarray->SetValue(i,i);
    }
  pointarray->SetName("mimx_Original_Point_Ids");
  this->Input->GetPointData()->AddArray(pointarray);
  pointarray->Delete();
  vtkGeometryFilter *fil = vtkGeometryFilter::New();
  fil->SetInput(this->Input);
  fil->Update();
  vtkCleanPolyData *clean = vtkCleanPolyData::New();
  clean->SetInput(fil->GetOutput());
  clean->Update();
  this->SurfaceMapper->SetInput(clean->GetOutput());
  this->SurfaceMapper->Modified();
  this->SurfaceActor->PickableOn();
  this->InputActor->PickableOn();
  this->Input->Modified();
  this->ComputeSphereRadius(this->Input);
  this->SelectedSphere->SetRadius(this->GetSphereRadius()/4.0);
  this->ExtractedSphere->SetRadius(this->GetSphereRadius()*1.1/4.0);
  this->SingleSphere->SetRadius(this->GetSphereRadius()*1.5/4.0);
  this->ExtractedPointIds->Initialize();
  this->SelectedPointIds->Initialize();

  fil->Delete();
  clean->Delete();
}
//---------------------------------------------------------------------------------------------
void vtkMimxSelectPointsWidget::UpdateExtractedGlyphDisplay(vtkDataSet *Ugrid, 
                                                            vtkMimxSelectPointsWidget *self)
{
  vtkPoints *points = NULL;
  if(Ugrid->GetDataObjectType() == VTK_POLY_DATA)
    points = vtkPolyData::SafeDownCast(Ugrid)->GetPoints();

  if (Ugrid->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
    points = vtkUnstructuredGrid::SafeDownCast(Ugrid)->GetPoints();

  if (Ugrid->GetDataObjectType() == VTK_STRUCTURED_GRID)
    points = vtkStructuredGrid::SafeDownCast(Ugrid)->GetPoints();
                
  if(Ugrid->GetNumberOfPoints())
    self->ComputeExtractedPointIds(Ugrid, self);
        
  self->ExtractedPointSet->Initialize();
  if(self->EditMeshState == vtkMimxSelectPointsWidget::Full 
     && self->BooleanState == vtkMimxSelectPointsWidget::Add)
    self->ExtractedPointSet->SetPoints(points);
  else
    {
    points = vtkPoints::New();
    int i;
    int numIds = self->ExtractedPointIds->GetNumberOfIds();
    vtkPoints *inputpoints = self->Input->GetPoints();
    for (i=0; i<numIds; i++)
      {
      points->InsertNextPoint(inputpoints->GetPoint(self->ExtractedPointIds->GetId(i)));
      }
    self->ExtractedPointSet->SetPoints(points);
    points->Delete();
    }
  self->ExtractedGlyph->SetInput(self->ExtractedPointSet);
  self->ExtractedGlyph->Modified();
  self->ExtractedGlyph->Update();
  self->CurrentRenderer->AddActor(self->ExtractedGlyphActor);
  self->CurrentRenderer->Render();
}
//----------------------------------------------------------------------------------------------
void vtkMimxSelectPointsWidget::ComputeExtractedPointIds(
  vtkDataSet *ExtractedUGrid, vtkMimxSelectPointsWidget *self)
{
  int i;
  vtkPoints *polypoints = NULL;
  if(ExtractedUGrid->GetDataObjectType() == VTK_POLY_DATA)
    polypoints = vtkPolyData::SafeDownCast(ExtractedUGrid)->GetPoints();

  if (ExtractedUGrid->GetDataObjectType() == VTK_STRUCTURED_GRID)
    polypoints = vtkStructuredGrid::SafeDownCast(ExtractedUGrid)->GetPoints();

  if (ExtractedUGrid->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
    polypoints = vtkUnstructuredGrid::SafeDownCast(ExtractedUGrid)->GetPoints();

  vtkPoints *inputpoints = self->Input->GetPoints();

  vtkPoints *newpts = vtkPoints::New();
  vtkPointLocator *locator = vtkPointLocator::New();
  locator->InitPointInsertion(newpts, self->Input->GetBounds());

  for (i=0; i< inputpoints->GetNumberOfPoints(); i++)
    {
    locator->InsertNextPoint(inputpoints->GetPoint(i));
    }
  self->ExtractedPointIds->Initialize();
//      if(!self->SelectedPointIds->GetNumberOfIds())
  for (i=0; i<polypoints->GetNumberOfPoints(); i++)
    {
    int location = locator->IsInsertedPoint(polypoints->GetPoint(i));
    if(location == -1)
      {
      vtkErrorMacro("Point sets do not match");
      locator->Delete();
      self->ExtractedPointIds->Initialize();
      return;
      }
    else
      {
      if(self->EditMeshState == vtkMimxSelectPointsWidget::Full)
        {
        if(self->BooleanState == vtkMimxSelectPointsWidget::Add)
          {
          if(self->SelectedPointIds->IsId(location) == -1)
            {
            self->ExtractedPointIds->InsertNextId(location);
            }
          }
        else
          {
          //if(self->SelectedPointIds->IsId(location) != -1)
          //{
          self->ExtractedPointIds->InsertNextId(location);
          //}
          }
        }
      else
        {
        if(self->SelectedPointIds->IsId(location) != -1)
          {
          self->ExtractedPointIds->InsertNextId(location);
          }
        }
      }
    }
  newpts->Delete();
  locator->Delete();
}
//------------------------------------------------------------------------------------------
void vtkMimxSelectPointsWidget::RightButtonDownCallback(vtkAbstractWidget* w)
{
  vtkMimxSelectPointsWidget *self = 
    reinterpret_cast<vtkMimxSelectPointsWidget*>(w);

  int *size;
  size = self->Interactor->GetRenderWindow()->GetSize();
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // Okay, make sure that the pick is in the current renderer
  if ( !self->CurrentRenderer || !self->CurrentRenderer->IsInViewport(X, Y) )
    {
    self->WidgetEvent = vtkMimxSelectPointsWidget::Outside;
    self->EventCallbackCommand->SetAbortFlag(1);
    self->EndInteraction();
    self->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
    self->Interactor->Render();
    return;
    }

  //      self->WidgetEvent = vtkMimxSelectPointsWidget::ShiftMouseMove;

  int i;
  vtkAssemblyPath *path;
  vtkCellPicker *CellPicker = vtkCellPicker::New();
  CellPicker->SetTolerance(0.01);
  CellPicker->AddPickList(self->ExtractedGlyphActor);
  CellPicker->PickFromListOn();
  CellPicker->Pick(X,Y,0.0,self->CurrentRenderer);
  path = CellPicker->GetPath();

  if ( path != NULL )
    {
    vtkIdType PickedCell = CellPicker->GetCellId();
    if(PickedCell != -1)
      {
      int numIds = self->ExtractedPointIds->GetNumberOfIds();
      if(!numIds)     return;
      if(self->EditMeshState == vtkMimxSelectPointsWidget::Partial)
        {
        if(self->BooleanState == vtkMimxSelectPointsWidget::Add)
          {
          self->SelectedPointIds->Initialize();
          self->SelectedPointIds->DeepCopy(self->ExtractedPointIds);
          }
        else
          {
          for (i=0; i< numIds; i++)
            {
            self->SelectedPointIds->DeleteId(
              self->ExtractedPointIds->GetId(i));
            }       
          }                               
        }
      else{
      if(self->BooleanState == vtkMimxSelectPointsWidget::Add)
        {
        for (i=0; i< numIds; i++)
          {
          self->SelectedPointIds->InsertUniqueId(
            self->ExtractedPointIds->GetId(i));
          }       
        }
      else
        {
        if(!self->SelectedPointIds->GetNumberOfIds())
          {
          for (i=0; i<self->Input->GetNumberOfPoints();i++)
            {
            self->SelectedPointIds->InsertNextId(i);
            }
          }
        for (i=0; i< numIds; i++)
          {
          self->SelectedPointIds->DeleteId(
            self->ExtractedPointIds->GetId(i));
          }       
        }
      }
      self->UpdateSelectedGlyphDisplay(self);
      }
    }
  CellPicker->Delete();
}
//--------------------------------------------------------------------------------------------------------
void vtkMimxSelectPointsWidget::RightButtonUpCallback(vtkAbstractWidget* vtkNotUsed(w))
{
}
//--------------------------------------------------------------------------------------------------------
void vtkMimxSelectPointsWidget::UpdateSelectedGlyphDisplay( vtkMimxSelectPointsWidget *self)
{
  vtkPoints *points = vtkPoints::New();
  points->SetNumberOfPoints(0);
  int i;
  int numIds = self->SelectedPointIds->GetNumberOfIds();
  for (i=0; i<numIds; i++)
    {
    points->InsertNextPoint(self->Input->GetPoint(self->SelectedPointIds->GetId(i)));
    }
  self->SelectedPointSet->Initialize();
  self->SelectedPointSet->SetPoints(points);
  self->SelectedGlyph->SetInput(self->SelectedPointSet);
  self->SelectedGlyph->Modified();
  self->SelectedGlyph->Update();
  self->CurrentRenderer->AddActor(self->SelectedGlyphActor);
  points->Delete();

  self->ExtractedPointIds->Initialize();
  if(self->ExtractedPointSet->GetPoints())
    self->ExtractedPointSet->GetPoints()->Initialize();
  self->ExtractedGlyph->SetInput(self->ExtractedPointSet);
  self->ExtractedGlyph->Modified();
  self->ExtractedGlyph->Update();

  self->CurrentRenderer->Render();
}
//--------------------------------------------------------------------------------------------------------
void vtkMimxSelectPointsWidget::SetPointSelectionState(int PointState)
{
  this->PointSelectionState = PointState;
  if(this->CurrentRenderer)
    {
    this->CurrentRenderer->RemoveActor(this->SingleSphereActor);
    if(PointState == vtkMimxSelectPointsWidget::SelectPointsBelongingToAFace
       || PointState == vtkMimxSelectPointsWidget::SelectSinglePoint)
      {
      this->CurrentRenderer->RemoveActor(this->InputActor);
      this->CurrentRenderer->AddActor(this->SurfaceActor);
      }
    else
      {
      this->CurrentRenderer->AddActor(this->InputActor);
      this->CurrentRenderer->RemoveActor(this->SurfaceActor);
      }
    this->ExtractedPointIds->Initialize();
    this->ExtractedGlyph->Modified();
    this->ExtractedGlyph->Update();
    this->CurrentRenderer->Render();
    }
}
//---------------------------------------------------------------------------------------------------------
void vtkMimxSelectPointsWidget::SelectSinglePointBelongingToAFaceFunction(
  vtkMimxSelectPointsWidget *self)
{
  if(self->EditMeshState == vtkMimxSelectPointsWidget::Partial)   return;

  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // Okay, we can process this. Try to pick handles first;
  // if no handles picked, then pick the bounding box.
  if (!self->CurrentRenderer || !self->CurrentRenderer->IsInViewport(X, Y))
    {
    return;
    }

  vtkAssemblyPath *path;
  vtkPointPicker *PointPicker = vtkPointPicker::New();
  PointPicker->SetTolerance(0.01);
  PointPicker->AddPickList(self->SurfaceActor);
  PointPicker->PickFromListOn();
  PointPicker->Pick(X,Y,0.0,self->CurrentRenderer);
  path = PointPicker->GetPath();
  if ( path != NULL )
    {
    vtkIdType PickedPoint = PointPicker->GetPointId();
    if(PickedPoint != -1)
      {
      vtkIntArray *intarray = vtkIntArray::SafeDownCast(self->SurfaceMapper->
                                                        GetInput()->GetPointData()->GetArray("mimx_Original_Point_Ids"));

      if(intarray)
        {
        self->SingleSphere->SetCenter(self->Input->GetPoint(
                                        intarray->GetValue(PickedPoint)));
        }
      }
    }
  self->CurrentRenderer->AddActor(self->SingleSphereActor);
  self->Interactor->Render();
  PointPicker->Delete();
}
//-------------------------------------------------------------------------------------------------
void vtkMimxSelectPointsWidget::SetInputOpacity(double Val)
{
  this->InputActor->GetProperty()->SetOpacity(Val);
  this->SurfaceActor->GetProperty()->SetOpacity(Val);
  if(Val == 1.0)
    {
    this->EditMeshState = vtkMimxSelectPointsWidget::Full;
    }
  else
    {
    this->EditMeshState = vtkMimxSelectPointsWidget::Partial;
    }
}
//-------------------------------------------------------------------------------------------------
void vtkMimxSelectPointsWidget::ClearSelections()
{
  if(this->Input)
    {
    this->SelectedPointIds->Initialize();
    this->UpdateSelectedGlyphDisplay(this);
    this->Interactor->Render();
    }
}
//----------------------------------------------------------------------------------------------------
//void vtkMimxSelectPointsWidget::ComputeExtratedPointSet(vtkPoints *Points)
//{
//      this->ExtractedPointSet->Initialize();
//
//      vtkPoints *inputpoints = this->Input->GetPoints();
//
//      vtkPoints *newpts = vtkPoints::New();
//      vtkPointLocator *locator = vtkPointLocator::New();
//      locator->InitPointInsertion(newpts, this->Input->GetBounds());
//      int i;
//      for (i=0; i< inputpoints->GetNumberOfPoints(); i++)
//      {
//              locator->InsertNextPoint(inputpoints->GetPoint(i));
//      }
//      this->ExtractedPointIds->Initialize();
//      for (i=0; i<Points->GetNumberOfPoints(); i++)
//      {
//              int location = locator->IsInsertedPoint(Points->GetPoint(i));
//              if(location == -1)
//              {
//                      vtkErrorMacro("Point sets do not match");
//                      locator->Delete();
//                      this->ExtractedPointIds->Initialize();
//                      return;
//              }
//              else
//              {
//                      self->ExtractedPointIds->InsertNextId(location);
//              }
//      }
//      newpts->Delete();
//      locator->Delete();
//
//      this->ExtractedPointSet->SetPoints(points);
//
//}
////----------------------------------------------------------------------------------------------------
