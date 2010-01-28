/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxEditElementSetNumbersGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.9.4.2 $

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

#include "vtkKWMimxEditElementSetNumbersGroup.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkKWMimxMainMenuGroup.h"
#include "vtkMimxErrorCallback.h"
#include "vtkKWMimxMainNotebook.h"

#include "vtkLinkedListWrapper.h"

#include "vtkActor.h"
#include "vtkMimxUnstructuredGridActor.h"
#include "vtkMimxMeshActor.h"
#include "vtkUnstructuredGrid.h"

#include "vtkKWApplication.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWEntry.h"
#include "vtkKWEvent.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWOptions.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWTkUtilities.h"
#include "vtkObjectFactory.h"
#include "vtkKWComboBoxWithLabel.h"
#include "vtkKWComboBox.h"
#include "vtkKWPushButton.h"
#include "vtkKWMimxMainUserInterfacePanel.h"
#include "vtkFieldData.h"
#include "vtkFloatArray.h"
#include "vtkCellData.h"
#include "vtkPointData.h"
#include "vtkIntArray.h"
#include "vtkStringArray.h"
#include "vtkDoubleArray.h"
#include "vtkKWMessageDialog.h"

#include "vtkRenderer.h"

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD                 1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxEditElementSetNumbersGroup);
vtkCxxRevisionMacro(vtkKWMimxEditElementSetNumbersGroup, "$Revision: 1.9.4.2 $");

