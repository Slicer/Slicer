/*=========================================================================

  Module:    $RCSfile: vtkKWScriptedWidget.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWScriptedWidget.h"

#include "vtkObjectFactory.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWScriptedWidget );
vtkCxxRevisionMacro(vtkKWScriptedWidget, "$Revision: 1.49 $");

//----------------------------------------------------------------------------
vtkKWScriptedWidget::vtkKWScriptedWidget()
{
  this->Command = NULL;
  this->InitializeCommand = NULL;

  this->UpdateWidget();
}

//----------------------------------------------------------------------------
vtkKWScriptedWidget::~vtkKWScriptedWidget()
{
  if (this->Command)
    {
    delete [] this->Command;
    }

  if (this->InitializeCommand)
    {
    delete [] this->InitializeCommand;
    }

}

//----------------------------------------------------------------------------
void vtkKWScriptedWidget::CreateWidget()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  if (this->InitializeCommand == NULL)
    {
    vtkErrorMacro(<< this->GetClassName() << " InitializeCommand is NULL");
    return;
    }


  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();

  // e.g. InitializeCommand could be "source MyWidget.tcl; MyWidgetInit"
  // where MyWidgetInit is a proc that takes as an argument the frame in which to
  // create and pack itself
  this->Script("%s %s %s", 
        InitializeCommand, this->GetTclName(), this->GetWidgetName());
}

//----------------------------------------------------------------------------
void vtkKWScriptedWidget::UpdateWidget()
{
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkKWScriptedWidget::UpdateVTK()
{
}

//----------------------------------------------------------------------------
void vtkKWScriptedWidget::SetCommand(vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(&this->Command, object, method);
}


//----------------------------------------------------------------------------
void vtkKWScriptedWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "InitializeCommand: " << 
      ((this->InitializeCommand) ? (this->InitializeCommand) : "NULL") << endl;
}

