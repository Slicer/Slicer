/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxUnstructuredGridWidget.cxx,v $
Language:  C++

Date:      $Date: 2008/07/06 20:30:14 $
Version:   $Revision: 1.43 $


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

// ***
#include "vtkRenderWindow.h"

vtkCxxRevisionMacro(vtkMimxUnstructuredGridWidget, "$Revision: 1.43 $");
vtkStandardNewMacro(vtkMimxUnstructuredGridWidget);
//----------------------------------------------------------------------
class vtkUGPWCallback : public vtkCommand
{
public:
  static vtkUGPWCallback *New() 
  { return new vtkUGPWCallback; }
  virtual void Execute(vtkObject *vtkNotUsed(caller), unsigned long, void*)
  {
    double x[3];
    this->PointWidget->GetPosition(x);
    this->UnstructuredGridWidget->SetPoint(this->HandleNumber,x, this->UnstructuredGridWidget);
  }
  vtkUGPWCallback():UnstructuredGridWidget(0),PointWidget(0) {}
  vtkMimxUnstructuredGridWidget  *UnstructuredGridWidget;
  vtkMimxModPointWidget *PointWidget;
  int HandleNumber;
};
//----------------------------------------------------------------------
vtkMimxUnstructuredGridWidget::vtkMimxUnstructuredGridWidget()
{
  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::LeftButtonPressEvent,
    vtkEvent::ShiftModifier, 0, 1, NULL,
    vtkMimxUnstructuredGridWidget::ShiftLeftMouseButtonDown,
    this, vtkMimxUnstructuredGridWidget::ShiftLeftButtonDownCallback);

  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::LeftButtonReleaseEvent,
    vtkEvent::ShiftModifier, 0, 1, NULL,
    vtkMimxUnstructuredGridWidget::ShiftLeftMouseButtonUp,
    this, vtkMimxUnstructuredGridWidget::ShiftLeftButtonUpCallback);

  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
                                          vtkMimxUnstructuredGridWidget::CrtlLeftMouseButtonDown,
                                          this, vtkMimxUnstructuredGridWidget::CrtlLeftButtonDownCallback);

  this->CallbackMapper->SetCallbackMethod(vtkCommand::MouseMoveEvent,
                                          vtkMimxUnstructuredGridWidget::CrtlLeftMouseButtonMove,
                                          this, vtkMimxUnstructuredGridWidget::CrtlMouseMoveCallback);

  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                          vtkMimxUnstructuredGridWidget::CrtlLeftMouseButtonUp,
                                          this, vtkMimxUnstructuredGridWidget::CrtlLeftButtonUpCallback);

  this->CallbackMapper->SetCallbackMethod(vtkCommand::MiddleButtonPressEvent,
                                          vtkMimxUnstructuredGridWidget::CrtlMiddleMouseButtonDown,
                                          this, vtkMimxUnstructuredGridWidget::CrtlMiddleButtonDownCallback);

  this->CallbackMapper->SetCallbackMethod(vtkCommand::MiddleButtonReleaseEvent,
                                          vtkMimxUnstructuredGridWidget::CrtlMiddleMouseButtonUp,
                                          this, vtkMimxUnstructuredGridWidget::CrtlMiddleButtonUpCallback);

  this->State = vtkMimxUnstructuredGridWidget::Start;

  //Build the representation of the widget

  // Control orientation of normals
  this->InsideOut = 0;
  this->OutlineFaceWires = 0;
  this->OutlineCursorWires = 1;

  // Construct the poly data representing the hex
  this->HexMapper = vtkDataSetMapper::New();
  this->HexActor = vtkActor::New();

  // surface of the unstructured grid 
  this->SurfaceMapper = vtkPolyDataMapper::New();
  this->SurfaceActor = vtkActor::New();

  this->HexOutline = vtkActor::New();

  // Set up the initial properties
  this->CreateDefaultProperties();

  // Create the handles
  this->Handle = vtkCollection::New();
  this->HandleMapper = vtkCollection::New();
  this->HandleGeometry = vtkCollection::New();
  this->PointWidget = vtkCollection::New();

  this->FaceGeometry = vtkActorCollection::New();

  this->PWCallback = vtkUGPWCallback::New();
  this->PWCallback->UnstructuredGridWidget = this;

  //Manage the picking stuff
  this->HandlePicker = vtkCellPicker::New();
  this->HandlePicker->SetTolerance(0.001);
  // for the surface
  this->SurfacePicker = vtkCellPicker::New();
  this->SurfacePicker->SetTolerance(0.001);
  this->HexPicker = vtkCellPicker::New();
  this->HexPicker->SetTolerance(0.001);
  this->HexPicker->AddPickList(HexActor);
  this->HexPicker->PickFromListOn();
  this->HandleSize = 1.0;
  this->Enabled = 0;
  this->CurrentHandle = NULL;
  this->UGrid = NULL;
  this->FaceActor = NULL;
  this->FaceMapper = NULL;
  this->CompleteUGrid = NULL;
  this->CurrentPointWidget = NULL;
  this->SelectedCellIds = vtkIdList::New();
  this->SelectedPointIds = vtkIdList::New();
  // for edge operations
  vtkPolyData *polydata = vtkPolyData::New();
  vtkPolyDataMapper *mapper = vtkPolyDataMapper::New();
  mapper->SetInput(polydata);
  polydata->Delete();
  this->EdgeActor = vtkActor::New();
  this->EdgeActor->SetMapper(mapper);
  mapper->Delete();
  this->EdgePicker = vtkCellPicker::New();
  this->EdgePicker->SetTolerance(0.01);
  this->EdgePicker->AddPickList(this->EdgeActor);
  this->EdgeGeometry = NULL;
  this->InteriorFaceList = vtkIdList::New();
  this->InteriorFaceGeometry = vtkActorCollection::New();
}

//----------------------------------------------------------------------
vtkMimxUnstructuredGridWidget::~vtkMimxUnstructuredGridWidget()
{
  this->HexActor->Delete();
  this->HexMapper->Delete();

  this->SurfaceMapper->Delete();
  this->SurfaceActor->Delete();

  this->HexOutline->Delete();

  this->HandleGeometry->Delete();
  this->HandleMapper->Delete();
  this->Handle->Delete();

  this->HandlePicker->Delete();
  this->HexPicker->Delete();

  this->HandleProperty->Delete();
  this->SelectedHandleProperty->Delete();
  this->OutlineProperty->Delete();
  this->SelectedOutlineProperty->Delete();
  if(this->FaceActor)
    {
    this->FaceActor->Delete();
    this->FaceActor = NULL;
    this->FaceMapper->Delete();
    this->FaceMapper = NULL;
    }
  this->SelectedCellIds->Delete();
  this->SelectedPointIds->Delete();
  this->EdgeActor->Delete();
  this->EdgePicker->Delete();
  this->FaceGeometry->Delete();
  if(this->EdgeGeometry)  this->EdgeGeometry->Delete();
  this->InteriorFaceList->Delete();
  this->InteriorFaceGeometry->Delete();
}

//----------------------------------------------------------------------
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
    // Add the outline
    //      this->CurrentRenderer->AddActor(this->HexActor);
    this->CurrentRenderer->AddActor(this->SurfaceActor);
    this->CurrentRenderer->AddActor(this->HexOutline);
    this->HexActor->SetProperty(this->OutlineProperty);
    this->HexOutline->SetProperty(this->OutlineProperty);

    this->HexActor->GetProperty()->SetRepresentationToWireframe();
    this->SurfaceActor->GetProperty()->SetRepresentationToWireframe();

    // turn on the handles
    for (int j=0; j<this->Handle->GetNumberOfItems(); j++)
      {
      this->CurrentRenderer->AddActor((vtkActor*) this->Handle->GetItemAsObject(j));
      ((vtkActor*) (this->Handle->GetItemAsObject(j)))->SetProperty(this->HandleProperty);
      }
    this->PositionHandles(this);
