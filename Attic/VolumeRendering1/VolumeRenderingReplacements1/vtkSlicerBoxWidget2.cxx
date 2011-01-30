/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkSlicerBoxWidget2.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkSlicerBoxWidget2.h"
#include "vtkSlicerBoxRepresentation.h"
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


vtkCxxRevisionMacro(vtkSlicerBoxWidget2, "$Revision: 1.2.2.2 $");
vtkStandardNewMacro(vtkSlicerBoxWidget2);

//----------------------------------------------------------------------------
vtkSlicerBoxWidget2::vtkSlicerBoxWidget2()
{
  this->WidgetState = vtkSlicerBoxWidget2::Start;
  this->ManagesCursor = 1;

  this->TranslationEnabled = 1;
  this->ScalingEnabled = 1;
  this->RotationEnabled = 1;

  // Define widget events
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
                                          vtkWidgetEvent::Select,
                                          this, vtkSlicerBoxWidget2::SelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
                                          vtkWidgetEvent::EndSelect,
                                          this, vtkSlicerBoxWidget2::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MiddleButtonPressEvent,
                                          vtkWidgetEvent::Translate,
                                          this, vtkSlicerBoxWidget2::TranslateAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MiddleButtonReleaseEvent,
                                          vtkWidgetEvent::EndTranslate,
                                          this, vtkSlicerBoxWidget2::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonPressEvent,
                                          vtkWidgetEvent::Scale,
                                          this, vtkSlicerBoxWidget2::ScaleAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonReleaseEvent,
                                          vtkWidgetEvent::EndScale,
                                          this, vtkSlicerBoxWidget2::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MouseMoveEvent,
                                          vtkWidgetEvent::Move,
                                          this, vtkSlicerBoxWidget2::MoveAction);
}

//----------------------------------------------------------------------------
vtkSlicerBoxWidget2::~vtkSlicerBoxWidget2()
{  
}

//----------------------------------------------------------------------
void vtkSlicerBoxWidget2::SelectAction(vtkAbstractWidget *w)
{
  // We are in a static method, cast to ourself
  vtkSlicerBoxWidget2 *self = reinterpret_cast<vtkSlicerBoxWidget2*>(w);

  // Get the event position
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];
  
  // Okay, make sure that the pick is in the current renderer
  if ( !self->CurrentRenderer || 
       !self->CurrentRenderer->IsInViewport(X,Y) )
    {
    self->WidgetState = vtkSlicerBoxWidget2::Start;
    return;
    }
  
  // Begin the widget interaction which has the side effect of setting the
  // interaction state.
  double e[2];
  e[0] = static_cast<double>(X);
  e[1] = static_cast<double>(Y);
  self->WidgetRep->StartWidgetInteraction(e);
  int interactionState = self->WidgetRep->GetInteractionState();
  if ( interactionState == vtkSlicerBoxRepresentation::Outside )
    {
    return;
    }
  
  // We are definitely selected
  self->WidgetState = vtkSlicerBoxWidget2::Active;
  self->GrabFocus(self->EventCallbackCommand);

  // Modifier keys force us into translare mode
  // The SetInteractionState has the side effect of highlighting the widget
  if ( self->Interactor->GetShiftKey() || self->Interactor->GetControlKey() )
    {
    reinterpret_cast<vtkSlicerBoxRepresentation*>(self->WidgetRep)->
      SetInteractionState(vtkSlicerBoxRepresentation::Translating);
    }
  else 
    {
    reinterpret_cast<vtkSlicerBoxRepresentation*>(self->WidgetRep)->
      SetInteractionState(interactionState);
    }

  // start the interaction
  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  self->Render();
}

