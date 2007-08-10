/*=========================================================================

  Module:    $RCSfile: vtkKWMimxSurfaceOperationsMenu.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
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
vtkStandardNewMacro(vtkKWMimxSurfaceOperationsMenu);
vtkCxxRevisionMacro(vtkKWMimxSurfaceOperationsMenu, "$Revision: 1.6 $");

//----------------------------------------------------------------------------
vtkKWMimxSurfaceOperationsMenu::vtkKWMimxSurfaceOperationsMenu()
{
  this->MimxViewWindow = NULL;
  this->MenuButton = vtkKWMenuButtonWithLabel::New();
  this->ObjectList = vtkKWMultiColumnList::New();
}

//----------------------------------------------------------------------------
vtkKWMimxSurfaceOperationsMenu::~vtkKWMimxSurfaceOperationsMenu()
{
  this->MenuButton->Delete();
  this->ObjectList->Delete();
}
//----------------------------------------------------------------------------
void vtkKWMimxSurfaceOperationsMenu::CreateWidget()
{
  if(this->IsCreated())
  {
    vtkErrorMacro("class already created");
    return;
  }
  this->Superclass::CreateWidget();
  // add menu button with options for various operations
  // for surface
  this->MenuButton->SetParent(this->GetParent());
  this->MenuButton->Create();
  this->MenuButton->SetBorderWidth(2);
  this->MenuButton->SetReliefToGroove();
  this->MenuButton->SetLabelText("Operation :");
  this->MenuButton->SetPadX(2);
  this->MenuButton->SetPadY(0);
  this->GetApplication()->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 0", 
    this->MenuButton->GetWidgetName());

  this->MenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "Load",this, "LoadFileCallback");
  this->SetEnabled(0);
  this->ObjectList->SetParent(this->GetParent());
  this->ObjectList->Create();
  int col_index;
  col_index = this->ObjectList->AddColumn(NULL);
  this->ObjectList->SetColumnFormatCommandToEmptyOutput(col_index);
  this->ObjectList->SetColumnLabelImageToPredefinedIcon(col_index, vtkKWIcon::IconEye);
  this->ObjectList->AddColumn("File Name");
//  this->GetApplication()->Script("pack %s -side top -anchor nw -expand n -padx 6 -pady 6", 
//    this->ObjectList->GetWidgetName());
//  this->UnpackSiblings();
}
//----------------------------------------------------------------------------
void vtkKWMimxSurfaceOperationsMenu::Update()
{
  this->UpdateEnableState();
}
//----------------------------------------------------------------------------
void vtkKWMimxSurfaceOperationsMenu::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
void vtkKWMimxSurfaceOperationsMenu::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxSurfaceOperationsMenu::LoadFileCallback()
{
  vtkKWFileBrowserDialog *filebrowser = vtkKWFileBrowserDialog::New();
  filebrowser->SetApplication(this->GetApplication());
  filebrowser->Create();
  filebrowser->SetDefaultExtension(".stl");
  filebrowser->SetFileTypes("{{STL files} {.stl}} {{VTK} {.vtk}}");
  filebrowser->Invoke();
  if(filebrowser->GetStatus() == vtkKWDialog::StatusOK)
  {
    if(filebrowser->GetFileName())
    {
      const char *filename = filebrowser->GetFileName();
      vtkSTLReader *reader = vtkSTLReader::New();
      reader->SetFileName(filebrowser->GetFileName());
      reader->Update();
      vtkPolyDataMapper *mapper = vtkPolyDataMapper::New();
      mapper->SetInput(reader->GetOutput());
      vtkActor *actor = vtkActor::New();
      actor->SetMapper(mapper);
      this->GetMimxViewWindow()->GetRenderWidget()->AddViewProp(actor);
      this->GetMimxViewWindow()->GetRenderWidget()->Render();
      this->GetMimxViewWindow()->GetRenderWidget()->ResetCamera();
      this->ObjectList->InsertCellTextAsInt(0,0,1);
      this->ObjectList->SetCellWindowCommandToCheckButton(0,0);
      int status = this->ObjectList->GetCellWindowAsCheckButton(0,0)->GetSelectedState();
      this->ObjectList->GetCellWindowAsCheckButton(0,0)->SetCommand(this, "ShowHideActor 0 0 status");
      this->ObjectList->InsertCellText(0,1,filebrowser->GetFileName());
    }
  }
  filebrowser->Delete();
}
//---------------------------------------------------------------------------
void vtkKWMimxSurfaceOperationsMenu::SetEnabled(int Enable)
{
  this->MenuButton->SetEnabled(Enable);
}
//---------------------------------------------------------------------------
