/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxReBinFEMeshMaterialPropertiesFromImageGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.17.4.2 $

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

#include "vtkKWMimxReBinFEMeshMaterialPropertiesFromImageGroup.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkMimxErrorCallback.h"
#include "vtkKWMimxMainNotebook.h"

#include "vtkLinkedListWrapper.h"

#include "vtkActor.h"
#include "vtkCellData.h"
#include "vtkIntArray.h"
#include "vtkMimxSurfacePolyDataActor.h"
#include "vtkPolyData.h"
#include "vtkProperty.h"
#include "vtkMimxUnstructuredGridActor.h"
#include "vtkUnstructuredGrid.h"
#include "vtkMimxImageActor.h"
#include "vtkMimxRebinMaterialProperty.h"
#include "vtkRenderer.h"
#include "vtkMimxMeshActor.h"

#include "vtkKWApplication.h"
#include "vtkKWFileBrowserDialog.h"
#include "vtkKWEvent.h"
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
#include "vtkKWMimxMainUserInterfacePanel.h"
#include "vtkStringArray.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkDoubleArray.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWCheckButton.h"


#include "itkImage.h"

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD                 1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxReBinFEMeshMaterialPropertiesFromImageGroup);
vtkCxxRevisionMacro(vtkKWMimxReBinFEMeshMaterialPropertiesFromImageGroup, "$Revision: 1.17.4.2 $");