//----------------------------------------------------------------------
void vtkSlicerBoxWidget2::TranslateAction(vtkAbstractWidget *w)
{
  // We are in a static method, cast to ourself
  vtkSlicerBoxWidget2 *self = reinterpret_cast<vtkSlicerBoxWidget2*>(w);

  // Get the event position
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];
  
  // Okay, make sure that the pick is in the current renderer
  if ( !self->CurrentRenderer || 
       !self->CurrentRenderer->IsInViewport(X,Y) )
    {
    self->WidgetState = vtkSlicerBoxWidget2::Start;
    return;
    }
  
  // Begin the widget interaction which has the side effect of setting the
  // interaction state.
  double e[2];
  e[0] = static_cast<double>(X);
  e[1] = static_cast<double>(Y);
  self->WidgetRep->StartWidgetInteraction(e);
  int interactionState = self->WidgetRep->GetInteractionState();
  if ( interactionState == vtkSlicerBoxRepresentation::Outside )
    {
    return;
    }
  
  // We are definitely selected
  self->WidgetState = vtkSlicerBoxWidget2::Active;
  self->GrabFocus(self->EventCallbackCommand);
  reinterpret_cast<vtkSlicerBoxRepresentation*>(self->WidgetRep)->
    SetInteractionState(vtkSlicerBoxRepresentation::Translating);
  
  // start the interaction
  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  self->Render();
}

//----------------------------------------------------------------------
void vtkSlicerBoxWidget2::ScaleAction(vtkAbstractWidget *w)
{
  // We are in a static method, cast to ourself
  vtkSlicerBoxWidget2 *self = reinterpret_cast<vtkSlicerBoxWidget2*>(w);

  // Get the event position
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];
  
  // Okay, make sure that the pick is in the current renderer
  if ( !self->CurrentRenderer || 
       !self->CurrentRenderer->IsInViewport(X,Y) )
    {
    self->WidgetState = vtkSlicerBoxWidget2::Start;
    return;
    }
  
  // Begin the widget interaction which has the side effect of setting the
  // interaction state.
  double e[2];
  e[0] = static_cast<double>(X);
  e[1] = static_cast<double>(Y);
  self->WidgetRep->StartWidgetInteraction(e);
  int interactionState = self->WidgetRep->GetInteractionState();
  if ( interactionState == vtkSlicerBoxRepresentation::Outside )
    {
    return;
    }
  
  // We are definitely selected
  self->WidgetState = vtkSlicerBoxWidget2::Active;
  self->GrabFocus(self->EventCallbackCommand);
  reinterpret_cast<vtkSlicerBoxRepresentation*>(self->WidgetRep)->
    SetInteractionState(vtkSlicerBoxRepresentation::Scaling);

  // start the interaction
  self->EventCallbackCommand->SetAbortFlag(1);
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  self->Render();
}

//----------------------------------------------------------------------
void vtkSlicerBoxWidget2::MoveAction(vtkAbstractWidget *w)
{
  vtkSlicerBoxWidget2 *self = reinterpret_cast<vtkSlicerBoxWidget2*>(w);

  // See whether we're active
  if ( self->WidgetState == vtkSlicerBoxWidget2::Start )
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
  self->InvokeEvent(vtkCommand::InteractionEvent,NULL);
  self->Render();
}

//----------------------------------------------------------------------
void vtkSlicerBoxWidget2::EndSelectAction(vtkAbstractWidget *w)
{
  vtkSlicerBoxWidget2 *self = reinterpret_cast<vtkSlicerBoxWidget2*>(w);
  if ( self->WidgetState == vtkSlicerBoxWidget2::Start )
    {
    return;
    }
  
  // Return state to not active
  self->WidgetState = vtkSlicerBoxWidget2::Start;
  reinterpret_cast<vtkSlicerBoxRepresentation*>(self->WidgetRep)->
    SetInteractionState(vtkSlicerBoxRepresentation::Outside);
  self->ReleaseFocus();

  self->EventCallbackCommand->SetAbortFlag(1);
  self->EndInteraction();
  self->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
  self->Render();
}

//----------------------------------------------------------------------
void vtkSlicerBoxWidget2::CreateDefaultRepresentation()
{
  if ( ! this->WidgetRep )
    {
    this->WidgetRep = vtkSlicerBoxRepresentation::New();
    }
}

//----------------------------------------------------------------------------
void vtkSlicerBoxWidget2::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Translation Enabled: " << (this->TranslationEnabled ? "On\n" : "Off\n");
  os << indent << "Scaling Enabled: " << (this->ScalingEnabled ? "On\n" : "Off\n");
  os << indent << "Rotation Enabled: " << (this->RotationEnabled ? "On\n" : "Off\n");
}


