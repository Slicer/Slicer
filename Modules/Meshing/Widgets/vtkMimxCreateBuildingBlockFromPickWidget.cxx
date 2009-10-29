/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxCreateBuildingBlockFromPickWidget.cxx,v $
Language:  C++

Date:      $Date: 2008/07/14 23:02:20 $
Version:   $Revision: 1.4 $


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
#include "vtkMimxCreateBuildingBlockFromPickWidget.h"

#include "vtkActor.h"
#include "vtkAssemblyPath.h"
#include "vtkCallbackCommand.h"
#include "vtkCamera.h"
#include "vtkCell.h"
#include "vtkCellData.h"
#include "vtkCellLocator.h"
#include "vtkCellPicker.h"
#include "vtkCommand.h"
#include "vtkDataSetMapper.h"
#include "vtkEvent.h"
#include "vtkMath.h"
#include "vtkExtractSelectedFrustum.h"
#include "vtkGarbageCollector.h"
#include "vtkHandleWidget.h"
#include "vtkInteractorObserver.h"
#include "vtkInteractorStyleRubberBandPick.h"
#include "vtkObjectFactory.h"
#include "vtkPlane.h"
#include "vtkPoints.h"
#include "vtkPointPicker.h"
#include "vtkPolyData.h"
#include "vtkProperty.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderedAreaPicker.h"
#include "vtkRenderer.h"
#include "vtkRendererCollection.h"
#include "vtkRenderWindow.h"
#include "vtkUnstructuredGrid.h"
#include "vtkWidgetCallbackMapper.h" 
#include "vtkWidgetEvent.h"
#include "vtkWidgetEventTranslator.h"

#include "vtkUnstructuredGridWriter.h"

enum CubeOrientations 
{
  UNKNOWN_ORIENT = 0,
  X_Y_Z          = 1,
  X_Y_NEGZ       = 2,
  X_NEGY_Z       = 3,
  X_NEGY_NEGZ    = 4,
  X_Z_Y          = 5,
  X_Z_NEGY       = 6,
  X_NEGZ_Y       = 7,
  X_NEGZ_NEGY    = 8,
  NEGX_Y_Z       = 9,
  NEGX_Y_NEGZ    = 10,
  NEGX_NEGY_Z    = 11,
  NEGX_NEGY_NEGZ = 12,
  NEGX_Z_Y       = 13,
  NEGX_Z_NEGY    = 14,
  NEGX_NEGZ_Y    = 15,
  NEGX_NEGZ_NEGY = 16,
  Y_X_Z          = 17,
  Y_X_NEGZ       = 18,
  Y_NEGX_Z       = 19,
  Y_NEGX_NEGZ    = 20,
  Y_Z_X          = 21,
  Y_Z_NEGX       = 22,
  Y_NEGZ_X       = 23,
  Y_NEGZ_NEGX    = 24,
  NEGY_X_Z       = 25,
  NEGY_X_NEGZ    = 26,
  NEGY_NEGX_Z    = 27,
  NEGY_NEGX_NEGZ = 28,
  NEGY_Z_X       = 29,
  NEGY_Z_NEGX    = 30,    
  NEGY_NEGZ_X    = 31,
  NEGY_NEGZ_NEGX = 32,  
  Z_X_Y          = 33,
  Z_X_NEGY       = 34,
  Z_NEGX_Y       = 35,
  Z_NEGX_NEGY    = 36,
  Z_Y_X          = 37,
  Z_Y_NEGX       = 38,
  Z_NEGY_X       = 39,
  Z_NEGY_NEGX    = 40,
  NEGZ_X_Y       = 41,
  NEGZ_X_NEGY    = 42,
  NEGZ_NEGX_Y    = 43,
  NEGZ_NEGX_NEGY = 44,
  NEGZ_Y_X       = 45,
  NEGZ_Y_NEGX    = 46,
  NEGZ_NEGY_X    = 47,
  NEGZ_NEGY_NEGX = 48
};


vtkCxxRevisionMacro(vtkMimxCreateBuildingBlockFromPickWidget, "$Revision: 1.4 $");
vtkStandardNewMacro(vtkMimxCreateBuildingBlockFromPickWidget);

//----------------------------------------------------------------------
vtkMimxCreateBuildingBlockFromPickWidget::vtkMimxCreateBuildingBlockFromPickWidget()
{
  this->CallbackMapper->SetCallbackMethod(
            vtkCommand::LeftButtonPressEvent,
            vtkEvent::ControlModifier, 0, 1, NULL,
                        vtkMimxCreateBuildingBlockFromPickWidget::CrtlLeftMouseButtonDown,
                        this, vtkMimxCreateBuildingBlockFromPickWidget::CrtlLeftButtonDownCallback);

  this->CallbackMapper->SetCallbackMethod(
          vtkCommand::MouseMoveEvent,
          vtkEvent::ControlModifier, 0, 1, NULL,
          vtkMimxCreateBuildingBlockFromPickWidget::CrtlLeftMouseButtonMove,
          this, vtkMimxCreateBuildingBlockFromPickWidget::CrtlMouseMoveCallback);

  this->CallbackMapper->SetCallbackMethod(
            vtkCommand::LeftButtonReleaseEvent,
                        vtkEvent::ControlModifier, 0, 1, NULL,
            vtkMimxCreateBuildingBlockFromPickWidget::CrtlLeftMouseButtonUp,
            this, vtkMimxCreateBuildingBlockFromPickWidget::CrtlLeftButtonUpCallback);

  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
          vtkMimxCreateBuildingBlockFromPickWidget::LeftMouseButtonUp,
          this, vtkMimxCreateBuildingBlockFromPickWidget::LeftButtonUpCallback);

  this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonPressEvent,
          vtkMimxCreateBuildingBlockFromPickWidget::RightMouseButtonDown,
          this, vtkMimxCreateBuildingBlockFromPickWidget::RightButtonDownCallback);


  this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonReleaseEvent,
          vtkMimxCreateBuildingBlockFromPickWidget::RightMouseButtonUp,
          this, vtkMimxCreateBuildingBlockFromPickWidget::RightButtonUpCallback);

  this->RubberBandStyle =  vtkInteractorStyleRubberBandPick::New();
  this->PickStatus = 0;
  this->BuildingBlock = vtkUnstructuredGrid::New();
  this->BuildingBlockMapper = vtkDataSetMapper::New();
  this->BuildingBlockActor = vtkActor::New();
  this->BuildingBlockActor->GetProperty()->SetColor(1.0, 0.0, 0.0);
  this->BuildingBlockMapper->SetInput(this->BuildingBlock);
  this->BuildingBlockActor->SetMapper(this->BuildingBlockMapper);
  this->BuildingBlockActor->GetProperty()->SetRepresentationToWireframe();
  this->Input = NULL;
  this->ExtrusionLength = 1.0;
  this->PickX0 = 0;
  this->PickX1 = 0;
  this->PickY0 = 0;
  this->PickY1 = 0;
  this->normal[0] = this->normal[1] = this->normal[2] = 0.0;
  
}

