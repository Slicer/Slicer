/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxEditNodeSetNumbersGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.6.4.2 $

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

#include "vtkKWMimxEditNodeSetNumbersGroup.h"
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
vtkStandardNewMacro(vtkKWMimxEditNodeSetNumbersGroup);
vtkCxxRevisionMacro(vtkKWMimxEditNodeSetNumbersGroup, "$Revision: 1.6.4.2 $");

//----------------------------------------------------------------------------
vtkKWMimxEditNodeSetNumbersGroup::vtkKWMimxEditNodeSetNumbersGroup()
{
  this->ObjectListComboBox = NULL;
  this->NodeSetComboBox = NULL;
  this->StartingNodeNumberEntry = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxEditNodeSetNumbersGroup::~vtkKWMimxEditNodeSetNumbersGroup()
{
  if(this->ObjectListComboBox)
     this->ObjectListComboBox->Delete();
  if(this->NodeSetComboBox)
          this->NodeSetComboBox->Delete();
  if(this->StartingNodeNumberEntry)
          this->StartingNodeNumberEntry->Delete();
}
//----------------------------------------------------------------------------
void vtkKWMimxEditNodeSetNumbersGroup::CreateWidget()
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
  //this->MainFrame->SetLabelText("Edit Node Numbers");

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 0 -fill x", 
    this->MainFrame->GetWidgetName());

  ObjectListComboBox->SetParent(this->MainFrame);
  ObjectListComboBox->Create();
  ObjectListComboBox->SetLabelText("Mesh : ");
  ObjectListComboBox->SetLabelWidth(20);
  ObjectListComboBox->GetWidget()->ReadOnlyOn();
  ObjectListComboBox->GetWidget()->SetCommand(this, "SelectionChangedCallback");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
    ObjectListComboBox->GetWidgetName());

  // for Node set listing
  if(!this->NodeSetComboBox)    
  {
          this->NodeSetComboBox = vtkKWComboBoxWithLabel::New();
  }
  NodeSetComboBox->SetParent(this->MainFrame);
  NodeSetComboBox->Create();
  NodeSetComboBox->SetLabelText("Node Set : ");
  NodeSetComboBox->SetLabelWidth(20);
  NodeSetComboBox->GetWidget()->ReadOnlyOn();
  this->NodeSetComboBox->GetWidget()->SetCommand(this, "NodeSetChangedCallback");
  this->GetApplication()->Script(
          "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
          NodeSetComboBox->GetWidgetName());

  //Young's modulus
  if (!this->StartingNodeNumberEntry)
          this->StartingNodeNumberEntry = vtkKWEntryWithLabel::New();

  this->StartingNodeNumberEntry->SetParent(this->MainFrame);
  this->StartingNodeNumberEntry->Create();
  //this->StartingNodeNumberEntry->SetWidth(10);
  this->StartingNodeNumberEntry->SetLabelWidth(20);
  this->StartingNodeNumberEntry->SetLabelText("Starting Node #: ");
