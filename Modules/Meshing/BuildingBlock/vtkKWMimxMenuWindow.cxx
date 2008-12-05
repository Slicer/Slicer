/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxMenuWindow.cxx,v $
Language:  C++
Date:      $Date: 2008/02/17 00:29:47 $
Version:   $Revision: 1.12 $

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

#include "vtkKWMimxMainNotebook.h"
#include "vtkKWMimxMenuWindow.h"
#include "vtkKWApplication.h"
#include "vtkKWFrameWithScrollbar.h"
#include "vtkKWMenu.h"
#include "vtkKWNotebook.h"
#include "vtkKWWindowBase.h"
#include "vtkObjectFactory.h"
#include <vtksys/SystemTools.hxx>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWMimxMenuWindow );
vtkCxxRevisionMacro(vtkKWMimxMenuWindow, "$Revision: 1.12 $");

//----------------------------------------------------------------------------
vtkKWMimxMenuWindow::vtkKWMimxMenuWindow()
{
        this->ViewMenu = NULL;
        this->MainNoteBookFrameScrollbar = NULL;
        this->MainNotebook = NULL;
        this->MimxViewWindow = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxMenuWindow::~vtkKWMimxMenuWindow()
{
        if(this->ViewMenu)
        {
                this->ViewMenu->Delete();
        }
        if(this->MainNoteBookFrameScrollbar)
        {
                this->MainNoteBookFrameScrollbar->Delete();
        }
        if(this->MainNotebook)
        {
                this->MainNotebook->Delete();
        }
}
//----------------------------------------------------------------------------
void vtkKWMimxMenuWindow::CreateWidget()
{
        // setting the position and size of the window
        if(this->IsCreated())
        {
                vtkErrorMacro("Class already created");
                return;
        }
        this->Superclass::CreateWidget();
        this->SetSize(300,700);
  this->SetResizable(0,1);
        this->SetPosition(0,0);

        // adding 'view' menu
        if(!this->ViewMenu)
        {
                this->ViewMenu = vtkKWMenu::New();
        }
        this->ViewMenu->SetParent(this->GetMenu());
        this->ViewMenu->Create();
        this->GetMenu()->AddCascade("Display", this->ViewMenu);
        this->ViewMenu->AddCommand("Background Color", this->MimxViewWindow, 
                "ViewWindowProperties");

        // frame with scroll bar to place all tabs 
        if(!this->MainNoteBookFrameScrollbar)
        {
                this->MainNoteBookFrameScrollbar = vtkKWFrameWithScrollbar::New();
        }
        this->MainNoteBookFrameScrollbar->HorizontalScrollbarVisibilityOn();
        this->MainNoteBookFrameScrollbar->VerticalScrollbarVisibilityOff();
        this->MainNoteBookFrameScrollbar->SetParent(this->GetViewFrame());
        this->MainNoteBookFrameScrollbar->Create();
        this->GetApplication()->Script("pack %s -side top -anchor nw -expand y -padx 2 -pady 1", 
                this->MainNoteBookFrameScrollbar->GetWidgetName());
//      this->MainNoteBookFrameScrollbar->SetHeight(300);
        // add tabs
        if(!this->MainNotebook)
        {
                this->MainNotebook = vtkKWMimxMainNotebook::New();
                this->MainNotebook->SetMimxRenderWidget(this->MimxViewWindow);
                //this->MainNotebook->SetMimxUserInterfaceManager(this->GetMainUserInterfaceManager());
        }
  vtkKWFrame *frame = this->MainNoteBookFrameScrollbar->GetFrame();
        this->MainNotebook->SetParent(this->MainNoteBookFrameScrollbar->GetFrame());
        this->MainNotebook->SetApplication(this->GetApplication());
        this->MainNotebook->Create();
  this->MainNotebook->SetWidth(200);
        this->GetApplication()->Script(
                "pack %s -side top -anchor nw  -expand y -padx 0 -pady 1", 
                this->MainNotebook->GetWidgetName());
}
