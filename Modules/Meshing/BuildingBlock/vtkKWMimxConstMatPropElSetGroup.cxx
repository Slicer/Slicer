/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxConstMatPropElSetGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.35.2.2 $

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

#include "vtkKWMimxConstMatPropElSetGroup.h"

#include "vtkActor.h"
#include "vtkCellData.h"
#include "vtkDoubleArray.h"
#include "vtkFieldData.h"
#include "vtkFloatArray.h"
#include "vtkIntArray.h"
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkStringArray.h"
#include "vtkUnstructuredGrid.h"

#include "vtkKWApplication.h"
#include "vtkKWComboBox.h"
#include "vtkKWComboBoxWithLabel.h"
#include "vtkKWEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWIcon.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWPushButton.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWTkUtilities.h"


#include "vtkLinkedListWrapper.h"

#include "vtkMimxErrorCallback.h"
#include "vtkMimxMeshActor.h"
#include "vtkMimxUnstructuredGridActor.h"

#include "vtkKWMimxDefineElSetGroup.h"
#include "vtkKWMimxMainNotebook.h"
#include "vtkKWMimxMainMenuGroup.h"
#include "vtkKWMimxMainUserInterfacePanel.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkKWMimxViewPropertiesOptionGroup.h"

#include "Resources/mimxElementSetDfns.h"

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD                 1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxConstMatPropElSetGroup);
vtkCxxRevisionMacro(vtkKWMimxConstMatPropElSetGroup, "$Revision: 1.35.2.2 $");

//----------------------------------------------------------------------------
vtkKWMimxConstMatPropElSetGroup::vtkKWMimxConstMatPropElSetGroup()
{
  this->ObjectListComboBox = NULL;
  this->ElementSetComboBox = NULL;
  this->YoungsModulusEntry = NULL;
  this->PoissonsRatioEntry = NULL;
  this->ComponentFrame = NULL;
  this->DefineElSetButton = NULL;
  this->DefineElementSetDialog = NULL;
  this->ViewOptionsButton = NULL;
  this->ViewOptionsGroup = NULL;
  strcpy(this->elementSetSelectionPrevious,"");
}

