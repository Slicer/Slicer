/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxSelectCellsWidget.cxx,v $
Language:  C++
Date:      $Date: 2008/06/28 01:24:43 $
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
#include "vtkMimxSelectCellsWidget.h"

#include "vtkActor.h"
#include "vtkAssemblyNode.h"
#include "vtkAssemblyPath.h"
#include "vtkCallbackCommand.h"
#include "vtkCamera.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCellPicker.h"
#include "vtkDataSetMapper.h"
#include "vtkExtractSelectedFrustum.h"
#include "vtkIntArray.h"
#include "vtkInteractorStyleRubberBandPick.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkProperty.h"
#include "vtkRenderedAreaPicker.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkUnstructuredGrid.h"
#include "vtkInteractorObserver.h"
#include "vtkInteractorStyleSwitch.h"
#include "vtkUnstructuredGridWriter.h"
#include "vtkMimxEquivalancePoints.h"
#include "vtkWidgetCallbackMapper.h"
#include "vtkWidgetEventTranslator.h"
#include "vtkRenderWindow.h"
#include "vtkPlane.h"

#include "vtkMimxMapOriginalCellAndPointIds.h"

vtkCxxRevisionMacro(vtkMimxSelectCellsWidget, "$Revision: 1.10 $");
vtkStandardNewMacro(vtkMimxSelectCellsWidget);

