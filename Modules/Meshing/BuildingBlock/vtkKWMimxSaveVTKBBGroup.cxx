/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxSaveVTKBBGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.24.4.2 $

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

#include "vtkKWMimxSaveVTKBBGroup.h"

#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkUnstructuredGrid.h"
#include "vtkUnstructuredGridWriter.h"
#include "vtkXMLUnstructuredGridWriter.h"

#include "vtkKWApplication.h"
#include "vtkKWComboBox.h"
#include "vtkKWComboBoxWithLabel.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWNotebook.h"
#include "vtkKWPushButton.h"
#include "vtkKWRenderWidget.h"

#include "vtkMimxErrorCallback.h"
#include "vtkMimxUnstructuredGridActor.h"
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
vtkStandardNewMacro(vtkKWMimxSaveVTKBBGroup);
vtkCxxRevisionMacro(vtkKWMimxSaveVTKBBGroup, "$Revision: 1.24.4.2 $");

//----------------------------------------------------------------------------
vtkKWMimxSaveVTKBBGroup::vtkKWMimxSaveVTKBBGroup()
{
  this->ObjectListComboBox = NULL;
  this->FileBrowserDialog = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxSaveVTKBBGroup::~vtkKWMimxSaveVTKBBGroup()
{
  if(this->ObjectListComboBox)
     this->ObjectListComboBox->Delete();
  if(this->FileBrowserDialog)
          this->FileBrowserDialog->Delete();
 }
//----------------------------------------------------------------------------
void vtkKWMimxSaveVTKBBGroup::CreateWidget()
{
  if(this->IsCreated())
  {
    vtkErrorMacro("class already created");
    return;
  }

  this->Superclass::CreateWidget();
  if(!this->ObjectListComboBox) 
  {
     this->ObjectListComboBox = vtkKWComboBoxWithLabel::New();
  }
  this->MainFrame->SetParent(this->GetParent());
  this->MainFrame->Create();
  //this->MainFrame->SetLabelText("Save BB (VTK file format)");

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 6 -fill x", 
    this->MainFrame->GetWidgetName());

  ObjectListComboBox->SetParent(this->MainFrame);
  ObjectListComboBox->Create();
  ObjectListComboBox->SetLabelText("Building Block : ");
  ObjectListComboBox->GetWidget()->ReadOnlyOn();
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
    ObjectListComboBox->GetWidgetName());

  this->ApplyButton->SetParent(this->MainFrame);
  this->ApplyButton->Create();
  this->ApplyButton->SetReliefToFlat();
  this->ApplyButton->SetImageToIcon( this->GetMimxMainWindow()->GetApplyButtonIcon() );
  //this->ApplyButton->SetText("Apply");
  this->ApplyButton->SetCommand(this, "SaveVTKBBApplyCallback");
  this->GetApplication()->Script(
          "pack %s -side left -anchor nw -expand y -padx 5 -pady 6", 
          this->ApplyButton->GetWidgetName());

  this->CancelButton->SetParent(this->MainFrame);
  this->CancelButton->Create();
  this->CancelButton->SetReliefToFlat();
  this->CancelButton->SetImageToIcon( this->GetMimxMainWindow()->GetCancelButtonIcon() );
  //this->CancelButton->SetText("Cancel");
  this->CancelButton->SetCommand(this, "SaveVTKBBCancelCallback");
  this->GetApplication()->Script(
    "pack %s -side right -anchor ne -expand y -padx 5 -pady 6", 
    this->CancelButton->GetWidgetName());

}
//----------------------------------------------------------------------------
void vtkKWMimxSaveVTKBBGroup::Update()
{
        this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxSaveVTKBBGroup::UpdateEnableState()
{
        this->UpdateObjectLists();
        this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
int vtkKWMimxSaveVTKBBGroup::SaveVTKBBApplyCallback()
{
        vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
        if(!strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
        {
                callback->ErrorMessage("Building Block selection required");
                return 0;
        }

  vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
    const char *name = combobox->GetValue();

        int num = combobox->GetValueIndex(name);
        if(num < 0 || num > combobox->GetNumberOfValues()-1)
        {
                callback->ErrorMessage("Choose valid Building-block structure");
                combobox->SetValue("");
                return 0;
        }

  vtkUnstructuredGrid *ugrid = vtkMimxUnstructuredGridActor::SafeDownCast(
          this->BBoxList->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
  const char *filename = this->BBoxList->GetItem(combobox->GetValueIndex(name))->GetFileName();
        if(!this->FileBrowserDialog)
        {
                this->FileBrowserDialog = vtkKWLoadSaveDialog::New() ;
                this->FileBrowserDialog->SaveDialogOn();
                this->FileBrowserDialog->SetApplication(this->GetApplication());
//              dialog->SetParent(this->RenderWidget->GetParentTopLevel()) ;
                this->FileBrowserDialog->Create();
                this->FileBrowserDialog->RetrieveLastPathFromRegistry("FEMeshDataPath");
                this->FileBrowserDialog->SetTitle ("Save Building Block");
                this->FileBrowserDialog->SetFileTypes ("{{VTK files} {.vtk}} {{VTK XML files} {.xml}} {{All files} {*.*}}");
                this->FileBrowserDialog->SetDefaultExtension (".vtk");
        }
        this->FileBrowserDialog->SetInitialFileName(filename);
        this->FileBrowserDialog->RetrieveLastPathFromRegistry("LastPath");
        this->FileBrowserDialog->Invoke();
        if(this->FileBrowserDialog->GetStatus() == vtkKWDialog::StatusOK)
        {
                if(this->FileBrowserDialog->GetFileName())
                {
                        std::string bbFileName = FileBrowserDialog->GetFileName();
                        
                        std::string extension = vtksys::SystemTools::LowerCase( 
                    vtksys::SystemTools::GetFilenameLastExtension( bbFileName ) );
                    
                        this->GetApplication()->SetRegistryValue(
                                1, "RunTime", "LastPath", vtksys::SystemTools::GetFilenamePath( bbFileName ).c_str());
                        this->FileBrowserDialog->SaveLastPathToRegistry("LastPath");
                        
                        if (extension == ".vtk")
                        {
                        vtkUnstructuredGridWriter *writer = vtkUnstructuredGridWriter::New();
                        writer->SetFileName( bbFileName.c_str() );
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
                        writer->SetFileName( bbFileName.c_str() );
                        writer->SetInput(ugrid);
                        writer->SetDataModeToAscii();
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
                        this->GetMimxMainWindow()->SetStatusText("Saved Building Block");
                        
                        return 1;
                }
        }
        return 0;
}
//----------------------------------------------------------------------------
void vtkKWMimxSaveVTKBBGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxSaveVTKBBGroup::SaveVTKBBCancelCallback()
{
//  this->MainFrame->UnpackChildren();
  this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName());
  this->MenuGroup->SetMenuButtonsEnabled(1);
    this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(1);
}
//-----------------------------------------------------------------------------
void vtkKWMimxSaveVTKBBGroup::UpdateObjectLists()
{
        this->UpdateBuildingBlockComboBox( this->ObjectListComboBox->GetWidget() );
        /*
        this->ObjectListComboBox->GetWidget()->DeleteAllValues();
        
        int defaultItem = -1;
        for (int i = 0; i < this->BBoxList->GetNumberOfItems(); i++)
        {
                ObjectListComboBox->GetWidget()->AddValue(
                        this->BBoxList->GetItem(i)->GetFileName());
          int viewedItem = this->GetMimxMainWindow()->GetRenderWidget()->GetRenderer()->HasViewProp(
                                         this->BBoxList->GetItem(i)->GetActor());
    if ( (defaultItem == -1) && ( viewedItem ) )
                {
                  defaultItem = i;
                }                               
        }
        
        if ((this->BBoxList->GetNumberOfItems() > 0) && (defaultItem == -1))
    defaultItem = this->BBoxList->GetNumberOfItems()-1;
    
        if (defaultItem != -1)
  {
    ObjectListComboBox->GetWidget()->SetValue(
          this->BBoxList->GetItem(defaultItem)->GetFileName());
  }
  */
}
//--------------------------------------------------------------------------------
void vtkKWMimxSaveVTKBBGroup::SaveVTKBBDoneCallback()
{
        if(this->SaveVTKBBApplyCallback())
                this->SaveVTKBBCancelCallback();
}
//---------------------------------------------------------------------------------
