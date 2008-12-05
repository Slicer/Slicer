/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxLoadSurfaceGroup.cxx,v $
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

#include "vtkKWMimxLoadSurfaceGroup.h"
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

#include "vtkSTLReader.h"
#include "vtkPolyDataReader.h"
#include "vtkXMLPolyDataReader.h"

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>
#include <vtksys/SystemTools.hxx>

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD                 1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxLoadSurfaceGroup);
vtkCxxRevisionMacro(vtkKWMimxLoadSurfaceGroup, "$Revision: 1.1.2.1 $");
//----------------------------------------------------------------------------
vtkKWMimxLoadSurfaceGroup::vtkKWMimxLoadSurfaceGroup()
{
  
}

//----------------------------------------------------------------------------
vtkKWMimxLoadSurfaceGroup::~vtkKWMimxLoadSurfaceGroup()
{
  
}
//----------------------------------------------------------------------------
void vtkKWMimxLoadSurfaceGroup::CreateWidget()
{
  vtkKWFileBrowserDialog *fileBrowserDialog = vtkKWFileBrowserDialog::New();
        fileBrowserDialog->SetApplication(this->GetApplication());
        fileBrowserDialog->Create();
        fileBrowserDialog->SetFileTypes("{{STL files} {.stl}} {{VTK files} {.vtk}} {{VTK XML files} {.vtp}} {{All files} {*.*}}");
        fileBrowserDialog->SetDefaultExtension(".stl");
        fileBrowserDialog->RetrieveLastPathFromRegistry("LastPath");
        fileBrowserDialog->Invoke();
        
        if(fileBrowserDialog->GetStatus() == vtkKWDialog::StatusOK)
        {
                vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
                callback->SetState(0);

                if(!fileBrowserDialog->GetFileName())
                {
                        callback->ErrorMessage("File name not chosen");
                        return;
                }

                std::string surfaceFileName = 
                   vtksys::SystemTools::CollapseFullPath( fileBrowserDialog->GetFileName() );
                std::string extension = 
                  vtksys::SystemTools::LowerCase( vtksys::SystemTools::GetFilenameLastExtension( surfaceFileName ) );
                this->GetApplication()->SetRegistryValue(
                        1, "RunTime", "LastPath", vtksys::SystemTools::GetFilenamePath( surfaceFileName ).c_str());
                fileBrowserDialog->SaveLastPathToRegistry("LastPath");
                
                vtkSTLReader *stlReader = vtkSTLReader::New();
                vtkPolyDataReader *vtkReader = vtkPolyDataReader::New();
                vtkXMLPolyDataReader *xmlReader = vtkXMLPolyDataReader::New();
                
                if (extension == ".stl")
                {
        stlReader->SetFileName( surfaceFileName.c_str() );
        stlReader->AddObserver(vtkCommand::ErrorEvent, callback, 1.0);
        stlReader->Update();
    }
    else if (extension == ".vtk")
    {
      vtkReader->SetFileName( surfaceFileName.c_str() );
                vtkReader->AddObserver(vtkCommand::ErrorEvent, callback, 1.0);
                vtkReader->Update();
    }
    else
    {
      xmlReader->SetFileName( surfaceFileName.c_str() );
                xmlReader->AddObserver(vtkCommand::ErrorEvent, callback, 1.0);
                xmlReader->Update();
    }
    
                if(!callback->GetState())
                {
                        std::string foundationName = 
                          vtksys::SystemTools::GetFilenameWithoutExtension( surfaceFileName.c_str() );

                        if (extension == ".stl")
                        {
                        this->AddSurfaceToDisplay(stlReader->GetOutput(), "", foundationName.c_str() );
                        }
                        else if (extension == ".vtk")
                        {
                        this->AddSurfaceToDisplay(vtkReader->GetOutput(), "", foundationName.c_str() );
                        }
                        else 
                        {
                        this->AddSurfaceToDisplay(xmlReader->GetOutput(), "", foundationName.c_str() );
                        }
                        
                        this->UpdateObjectLists();
                        
      std::string surfaceDirectory = vtksys::SystemTools::GetFilenamePath( surfaceFileName );
      this->GetMimxMainWindow()->InitializeWorkingDirectory( surfaceDirectory.c_str() ); 
                        this->GetMimxMainWindow()->SetStatusText("Loaded Surface");
                }
                else
                {
                callback->ErrorMessage("Failed to load the requested file.");
                }
                stlReader->Delete();
                vtkReader->Delete();
                xmlReader->Delete();
        }
}
//----------------------------------------------------------------------------
void vtkKWMimxLoadSurfaceGroup::Update()
{
        this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxLoadSurfaceGroup::UpdateEnableState()
{
        this->UpdateObjectLists();
        this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
void vtkKWMimxLoadSurfaceGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//------------------------------------------------------------------------------
void vtkKWMimxLoadSurfaceGroup::UpdateObjectLists()
{
        
}
//---------------------------------------------------------------------------------
