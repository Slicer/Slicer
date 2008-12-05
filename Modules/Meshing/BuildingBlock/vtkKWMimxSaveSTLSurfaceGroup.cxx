/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxSaveSTLSurfaceGroup.cxx,v $
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

#include "vtkKWMimxSaveSTLSurfaceGroup.h"

#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkRenderer.h"
#include "vtkSTLWriter.h"
#include "vtkMimxSurfacePolyDataActor.h"

#include "vtkKWApplication.h"
#include "vtkKWComboBox.h"
#include "vtkKWComboBoxWithLabel.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWPushButton.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWTkUtilities.h"
#include "vtkObjectFactory.h"
#include "vtkKWComboBoxWithLabel.h"
#include "vtkKWComboBox.h"
#include "vtkKWPushButton.h"
#include "vtkRenderer.h"
#include "vtkKWMimxMainUserInterfacePanel.h"
#include "vtkCommand.h"
#include "vtkKWEntry.h"
#include "vtkXMLPolyDataWriter.h"


#include "vtkKWMimxMainWindow.h"
#include "vtkKWMimxMainNotebook.h"
#include "vtkKWMimxMainUserInterfacePanel.h"
#include "vtkMimxErrorCallback.h"

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>
#include <vtksys/SystemTools.hxx>

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD                 1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxSaveSTLSurfaceGroup);
vtkCxxRevisionMacro(vtkKWMimxSaveSTLSurfaceGroup, "$Revision: 1.29.4.2 $");
//----------------------------------------------------------------------------
vtkKWMimxSaveSTLSurfaceGroup::vtkKWMimxSaveSTLSurfaceGroup()
{
  this->ObjectListComboBox = NULL;
  this->FileBrowserDialog = NULL;
  this->ComponentFrame = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxSaveSTLSurfaceGroup::~vtkKWMimxSaveSTLSurfaceGroup()
{
  if(this->ObjectListComboBox)
     this->ObjectListComboBox->Delete();
  if(this->FileBrowserDialog)
          this->FileBrowserDialog->Delete();
        if (this->ComponentFrame)
          this->ComponentFrame->Delete();
}
//----------------------------------------------------------------------------
void vtkKWMimxSaveSTLSurfaceGroup::CreateWidget()
{
  if(this->IsCreated())
  {
    vtkErrorMacro("class already created");
    return;
  }

  this->Superclass::CreateWidget();

  this->MainFrame->SetParent(this->GetParent());
  this->MainFrame->Create();
  //this->MainFrame->SetLabelText("Save Surface");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 6 -fill x", 
    this->MainFrame->GetWidgetName());

  if (!this->ObjectListComboBox)
    this->ObjectListComboBox = vtkKWComboBoxWithLabel::New();
  this->ObjectListComboBox->SetParent(this->MainFrame);
  this->ObjectListComboBox->Create();
  this->ObjectListComboBox->SetLabelText("Surface : ");
  this->ObjectListComboBox->SetLabelWidth( 15 );
  this->ObjectListComboBox->GetWidget()->ReadOnlyOn();
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
    this->ObjectListComboBox->GetWidgetName());

  this->ApplyButton->SetParent(this->MainFrame);
  this->ApplyButton->Create();
  this->ApplyButton->SetReliefToFlat();
  this->ApplyButton->SetImageToIcon( this->GetMimxMainWindow()->GetApplyButtonIcon() );
  //this->ApplyButton->SetText("Apply");
  this->ApplyButton->SetCommand(this, "SaveSTLSurfaceApplyCallback");
  this->GetApplication()->Script(
          "pack %s -side left -anchor nw -expand y -padx 5 -pady 6", 
          this->ApplyButton->GetWidgetName());

  this->CancelButton->SetParent(this->MainFrame);
  this->CancelButton->Create();
  this->CancelButton->SetReliefToFlat();
  this->CancelButton->SetImageToIcon( this->GetMimxMainWindow()->GetCancelButtonIcon() );
  //this->CancelButton->SetText("Cancel");
  this->CancelButton->SetCommand(this, "SaveSTLSurfaceCancelCallback");
  this->GetApplication()->Script(
    "pack %s -side right -anchor ne -expand y -padx 5 -pady 6", 
    this->CancelButton->GetWidgetName());
}
//----------------------------------------------------------------------------
void vtkKWMimxSaveSTLSurfaceGroup::Update()
{
        this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxSaveSTLSurfaceGroup::UpdateEnableState()
{
        this->UpdateObjectLists();
        this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
int vtkKWMimxSaveSTLSurfaceGroup::SaveSTLSurfaceApplyCallback()
{
        vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
  if(!strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
  {
          callback->ErrorMessage("Surface selection required");
          return 0;
  }
    vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
    const char *name = combobox->GetValue();

        int num = combobox->GetValueIndex(name);
        if(num < 0 || num > combobox->GetNumberOfValues()-1)
        {
                callback->ErrorMessage("Choose valid Surface");
                combobox->SetValue("");
                return 0;
        }

  vtkPolyData *polydata = vtkMimxSurfacePolyDataActor::SafeDownCast(this->SurfaceList
     ->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
  const char *filename = this->SurfaceList->GetItem(combobox->GetValueIndex(name))->GetFileName();
        if(!this->FileBrowserDialog)
        {
                this->FileBrowserDialog = vtkKWLoadSaveDialog::New() ;
                this->FileBrowserDialog->SaveDialogOn();
                this->FileBrowserDialog->SetApplication(this->GetApplication());
                this->FileBrowserDialog->Create();
                this->FileBrowserDialog->RetrieveLastPathFromRegistry("FEMeshDataPath");
                this->FileBrowserDialog->SetTitle ("Save Surface");
                this->FileBrowserDialog->SetFileTypes ("{{STL files} {.stl}} {{VTK files} {.vtk}} {{VTK XML files} {.vtp}} {{All files} {*.*}}");
                this->FileBrowserDialog->SetDefaultExtension (".stl");
        }
        this->FileBrowserDialog->SetInitialFileName(filename);
        this->FileBrowserDialog->RetrieveLastPathFromRegistry("LastPath");
        this->FileBrowserDialog->Invoke();
        
        if(this->FileBrowserDialog->GetStatus() == vtkKWDialog::StatusOK)
        {
                if ( this->FileBrowserDialog->GetFileName() )
                {
                        std::string surfaceFileName = this->FileBrowserDialog->GetFileName();
                        std::string extension = vtksys::SystemTools::LowerCase( 
                    vtksys::SystemTools::GetFilenameLastExtension( surfaceFileName ) );
                                        
                        this->GetApplication()->SetRegistryValue(
                                1, "RunTime", "LastPath", vtksys::SystemTools::GetFilenamePath( surfaceFileName ).c_str());
                        this->FileBrowserDialog->SaveLastPathToRegistry("LastPath");
                        
                        if (extension == ".stl")
                        {
                        vtkSTLWriter *writer = vtkSTLWriter::New();
                        writer->SetFileName( surfaceFileName.c_str() );
                        writer->SetInput(polydata);
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
                        else if (extension == ".vtk")
                        {
                        vtkPolyDataWriter *writer = vtkPolyDataWriter::New();
                        writer->SetFileName( surfaceFileName.c_str() );
                        writer->SetInput(polydata);
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
                        vtkXMLPolyDataWriter *writer = vtkXMLPolyDataWriter::New();
                        writer->SetFileName( surfaceFileName.c_str() );
                        writer->SetInput(polydata);
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
                                
                        this->GetMimxMainWindow()->SetStatusText("Saved Surface");
                        
                        return 1;
                }
        }
        return 0;
}
//----------------------------------------------------------------------------
void vtkKWMimxSaveSTLSurfaceGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxSaveSTLSurfaceGroup::SaveSTLSurfaceCancelCallback()
{
//  this->MainFrame->UnpackChildren();
  this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName());
  this->MenuGroup->SetMenuButtonsEnabled(1);
    this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(1);
}
//------------------------------------------------------------------------------
void vtkKWMimxSaveSTLSurfaceGroup::UpdateObjectLists()
{
        this->UpdateSurfaceComboBox( this->ObjectListComboBox->GetWidget() );
        /*
        this->ObjectListComboBox->GetWidget()->DeleteAllValues();
        
        int defaultItem = -1;
        for (int i = 0; i < this->SurfaceList->GetNumberOfItems(); i++)
        {
                ObjectListComboBox->GetWidget()->AddValue(
                        this->SurfaceList->GetItem(i)->GetFileName());
                        
                int viewedItem = this->GetMimxMainWindow()->GetRenderWidget()->GetRenderer()->HasViewProp(
                        this->SurfaceList->GetItem(i)->GetActor());
                if ( (defaultItem == -1) && ( viewedItem ) )
                {
                  defaultItem = i;
                }
        }
        
        if (defaultItem != -1)
  {
    ObjectListComboBox->GetWidget()->SetValue(
          this->SurfaceList->GetItem(defaultItem)->GetFileName());
  }
  */
}
//--------------------------------------------------------------------------------
void vtkKWMimxSaveSTLSurfaceGroup::SaveSTLSurfaceDoneCallback()
{
        if(this->SaveSTLSurfaceApplyCallback())
                this->SaveSTLSurfaceCancelCallback();
}
//---------------------------------------------------------------------------------