//              this->ComputeAverageHandleSize(this);
    this->SizeHandles(this);
    this->InvokeEvent(vtkCommand::EnableEvent,NULL);
    this->SelectedCellIds->Initialize();
    this->ShowInteriorHighlightedFaces();
    }

  else //disabling-------------------------------------------------------------
    {
    vtkDebugMacro(<<"Disabling widget");
                
    if ( ! this->Enabled ) //already disabled, just return
      {
      return;
      }

    this->RemoveHighlightedFaces(this);
    this->RemoveHighlightedEdges(this);
    this->Enabled = 0;

    // don't listen for events any more
    this->Interactor->RemoveObserver(this->EventCallbackCommand);

    // turn off the outline
    this->CurrentRenderer->RemoveActor(this->HexActor);
    this->CurrentRenderer->RemoveActor(this->SurfaceActor);
    this->CurrentRenderer->RemoveActor(this->HexOutline);
    if (this->EdgeActor)
      {
      this->CurrentRenderer->RemoveActor(this->EdgeActor);
      }
    // turn off the handles
    for (int i=0; i<this->Handle->GetNumberOfItems(); i++)
      {
      this->CurrentRenderer->RemoveActor((vtkActor*)
                                         this->Handle->GetItemAsObject(i));
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
    this->HideInteriorHighlightedFaces();
    this->CurrentHandle = NULL;
    this->InvokeEvent(vtkCommand::DisableEvent,NULL);
    this->CurrentRenderer = NULL;
//              this->UGrid = NULL;
    }

  if ( this->Interactor && !this->Parent )
    {
    this->Interactor->Render();
    }
}
//----------------------------------------------------------------------------------------
void vtkMimxUnstructuredGridWidget::SetEnabled(int enabling, vtkMimxUnstructuredGridWidget *Self)
{ 
  if ( ! Self->Interactor )
    {
    vtkErrorMacro(<<"The interactor must be set prior to enabling/disabling widget");
    return;
    }

  if ( enabling ) //------------------------------------------------------------
    {
    vtkDebugMacro(<<"Enabling widget");

    if ( Self->Enabled || !Self->UGrid) //already enabled, just return
      {
      return;
      }

    if ( ! Self->CurrentRenderer )
      {
      Self->CurrentRenderer = Self->Interactor->FindPokedRenderer(
        Self->Interactor->GetLastEventPosition()[0],
        Self->Interactor->GetLastEventPosition()[1]);
      if (Self->CurrentRenderer == NULL)
        {
        return;
        }
      }

    // We're ready to enable
    Self->Enabled = 1;

    // listen for the events found in the EventTranslator
    if ( ! Self->Parent )
      {
      Self->EventTranslator->AddEventsToInteractor(Self->Interactor,
                                                   Self->EventCallbackCommand,Self->Priority);
      }
    else
      {
      Self->EventTranslator->AddEventsToParent(Self->Parent,
                                               Self->EventCallbackCommand,Self->Priority);
      }


    // Add the various actors
    // Add the outline
    //      Self->CurrentRenderer->AddActor(Self->HexActor);
    Self->CurrentRenderer->AddActor(Self->SurfaceActor);
    Self->CurrentRenderer->AddActor(Self->HexOutline);
    Self->HexActor->SetProperty(Self->OutlineProperty);
    Self->HexOutline->SetProperty(Self->OutlineProperty);

    Self->HexActor->GetProperty()->SetRepresentationToWireframe();
    Self->SurfaceActor->GetProperty()->SetRepresentationToWireframe();

    // turn on the handles
    for (int j=0; j<Self->GetUGrid()->GetNumberOfPoints(); j++)
      {
      Self->CurrentRenderer->AddActor((vtkActor*) Self->Handle->GetItemAsObject(j));
      ((vtkActor*) (Self->Handle->GetItemAsObject(j)))->SetProperty(Self->HandleProperty);
      }
    Self->PositionHandles(Self);
//              Self->ComputeAverageHandleSize(Self);
    Self->SizeHandles(Self);
    Self->InvokeEvent(vtkCommand::EnableEvent,NULL);
    Self->SelectedCellIds->Initialize();
    Self->ShowInteriorHighlightedFaces();
    }

  else //disabling-------------------------------------------------------------
    {
    vtkDebugMacro(<<"Disabling widget");

    if ( ! Self->Enabled ) //already disabled, just return
      {
      return;
      }

    Self->Enabled = 0;

    // don't listen for events any more
    Self->Interactor->RemoveObserver(Self->EventCallbackCommand);

    // turn off the outline
    Self->CurrentRenderer->RemoveActor(Self->HexActor);
    Self->CurrentRenderer->RemoveActor(Self->SurfaceActor);
    Self->CurrentRenderer->RemoveActor(Self->HexOutline);
    if (Self->EdgeActor)
      {
      Self->CurrentRenderer->RemoveActor(Self->EdgeActor);
      }
    // turn off the handles
    for (int i=0; i<Self->GetUGrid()->GetNumberOfPoints(); i++)
      {
      Self->CurrentRenderer->RemoveActor((vtkActor*)
                                         Self->Handle->GetItemAsObject(i));
      }

    Self->Enabled = 0;

    // don't listen for events any more
    if ( ! Self->Parent )
      {
      Self->Interactor->RemoveObserver(Self->EventCallbackCommand);
      }
    else
      {
      Self->Parent->RemoveObserver(Self->EventCallbackCommand);
      }
    Self->HideInteriorHighlightedFaces();

    Self->CurrentHandle = NULL;
    Self->InvokeEvent(vtkCommand::DisableEvent,NULL);
    Self->CurrentRenderer = NULL;
    //              Self->UGrid = NULL;
    }

  if ( Self->Interactor && !Self->Parent )
    {
    Self->Interactor->Render();
    }
}
//----------------------------------------------------------------------------------------
void vtkMimxUnstructuredGridWidget::Execute(vtkMimxUnstructuredGridWidget *Self)
{
  Self->SetEnabled(0, Self);
  if(Self->UGrid)
    {
    Self->Initialize(Self);
    }
  Self->ExtractEdge(Self);

  Self->HexMapper->SetInput(Self->UGrid);
  Self->HexActor->SetMapper(Self->HexMapper);

  vtkGeometryFilter *filter = vtkGeometryFilter::New();
  filter->SetInput(Self->UGrid);
  filter->Update();

  if (!Self->SurfaceMapper)
    {
    Self->SurfaceMapper = vtkPolyDataMapper::New();
    }
  if (!Self->SurfaceActor)
    {
    Self->SurfaceActor = vtkActor::New();
    }

  Self->SurfaceMapper->SetInput(filter->GetOutput());
  Self->SurfaceActor->SetMapper(Self->SurfaceMapper);
  Self->SurfacePicker->AddPickList(SurfaceActor);
  Self->SurfacePicker->PickFromListOn();


  for (int i=0; i<Self->UGrid->GetNumberOfPoints(); i++)
    {
    Self->HandleGeometry->AddItem((vtkObject*) vtkSphereSource::New());
    ((vtkSphereSource*)(Self->HandleGeometry->GetItemAsObject(i)))->SetThetaResolution(8);
    ((vtkSphereSource*)(Self->HandleGeometry->GetItemAsObject(i)))->SetPhiResolution(8);
    Self->HandleMapper->AddItem((vtkObject*) vtkDataSetMapper::New());
    ((vtkDataSetMapper*)(Self->HandleMapper->GetItemAsObject(i)))->
      SetInput(((vtkSphereSource*)(Self->HandleGeometry->GetItemAsObject(i)))->
               GetOutput());
    Self->Handle->AddItem((vtkObject*) vtkActor::New());
    ((vtkActor*) (Self->Handle->GetItemAsObject(i)))->SetMapper
      ((vtkDataSetMapper*) Self->HandleMapper->GetItemAsObject(i));
    ((vtkActor*) (Self->Handle->GetItemAsObject(i)))->GetProperty()->SetColor(1.0,0.0,0.0);
    }
  //      Pointwidget declarations
  for (int i=0; i < Self->UGrid->GetNumberOfPoints(); i++)
    {
    Self->PointWidget->AddItem((vtkObject*) vtkMimxModPointWidget::New());
    ((vtkMimxModPointWidget*) (Self->PointWidget->GetItemAsObject(i)))->AllOff();
    ((vtkMimxModPointWidget*) (Self->PointWidget->GetItemAsObject(i)))
      ->SetHotSpotSize(0.5);
    ((vtkMimxModPointWidget*) (Self->PointWidget->GetItemAsObject(i)))
      ->GetSelectedProperty()->SetColor(0,1,0);
    ((vtkMimxModPointWidget*) (Self->PointWidget->GetItemAsObject(i)))
      ->GetProperty()->SetColor(1,0,0);
    ((vtkMimxModPointWidget*) (Self->PointWidget->GetItemAsObject(i)))
      ->AllOff();
    }

  Self->PWCallback->PointWidget = (vtkMimxModPointWidget*) 
    (Self->PointWidget->GetItemAsObject(0));

  for (int i=0; i < Self->UGrid->GetNumberOfPoints(); i++)
    {
    Self->HandlePicker->AddPickList(
      (vtkActor*) Self->Handle->GetItemAsObject(i));
    }
  Self->HandlePicker->PickFromListOn();

  for(int i=0; i < Self->UGrid->GetNumberOfPoints(); i++)
    {
    ((vtkMimxModPointWidget*) (Self->PointWidget->GetItemAsObject(i)))
      ->AddObserver(vtkCommand::InteractionEvent,Self->PWCallback, 0.0);
    }
  Self->PositionHandles(Self);
  Self->SizeHandles(Self);
  Self->ComputeInteriorHighlightedFaces();
  Self->SetEnabled(1, Self);
  filter->Delete();
}
//--------------------------------------------------------------------------------------
void vtkMimxUnstructuredGridWidget::CrtlLeftButtonDownCallback(vtkAbstractWidget *w)
{
  vtkMimxUnstructuredGridWidget *Self = reinterpret_cast<vtkMimxUnstructuredGridWidget*>(w);

  int X = Self->Interactor->GetEventPosition()[0];
  int Y = Self->Interactor->GetEventPosition()[1];
  //cout << "x,y " << X << " " << Y << endl; 

  // Okay, we can process Self. Try to pick handles first;
  // if no handles picked, then pick the bounding box.
  vtkRenderer *ren = Self->Interactor->FindPokedRenderer(X,Y);
  if ( ren != Self->CurrentRenderer )
    {
    Self->State = vtkMimxUnstructuredGridWidget::Outside;
    Self->RemoveHighlightedFaces(Self);
    Self->RemoveHighlightedEdges(Self);
    Self->ShowInteriorHighlightedFaces();
    return;
    }

  if(!Self->FaceGeometry->GetNumberOfItems())     Self->SelectedCellIds->Initialize();
  Self->RemoveHighlightedEdges(Self);

  // Okay, we can process Self. Try to pick handles first;
  // if no handles picked, then try to pick the line.
  vtkAssemblyPath *path;
  Self->HandlePicker->Pick(X,Y,0.0,Self->CurrentRenderer);
  path = Self->HandlePicker->GetPath();
  if ( path != NULL )
    {
    Self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
    Self->State = vtkMimxUnstructuredGridWidget::MovingHandle;
    Self->HighlightHandle(path->GetFirstNode()->GetViewProp(), Self);
    Self->EnablePointWidget(Self);
    Self->RemoveHighlightedFaces(Self);
    Self->ShowInteriorHighlightedFaces();
    }
  else
    {
    Self->SurfacePicker->Pick(X,Y,0.0, Self->CurrentRenderer);
    path = Self->SurfacePicker->GetPath();
    if(path !=NULL)
      {
      Self->State = vtkMimxUnstructuredGridWidget::MovingMultipleFace;
      int PickedCell = Self->SurfacePicker->GetCellId();
      if(PickedCell > -1)
        {
        if (Self->SelectedCellIds->GetNumberOfIds() == 1)
          {
          Self->RemoveHighlightedFaces(Self);
          Self->ShowInteriorHighlightedFaces();
          Self->Interactor->Render();
          }
        Self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
        // hide handles so that you can place a quad at the face picked
        Self->HideHandles(Self);
//                              Self->State = vtkMimxUnstructuredGridWidget::PickMultipleFace;
        int location = Self->InteriorFaceList->IsId(PickedCell);
        if(location != -1)      Self->HideInteriorHighlightedFace(PickedCell);
        location = Self->SelectedCellIds->IsId(PickedCell);
        if (location == -1)
          {
          if(Self->FaceGeometry->GetNumberOfItems() == 0)
            {
            Self->SelectedCellIds->InsertNextId(PickedCell);
            Self->FaceGeometry->AddItem(vtkActor::New());
            int curnum = Self->FaceGeometry->GetNumberOfItems()-1;
            vtkActor *curactor = vtkActor::SafeDownCast(
              Self->FaceGeometry->GetItemAsObject(curnum));
            vtkCellArray *cellarray = vtkCellArray::New();
            vtkPolyData *surfacepolydata = Self->SurfaceMapper->GetInput();
            cellarray->InsertNextCell(surfacepolydata->GetCell(Self->SurfacePicker->GetCellId()));
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
            Self->SurfacePicker->GetPickPosition(Self->LastPickPosition);
            Self->ComputeSelectedPointIds(Self);
            }
          }
        }
      }
    else
      {
      Self->State = vtkMimxUnstructuredGridWidget::Start;
      Self->SurfacePicker->GetPickPosition(Self->LastPickPosition);
      Self->RemoveHighlightedFaces(Self);
      Self->RemoveHighlightedEdges(Self);
      Self->ShowInteriorHighlightedFaces();
      return;
      }
    }
  Self->EventCallbackCommand->SetAbortFlag(1);
  Self->StartInteraction();

  Self->Interactor->Render();
}
//----------------------------------------------------------------------
void vtkMimxUnstructuredGridWidget::ShiftLeftButtonDownCallback(vtkAbstractWidget *w)
{
  vtkMimxUnstructuredGridWidget *Self = reinterpret_cast<vtkMimxUnstructuredGridWidget*>(w);

  int X = Self->Interactor->GetEventPosition()[0];
  int Y = Self->Interactor->GetEventPosition()[1];

  // Okay, we can process Self. Try to pick handles first;
  // if no handles picked, then pick the bounding box.
  vtkRenderer *ren = Self->Interactor->FindPokedRenderer(X,Y);
  if ( ren != Self->CurrentRenderer )
    {
    Self->State = vtkMimxUnstructuredGridWidget::Outside;
    return;
    }


  Self->State = vtkMimxUnstructuredGridWidget::Start;
  // Okay, we can process Self. Try to pick handles first;
  // if no handles picked, then try to pick the line.
  Self->RemoveHighlightedEdges(Self);
  vtkAssemblyPath *path;
  Self->SurfacePicker->Pick(X,Y,0.0, Self->CurrentRenderer);
  path = Self->SurfacePicker->GetPath();
  if(path !=NULL)
    {
    int PickedCell = Self->SurfacePicker->GetCellId();
    if(PickedCell > -1)
      {
      Self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
      // hide handles so that you can place a quad at the face picked
      Self->HideHandles(Self);
      Self->State = vtkMimxUnstructuredGridWidget::PickMultipleFace;

      int location = Self->SelectedCellIds->IsId(PickedCell);
      if (location == -1)
        {
        Self->SelectedCellIds->InsertNextId(PickedCell);
        Self->FaceGeometry->AddItem(vtkActor::New());
        int curnum = Self->FaceGeometry->GetNumberOfItems()-1;
        vtkActor *curactor = vtkActor::SafeDownCast(
          Self->FaceGeometry->GetItemAsObject(curnum));
        vtkCellArray *cellarray = vtkCellArray::New();
        vtkPolyData *surfacepolydata = Self->SurfaceMapper->GetInput();
        cellarray->InsertNextCell(surfacepolydata->GetCell(Self->SurfacePicker->GetCellId()));
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
        Self->SurfacePicker->GetPickPosition(Self->LastPickPosition);
        Self->ComputeSelectedPointIds(Self);
        location = Self->InteriorFaceList->IsId(PickedCell);
        if(location != -1)      Self->HideInteriorHighlightedFace(PickedCell);
        }
      else
        {
        vtkActor *curactor = vtkActor::SafeDownCast(
          Self->FaceGeometry->GetItemAsObject(location));
        Self->CurrentRenderer->RemoveActor(curactor);
        curactor->Delete();
        Self->SelectedCellIds->DeleteId(PickedCell);
        Self->FaceGeometry->RemoveItem(location);
        Self->ComputeSelectedPointIds(Self);
        location = Self->InteriorFaceList->IsId(PickedCell);
        if(location != -1)      Self->ShowInteriorHighlightedFace(PickedCell);
        }
      }
    }
  else
    {
    Self->State = vtkMimxUnstructuredGridWidget::Outside;
    return;
    }
  Self->EventCallbackCommand->SetAbortFlag(1);
  Self->StartInteraction();
  Self->Interactor->Render();
}
//-------------------------------------------------------------------------
void vtkMimxUnstructuredGridWidget::ShiftLeftButtonUpCallback(vtkAbstractWidget *w)
{
  vtkMimxUnstructuredGridWidget *Self = reinterpret_cast<vtkMimxUnstructuredGridWidget*>(w);
  //if ( Self->State == vtkMimxUnstructuredGridWidget::Outside ||
  //      Self->State == vtkMimxUnstructuredGridWidget::Start )
  //{
  Self->ShowHandles(Self);
  //      Self->Interactor->Render();
  //      return;
  //}

  //if(Self->State == vtkMimxUnstructuredGridWidget::MovingHandle)
  //{
  //      Self->HighlightHandle(NULL, Self);
  //      Self->SizeHandles(Self);
  //      Self->DisablePointWidget(Self);
  //}
  //else
  //{
  //      Self->CurrentRenderer->RemoveActor(Self->FaceActor);
  //}
  Self->State = vtkMimxUnstructuredGridWidget::Start;


  //Self->EventCallbackCommand->SetAbortFlag(1);
  Self->EndInteraction();
  Self->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
  Self->Interactor->Render();
}
//----------------------------------------------------------------------
void vtkMimxUnstructuredGridWidget::CrtlLeftButtonUpCallback(vtkAbstractWidget *w)
{
  vtkMimxUnstructuredGridWidget *Self = reinterpret_cast<vtkMimxUnstructuredGridWidget*>(w);
  //if ( Self->State == vtkMimxUnstructuredGridWidget::Outside ||
  // Self->State == vtkMimxUnstructuredGridWidget::Start)
  //{
  // Self->Interactor->Render();
  // return;
  //}

  if(Self->State == vtkMimxUnstructuredGridWidget::MovingHandle)
    {
    Self->HighlightHandle(NULL, Self);
    Self->SizeHandles();
    Self->DisablePointWidget(Self);
    }
  //else
  //{
  // Self->CurrentRenderer->RemoveActor(Self->FaceActor);
  //}
  Self->State = vtkMimxUnstructuredGridWidget::Start;

  Self->ShowHandles(Self);

  // *** commented out to prevent Dolly mode from staying on constantly during Unstructured Widget display
  //Self->EventCallbackCommand->SetAbortFlag(1);
  Self->EndInteraction();
  Self->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);

  Self->Interactor->Render();
}
//---------------------------------------------------------------------------------------
void vtkMimxUnstructuredGridWidget::CrtlMiddleButtonDownCallback(vtkAbstractWidget* w)
{
  vtkMimxUnstructuredGridWidget *Self = reinterpret_cast<vtkMimxUnstructuredGridWidget*>(w);

  int X = Self->Interactor->GetEventPosition()[0];
  int Y = Self->Interactor->GetEventPosition()[1];

  // Okay, we can process Self. Try to pick handles first;
  // if no handles picked, then pick the bounding box.
  vtkRenderer *ren = Self->Interactor->FindPokedRenderer(X,Y);
  if ( ren != Self->CurrentRenderer )
    {
    Self->State = vtkMimxUnstructuredGridWidget::Outside;
    return;
    }

//      int i;
  // pick the edges
  vtkAssemblyPath *path;
  Self->EdgePicker->Pick(X,Y,0.0, Self->CurrentRenderer);
  path = Self->EdgePicker->GetPath();
//  int PickedCell = Self->EdgePicker->GetCellId();
  if(path !=NULL)
    {
    //Self->CurrentRenderer->AddActor(Self->EdgeActor);
    //Self->CurrentRenderer->RemoveActor(Self->SurfaceActor);
    //Self->CurrentRenderer->RemoveActor(Self->HexOutline);
    //Self->Interactor->Render();

    //Self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);

    //Self->State = vtkMimxUnstructuredGridWidget::MovingEdge;
    //// hide handles so that you can place a quad at the face picked
    //Self->HideHandles(Self);

    //vtkPolyData *edgepolydata = vtkPolyData::SafeDownCast(
    //      Self->EdgeActor->GetMapper()->GetInput());
    //vtkIntArray *intarray = vtkIntArray::New();
    //intarray->SetNumberOfValues(edgepolydata->GetNumberOfCells());
    //for (i=0; i<edgepolydata->GetNumberOfCells(); i++)
    //{
    //      intarray->SetValue(i, 0);
    //}
    //intarray->SetValue(PickedCell, 1);
    //edgepolydata->GetCellData()->SetScalars(intarray);
    //edgepolydata->Modified();
    //intarray->Delete();
    //Self->SelectedCellIds->Initialize();
    //Self->SelectedCellIds->SetNumberOfIds(1);
    //Self->SelectedCellIds->SetId(0, PickedCell);
    //Self->ComputeSelectedEdgePointIds(Self);
    //Self->EdgePicker->GetPickPosition(Self->LastPickPosition);
    //Self->RemoveHighlightedFaces(Self);
    int PickedCell = Self->EdgePicker->GetCellId();
    if(PickedCell > -1)
      {
      Self->State = vtkMimxUnstructuredGridWidget::MovingEdge;
      Self->RemoveHighlightedFaces(Self);
      Self->Interactor->Render();
      Self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
      // hide handles so that you can place a quad at the face picked
      Self->HideHandles(Self);
      Self->SelectedCellIds->Initialize();
      Self->SelectedCellIds->InsertNextId(PickedCell);
      if(!Self->EdgeGeometry)
        {
        Self->EdgeGeometry = vtkActor::New();
        }

      vtkCellArray *cellarray = vtkCellArray::New();
      vtkPolyData *edgepolydata = vtkPolyData::SafeDownCast(
        Self->EdgeActor->GetMapper()->GetInput());
      cellarray->InsertNextCell(edgepolydata->GetCell(PickedCell));
      vtkPolyData *polydata = vtkPolyData::New();
      polydata->SetPoints(Self->UGrid->GetPoints());
      polydata->SetLines(cellarray);
      cellarray->Delete();
      vtkPolyDataMapper *mapper = vtkPolyDataMapper::New();
      mapper->SetInput(polydata);
      polydata->Delete();
      Self->EdgeGeometry->SetMapper(mapper);
      mapper->Delete();
      Self->EdgeGeometry->GetProperty()->SetLineWidth(
        Self->EdgeActor->GetProperty()->GetLineWidth()*3);

      Self->EdgeGeometry->GetProperty()->SetColor(0.0,1.0,0.0);
      Self->CurrentRenderer->AddActor(Self->EdgeGeometry);
      Self->SurfacePicker->GetPickPosition(Self->LastPickPosition);
      Self->ComputeSelectedEdgePointIds(Self);
      }
    }
  else
    {
    Self->State = vtkMimxUnstructuredGridWidget::Outside;
    return;
    }
  Self->EventCallbackCommand->SetAbortFlag(1);
  Self->StartInteraction();

  Self->Interactor->Render();
}
//---------------------------------------------------------------------------------------
void vtkMimxUnstructuredGridWidget::CrtlMiddleButtonUpCallback(vtkAbstractWidget* w)
{
  vtkMimxUnstructuredGridWidget *Self = reinterpret_cast<vtkMimxUnstructuredGridWidget*>(w);

  if ( Self->State == vtkMimxUnstructuredGridWidget::Outside ||
       Self->State == vtkMimxUnstructuredGridWidget::Start )
    {
    return;
    }
  Self->CurrentRenderer->RemoveActor(Self->EdgeActor);
  //      Self->CurrentRenderer->AddActor(Self->HexActor);
  Self->CurrentRenderer->AddActor(Self->SurfaceActor);
  Self->CurrentRenderer->AddActor(Self->HexOutline);

  //if(Self->State == vtkMimxUnstructuredGridWidget::MovingEdge)
  //{
  Self->ShowHandles(Self);
  //}

  Self->State = vtkMimxUnstructuredGridWidget::Start;


  //Self->EventCallbackCommand->SetAbortFlag(1);
  Self->EndInteraction();
  Self->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);

  Self->SelectedCellIds->Initialize();
  Self->SelectedPointIds->Initialize();

  Self->Interactor->Render();

}
//----------------------------------------------------------------------
void vtkMimxUnstructuredGridWidget::CrtlMouseMoveCallback(vtkAbstractWidget *w)
{
  vtkMimxUnstructuredGridWidget *Self = reinterpret_cast<vtkMimxUnstructuredGridWidget*>(w);
  // See whether we're active
  if ( Self->State == vtkMimxUnstructuredGridWidget::Outside || 
       Self->State == vtkMimxUnstructuredGridWidget::Start ||
       Self->State == vtkMimxUnstructuredGridWidget::PickMultipleFace)
    {
    return;
    }
  if (Self->State == vtkMimxUnstructuredGridWidget::MovingMultipleFace)
    {
    Self->HideHandles(Self);
    }
  int X = Self->Interactor->GetEventPosition()[0];
  int Y = Self->Interactor->GetEventPosition()[1];

  // Do different things depending on state
  // Calculations everybody does
  double focalPoint[4], pickPoint[4], prevPickPoint[4];
  double z;

  vtkRenderer *renderer = Self->Interactor->FindPokedRenderer(X,Y);
  vtkCamera *camera = renderer->IsActiveCameraCreated() ? renderer->GetActiveCamera() : NULL;
  if ( !camera )
    {
    return;
    }

  // Compute the two points defining the motion vector
  Self->ComputeWorldToDisplay(Self->LastPickPosition[0], Self->LastPickPosition[1],
                              Self->LastPickPosition[2], focalPoint);
  z = focalPoint[2];
  Self->ComputeDisplayToWorld(double(Self->Interactor->GetLastEventPosition()[0]),
                              double(Self->Interactor->GetLastEventPosition()[1]),
                              z, prevPickPoint);
  Self->ComputeDisplayToWorld(double(X), double(Y), z, pickPoint);

  // Process the motion
  if ( Self->State == vtkMimxUnstructuredGridWidget::MovingHandle ||
       Self->State == vtkMimxUnstructuredGridWidget::MovingMultipleFace
       || Self->State == vtkMimxUnstructuredGridWidget::MovingEdge)
    {
    if(Self->State == vtkMimxUnstructuredGridWidget::MovingHandle)
      {
      vtkPoints *hexpoints = vtkUnstructuredGrid::SafeDownCast(
        Self->HexMapper->GetInput())->GetPoints();
      vtkPoints *surfacepoints = Self->SurfaceMapper->GetInput()->GetPoints();
      vtkPoints *ugridpoints = Self->UGrid->GetPoints();
      double x[3];
      ugridpoints->GetPoint(Self->PWCallback->HandleNumber, x);
      for (int i=0; i<3; i++)
        {
        x[i] = x[i] + pickPoint[i] - prevPickPoint[i];
        }
      Self->SetPoint(Self->PWCallback->HandleNumber, x, Self);
      hexpoints->SetPoint(Self->PWCallback->HandleNumber, x);
      surfacepoints->SetPoint(Self->PWCallback->HandleNumber, x);
      hexpoints->Modified();
      surfacepoints->Modified();
      ugridpoints->Modified();
      Self->PositionHandles(Self);
      }
    else{
    Self->TranslateProp(prevPickPoint, pickPoint, Self);
    }
    }
  else
    {
    //Self->TranslateProp(prevPickPoint, pickPoint);
    //int i;
    //for (i=0; i<4; i++)   Self->LastPickPosition[i] = pickPoint[i];
    }
  Self->EventCallbackCommand->SetAbortFlag(1);
  Self->InvokeEvent(vtkCommand::InteractionEvent,NULL);

  Self->Interactor->Render();

}
//---------------------------------------------------------------------
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
//-----------------------------------------------------------------------------------------------
void vtkMimxUnstructuredGridWidget::PositionHandles(vtkMimxUnstructuredGridWidget *Self)
{
  for(int i=0; i < Self->Handle->GetNumberOfItems(); i++)
    {
    double x[3];
    Self->UGrid->GetPoint(i,x);
    ((vtkSphereSource*) (Self->HandleGeometry->GetItemAsObject(i)))->
      SetCenter(Self->UGrid->GetPoint(i));
    }
}
//-----------------------------------------------------------------------------------
void vtkMimxUnstructuredGridWidget::SizeHandles(vtkMimxUnstructuredGridWidget *Self)
{
  vtkIntArray *constrain = vtkIntArray::SafeDownCast(
    Self->UGrid->GetPointData()->GetArray("Constrain"));
  if(!constrain)
    {
    for(int i=0; i< Self->Handle->GetNumberOfItems(); i++)
      {
      ((vtkSphereSource*) (Self->HandleGeometry->GetItemAsObject(i)))
        ->SetRadius(Self->HandleSize);
      }
    }
  else
    {
    for(int i=0; i< Self->Handle->GetNumberOfItems(); i++)
      {
      if(!constrain->GetValue(i))
        {
        ((vtkSphereSource*) (Self->HandleGeometry->GetItemAsObject(i)))
          ->SetRadius(Self->HandleSize);
        }
      else
        {
        ((vtkSphereSource*) (Self->HandleGeometry->GetItemAsObject(i)))
          ->SetRadius(0.0);
        }
      }
    }
}
//------------------------------------------------------------------------------------------------
void vtkMimxUnstructuredGridWidget::SizeHandles()
{
  vtkIntArray *constrain = vtkIntArray::SafeDownCast(
    this->UGrid->GetPointData()->GetArray("Constrain"));
  if(!constrain)
    {
    for(int i=0; i< this->Handle->GetNumberOfItems(); i++)
      {
      ((vtkSphereSource*) (this->HandleGeometry->GetItemAsObject(i)))
        ->SetRadius(this->HandleSize);
      }
    }
  else
    {
    for(int i=0; i< this->Handle->GetNumberOfItems(); i++)
      {
      if(!constrain->GetValue(i))
        {
        ((vtkSphereSource*) (this->HandleGeometry->GetItemAsObject(i)))
          ->SetRadius(this->HandleSize);
        }
      else
        {
        ((vtkSphereSource*) (this->HandleGeometry->GetItemAsObject(i)))
          ->SetRadius(0.0);
        }
      }
    }
}
//-----------------------------------------------------------------------------
int vtkMimxUnstructuredGridWidget::HighlightHandle(vtkProp *prop, vtkMimxUnstructuredGridWidget *Self)
{
  // first unhighlight anything picked
  if ( Self->CurrentHandle )
    {
    Self->CurrentHandle->SetProperty(Self->HandleProperty);
    }

  // set the current handle
  Self->CurrentHandle = (vtkActor *)prop;

  // find the current handle
  if ( Self->CurrentHandle )
    {
    Self->ValidPick = 1;
    Self->HandlePicker->GetPickPosition(Self->LastPickPosition);
    Self->CurrentHandle->SetProperty(Self->SelectedHandleProperty);
    return 1;
    }
  else
    {
    return -1;
    }
}
//------------------------------------------------------------------------------------
void vtkMimxUnstructuredGridWidget::CreateDefaultProperties()
{
  // Handle properties
  this->HandleProperty = vtkProperty::New();
  this->HandleProperty->SetColor(1,0,0);

  this->SelectedHandleProperty = vtkProperty::New();
  this->SelectedHandleProperty->SetColor(0,1,0);  

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
//----------------------------------------------------------------------------
void vtkMimxUnstructuredGridWidget::EnablePointWidget(vtkMimxUnstructuredGridWidget *Self)
{
  // Set up the point widgets
  double x[3];
  if ( Self->CurrentHandle ) //picking the handles
    {
    bool status = false;
    int count = 0;
    do {
    if ( Self->CurrentHandle == (vtkActor*) 
         Self->Handle->GetItemAsObject(count) )
      {
      Self->CurrentPointWidget = (vtkMimxModPointWidget*) 
        (Self->PointWidget->GetItemAsObject(count));
      Self->UGrid->GetPoint(count,x);
      status = true;
      PWCallback->HandleNumber = count;
      PWCallback->PointWidget = Self->CurrentPointWidget;
      }
    count++;
    }
    while(count < Self->UGrid->GetNumberOfPoints() && status == false);

    }

  double bounds[6];
  for (int i=0; i<3; i++)
    {
    bounds[2*i] = x[i] - 0.1*Self->InitialLength;
    bounds[2*i+1] = x[i] + 0.1*Self->InitialLength;
    }
  bounds[0] = 0; bounds[2] = 0; bounds[4] = 0;
  bounds[1] = 1; bounds[3] = 3; bounds[5] = 5;
  // Note: translation mode is disabled and enabled to control
  // the proper positioning of the bounding box.
  Self->CurrentPointWidget->SetInteractor(Self->Interactor);
  Self->CurrentPointWidget->TranslationModeOff();
  Self->CurrentPointWidget->SetPlaceFactor(1.0);
  Self->CurrentPointWidget->PlaceWidget(bounds);
  Self->CurrentPointWidget->TranslationModeOn();
  Self->CurrentPointWidget->SetPosition(x);
  Self->CurrentPointWidget->On();
}
//--------------------------------------------------------------------------------------
void vtkMimxUnstructuredGridWidget::DisablePointWidget(
  vtkMimxUnstructuredGridWidget *Self)
{
  Self->CurrentPointWidget->Off();
  Self->CurrentPointWidget = NULL;
}
//-----------------------------------------------------------------------------------
void vtkMimxUnstructuredGridWidget::SetPoint(int i, double x[3], vtkMimxUnstructuredGridWidget *Self)
{
  //      double x1[3];
  Self->UGrid->GetPoints()->SetPoint(i,x);
  Self->UGrid->GetPoints()->Modified();
  Self->UGrid->Modified();
  ((vtkSphereSource*) (Self->HandleGeometry->GetItemAsObject(i)))->SetCenter(x);
  ((vtkSphereSource*) (Self->HandleGeometry->GetItemAsObject(i)))->Update();

  // updating complete mesh if set
  vtkIntArray *intarray = vtkIntArray::SafeDownCast(
    Self->UGrid->GetPointData()->GetArray("Original_Point_Ids"));
  if(Self->CompleteUGrid)
    {
    if(intarray)
      {
      Self->CompleteUGrid->GetPoints()->SetPoint(intarray->GetValue(i), x);
      Self->CompleteUGrid->GetPoints()->Modified();
      }
    }
}

void vtkMimxUnstructuredGridWidget::Initialize(vtkMimxUnstructuredGridWidget *Self)
{
  Self->HandleGeometry->InitTraversal();
  Self->HandleMapper->InitTraversal();
  Self->Handle->InitTraversal();
  Self->PointWidget->InitTraversal();
  Self->SelectedCellIds->Initialize();
  vtkIdType numPts = Self->HandleGeometry->GetNumberOfItems();
  while (numPts !=0)
    {
    Self->HandleGeometry->GetNextItemAsObject()->Delete();
    Self->HandleMapper->GetNextItemAsObject()->Delete();
    Self->Handle->GetNextItemAsObject()->Delete();
    Self->PointWidget->GetNextItemAsObject()->Delete();
    numPts--;
    }
  Self->HandleGeometry->RemoveAllItems();
  Self->HandleMapper->RemoveAllItems();
  Self->Handle->RemoveAllItems();
  Self->PointWidget->RemoveAllItems();
  Self->HandlePicker->InitializePickList();

  // initializing the surface actor
  if(Self->SurfaceMapper)
    {
    Self->SurfaceMapper->Delete();
    Self->SurfaceMapper = NULL;
    }
  if(Self->SurfaceActor)
    {
    Self->SurfaceActor->Delete();
    Self->SurfaceActor = NULL;
    }

  Self->SurfacePicker->InitializePickList();

  Self->DeleteInteriorHighlightedFaces();
}

void vtkMimxUnstructuredGridWidget::TranslateProp(double p1[3], double p2[3], vtkMimxUnstructuredGridWidget *Self)
{
  double dirvec[3];
  int i, j;
  for (i=0; i<3; i++)     dirvec[i] = p2[i] - p1[i];

  //vtkPoints *facepoints = Self->FaceMapper->GetInput()->GetPoints();
  vtkPoints *surfacepoints = Self->SurfaceMapper->GetInput()->GetPoints();
  vtkPoints *ugridpoints = Self->UGrid->GetPoints();
  //      vtkPoints *edgepoints = Self->EdgeActor->GetMapper()->GetInpu
  vtkIdList *idlist = Self->SelectedPointIds;
  for (i=0; i < idlist->GetNumberOfIds(); i++)
    {
    double point[3];
    Self->UGrid->GetPoint(idlist->GetId(i), point);
    for(j=0; j<3; j++)      point[j] += dirvec[j];
    //facepoints->SetPoint(idlist->GetId(i), point);
    surfacepoints->SetPoint(idlist->GetId(i), point);
    ugridpoints->SetPoint(idlist->GetId(i), point);
    Self->SetPoint(idlist->GetId(i), point, Self);
    }
  //facepoints->Modified();
  surfacepoints->Modified();
  ugridpoints->Modified();
  Self->PositionHandles(Self);
}
//----------------------------------------------------------------------------------
void vtkMimxUnstructuredGridWidget::HideHandles(vtkMimxUnstructuredGridWidget *Self)
{
  // turn off the handles
  for (int i=0; i<Self->Handle->GetNumberOfItems(); i++)
    {
    Self->CurrentRenderer->RemoveActor((vtkActor*)
                                       Self->Handle->GetItemAsObject(i));
    }
}
//-----------------------------------------------------------------------------------
void vtkMimxUnstructuredGridWidget::ShowHandles(vtkMimxUnstructuredGridWidget *Self)
{
  // turn off the handles
  for (int i=0; i<Self->Handle->GetNumberOfItems(); i++)
    {
    Self->CurrentRenderer->AddActor((vtkActor*)
                                    Self->Handle->GetItemAsObject(i));
    }
}
//--------------------------------------------------------------------------------------
void vtkMimxUnstructuredGridWidget::TranslatePropFace(double p1[3], double p2[3], vtkMimxUnstructuredGridWidget *Self)
{
  double dirvec[3];
  int i, j;
  for (i=0; i<3; i++)     dirvec[i] = p2[i] - p1[i];
  vtkMath::Normalize(dirvec);
  double dot = vtkMath::Dot(dirvec,Self->FaceNormal);

  vtkIdList *idlist = vtkIdList::New();
  Self->FaceMapper->GetInput()->GetCellPoints(0,idlist);
  vtkPoints *facepoints = Self->FaceMapper->GetInput()->GetPoints();
  vtkPoints *surfacepoints = Self->SurfaceMapper->GetInput()->GetPoints();
  vtkPoints *ugridpoints = Self->SurfaceMapper->GetInput()->GetPoints();

  vtkIntArray *constrain = vtkIntArray::SafeDownCast(
    Self->UGrid->GetPointData()->GetArray("Constrain"));

  if(!constrain)
    {
    for (i=0; i < idlist->GetNumberOfIds(); i++)
      {
      double point[3];
      facepoints->GetPoint(idlist->GetId(i), point);
      for(j=0; j<3; j++)      point[j] += Self->FaceNormal[j]*dot*0.15;
      facepoints->SetPoint(idlist->GetId(i), point);
      surfacepoints->SetPoint(idlist->GetId(i), point);
      ugridpoints->SetPoint(idlist->GetId(i), point);
      Self->SetPoint(idlist->GetId(i), point, Self);
      }
    }
  else{
  int stat[4];
  for (i=0; i<4; i++)
    {
    stat[i] = constrain->GetValue(idlist->GetId(i));
    }
  if(!stat[0] && !stat[1] && !stat[2] && !stat[3])
    {
    for (i=0; i < idlist->GetNumberOfIds(); i++)
      {
      double point[3];
      facepoints->GetPoint(idlist->GetId(i), point);
      for(j=0; j<3; j++)      point[j] += Self->FaceNormal[j]*dot*0.15;
      facepoints->SetPoint(idlist->GetId(i), point);
      surfacepoints->SetPoint(idlist->GetId(i), point);
      ugridpoints->SetPoint(idlist->GetId(i), point);
      Self->SetPoint(idlist->GetId(i), point, Self);
      }
    }
  }

  facepoints->Modified();
  surfacepoints->Modified();
  ugridpoints->Modified();
  Self->PositionHandles(Self);
  idlist->Delete();
}
//-----------------------------------------------------------------------------------------
void vtkMimxUnstructuredGridWidget::TranslatePropFaceParallel(double p1[3], double p2[3], vtkMimxUnstructuredGridWidget *Self)
{
  double dirvec[3], pt1[3], pt2[3], edgedirvec[3], edgedirvecnor[3],
    dot, choicedirvec[3];
  int i, j;
  double dist = sqrt(vtkMath::Distance2BetweenPoints(p1,p2));
  // initially obtain the direction of an edge.
  vtkIdType npts, *pts;
  Self->FaceMapper->GetInput()->GetPolys()->GetCell(0, npts, pts);
  Self->FaceMapper->GetInput()->GetPoint(pts[0], pt1);
  Self->FaceMapper->GetInput()->GetPoint(pts[1], pt2);
  //
  for (i=0; i<3; i++)     edgedirvec[i] = pt2[i] - pt1[i];
  vtkMath::Normalize(edgedirvec);
  //
  for (i=0; i<3; i++)     dirvec[i] = p2[i] - p1[i];
  vtkMath::Normalize(dirvec);
  // calculate the third axis
  vtkMath::Cross(edgedirvec, Self->FaceNormal, edgedirvecnor);
  vtkMath::Normalize(edgedirvecnor);
  // calculate the dotproducts with all the three vectors
  double dot1 = vtkMath::Dot(Self->FaceNormal, dirvec);
  double dot2 = vtkMath::Dot(edgedirvec,dirvec);
  double dot3 = vtkMath::Dot(edgedirvecnor,dirvec);
  // check for the largest dot product absolute value
  if(fabs(dot1) > fabs(dot2))
    {
    dot = dot1;
    for(i=0; i<3; i++)
      choicedirvec[i] = Self->FaceNormal[i];
    }
  else
    {
    dot = dot2;
    for(i=0; i<3; i++)
      choicedirvec[i] = edgedirvec[i];
    }

  if(fabs(dot3) > fabs(dot))
    {
    dot = dot3;
    for(i=0; i<3; i++)
      choicedirvec[i] = edgedirvecnor[i];
    }

  vtkIdList *idlist = vtkIdList::New();
  Self->FaceMapper->GetInput()->GetCellPoints(0,idlist);
  vtkPoints *facepoints = Self->FaceMapper->GetInput()->GetPoints();
  vtkPoints *surfacepoints = Self->SurfaceMapper->GetInput()->GetPoints();
  vtkPoints *ugridpoints = Self->SurfaceMapper->GetInput()->GetPoints();

  vtkIntArray *constrain = vtkIntArray::SafeDownCast(
    Self->UGrid->GetPointData()->GetArray("Constrain"));

  if(!constrain)
    {
    for (i=0; i < idlist->GetNumberOfIds(); i++)
      {
      double point[3];
      facepoints->GetPoint(idlist->GetId(i), point);
      //                      for(j=0; j<3; j++)      point[j] += choicedirvec[j]*dot*0.15;
      for(j=0; j<3; j++)      point[j] += dirvec[j]*dist;
      facepoints->SetPoint(idlist->GetId(i), point);
      surfacepoints->SetPoint(idlist->GetId(i), point);
      ugridpoints->SetPoint(idlist->GetId(i), point);
      Self->SetPoint(idlist->GetId(i), point, Self);
      }
    }
  else{
  int stat[4];
  for (i=0; i<4; i++)
    {
    stat[i] = constrain->GetValue(idlist->GetId(i));
    }
  if(!stat[0] && !stat[1] && !stat[2] && !stat[3])
    {
    for (i=0; i < idlist->GetNumberOfIds(); i++)
      {
      double point[3];
      facepoints->GetPoint(idlist->GetId(i), point);
      //                      for(j=0; j<3; j++)      point[j] += choicedirvec[j]*dot*0.15;
      for(j=0; j<3; j++)      point[j] += dirvec[j]*dist;
      facepoints->SetPoint(idlist->GetId(i), point);
      surfacepoints->SetPoint(idlist->GetId(i), point);
      ugridpoints->SetPoint(idlist->GetId(i), point);
      Self->SetPoint(idlist->GetId(i), point, Self);
      }
    }
  }

  facepoints->Modified();
  surfacepoints->Modified();
  ugridpoints->Modified();
  Self->PositionHandles(Self);
  idlist->Delete();
}
//------------------------------------------------------------------------------------------------
void vtkMimxUnstructuredGridWidget::ComputeSelectedPointIds(vtkMimxUnstructuredGridWidget *Self)
{
  int i, j;
  Self->SelectedPointIds->Initialize();
  for(i=0; i<Self->SelectedCellIds->GetNumberOfIds(); i++)
    {
    vtkIdList *idlist;
    idlist = Self->SurfaceMapper->GetInput()->GetCell(
      Self->SelectedCellIds->GetId(i))->GetPointIds();
    for(j=0; j<idlist->GetNumberOfIds(); j++)
      {
      Self->SelectedPointIds->InsertUniqueId(idlist->GetId(j));
      }
    }
}
//-------------------------------------------------------------------------------------------------
void vtkMimxUnstructuredGridWidget::ExtractEdge(vtkMimxUnstructuredGridWidget *Self)
{
  // the unstructured grid is converted to polydata with each edge 
  // stored as a cell in polydata
  vtkIdType* pts=0;
  vtkIdType t=0;
  Self->UGrid->GetCells()->InitTraversal();
  //vtkPoints* points = vtkPoints::New();
  vtkPolyData *edgepolydata = vtkPolyData::New();
  //points->SetNumberOfPoints(Self->UGrid->GetNumberOfPoints());
  //for(int i=0; i <Self->UGrid->GetNumberOfPoints(); i++)
  //      points->SetPoint(i,Self->UGrid->GetPoint(i));
  edgepolydata->SetPoints(Self->UGrid->GetPoints());

  vtkCell* cell;
  vtkCellArray* edgelist = vtkCellArray::New();
  edgelist->InitTraversal();
  for(int i=0; i < Self->UGrid->GetNumberOfCells(); i++)
    {
    if(i >0)
      {
      cell = Self->UGrid->GetCell(i);
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
        }
        while(!status && edgelist->GetNextCell(t,pts));
        if(!status)     
          {edgelist->InsertNextCell(2);
          edgelist->InsertCellPoint(pt1);
          edgelist->InsertCellPoint(pt2);
          }
        }
      }
    else
      {
      cell = Self->UGrid->GetCell(i);
      for(int j=0; j < cell->GetNumberOfEdges(); j++)
        {
        vtkCell* edgecell = cell->GetEdge(j);
        vtkIdList* pointlist = edgecell->GetPointIds();
        vtkIdType pt1 = pointlist->GetId(0);
        vtkIdType pt2 = pointlist->GetId(1);
        edgelist->InsertNextCell(2);
        edgelist->InsertCellPoint(pt1);
        edgelist->InsertCellPoint(pt2);
        }
      }
    }
  edgepolydata->SetLines(edgelist);
  vtkPolyDataMapper::SafeDownCast(
    Self->EdgeActor->GetMapper())->SetInput(edgepolydata);
  Self->EdgeActor->GetMapper()->GetInput()->Modified();
