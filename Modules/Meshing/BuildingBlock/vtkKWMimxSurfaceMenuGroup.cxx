/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxSurfaceMenuGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.38.4.2 $

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

#include "vtkKWMimxSurfaceMenuGroup.h"

/* VTK Headers */
#include "vtkActor.h"
#include "vtkObjectFactory.h"
//#include "vtkPolyDataMapper.h"
#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>
#include <vtksys/SystemTools.hxx>

/* KWWidget Headers */
#include "vtkKWApplication.h"
#include "vtkKWComboBoxWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
//#include "vtkKWNotebook.h"
#include "vtkKWRenderWidget.h"

/* MIMX Headers */
#include "vtkMimxErrorCallback.h"
#include "vtkMimxSurfacePolyDataActor.h"
#include "vtkKWMimxDeleteObjectGroup.h"
#include "vtkKWMimxMainNotebook.h"
//#include "vtkKWMimxMainUserInterfacePanel.h"
#include "vtkKWMimxSaveSTLSurfaceGroup.h"
#include "vtkKWMimxLoadSurfaceGroup.h"


#include "vtkKWMimxImportModelSurfaceGroup.h"


//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxSurfaceMenuGroup);
vtkCxxRevisionMacro(vtkKWMimxSurfaceMenuGroup, "$Revision: 1.38.4.2 $");

//----------------------------------------------------------------------------
vtkKWMimxSurfaceMenuGroup::vtkKWMimxSurfaceMenuGroup()
{
        this->SurfaceList = vtkLinkedListWrapper::New();
        
        this->OperationMenuButton = NULL;
  this->SaveSTLGroup = NULL;
  this->DeleteObjectGroup = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxSurfaceMenuGroup::~vtkKWMimxSurfaceMenuGroup()
{
        if (this->SurfaceList) 
        {
                for (int i=0; i < this->SurfaceList->GetNumberOfItems(); i++) {
                        vtkMimxSurfacePolyDataActor::SafeDownCast(
        this->SurfaceList->GetItem(i))->Delete();
                }
                this->SurfaceList->Delete();
        }
        
  if(this->OperationMenuButton)
    this->OperationMenuButton->Delete();
  if(this->SaveSTLGroup)
        this->SaveSTLGroup->Delete();
  if(this->DeleteObjectGroup)
          this->DeleteObjectGroup->Delete();
}

//----------------------------------------------------------------------------
void vtkKWMimxSurfaceMenuGroup::CreateWidget()
{
        if(this->IsCreated())
        {
                vtkErrorMacro("class already created");
                return;
        }
        this->Superclass::CreateWidget();
        
        if(!this->MainFrame)
    this->MainFrame = vtkKWFrame::New();
  this->MainFrame->SetParent(this->GetParent());
  this->MainFrame->Create();
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 6 -fill x", 
    this->MainFrame->GetWidgetName());

        // declare operations menu 
        if(!this->OperationMenuButton)  
                this->OperationMenuButton = vtkKWMenuButtonWithLabel::New();
        this->OperationMenuButton->SetParent(this->MainFrame);
        this->OperationMenuButton->Create();
        this->OperationMenuButton->SetBorderWidth(0);
        this->OperationMenuButton->SetReliefToGroove();
        this->OperationMenuButton->SetPadX(2);
        this->OperationMenuButton->SetPadY(2);
        this->OperationMenuButton->GetWidget()->SetWidth(40);
        this->GetApplication()->Script("pack %s -side top -anchor n -padx 2 -pady 15", 
                this->OperationMenuButton->GetWidgetName());
        this->OperationMenuButton->SetEnabled(1);

        this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
                "Load",this, "LoadSurfaceCallback");
        this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
                "Import Model",this, "ImportModelSurfaceCallback");
        this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
                "Delete",this, "DeleteSurfaceCallback");
        this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
                "Save",this, "SaveSurfaceCallback");
        
        // Set the default value
        this->OperationMenuButton->GetWidget()->SetValue( "Load" );     
}

//----------------------------------------------------------------------------
void vtkKWMimxSurfaceMenuGroup::Update()
{
        this->UpdateEnableState();
}

//----------------------------------------------------------------------------
void vtkKWMimxSurfaceMenuGroup::UpdateEnableState()
{
        this->Superclass::UpdateEnableState();
}

