/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxInteractorStyleUser.cxx,v $
Language:  C++
Date:      $Date: 2008/07/06 20:30:13 $
Version:   $Revision: 1.2 $

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
#include "vtkMimxInteractorStyleUser.h"
#include "vtkMath.h"
#include "vtkCellPicker.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"


vtkCxxRevisionMacro(vtkMimxInteractorStyleUser, "$Revision: 1.2 $");
vtkStandardNewMacro(vtkMimxInteractorStyleUser);

//----------------------------------------------------------------------------
vtkMimxInteractorStyleUser::vtkMimxInteractorStyleUser()
{
  // Tell the parent class not to handle observers
  // that has to be done here
  this->HandleObserversOff();
  this->LastPos[0] = this->LastPos[1] = 0;
  this->OldPos[0] = this->OldPos[1] = 0;
  this->ShiftKey = 0;
  this->CtrlKey = 0;
  this->Char = '\0';
  this->KeySym = (char *) "";
  this->Button = 0;
}

//----------------------------------------------------------------------------
vtkMimxInteractorStyleUser::~vtkMimxInteractorStyleUser() 
{
}

//----------------------------------------------------------------------------
void vtkMimxInteractorStyleUser::PrintSelf(ostream& os, vtkIndent indent) 
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "LastPos: (" << this->LastPos[0] << ", " 
                               << this->LastPos[1] << ")\n";  
  os << indent << "OldPos: (" << this->OldPos[0] << ", " 
                              << this->OldPos[1] << ")\n";
  os << indent << "ShiftKey: " << this->ShiftKey << "\n";
  os << indent << "CtrlKey: " << this->CtrlKey << "\n";
  os << indent << "Char: " << this->Char << "\n";
  os << indent << "KeySym: " << this->KeySym << "\n";
  os << indent << "Button: " << this->Button << "\n";
}

//----------------------------------------------------------------------------
// checks for USERINTERACTION state, then defers to the superclass modes
void vtkMimxInteractorStyleUser::OnTimer() 
{
  if (this->HasObserver(vtkCommand::TimerEvent)) 
    {
    this->InvokeEvent(vtkCommand::TimerEvent,(void*)&(this->TimerId));
    }

  if (this->State == VTKIS_USERINTERACTION)
    {
    if (this->HasObserver(vtkCommand::UserEvent)) 
      {
      this->InvokeEvent(vtkCommand::UserEvent,NULL);
      this->OldPos[0] = this->LastPos[0];
      this->OldPos[1] = this->LastPos[1];
      if (this->UseTimers) 
        {
        this->Interactor->ResetTimer(this->TimerId);
        }
      }
    }
  else if (!(this->HasObserver(vtkCommand::MouseMoveEvent) && 
             (this->Button == 0 ||
              (this->HasObserver(vtkCommand::LeftButtonPressEvent) && this->Button == 1) ||
              (this->HasObserver(vtkCommand::MiddleButtonPressEvent) && this->Button == 2) ||
              (this->HasObserver(vtkCommand::RightButtonPressEvent) && this->Button == 3))))
    {
    this->vtkInteractorStyle::OnTimer();
    }
  else if (this->HasObserver(vtkCommand::TimerEvent))
    {
    if (this->UseTimers) 
      {
      this->Interactor->ResetTimer(this->TimerId);
      }
    }
}

//----------------------------------------------------------------------------
void vtkMimxInteractorStyleUser::OnKeyPress()
{
  if (this->HasObserver(vtkCommand::KeyPressEvent)) 
    {
    this->ShiftKey = this->Interactor->GetShiftKey();
    this->CtrlKey = this->Interactor->GetControlKey();
    this->KeySym = this->Interactor->GetKeySym();
    this->Char = this->Interactor->GetKeyCode();  
    this->InvokeEvent(vtkCommand::KeyPressEvent, NULL);
    }
}