vtkMimxSelectCellsWidget::vtkMimxSelectCellsWidget()
{
  this->WidgetEvent = vtkMimxSelectCellsWidget::Start;
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                          vtkMimxSelectCellsWidget::LeftMouseButtonUp,
                                          this, vtkMimxSelectCellsWidget::LeftButtonUpCallback);

  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
                                          vtkMimxSelectCellsWidget::LeftMouseButtonDown,
                                          this, vtkMimxSelectCellsWidget::LeftButtonDownCallback);

  this->CallbackMapper->SetCallbackMethod(vtkCommand::MouseMoveEvent,
                                          vtkMimxSelectCellsWidget::MouseMove,
                                          this, vtkMimxSelectCellsWidget::MouseMoveCallback);

  this->RubberBandStyle =  vtkInteractorStyleRubberBandPick::New();
  this->AreaPicker = vtkRenderedAreaPicker::New();
  this->Inputset = NULL;
  //this->CurrentSelectedSubset = NULL;
  this->CurrentSelectedSubset = vtkUnstructuredGrid::New();
  this->CurrentSelectedSubsetMapper = vtkDataSetMapper::New();
  this->CurrentSelectedSubsetMapper->SetInput(this->CurrentSelectedSubset);
  this->CurrentSelectedSubsetActor = vtkActor::New();
  this->CurrentSelectedSubsetActor->SetMapper(this->CurrentSelectedSubsetMapper);

  //this->PreviousSelectedSubset = NULL;
  //this->PreviousSelectedSubsetActor = NULL;
  //this->PreviousSelectedSubsetMapper = NULL;
 
 
    

  
  
  
}
//---------------------------------------------------------------------------------
vtkMimxSelectCellsWidget::~vtkMimxSelectCellsWidget()
{
  this->Enabled = 0;
  if(this->RubberBandStyle)
    this->RubberBandStyle->Delete();
  this->RubberBandStyle = NULL;
  this->AreaPicker->Delete();

  /*if(this->CurrentSelectedSubset)
         this->CurrentSelectedSubset->Delete();*/

  /*if(this->PreviousSelectedSubset)
         this->PreviousSelectedSubset->Delete();*/

  /*if(this->CurrentSelectedSubsetMapper)
         this->CurrentSelectedSubsetMapper->Delete();*/

  this->CurrentSelectedSubset->Delete();
  this->CurrentSelectedSubsetMapper->Delete();
  if(this->CurrentSelectedSubsetActor)
    this->CurrentSelectedSubsetActor->Delete();

  //if(this->PreviousSelectedSubsetMapper)
  // this->PreviousSelectedSubsetMapper->Delete();

  //if(this->PreviousSelectedSubsetActor)
  // this->PreviousSelectedSubsetActor->Delete();
}
//---------------------------------------------------------------------------------
void vtkMimxSelectCellsWidget::SetEnabled(int enabling)
{
  if ( !this->Interactor )
    {
    vtkErrorMacro(<<"The interactor must be set prior to enabling/disabling widget");
    return;
    }

  if ( enabling ) //------------------------------------------------------------
    {
//              this->CurrentStyle = this->Interactor->GetInteractorStyle();

    if ( this->Enabled ) //already enabled, just return
      {
      return;
      }

    if ( !this->CurrentRenderer )
      {
      this->SetCurrentRenderer(this->Interactor->FindPokedRenderer(
                                 this->Interactor->GetLastEventPosition()[0],
                                 this->Interactor->GetLastEventPosition()[1]));
      if ( this->CurrentRenderer == NULL )
        {
        return;
        }
      }
    //if(!this->PreviousSelectedSubset)
    //{
    //      vtkErrorMacro("If enabling the widget for the first time, Call functions SetInputActor(), Initialize() and then call SetEnabled()");
    //      return;
    //}
    this->Enabled = 1;
    // set all the interactions
    if(this->CurrentSelectedSubsetActor)
      {
      this->CurrentRenderer->AddActor(this->CurrentSelectedSubsetActor);
      this->CurrentSelectedSubsetActor->GetProperty()->SetLineWidth(
        this->CurrentSelectedSubsetActor->GetProperty()->GetLineWidth()*2.0);
      }
//      this->CurrentRenderer->RemoveActor(this->PreviousSelectedSubsetActor);
    //this->CurrentRenderer->AddActor(this->PreviousSelectedSubsetActor);
//      char KeyCode = 'r";
    // Listen for the following events
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

  else //disabling----------------------------------------------------------
    {
    vtkDebugMacro(<<"Disabling selectcells widget");

    if ( !this->Enabled ) //already disabled, just return
      {
      return;
      }
    //this->CurrentRenderer->RemoveActor(this->PreviousSelectedSubsetActor);
    if(this->CurrentSelectedSubsetActor)
      {
//              this->CurrentRenderer->RemoveActor(this->CurrentSelectedSubsetActor);
      this->CurrentSelectedSubsetActor->GetProperty()->SetLineWidth(
        this->CurrentSelectedSubsetActor->GetProperty()->GetLineWidth()/2.0);
      }
    // don't listen for events any more
    if ( ! this->Parent )
      {
      this->Interactor->RemoveObserver(this->EventCallbackCommand);
      }
    else
      {
      this->Parent->RemoveObserver(this->EventCallbackCommand);
      }
    this->Enabled = 0;
    }

  this->Interactor->Render();
  
  if ( !this->Interactor )
    {
    vtkErrorMacro(<<"The interactor must be set prior to fixing the interactor size");
    
    int rwSizeX = this->Interactor->GetRenderWindow()->GetSize()[0];
    int rwSizeY = this->Interactor->GetRenderWindow()->GetSize()[1];
    cout << "render window says: " << rwSizeX << " " << rwSizeY << endl;
    int interSizeX = this->Interactor->GetSize()[0];
    int interSizeY = this->Interactor->GetSize()[1];
    cout << "interactor says: " << interSizeX << " " << interSizeY << endl;
    this->Interactor->UpdateSize(rwSizeX,rwSizeY);
    interSizeX = this->Interactor->GetRenderWindow()->GetSize()[0];
    interSizeY = this->Interactor->GetRenderWindow()->GetSize()[1];
    cout << "interactor says: " << interSizeX << " " << interSizeY << endl;
    }
 
  
}
//---------------------------------------------------------------------------------
void vtkMimxSelectCellsWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//---------------------------------------------------------------------------------
void vtkMimxSelectCellsWidget::LeftButtonDownCallback(vtkAbstractWidget *w)
{
  vtkMimxSelectCellsWidget *self = reinterpret_cast<vtkMimxSelectCellsWidget*>(w);
  //if(self->RubberBandStyle->GetEnabled())
  //{
  self->RubberBandStyle->SetEnabled(1);
  //      self->RubberBandStyle->OnChar();

  int *size;
  size = self->Interactor->GetRenderWindow()->GetSize();
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];
  self->PickX0 = X;
  self->PickY0 = Y;

  //cout << "picked at " <<X<<"  "<<Y<<endl;
  // Okay, make sure that the pick is in the current renderer
  if ( !self->CurrentRenderer || !self->CurrentRenderer->IsInViewport(X, Y) )
    {
    self->WidgetEvent = vtkMimxSelectCellsWidget::Outside;
    return;
    }

  self->WidgetEvent = vtkMimxSelectCellsWidget::LeftMouseButtonDown;
  self->RubberBandStyle->GetInteractor()->SetKeyCode('r');
  self->RubberBandStyle->OnChar();
  self->RubberBandStyle->OnLeftButtonDown();
  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  self->Interactor->Render();
}
//---------------------------------------------------------------------------------
void vtkMimxSelectCellsWidget::LeftButtonUpCallback(vtkAbstractWidget *w)
{
  vtkMimxSelectCellsWidget *self = reinterpret_cast<vtkMimxSelectCellsWidget*>(w);
  int *size;
  size = self->Interactor->GetRenderWindow()->GetSize();

  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  self->PickX1 = X;
  self->PickY1 = Y;
  // cout <<X<<"  "<<Y<<endl;

  if ( self->WidgetEvent == vtkMimxSelectCellsWidget::Outside ||
       self->WidgetEvent == vtkMimxSelectCellsWidget::Start )
    {
    return;
    }


  self->WidgetEvent = vtkMimxSelectCellsWidget::Start;

  //if(self->PreviousSelectedSubsetActor)
  //{
  double x0 = self->PickX0;
  double y0 = self->PickY0;
  double x1 = self->PickX1;
  double y1 = self->PickY1;
  self->AreaPicker->AreaPick(static_cast<int>(x0), static_cast<int>(y0), 
                             static_cast<int>(x1), static_cast<int>(y1), self->CurrentRenderer);

  vtkUnstructuredGrid *selectedgrid = vtkUnstructuredGrid::SafeDownCast(
    self->CurrentSelectedSubsetActor->GetMapper()->GetInput());
  vtkExtractSelectedFrustum *Extract = vtkExtractSelectedFrustum::New();
  Extract->SetInput(selectedgrid);

  Extract->SetFrustum(self->AreaPicker->GetFrustum());
  Extract->Update();

  vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::SafeDownCast(
    Extract->GetOutput());
  if(ugrid->GetNumberOfCells() <1)
    {
    Extract->Delete();
    }
  else
    {
    // to remove arrays introduce by vtk
    if(ugrid->GetCellData()->GetArray("vtkOriginalCellIds"))
      {
      ugrid->GetCellData()->RemoveArray("vtkOriginalCellIds");
      }
    if(ugrid->GetPointData()->GetArray("vtkOriginalPointIds"))
      {
      ugrid->GetPointData()->RemoveArray("vtkOriginalPointIds");
      }

    // hack because vtkExtractSelectedFrustum introduces duplicate points
    vtkMimxEquivalancePoints *equivalance = vtkMimxEquivalancePoints::New();
    equivalance->SetInput(ugrid);
    equivalance->Update();

    // for original point and cell ids
    // idea is to manage the mapping at the picking level itself
    vtkMimxMapOriginalCellAndPointIds *mapcellspoints = 
      vtkMimxMapOriginalCellAndPointIds::New();
    mapcellspoints->SetPartialMesh(equivalance->GetOutput());
    mapcellspoints->SetCompleteMesh(self->Inputset);
    mapcellspoints->Update();
    //
    self->CurrentSelectedSubset->DeepCopy(equivalance->GetOutput());
    Extract->Delete();
    mapcellspoints->Delete();
    equivalance->Delete();
    }
  self->CurrentSelectedSubset->Modified();
  self->RubberBandStyle->GetInteractor()->SetKeyCode('r');
  self->RubberBandStyle->OnChar();
  self->RubberBandStyle->OnLeftButtonUp();
  self->RubberBandStyle->SetEnabled(0);
  self->EventCallbackCommand->SetAbortFlag(1);
  self->EndInteraction();
  self->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
  self->Interactor->Render();
  self->SetEnabled(0, self);
  //}
}
//-----------------------------------------------------------------------------------
void vtkMimxSelectCellsWidget::Initialize()
{
  //if(!this->InputActor)
  //{
  //      vtkErrorMacro("Actor should be set");
  //      return;
  //}

  //if(this->PreviousSelectedSubsetActor)
  //{
  //      if(this->CurrentRenderer)
  //              this->CurrentRenderer->RemoveActor(this->PreviousSelectedSubsetActor);
  //      this->PreviousSelectedSubsetActor->Delete();
  //      this->Interactor->Render();
  //}
  //this->PreviousSelectedSubsetActor = NULL;

  //if(this->PreviousSelectedSubset)
  //      this->PreviousSelectedSubset->Delete();
  //this->PreviousSelectedSubset = NULL;

  //if(this->PreviousSelectedSubsetMapper)
  //      this->PreviousSelectedSubsetMapper->Delete();
  //this->PreviousSelectedSubsetMapper = NULL;

  //vtkDataSet *DataSet = this->InputActor->GetMapper()->GetInput();

  //if(DataSet->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
  //{
  //      // make two copies
  //      this->PreviousSelectedSubset = vtkUnstructuredGrid::New();
  //      this->PreviousSelectedSubset->DeepCopy(vtkUnstructuredGrid::
  //              SafeDownCast(DataSet));
  //      int i;
  //      vtkIntArray *NodeArray = vtkIntArray::New();
  //      NodeArray->SetNumberOfValues(
  //              this->PreviousSelectedSubset->GetNumberOfPoints());
  //      // set the original point ids
  //      for (i=0; i<this->PreviousSelectedSubset->GetNumberOfPoints();
  //              i++)
  //      {
  //              NodeArray->SetValue(i,i);
  //      }
  //      NodeArray->SetName("Original_Point_Ids");
  //      this->PreviousSelectedSubset->GetPointData()->AddArray(NodeArray);
  //      NodeArray->Delete();

  //      vtkIntArray *CellArray = vtkIntArray::New();
  //      CellArray->SetNumberOfValues(
  //              this->PreviousSelectedSubset->GetNumberOfCells());
  //      // set the original point ids
  //      for (i=0; i<this->PreviousSelectedSubset->GetNumberOfCells();
  //              i++)
  //      {
  //              CellArray->SetValue(i,i);
  //      }
  //      CellArray->SetName("Original_Cell_Ids");
  //      this->PreviousSelectedSubset->GetCellData()->AddArray(CellArray);
  //      CellArray->Delete();
  //      this->PreviousSelectedSubsetMapper = vtkDataSetMapper::New();
  //      this->PreviousSelectedSubsetMapper->SetInput(this->PreviousSelectedSubset);
  //      this->PreviousSelectedSubsetActor = vtkActor::New();
  //      this->PreviousSelectedSubsetActor->SetMapper(this->PreviousSelectedSubsetMapper);
  //      this->PreviousSelectedSubsetActor->GetProperty()->SetRepresentationToWireframe();
  //}
  //else{
  //      vtkErrorMacro("Only unstructuredgrid data type is allowed");
  //      return;
  //}
}
//-----------------------------------------------------------------------------------------------
void vtkMimxSelectCellsWidget::AcceptSelectedMesh(vtkMimxSelectCellsWidget *vtkNotUsed(self))
{
  //if(self->CurrentSelectedSubsetActor)
  //{
  //      self->CurrentRenderer->RemoveActor(self->CurrentSelectedSubsetActor);
  //      self->CurrentRenderer->RemoveActor(self->PreviousSelectedSubsetActor);
  //      self->CurrentRenderer->RemoveActor(self->InputActor);
  //      if(self->PreviousSelectedSubset)
  //              self->PreviousSelectedSubset->Delete();
  //      self->PreviousSelectedSubset = vtkUnstructuredGrid::New();
  //      self->PreviousSelectedSubset->DeepCopy(
  //              self->CurrentSelectedSubset);

  //      if(self->PreviousSelectedSubsetMapper)
  //              self->PreviousSelectedSubsetMapper->Delete();
  //      self->PreviousSelectedSubsetMapper = vtkDataSetMapper::New();
  //      self->PreviousSelectedSubsetMapper->SetInput(
  //              self->PreviousSelectedSubset);

  //      if(self->PreviousSelectedSubsetActor)
  //              self->PreviousSelectedSubsetActor->Delete();
  //      self->PreviousSelectedSubsetActor = vtkActor::New();
  //      self->PreviousSelectedSubsetActor->SetMapper(
  //              self->PreviousSelectedSubsetMapper);

  //      self->CurrentRenderer->AddActor(self->PreviousSelectedSubsetActor);
  //      self->Interactor->Render();
  //}
}
//-------------------------------------------------------------------------------------------------
void vtkMimxSelectCellsWidget::SetInputAndCurrentSelectedMesh(
  vtkUnstructuredGrid *Input, vtkUnstructuredGrid *CurrSelSubset)
{
  this->Inputset = Input;
//      this->CurrentSelectedSubset->Initialize();
  this->CurrentSelectedSubset->DeepCopy(CurrSelSubset);
  this->CurrentSelectedSubset->Modified();
}
//-----------------------------------------------------------------------------------------------
void vtkMimxSelectCellsWidget::MouseMoveCallback(vtkAbstractWidget *w)
{
  vtkMimxSelectCellsWidget *self = reinterpret_cast<vtkMimxSelectCellsWidget*>(w);
  self->RubberBandStyle->OnMouseMove();
}
//------------------------------------------------------------------------------------------------
void vtkMimxSelectCellsWidget::SetEnabled(int enabling, vtkMimxSelectCellsWidget *self)
{
  if ( !self->Interactor )
    {
    vtkErrorMacro(<<"The interactor must be set prior to enabling/disabling widget");
    return;
    }

  if ( enabling ) //------------------------------------------------------------
    {
    //              self->CurrentStyle = self->Interactor->GetInteractorStyle();

    if ( self->Enabled ) //already enabled, just return
      {
      return;
      }

    if ( !self->CurrentRenderer )
      {
      self->SetCurrentRenderer(self->Interactor->FindPokedRenderer(
                                 self->Interactor->GetLastEventPosition()[0],
                                 self->Interactor->GetLastEventPosition()[1]));
      if ( self->CurrentRenderer == NULL )
        {
        return;
        }
      }
    //if(!self->PreviousSelectedSubset)
    //{
    //      vtkErrorMacro("If enabling the widget for the first time, Call functions SetInputActor(), Initialize() and then call SetEnabled()");
    //      return;
    //}
    self->Enabled = 1;
    // set all the interactions
    if(self->CurrentSelectedSubsetActor)
      {
//                      self->CurrentRenderer->RemoveActor(self->CurrentSelectedSubsetActor);
      }
    //      self->CurrentRenderer->RemoveActor(self->PreviousSelectedSubsetActor);
    self->CurrentRenderer->AddActor(self->CurrentSelectedSubsetActor);
    self->CurrentSelectedSubsetActor->GetProperty()->SetLineWidth(
      self->CurrentSelectedSubsetActor->GetProperty()->GetLineWidth()*2.0);

    //      char KeyCode = 'r";
    // Listen for the following events
    if ( ! self->Parent )
      {
      self->EventTranslator->AddEventsToInteractor(self->Interactor,
                                                   self->EventCallbackCommand,self->Priority);
      }
    else
      {
      self->EventTranslator->AddEventsToParent(self->Parent,
                                               self->EventCallbackCommand,self->Priority);
      }

    if ( !self->Interactor )
      {
      vtkErrorMacro(<<"The interactor must be set prior to enabling/disabling widget");
      return;
      }

    self->Interactor->SetInteractorStyle(self->RubberBandStyle);
    self->RubberBandStyle->SetEnabled(1);

    self->Interactor->SetPicker(self->AreaPicker);
    }

  else //disabling----------------------------------------------------------
    {
    vtkDebugMacro(<<"Disabling selectcells widget");

    if ( !self->Enabled ) //already disabled, just return
      {
      return;
      }
    //self->CurrentRenderer->RemoveActor(self->PreviousSelectedSubsetActor);
    if(self->CurrentSelectedSubsetActor)
      {
//                      self->CurrentRenderer->RemoveActor(self->CurrentSelectedSubsetActor);
      self->CurrentSelectedSubsetActor->GetProperty()->SetLineWidth(
        self->CurrentSelectedSubsetActor->GetProperty()->GetLineWidth()/2.0);
      }
    // don't listen for events any more
    if ( ! self->Parent )
      {
      self->Interactor->RemoveObserver(self->EventCallbackCommand);
      }
    else
      {
      self->Parent->RemoveObserver(self->EventCallbackCommand);
      }
    self->Enabled = 0;
    }

  self->Interactor->Render();
}
//------------------------------------------------------------------------------------------------
void vtkMimxSelectCellsWidget::SetInput(vtkUnstructuredGrid *Input)
{
  this->Inputset = Input; 
        
  this->CurrentSelectedSubset->DeepCopy(Input);
  //vtkDataSetMapper::SafeDownCast(this->CurrentSelectedSubsetActor->GetMapper())
  //      ->SetInput(ugrid);

  int numPoints = Input->GetNumberOfPoints();

  int i;
  vtkIntArray *NodeArray = vtkIntArray::New();
  NodeArray->SetNumberOfValues(Input->GetNumberOfPoints());
  // set the original point ids
  for (i=0; i<numPoints;i++)
    {
    NodeArray->SetValue(i,i);
    }
  NodeArray->SetName("Original_Point_Ids");
  this->CurrentSelectedSubset->GetPointData()->AddArray(NodeArray);
  NodeArray->Delete();

  vtkIntArray *CellArray = vtkIntArray::New();
  CellArray->SetNumberOfValues(Input->GetNumberOfCells());
  // set the original point ids
  for (i=0; i<Input->GetNumberOfCells();i++)
    {
    CellArray->SetValue(i,i);
    }
  CellArray->SetName("Original_Cell_Ids");
  this->CurrentSelectedSubset->GetCellData()->AddArray(CellArray);
  CellArray->Delete();
  this->CurrentSelectedSubsetActor->GetProperty()->SetRepresentationToWireframe();
  this->CurrentSelectedSubset->Modified();
}
//---------------------------------------------------------------------------------------
