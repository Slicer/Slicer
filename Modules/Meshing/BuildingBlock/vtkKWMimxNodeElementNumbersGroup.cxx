/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxNodeElementNumbersGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:39 $
Version:   $Revision: 1.6.4.1 $

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

#include "vtkKWMimxNodeElementNumbersGroup.h"

#include "vtkObjectFactory.h"

#include "vtkKWApplication.h"
#include "vtkKWComboBox.h"
#include "vtkKWComboBoxWithLabel.h"
#include "vtkKWEntry.h"
#include "vtkKWFrame.h"
#include "vtkKWLabel.h"

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD                 1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxNodeElementNumbersGroup);
vtkCxxRevisionMacro(vtkKWMimxNodeElementNumbersGroup, "$Revision: 1.6.4.1 $");

//----------------------------------------------------------------------------
vtkKWMimxNodeElementNumbersGroup::vtkKWMimxNodeElementNumbersGroup()
{
  this->ObjectListComboBox = NULL;
  this->NodeNumberEntry = NULL;
  this->ElementNumberEntry = NULL;
  this->NodeSetNameEntry = NULL;
  this->ElementSetNameEntry = NULL;
  this->IdLabel = NULL;
  this->NumberLabel = NULL;
  this->NodeLabel = NULL;
  this->ElementLabel = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxNodeElementNumbersGroup::~vtkKWMimxNodeElementNumbersGroup()
{
  if(this->ObjectListComboBox)
     this->ObjectListComboBox->Delete();
  if(this->NodeNumberEntry)
          this->NodeNumberEntry->Delete();
  if(this->ElementNumberEntry)
          this->ElementNumberEntry->Delete();
  if(this->NodeSetNameEntry)
          this->NodeSetNameEntry->Delete();
  if(this->ElementSetNameEntry)
          this->ElementSetNameEntry->Delete();
        if (this->IdLabel)
          this->IdLabel->Delete();
        if (this->NumberLabel)
          this->NumberLabel->Delete();
        if (this->NodeLabel)
          this->NodeLabel->Delete();
        if (this->ElementLabel)
          this->ElementLabel->Delete();
}
//----------------------------------------------------------------------------
void vtkKWMimxNodeElementNumbersGroup::CreateWidget()
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
  //this->MainFrame->SetLabelText("Node & Elements");

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
    this->MainFrame->GetWidgetName());

  if(!this->NumberLabel)
          this->NumberLabel = vtkKWLabel::New();
        this->NumberLabel->SetParent(this->MainFrame);
  this->NumberLabel->Create();
  this->NumberLabel->SetText("#");
  this->GetApplication()->Script(
          "grid %s -row 0 -column 1 -sticky nswe -padx 2 -pady 2",
          this->NumberLabel->GetWidgetName());
          
        if(!this->IdLabel)
          this->IdLabel = vtkKWLabel::New();
        this->IdLabel->SetParent(this->MainFrame);
  this->IdLabel->Create();
  this->IdLabel->SetText("Label");
  this->GetApplication()->Script(
          "grid %s -row 0 -column 2 -sticky nswe -padx 2 -pady 2",
          this->IdLabel->GetWidgetName());
  
  if(!this->NodeLabel)
          this->NodeLabel = vtkKWLabel::New();
        this->NodeLabel->SetParent(this->MainFrame);
  this->NodeLabel->Create();
  this->NodeLabel->SetText("Node");
  this->GetApplication()->Script(
          "grid %s -row 1 -column 0 -sticky ne -padx 2 -pady 2",
          this->NodeLabel->GetWidgetName());
        
        if(!this->ElementLabel)
          this->ElementLabel = vtkKWLabel::New();
        this->ElementLabel->SetParent(this->MainFrame);
  this->ElementLabel->Create();
  this->ElementLabel->SetText("Element");
  this->GetApplication()->Script(
          "grid %s -row 2 -column 0 -sticky ne -padx 2 -pady 2",
          this->ElementLabel->GetWidgetName());
          
                  
  

  if(!this->NodeNumberEntry)
          this->NodeNumberEntry = vtkKWEntry::New();
  this->NodeNumberEntry->SetParent(this->MainFrame);
  this->NodeNumberEntry->Create();
  this->NodeNumberEntry->SetWidth(8); 
 
  this->NodeNumberEntry->SetValueAsInt( 1 );
  this->NodeNumberEntry->SetRestrictValueToInteger();
  this->GetApplication()->Script(
          "grid %s -row 1 -column 1 -sticky ne -padx 2 -pady 2", 
          this->NodeNumberEntry->GetWidgetName());

  if(!this->NodeSetNameEntry)
          this->NodeSetNameEntry = vtkKWEntry::New();
  this->NodeSetNameEntry->SetParent(this->MainFrame);
  this->NodeSetNameEntry->Create();
 
  this->GetApplication()->Script(
          "grid %s -row 1 -column 2 -sticky nswe -padx 2 -pady 2", 
          this->NodeSetNameEntry->GetWidgetName());
          
  if(!this->ElementNumberEntry)
          this->ElementNumberEntry = vtkKWEntry::New();

  this->ElementNumberEntry->SetParent(this->MainFrame);
  this->ElementNumberEntry->Create();
  this->ElementNumberEntry->SetWidth(8);
 
  this->ElementNumberEntry->SetValueAsInt(1);
  this->ElementNumberEntry->SetRestrictValueToInteger();
  this->GetApplication()->Script(
          "grid %s -row 2 -column 1 -sticky ne -padx 2 -pady 2", 
          this->ElementNumberEntry->GetWidgetName());

  if(!this->ElementSetNameEntry)
          this->ElementSetNameEntry = vtkKWEntry::New();

  this->ElementSetNameEntry->SetParent(this->MainFrame);
  this->ElementSetNameEntry->Create();
  this->GetApplication()->Script(
          "grid %s -row 2 -column 2 -sticky nswe -padx 2 -pady 2", 
          this->ElementSetNameEntry->GetWidgetName());
}
//----------------------------------------------------------------------------
void vtkKWMimxNodeElementNumbersGroup::Update()
{
        this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxNodeElementNumbersGroup::UpdateEnableState()
{
        this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
void vtkKWMimxNodeElementNumbersGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