//----------------------------------------------------------------------------
void vtkMimxInteractorStyleUser::OnKeyRelease()
{
  if (this->HasObserver(vtkCommand::KeyReleaseEvent)) 
    {
    this->ShiftKey = this->Interactor->GetShiftKey();
    this->CtrlKey = this->Interactor->GetControlKey();
    this->KeySym = this->Interactor->GetKeySym();
    this->Char = this->Interactor->GetKeyCode();  

    this->InvokeEvent(vtkCommand::KeyReleaseEvent,NULL);
    }
}

//----------------------------------------------------------------------------
void vtkMimxInteractorStyleUser::OnChar() 
{
  // otherwise pass the OnChar to the vtkInteractorStyle.
  if (this->HasObserver(vtkCommand::CharEvent)) 
    {
    this->ShiftKey = this->Interactor->GetShiftKey();
    this->CtrlKey = this->Interactor->GetControlKey();
    this->Char = this->Interactor->GetKeyCode();  
    
    this->InvokeEvent(vtkCommand::CharEvent,NULL);
    }
  else
    {
    this->vtkInteractorStyle::OnChar();
    }
}

//----------------------------------------------------------------------------
void vtkMimxInteractorStyleUser::OnRightButtonDown() 
{
  this->Button = 3;

  if (this->HasObserver(vtkCommand::RightButtonPressEvent)) 
    {
    int x = this->Interactor->GetEventPosition()[0];
    int y = this->Interactor->GetEventPosition()[1];
    this->CtrlKey  = this->Interactor->GetControlKey();
    this->ShiftKey = this->Interactor->GetShiftKey();
    this->LastPos[0] = x;
    this->LastPos[1] = y;
    this->InvokeEvent(vtkCommand::RightButtonPressEvent,NULL);
    this->OldPos[0] = x;
    this->OldPos[1] = y;
    }
  else 
    {
    this->vtkInteractorStyle::OnRightButtonDown();
    }
}
//----------------------------------------------------------------------------
void vtkMimxInteractorStyleUser::OnRightButtonUp() 
{
  if (this->HasObserver(vtkCommand::RightButtonReleaseEvent)) 
    {
    int x = this->Interactor->GetEventPosition()[0];
    int y = this->Interactor->GetEventPosition()[1];
    this->CtrlKey  = this->Interactor->GetControlKey();
    this->ShiftKey = this->Interactor->GetShiftKey();
    this->LastPos[0] = x;
    this->LastPos[1] = y;
    this->InvokeEvent(vtkCommand::RightButtonReleaseEvent,NULL);
    this->OldPos[0] = x;
    this->OldPos[1] = y;
    }
  else 
    {
    this->vtkInteractorStyle::OnRightButtonUp();
    }

  if (this->Button == 3)
    {
    this->Button = 0;
    }
}

//----------------------------------------------------------------------------
void vtkMimxInteractorStyleUser::OnMiddleButtonDown() 
{
  this->Button = 2;

  if (this->HasObserver(vtkCommand::MiddleButtonPressEvent)) 
    {
    int x = this->Interactor->GetEventPosition()[0];
    int y = this->Interactor->GetEventPosition()[1];
    this->CtrlKey  = this->Interactor->GetControlKey();
    this->ShiftKey = this->Interactor->GetShiftKey();
    this->LastPos[0] = x;
    this->LastPos[1] = y;
    this->InvokeEvent(vtkCommand::MiddleButtonPressEvent,NULL);
    this->OldPos[0] = x;
    this->OldPos[1] = y;
    }
  else 
    {
    this->vtkInteractorStyle::OnMiddleButtonDown();
    }
}
//----------------------------------------------------------------------------
void vtkMimxInteractorStyleUser::OnMiddleButtonUp() 
{
  if (this->HasObserver(vtkCommand::MiddleButtonReleaseEvent)) 
    {
    int x = this->Interactor->GetEventPosition()[0];
    int y = this->Interactor->GetEventPosition()[1];
    this->CtrlKey  = this->Interactor->GetControlKey();
    this->ShiftKey = this->Interactor->GetShiftKey();
    this->LastPos[0] = x;
    this->LastPos[1] = y;
    this->InvokeEvent(vtkCommand::MiddleButtonReleaseEvent,NULL);
    this->OldPos[0] = x;
    this->OldPos[1] = y;
    }
  else 
    {
    this->vtkInteractorStyle::OnMiddleButtonUp();
    }

  if (this->Button == 2)
    {
    this->Button = 0;
    }
}

