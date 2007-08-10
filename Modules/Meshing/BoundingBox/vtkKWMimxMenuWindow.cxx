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
vtkCxxRevisionMacro(vtkKWMimxMenuWindow, "$Revision: 1.7 $");

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
  this->GetMenu()->AddCascade("View", this->ViewMenu);
  this->ViewMenu->AddCommand("3D View Properties");

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
//  this->MainNoteBookFrameScrollbar->SetHeight(300);
  // add tabs
  if(!this->MainNotebook)
  {
    this->MainNotebook = vtkKWMimxMainNotebook::New();
    this->MainNotebook->SetMimxViewWindow(this->MimxViewWindow);
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