//----------------------------------------------------------------------------
vtkKWMimxConstMatPropElSetGroup::~vtkKWMimxConstMatPropElSetGroup()
{
  if(this->ObjectListComboBox)
    this->ObjectListComboBox->Delete();
  if(this->ElementSetComboBox)
    this->ElementSetComboBox->Delete();
  if(this->YoungsModulusEntry)
    this->YoungsModulusEntry->Delete();
  if(this->PoissonsRatioEntry)
    this->PoissonsRatioEntry->Delete();
  if (this->ComponentFrame)
    this->ComponentFrame->Delete();
  if (this->DefineElSetButton)
    this->DefineElSetButton->Delete();
  if (this->DefineElementSetDialog)
    this->DefineElementSetDialog->Delete();
  if(this->ViewOptionsButton)
    this->ViewOptionsButton->Delete();
  if(this->ViewOptionsGroup)
    this->ViewOptionsGroup->Delete();
}
//----------------------------------------------------------------------------
void vtkKWMimxConstMatPropElSetGroup::CreateWidget()
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
  //this->MainFrame->SetLabelText("Constant Material Properties");

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 0 -fill x", 
    this->MainFrame->GetWidgetName());

  if ( !this->ComponentFrame )
    this->ComponentFrame = vtkKWFrameWithLabel::New();
  this->ComponentFrame->SetParent( this->MainFrame );
  this->ComponentFrame->Create();
  this->ComponentFrame->SetLabelText("Mesh");
  this->ComponentFrame->CollapseFrame();
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 6 -fill x", 
    this->ComponentFrame->GetWidgetName());
    
  ObjectListComboBox->SetParent(this->ComponentFrame->GetFrame());
  ObjectListComboBox->Create();
  ObjectListComboBox->SetLabelText("Mesh : ");
  ObjectListComboBox->SetLabelWidth(15);
  ObjectListComboBox->GetWidget()->ReadOnlyOn();
  ObjectListComboBox->GetWidget()->SetCommand(this, "SelectionChangedCallback");
  ObjectListComboBox->GetWidget()->SetBalloonHelpString(
    "List of Finite element meshes containing material property information");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
    ObjectListComboBox->GetWidgetName());

  vtkKWIcon *defineElSetIcon = vtkKWIcon::New();
  defineElSetIcon->SetImage(    image_mimxElementSetDfns, 
                                image_mimxElementSetDfns_width, 
                                image_mimxElementSetDfns_height, 
                                image_mimxElementSetDfns_pixel_size); 
                          
  if ( !this->DefineElSetButton )
    this->DefineElSetButton = vtkKWPushButton::New();
  this->DefineElSetButton->SetParent(this->MainFrame);
  this->DefineElSetButton->Create();
  this->DefineElSetButton->SetCommand(this, "DefineElementSetCallback");
  this->DefineElSetButton->SetImageToIcon( defineElSetIcon );
  this->DefineElSetButton->SetReliefToFlat( );
  this->GetApplication()->Script(
    "pack %s -side top -anchor n -padx 2 -pady 6", 
    this->DefineElSetButton->GetWidgetName());
  defineElSetIcon->Delete();

  // for element set listing
  if ( !this->ElementSetComboBox )      
    this->ElementSetComboBox = vtkKWComboBoxWithLabel::New();
  ElementSetComboBox->SetParent(this->MainFrame);
  ElementSetComboBox->Create();
  ElementSetComboBox->SetLabelText("Element Set : ");
  ElementSetComboBox->SetLabelWidth(15);
  ElementSetComboBox->GetWidget()->ReadOnlyOn();
  this->ElementSetComboBox->GetWidget()->SetCommand(this, "ElementSetChangedCallback");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
    ElementSetComboBox->GetWidgetName());

  //Young's modulus
  if (!this->YoungsModulusEntry)
    this->YoungsModulusEntry = vtkKWEntryWithLabel::New();
  this->YoungsModulusEntry->SetParent(this->MainFrame);
  this->YoungsModulusEntry->Create();
  this->YoungsModulusEntry->SetLabelWidth(15);
  this->YoungsModulusEntry->SetLabelText("Modulus : ");
  this->YoungsModulusEntry->GetWidget()->SetRestrictValueToDouble();
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
    this->YoungsModulusEntry->GetWidgetName());

  // Poisson's ratio
  if (!this->PoissonsRatioEntry)
    this->PoissonsRatioEntry = vtkKWEntryWithLabel::New();
  this->PoissonsRatioEntry->SetParent(this->MainFrame);
  this->PoissonsRatioEntry->Create();
  this->PoissonsRatioEntry->SetLabelWidth(15);
  this->PoissonsRatioEntry->SetLabelText("Poisson's Ratio : ");
  this->PoissonsRatioEntry->GetWidget()->SetRestrictValueToDouble();
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
    this->PoissonsRatioEntry->GetWidgetName());

  this->ApplyButton->SetParent(this->MainFrame);
  this->ApplyButton->Create();
  this->ApplyButton->SetReliefToFlat();
  this->ApplyButton->SetImageToIcon( this->GetMimxMainWindow()->GetApplyButtonIcon() );
  //this->ApplyButton->SetText("Apply");
  this->ApplyButton->SetCommand(this, "ConstMatPropElSetApplyCallback");
  this->GetApplication()->Script(
    "pack %s -side left -anchor nw -expand y -padx 5 -pady 2", 
    this->ApplyButton->GetWidgetName());

  this->CancelButton->SetParent(this->MainFrame);
  this->CancelButton->Create();
  this->CancelButton->SetReliefToFlat();
  this->CancelButton->SetImageToIcon( this->GetMimxMainWindow()->GetCancelButtonIcon() );
  //this->CancelButton->SetText("Cancel");
  this->CancelButton->SetCommand(this, "ConstMatPropElSetCancelCallback");
  this->GetApplication()->Script(
    "pack %s -side right -anchor ne -expand y -padx 5 -pady 2", 
    this->CancelButton->GetWidgetName());
}
//----------------------------------------------------------------------------
void vtkKWMimxConstMatPropElSetGroup::Update()
{
  this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxConstMatPropElSetGroup::UpdateEnableState()
{
  this->UpdateObjectLists();
  this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
int vtkKWMimxConstMatPropElSetGroup::ConstMatPropElSetApplyCallback()
{
  vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
  if(!strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
    {
    callback->ErrorMessage("FE Mesh selection required");
    return 0;
    }

  vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
  const char *name = combobox->GetValue();
  strcpy(this->meshName, name);
  
  int num = combobox->GetValueIndex(name);
  if(num < 0 || num > combobox->GetNumberOfValues()-1)
    {
    callback->ErrorMessage("Choose valid FE Mesh");
    combobox->SetValue("");
    return 0;
    }

  float youngsModulus = this->YoungsModulusEntry->GetWidget()->GetValueAsDouble();
  float poissonsratio = this->PoissonsRatioEntry->GetWidget()->GetValueAsDouble();

  if(youngsModulus < 0)
    {
    callback->ErrorMessage("Young's Modulus cannot be < 0");
    return 0;
    }

  if(poissonsratio < -1.0)
    {
    callback->ErrorMessage("poissons ratio value should be >= -1.0");
    return 0;
    }
        
  vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(
    this->FEMeshList->GetItem(combobox->GetValueIndex(name)));
  vtkUnstructuredGrid *ugrid = vtkMimxMeshActor::SafeDownCast(
    this->FEMeshList->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
        
  const char *elementsetname = this->ElementSetComboBox->GetWidget()->GetValue();
  strcpy(this->elementSetName, elementsetname);
  
  if(!strcmp(elementsetname,""))
    {
    callback->ErrorMessage("Choose valid element set name");
    return 0;
    }
  
  /* If both Fields are empty then clear out the values */
  std::string youngStr = this->YoungsModulusEntry->GetWidget()->GetValue();
  std::string poissonStr = this->PoissonsRatioEntry->GetWidget()->GetValue();
  if ((youngStr == "") && (poissonStr == ""))
    {
    std::string propName = this->elementSetName;
    propName += "_Constant_Youngs_Modulus";
    ugrid->GetFieldData()->RemoveArray( propName.c_str() );

    vtkIntArray *cellArray = vtkIntArray::SafeDownCast(ugrid->GetCellData()->GetArray(this->elementSetName));
    vtkDoubleArray *youngsmodulus = vtkDoubleArray::SafeDownCast(ugrid->GetCellData()->GetArray("Youngs_Modulus"));       
    if (cellArray && youngsmodulus)
      {
      int numCells = ugrid->GetNumberOfCells();
      for (int i=0; i< numCells; i++)
        {
        if (cellArray->GetValue(i))
          {
          youngsmodulus->SetValue(i, -9999);
          }
        }
      }

    propName = this->elementSetName;
    propName += "_Constant_Poissons_Ratio";
    ugrid->GetFieldData()->RemoveArray( propName.c_str() );

    this->YoungsModulusEntry->SetEnabled( 0 );
    this->PoissonsRatioEntry->SetEnabled( 0 );
    this->GetMimxMainWindow()->SetStatusText("Cleared user defined material properties");

    return 1;
    }

  std::string checkPropName;
  checkPropName = this->elementSetName;
  checkPropName += "_Image_Based_Material_Property";
  
  vtkDoubleArray *matarray = vtkDoubleArray::SafeDownCast( ugrid->GetCellData()->GetArray(checkPropName.c_str()) );
  if( matarray )
    {
    vtkKWMessageDialog *Dialog = vtkKWMessageDialog::New();
    Dialog->SetStyleToYesNo();
    Dialog->SetApplication(this->GetApplication());
    Dialog->Create();
    Dialog->SetTitle("Your Attention Please!");
    Dialog->SetText("An image-based material property has already been assigned to this element set. Would you like to overwrite?");
    Dialog->Invoke();
    if(Dialog->GetStatus() == vtkKWMessageDialog::StatusCanceled)
      {
      Dialog->Delete();
      return 1;
      }
    Dialog->Delete();
    ugrid->GetCellData()->RemoveArray( checkPropName.c_str() );
    }

  meshActor->StoreConstantMaterialProperty(elementsetname, youngsModulus);
  meshActor->StoreConstantPoissonsRatio(elementsetname, poissonsratio);
  this->YoungsModulusEntry->SetEnabled( 0 );
  this->PoissonsRatioEntry->SetEnabled( 0 );
  this->GetMimxMainWindow()->SetStatusText("Assigned user defined material properties");          

  return 1;
}
//----------------------------------------------------------------------------
void vtkKWMimxConstMatPropElSetGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxConstMatPropElSetGroup::ConstMatPropElSetCancelCallback()
{
  this->CancelStatus = 1;
  if(this->ViewOptionsGroup)
    {
    this->ViewOptionsGroup->DeselectAllButtons();
    this->ViewOptionsGroup->Withdraw();
    }
  this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName());
  this->MenuGroup->SetMenuButtonsEnabled(1);
  this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(1);
  this->CancelStatus = 0;
  strcpy(this->objectSelectionPrevious, "");
  strcpy(this->elementSetSelectionPrevious, "");
}
//-----------------------------------------------------------------------------
void vtkKWMimxConstMatPropElSetGroup::UpdateObjectLists()
{
  this->UpdateMeshComboBox( this->ObjectListComboBox->GetWidget() );
        
  /*
        this->ObjectListComboBox->GetWidget()->DeleteAllValues();
        
        int defaultItem = -1;
        for (int i = 0; i < this->FEMeshList->GetNumberOfItems(); i++)
        {
                vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(this->FEMeshList->GetItem(i));  
                        ObjectListComboBox->GetWidget()->AddValue(
                                this->FEMeshList->GetItem(i)->GetFileName());
                  
                        bool viewedItem = meshActor->GetMeshVisibility();
            
                        if ((viewedItem) && (defaultItem == -1))
                        {
                          defaultItem = i;
                        }                               

        //}
        / *
        else
        {
          std::cout << "Failed to find Youngs_Modulus Array" << std::endl;
        }
        * /

        }
        if ((defaultItem == -1) && (this->FEMeshList->GetNumberOfItems() > 0))
    defaultItem = this->FEMeshList->GetNumberOfItems()-1;
    
        if (defaultItem != -1)
  {
    ObjectListComboBox->GetWidget()->SetValue(
          this->FEMeshList->GetItem(defaultItem)->GetFileName());
  }
        else
        {
    ObjectListComboBox->GetWidget()->SetValue("");
        }
        */
  this->SelectionChangedCallback(ObjectListComboBox->GetWidget()->GetValue());
}
//--------------------------------------------------------------------------------
void vtkKWMimxConstMatPropElSetGroup::SelectionChangedCallback(const char *Selection)
{
  if(this->CancelStatus)  return;
  if(!strcmp(Selection,""))
    {
    this->DefineElSetButton->SetStateToDisabled();
    return;
    }
  if(!strcmp(this->objectSelectionPrevious, Selection))
    {
    return;
    }
  //this->RemovePreviousSelectionDisplay();
  strcpy(this->objectSelectionPrevious,Selection);

  vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
  vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(
    this->FEMeshList->GetItem(combobox->GetValueIndex(Selection)));
  vtkUnstructuredGrid *ugrid = meshActor->GetDataSet();

  // populate the element set list
  this->ElementSetComboBox->GetWidget()->DeleteAllValues();
  int i;
  vtkStringArray *strarray = vtkStringArray::SafeDownCast(
    ugrid->GetFieldData()->GetAbstractArray("Element_Set_Names"));

  int numarrrays = strarray->GetNumberOfValues();

  for (i=0; i<numarrrays; i++)
    {
    this->ElementSetComboBox->GetWidget()->AddValue(
      strarray->GetValue(i));
    }
  this->ElementSetComboBox->GetWidget()->SetValue( strarray->GetValue(0) );
  strcpy(this->elementSetSelectionPrevious, "");
  if(this->ViewOptionsGroup)
    {
    this->ViewOptionsGroup->SetMeshActor(meshActor);
    }
  if(this->DefineElementSetDialog)
    {
    this->DefineElementSetDialog->SetMeshActor(meshActor);
    this->DefineElementSetDialog->ResetState();
    }
  this->ElementSetChangedCallback(this->ElementSetComboBox->GetWidget()->GetValue());
}
//-------------------------------------------------------------------------------------
void vtkKWMimxConstMatPropElSetGroup::ElementSetChangedCallback(const char *Selection)
{
  if(this->CancelStatus)  return;
  vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
  if(!strcmp(Selection,""))
    {
    callback->ErrorMessage("Element Set Selection Required");
    return;
    }

  this->DefineElSetButton->SetStateToNormal();
  
  // get the femesh
  vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
//      const char *elSetName = this->ElementSetComboBox->GetWidget()->GetValue();
  vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(
    this->FEMeshList->GetItem(combobox->GetValueIndex(
                                ObjectListComboBox->GetWidget()->GetValue())));
  vtkUnstructuredGrid *ugrid = meshActor->GetDataSet();
        
//      meshActor->HideMesh();
  char young[256];
  strcpy(young, Selection);
  strcat(young, "_Constant_Youngs_Modulus");

  char poisson[256];
  strcpy(poisson, Selection);
  strcat(poisson, "_Constant_Poissons_Ratio");

  // check if the field data exists.
  vtkDoubleArray *Earray = vtkDoubleArray::SafeDownCast(
    ugrid->GetFieldData()->GetArray(young));        

  vtkFloatArray *Nuarray = vtkFloatArray::SafeDownCast(
    ugrid->GetFieldData()->GetArray(poisson));      

  float youngsmodulus;
  float poissonsratio;
  if(Earray)
    {
    youngsmodulus = Earray->GetValue(0);
    this->YoungsModulusEntry->GetWidget()->SetValueAsDouble(youngsmodulus);
    }
  else
    {
    this->YoungsModulusEntry->GetWidget()->SetValue("");
    }
  if(Nuarray)
    {
    poissonsratio = Nuarray->GetValue(0);
    this->PoissonsRatioEntry->GetWidget()->SetValueAsDouble(poissonsratio);
    }
  else{
  this->PoissonsRatioEntry->GetWidget()->SetValue("");
  }
  if(this->ViewOptionsGroup)
    {
    meshActor->ShowHideAllElementSets(0);
    this->ViewOptionsGroup->SetElementSetName(Selection);
    this->ViewOptionsGroup->Update();
    }
  this->GetMimxMainWindow()->GetRenderWidget()->Render();
        
  this->YoungsModulusEntry->SetEnabled( 1 );
  this->PoissonsRatioEntry->SetEnabled( 1 );
}
//----------------------------------------------------------------------------
void vtkKWMimxConstMatPropElSetGroup::DefineElementSetCallback()
{
  vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
        
  if(!strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
    {
    callback->ErrorMessage("Mesh must be selected");
    return;
    }

  vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
  const char *name = combobox->GetValue();
  
  int num = combobox->GetValueIndex(name);
  if(num < 0 || num > combobox->GetNumberOfValues()-1)
    {
    callback->ErrorMessage("Invalid Mesh was selected");
    combobox->SetValue("");
    return;
    }
  vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(
    this->FEMeshList->GetItem(combobox->GetValueIndex(name)));
                
  if (!this->DefineElementSetDialog)
    {
    this->DefineElementSetDialog = vtkKWMimxDefineElSetGroup::New();
    this->DefineElementSetDialog->SetApplication( this->GetApplication() );
    this->DefineElementSetDialog->SetMimxMainWindow( this->GetMimxMainWindow() );
    this->DefineElementSetDialog->Create();
    }
  this->DefineElementSetDialog->SetMeshActor( meshActor );
  this->DefineElementSetDialog->SetElementSetCombobox(this->ElementSetComboBox->GetWidget());
  this->DefineElementSetDialog->Display();
}
//--------------------------------------------------------------------------------------------