//----------------------------------------------------------------------------
vtkKWMimxReBinFEMeshMaterialPropertiesFromImageGroup::vtkKWMimxReBinFEMeshMaterialPropertiesFromImageGroup()
{
  this->FEMeshListComboBox = NULL;
  this->ElementSetComboBox = NULL;
  this->NumOfBinsEntry = NULL;
  this->MinimumRebinEntry = NULL;
  this->MaximumRebinEntry = NULL;
  this->MinimumValueFrame = NULL;
  this->MaximumValueFrame = NULL;
  this->MinimumValueCheckButton = NULL;
  this->MaximumValueCheckButton = NULL;
  
  this->ViewFrame = NULL;
  this->ViewPropertyButton = NULL;
  this->ViewLegendButton = NULL;
  this->ClippingPlaneMenuButton = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxReBinFEMeshMaterialPropertiesFromImageGroup::~vtkKWMimxReBinFEMeshMaterialPropertiesFromImageGroup()
{
  if(this->FEMeshListComboBox)
     this->FEMeshListComboBox->Delete();
  if(this->ElementSetComboBox)
          this->ElementSetComboBox->Delete();
  if(this->NumOfBinsEntry)
          this->NumOfBinsEntry->Delete();
  if(this->MinimumRebinEntry)
          this->MinimumRebinEntry->Delete();
  if(this->MaximumRebinEntry)
          this->MaximumRebinEntry->Delete();
  if(this->MinimumValueFrame)
          this->MinimumValueFrame->Delete();
  if(this->MaximumValueFrame)
          this->MaximumValueFrame->Delete();
  if (this->MinimumValueCheckButton)
          this->MinimumValueCheckButton->Delete();
  if (this->MaximumValueCheckButton)
          this->MaximumValueCheckButton->Delete();
        if(this->ViewFrame)
          this->ViewFrame->Delete();  
        if(this->ViewPropertyButton)
          this->ViewPropertyButton->Delete();
  if(this->ViewLegendButton)
          this->ViewLegendButton->Delete();
  if(this->ClippingPlaneMenuButton)
          this->ClippingPlaneMenuButton->Delete();
 }
//----------------------------------------------------------------------------
void vtkKWMimxReBinFEMeshMaterialPropertiesFromImageGroup::CreateWidget()
{
  if(this->IsCreated())
  {
    vtkErrorMacro("class already created");
    return;
  }

  this->Superclass::CreateWidget();
  if(!this->FEMeshListComboBox) 
  {
     this->FEMeshListComboBox = vtkKWComboBoxWithLabel::New();
  }
  this->MainFrame->SetParent(this->GetParent());
  this->MainFrame->Create();
  //this->MainFrame->SetLabelText("Bin Material Properties");

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 6 -fill x", 
    this->MainFrame->GetWidgetName());

  FEMeshListComboBox->SetParent(this->MainFrame);
  FEMeshListComboBox->Create();
  this->FEMeshListComboBox->GetWidget()->SetCommand(
          this, "FEMeshSelectionChangedCallback");
  FEMeshListComboBox->SetLabelText("Mesh : ");
  FEMeshListComboBox->SetLabelWidth( 20 );
  FEMeshListComboBox->GetWidget()->ReadOnlyOn();
  //FEMeshListComboBox->GetWidget()->SetBalloonHelpString("Surface onto which the resulting FE Mesh projected");

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
    FEMeshListComboBox->GetWidgetName());

  if(!this->ElementSetComboBox) 
  {
          this->ElementSetComboBox = vtkKWComboBoxWithLabel::New();
  }
  ElementSetComboBox->SetParent(this->MainFrame);
  ElementSetComboBox->Create();
  this->ElementSetComboBox->GetWidget()->SetCommand(
          this, "ElementSetChangedCallback");
  ElementSetComboBox->SetLabelText("Element Set : ");
  ElementSetComboBox->GetWidget()->ReadOnlyOn();
  ElementSetComboBox->SetLabelWidth( 20 );
  this->GetApplication()->Script(
          "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
          ElementSetComboBox->GetWidgetName());

  if(!this->MinimumValueFrame)
          this->MinimumValueFrame = vtkKWFrame::New();
  this->MinimumValueFrame->SetParent(this->MainFrame);
  this->MinimumValueFrame->Create();
  this->GetApplication()->Script(
          "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
          this->MinimumValueFrame->GetWidgetName());

  this->MinimumValueCheckButton = vtkKWCheckButtonWithLabel::New();
  this->MinimumValueCheckButton->SetParent(this->MinimumValueFrame);
  this->MinimumValueCheckButton->Create();
  this->MinimumValueCheckButton->SetLabelText("Specify Minimum Value : ");
  this->MinimumValueCheckButton->SetLabelWidth(20);
  this->MinimumValueCheckButton->SetLabelPositionToLeft();
  this->MinimumValueCheckButton->GetWidget()->SetCommand(this, "MinimumValueCheckButtonCallback");
  this->GetApplication()->Script(
   "pack %s -side top -anchor nw -expand n -padx 2 -pady 6", 
   this->MinimumValueCheckButton->GetWidgetName());

  if (!this->MinimumRebinEntry)
          this->MinimumRebinEntry = vtkKWEntryWithLabel::New();

  this->MinimumRebinEntry->SetParent(this->MinimumValueFrame);
  this->MinimumRebinEntry->Create();
  //this->MinimumRebinEntry->SetWidth(10);
  this->MinimumRebinEntry->SetLabelText("Minimum Value : ");
  this->MinimumRebinEntry->SetLabelWidth(20);
  this->MinimumRebinEntry->GetWidget()->SetValueAsDouble(0.0);
  this->MinimumRebinEntry->GetWidget()->SetRestrictValueToDouble();

  this->GetApplication()->Script(
          "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
          this->MinimumRebinEntry->GetWidgetName());

  this->MinimumRebinEntry->SetEnabled(0);

  if(!this->MaximumValueFrame)
          this->MaximumValueFrame = vtkKWFrame::New();
  this->MaximumValueFrame->SetParent(this->MainFrame);
  this->MaximumValueFrame->Create();
  this->GetApplication()->Script(
          "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
          this->MaximumValueFrame->GetWidgetName());
 
  this->MaximumValueCheckButton = vtkKWCheckButtonWithLabel::New();
  this->MaximumValueCheckButton->SetParent(this->MaximumValueFrame);
  this->MaximumValueCheckButton->Create();
  this->MaximumValueCheckButton->SetLabelText("Specify Maximum Value : ");
  this->MaximumValueCheckButton->SetLabelWidth(20);
  this->MaximumValueCheckButton->SetLabelPositionToLeft();
  this->MaximumValueCheckButton->GetWidget()->SetCommand(this, "MaximumValueCheckButtonCallback");
  this->GetApplication()->Script(
          "pack %s -side top -anchor nw -expand n -padx 2 -pady 6", 
          this->MaximumValueCheckButton->GetWidgetName());

  if (!this->MaximumRebinEntry)
          this->MaximumRebinEntry = vtkKWEntryWithLabel::New();

  this->MaximumRebinEntry->SetParent(this->MaximumValueFrame);
  this->MaximumRebinEntry->Create();
  this->MaximumRebinEntry->SetLabelText("Maximum Value : ");
  //this->MaximumRebinEntry->SetWidth(10);
  this->MaximumRebinEntry->SetLabelWidth(20);
  this->MaximumRebinEntry->GetWidget()->SetValueAsDouble(0.0);
  this->MaximumRebinEntry->GetWidget()->SetRestrictValueToDouble();

  this->GetApplication()->Script(
          "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
          this->MaximumRebinEntry->GetWidgetName());
  this->MaximumRebinEntry->SetEnabled(0);

  if (!this->NumOfBinsEntry)
          this->NumOfBinsEntry = vtkKWEntryWithLabel::New();

  this->NumOfBinsEntry->SetParent(this->MainFrame);
  this->NumOfBinsEntry->Create();
  //this->NumOfBinsEntry->SetWidth(4);
  this->NumOfBinsEntry->SetLabelText("Number of Bins : ");
  this->NumOfBinsEntry->SetLabelWidth(20);
  this->NumOfBinsEntry->GetWidget()->SetValueAsInt(1);
  this->NumOfBinsEntry->GetWidget()->SetRestrictValueToInteger();

  this->GetApplication()->Script(
          "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
          this->NumOfBinsEntry->GetWidgetName());

  if (!this->ViewFrame)
    this->ViewFrame = vtkKWFrameWithLabel::New();
  this->ViewFrame->SetParent( this->MainFrame );
  this->ViewFrame->Create();
  this->ViewFrame->SetLabelText("View");
  this->GetApplication()->Script("pack %s -side top -anchor nw -expand n -fill x -padx 2 -pady 2",
              this->ViewFrame->GetWidgetName() );    
  this->ViewFrame->CollapseFrame();
  
  if (!this->ViewPropertyButton)
    this->ViewPropertyButton = vtkKWCheckButtonWithLabel::New();
  this->ViewPropertyButton->SetParent(this->ViewFrame->GetFrame());
  this->ViewPropertyButton->Create();
  this->ViewPropertyButton->GetWidget()->SetCommand(this, "ViewMaterialPropertyCallback");
  this->ViewPropertyButton->GetWidget()->SetText("View Properties");
  this->ViewPropertyButton->GetWidget()->SetEnabled( 0 );
  this->GetApplication()->Script(
        "grid %s -row 0 -column 0 -sticky ne -padx 2 -pady 2", 
        this->ViewPropertyButton->GetWidgetName());

  if (!this->ViewLegendButton)
    this->ViewLegendButton = vtkKWCheckButtonWithLabel::New();
  this->ViewLegendButton->SetParent(this->ViewFrame->GetFrame());
  this->ViewLegendButton->Create();
  this->ViewLegendButton->GetWidget()->SetCommand(this, "ViewPropertyLegendCallback");
  this->ViewLegendButton->GetWidget()->SetText("View Legend");
  this->ViewLegendButton->GetWidget()->SetEnabled( 0 );
  this->GetApplication()->Script(
        "grid %s -row 0 -column 1 -sticky ne -padx 2 -pady 2", 
        this->ViewLegendButton->GetWidgetName());
  
  if(!this->ClippingPlaneMenuButton)    
                this->ClippingPlaneMenuButton = vtkKWMenuButtonWithLabel::New();
        this->ClippingPlaneMenuButton->SetParent(this->ViewFrame->GetFrame());
        this->ClippingPlaneMenuButton->Create();
        this->ClippingPlaneMenuButton->SetBorderWidth(0);
        this->ClippingPlaneMenuButton->SetReliefToGroove();
        this->ClippingPlaneMenuButton->GetWidget()->SetEnabled( 0 );
        this->ClippingPlaneMenuButton->SetLabelText("Clipping Plane :");
        this->GetApplication()->Script(
          "grid %s -row 1 -column 0 -sticky ne -padx 2 -pady 2", 
                this->ClippingPlaneMenuButton->GetWidgetName());
        this->ClippingPlaneMenuButton->GetWidget()->GetMenu()->AddRadioButton(
                "Off",this, "ClippingPlaneCallback 1");
        this->ClippingPlaneMenuButton->GetWidget()->GetMenu()->AddRadioButton(
                "On",this, "ClippingPlaneCallback 2");
  this->ClippingPlaneMenuButton->GetWidget()->GetMenu()->AddRadioButton(
                "Invert",this, "ClippingPlaneCallback 3");
        this->ClippingPlaneMenuButton->GetWidget()->SetValue("Off");
        
        
  this->ApplyButton->SetParent(this->MainFrame);
  this->ApplyButton->Create();
  this->ApplyButton->SetReliefToFlat();
  this->ApplyButton->SetImageToIcon( this->GetMimxMainWindow()->GetApplyButtonIcon() );
  //this->ApplyButton->SetText("Apply");
  this->ApplyButton->SetCommand(this, "ReBinFEMeshMaterialPropertiesFromImageApplyCallback");
  this->GetApplication()->Script(
          "pack %s -side left -anchor nw -expand y -padx 5 -pady 2", 
          this->ApplyButton->GetWidgetName());

  this->CancelButton->SetParent(this->MainFrame);
  this->CancelButton->Create();
  this->CancelButton->SetReliefToFlat();
  this->CancelButton->SetImageToIcon( this->GetMimxMainWindow()->GetCancelButtonIcon() );
  //this->CancelButton->SetText("Cancel");
  this->CancelButton->SetCommand(this, "ReBinFEMeshMaterialPropertiesFromImageCancelCallback");
  this->GetApplication()->Script(
    "pack %s -side right -anchor ne -expand y -padx 5 -pady 2", 
    this->CancelButton->GetWidgetName());

}
//----------------------------------------------------------------------------
void vtkKWMimxReBinFEMeshMaterialPropertiesFromImageGroup::Update()
{
        this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxReBinFEMeshMaterialPropertiesFromImageGroup::UpdateEnableState()
{
        this->UpdateObjectLists();
        this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
int vtkKWMimxReBinFEMeshMaterialPropertiesFromImageGroup::
        ReBinFEMeshMaterialPropertiesFromImageApplyCallback()
{
        vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();

  if(!strcmp(this->FEMeshListComboBox->GetWidget()->GetValue(),""))
  {
        callback->ErrorMessage("FE Mesh not chosen");
        return 0;
  }
    vtkKWComboBox *combobox = this->FEMeshListComboBox->GetWidget();
    const char *name = combobox->GetValue();
  strcpy(this->meshName, name);
        int num = combobox->GetValueIndex(name);
        if(num < 0 || num > combobox->GetNumberOfValues()-1)
        {
                callback->ErrorMessage("Choose valid FE mesh");
                combobox->SetValue("");
                return 0;
        }
        
        int numberofbins = this->NumOfBinsEntry->GetWidget()->GetValueAsInt();

        if(numberofbins <= 1)
        {
                callback->ErrorMessage("Number of Bins Should always be > 1. If the number of bins = 1 then apply material property as constant" );
                return 0;
        }

        const char *elsetname = this->ElementSetComboBox->GetWidget()->GetValue();

        if(!strcmp(elsetname,""))
        {
                callback->ErrorMessage("Choose valid Element Set");
                return 0;
        }
        strcpy(this->elementSetName, elsetname);

        vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(
                this->FEMeshList->GetItem(combobox->GetValueIndex(name)));

  vtkUnstructuredGrid *ugrid = vtkMimxMeshActor::SafeDownCast(this->FEMeshList
     ->GetItem(combobox->GetValueIndex(name)))->GetDataSet();

        char str[256];
        strcpy(str, elsetname);
        strcat(str, "_Image_Based_Material_Property");
        
        vtkDoubleArray *matarray = vtkDoubleArray::SafeDownCast(
                ugrid->GetCellData()->GetArray(str));
        if(!matarray)
        {
                callback->ErrorMessage("Image based material properties do not exist for binning. Calculate material properties first");
                return 0;
        }

        float min = this->MinimumRebinEntry->GetWidget()->GetValueAsDouble();
        float max = this->MaximumRebinEntry->GetWidget()->GetValueAsDouble();
        int minstate = this->MinimumValueCheckButton->GetWidget()->GetState();
        int maxstate = this->MaximumValueCheckButton->GetWidget()->GetState();
        if(minstate && maxstate)
        {
                if(min >= max)
                {
                        callback->ErrorMessage("Invalid minimum and maximum values");
                        return 0;               
                }
        }
        strcat(str, "_ReBin");
        matarray = vtkDoubleArray::SafeDownCast(ugrid->GetCellData()->GetArray(str));
        if(matarray)
        {
                vtkKWMessageDialog *Dialog = vtkKWMessageDialog::New();
                Dialog->SetStyleToYesNo();
                Dialog->SetApplication(this->GetApplication());
                Dialog->Create();
                Dialog->SetTitle("Your Attention Please!");
                Dialog->SetText("Binned material property data already present. Would you like to overwrite the data ?");
                Dialog->Invoke();
                if(Dialog->GetStatus() == vtkKWMessageDialog::StatusCanceled)
                {
                        Dialog->Delete();
                        return 1;
                }
                Dialog->Delete();
        }
        //
        vtkMimxRebinMaterialProperty *rebinmatprops = 
                vtkMimxRebinMaterialProperty::New();
        rebinmatprops->SetInput(ugrid);
        rebinmatprops->SetElementSetName(elsetname);
        rebinmatprops->SetNumberOfHistogramBins(numberofbins+1);
        callback->SetState(0);
        rebinmatprops->AddObserver(vtkCommand::ErrorEvent, callback, 1.0);
        if(minstate)
        {
                rebinmatprops->SetComputeMinBin(0);
                rebinmatprops->SetBinLowerBound(min);
        }
        if(maxstate)
        {
                rebinmatprops->SetComputeMaxBin(0);
                rebinmatprops->SetBinUpperBound(max);
        }

        rebinmatprops->Update();
        if (!callback->GetState())
        {
                ugrid->Initialize();
                ugrid->DeepCopy(rebinmatprops->GetOutput());
                meshActor->StoreImageBasedMaterialPropertyReBin(elsetname);
        }
        else
        {
                rebinmatprops->RemoveObserver(callback);
                rebinmatprops->Delete();
                return 0;
        }
  
  this->ViewPropertyButton->GetWidget()->SetEnabled( 1 );
  this->ViewLegendButton->GetWidget()->SetEnabled( 1 );
  this->ClippingPlaneMenuButton->GetWidget()->SetEnabled( 1 );
  
  this->GetMimxMainWindow()->SetStatusText("Binned Material Properties");
  
        return 0;
}
//----------------------------------------------------------------------------
void vtkKWMimxReBinFEMeshMaterialPropertiesFromImageGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxReBinFEMeshMaterialPropertiesFromImageGroup::ReBinFEMeshMaterialPropertiesFromImageCancelCallback()
{
        this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName());
        this->MenuGroup->SetMenuButtonsEnabled(1);
          this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(1);
}
//------------------------------------------------------------------------------
void vtkKWMimxReBinFEMeshMaterialPropertiesFromImageGroup::UpdateObjectLists()
{
  this->UpdateMeshComboBox( this->FEMeshListComboBox->GetWidget() );
  /*
  this->FEMeshListComboBox->GetWidget()->DeleteAllValues();
  
  int defaultItem = -1;
  for (int i = 0; i < this->FEMeshList->GetNumberOfItems(); i++)
  {
          FEMeshListComboBox->GetWidget()->AddValue(
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
    FEMeshListComboBox->GetWidget()->SetValue(
          this->FEMeshList->GetItem(defaultItem)->GetFileName());
  }
  */
  this->FEMeshSelectionChangedCallback(NULL);
}
//------------------------------------------------------------------------------
void vtkKWMimxReBinFEMeshMaterialPropertiesFromImageGroup::
        ReBinFEMeshMaterialPropertiesFromImageDoneCallback()
{
        //if(this->ReBinFEMeshMaterialPropertiesFromImageApplyCallback())
        //      this->ReBinFEMeshMaterialPropertiesFromImageCancelCallback();
}
//---------------------------------------------------------------------------------
void vtkKWMimxReBinFEMeshMaterialPropertiesFromImageGroup::
        FEMeshSelectionChangedCallback(const char *vtkNotUsed(Selection))
{
        if(!this->FEMeshList->GetNumberOfItems())
        {
                return;
        }

        const char *selection = this->FEMeshListComboBox->GetWidget()->GetValue();
        vtkUnstructuredGrid *ugrid = vtkMimxMeshActor::SafeDownCast(this->FEMeshList->
                GetItem(FEMeshListComboBox->GetWidget()->GetValueIndex(selection)))->GetDataSet();

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
        this->ElementSetChangedCallback(this->ElementSetComboBox->GetWidget()->GetValue());
}
//-------------------------------------------------------------------------------------------------
void vtkKWMimxReBinFEMeshMaterialPropertiesFromImageGroup::
        ElementSetChangedCallback(const char *Selection)
{
        if(!strcmp(Selection,""))
        {
                return;
        }

        if(!strcmp(this->FEMeshListComboBox->GetWidget()->GetValue(),""))
        {
                return;
        }

        char str[256];
        strcpy(str, Selection);
        strcat(str, "_Image_Based_Material_Property");

        vtkKWComboBox *combobox = this->FEMeshListComboBox->GetWidget();
        const char *name = combobox->GetValue();

        vtkUnstructuredGrid *ugrid = vtkMimxMeshActor::SafeDownCast(this->FEMeshList
                ->GetItem(combobox->GetValueIndex(name)))->GetDataSet();

        vtkDoubleArray *matarray = vtkDoubleArray::SafeDownCast(
                ugrid->GetCellData()->GetArray(str));
        float min = VTK_FLOAT_MAX;
        float max = VTK_FLOAT_MIN;
        if(matarray)
        {
                int i;
                for (i=0; i<matarray->GetNumberOfTuples(); i++)
                {
                        float matvalue = matarray->GetValue(i);
                        if(matvalue >= 0.0)
                        {
                                if(matvalue < min)      min = matvalue;
                                if(matvalue > max)      max = matvalue;
                        }
                }
        }
        else
        {
                min = 0.0;
                max = 0.0;
        }
        this->MinimumRebinEntry->GetWidget()->SetValueAsDouble(min);
        this->MaximumRebinEntry->GetWidget()->SetValueAsDouble(max);
}
//--------------------------------------------------------------------------------------------------
void vtkKWMimxReBinFEMeshMaterialPropertiesFromImageGroup::
        MinimumValueCheckButtonCallback(int State)
{
        this->MinimumRebinEntry->SetEnabled(State);
}
//--------------------------------------------------------------------------------------------------
void vtkKWMimxReBinFEMeshMaterialPropertiesFromImageGroup::
        MaximumValueCheckButtonCallback(int State)
{
        this->MaximumRebinEntry->SetEnabled(State);
}
//-------------------------------------------------------------------------------------
void vtkKWMimxReBinFEMeshMaterialPropertiesFromImageGroup::ViewMaterialPropertyCallback( int mode )
{

  vtkKWComboBox *combobox = this->FEMeshListComboBox->GetWidget();
  vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(
           this->FEMeshList->GetItem(combobox->GetValueIndex( this->meshName )));
           
  if ( mode )
  {
    char scalarName[256];
    strcpy(scalarName, this->elementSetName);
          strcat(scalarName, "_Image_Based_Material_Property_ReBin");
          meshActor->SetMeshScalarVisibility(true);
          meshActor->SetMeshScalarName(scalarName);
    //meshActor->SetElementSetScalarName(this->elementSetName, scalarName);
    //meshActor->SetElementSetScalarVisibility(this->elementSetName, true);
    this->GetMimxMainWindow()->GetRenderWidget()->Render();
  }
  else
  {
 //   meshActor->SetElementSetScalarVisibility(this->elementSetName, false);
        meshActor->SetMeshScalarVisibility(false);
    this->ViewPropertyLegendCallback(0);
    this->ViewLegendButton->GetWidget()->SetSelectedState( 0 );
    this->GetMimxMainWindow()->GetRenderWidget()->Render(); 
  }
 
}

//-------------------------------------------------------------------------------------
void vtkKWMimxReBinFEMeshMaterialPropertiesFromImageGroup::ViewPropertyLegendCallback( int mode )
{

  vtkKWComboBox *combobox = this->FEMeshListComboBox->GetWidget();
  vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(
           this->FEMeshList->GetItem(combobox->GetValueIndex( this->meshName )));
           
  if ( mode )
  {
    meshActor->SetMeshLegendVisibility(true);
    this->GetMimxMainWindow()->GetRenderWidget()->Render();
  }
  else
  {
    meshActor->SetMeshLegendVisibility(false);
    this->GetMimxMainWindow()->GetRenderWidget()->Render(); 
  }

}

//-------------------------------------------------------------------------------------
int vtkKWMimxReBinFEMeshMaterialPropertiesFromImageGroup::ClippingPlaneCallback( int mode )
{
  //vtkKWComboBox *combobox = this->FEMeshListComboBox->GetWidget();
  //vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(
  //         this->FEMeshList->GetItem(combobox->GetValueIndex( this->meshName )));
  //         
  //if (mode == 1)
  //{
  //  meshActor->DisableElementSetCuttingPlane(this->elementSetName);
  //}
  //else if (mode == 2)
  //{
  //  meshActor->EnableElementSetCuttingPlane(this->elementSetName);
  //  meshActor->SetInvertCuttingPlane( false );
  //}
  //else
  //{
  //  meshActor->EnableElementSetCuttingPlane(this->elementSetName);
  //  meshActor->SetInvertCuttingPlane( true );  
  //}
  //this->GetMimxMainWindow()->GetRenderWidget()->Render(); 
  //return 1;

        vtkKWComboBox *combobox = this->FEMeshListComboBox->GetWidget();
        vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(
                this->FEMeshList->GetItem(combobox->GetValueIndex( combobox->GetValue())));

        if (mode == 1)
        {
                meshActor->DisableMeshCuttingPlane();
        }
        else if (mode == 2)
        {
                meshActor->EnableMeshCuttingPlane();
                meshActor->SetInvertCuttingPlane( false );
        }
        else
        {
                meshActor->EnableMeshCuttingPlane();
                meshActor->SetInvertCuttingPlane( true );  
        }
        this->GetMimxMainWindow()->GetRenderWidget()->Render(); 
        return 1;

}
//--------------------------------------------------------------------------------------------------
