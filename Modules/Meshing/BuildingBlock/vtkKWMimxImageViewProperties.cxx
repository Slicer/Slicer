/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxImageViewProperties.cxx,v $
Language:  C++
Date:      $Date: 2008/02/23 16:16:23 $
Version:   $Revision: 1.6 $

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

#include "vtkKWMimxImageViewProperties.h"

#include "vtkMimxImageActor.h"
#include "vtkLocalLinkedListWrapper.h"

#include "vtkKWApplication.h"
#include "vtkKWChangeColorButton.h"
#include "vtkKWCheckButton.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWIcon.h"
#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
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
#define VTK_KW_OPTION_LOAD                 1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxImageViewProperties);
vtkCxxRevisionMacro(vtkKWMimxImageViewProperties, "$Revision: 1.6 $");

//----------------------------------------------------------------------------
vtkKWMimxImageViewProperties::vtkKWMimxImageViewProperties()
{
  this->MultiColumnList = NULL;
  this->ObjectList = NULL;
  this->MimxMainWindow = NULL;
  this->MainFrame = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxImageViewProperties::~vtkKWMimxImageViewProperties()
{
  if(this->MainFrame)
          this->MainFrame->Delete();
  if(this->MultiColumnList)
          this->MultiColumnList->Delete();
}
//----------------------------------------------------------------------------
void vtkKWMimxImageViewProperties::CreateWidget()
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
    this->MultiColumnList = vtkKWMultiColumnListWithScrollbars::New();
  }
  this->MultiColumnList->SetParent(this->MainFrame->GetFrame());

  this->MultiColumnList->Create();
  this->MultiColumnList->SetHorizontalScrollbarVisibility(1);
  this->MultiColumnList->SetVerticalScrollbarVisibility(1);
  this->MultiColumnList->GetWidget()->SetHeight(5);
   int col_index;

  // Add the columns 
  col_index = MultiColumnList->GetWidget()->AddColumn("Name");

  col_index = MultiColumnList->GetWidget()->AddColumn(NULL);
  MultiColumnList->GetWidget()->SetColumnFormatCommandToEmptyOutput(col_index);
  MultiColumnList->GetWidget()->SetColumnLabelImageToPredefinedIcon(
          col_index, vtkKWIcon::IconEye);
  this->MultiColumnList->GetWidget()->SetSortArrowVisibility(0);
  this->MultiColumnList->GetWidget()->ColumnSeparatorsVisibilityOff();
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
    this->MultiColumnList->GetWidgetName());
}
//----------------------------------------------------------------------------
void vtkKWMimxImageViewProperties::Update()
{
        this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxImageViewProperties::UpdateEnableState()
{
        this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
void vtkKWMimxImageViewProperties::VisibilityCallback(int vtkNotUsed(flag))
{
  for (int i=0; i<this->ObjectList->GetNumberOfItems(); i++)
  {
    if(this->MultiColumnList->GetWidget()->GetCellWindowAsCheckButton(i,1)->GetSelectedState())
    {
          vtkMimxImageActor::SafeDownCast(this->ObjectList->GetItem(i))->DisplayActor(1);
     }
        else
        {
                vtkMimxImageActor::SafeDownCast(this->ObjectList->GetItem(i))->DisplayActor(0);
        }
  }
  this->GetMimxMainWindow()->GetRenderWidget()->Render();
  this->GetMimxMainWindow()->GetRenderWidget()->ResetCamera();
}
//----------------------------------------------------------------------------
void vtkKWMimxImageViewProperties::AddObjectList()
{
/*
  if(!this->ImageList)
                this->ImageList = vtkLinkedListWrapper::New();
                
  int rowIndex = this->ImageList->GetNumberOfItems();
        
        std::cout << "Index " << rowIndex << std::endl;
        std::cout << "Filename " << this->ImageList->GetItem(rowIndex)->GetFileName() << std::endl;
        this->MultiColumnList->GetWidget()->InsertCellTextAsInt(rowIndex, 0, 1);
  this->MultiColumnList->GetWidget()->SetCellWindowCommandToCheckButton( rowIndex, 0);
        this->MultiColumnList->GetWidget()->GetCellWindowAsCheckButton(rowIndex,0)->SetCommand(this, "VisibilityCallback");
  this->MultiColumnList->GetWidget()->InsertCellText(rowIndex, 1, this->ImageList->GetItem(rowIndex)->GetFileName());
  */
        //this->MultiColumnList->SetSelectionChangedCommand(this, "SetViewProperties");
        
        /*
    this->MultiColumnList->GetWidget()->InsertCellText(this->ObjectList->GetNumberOfItems()-1,
                0, this->ObjectList->GetItem(this->ObjectList->GetNumberOfItems()-1)->GetFileName());
    this->MultiColumnList->GetWidget()->InsertCellTextAsInt(this->ObjectList->GetNumberOfItems()-1, 1, 1);
    this->MultiColumnList->GetWidget()->SetCellWindowCommandToCheckButton(
                this->ObjectList->GetNumberOfItems()-1, 1);
        this->MultiColumnList->GetWidget()->GetCellWindowAsCheckButton(
                this->ObjectList->GetNumberOfItems()-1,1)->SetCommand(this, "VisibilityCallback");
                */
//      this->UpdateVisibility();
}
//----------------------------------------------------------------------------
void vtkKWMimxImageViewProperties::DeleteObjectList(int Num)
{
        this->MultiColumnList->GetWidget()->DeleteRow(Num);
        vtkMimxImageActor::SafeDownCast(this->ObjectList->GetItem(Num))->DisplayActor(0);
        this->GetMimxMainWindow()->GetRenderWidget()->Render();
        this->GetMimxMainWindow()->GetRenderWidget()->ResetCamera();
}
//----------------------------------------------------------------------------
void vtkKWMimxImageViewProperties::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
