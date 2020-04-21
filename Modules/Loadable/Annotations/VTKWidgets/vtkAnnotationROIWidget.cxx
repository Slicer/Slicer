/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkAnnotationROIWidget.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

// Annotations includes
#include "vtkAnnotationROIWidget.h"
#include "vtkAnnotationROIRepresentation.h"

// VTK includes
#include "vtkCommand.h"
#include "vtkCallbackCommand.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkObjectFactory.h"
#include "vtkWidgetEventTranslator.h"
#include "vtkWidgetCallbackMapper.h"
#include "vtkEvent.h"
#include "vtkWidgetEvent.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"


vtkStandardNewMacro(vtkAnnotationROIWidget);

//----------------------------------------------------------------------------
vtkAnnotationROIWidget::vtkAnnotationROIWidget()
{
  this->WidgetState = vtkAnnotationROIWidget::Start;
  this->ManagesCursor = 1;

  this->TranslationEnabled = 1;
  this->ScalingEnabled = 1;
  this->RotationEnabled = 1;

  // Define widget events
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
                                          vtkWidgetEvent::Select,
                                          this, vtkAnnotationROIWidget::SelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                          vtkWidgetEvent::EndSelect,
                                          this, vtkAnnotationROIWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MiddleButtonPressEvent,
                                          vtkWidgetEvent::Translate,
                                          this, vtkAnnotationROIWidget::TranslateAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MiddleButtonReleaseEvent,
                                          vtkWidgetEvent::EndTranslate,
                                          this, vtkAnnotationROIWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonPressEvent,
                                          vtkWidgetEvent::Scale,
                                          this, vtkAnnotationROIWidget::ScaleAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonReleaseEvent,
                                          vtkWidgetEvent::EndScale,
                                          this, vtkAnnotationROIWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MouseMoveEvent,
                                          vtkWidgetEvent::Move,
                                          this, vtkAnnotationROIWidget::MoveAction);
}

//----------------------------------------------------------------------------
vtkAnnotationROIWidget::~vtkAnnotationROIWidget() = default;

//----------------------------------------------------------------------
void vtkAnnotationROIWidget::SelectAction(vtkAbstractWidget *w)
{
  // We are in a static method, cast to ourself
  vtkAnnotationROIWidget *self = reinterpret_cast<vtkAnnotationROIWidget*>(w);

  // Get the event position
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // Okay, make sure that the pick is in the current renderer
  if ( !self->CurrentRenderer ||
       !self->CurrentRenderer->IsInViewport(X,Y) )
    {
    self->WidgetState = vtkAnnotationROIWidget::Start;
    return;
    }

  // Begin the widget interaction which has the side effect of setting the
  // interaction state.
  double e[2];
  e[0] = static_cast<double>(X);
  e[1] = static_cast<double>(Y);
  self->WidgetRep->StartWidgetInteraction(e);
  int interactionState = self->WidgetRep->GetInteractionState();
  if ( interactionState == vtkAnnotationROIRepresentation::Outside )
    {
    return;
    }

  // We are definitely selected
  self->WidgetState = vtkAnnotationROIWidget::Active;
  self->GrabFocus(self->EventCallbackCommand);

  // Modifier keys force us into translare mode
  // The SetInteractionState has the side effect of highlighting the widget
  if ( self->Interactor->GetShiftKey() || self->Interactor->GetControlKey() )
    {
    reinterpret_cast<vtkAnnotationROIRepresentation*>(self->WidgetRep)->
      SetInteractionState(vtkAnnotationROIRepresentation::Translating);
    }
  else
    {
    reinterpret_cast<vtkAnnotationROIRepresentation*>(self->WidgetRep)->
      SetInteractionState(interactionState);
    }

  // start the interaction
  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent,nullptr);
  self->Render();
}

