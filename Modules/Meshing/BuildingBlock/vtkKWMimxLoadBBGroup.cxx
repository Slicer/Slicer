/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxLoadBBGroup.cxx,v $
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

#include "vtkKWMimxLoadBBGroup.h"
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
vtkStandardNewMacro(vtkKWMimxLoadBBGroup);
vtkCxxRevisionMacro(vtkKWMimxLoadBBGroup, "$Revision: 1.1.2.1 $");
//----------------------------------------------------------------------------
vtkKWMimxLoadBBGroup::vtkKWMimxLoadBBGroup()
{
  
}

//----------------------------------------------------------------------------
vtkKWMimxLoadBBGroup::~vtkKWMimxLoadBBGroup()
{
  
}
//----------------------------------------------------------------------------
void vtkKWMimxLoadBBGroup::CreateWidget()
{
  vtkKWFileBrowserDialog *fileBrowserDialog = vtkKWFileBrowserDialog::New();
        fileBrowserDialog->SetApplication(this->GetApplication());
        fileBrowserDialog->Create();
        fileBrowserDialog->SetTitle ("Load Building Block");
        fileBrowserDialog->SetFileTypes("{{VTK files} {.vtk}} {{VTK XML files} {.vtu}} {{All files} {*.*}}");
        fileBrowserDialog->SetDefaultExtension(".vtk");
        fileBrowserDialog->RetrieveLastPathFromRegistry("LastPath");
        fileBrowserDialog->Invoke();
        
        if (fileBrowserDialog->GetStatus() == vtkKWDialog::StatusOK)
        {
                vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
                callback->SetState(0);

                if (!fileBrowserDialog->GetFileName())
                {
                        callback->ErrorMessage("File name not chosen");
                        return;
                }

                std::string bbFileName = 
                   vtksys::SystemTools::CollapseFullPath( fileBrowserDialog->GetFileName() );
                std::string extension = 
                  vtksys::SystemTools::LowerCase( vtksys::SystemTools::GetFilenameLastExtension( bbFileName ) );
                this->GetApplication()->SetRegistryValue(
                        1, "RunTime", "LastPath", vtksys::SystemTools::GetFilenamePath( bbFileName ).c_str());
                fileBrowserDialog->SaveLastPathToRegistry("LastPath");
                
                vtkUnstructuredGridReader *vtkReader = vtkUnstructuredGridReader::New();
                vtkXMLUnstructuredGridReader *xmlReader = vtkXMLUnstructuredGridReader::New();
                
                if (extension == ".vtk")
    {
      vtkReader->SetFileName( bbFileName.c_str() );
                vtkReader->AddObserver(vtkCommand::ErrorEvent, callback, 1.0);
                vtkReader->Update();
    }
    else
    {
      xmlReader->SetFileName( bbFileName.c_str() );
                xmlReader->AddObserver(vtkCommand::ErrorEvent, callback, 1.0);
                xmlReader->Update();
    }
    
                if (!callback->GetState())
                {
                        std::string foundationName = 
                          vtksys::SystemTools::GetFilenameWithoutExtension( bbFileName.c_str() );

                        if (extension == ".vtk")
                        {
                        std::cout << "Load VTK" << std::endl;
                        this->AddBuildingBlockToDisplay(vtkReader->GetOutput(), "", foundationName.c_str() );
                        }
                        else 
                        {
                        std::cout << "Load XML" << std::endl;
                        this->AddBuildingBlockToDisplay(xmlReader->GetOutput(), "", foundationName.c_str() );
                        }
                        
                        this->UpdateObjectLists();
                        
      std::string bbDirectory = vtksys::SystemTools::GetFilenamePath( bbFileName );
      this->GetMimxMainWindow()->InitializeWorkingDirectory( bbDirectory.c_str() );      
                        this->GetMimxMainWindow()->SetStatusText("Loaded Building Block");
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
void vtkKWMimxLoadBBGroup::Update()
{
        this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxLoadBBGroup::UpdateEnableState()
{
        this->UpdateObjectLists();
        this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
void vtkKWMimxLoadBBGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//------------------------------------------------------------------------------
void vtkKWMimxLoadBBGroup::UpdateObjectLists()
{
        
}
//---------------------------------------------------------------------------------