//  this->StartingNodeNumberEntry->GetWidget()->SetCommand(this, "RadiusChangeCallback");
  this->StartingNodeNumberEntry->GetWidget()->SetRestrictValueToInteger();
  this->StartingNodeNumberEntry->GetWidget()->SetValueAsInt(1);
  this->GetApplication()->Script(
          "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
          this->StartingNodeNumberEntry->GetWidgetName());

  this->ApplyButton->SetParent(this->MainFrame);
  this->ApplyButton->Create();
  this->ApplyButton->SetReliefToFlat();
  this->ApplyButton->SetImageToIcon( this->GetMimxMainWindow()->GetApplyButtonIcon() );
  //this->ApplyButton->SetText("Apply");
  this->ApplyButton->SetCommand(this, "EditNodeSetNumbersApplyCallback");
  this->GetApplication()->Script(
          "pack %s -side left -anchor nw -expand y -padx 5 -pady 6", 
          this->ApplyButton->GetWidgetName());

  this->CancelButton->SetParent(this->MainFrame);
  this->CancelButton->Create();
  this->CancelButton->SetReliefToFlat();
  this->CancelButton->SetImageToIcon( this->GetMimxMainWindow()->GetCancelButtonIcon() );
  //this->CancelButton->SetText("Cancel");
  this->CancelButton->SetCommand(this, "EditNodeSetNumbersCancelCallback");
  this->GetApplication()->Script(
    "pack %s -side right -anchor ne -expand y -padx 5 -pady 6", 
    this->CancelButton->GetWidgetName());

}
//----------------------------------------------------------------------------
void vtkKWMimxEditNodeSetNumbersGroup::Update()
{
        this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxEditNodeSetNumbersGroup::UpdateEnableState()
{
        this->UpdateObjectLists();
        this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
int vtkKWMimxEditNodeSetNumbersGroup::EditNodeSetNumbersApplyCallback()
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
        
        int startelenum = this->StartingNodeNumberEntry->GetWidget()->GetValueAsInt();

        if(startelenum < 1)
        {
                callback->ErrorMessage("Starting Node number cannot be < 1");
                return 0;
        }

        vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(
                this->FEMeshList->GetItem(combobox->GetValueIndex(name)));
  //vtkUnstructuredGrid *ugrid = meshActor->GetDataSet();
        
  const char *Nodesetname = this->NodeSetComboBox->GetWidget()->GetValue();

  if(!strcmp(Nodesetname,""))
  {
          callback->ErrorMessage("Choose valid Node set name");
          return 0;
  }

  meshActor->ChangeNodeSetNumbers(Nodesetname, startelenum);  
  
  this->GetMimxMainWindow()->SetStatusText("Assigned Node Set Numbers");
  
  return 1;
}
//----------------------------------------------------------------------------
void vtkKWMimxEditNodeSetNumbersGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxEditNodeSetNumbersGroup::EditNodeSetNumbersCancelCallback()
{
//  this->MainFrame->UnpackChildren();
  this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName());
  this->MenuGroup->SetMenuButtonsEnabled(1);
    this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(1);
}
//-----------------------------------------------------------------------------
void vtkKWMimxEditNodeSetNumbersGroup::UpdateObjectLists()
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
        if (defaultItem != -1)
  {
    ObjectListComboBox->GetWidget()->SetValue(
          this->FEMeshList->GetItem(defaultItem)->GetFileName());
  }
  */
        this->SelectionChangedCallback(ObjectListComboBox->GetWidget()->GetValue());
        
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
void vtkKWMimxEditNodeSetNumbersGroup::SelectionChangedCallback(const char *Selection)
{
        if(!strcmp(Selection,""))
        {
                return;
        }
        vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
        vtkUnstructuredGrid *ugrid = vtkMimxMeshActor::SafeDownCast(
                this->FEMeshList->GetItem(combobox->GetValueIndex(Selection)))->GetDataSet();

        // populate the Node set list
        this->NodeSetComboBox->GetWidget()->DeleteAllValues();
        int i;
        vtkStringArray *strarray = vtkStringArray::SafeDownCast(
                ugrid->GetFieldData()->GetAbstractArray("Node_Set_Names"));

        int numarrrays = strarray->GetNumberOfValues();

        for (i=0; i<numarrrays; i++)
        {
                this->NodeSetComboBox->GetWidget()->AddValue(
                        strarray->GetValue(i));
        }
        this->NodeSetComboBox->GetWidget()->SetValue( strarray->GetValue(0) );
}
//-------------------------------------------------------------------------------------
void vtkKWMimxEditNodeSetNumbersGroup::NodeSetChangedCallback(const char *vtkNotUsed(Selection))
{
        //if(!strcmp(Selection,""))
        //{
        //      return;
        //}
        //// get the femesh
        //vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
        //vtkUnstructuredGrid *ugrid = vtkMimxMeshActor::SafeDownCast(
        //      this->FEMeshList->GetItem(combobox->GetValueIndex(
        //      ObjectListComboBox->GetWidget()->GetValue())))->GetDataSet();

        //char young[256];
        //strcpy(young, Selection);
        //strcat(young, "_Constant_Youngs_Modulus");

        //char poisson[256];
        //strcpy(poisson, Selection);
        //strcat(poisson, "_Constant_Poissons_Ratio");

        //// check if the field data exists.
        //vtkFloatArray *Earray = vtkFloatArray::SafeDownCast(
        //      ugrid->GetFieldData()->GetArray(young));        

        //vtkFloatArray *Nuarray = vtkFloatArray::SafeDownCast(
        //      ugrid->GetFieldData()->GetArray(poisson));      

        //float startelenum = -1.0;
        //float poissonsratio = -1.0;
        //if(Earray)    startelenum = Earray->GetValue(0);
        //if(Nuarray)   poissonsratio = Nuarray->GetValue(0);
        //// youngs modulus and poissons ratio data exists
        //this->StartingNodeNumberEntry->GetWidget()->SetValueAsDouble(startelenum);
        //this->PoissonsRatioEntry->GetWidget()->SetValueAsDouble(poissonsratio);
}
//----------------------------------------------------------------------------------------