//----------------------------------------------------------------------------
void vtkMimxInteractorStyleUser::OnLeftButtonDown() 
{
  this->Button = 1;

  if (this->HasObserver(vtkCommand::LeftButtonPressEvent)) 
    {
    int x = this->Interactor->GetEventPosition()[0];
    int y = this->Interactor->GetEventPosition()[1];
    this->CtrlKey  = this->Interactor->GetControlKey();
    this->ShiftKey = this->Interactor->GetShiftKey();
    this->LastPos[0] = x;
    this->LastPos[1] = y;
    this->InvokeEvent(vtkCommand::LeftButtonPressEvent,NULL);
    this->OldPos[0] = x;
    this->OldPos[1] = y;
    }
  else 
    {
    this->vtkInteractorStyle::OnLeftButtonDown();
    }
}
//----------------------------------------------------------------------------
void vtkMimxInteractorStyleUser::OnLeftButtonUp() 
{
  if (this->HasObserver(vtkCommand::LeftButtonReleaseEvent)) 
    {
    int x = this->Interactor->GetEventPosition()[0];
    int y = this->Interactor->GetEventPosition()[1];
    this->CtrlKey  = this->Interactor->GetControlKey();
    this->ShiftKey = this->Interactor->GetShiftKey();
    this->LastPos[0] = x;
    this->LastPos[1] = y;
    this->InvokeEvent(vtkCommand::LeftButtonReleaseEvent,NULL);
    this->OldPos[0] = x;
    this->OldPos[1] = y;
    }
  else 
    {
    this->vtkInteractorStyle::OnLeftButtonUp();
    }

  if (this->Button == 1)
    {
    this->Button = 0;
    }
}

//----------------------------------------------------------------------------
void vtkMimxInteractorStyleUser::OnMouseMove() 
{
  this->vtkInteractorStyle::OnMouseMove();

  int x = this->Interactor->GetEventPosition()[0];
  int y = this->Interactor->GetEventPosition()[1];
  this->LastPos[0] = x;
  this->LastPos[1] = y;
  this->ShiftKey = this->Interactor->GetShiftKey();
  this->CtrlKey = this->Interactor->GetControlKey();

  if (this->HasObserver(vtkCommand::MouseMoveEvent)) 
    {
    this->InvokeEvent(vtkCommand::MouseMoveEvent,NULL);
    this->OldPos[0] = x;
    this->OldPos[1] = y;
    }
}

//----------------------------------------------------------------------------
void vtkMimxInteractorStyleUser::OnExpose()
{
  if (this->HasObserver(vtkCommand::ExposeEvent)) 
    {
    this->InvokeEvent(vtkCommand::ExposeEvent,NULL);
    }
}

//----------------------------------------------------------------------------
void vtkMimxInteractorStyleUser::OnConfigure() 
{
  if (this->HasObserver(vtkCommand::ConfigureEvent)) 
    {
    this->InvokeEvent(vtkCommand::ConfigureEvent,NULL);
    }
}

//----------------------------------------------------------------------------
void vtkMimxInteractorStyleUser::OnEnter()
{
  if (this->HasObserver(vtkCommand::EnterEvent)) 
    {
    this->LastPos[0] = this->Interactor->GetEventPosition()[0];
    this->LastPos[1] = this->Interactor->GetEventPosition()[1];
    this->InvokeEvent(vtkCommand::EnterEvent,NULL);
    }
}

//----------------------------------------------------------------------------
void vtkMimxInteractorStyleUser::OnLeave()
{
  if (this->HasObserver(vtkCommand::LeaveEvent)) 
    {
    this->LastPos[0] = this->Interactor->GetEventPosition()[0];
    this->LastPos[1] = this->Interactor->GetEventPosition()[1];
    this->InvokeEvent(vtkCommand::LeaveEvent,NULL);
    }
}