//      Self->EdgeActor->GetProperty()->SetLineWidth(3);

  Self->EdgePicker->AddPickList(Self->EdgeActor);
  Self->EdgePicker->PickFromListOn();

  //vtkLookupTable *lut = vtkLookupTable::New();
  //lut->SetNumberOfColors(2);
  //lut->Build();
  //lut->SetTableValue(0, 1.0, 1.0, 1.0, 1.0);
  //lut->SetTableValue(1, 0.0, 1.0, 0.0, 1.0);
  //lut->SetTableRange(0,1);
  //Self->EdgeActor->GetMapper()->SetLookupTable(lut);
  //Self->EdgeActor->GetMapper()->SetScalarRange(0,1);
  //lut->Delete();
  //      points->Delete();
  edgepolydata->Delete();
  edgelist->Delete();
}
//------------------------------------------------------------------------------------------------
void vtkMimxUnstructuredGridWidget::ComputeSelectedEdgePointIds(vtkMimxUnstructuredGridWidget *Self)
{
  int i, j;
  Self->SelectedPointIds->Initialize();
  for(i=0; i<Self->SelectedCellIds->GetNumberOfIds(); i++)
    {
    vtkIdList *idlist;
    vtkPolyDataMapper *mapper = vtkPolyDataMapper::SafeDownCast(
      Self->EdgeActor->GetMapper());
    vtkPolyData *polydata = vtkPolyData::SafeDownCast(mapper->GetInput());
    idlist = polydata->GetCell(Self->SelectedCellIds->GetId(i))->GetPointIds();
    for(j=0; j<idlist->GetNumberOfIds(); j++)
      {
      Self->SelectedPointIds->InsertUniqueId(idlist->GetId(j));
      }
    }
}
//-------------------------------------------------------------------------------------------------
void vtkMimxUnstructuredGridWidget::RemoveHighlightedFaces(vtkMimxUnstructuredGridWidget *Self)
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
    Self->SelectedCellIds->Initialize();
    Self->SelectedPointIds->Initialize();
    }
}
//-------------------------------------------------------------------------------------------------
void vtkMimxUnstructuredGridWidget::ComputeAverageHandleSize(vtkMimxUnstructuredGridWidget *Self)
{
  if(Self->UGrid)
    {
    double cumdist = 0.0;
    int count = 0;
    Self->HandleSize = 0.0;
    int i,j;
    for (i=0; i<Self->UGrid->GetNumberOfCells(); i++)
      {
      vtkCell *cell = Self->UGrid->GetCell(i);
      for (j=0; j<cell->GetNumberOfEdges(); j++)
        {
        vtkCell *edge = cell->GetEdge(j);
        vtkIdList *ptids = edge->GetPointIds();
        int pt1 = ptids->GetId(0);
        int pt2 = ptids->GetId(1);
        double p1[3], p2[3];
        Self->UGrid->GetPoint(pt1, p1); Self->UGrid->GetPoint(pt2, p2);
        cumdist = cumdist + sqrt(vtkMath::Distance2BetweenPoints(p1, p2));
        count ++;
        }
      }
    Self->HandleSize = 0.0625*cumdist/count;
    }
}

