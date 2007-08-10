/*=========================================================================

  Module:    $RCSfile: vtkKWMimxViewProperties.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWMimxViewProperties.h"

#include "vtkLinkedListWrapper.h"

#include "vtkKWApplication.h"
#include "vtkKWChangeColorButton.h"
#include "vtkKWCheckButton.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWIcon.h"
#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWMultiColumnList.h"
#include "vtkRenderer.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWThumbWheel.h"
#include "vtkKWTkUtilities.h"

#include "vtkActor.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD       1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxViewProperties);
vtkCxxRevisionMacro(vtkKWMimxViewProperties, "$Revision: 1.5 $");

//----------------------------------------------------------------------------
vtkKWMimxViewProperties::vtkKWMimxViewProperties()
{
  this->MultiColumnList = NULL;
  this->ObjectList = NULL;
  this->MimxViewWindow = NULL;
  this->MainFrame = NULL;
  this->ActorColorButton = NULL;
  this->DisplayStyleMenuButton = NULL;
  this->LinewidthThumbwheel = NULL;
  this->OpacityThumbwheel = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxViewProperties::~vtkKWMimxViewProperties()
{
  if(this->MainFrame)
    this->MainFrame->Delete();
  if(this->ActorColorButton)
    this->ActorColorButton->Delete();
  if(this->DisplayStyleMenuButton)
    this->DisplayStyleMenuButton->Delete();
  if(this->LinewidthThumbwheel)
    this->LinewidthThumbwheel->Delete();
  if(this->OpacityThumbwheel)
    this->OpacityThumbwheel->Delete();
}
//----------------------------------------------------------------------------
void vtkKWMimxViewProperties::CreateWidget()
{
  if(this->IsCreated())
  {
    vtkErrorMacro("class already created");
    return;
  }
  this->Superclass::CreateWidget();
  if(!this->MainFrame)
  {
    this->MainFrame = vtkKWFrameWithLabel::New();
  }
  this->MainFrame->SetParent(this->GetParent());
  this->MainFrame->Create();

  this->MainFrame->SetLabelText("View Properties");
  if(!this->MultiColumnList)
  {
    this->MultiColumnList = vtkKWMultiColumnList::New();
  }
  this->MultiColumnList->SetParent(this->MainFrame->GetFrame());

  this->MultiColumnList->Create();
  this->MultiColumnList->SetHeight(5);
 // this->MultiColumnList->SetWidth(10);
  this->MultiColumnList->MovableColumnsOn();
//  this->MultiColumnList->SetPotentialCellColorsChangedCommand(
//    this->MultiColumnList, "ScheduleRefreshColorsOfAllCellsWithWindowCommand");
  int col_index;

  // Add the columns 
  col_index = MultiColumnList->AddColumn("Name");

  col_index = MultiColumnList->AddColumn(NULL);
  MultiColumnList->SetColumnFormatCommandToEmptyOutput(col_index);
  MultiColumnList->SetColumnLabelImageToPredefinedIcon(col_index, vtkKWIcon::IconEye);
 //this->MultiColumnList->InsertCellText(0, 0, "");
 // this->MultiColumnList->InsertCellTextAsInt(0, 1, 0);
 // this->MultiColumnList->SetCellWindowCommandToCheckButton(0, 1);
 // this->MultiColumnList->GetCellWindowAsCheckButton(0,1)->SetEnabled(0);
  this->MultiColumnList->SetSortArrowVisibility(0);
  this->MultiColumnList->ColumnSeparatorsVisibilityOff();
//  this->MultiColumnList->SetSelectioModeToSingle();
//  this->MultiColumnList->RowSeparatorsVisibilityOff();
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
    this->MultiColumnList->GetWidgetName());
// color button
  if(!this->ActorColorButton)
  {
    this->ActorColorButton = vtkKWChangeColorButton::New();
  }
  this->ActorColorButton->SetParent(this->MainFrame->GetFrame());

  this->ActorColorButton->Create();

  this->ActorColorButton->SetColor(0.5,0.5,0.5);
  this->ActorColorButton->SetLabelPositionToLeft();
  this->ActorColorButton->SetLabelText("Actor Color");
  this->ActorColorButton->SetCommand(this, "SetActorColor");
  this->ActorColorButton->SetEnabled(0);
//  this->ActorColorButton->SetColor(this->MainFrame->GetFrame()->GetBackgroundColor());
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 6", 
    this->ActorColorButton->GetWidgetName());

  if(!this->DisplayStyleMenuButton)
  {
    this->DisplayStyleMenuButton = vtkKWMenuButtonWithLabel::New();
  }
  this->DisplayStyleMenuButton->SetParent(this->MainFrame->GetFrame());
  this->DisplayStyleMenuButton->Create();
  this->DisplayStyleMenuButton->SetLabelPositionToLeft();
  this->DisplayStyleMenuButton->SetLabelText("Representation: ");
  this->DisplayStyleMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "Wire frame",this, "WireframeRepresentationCallback");
  this->DisplayStyleMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "Surface",this, "SurfaceRepresentationCallback");
  this->DisplayStyleMenuButton->SetEnabled(0);

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 6", 
    this->DisplayStyleMenuButton->GetWidgetName());

  if(!this->LinewidthThumbwheel)
    this->LinewidthThumbwheel = vtkKWThumbWheel::New();
  this->LinewidthThumbwheel->SetParent(this->MainFrame->GetFrame());
  this->LinewidthThumbwheel->PopupModeOn();
  this->LinewidthThumbwheel->Create();
  this->LinewidthThumbwheel->SetRange(0.0, 100.0);
  this->LinewidthThumbwheel->SetResolution(1.0);
  this->LinewidthThumbwheel->DisplayEntryOn();
  this->LinewidthThumbwheel->DisplayLabelOn();
  this->LinewidthThumbwheel->GetLabel()->SetText("Line width:");
  this->LinewidthThumbwheel->SetCommand(this, "LineWidthCallback");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 6", 
    this->LinewidthThumbwheel->GetWidgetName());
  this->LinewidthThumbwheel->SetEnabled(0);

  if(!this->OpacityThumbwheel)
    this->OpacityThumbwheel = vtkKWThumbWheel::New();
  this->OpacityThumbwheel->SetParent(this->MainFrame->GetFrame());
  this->OpacityThumbwheel->PopupModeOn();
  this->OpacityThumbwheel->Create();
  this->OpacityThumbwheel->SetRange(0.0, 1);
  this->OpacityThumbwheel->SetResolution(0.05);
  this->OpacityThumbwheel->DisplayEntryOn();
  this->OpacityThumbwheel->DisplayLabelOn();
  this->OpacityThumbwheel->GetLabel()->SetText("Opacity :");
  this->OpacityThumbwheel->SetCommand(this, "OpacityCallback");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 6", 
    this->OpacityThumbwheel->GetWidgetName());
  this->OpacityThumbwheel->SetEnabled(0);

}
//----------------------------------------------------------------------------
void vtkKWMimxViewProperties::Update()
{
  this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxViewProperties::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
void vtkKWMimxViewProperties::VisibilityCallback(int flag)
{
  for (int i=0; i<this->ObjectList->GetNumberOfItems(); i++)
  {
    if(this->MultiColumnList->GetCellWindowAsCheckButton(i,1)->GetSelectedState())
    {
      this->GetMimxViewWindow()->GetRenderWidget()->AddViewProp(
        this->ObjectList->GetItem(i)->GetActor());
    }
  else
  {
    this->GetMimxViewWindow()->GetRenderWidget()->RemoveViewProp(
      this->ObjectList->GetItem(i)->GetActor());
  }
  }
  this->GetMimxViewWindow()->GetRenderWidget()->Render();
  this->GetMimxViewWindow()->GetRenderWidget()->ResetCamera();
}
//----------------------------------------------------------------------------
void vtkKWMimxViewProperties::AddObjectList()
{
  //this->GetApplication()->Script("pack forget %s", 
  //  this->MultiColumnList->GetWidgetName());

    this->MultiColumnList->InsertCellText(this->ObjectList->GetNumberOfItems()-1,
    0, this->ObjectList->GetItem(this->ObjectList->GetNumberOfItems()-1)->GetFileName());
  this->MultiColumnList->SetSelectionChangedCommand(this, "SetViewProperties");
//  this->MultiColumnList->SetSortCommand(this, "SortCommand");
    this->MultiColumnList->InsertCellTextAsInt(this->ObjectList->GetNumberOfItems()-1, 1, 1);
    this->MultiColumnList->SetCellWindowCommandToCheckButton(this->ObjectList->GetNumberOfItems()-1, 1);
  this->MultiColumnList->GetCellWindowAsCheckButton(this->ObjectList->GetNumberOfItems()-1,
    1)->SetCommand(this, "VisibilityCallback");
  this->UpdateVisibility();
 /* this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
    this->MultiColumnList->GetWidgetName());*/
}
//----------------------------------------------------------------------------
void vtkKWMimxViewProperties::SetActorColor(double color[3])
{
  int numrows, rows[1];
  numrows = this->MultiColumnList->GetSelectedRows(rows);
  if(numrows)
  {
  this->ObjectList->GetItem(rows[0])->GetActor()->GetProperty()
    ->SetColor(this->ActorColorButton->GetColor());
  this->GetMimxViewWindow()->GetRenderWidget()->Render();
  }
}
//----------------------------------------------------------------------------
void vtkKWMimxViewProperties::SetViewProperties()
{
  int numrows, rows[1];
  numrows = this->MultiColumnList->GetSelectedRows(rows);
  if(numrows)
  {
    this->ActorColorButton->SetColor(this->ObjectList->GetItem(rows[0])
      ->GetActor()->GetProperty()->GetColor());
    this->ActorColorButton->SetEnabled(1);

    int representation = this->ObjectList->GetItem(rows[0])->GetActor()
      ->GetProperty()->GetRepresentation();
    if(representation == 1)
      this->DisplayStyleMenuButton->GetWidget()->SetValue("Wire frame");
    else
      this->DisplayStyleMenuButton->GetWidget()->SetValue("Surface");
    this->DisplayStyleMenuButton->SetEnabled(1);
    
    this->LinewidthThumbwheel->SetValue(this->ObjectList->GetItem(rows[0])
      ->GetActor()->GetProperty()->GetLineWidth());
    this->LinewidthThumbwheel->SetEnabled(1);

    this->OpacityThumbwheel->SetValue(this->ObjectList->GetItem(rows[0])
      ->GetActor()->GetProperty()->GetLineWidth());
    this->OpacityThumbwheel->SetEnabled(1);
  }
  
}
//----------------------------------------------------------------------------
void vtkKWMimxViewProperties::SortCommand()
{
  int x = 0;
}
//----------------------------------------------------------------------------
void vtkKWMimxViewProperties::SurfaceRepresentationCallback()
{
  int numrows, rows[1];
  numrows = this->MultiColumnList->GetSelectedRows(rows);
  if(numrows)
  {
  this->ObjectList->GetItem(rows[0])->GetActor()
    ->GetProperty()->SetRepresentationToSurface();
  this->GetMimxViewWindow()->GetRenderWidget()->Render();
  }
}
//----------------------------------------------------------------------------
void vtkKWMimxViewProperties::WireframeRepresentationCallback()
{
  int numrows, rows[1];
  numrows = this->MultiColumnList->GetSelectedRows(rows);
  if(numrows)
  {
  this->ObjectList->GetItem(rows[0])->GetActor()
    ->GetProperty()->SetRepresentationToWireframe();
  this->GetMimxViewWindow()->GetRenderWidget()->Render();
  }
}
//----------------------------------------------------------------------------
void vtkKWMimxViewProperties::LineWidthCallback(double width)
{
  int numrows, rows[1];
  numrows = this->MultiColumnList->GetSelectedRows(rows);
  if(numrows)
  {
  this->ObjectList->GetItem(rows[0])->GetActor()
    ->GetProperty()->SetLineWidth(this->LinewidthThumbwheel->GetValue());
  this->GetMimxViewWindow()->GetRenderWidget()->Render();
  }
}
//----------------------------------------------------------------------------
void vtkKWMimxViewProperties::UpdateVisibility()
{
  for (int i=0; i<this->ObjectList->GetNumberOfItems(); i++)
  {
    if(this->GetMimxViewWindow()->GetRenderWidget()->GetRenderer()->HasViewProp(
      this->ObjectList->GetItem(i)->GetActor()))
      this->MultiColumnList->GetCellWindowAsCheckButton(i,1)->SetSelectedState(1);
    else
      this->MultiColumnList->GetCellWindowAsCheckButton(i,1)->SetSelectedState(0);
  }
}
//----------------------------------------------------------------------------
void vtkKWMimxViewProperties::OpacityCallback(double width)
{
  int numrows, rows[1];
  numrows = this->MultiColumnList->GetSelectedRows(rows);
  if(numrows)
  {
  this->ObjectList->GetItem(rows[0])->GetActor()
    ->GetProperty()->SetOpacity(this->OpacityThumbwheel->GetValue());
  this->GetMimxViewWindow()->GetRenderWidget()->Render();
  }
}
//----------------------------------------------------------------------------
void vtkKWMimxViewProperties::DeleteObjectList(int Num)
{
  this->MultiColumnList->DeleteRow(Num);
  this->GetMimxViewWindow()->GetRenderWidget()->RemoveViewProp(
    this->ObjectList->GetItem(Num)->GetActor());
  this->GetMimxViewWindow()->GetRenderWidget()->Render();
  this->GetMimxViewWindow()->GetRenderWidget()->ResetCamera();
  if(!MultiColumnList->GetNumberOfRows())
  {
    this->ActorColorButton->SetEnabled(0);
    this->DisplayStyleMenuButton->SetEnabled(0);
    this->LinewidthThumbwheel->SetEnabled(0);
    this->OpacityThumbwheel->SetEnabled(0);
  }
}
//----------------------------------------------------------------------------
void vtkKWMimxViewProperties::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