//----------------------------------------------------------------------
void vtkAnnotationROIWidget::TranslateAction(vtkAbstractWidget *w)
{
  // We are in a static method, cast to ourself
  vtkAnnotationROIWidget *self = reinterpret_cast<vtkAnnotationROIWidget*>(w);

  // Get the event position
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // Okay, make sure that the pick is in the current renderer
  if ( !self->CurrentRenderer ||
       !self->CurrentRenderer->IsInViewport(X,Y) )
    {
    self->WidgetState = vtkAnnotationROIWidget::Start;
    return;
    }

  // Begin the widget interaction which has the side effect of setting the
  // interaction state.
  double e[2];
  e[0] = static_cast<double>(X);
  e[1] = static_cast<double>(Y);
  self->WidgetRep->StartWidgetInteraction(e);
  int interactionState = self->WidgetRep->GetInteractionState();
  if ( interactionState == vtkAnnotationROIRepresentation::Outside )
    {
    return;
    }

  // We are definitely selected
  self->WidgetState = vtkAnnotationROIWidget::Active;
  self->GrabFocus(self->EventCallbackCommand);
  reinterpret_cast<vtkAnnotationROIRepresentation*>(self->WidgetRep)->
    SetInteractionState(vtkAnnotationROIRepresentation::Translating);

  // start the interaction
  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent,nullptr);
  self->Render();
}

//----------------------------------------------------------------------
void vtkAnnotationROIWidget::ScaleAction(vtkAbstractWidget *w)
{
  // We are in a static method, cast to ourself
  vtkAnnotationROIWidget *self = reinterpret_cast<vtkAnnotationROIWidget*>(w);

  // Get the event position
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // Okay, make sure that the pick is in the current renderer
  if ( !self->CurrentRenderer ||
       !self->CurrentRenderer->IsInViewport(X,Y) )
    {
    self->WidgetState = vtkAnnotationROIWidget::Start;
    return;
    }

  // Begin the widget interaction which has the side effect of setting the
  // interaction state.
  double e[2];
  e[0] = static_cast<double>(X);
  e[1] = static_cast<double>(Y);
  self->WidgetRep->StartWidgetInteraction(e);
  int interactionState = self->WidgetRep->GetInteractionState();
  if ( interactionState == vtkAnnotationROIRepresentation::Outside )
    {
    return;
    }

  // We are definitely selected
  self->WidgetState = vtkAnnotationROIWidget::Active;
  self->GrabFocus(self->EventCallbackCommand);
  reinterpret_cast<vtkAnnotationROIRepresentation*>(self->WidgetRep)->
    SetInteractionState(vtkAnnotationROIRepresentation::Scaling);

  // start the interaction
  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent,nullptr);
  self->Render();
}

//----------------------------------------------------------------------
void vtkAnnotationROIWidget::MoveAction(vtkAbstractWidget *w)
{
  vtkAnnotationROIWidget *self = reinterpret_cast<vtkAnnotationROIWidget*>(w);

  // See whether we're active
  if ( self->WidgetState == vtkAnnotationROIWidget::Start )
    {
    return;
    }

  // compute some info we need for all cases
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // Okay, adjust the representation
  double e[2];
  e[0] = static_cast<double>(X);
  e[1] = static_cast<double>(Y);
  self->WidgetRep->WidgetInteraction(e);

  // moving something
  self->EventCallbackCommand->SetAbortFlag(1);
  self->InvokeEvent(vtkCommand::InteractionEvent,nullptr);
  self->Render();
}

//----------------------------------------------------------------------
void vtkAnnotationROIWidget::EndSelectAction(vtkAbstractWidget *w)
{
  vtkAnnotationROIWidget *self = reinterpret_cast<vtkAnnotationROIWidget*>(w);
  if ( self->WidgetState == vtkAnnotationROIWidget::Start )
    {
    return;
    }

  // Return state to not active
  self->WidgetState = vtkAnnotationROIWidget::Start;
  reinterpret_cast<vtkAnnotationROIRepresentation*>(self->WidgetRep)->
    SetInteractionState(vtkAnnotationROIRepresentation::Outside);
  self->ReleaseFocus();

  self->EventCallbackCommand->SetAbortFlag(1);
  self->EndInteraction();
  self->InvokeEvent(vtkCommand::EndInteractionEvent,nullptr);
  self->Render();
}

//----------------------------------------------------------------------
void vtkAnnotationROIWidget::CreateDefaultRepresentation()
{
  if ( ! this->WidgetRep )
    {
    this->WidgetRep = vtkAnnotationROIRepresentation::New();
    }
}

//----------------------------------------------------------------------------
void vtkAnnotationROIWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Translation Enabled: " << (this->TranslationEnabled ? "On\n" : "Off\n");
  os << indent << "Scaling Enabled: " << (this->ScalingEnabled ? "On\n" : "Off\n");
  os << indent << "Rotation Enabled: " << (this->RotationEnabled ? "On\n" : "Off\n");
}