//----------------------------------------------------------------------------
vtkKWMimxEditElementSetNumbersGroup::vtkKWMimxEditElementSetNumbersGroup()
{
  this->ComponentFrame = NULL;
  this->ObjectListComboBox = NULL;
  this->ElementSetComboBox = NULL;
  this->StartingElementNumberEntry = NULL;
  this->StartingNodeNumberEntry = NULL;
  this->NodeSetComboBox = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxEditElementSetNumbersGroup::~vtkKWMimxEditElementSetNumbersGroup()
{
  if(this->ComponentFrame)
     this->ComponentFrame->Delete();
  if(this->ObjectListComboBox)
     this->ObjectListComboBox->Delete();
  if(this->ElementSetComboBox)
          this->ElementSetComboBox->Delete();
  if(this->StartingElementNumberEntry)
          this->StartingElementNumberEntry->Delete();
        if(this->NodeSetComboBox)
          this->NodeSetComboBox->Delete();
  if(this->StartingNodeNumberEntry)
          this->StartingNodeNumberEntry->Delete();
}
//----------------------------------------------------------------------------
void vtkKWMimxEditElementSetNumbersGroup::CreateWidget()
{
  if(this->IsCreated())
  {
    vtkErrorMacro("class already created");
    return;
  }

  this->Superclass::CreateWidget();
  this->MainFrame->SetParent(this->GetParent());
  this->MainFrame->Create();
  //this->MainFrame->SetLabelText("Edit Node/Element Numbers");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 0 -fill x", 
    this->MainFrame->GetWidgetName());

  if (!this->ComponentFrame)    
     this->ComponentFrame = vtkKWFrameWithLabel::New();
  this->ComponentFrame->SetParent( this->MainFrame );
  this->ComponentFrame->Create();
  this->ComponentFrame->SetLabelText("Mesh");
  this->ComponentFrame->CollapseFrame( );
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 2 -fill x", 
    this->ComponentFrame->GetWidgetName());
    
  if (!this->ObjectListComboBox)        
     this->ObjectListComboBox = vtkKWComboBoxWithLabel::New();
  ObjectListComboBox->SetParent(this->ComponentFrame->GetFrame());
  ObjectListComboBox->Create();
  ObjectListComboBox->SetLabelText("Mesh : ");
  ObjectListComboBox->SetLabelWidth(20);
  ObjectListComboBox->GetWidget()->ReadOnlyOn();
  ObjectListComboBox->GetWidget()->SetCommand(this, "SelectionChangedCallback");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
    ObjectListComboBox->GetWidgetName());

  // Node Set Entries
  if(!this->NodeSetComboBox)    
          this->NodeSetComboBox = vtkKWComboBoxWithLabel::New();
  this->NodeSetComboBox->SetParent( this->MainFrame );
  this->NodeSetComboBox->Create();
  this->NodeSetComboBox->SetLabelText("Node Set : ");
  this->NodeSetComboBox->SetLabelWidth(20);
  this->NodeSetComboBox->GetWidget()->ReadOnlyOn();
  this->NodeSetComboBox->GetWidget()->SetCommand(this, "NodeSetChangedCallback");
  this->GetApplication()->Script(
          "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
          NodeSetComboBox->GetWidgetName());

  if (!this->StartingNodeNumberEntry)
          this->StartingNodeNumberEntry = vtkKWEntryWithLabel::New();
  this->StartingNodeNumberEntry->SetParent( this->MainFrame );
  this->StartingNodeNumberEntry->Create();
  this->StartingNodeNumberEntry->SetLabelWidth(30);
  this->StartingNodeNumberEntry->SetLabelText("Starting Node Number: ");
  this->StartingNodeNumberEntry->GetWidget()->SetRestrictValueToInteger();
  this->StartingNodeNumberEntry->GetWidget()->SetValueAsInt(1);
  this->GetApplication()->Script(
          "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
          this->StartingNodeNumberEntry->GetWidgetName());
          
  // Element Set Entries
  if(!this->ElementSetComboBox) 
          this->ElementSetComboBox = vtkKWComboBoxWithLabel::New();
  this->ElementSetComboBox->SetParent( this->MainFrame );
  this->ElementSetComboBox->Create();
  this->ElementSetComboBox->SetLabelText("Element Set : ");
  this->ElementSetComboBox->SetLabelWidth(20);
  this->ElementSetComboBox->GetWidget()->ReadOnlyOn();
  this->ElementSetComboBox->GetWidget()->SetCommand(this, "ElementSetChangedCallback");
  this->GetApplication()->Script(
          "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
          ElementSetComboBox->GetWidgetName());

  if (!this->StartingElementNumberEntry)
          this->StartingElementNumberEntry = vtkKWEntryWithLabel::New();
  this->StartingElementNumberEntry->SetParent( this->MainFrame );
  this->StartingElementNumberEntry->Create();
  this->StartingElementNumberEntry->SetLabelWidth(30);
  this->StartingElementNumberEntry->SetLabelText("Starting Element Number: ");
  this->StartingElementNumberEntry->GetWidget()->SetRestrictValueToInteger();
  this->StartingElementNumberEntry->GetWidget()->SetValueAsInt(1);
  this->GetApplication()->Script(
          "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
          this->StartingElementNumberEntry->GetWidgetName());


  
          

  this->ApplyButton->SetParent( this->MainFrame );
  this->ApplyButton->Create();
  this->ApplyButton->SetReliefToFlat();
  this->ApplyButton->SetImageToIcon( this->GetMimxMainWindow()->GetApplyButtonIcon() );
  this->ApplyButton->SetCommand(this, "EditElementSetNumbersApplyCallback");
  this->GetApplication()->Script(
          "pack %s -side left -anchor nw -expand y -padx 5 -pady 6", 
          this->ApplyButton->GetWidgetName());

  this->CancelButton->SetParent( this->MainFrame );
  this->CancelButton->Create();
  this->CancelButton->SetReliefToFlat();
  this->CancelButton->SetImageToIcon( this->GetMimxMainWindow()->GetCancelButtonIcon() );
  this->CancelButton->SetCommand(this, "EditElementSetNumbersCancelCallback");
  this->GetApplication()->Script(
    "pack %s -side right -anchor ne -expand y -padx 5 -pady 6", 
    this->CancelButton->GetWidgetName());

}
//----------------------------------------------------------------------------
void vtkKWMimxEditElementSetNumbersGroup::Update()
{
        this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxEditElementSetNumbersGroup::UpdateEnableState()
{
        this->UpdateObjectLists();
        this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
int vtkKWMimxEditElementSetNumbersGroup::EditElementSetNumbersApplyCallback()
{
        vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
        if(!strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
        {
                callback->ErrorMessage("FE Mesh selection required");
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
        
        int startelenum = this->StartingElementNumberEntry->GetWidget()->GetValueAsInt();
        if (startelenum < 1)
        {
                callback->ErrorMessage("Starting element number cannot be less than 1");
                return 0;
        }
  
        int startNodeNumber = this->StartingNodeNumberEntry->GetWidget()->GetValueAsInt();
        if (startNodeNumber < 1)
        {
                callback->ErrorMessage("Starting Node number cannot be less than 1");
                return 0;
        }

        vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(
                this->FEMeshList->GetItem(combobox->GetValueIndex(name)));
  //vtkUnstructuredGrid *ugrid = meshActor->GetDataSet();
        
  
  const char *elementsetname = this->ElementSetComboBox->GetWidget()->GetValue();

  if (strcmp(elementsetname,"") != 0)
  {
          meshActor->ChangeElementSetNumbers(elementsetname, startelenum); 
  }
  
  const char *nodeSetName = this->NodeSetComboBox->GetWidget()->GetValue();
  if (strcmp(nodeSetName,"") != 0)
  {
          meshActor->ChangeNodeSetNumbers(nodeSetName, startNodeNumber); 
  }
  
  this->GetMimxMainWindow()->SetStatusText("Renumbered Node/Element Set Numbers"); 
  
  return 1;
}
//----------------------------------------------------------------------------
void vtkKWMimxEditElementSetNumbersGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxEditElementSetNumbersGroup::EditElementSetNumbersCancelCallback()
{
//  this->MainFrame->UnpackChildren();
  this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName());
  this->MenuGroup->SetMenuButtonsEnabled(1);
    this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(1);
}
//-----------------------------------------------------------------------------
void vtkKWMimxEditElementSetNumbersGroup::UpdateObjectLists()
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
        this->SelectionChangedCallback(ObjectListComboBox->GetWidget()->GetValue());
        
        /* Set the Default Element Number */
        int maxElementNumber = 0;
        
        for (int i = 0; i < this->FEMeshList->GetNumberOfItems(); i++)
  {
    vtkUnstructuredGrid *ugrid = vtkMimxMeshActor::SafeDownCast(this->FEMeshList
       ->GetItem(i))->GetDataSet();

    vtkIntArray *elementArray = vtkIntArray::SafeDownCast(ugrid->GetCellData()->GetArray("Element_Numbers"));
    if ( elementArray )
    {
      int maxIndex = elementArray->GetNumberOfTuples();
      int currentValue = 0;
      if (maxIndex > 0) currentValue = elementArray->GetValue( maxIndex -1 );
      if (currentValue > maxElementNumber ) maxElementNumber = currentValue;
    }
  }
  maxElementNumber++;
  this->StartingElementNumberEntry->GetWidget()->SetValueAsInt( maxElementNumber );
  
  /* Set the Default Node Number */
        int maxNodeNumber = 0;
        
        for (int i = 0; i < this->FEMeshList->GetNumberOfItems(); i++)
  {
    vtkUnstructuredGrid *ugrid = vtkMimxMeshActor::SafeDownCast(this->FEMeshList
       ->GetItem(i))->GetDataSet();

    vtkIntArray *nodeArray = vtkIntArray::SafeDownCast(ugrid->GetPointData()->GetArray("Node_Numbers"));
    if ( nodeArray )
    {
      int maxIndex = nodeArray->GetNumberOfTuples();
      int currentValue = 0;
      if (maxIndex > 0) currentValue = nodeArray->GetValue( maxIndex -1 );
      if (currentValue > maxNodeNumber ) maxNodeNumber = currentValue;
    }
  }
  maxNodeNumber++;
  
  this->StartingNodeNumberEntry->GetWidget()->SetValueAsInt( maxNodeNumber );
}
//--------------------------------------------------------------------------------
void vtkKWMimxEditElementSetNumbersGroup::SelectionChangedCallback(const char *Selection)
{
        if (!strcmp(Selection,""))
        {
                return;
        }
        vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
        vtkUnstructuredGrid *ugrid = vtkMimxMeshActor::SafeDownCast(
                this->FEMeshList->GetItem(combobox->GetValueIndex(Selection)))->GetDataSet();

        // populate the element set list
        this->ElementSetComboBox->GetWidget()->DeleteAllValues();
        vtkStringArray *elementArray = vtkStringArray::SafeDownCast(
                ugrid->GetFieldData()->GetAbstractArray("Element_Set_Names"));

  if (elementArray != NULL)
  {
        int numarrrays = elementArray->GetNumberOfValues();

        for (int i=0; i<numarrrays; i++)
        {
                this->ElementSetComboBox->GetWidget()->AddValue(elementArray->GetValue(i));
        }
        //this->ElementSetComboBox->GetWidget()->SetValue( elementArray->GetValue(0) );
        }
        // populate the Node set list
        this->NodeSetComboBox->GetWidget()->DeleteAllValues();
        vtkStringArray *nodeArray = vtkStringArray::SafeDownCast(
                ugrid->GetFieldData()->GetAbstractArray("Node_Set_Names"));

  if (nodeArray != NULL)
  {
        int numarrrays = nodeArray->GetNumberOfValues();
        for (int i=0; i<numarrrays; i++)
        {
                this->NodeSetComboBox->GetWidget()->AddValue(nodeArray->GetValue(i));
        }
        //this->NodeSetComboBox->GetWidget()->SetValue( nodeArray->GetValue(0) );
        }
        
}
//-------------------------------------------------------------------------------------
void vtkKWMimxEditElementSetNumbersGroup::ElementSetChangedCallback(const char *vtkNotUsed(Selection))
{

}
//-------------------------------------------------------------------------------------
void vtkKWMimxEditElementSetNumbersGroup::NodeSetChangedCallback(const char *vtkNotUsed(Selection))
{

}
//----------------------------------------------------------------------------------------
