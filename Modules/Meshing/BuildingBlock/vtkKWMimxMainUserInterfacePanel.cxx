/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxMainUserInterfacePanel.cxx,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.4.4.2 $

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

#include "vtkKWMimxMainUserInterfacePanel.h"

#include "vtkActor.h"
#include "vtkPolyDataMapper.h"
#include "vtkSTLReader.h"

#include "vtkKWApplication.h"
#include "vtkKWFileBrowserDialog.h"
#include "vtkKWEvent.h"
#include "vtkKWFrame.h"
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
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkMimxErrorCallback.h"
#include "vtkKWMimxImageMenuGroup.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWNotebook.h"
#include "vtkKWMimxQualityMenuGroup.h"
#include "vtkKWMimxMainNotebook.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWMimxViewPropertiesGroup.h"
#include "vtkKWMimxViewProperties.h"
#include "vtkKWMimxDisplayPropertiesGroup.h"
#include "vtkKWUserInterfaceManager.h"

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD                 1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxMainUserInterfacePanel);
vtkCxxRevisionMacro(vtkKWMimxMainUserInterfacePanel, "$Revision: 1.4.4.2 $");

//----------------------------------------------------------------------------
vtkKWMimxMainUserInterfacePanel::vtkKWMimxMainUserInterfacePanel()
{
        this->MainFrame = NULL;
        this->MimxMainWindow = NULL;
        this->UserInterfacePanel = NULL;
        this->DoUndoTree = NULL;
        this->MimxMainNotebook = NULL;
        this->MultiColumnList = NULL;
        this->ViewPropertiesGroup = NULL;
        this->DisplayPropertiesGroup = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxMainUserInterfacePanel::~vtkKWMimxMainUserInterfacePanel()
{
  if(this->MainFrame)
                this->MainFrame->Delete();
  if(this->MimxMainNotebook)
          this->MimxMainNotebook->Delete();
  if(this->ViewPropertiesGroup)
          this->ViewPropertiesGroup->Delete();
  if(this->DisplayPropertiesGroup)
          this->DisplayPropertiesGroup->Delete();
}
//----------------------------------------------------------------------------
void vtkKWMimxMainUserInterfacePanel::CreateWidget()
{
        if(this->IsCreated())
        {
                vtkErrorMacro("class already created");
                return;
        }
        this->Superclass::CreateWidget();
        
        if (!this->MainFrame)   
          this->MainFrame = vtkKWFrame::New();
        this->MainFrame->SetParent(this->GetParent());
  this->MainFrame->Create();
  //this->MainFrame->GetFrame()->SetReliefToGroove();
  //this->MainFrame->SetLabelText("Options");
 // this->MainFrame->AllowFrameToCollapseOn();
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand yes -padx 2 -pady 2 -fill both", 
    this->MainFrame->GetWidgetName());
  
   cout << "instantiating notebook inside interfacePanel" << endl; 
/*      
        if (!this->UserInterfacePanel)  
          this->UserInterfacePanel = vtkKWNotebook::New();
//      this->UserInterfacePanel->SetName("Display Interface");
        this->UserInterfacePanel->SetParent(this->MainFrame->GetFrame());
        this->UserInterfacePanel->Create();

        cout << "created notebook as UserInterfacePanel " << endl; 
    this->UserInterfacePanel->SetApplication(this->GetApplication());
        this->UserInterfacePanel->AddPage("Operation");
    cout << "done with UIPanel AddPage " << endl; 


        //this->UserInterfacePanel->AddPage("Display");
        //this->UserInterfacePanel->AddPage("View");
        this->UserInterfacePanel->SetWidth( 50 );
        this->UserInterfacePanel->SetHeight( 100 );
        this->UserInterfacePanel->SetMinimumWidth( 400 );
        this->UserInterfacePanel->SetMinimumHeight( 500 ); 
 cout << "1" << endl;;
        this->GetApplication()->Script("pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x -fill y", 
                this->UserInterfacePanel->GetWidgetName());
    cout << "2" << endl;  
*/
        if(!this->MimxMainNotebook)
        {
                this->MimxMainNotebook = vtkKWMimxMainNotebook::New();
                this->MimxMainNotebook->SetMimxMainWindow(this->GetMimxMainWindow());
                this->MimxMainNotebook->SetDoUndoTree(this->DoUndoTree);
        }
        this->MimxMainNotebook->SetParent( this->MainFrame );
        this->MimxMainNotebook->SetApplication(this->GetApplication());
 cout << "3" << endl;
        this->MimxMainNotebook->Create();
           cout << "created mimxMainNotebook " << endl; 
cout << "4" << endl;
        this->MimxMainNotebook->SetBorderWidth(3);
        this->MimxMainNotebook->SetReliefToGroove();
        //      this->MainNotebook->SetWidth(200);
        this->GetApplication()->Script(
                "pack %s -side top -anchor nw -expand yes -pady 2 -fill both", 
                this->MimxMainNotebook->GetWidgetName());
        // view properties
/*      this->ViewPropertiesGroup = vtkKWMimxViewPropertiesGroup::New();
        this->ViewPropertiesGroup->SetMimxMainWindow(this->GetMimxMainWindow());
        this->ViewPropertiesGroup->SetMultiColumnList(this->MultiColumnList);
        this->ViewPropertiesGroup->SetObjectList(
                this->GetMimxMainWindow()->GetViewProperties()->GetObjectList());
        this->ViewPropertiesGroup->SetParent(
                this->UserInterfacePanel->GetFrame("Display"));
        this->ViewPropertiesGroup->SetApplication(this->GetApplication());
        this->ViewPropertiesGroup->Create();
        this->ViewPropertiesGroup->SetBorderWidth(3);
        this->ViewPropertiesGroup->SetReliefToGroove();
*/
//      this->ViewPropertiesGroup->GetMainFrame()->ExpandFrame();
        //      this->MainNotebook->SetWidth(200);
/*      this->GetApplication()->Script(
                "pack %s -side top -anchor nw -expand n -fill y -pady 2 -fill x", 
                this->ViewPropertiesGroup->GetWidgetName());
*/      
        // display properties
/*      this->DisplayPropertiesGroup = vtkKWMimxDisplayPropertiesGroup::New();
        this->DisplayPropertiesGroup->SetMimxMainWindow(this->GetMimxMainWindow());
        this->DisplayPropertiesGroup->SetParent(
                this->UserInterfacePanel->GetFrame("View"));
        this->DisplayPropertiesGroup->SetApplication(this->GetApplication());
        this->DisplayPropertiesGroup->Create();
        this->DisplayPropertiesGroup->SetBorderWidth(3);
        this->DisplayPropertiesGroup->SetReliefToGroove();
        this->GetApplication()->Script(
                "pack %s -side top -anchor nw -expand n -fill y -pady 2 -fill x", 
                this->DisplayPropertiesGroup->GetWidgetName());
*/
}
//----------------------------------------------------------------------------
void vtkKWMimxMainUserInterfacePanel::Update()
{
        this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxMainUserInterfacePanel::UpdateEnableState()
{
        this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
void vtkKWMimxMainUserInterfacePanel::SetLists()
{
}
//----------------------------------------------------------------------------
void vtkKWMimxMainUserInterfacePanel::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
