/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxImageMenuGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.21.4.2 $

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

#include "vtkKWMimxImageMenuGroup.h"

#include "vtkMimxErrorCallback.h"
#include "vtkKWMimxImageViewProperties.h"

#include "vtkMimxActorBase.h"
#include "vtkKWMimxDeleteObjectGroup.h"
#include "vtkMimxImageActor.h"
#include "vtkKWMimxImportImageGroup.h"

#include "vtkActor.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyDataReader.h"
#include "vtkSTLReader.h"
#include "vtkExecutive.h"
#include "vtkCommand.h"
#include "vtkCallbackCommand.h"
#include "vtkKWMessageDialog.h"
#include "vtkMimxErrorCallback.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkImageActorPointPlacer.h"
#include "vtkPolyData.h"
#include "vtkContourRepresentation.h"
//#include "vtkOrientedGlyphContourRepresentation.h"
#include "vtkFocalPlaneContourRepresentation.h"
#include "vtkBoundedPlanePointPlacer.h"
#include "vtkContourWidget.h"
#include "vtkImageActor.h"
#include "vtkRenderer.h"
#include "vtkImageShiftScale.h"

#include "vtkKWApplication.h"
#include "vtkKWCheckButton.h"
#include "vtkKWFileBrowserDialog.h"
#include "vtkKWEvent.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
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
#include "vtkKWMimxMainUserInterfacePanel.h"
#include "vtkKWMimxMainNotebook.h"
#include "vtkKWComboBoxWithLabel.h"

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>
#include <vtksys/SystemTools.hxx>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxImageMenuGroup);
vtkCxxRevisionMacro(vtkKWMimxImageMenuGroup, "$Revision: 1.21.4.2 $");

//----------------------------------------------------------------------------
vtkKWMimxImageMenuGroup::vtkKWMimxImageMenuGroup()
{
        this->OperationMenuButton = NULL;
        this->ImageList = vtkLinkedListWrapper::New();
  this->FileBrowserDialog = NULL;
  this->DeleteObjectGroup = NULL;
}
//----------------------------------------------------------------------------
vtkKWMimxImageMenuGroup::~vtkKWMimxImageMenuGroup()
{
        this->ImageList->Delete();
  if(this->OperationMenuButton)
    this->OperationMenuButton->Delete();
  if(this->DeleteObjectGroup)
    this->DeleteObjectGroup->Delete();
  if(this->FileBrowserDialog)
          this->FileBrowserDialog->Delete();
 }
//----------------------------------------------------------------------------
void vtkKWMimxImageMenuGroup::CreateWidget()
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

        // Operations menu
        if(!this->OperationMenuButton)  
                this->OperationMenuButton = vtkKWMenuButtonWithLabel::New();
        this->OperationMenuButton->SetParent(this->MainFrame);
        this->OperationMenuButton->Create();
        this->OperationMenuButton->SetBorderWidth(0);
        this->OperationMenuButton->SetReliefToGroove();
        this->OperationMenuButton->GetWidget()->SetWidth(40);
        this->GetApplication()->Script("pack %s -side top -anchor n -padx 2 -pady 15", 
                this->OperationMenuButton->GetWidgetName());
        this->OperationMenuButton->SetEnabled(1);
//        this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
//                "Load",this, "LoadImageCallback");
        this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
                "Import",this, "ImportMRMLVolumeCallback");
        this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
                "Delete",this, "DeleteImageCallback");
        
        // Set the default mode
        this->OperationMenuButton->GetWidget()->SetValue( "Import" );
        
}

//----------------------------------------------------------------------------
void vtkKWMimxImageMenuGroup::Update()
{
        this->UpdateEnableState();
}

//----------------------------------------------------------------------------
void vtkKWMimxImageMenuGroup::UpdateEnableState()
{
        this->Superclass::UpdateEnableState();
}


//----------------------------------------------------------------------------
void vtkKWMimxImageMenuGroup::LoadImageCallback()
{
        if(!this->FileBrowserDialog)
        {
                this->FileBrowserDialog = vtkKWFileBrowserDialog::New();
                this->FileBrowserDialog->SetApplication(this->GetApplication());
                this->FileBrowserDialog->Create();
        }
        this->FileBrowserDialog->SetDefaultExtension(".hdr");
        this->FileBrowserDialog->SetFileTypes("{{Analyze files} {.hdr}} {{Meta-Image} {.mhd .mha}} {{NifTI} {.nii .nii.gz}}");
        this->FileBrowserDialog->RetrieveLastPathFromRegistry("LastPath");
        this->FileBrowserDialog->Invoke();
        if(this->FileBrowserDialog->GetStatus() == vtkKWDialog::StatusOK)
        {
                vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
                callback->SetState(0);

                if(!this->FileBrowserDialog->GetFileName())
                {
                        callback->ErrorMessage("File name not chosen");
                        return;
                }

                const char *filename = FileBrowserDialog->GetFileName();
                this->GetApplication()->SetRegistryValue(
                        1, "RunTime", "LastPath", vtksys::SystemTools::GetFilenamePath( filename ).c_str());
                this->FileBrowserDialog->SaveLastPathToRegistry("LastPath");
                this->ImageList->AppendItem(vtkMimxImageActor::New());
                this->ImageList->GetItem(this->ImageList->GetNumberOfItems()-1)->
                        SetDataType(ACTOR_IMAGE);
                vtkMimxImageActor::SafeDownCast(this->ImageList->GetItem(this->ImageList->GetNumberOfItems()-1))->
                        SetInteractor(this->GetMimxMainWindow()->GetRenderWidget()->GetRenderWindowInteractor());
                vtkMimxImageActor::SafeDownCast(this->ImageList->GetItem(this->ImageList->GetNumberOfItems()-1))->
                        SetITKImageFilePath(this->FileBrowserDialog->GetFileName());
                this->ImageList->GetItem(this->ImageList->GetNumberOfItems()-1)->SetFoundationName(
                        this->ExtractFileName(this->FileBrowserDialog->GetFileName()));
                this->GetMimxMainWindow()->GetRenderWidget()->Render();
                this->GetMimxMainWindow()->GetRenderWidget()->ResetCamera();
                this->GetMimxMainWindow()->GetViewProperties()->AddObjectList( 
                  this->ImageList->GetItem(this->ImageList->GetNumberOfItems()-1) );
                
    this->GetMimxMainWindow()->SetStatusText("Loaded Image");
        }
}