//----------------------------------------------------------------------
vtkMimxCreateBuildingBlockFromPickWidget::~vtkMimxCreateBuildingBlockFromPickWidget()
{
        this->RubberBandStyle->Delete();
        this->BuildingBlock->Delete();
        this->BuildingBlockMapper->Delete();
        this->BuildingBlockActor->Delete();
}

//----------------------------------------------------------------------
void vtkMimxCreateBuildingBlockFromPickWidget::SetEnabled(int enabling)
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
//      this->CurrentRenderer->GetActiveCamera()->ParallelProjectionOn();
//      this->RubberBandStyle->SetEnabled(1);

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
        this->CurrentRenderer->RemoveActor(this->BuildingBlockActor);
        this->BuildingBlock->Initialize();
        this->BuildingBlock->Modified();
//      this->CurrentRenderer->GetActiveCamera()->ParallelProjectionOff();
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
void vtkMimxCreateBuildingBlockFromPickWidget::CrtlLeftButtonDownCallback(vtkAbstractWidget *w)
{
        vtkMimxCreateBuildingBlockFromPickWidget *self = reinterpret_cast<vtkMimxCreateBuildingBlockFromPickWidget*>(w);
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
                self->WidgetEvent = vtkMimxCreateBuildingBlockFromPickWidget::Outside;
                return;
        }

        self->WidgetEvent = vtkMimxCreateBuildingBlockFromPickWidget::CrtlLeftMouseButtonDown;
        self->CurrentRenderer->RemoveActor(self->BuildingBlockActor);
        self->BuildingBlock->Initialize();
        self->RubberBandStyle->GetInteractor()->SetKeyCode('r');
        self->RubberBandStyle->OnChar();
        self->RubberBandStyle->OnLeftButtonDown();
        //
        if(!self->Input)        return;

        self->EventCallbackCommand->SetAbortFlag(1);
        self->StartInteraction();
        self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
        self->PickStatus = 1;
        self->Interactor->Render();
}
//----------------------------------------------------------------------
void vtkMimxCreateBuildingBlockFromPickWidget::CrtlLeftButtonUpCallback(vtkAbstractWidget *w)
{
  vtkMimxCreateBuildingBlockFromPickWidget *self = reinterpret_cast<vtkMimxCreateBuildingBlockFromPickWidget*>(w);
  int *size;
  size = self->Interactor->GetRenderWindow()->GetSize();

  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  self->PickX1 = X;
  self->PickY1 = Y;

  if ( self->WidgetEvent == vtkMimxCreateBuildingBlockFromPickWidget::Outside ||
          self->WidgetEvent == vtkMimxCreateBuildingBlockFromPickWidget::Start )
  {
          return;
  }

  if(self->WidgetEvent == vtkMimxCreateBuildingBlockFromPickWidget::CrtlLeftMouseButtonDown)
  {
          self->WidgetEvent = vtkMimxCreateBuildingBlockFromPickWidget::Start;
          vtkCamera *camera = self->CurrentRenderer->IsActiveCameraCreated() ? self->CurrentRenderer->GetActiveCamera() : NULL;

          double position[3], frustumPlanes[24];
          double viewFocus[3], firstPickPoint[4], secondPickPoint[4], 
                  firstPoint[4], secondPoint[4], positionDisplay[3];
          int i;

          camera->GetPosition(position);
          camera->GetFocalPoint(viewFocus);
          camera->GetViewPlaneNormal(self->normal);
          double aspect[2];
          self->CurrentRenderer->GetAspect(aspect);
          camera->GetFrustumPlanes((aspect[0]/aspect[1]), frustumPlanes);
          double cuttingPlanePosition[3];
          for (i=0; i<3; i++)
          {
                  cuttingPlanePosition[i] = 0.0;
          }
          if(frustumPlanes[20] != 0.0)
          {
                  cuttingPlanePosition[0] = frustumPlanes[23]/frustumPlanes[20];
          }
          else if(frustumPlanes[21] != 0.0)
          {
                  cuttingPlanePosition[1] = frustumPlanes[23]/frustumPlanes[21];
          }
          else  cuttingPlanePosition[2] = frustumPlanes[23]/frustumPlanes[22];

          double lineIntersection[3];
          double t;
          vtkPlane::IntersectWithLine(position, viewFocus, self->normal, cuttingPlanePosition, t, lineIntersection);
          self->ComputeWorldToDisplay(position[0], position[1], position[2], positionDisplay);
          double focalDepth = positionDisplay[2];
          
          /*
    std::cout << "Pick Point X0: " << self->PickX0 << std::endl;
    std::cout << "Pick Point X1: " << self->PickX1 << std::endl;
    std::cout << "Pick Point Y0: " << self->PickY0 << std::endl;
    std::cout << "Pick Point Y1: " << self->PickY1 << std::endl;
    */
    
    if (self->PickX0 > self->PickX1)
    {
      vtkIdType tmp;
      tmp = self->PickX0;
      self->PickX0 = self->PickX1;
      self->PickX1 = tmp;
    }
    if (self->PickY0 < self->PickY1)
    {
      vtkIdType tmp;
      tmp = self->PickY0;
      self->PickY0 = self->PickY1;
      self->PickY1 = tmp;
    }
    
    /*
    std::cout << "Modified Point X0: " << self->PickX0 << std::endl;
    std::cout << "Modified Point X1: " << self->PickX1 << std::endl;
    std::cout << "Modified Point Y0: " << self->PickY0 << std::endl;
    std::cout << "Modified Point Y1: " << self->PickY1 << std::endl;
    */
          self->ComputeDisplayToWorld((double)self->PickX0, (double)self->PickY0, focalDepth, firstPickPoint);
          self->ComputeDisplayToWorld((double)self->PickX1, (double)self->PickY1, focalDepth, secondPickPoint);
          self->ComputeDisplayToWorld((double)self->PickX0, (double)self->PickY1, focalDepth, firstPoint);
          self->ComputeDisplayToWorld((double)self->PickX1, (double)self->PickY0, focalDepth, secondPoint);
        

          // place a plane at the position of the camera
          vtkPoints *points = vtkPoints::New();
          points->SetNumberOfPoints(4);
          points->SetPoint(0, firstPickPoint[0], firstPickPoint[1], firstPickPoint[2]);
          points->SetPoint(1, firstPoint[0], firstPoint[1], firstPoint[2]);
          points->SetPoint(2, secondPickPoint[0], secondPickPoint[1], secondPickPoint[2]);
          points->SetPoint(3, secondPoint[0], secondPoint[1], secondPoint[2]);
          vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
          ugrid->Allocate(1,1);
          vtkIdList *idlist = vtkIdList::New();
          idlist->SetNumberOfIds(4);
          for (i=0; i<4; i++)
          {
                  idlist->SetId(i,i);
          }
          ugrid->InsertNextCell(9, idlist);
          ugrid->SetPoints(points);
          points->Delete();
          idlist->Delete();
          ugrid->Delete();

          double x0 = self->PickX0;
          double y0 = self->PickY0;
          double x1 = self->PickX1;
          double y1 = self->PickY1;
          vtkRenderedAreaPicker *AreaPicker = vtkRenderedAreaPicker::New();
          AreaPicker->AreaPick(static_cast<int>(x0), static_cast<int>(y0), 
                  static_cast<int>(x1), static_cast<int>(y1), self->CurrentRenderer);
          vtkExtractSelectedFrustum *Extract = vtkExtractSelectedFrustum::New();
          Extract->SetInput(self->Input);
          Extract->SetFrustum(AreaPicker->GetFrustum());
          Extract->Update();
          ugrid = vtkUnstructuredGrid::SafeDownCast(Extract->GetOutput());

          if(ugrid->GetNumberOfCells() <1)
          {
                  AreaPicker->Delete();
                  Extract->Delete();
                  self->RubberBandStyle->GetInteractor()->SetKeyCode('r');
                  self->RubberBandStyle->OnChar();
                  self->RubberBandStyle->OnLeftButtonUp();
                  self->RubberBandStyle->SetEnabled(0);
                  self->EventCallbackCommand->SetAbortFlag(1);
                  self->EndInteraction();
                  self->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
                  self->PickStatus = 0;
                  self->Interactor->Render();
                  return;
          }

          double distmin = VTK_DOUBLE_MAX, dist;
          int minPointNum = -1;
          double ClosestPoint[3];
          for (i=0; i<ugrid->GetNumberOfPoints(); i++)
          {
                dist = vtkPlane::DistanceToPlane(ugrid->GetPoint(i), self->normal, position);
                if(dist < distmin)
                {
                        minPointNum = i;
                        distmin = dist;
                }
          }
        
          ugrid->GetPoint(minPointNum, ClosestPoint);
          self->ComputeWorldToDisplay(ClosestPoint[0], ClosestPoint[1], ClosestPoint[2], positionDisplay);
          focalDepth = positionDisplay[2];

    //std::cout << "Normal: " << self->normal[0] << " " << self->normal[1] << " " << self->normal[2] << std::endl;

          self->ComputeDisplayToWorld((double)self->PickX0, (double)self->PickY0, focalDepth, firstPickPoint);
          self->ComputeDisplayToWorld((double)self->PickX1, (double)self->PickY1, focalDepth, secondPickPoint);
          self->ComputeDisplayToWorld((double)self->PickX0, (double)self->PickY1, focalDepth, firstPoint);
          self->ComputeDisplayToWorld((double)self->PickX1, (double)self->PickY0, focalDepth, secondPoint);
          
          for (i=0;i<3;i++)
          {
            self->pickPoint1[i] = firstPickPoint[i];
      self->pickPoint2[i] = secondPickPoint[i];
      self->point1[i] = firstPoint[i];
      self->point2[i] = secondPoint[i];
          }
          

           // compute the 4 points by extruding the selected plane.
          double firstPickPointBack[3], secondPickPointBack[3], firstPointBack[3], secondPointBack[3];   
          for (i=0; i<3; i++)
          {
                firstPickPointBack[i] = firstPickPoint[i] - self->normal[i]*self->ExtrusionLength;
                secondPickPointBack[i] = secondPickPoint[i] - self->normal[i]*self->ExtrusionLength;
                firstPointBack[i] = firstPoint[i] - self->normal[i]*self->ExtrusionLength;
                secondPointBack[i] = secondPoint[i] - self->normal[i]*self->ExtrusionLength;
          }
          points = vtkPoints::New();
          points->SetNumberOfPoints(8);


    self->OrderBuildingBlockPoints(&(firstPickPoint[0]), &(secondPickPoint[0]), &(firstPoint[0]), &(secondPoint[0]),
                    &(firstPickPointBack[0]), &(secondPickPointBack[0]), &(firstPointBack[0]), &(secondPointBack[0]), points);
    
          
          self->BuildingBlock->Initialize();
          self->BuildingBlock->Allocate(1,1);
          idlist = vtkIdList::New();
          idlist->SetNumberOfIds(8);
          for (i=0; i<8; i++)
          {
                  idlist->SetId(i,i);
          }
          self->BuildingBlock->InsertNextCell(12, idlist);
          self->BuildingBlock->SetPoints(points);
          points->Delete();
          idlist->Delete();
          self->BuildingBlock->Modified();        
          Extract->Delete();
          AreaPicker->Delete();
          self->CurrentRenderer->AddActor(self->BuildingBlockActor);
//        camera->ParallelProjectionOff();
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
void vtkMimxCreateBuildingBlockFromPickWidget::CrtlMouseMoveCallback(vtkAbstractWidget *w)
{
        vtkMimxCreateBuildingBlockFromPickWidget *Self = 
                reinterpret_cast<vtkMimxCreateBuildingBlockFromPickWidget*>(w);
        if(Self->PickStatus)
        {
                Self->RubberBandStyle->OnMouseMove();
        }
}
//----------------------------------------------------------------------
void vtkMimxCreateBuildingBlockFromPickWidget::LeftButtonUpCallback(vtkAbstractWidget* w)
{
        vtkMimxCreateBuildingBlockFromPickWidget *Self = 
                reinterpret_cast<vtkMimxCreateBuildingBlockFromPickWidget*>(w);
        if(Self->PickStatus)
        {
                Self->CrtlLeftButtonUpCallback(w);
        }
}
//---------------------------------------------------------------------
void vtkMimxCreateBuildingBlockFromPickWidget::SetInput(vtkPolyData *input)
{
        this->Input = input;
}
//---------------------------------------------------------------------
void vtkMimxCreateBuildingBlockFromPickWidget::SetExtrusionLength(double length)
{
        this->ExtrusionLength = length;
        
        if ( (this->PickX0 > 0) && (this->PickX1 > 0) )
        {
        double firstPickPointBack[3], secondPickPointBack[3], firstPointBack[3], secondPointBack[3];     
          for (int i=0; i<3; i++)
          {
                firstPickPointBack[i] = this->pickPoint1[i] - this->normal[i]*this->ExtrusionLength;
                secondPickPointBack[i] = this->pickPoint2[i] - this->normal[i]*this->ExtrusionLength;
                firstPointBack[i] = this->point1[i] - this->normal[i]*this->ExtrusionLength;
                secondPointBack[i] = this->point2[i] - this->normal[i]*this->ExtrusionLength;
          }
          vtkPoints *points = vtkPoints::New();
          points->SetNumberOfPoints(8);
          
          this->OrderBuildingBlockPoints(&(this->pickPoint1[0]), &(this->pickPoint2[0]), &(this->point1[0]), &(this->point2[0]),
                    &(firstPickPointBack[0]), &(secondPickPointBack[0]), &(firstPointBack[0]), &(secondPointBack[0]), points);
                    
          
          vtkIdList *idlist = vtkIdList::New();
          idlist->SetNumberOfIds(8);
          for (int i=0; i<8; i++)
          {
                  idlist->SetId(i,i);
          }
          this->BuildingBlock->Initialize();
          this->BuildingBlock->Allocate(1,1);
          this->BuildingBlock->InsertNextCell(12, idlist);
          this->BuildingBlock->SetPoints(points);
          points->Delete();
          idlist->Delete();
          this->BuildingBlock->Modified();
          this->Interactor->Render();
        }
}

void vtkMimxCreateBuildingBlockFromPickWidget::OrderBuildingBlockPoints(
    double *firstPickPoint,
    double *secondPickPoint,
    double *firstPoint,
    double *secondPoint,
    double *firstPickPointBack,
    double *secondPickPointBack,
    double *firstPointBack,
    double *secondPointBack,
    vtkPoints *points)
{
  double xDir[3];
  xDir[0] = firstPointBack[0] - firstPickPointBack[0];
  xDir[1] = firstPointBack[1] - firstPickPointBack[1];
  xDir[2] = firstPointBack[2] - firstPickPointBack[2];
  //std::cout << "Direction 0 : " << xDir[0] << " " << xDir[1] << " " << xDir[2] << std::endl;
  
  double yDir[3];
  yDir[0] = secondPickPointBack[0] - firstPointBack[0];
  yDir[1] = secondPickPointBack[1] - firstPointBack[1];
  yDir[2] = secondPickPointBack[2] - firstPointBack[2];
  //std::cout << "Direction 1 : " << yDir[0] << " " << yDir[1] << " " << yDir[2] << std::endl;
  
  double zDir[3];
  zDir[0] = firstPickPoint[0] - firstPickPointBack[0];
  zDir[1] = firstPickPoint[1] - firstPickPointBack[1];
  zDir[2] = firstPickPoint[2] - firstPickPointBack[2];
  //std::cout << "Direction 2 : " << zDir[0] << " " << zDir[1] << " " << zDir[2] << std::endl;
  
  double xAxis[3] = {1,0,0};
  double yAxis[3] = {0,1,0};
  double zAxis[3] = {0,0,1};
  double xDotX, yDotX, zDotX;
  double xDotY, yDotY, zDotY;
  double xDotZ, yDotZ, zDotZ;
  int xSignX = 1;
  int ySignX = 1;
  int zSignX = 1;
  int xSignY = 1;
  int ySignY = 1;
  int zSignY = 1;
  int xSignZ = 1;
  int ySignZ = 1;
  int zSignZ = 1;
  
  /* Find the Primary X Axis */
  xDotX = fabs ( vtkMath::Dot(xDir, xAxis) );
  if ( vtkMath::Dot(xDir, xAxis) < 0.0 )
    xSignX = -1;
  
  yDotX = fabs ( vtkMath::Dot(yDir, xAxis) );
  if ( vtkMath::Dot(yDir, xAxis) < 0.0 )
    ySignX = -1;
  
  zDotX = fabs ( vtkMath::Dot(zDir, xAxis) );
  if ( vtkMath::Dot(zDir, xAxis) < 0.0 )
    zSignX = -1;
  
  /* Find the Primary Y Axis */
  xDotY = fabs ( vtkMath::Dot(xDir, yAxis) );
  if ( vtkMath::Dot(xDir, yAxis) < 0.0 )
    xSignY = -1;
  
  yDotY = fabs ( vtkMath::Dot(yDir, yAxis) );
  if ( vtkMath::Dot(yDir, yAxis) < 0.0 )
    ySignY = -1;
  
  zDotY = fabs ( vtkMath::Dot(zDir, yAxis) );
  if ( vtkMath::Dot(zDir, yAxis) < 0.0 )
    zSignY = -1;
 
  /* Find the Primary Z Axis */
  xDotZ = fabs ( vtkMath::Dot(xDir, zAxis) );
  if ( vtkMath::Dot(xDir, zAxis) < 0.0 )
    xSignZ = -1;
  
  yDotZ = fabs ( vtkMath::Dot(yDir, zAxis) );
  if ( vtkMath::Dot(yDir, zAxis) < 0.0 )
    ySignZ = -1;
  
  zDotZ = fabs ( vtkMath::Dot(zDir, zAxis) );
  if ( vtkMath::Dot(zDir, zAxis) < 0.0 )
    zSignZ = -1;
  
  int cubeAxis = UNKNOWN_ORIENT;
  
  /* Now Define the Cube Orientation */
  if ( (xDotX >= yDotX) && (xDotX >= zDotX) && (xSignX == 1))
  {
    if ( (yDotY >= zDotY) && (ySignY == 1) )
    {
      if (zSignZ == 1)
        cubeAxis = X_Y_Z;
      else
        cubeAxis = X_Y_NEGZ;
    }
    else if ( (yDotY >= zDotY) && (ySignY == -1) )
    {
      if (zSignZ == 1)
        cubeAxis = X_NEGY_Z;
      else
        cubeAxis = X_NEGY_NEGZ;
    }
    else if ( ( yDotY < zDotY) && (zSignY == 1) )
    {
      if (ySignZ == 1)
        cubeAxis = X_Z_Y;
      else
        cubeAxis = X_Z_NEGY;
    }
    else if ( ( yDotY < zDotY) && (zSignY == -1) )
    {
      if (ySignZ == 1)
        cubeAxis = X_NEGZ_Y;
      else
        cubeAxis = X_NEGZ_NEGY;
    }
  }
  else if ( (xDotX >= yDotX) && (xDotX >= zDotX) && (xSignX == -1))
  {
    if ( (yDotY >= zDotY) && (ySignY == 1) )
    {
      if (zSignZ == 1)
        cubeAxis = NEGX_Y_Z;
      else
        cubeAxis = NEGX_Y_NEGZ;
    }
    else if ( (yDotY >= zDotY) && (ySignY == -1) )
    {
      if (zSignZ == 1)
        cubeAxis = NEGX_NEGY_Z;
      else
        cubeAxis = NEGX_NEGY_NEGZ;
    }
    else if ( ( yDotY < zDotY) && (zSignY == 1) )
    {
      if (ySignZ == 1)
        cubeAxis = NEGX_Z_Y;
      else
        cubeAxis = NEGX_Z_NEGY;
    }
    else if ( ( yDotY < zDotY) && (zSignY == -1) )
    {
      if (ySignZ == 1)
        cubeAxis = NEGX_NEGZ_Y;
      else
        cubeAxis = NEGX_NEGZ_NEGY;
    }
  }
  else if ( (yDotX >= xDotX) && (yDotX >= zDotX) && (ySignX == 1))
  {
    if ( (xDotY >= zDotY) && (xSignY == 1) )
    {
      if (zSignZ == 1)
        cubeAxis = Y_X_Z;
      else
        cubeAxis = Y_X_NEGZ;
    }
    else if ( (xDotY >= zDotY) && (xSignY == -1) )
    {
      if (zSignZ == 1)
        cubeAxis = Y_NEGX_Z;
      else
        cubeAxis = Y_NEGX_NEGZ;
    }
    else if ( ( xDotY < zDotY) && (zSignY == 1) )
    {
      if (xSignZ == 1)
        cubeAxis = Y_Z_X;
      else
        cubeAxis = Y_Z_NEGX;
    }
    else if ( ( xDotY < zDotY) && (zSignY == -1) )
    {
      if (xSignZ == 1)
        cubeAxis = Y_NEGZ_X;
      else
        cubeAxis = Y_NEGZ_NEGX;
    }
  }
  else if ( (yDotX >= xDotX) && (yDotX >= zDotX) && (ySignX == -1))
  {
    if ( (xDotY >= zDotY) && (xSignY == 1) )
    {
      if (zSignZ == 1)
        cubeAxis = NEGY_X_Z;
      else
        cubeAxis = NEGY_X_NEGZ;
    }
    else if ( (xDotY >= zDotY) && (xSignY == -1) )
    {
      if (zSignZ == 1)
        cubeAxis = NEGY_NEGX_Z;
      else
        cubeAxis = NEGY_NEGX_NEGZ;
    }
    else if ( ( xDotY < zDotY) && (zSignY == 1) )
    {
      if (xSignZ == 1)
        cubeAxis = NEGY_Z_X;
      else
        cubeAxis = NEGY_Z_NEGX;    
    }
    else if ( ( xDotY < zDotY) && (zSignY == -1) )
    {
      if (xSignZ == 1)
        cubeAxis = NEGY_NEGZ_X;
      else
        cubeAxis = NEGY_NEGZ_NEGX;  
    }
  }
  else if ( (zDotX >= xDotX) && (zDotX >= yDotX) && (zSignX == 1))
  {
    if ( (xDotY >= yDotY) && (xSignY == 1) )
    {
      if (ySignZ == 1)
        cubeAxis = Z_X_Y;
      else
        cubeAxis = Z_X_NEGY;
    }
    else if ( (xDotY >= yDotY) && (xSignY == -1) )
    {
      if (ySignZ == 1)
        cubeAxis = Z_NEGX_Y;
      else
        cubeAxis = Z_NEGX_NEGY;
    }
    else if ( ( xDotY < yDotY) && (ySignY == 1) )
    {
      if (xSignZ == 1)
        cubeAxis = Z_Y_X;
      else
        cubeAxis = Z_Y_NEGX;
    }
    else if ( ( xDotY < yDotY) && (ySignY == -1) )
    {
      if (xSignZ == 1)
        cubeAxis = Z_NEGY_X;
      else
        cubeAxis = Z_NEGY_NEGX;
    }
  }
  else if ( (zDotX >= xDotX) && (zDotX >= yDotX) && (zSignX == -1))
  {
    if ( (xDotY >= yDotY) && (xSignY == 1) )
    {
      if (ySignZ == 1)
        cubeAxis = NEGZ_X_Y;
      else
        cubeAxis = NEGZ_X_NEGY;
    }
    else if ( (xDotY >= yDotY) && (xSignY == -1) )
    {
      if (ySignZ == 1)
        cubeAxis = NEGZ_NEGX_Y;
      else
        cubeAxis = NEGZ_NEGX_NEGY;
    }
    else if ( ( xDotY < yDotY) && (ySignY == 1) )
    {
      if (xSignZ == 1)
        cubeAxis = NEGZ_Y_X;
      else
        cubeAxis = NEGZ_Y_NEGX;
    }
    else if ( ( xDotY < yDotY) && (ySignY == -1) )
    {
      if (xSignZ == 1)
        cubeAxis = NEGZ_NEGY_X;
      else
        cubeAxis = NEGZ_NEGY_NEGX;
    }
  }
  
  switch ( cubeAxis )
  {
    case X_Y_Z:
      /* (X, Y, Z) */
          points->SetPoint(0, firstPickPointBack);
          points->SetPoint(1, firstPickPoint[0], firstPickPoint[1], firstPickPoint[2]);
          points->SetPoint(2, firstPoint[0], firstPoint[1], firstPoint[2]);
          points->SetPoint(3, firstPointBack); 
          points->SetPoint(4, secondPointBack);
          points->SetPoint(5, secondPoint[0], secondPoint[1], secondPoint[2]);
          points->SetPoint(6, secondPickPoint[0], secondPickPoint[1], secondPickPoint[2]);
          points->SetPoint(7, secondPickPointBack);
          break;
        case X_NEGY_NEGZ:
        /* (X, -Y, -Z) */
          points->SetPoint(0, secondPoint[0], secondPoint[1], secondPoint[2]);
          points->SetPoint(1, secondPointBack);
          points->SetPoint(2, secondPickPointBack);
          points->SetPoint(3, secondPickPoint[0], secondPickPoint[1], secondPickPoint[2]);
          points->SetPoint(4, firstPickPoint[0], firstPickPoint[1], firstPickPoint[2]);
          points->SetPoint(5, firstPickPointBack);
          points->SetPoint(6, firstPointBack); 
          points->SetPoint(7, firstPoint[0], firstPoint[1], firstPoint[2]);
          break;
        case X_NEGZ_Y:
          /* (X, Z, -Y) */
          points->SetPoint(0, firstPickPoint[0], firstPickPoint[1], firstPickPoint[2]);
          points->SetPoint(1, secondPoint[0], secondPoint[1], secondPoint[2]);
          points->SetPoint(2, secondPickPoint[0], secondPickPoint[1], secondPickPoint[2]);
          points->SetPoint(3, firstPoint[0], firstPoint[1], firstPoint[2]);
          points->SetPoint(4, firstPickPointBack);
          points->SetPoint(5, secondPointBack); 
          points->SetPoint(6, secondPickPointBack);
          points->SetPoint(7, firstPointBack);
          break;
        case X_Z_NEGY:
          /* (X, -Z, Y) */
          points->SetPoint(0, secondPointBack);
          points->SetPoint(1, firstPickPointBack);
          points->SetPoint(2, firstPointBack); 
          points->SetPoint(3, secondPickPointBack);
          points->SetPoint(4, secondPoint[0], secondPoint[1], secondPoint[2]);
          points->SetPoint(5, firstPickPoint[0], firstPickPoint[1], firstPickPoint[2]);
          points->SetPoint(6, firstPoint[0], firstPoint[1], firstPoint[2]);
          points->SetPoint(7, secondPickPoint[0], secondPickPoint[1], secondPickPoint[2]);
          break;
        case NEGX_Y_NEGZ:
          /* (-X, Y, -Z) */
          points->SetPoint(0, firstPoint[0], firstPoint[1], firstPoint[2]);
          points->SetPoint(1, firstPointBack); 
          points->SetPoint(2, firstPickPointBack);
          points->SetPoint(3, firstPickPoint[0], firstPickPoint[1], firstPickPoint[2]);
          points->SetPoint(4, secondPickPoint[0], secondPickPoint[1], secondPickPoint[2]);
          points->SetPoint(5, secondPickPointBack);
          points->SetPoint(6, secondPointBack);
          points->SetPoint(7, secondPoint[0], secondPoint[1], secondPoint[2]);
          break;
        case NEGX_NEGY_Z:
          /* (-X, -Y, Z) */
          points->SetPoint(0, secondPickPointBack);
          points->SetPoint(1, secondPickPoint[0], secondPickPoint[1], secondPickPoint[2]);
          points->SetPoint(2, secondPoint[0], secondPoint[1], secondPoint[2]);
          points->SetPoint(3, secondPointBack);
          points->SetPoint(4, firstPointBack); 
          points->SetPoint(5, firstPoint[0], firstPoint[1], firstPoint[2]);
          points->SetPoint(6, firstPickPoint[0], firstPickPoint[1], firstPickPoint[2]);
          points->SetPoint(7, firstPickPointBack);
          break;
        case NEGX_Z_Y:
          /* (-X, Z, Y) */
          points->SetPoint(0, firstPointBack);
          points->SetPoint(1, secondPickPointBack);
          points->SetPoint(2, secondPointBack); 
          points->SetPoint(3, firstPickPointBack);
          points->SetPoint(4, firstPoint[0], firstPoint[1], firstPoint[2]);
          points->SetPoint(5, secondPickPoint[0], secondPickPoint[1], secondPickPoint[2]);
          points->SetPoint(6, secondPoint[0], secondPoint[1], secondPoint[2]);
          points->SetPoint(7, firstPickPoint[0], firstPickPoint[1], firstPickPoint[2]);
          break;
        case NEGX_NEGZ_NEGY:
          /* (-X, -Z, -Y) */
          points->SetPoint(0, secondPickPoint[0], secondPickPoint[1], secondPickPoint[2]);
          points->SetPoint(1, firstPoint[0], firstPoint[1], firstPoint[2]);
          points->SetPoint(2, firstPickPoint[0], firstPickPoint[1], firstPickPoint[2]);
          points->SetPoint(3, secondPoint[0], secondPoint[1], secondPoint[2]);
          points->SetPoint(4, secondPickPointBack);
          points->SetPoint(5, firstPointBack);
          points->SetPoint(6, firstPickPointBack);
          points->SetPoint(7, secondPointBack); 
          break;
        case Y_X_NEGZ:
          /* (Y, X, -Z) */
          points->SetPoint(0, firstPickPoint[0], firstPickPoint[1], firstPickPoint[2]);
          points->SetPoint(1, firstPickPointBack);
          points->SetPoint(2, secondPointBack);
          points->SetPoint(3, secondPoint[0], secondPoint[1], secondPoint[2]);
          points->SetPoint(4, firstPoint[0], firstPoint[1], firstPoint[2]);
          points->SetPoint(5, firstPointBack);
          points->SetPoint(6, secondPickPointBack);
          points->SetPoint(7, secondPickPoint[0], secondPickPoint[1], secondPickPoint[2]);
          break;
        case NEGY_X_Z:
          /* (Y, -X, Z) */
          points->SetPoint(0, secondPointBack);
          points->SetPoint(1, secondPoint[0], secondPoint[1], secondPoint[2]);
          points->SetPoint(2, firstPickPoint[0], firstPickPoint[1], firstPickPoint[2]);
          points->SetPoint(3, firstPickPointBack);
          points->SetPoint(4, secondPickPointBack);
          points->SetPoint(5, secondPickPoint[0], secondPickPoint[1], secondPickPoint[2]);
          points->SetPoint(6, firstPoint[0], firstPoint[1], firstPoint[2]);
          points->SetPoint(7, firstPointBack);
          break;
        case Z_X_Y:
          /* (Y, Z, X) */
          points->SetPoint(0, firstPickPointBack);
          points->SetPoint(1, secondPointBack);
          points->SetPoint(2, secondPoint[0], secondPoint[1], secondPoint[2]);
          points->SetPoint(3, firstPickPoint[0], firstPickPoint[1], firstPickPoint[2]);
          points->SetPoint(4, firstPointBack);
          points->SetPoint(5, secondPickPointBack);
          points->SetPoint(6, secondPickPoint[0], secondPickPoint[1], secondPickPoint[2]);
          points->SetPoint(7, firstPoint[0], firstPoint[1], firstPoint[2]); 
          break;
        case NEGZ_X_NEGY:
          /* (Y, -Z, -X) */
          points->SetPoint(0, secondPoint[0], secondPoint[1], secondPoint[2]);
          points->SetPoint(1, firstPickPoint[0], firstPickPoint[1], firstPickPoint[2]);   
          points->SetPoint(2, firstPickPointBack);
          points->SetPoint(3, secondPointBack);
          points->SetPoint(4, secondPickPoint[0], secondPickPoint[1], secondPickPoint[2]);
          points->SetPoint(5, firstPoint[0], firstPoint[1], firstPoint[2]);  
          points->SetPoint(6, firstPointBack);
          points->SetPoint(7, secondPickPointBack);
          break;
        case Y_NEGX_Z:
          /* (-Y, X, Z) */ 
          points->SetPoint(0, firstPointBack);
          points->SetPoint(1, firstPoint[0], firstPoint[1], firstPoint[2]);
          points->SetPoint(2, secondPickPoint[0], secondPickPoint[1], secondPickPoint[2]);
          points->SetPoint(3, secondPickPointBack);
          points->SetPoint(4, firstPickPointBack);
          points->SetPoint(5, firstPickPoint[0], firstPickPoint[1], firstPickPoint[2]);
          points->SetPoint(6, secondPoint[0], secondPoint[1], secondPoint[2]);
          points->SetPoint(7, secondPointBack);
          break;
        case NEGY_NEGX_NEGZ:
          /* (-Y, -X, -Z) */ 
          points->SetPoint(0, secondPickPoint[0], secondPickPoint[1], secondPickPoint[2]);
          points->SetPoint(1, secondPickPointBack);
          points->SetPoint(2, firstPointBack);
          points->SetPoint(3, firstPoint[0], firstPoint[1], firstPoint[2]);
          points->SetPoint(4, secondPoint[0], secondPoint[1], secondPoint[2]);
          points->SetPoint(5, secondPointBack);
          points->SetPoint(6, firstPickPointBack);
          points->SetPoint(7, firstPickPoint[0], firstPickPoint[1], firstPickPoint[2]);
          break;
        case NEGZ_NEGX_Y:
          /* (-Y, Z, -X) */
          points->SetPoint(0, firstPoint[0], firstPoint[1], firstPoint[2]);
          points->SetPoint(1, secondPickPoint[0], secondPickPoint[1], secondPickPoint[2]);
          points->SetPoint(2, secondPickPointBack);
          points->SetPoint(3, firstPointBack);
          points->SetPoint(4, firstPickPoint[0], firstPickPoint[1], firstPickPoint[2]);
          points->SetPoint(5, secondPoint[0], secondPoint[1], secondPoint[2]);
          points->SetPoint(6, secondPointBack);
          points->SetPoint(7, firstPickPointBack);
          break;
        case Z_NEGX_NEGY:
          /* (-Y, -Z, X) */
          points->SetPoint(0, secondPickPointBack);
          points->SetPoint(1, firstPointBack);
          points->SetPoint(2, firstPoint[0], firstPoint[1], firstPoint[2]);
          points->SetPoint(3, secondPickPoint[0], secondPickPoint[1], secondPickPoint[2]);
          points->SetPoint(4, secondPointBack);
          points->SetPoint(5, firstPickPointBack);
          points->SetPoint(6, firstPickPoint[0], firstPickPoint[1], firstPickPoint[2]);     
          points->SetPoint(7, secondPoint[0], secondPoint[1], secondPoint[2]);
          break;
        case Y_Z_X:
          /* (Z,X,Y) */
          points->SetPoint(0, firstPickPointBack);
          points->SetPoint(1, firstPointBack);
          points->SetPoint(2, secondPickPointBack);
          points->SetPoint(3, secondPointBack); 
          points->SetPoint(4, firstPickPoint[0], firstPickPoint[1], firstPickPoint[2]);
          points->SetPoint(5, firstPoint[0], firstPoint[1], firstPoint[2]);
          points->SetPoint(6, secondPickPoint[0], secondPickPoint[1], secondPickPoint[2]);
          points->SetPoint(7, secondPoint[0], secondPoint[1], secondPoint[2]);
          break;
        case NEGY_NEGZ_X:
          /* (Z,-X,-Y) */
          points->SetPoint(0, secondPoint[0], secondPoint[1], secondPoint[2]);
          points->SetPoint(1, secondPickPoint[0], secondPickPoint[1], secondPickPoint[2]);
          points->SetPoint(2, firstPoint[0], firstPoint[1], firstPoint[2]);
          points->SetPoint(3, firstPickPoint[0], firstPickPoint[1], firstPickPoint[2]);
          points->SetPoint(4, secondPointBack); 
          points->SetPoint(5, secondPickPointBack);
          points->SetPoint(6, firstPointBack);
          points->SetPoint(7, firstPickPointBack);
          break;
        case NEGZ_Y_X:
          /* (Z, Y, -X) */
          points->SetPoint(0, firstPickPoint[0], firstPickPoint[1], firstPickPoint[2]);
          points->SetPoint(1, firstPoint[0], firstPoint[1], firstPoint[2]);
          points->SetPoint(2, firstPointBack);
          points->SetPoint(3, firstPickPointBack);
          points->SetPoint(4, secondPoint[0], secondPoint[1], secondPoint[2]);
          points->SetPoint(5, secondPickPoint[0], secondPickPoint[1], secondPickPoint[2]);
          points->SetPoint(6, secondPickPointBack);
          points->SetPoint(7, secondPointBack);
          break;
        case Z_NEGY_X:
          /* (Z, -Y, X) */
          points->SetPoint(0, secondPointBack);
          points->SetPoint(1, secondPickPointBack);
          points->SetPoint(2, secondPickPoint[0], secondPickPoint[1], secondPickPoint[2]);
          points->SetPoint(3, secondPoint[0], secondPoint[1], secondPoint[2]);
          points->SetPoint(4, firstPickPointBack);
          points->SetPoint(5, firstPointBack);
          points->SetPoint(6, firstPoint[0], firstPoint[1], firstPoint[2]);
          points->SetPoint(7, firstPickPoint[0], firstPickPoint[1], firstPickPoint[2]);
          break;
        case Y_NEGZ_NEGX:
          /* (-Z,X,-Y) */
          points->SetPoint(0, firstPoint[0], firstPoint[1], firstPoint[2]);
          points->SetPoint(1, firstPickPoint[0], firstPickPoint[1], firstPickPoint[2]);
          points->SetPoint(2, secondPoint[0], secondPoint[1], secondPoint[2]);
          points->SetPoint(3, secondPickPoint[0], secondPickPoint[1], secondPickPoint[2]);
          points->SetPoint(4, firstPointBack);
          points->SetPoint(5, firstPickPointBack);
          points->SetPoint(6, secondPointBack); 
          points->SetPoint(7, secondPickPointBack);
          break;
        case NEGY_Z_NEGX:
          /* (-Z, -X, Y) */
          points->SetPoint(0, secondPickPointBack);
          points->SetPoint(1, secondPointBack); 
          points->SetPoint(2, firstPickPointBack);
          points->SetPoint(3, firstPointBack);
          points->SetPoint(4, secondPickPoint[0], secondPickPoint[1], secondPickPoint[2]);
          points->SetPoint(5, secondPoint[0], secondPoint[1], secondPoint[2]);
          points->SetPoint(6, firstPickPoint[0], firstPickPoint[1], firstPickPoint[2]);
          points->SetPoint(7, firstPoint[0], firstPoint[1], firstPoint[2]);
          break;
        case Z_Y_NEGX:
          /* (-Z, Y, X) */
          points->SetPoint(0, firstPointBack);
          points->SetPoint(1, firstPickPointBack);
          points->SetPoint(2, firstPickPoint[0], firstPickPoint[1], firstPickPoint[2]);
          points->SetPoint(3, firstPoint[0], firstPoint[1], firstPoint[2]);
          points->SetPoint(4, secondPickPointBack);
          points->SetPoint(5, secondPointBack);
          points->SetPoint(6, secondPoint[0], secondPoint[1], secondPoint[2]);    
          points->SetPoint(7, secondPickPoint[0], secondPickPoint[1], secondPickPoint[2]);
          break;
        case NEGZ_NEGY_NEGX:
          /* (-Z, -Y, -X) */
          points->SetPoint(0, secondPickPoint[0], secondPickPoint[1], secondPickPoint[2]);
          points->SetPoint(1, secondPoint[0], secondPoint[1], secondPoint[2]);
          points->SetPoint(2, secondPointBack);
          points->SetPoint(3, secondPickPointBack);
          points->SetPoint(4, firstPoint[0], firstPoint[1], firstPoint[2]);
          points->SetPoint(5, firstPickPoint[0], firstPickPoint[1], firstPickPoint[2]);
          points->SetPoint(6, firstPickPointBack);
          points->SetPoint(7, firstPointBack);
          break;
        default:
          std::cout << "Error: Invalid box orientation..." << std::endl;
          break;
  }
}
//--------------------------------------------------------------------------------------------
void vtkMimxCreateBuildingBlockFromPickWidget::RightButtonDownCallback(vtkAbstractWidget *w)
{
        vtkMimxCreateBuildingBlockFromPickWidget *Self = reinterpret_cast<vtkMimxCreateBuildingBlockFromPickWidget*>(w);
        int X = Self->Interactor->GetEventPosition()[0];
        int Y = Self->Interactor->GetEventPosition()[1];

        // Okay, we can process this. Try to pick handles first;
        // if no handles picked, then pick the bounding box.
        if (!Self->CurrentRenderer || !Self->CurrentRenderer->IsInViewport(X, Y))
        {
                Self->WidgetEvent = vtkMimxCreateBuildingBlockFromPickWidget::Outside;
                return;
        }

        vtkAssemblyPath *path;
        vtkCellPicker *facePicker = vtkCellPicker::New();
        facePicker->AddPickList(Self->BuildingBlockActor);
        facePicker->Pick(X,Y,0.0, Self->CurrentRenderer);
        path = facePicker->GetPath();
        if(path !=NULL)
        {
                int PickedCell = facePicker->GetCellId();
                if(PickedCell > -1)
                {
                        Self->InvokeEvent(vtkCommand::RightButtonPressEvent, NULL);
                        Self->InvokeEvent(vtkCommand::RightButtonReleaseEvent, NULL);
                        Self->EventCallbackCommand->SetAbortFlag(1);
                        Self->StartInteraction();
                        Self->InvokeEvent(vtkCommand::StartInteractionEvent, NULL);
                        Self->Interactor->Render();
                        facePicker->Delete();
                        return;
                }
        }
        else
        {
                facePicker->Delete();
                return;
        }

        Self->EventCallbackCommand->SetAbortFlag(1);
        Self->StartInteraction();
        Self->InvokeEvent(vtkCommand::StartInteractionEvent, NULL);
        Self->Interactor->Render();
}
//----------------------------------------------------------------------------------------------------
void vtkMimxCreateBuildingBlockFromPickWidget::RightButtonUpCallback(vtkAbstractWidget *w)
{
        vtkMimxCreateBuildingBlockFromPickWidget *Self = reinterpret_cast<vtkMimxCreateBuildingBlockFromPickWidget*>(w);
        if ( Self->WidgetEvent == vtkMimxCreateBuildingBlockFromPickWidget::Outside ||
                Self->WidgetEvent == vtkMimxCreateBuildingBlockFromPickWidget::Start )
        {
                return;
        }

        Self->WidgetEvent = vtkMimxCreateBuildingBlockFromPickWidget::Start;

        Self->EventCallbackCommand->SetAbortFlag(1);
        Self->EndInteraction();
        Self->InvokeEvent(vtkCommand::EndInteractionEvent, NULL);
        Self->Interactor->Render();
}
//--------------------------------------------------------------------------------------------
vtkUnstructuredGrid* vtkMimxCreateBuildingBlockFromPickWidget::GetBuildingBlock()
{
        return  this->BuildingBlock;
}
//--------------------------------------------------------------------------------------------
void vtkMimxCreateBuildingBlockFromPickWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);  
}
//----------------------------------------------------------------------------------------
