/*=========================================================================

  Module:    $RCSfile: vtkKWMimxSurfaceObjectMenu.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWMimxSurfaceObjectMenu.h"
//#include "vtkKWMimxOperationsMenu.h"
#include "vtkKWMimxSurfaceOperationsMenu.h"

#include "vtkActor.h"
#include "vtkPolyDataMapper.h"
#include "vtkSTLReader.h"

#include "vtkKWApplication.h"
#include "vtkKWCheckButton.h"
#include "vtkKWFileBrowserDialog.h"
#include "vtkKWEvent.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithScrollbar.h"
#include "vtkKWIcon.h"
#include "vtkKWInternationalization.h"
#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWNotebook.h"
#include "vtkKWOptions.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWTkUtilities.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxSurfaceObjectMenu);
vtkCxxRevisionMacro(vtkKWMimxSurfaceObjectMenu, "$Revision: 1.6 $");

//----------------------------------------------------------------------------
vtkKWMimxSurfaceObjectMenu::vtkKWMimxSurfaceObjectMenu()
{
  this->MenuButton = vtkKWMenuButtonWithLabel::New();
}

//----------------------------------------------------------------------------
vtkKWMimxSurfaceObjectMenu::~vtkKWMimxSurfaceObjectMenu()
{
  this->MenuButton->Delete();
}
//----------------------------------------------------------------------------
void vtkKWMimxSurfaceObjectMenu::CreateWidget()
{
  if(this->IsCreated())
  {
    vtkErrorMacro("class already created");
    return;
  }
  this->Superclass::CreateWidget();
  // add menu button with options for various Object
  // for surface
  this->MenuButton->SetParent(this->GetParent());
  this->MenuButton->Create();
  this->MenuButton->SetBorderWidth(2);
  this->MenuButton->SetReliefToGroove();
  this->MenuButton->SetLabelText("Object :");
  this->MenuButton->SetPadX(2);
  this->MenuButton->SetPadY(0);
  this->GetApplication()->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 5", 
    this->MenuButton->GetWidgetName());
  this->MenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "Surface",this, "SurfaceMenuCallback");
}
//----------------------------------------------------------------------------
void vtkKWMimxSurfaceObjectMenu::Update()
{
  this->UpdateEnableState();
}
//----------------------------------------------------------------------------
void vtkKWMimxSurfaceObjectMenu::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
void vtkKWMimxSurfaceObjectMenu::SurfaceMenuCallback()
{
  vtkKWMimxSurfaceOperationsMenu::SafeDownCast(this->GetMimxOperationsMenu())->SetEnabled(1);
}
//----------------------------------------------------------------------------
void vtkKWMimxSurfaceObjectMenu::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//---------------------------------------------------------------------------