//-------------------------------------------------------------------------------------------------
void vtkMimxUnstructuredGridWidget::ComputeHandleSize(vtkMimxUnstructuredGridWidget *Self)
{
  if (Self->UGrid)
    {
    double bounds[6];
    Self->UGrid->GetBounds( bounds );
    double size = bounds[1] - bounds[0];
    size += bounds[3] - bounds[2];
    size += bounds[5] - bounds[4];
                
    size /= 3.0;
                
    Self->HandleSize = 0.03125*size;
    }
}


//-------------------------------------------------------------------------------------------------
void vtkMimxUnstructuredGridWidget::RemoveHighlightedEdges(vtkMimxUnstructuredGridWidget *Self)
{
  if(Self->EdgeGeometry)
    Self->CurrentRenderer->RemoveActor(Self->EdgeGeometry);
}
//-------------------------------------------------------------------------------------------------
void vtkMimxUnstructuredGridWidget::ShowInteriorHighlightedFaces()
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
void vtkMimxUnstructuredGridWidget::HideInteriorHighlightedFaces()
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
void vtkMimxUnstructuredGridWidget::DeleteInteriorHighlightedFaces()
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
void vtkMimxUnstructuredGridWidget::ComputeInteriorHighlightedFaces()
{
  int i,CellFace;
  int CompleteCell, CompleteFace;
  int numCells = this->SurfaceMapper->GetInput()->GetNumberOfCells();
  this->InteriorFaceList->Initialize();

  for (i=0; i<numCells; i++)
    {
    int CellNum = this->GetInputPickedCellAndFace(i, CellFace);
    if(!this->GetInputPickedCompleteFace(CellNum, CellFace, CompleteCell, CompleteFace))
      {
      this->InteriorFaceList->InsertNextId(i);
      this->InteriorFaceGeometry->AddItem(vtkActor::New());
      int curnum = this->InteriorFaceGeometry->GetNumberOfItems()-1;
      vtkActor *curactor = vtkActor::SafeDownCast(
        this->InteriorFaceGeometry->GetItemAsObject(curnum));

      vtkCellArray *cellarray = vtkCellArray::New();
      cellarray->InsertNextCell(this->SurfaceMapper->GetInput()->GetCell(i));

      vtkPolyData *polydata = vtkPolyData::New();
      polydata->SetPoints(this->SurfaceMapper->GetInput()->GetPoints());
      polydata->SetPolys(cellarray);
      cellarray->Delete();
      vtkPolyDataMapper *mapper = vtkPolyDataMapper::New();
      mapper->SetInput(polydata);
      polydata->Delete();
      curactor->SetMapper(mapper);
      mapper->Delete();
      curactor->GetProperty()->SetColor(1.0,1.0,0.0);
      }
    }
}
//------------------------------------------------------------------------------------------------
int vtkMimxUnstructuredGridWidget::GetInputPickedCellAndFace(
  int PickedFace, int &CellFace)
{
  // check to which cell and face it belongs
  int i,j, k;
  vtkIdList *surfacepointlist;
  vtkIdList *cellfacepointlist;
  // get pointids of the picked cell in the surface
  surfacepointlist = this->SurfaceMapper->GetInput()->GetCell(PickedFace)->GetPointIds();
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
int vtkMimxUnstructuredGridWidget::GetInputPickedCompleteFace(
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
void vtkMimxUnstructuredGridWidget::ShowInteriorHighlightedFace(int FaceNum)
{
  int location = this->InteriorFaceList->IsId(FaceNum);
  if(location == -1)      return;

  vtkActor *curactor = vtkActor::SafeDownCast(
    this->InteriorFaceGeometry->GetItemAsObject(location));
  this->CurrentRenderer->AddActor(curactor);
}
//--------------------------------------------------------------------------------------------
void vtkMimxUnstructuredGridWidget::HideInteriorHighlightedFace(int FaceNum)
{
  int location = this->InteriorFaceList->IsId(FaceNum);
  if(location == -1)      return;

  vtkActor *curactor = vtkActor::SafeDownCast(
    this->InteriorFaceGeometry->GetItemAsObject(location));
  this->CurrentRenderer->RemoveActor(curactor);
}
//--------------------------------------------------------------------------------------------
void vtkMimxUnstructuredGridWidget::ComputeSmallestEdgeLength(vtkMimxUnstructuredGridWidget *Self)
{
  if(Self->UGrid)
    {
    Self->HandleSize = VTK_FLOAT_MAX;
    int i,j;
    for (i=0; i<Self->UGrid->GetNumberOfCells(); i++)
      {
      vtkCell *cell = Self->UGrid->GetCell(i);
      for (j=0; j<cell->GetNumberOfEdges(); j++)
        {
        vtkCell *edge = cell->GetEdge(j);
        vtkIdList *ptids = edge->GetPointIds();
        int pt1 = ptids->GetId(0);
        int pt2 = ptids->GetId(1);
        double p1[3], p2[3];
        Self->UGrid->GetPoint(pt1, p1); Self->UGrid->GetPoint(pt2, p2);
        double dist = sqrt(vtkMath::Distance2BetweenPoints(p1, p2));
        if(Self->HandleSize > dist)     Self->HandleSize = dist; 
        }
      }
    }
}
//-------------------------------------------------------------------------------------------------
