/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxSaveVTKFEMeshGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.29.4.2 $

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

#include "vtkKWMimxSaveVTKFEMeshGroup.h"

#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkUnstructuredGrid.h"
#include "vtkUnstructuredGridWriter.h"
#include "vtkXMLUnstructuredGridWriter.h"

#include "vtkKWApplication.h"
#include "vtkKWComboBox.h"
#include "vtkKWComboBoxWithLabel.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWNotebook.h"
#include "vtkKWPushButton.h"
#include "vtkKWRenderWidget.h"

#include "vtkMimxErrorCallback.h"
#include "vtkMimxMeshActor.h"
#include "vtkLinkedListWrapper.h"
#include "vtkKWMimxMainUserInterfacePanel.h"
#include "vtkKWMimxMainNotebook.h"

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>
#include <vtksys/SystemTools.hxx>

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD                 1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxSaveVTKFEMeshGroup);
vtkCxxRevisionMacro(vtkKWMimxSaveVTKFEMeshGroup, "$Revision: 1.29.4.2 $");

//----------------------------------------------------------------------------
vtkKWMimxSaveVTKFEMeshGroup::vtkKWMimxSaveVTKFEMeshGroup()
{
  this->ObjectListComboBox = NULL;
  this->FileBrowserDialog = NULL;
  this->ComponentFrame = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxSaveVTKFEMeshGroup::~vtkKWMimxSaveVTKFEMeshGroup()
{
  if(this->ObjectListComboBox)
     this->ObjectListComboBox->Delete();
  if(this->FileBrowserDialog)
          this->FileBrowserDialog->Delete();
        if(this->ComponentFrame)
          this->ComponentFrame->Delete();
 }
//----------------------------------------------------------------------------
void vtkKWMimxSaveVTKFEMeshGroup::CreateWidget()
{
  if(this->IsCreated())
  {
  vtkErrorMacro("class already created");
    return;
  }

  this->Superclass::CreateWidget();
  this->MainFrame->SetParent(this->GetParent());
  this->MainFrame->Create();
  //this->MainFrame->SetLabelText("Save FE Mesh (VTK file format)");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 6 -fill x", 
    this->MainFrame->GetWidgetName());

  if (!this->ObjectListComboBox)        
     this->ObjectListComboBox = vtkKWComboBoxWithLabel::New();  
  ObjectListComboBox->SetParent(this->MainFrame);
  ObjectListComboBox->Create();
  ObjectListComboBox->SetLabelText("Mesh : ");
  ObjectListComboBox->SetLabelWidth( 15 );
  ObjectListComboBox->GetWidget()->ReadOnlyOn();
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
    ObjectListComboBox->GetWidgetName());

  this->ApplyButton->SetParent(this->MainFrame);
  this->ApplyButton->Create();
  this->ApplyButton->SetReliefToFlat();
  this->ApplyButton->SetImageToIcon( this->GetMimxMainWindow()->GetApplyButtonIcon() );
  this->ApplyButton->SetCommand(this, "SaveVTKFEMeshApplyCallback");
  this->GetApplication()->Script(
          "pack %s -side left -anchor nw -expand y -padx 5 -pady 6", 
          this->ApplyButton->GetWidgetName());

  this->CancelButton->SetParent(this->MainFrame);
  this->CancelButton->Create();
  this->CancelButton->SetReliefToFlat();
  this->CancelButton->SetImageToIcon( this->GetMimxMainWindow()->GetCancelButtonIcon() );
  this->CancelButton->SetCommand(this, "SaveVTKFEMeshCancelCallback");
  this->GetApplication()->Script(
    "pack %s -side right -anchor ne -expand y -padx 5 -pady 6", 
    this->CancelButton->GetWidgetName());

}
//----------------------------------------------------------------------------
void vtkKWMimxSaveVTKFEMeshGroup::Update()
{
        this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxSaveVTKFEMeshGroup::UpdateEnableState()
{
        this->UpdateObjectLists();
        this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
int vtkKWMimxSaveVTKFEMeshGroup::SaveVTKFEMeshApplyCallback()
{
        vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
        if(!strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
        {
                callback->ErrorMessage("FE Mesh selection required");;
                return 0;
        }

  vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
    const char *name = combobox->GetValue();

        int num = combobox->GetValueIndex(name);
        if(num < 0 || num > combobox->GetNumberOfValues()-1)
        {
                callback->ErrorMessage("Choose valid FE Mesh");
                combobox->SetValue("");
                return 0;
        }

  vtkUnstructuredGrid *ugrid = vtkMimxMeshActor::SafeDownCast(this->FEMeshList
     ->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
  const char *filename = this->FEMeshList->GetItem(combobox->GetValueIndex(name))->GetFileName();
        if(!this->FileBrowserDialog)
        {
                this->FileBrowserDialog = vtkKWLoadSaveDialog::New() ;
                this->FileBrowserDialog->SaveDialogOn();
                this->FileBrowserDialog->SetApplication(this->GetApplication());
//              dialog->SetParent(this->RenderWidget->GetParentTopLevel()) ;
                this->FileBrowserDialog->Create();      
                this->FileBrowserDialog->RetrieveLastPathFromRegistry("FEMeshDataPath");
                this->FileBrowserDialog->SetTitle ("Save Mesh");
                this->FileBrowserDialog->SetFileTypes ("{{VTK files} {.vtk}} {{VTK XML files} {.vtu}} {{All files} {*.*}}");
                this->FileBrowserDialog->SetDefaultExtension (".vtk");
        }
        this->FileBrowserDialog->SetInitialFileName(filename);
        this->FileBrowserDialog->RetrieveLastPathFromRegistry("LastPath");
        this->FileBrowserDialog->Invoke();
        if(this->FileBrowserDialog->GetStatus() == vtkKWDialog::StatusOK)
        {
                if(this->FileBrowserDialog->GetFileName())
                {
                        std::string meshFileName = FileBrowserDialog->GetFileName();
                        std::string extension = vtksys::SystemTools::LowerCase( 
                    vtksys::SystemTools::GetFilenameLastExtension( meshFileName ) );
                    
                        this->GetApplication()->SetRegistryValue(
                                1, "RunTime", "LastPath", vtksys::SystemTools::GetFilenamePath( meshFileName ).c_str());
                        this->FileBrowserDialog->SaveLastPathToRegistry("LastPath");
                        
                        if ( extension == ".vtk" )
                        {
                        vtkUnstructuredGridWriter *writer = vtkUnstructuredGridWriter::New();
                        writer->SetFileName( meshFileName.c_str() );
                        writer->SetInput(ugrid);
                        writer->AddObserver(vtkCommand::ErrorEvent, callback, 1.0);
                        writer->Update();
                        writer->RemoveObserver(callback);
                        if(callback->GetState())
                        {
                                writer->Delete();
                                return 0;
                        }
                        writer->Delete();
                }
                        else
                        {
                        vtkXMLUnstructuredGridWriter *writer = vtkXMLUnstructuredGridWriter::New();
                        writer->SetFileName( meshFileName.c_str() );
                        writer->SetInput(ugrid);
                        writer->AddObserver(vtkCommand::ErrorEvent, callback, 1.0);
                        writer->SetDataModeToAscii();
                        writer->Update();
                        writer->RemoveObserver(callback);
                        if(callback->GetState())
                        {
                                writer->Delete();
                                return 0;
                        }
                        writer->Delete();
                }               
                        this->GetMimxMainWindow()->SetStatusText("Saved Mesh");
                        return 1;
                }
        }
        return 0;
}
//----------------------------------------------------------------------------
void vtkKWMimxSaveVTKFEMeshGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxSaveVTKFEMeshGroup::SaveVTKFEMeshCancelCallback()
{
//  this->MainFrame->UnpackChildren();
  this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName());
  this->MenuGroup->SetMenuButtonsEnabled(1);
    this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(1);
}
//------------------------------------------------------------------------------
void vtkKWMimxSaveVTKFEMeshGroup::UpdateObjectLists()
{
        this->UpdateMeshComboBox( this->ObjectListComboBox->GetWidget() );
/*      
        this->ObjectListComboBox->GetWidget()->DeleteAllValues();
        
        int defaultItem = -1;
        for (int i = 0; i < this->FEMeshList->GetNumberOfItems(); i++)
        {
                ObjectListComboBox->GetWidget()->AddValue(
                        this->FEMeshList->GetItem(i)->GetFileName());
                
                vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(this->FEMeshList->GetItem(i));  
    bool viewedItem = meshActor->GetMeshVisibility();
    
                if ( (defaultItem == -1) && ( viewedItem ) )
                {
                  defaultItem = i;
                }
        }
        
        if ((this->FEMeshList->GetNumberOfItems() > 0) && (defaultItem == -1))
    defaultItem = this->FEMeshList->GetNumberOfItems()-1;
    
        if (defaultItem != -1)
  {
    ObjectListComboBox->GetWidget()->SetValue(
          this->FEMeshList->GetItem(defaultItem)->GetFileName());
  }
*/
}
//---------------------------------------------------------------------------------