//----------------------------------------------------------------------------
void vtkKWMimxSurfaceMenuGroup::LoadSurfaceCallback()
{
        vtkKWMimxLoadSurfaceGroup *loadSurfaceGroup = vtkKWMimxLoadSurfaceGroup::New();
        loadSurfaceGroup->SetParent( this->GetParent() );
  loadSurfaceGroup->SetSurfaceList(this->SurfaceList);
  loadSurfaceGroup->SetMenuGroup(this);
  loadSurfaceGroup->SetMimxMainWindow(this->GetMimxMainWindow());
  loadSurfaceGroup->Create();
  loadSurfaceGroup->Delete();   
}



//----------------------------------------------------------------------------
void vtkKWMimxSurfaceMenuGroup::ImportModelSurfaceCallback()
{
    vtkKWMimxImportModelSurfaceGroup *importSurfaceGroup = vtkKWMimxImportModelSurfaceGroup::New();
        importSurfaceGroup->SetParent( this->GetParent() );
        importSurfaceGroup->SetSurfaceList(this->SurfaceList);
        importSurfaceGroup->SetMenuGroup(this);
        importSurfaceGroup->SetMimxMainWindow(this->GetMimxMainWindow());
        importSurfaceGroup->Create();
        importSurfaceGroup->Delete();
}


//----------------------------------------------------------------------------
void vtkKWMimxSurfaceMenuGroup::SaveSurfaceCallback()
{

  if (!this->SaveSTLGroup)
        {
                this->SaveSTLGroup = vtkKWMimxSaveSTLSurfaceGroup::New();
                this->SaveSTLGroup->SetParent( this->GetParent() );
                this->SaveSTLGroup->SetSurfaceList(this->SurfaceList);
                this->SaveSTLGroup->SetMenuGroup(this);
                this->SaveSTLGroup->SetMimxMainWindow(this->GetMimxMainWindow());
                this->SaveSTLGroup->Create();
        }
        else
        {
                this->SaveSTLGroup->UpdateObjectLists();
        }
        this->SetMenuButtonsEnabled(0);
          this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(0);

        this->GetApplication()->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 5 -fill x", 
                this->SaveSTLGroup->GetMainFrame()->GetWidgetName());
                
}

//----------------------------------------------------------------------------
void vtkKWMimxSurfaceMenuGroup::DeleteSurfaceCallback()
{
        if(!this->DeleteObjectGroup)
        {
                this->DeleteObjectGroup = vtkKWMimxDeleteObjectGroup::New();
                this->DeleteObjectGroup->SetParent( this->GetParent() );
                this->DeleteObjectGroup->SetFEMeshList(NULL);
                this->DeleteObjectGroup->SetBBoxList(NULL);
                this->DeleteObjectGroup->SetSurfaceList(this->SurfaceList);
                this->DeleteObjectGroup->SetMenuGroup(this);
                this->DeleteObjectGroup->SetMimxMainWindow(this->GetMimxMainWindow());
                this->DeleteObjectGroup->Create();
        }
        else
        {
                this->DeleteObjectGroup->SetFEMeshList(NULL);
                this->DeleteObjectGroup->SetBBoxList(NULL);
                this->DeleteObjectGroup->SetSurfaceList(this->SurfaceList);
                this->UpdateObjectLists();
        }
        this->SetMenuButtonsEnabled(0);
          this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(0);

        this->DeleteObjectGroup->GetObjectListComboBox()->SetLabelText("Surface: ");
        this->GetApplication()->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 5 -fill x", 
                this->DeleteObjectGroup->GetMainFrame()->GetWidgetName());
}

//----------------------------------------------------------------------------
void vtkKWMimxSurfaceMenuGroup::UpdateObjectLists()
{
        if(this->SaveSTLGroup)
                this->SaveSTLGroup->UpdateObjectLists();
        if(this->DeleteObjectGroup)
                this->DeleteObjectGroup->UpdateObjectLists();
}

//----------------------------------------------------------------------------
void vtkKWMimxSurfaceMenuGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  
  os << indent << "SurfaceList: " << this->SurfaceList << endl;
  
  os << indent << "MainFrame: " << this->MainFrame << endl;
  os << indent << "OperationMenuButton: " << this->OperationMenuButton << endl;
  
  os << indent << "SaveSTLGroup: " << this->SaveSTLGroup << endl;
  os << indent << "DeleteObjectGroup: " << this->DeleteObjectGroup << endl;
}
//---------------------------------------------------------------------------
