/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxLoadMeshGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:39 $
Version:   $Revision: 1.1.2.1 $

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

#include "vtkKWMimxLoadMeshGroup.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkKWMimxMainMenuGroup.h"
#include "vtkMimxErrorCallback.h"
#include "vtkKWMimxMainNotebook.h"

#include "vtkLinkedListWrapper.h"
#include "vtkPolyData.h"

#include "vtkKWApplication.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWEvent.h"
#include "vtkObjectFactory.h"
/*
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWNotebook.h"
#include "vtkKWOptions.h"
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
*/

#include "vtkUnstructuredGridReader.h"
#include "vtkXMLUnstructuredGridReader.h"

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>
#include <vtksys/SystemTools.hxx>

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD                 1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxLoadMeshGroup);
vtkCxxRevisionMacro(vtkKWMimxLoadMeshGroup, "$Revision: 1.1.2.1 $");
//----------------------------------------------------------------------------
vtkKWMimxLoadMeshGroup::vtkKWMimxLoadMeshGroup()
{
  
}

//----------------------------------------------------------------------------
vtkKWMimxLoadMeshGroup::~vtkKWMimxLoadMeshGroup()
{
  
}
//----------------------------------------------------------------------------
void vtkKWMimxLoadMeshGroup::CreateWidget()
{
  vtkKWFileBrowserDialog *fileBrowserDialog = vtkKWFileBrowserDialog::New();
        fileBrowserDialog->SetApplication(this->GetApplication());
        fileBrowserDialog->Create();
        fileBrowserDialog->SetTitle ("Load Mesh");
        fileBrowserDialog->SetFileTypes("{{VTK files} {.vtk}} {{VTK XML files} {.vtu}} {{All files} {*.*}}");
        fileBrowserDialog->SetDefaultExtension(".vtk");
        fileBrowserDialog->RetrieveLastPathFromRegistry("LastPath");
        fileBrowserDialog->Invoke();
        
        if (fileBrowserDialog->GetStatus() == vtkKWDialog::StatusOK)
        {
                vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
                callback->SetState(0);

                if(!fileBrowserDialog->GetFileName())
                {
                        callback->ErrorMessage("File name not chosen");
                        return;
                }

                std::string meshFileName = 
                   vtksys::SystemTools::CollapseFullPath( fileBrowserDialog->GetFileName() );
                std::string extension = 
                  vtksys::SystemTools::LowerCase( vtksys::SystemTools::GetFilenameLastExtension( meshFileName ) );
                this->GetApplication()->SetRegistryValue(
                        1, "RunTime", "LastPath", vtksys::SystemTools::GetFilenamePath( meshFileName ).c_str());
                fileBrowserDialog->SaveLastPathToRegistry("LastPath");
                
                vtkUnstructuredGridReader *vtkReader = vtkUnstructuredGridReader::New();
                vtkXMLUnstructuredGridReader *xmlReader = vtkXMLUnstructuredGridReader::New();
                
                if (extension == ".vtk")
    {
      vtkReader->SetFileName( meshFileName.c_str() );
                vtkReader->AddObserver(vtkCommand::ErrorEvent, callback, 1.0);
                vtkReader->Update();
    }
    else
    {
      xmlReader->SetFileName( meshFileName.c_str() );
                xmlReader->AddObserver(vtkCommand::ErrorEvent, callback, 1.0);
                xmlReader->Update();
    }
    
                if(!callback->GetState())
                {
                        std::string foundationName = 
                          vtksys::SystemTools::GetFilenameWithoutExtension( meshFileName.c_str() );

                        if (extension == ".vtk")
                        {
                        this->AddMeshToDisplay(vtkReader->GetOutput(), "", foundationName.c_str(), NULL );
                        }
                        else 
                        {
                        this->AddMeshToDisplay(xmlReader->GetOutput(), "", foundationName.c_str(), NULL );
                        }
                        
                        this->UpdateObjectLists();
                        
      std::string meshDirectory = vtksys::SystemTools::GetFilenamePath( meshFileName );
      this->GetMimxMainWindow()->InitializeWorkingDirectory( meshDirectory.c_str() ); 
                        this->GetMimxMainWindow()->SetStatusText("Loaded Mesh");
                }
                else
                {
                callback->ErrorMessage("Failed to load the requested file.");
                }
                vtkReader->Delete();
                xmlReader->Delete();
        }
}
//----------------------------------------------------------------------------
void vtkKWMimxLoadMeshGroup::Update()
{
        this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxLoadMeshGroup::UpdateEnableState()
{
        this->UpdateObjectLists();
        this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
void vtkKWMimxLoadMeshGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//------------------------------------------------------------------------------
void vtkKWMimxLoadMeshGroup::UpdateObjectLists()
{
        
}
//---------------------------------------------------------------------------------