//----------------------------------------------------------------------------
void vtkKWMimxImageMenuGroup::ImportImageCallback()
{
        if(!this->FileBrowserDialog)
        {
                this->FileBrowserDialog = vtkKWFileBrowserDialog::New();
                this->FileBrowserDialog->SetApplication(this->GetApplication());
                this->FileBrowserDialog->Create();
        }
        this->FileBrowserDialog->SetDefaultExtension(".dcm");
        this->FileBrowserDialog->SetFileTypes("{{DICOM} {.dcm}}");
        this->FileBrowserDialog->Invoke();
        if(this->FileBrowserDialog->GetStatus() == vtkKWDialog::StatusOK)
        {
                vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
                callback->SetState(0);

                if(!this->FileBrowserDialog->GetFileName())
                {
                        callback->ErrorMessage("File name not chosen");
                        return;
                }

                const char *filename = this->FileBrowserDialog->GetFileName();
                this->ImageList->AppendItem(vtkMimxImageActor::New());
                this->ImageList->GetItem(this->ImageList->GetNumberOfItems()-1)->
                        SetDataType(ACTOR_IMAGE);
                vtkMimxImageActor::SafeDownCast(this->ImageList->GetItem(this->ImageList->GetNumberOfItems()-1))->
                        SetInteractor(this->GetMimxMainWindow()->GetRenderWidget()->GetRenderWindowInteractor());
                vtkMimxImageActor::SafeDownCast(this->ImageList->GetItem(this->ImageList->GetNumberOfItems()-1))->
                        SetITKImageFilePath( filename );
                this->ImageList->GetItem(this->ImageList->GetNumberOfItems()-1)->SetFoundationName(
                        this->ExtractFileName( filename ) );
                this->GetMimxMainWindow()->GetRenderWidget()->Render();
                this->GetMimxMainWindow()->GetRenderWidget()->ResetCamera();
                        this->MimxImageViewProperties->AddObjectList();
//                      this->UpdateObjectLists();
        }
}


//----------------------------------------------------------------------------
void vtkKWMimxImageMenuGroup::ImportMRMLVolumeCallback()
{
    vtkKWMimxImportImageGroup *importImageGroup = vtkKWMimxImportImageGroup::New();
    importImageGroup->SetParent( this->GetParent() );
    importImageGroup->SetImageList(this->ImageList);
    importImageGroup->SetMenuGroup(this);
    importImageGroup->SetMimxMainWindow(this->GetMimxMainWindow());
    importImageGroup->Create();
    importImageGroup->Delete();
}


//----------------------------------------------------------------------------
void vtkKWMimxImageMenuGroup::DeleteImageCallback()
{
        if(!this->DeleteObjectGroup)
        {
                this->DeleteObjectGroup = vtkKWMimxDeleteObjectGroup::New();
                this->DeleteObjectGroup->SetParent( this->GetParent() );
                this->DeleteObjectGroup->SetFEMeshList(NULL);
                this->DeleteObjectGroup->SetBBoxList(NULL);
                this->DeleteObjectGroup->SetSurfaceList(NULL);
                this->DeleteObjectGroup->SetImageList(this->ImageList);
                this->DeleteObjectGroup->SetMenuGroup(this);
                this->DeleteObjectGroup->SetMimxMainWindow(this->GetMimxMainWindow());
                this->DeleteObjectGroup->Create();
                this->DeleteObjectGroup->UpdateObjectLists();
        }
        else
        {
                this->DeleteObjectGroup->SetFEMeshList(NULL);
                this->DeleteObjectGroup->SetBBoxList(NULL);
                this->DeleteObjectGroup->SetSurfaceList(NULL);
                this->DeleteObjectGroup->SetImageList(this->ImageList);
                this->DeleteObjectGroup->UpdateObjectLists();
        }
        this->SetMenuButtonsEnabled(0);
        this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(0);

        this->DeleteObjectGroup->GetObjectListComboBox()->SetLabelText("Image: ");
        this->GetApplication()->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 5 -fill x", 
                this->DeleteObjectGroup->GetMainFrame()->GetWidgetName());
}
//----------------------------------------------------------------------------
void vtkKWMimxImageMenuGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  
  os << indent << "ImageList: " << this->ImageList << endl;
  
  os << indent << "OperationMenuButton: " << this->OperationMenuButton << endl;
  os << indent << "FileBrowserDialog: " << this->FileBrowserDialog << endl;
  os << indent << "DeleteObjectGroup: " << this->DeleteObjectGroup << endl;
}
//---------------------------------------------------------------------------
