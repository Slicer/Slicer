/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxDisplayMatPropGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.14.4.3 $

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

#include "vtkKWMimxDisplayMatPropGroup.h"

#include "vtkActor.h"
#include "vtkCellData.h"
#include "vtkDoubleArray.h"
#include "vtkFieldData.h"
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkStringArray.h"
#include "vtkUnstructuredGrid.h"

#include "vtkKWApplication.h"
#include "vtkKWCheckButton.h"
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWComboBox.h"
#include "vtkKWComboBoxWithLabel.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWIcon.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWPushButton.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWTkUtilities.h"

#include "vtkMimxErrorCallback.h"
#include "vtkMimxMeshActor.h"
#include "vtkMimxUnstructuredGridActor.h"

#include "vtkKWMimxDefineElSetGroup.h"
#include "vtkKWMimxMainMenuGroup.h"
#include "vtkKWMimxMainNotebook.h"
#include "vtkKWMimxMainUserInterfacePanel.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkKWMimxViewPropertiesOptionGroup.h"

#include "vtkLinkedListWrapper.h"

#include "Resources/mimxClipPlane.h"
#include "Resources/mimxDisplayOptions.h"
#include "Resources/mimxLegend.h"

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

// define the option types
#define VTK_KW_OPTION_NONE       0
#define VTK_KW_OPTION_LOAD                 1
#define ALL_SET_STRING           "All Elements"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxDisplayMatPropGroup);
vtkCxxRevisionMacro(vtkKWMimxDisplayMatPropGroup, "$Revision: 1.14.4.3 $");

//----------------------------------------------------------------------------
vtkKWMimxDisplayMatPropGroup::vtkKWMimxDisplayMatPropGroup()
{
  this->ComponentFrame = NULL;
  this->ObjectListComboBox = NULL;
  this->ElementSetComboBox = NULL;
  this->ViewLegendButton = NULL;
  this->EnablePlaneButton = NULL;
  this->InvertPlaneButton = NULL;
  this->ViewFrame = NULL;
  this->ViewOptionsGroup = NULL;
  this->DisplayOptionsButton = NULL;
  
  strcpy(this->elementSetSelectionPrevious,"");
  strcpy(this->elementSetName,"");
  strcpy(this->meshName,"");
}

//----------------------------------------------------------------------------
vtkKWMimxDisplayMatPropGroup::~vtkKWMimxDisplayMatPropGroup()
{
  if(this->ComponentFrame)
          this->ComponentFrame->Delete();
  if(this->ObjectListComboBox)
          this->ObjectListComboBox->Delete();
  if(this->ElementSetComboBox)
          this->ElementSetComboBox->Delete();
        if (this->ViewLegendButton)
          this->ViewLegendButton->Delete();
        if (this->EnablePlaneButton)
          this->EnablePlaneButton->Delete();
        if (this->InvertPlaneButton)
          this->InvertPlaneButton->Delete();
        if (this->ViewFrame)
          this->ViewFrame->Delete();
        if(this->ViewOptionsGroup)
          this->ViewOptionsGroup->Delete();
        if(this->DisplayOptionsButton)
          this->DisplayOptionsButton->Delete();
}
//----------------------------------------------------------------------------
void vtkKWMimxDisplayMatPropGroup::CreateWidget()
{
  if(this->IsCreated())
  {
    vtkErrorMacro("class already created");
    return;
  }

  this->Superclass::CreateWidget();
  

  this->MainFrame->SetParent(this->GetParent());
  this->MainFrame->Create();
  //this->MainFrame->SetLabelText("Display Material Properties");
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
  
  if (!this->ObjectListComboBox)        
     this->ObjectListComboBox = vtkKWComboBoxWithLabel::New();  
  ObjectListComboBox->SetParent(this->ComponentFrame->GetFrame());
  ObjectListComboBox->Create();
  ObjectListComboBox->SetLabelText("Mesh : ");
  ObjectListComboBox->SetLabelWidth(15);
  ObjectListComboBox->GetWidget()->ReadOnlyOn();
  ObjectListComboBox->GetWidget()->SetCommand(this, "SelectionChangedCallback");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
    ObjectListComboBox->GetWidgetName());
  
  
  // for element set listing
  if ( !this->ElementSetComboBox )      
          this->ElementSetComboBox = vtkKWComboBoxWithLabel::New();
  this->ElementSetComboBox->SetParent(this->MainFrame);
  this->ElementSetComboBox->Create();
  this->ElementSetComboBox->SetLabelText("Element Set : ");
  this->ElementSetComboBox->SetLabelWidth(15);
  this->ElementSetComboBox->GetWidget()->ReadOnlyOn();
  this->ElementSetComboBox->SetEnabled( 0 );
  this->ElementSetComboBox->GetWidget()->SetCommand(this, "ElementSetChangedCallback");
  this->GetApplication()->Script(
          "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
          ElementSetComboBox->GetWidgetName());


  if (!this->ViewFrame)
    this->ViewFrame = vtkKWFrame::New();
  this->ViewFrame->SetParent( this->MainFrame );
  this->ViewFrame->Create();
  this->GetApplication()->Script("pack %s -side top -anchor nw -expand n -fill x -padx 2 -pady 2",
              this->ViewFrame->GetWidgetName() );    
  
  vtkKWIcon *displayOptionIcon = vtkKWIcon::New();
  displayOptionIcon->SetImage( image_mimxDisplayOptions, 
                         image_mimxDisplayOptions_width, 
                         image_mimxDisplayOptions_height, 
                         image_mimxDisplayOptions_pixel_size);
                         
  if ( !this->DisplayOptionsButton )
          this->DisplayOptionsButton = vtkKWPushButton::New();
  this->DisplayOptionsButton->SetParent(this->ViewFrame);
  this->DisplayOptionsButton->Create();
  this->DisplayOptionsButton->SetCommand(this, "DisplayOptionsCallback");
  this->DisplayOptionsButton->SetImageToIcon( displayOptionIcon ); 
  this->DisplayOptionsButton->SetReliefToFlat( ); 
  this->DisplayOptionsButton->SetBalloonHelpString("Adjusts the display properties for the material properties");
  //this->DisplayOptionsButton->SetText("Display Options");
  this->GetApplication()->Script(
          "pack %s -side left -anchor n -padx 2 -pady 6", 
          this->DisplayOptionsButton->GetWidgetName());
                
  if (!this->ViewLegendButton)
    this->ViewLegendButton = vtkKWCheckButton::New();
  this->ViewLegendButton->SetParent(this->ViewFrame);
  this->ViewLegendButton->Create();
  vtkKWIcon *legendIcon = vtkKWIcon::New();
  legendIcon->SetImage(    image_mimxLegend, 
                           image_mimxLegend_width, 
                           image_mimxLegend_height, 
                           image_mimxLegend_pixel_size); 
  this->ViewLegendButton->SetImageToIcon( legendIcon ); 
  this->ViewLegendButton->SetSelectImageToIcon( legendIcon );
  this->ViewLegendButton->SetBorderWidth( 2 );                         
  this->ViewLegendButton->SetCommand(this, "ViewPropertyLegendCallback");
  this->ViewLegendButton->SetBalloonHelpString("Toggles display of the legend");
  this->GetApplication()->Script(
        "pack %s -side left -anchor center -padx 2 -pady 2", 
        this->ViewLegendButton->GetWidgetName());
  
              
  if (!this->EnablePlaneButton)
    this->EnablePlaneButton = vtkKWCheckButton::New();
  this->EnablePlaneButton->SetParent(this->ViewFrame);
  this->EnablePlaneButton->Create();
  vtkKWIcon *clipIcon = vtkKWIcon::New();
  clipIcon->SetImage(    image_mimxClipPlane, 
                         image_mimxClipPlane_width, 
                         image_mimxClipPlane_height, 
                         image_mimxClipPlane_pixel_size); 
  this->EnablePlaneButton->SetImageToIcon( clipIcon ); 
  this->EnablePlaneButton->SetSelectImageToIcon( clipIcon );
  this->EnablePlaneButton->SetBorderWidth( 2 );      
  this->EnablePlaneButton->SetCommand(this, "ClippingPlaneCallback");
  this->EnablePlaneButton->SetBalloonHelpString("Activate/deactivate the cutting plane to visualize the mesh cross section");
  this->GetApplication()->Script(
        "pack %s -side left -anchor center -padx 2 -pady 2", 
        this->EnablePlaneButton->GetWidgetName());
  
  if (!this->InvertPlaneButton)
    this->InvertPlaneButton = vtkKWCheckButtonWithLabel::New();
  this->InvertPlaneButton->SetParent(this->ViewFrame);
  this->InvertPlaneButton->Create();
  this->InvertPlaneButton->SetLabelText("Invert View");
  this->InvertPlaneButton->GetWidget()->SetCommand(this, "InvertPlaneCallback");
  this->InvertPlaneButton->SetEnabled( 0 );
  this->GetApplication()->Script(
        "pack %s -side left -anchor center -padx 2 -pady 2", 
        this->InvertPlaneButton->GetWidgetName());  

  
          
  this->ApplyButton->SetParent( this->MainFrame );
  this->ApplyButton->Create();
  this->ApplyButton->SetReliefToFlat();

  this->CancelButton->SetParent( this->MainFrame );
  this->CancelButton->Create();
  this->CancelButton->SetReliefToFlat();
  this->CancelButton->SetImageToIcon( this->GetMimxMainWindow()->GetCancelButtonIcon() );
  this->CancelButton->SetCommand(this, "DisplayCancelCallback");
  this->GetApplication()->Script(
    "pack %s -side right -anchor ne -expand y -padx 5 -pady 2", 
    this->CancelButton->GetWidgetName());
}
//----------------------------------------------------------------------------
void vtkKWMimxDisplayMatPropGroup::Update()
{
        this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxDisplayMatPropGroup::UpdateEnableState()
{
        this->UpdateObjectLists();
        this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
void vtkKWMimxDisplayMatPropGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxDisplayMatPropGroup::DisplayCancelCallback()
{
        vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
  vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(
           this->FEMeshList->GetItem(combobox->GetValueIndex( combobox->GetValue() )));
  
  this->ClippingPlaneCallback( 0 );
        this->ViewPropertyLegendCallback( 0 );    
                  
  if ( meshActor )
  {
          meshActor->SetMeshScalarVisibility(false);
          meshActor->SetAllElementSetScalarVisibility(true);
          meshActor->SetDisplayMode(vtkMimxMeshActor::DisplayMesh);
        meshActor->ShowHideAllElementSets(0);
  }
  
  this->ViewLegendButton->SetSelectedState( 0 );
  this->EnablePlaneButton->SetSelectedState( 0 );
  this->InvertPlaneButton->GetWidget()->SetSelectedState( 0 );
  this->InvertPlaneButton->SetEnabled( 0 );
  
  
        this->GetMimxMainWindow()->GetRenderWidget()->Render(); 
        
        if (this->ViewOptionsGroup)
          this->ViewOptionsGroup->Withdraw();
          
        this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName());
        /* These lines must be after pack forget */
        strcpy(this->elementSetSelectionPrevious,"");
  strcpy(this->objectSelectionPrevious,"");
  
        this->MenuGroup->SetMenuButtonsEnabled(1);
  this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(1);
}
//-----------------------------------------------------------------------------
void vtkKWMimxDisplayMatPropGroup::UpdateObjectLists()
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
    
    if (  viewedItem )
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
        else
        {
    ObjectListComboBox->GetWidget()->SetValue("");
        }
        */
        this->SelectionChangedCallback(ObjectListComboBox->GetWidget()->GetValue());
}
//--------------------------------------------------------------------------------
void vtkKWMimxDisplayMatPropGroup::SelectionChangedCallback(const char *Selection)
{
        if ( (strcmp(Selection,"") == 0) || (strcmp(this->objectSelectionPrevious, Selection) == 0) )
        {
                return;
        }
        
        this->RemovePreviousSelectionDisplay();
        
        vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
        vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(
                this->FEMeshList->GetItem(combobox->GetValueIndex( Selection )));
        vtkUnstructuredGrid *ugrid = meshActor->GetDataSet();

        // populate the element set list
        this->ElementSetComboBox->GetWidget()->DeleteAllValues();
        int i;
        vtkStringArray *strarray = vtkStringArray::SafeDownCast(
                ugrid->GetFieldData()->GetAbstractArray("Element_Set_Names"));

  if ( strarray )
  {
        int numarrrays = strarray->GetNumberOfValues();

        for (i=0; i<numarrrays; i++)
        {
                this->ElementSetComboBox->GetWidget()->AddValue( strarray->GetValue(i) );
        }
        this->ElementSetComboBox->GetWidget()->AddValue( ALL_SET_STRING );
        this->ElementSetComboBox->GetWidget()->SetValue( ALL_SET_STRING );
        }
        else
        {
        this->ElementSetComboBox->GetWidget()->AddValue( ALL_SET_STRING );
        this->ElementSetComboBox->GetWidget()->SetValue( ALL_SET_STRING );
        }
        
        meshActor->SetDisplayMode(vtkMimxMeshActor::DisplayMesh);
        meshActor->SetMeshScalarName( "Youngs_Modulus" );
        double *range = meshActor->ComputeMeshScalarRange("Youngs_Modulus");
        //std::cout << "Range : " << range[0] << " " << range[1] << std::endl;
        meshActor->SetColorRangeType(vtkMimxMeshActor::BlueToRed, "Youngs_Modulus", NULL, range);
        meshActor->ShowMesh();
  meshActor->SetMeshScalarVisibility(true);
  meshActor->SetAllElementSetScalarName( "Youngs_Modulus" );
  meshActor->SetAllElementSetScalarVisibility(true);
        meshActor->SetLegendTitle("Modulus");
        
        this->ElementSetComboBox->SetEnabled( 1 );
        this->ElementSetChangedCallback(this->ElementSetComboBox->GetWidget()->GetValue());
        
        if ( this->ViewLegendButton->GetSelectedState() )
          this->ViewPropertyLegendCallback( 1 );
        if ( this->EnablePlaneButton->GetSelectedState() )
    this->ClippingPlaneCallback( 1 );
  if ( this->InvertPlaneButton->GetWidget()->GetSelectedState() )
    this->InvertPlaneCallback( 1 );
  
  this->GetMimxMainWindow()->GetViewProperties()->UpdateVisibility();     
     
        strcpy(this->objectSelectionPrevious, Selection);
        
}
//-------------------------------------------------------------------------------------
void vtkKWMimxDisplayMatPropGroup::ElementSetChangedCallback(const char *Selection)
{
  //std::cout << "Element Set Changed " << Selection << std::endl;
        if ((strcmp(Selection,"") == 0) || (strcmp(this->elementSetSelectionPrevious, Selection) == 0) )
        {
                return;
        }
        
        // Get the Mesh 
        vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
        //const char *elSetName = this->ElementSetComboBox->GetWidget()->GetValue();
        vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(
                this->FEMeshList->GetItem(combobox->GetValueIndex(
                ObjectListComboBox->GetWidget()->GetValue())));
                
        
        std::string scalarName = "Youngs_Modulus";
        std::string setName = Selection;
        
        if (strcmp(this->objectSelectionPrevious, "") != 0)
        {
          //std::cout << "Clear Display " << std::endl; 
        vtkMimxMeshActor *prevMeshActor = vtkMimxMeshActor::SafeDownCast(
                  this->FEMeshList->GetItem(combobox->GetValueIndex(this->objectSelectionPrevious)));
                
        if (strcmp(this->elementSetSelectionPrevious, ALL_SET_STRING) == 0)
    {
      prevMeshActor->DisableMeshCuttingPlane( );
      prevMeshActor->SetMeshLegendVisibility( false );
        prevMeshActor->SetDisplayMode(vtkMimxMeshActor::DisplayMesh);
        prevMeshActor->ShowHideAllElementSets(0);
        }
        else
        {
        prevMeshActor->DisableElementSetCuttingPlane( this->elementSetSelectionPrevious );
      prevMeshActor->SetElementSetLegendVisibility( this->elementSetSelectionPrevious, false );
        prevMeshActor->SetDisplayMode(vtkMimxMeshActor::DisplayMesh);
        prevMeshActor->ShowHideAllElementSets(0);
        }
        }
        
        vtkUnstructuredGrid *ugrid = meshActor->GetDataSet();
        vtkDoubleArray *materialProperties = vtkDoubleArray::SafeDownCast(
                            ugrid->GetCellData()->GetArray(scalarName.c_str()));
        if (materialProperties == NULL)
        {
          materialProperties = vtkDoubleArray::New();
          materialProperties->SetNumberOfValues( ugrid->GetNumberOfCells() );
          materialProperties->SetName( scalarName.c_str() );
          for (int i=0;i<ugrid->GetNumberOfCells();i++) materialProperties->InsertValue(i,-9999);
          ugrid->GetCellData()->AddArray(materialProperties);
          materialProperties->Delete();
          //std::cout << "No mat property array" << std::endl;
        }
        
  if (strcmp(Selection, ALL_SET_STRING) == 0)
  {
    meshActor->SetMeshScalarName( scalarName );
    meshActor->SetMeshScalarVisibility(true);
        meshActor->SetDisplayMode(vtkMimxMeshActor::DisplayMesh);
        meshActor->ShowHideAllElementSets(0);
        meshActor->SetColorRangeType(vtkMimxMeshActor::BlueToRed, scalarName.c_str(), NULL, meshActor->ComputeMeshScalarRange(scalarName.c_str()));
        }
        else
        {
        meshActor->SetDisplayMode(vtkMimxMeshActor::DisplayElementSets);
        meshActor->ShowElementSet( setName );
        meshActor->SetElementSetScalarName( setName, scalarName );
    meshActor->SetElementSetScalarVisibility( setName, true);
    meshActor->SetColorRangeType(vtkMimxMeshActor::BlueToRed, scalarName.c_str(), setName.c_str(), meshActor->ComputeMeshScalarRange(scalarName.c_str()));
        }
        
  meshActor->SetLegendTextColor( this->GetMimxMainWindow()->GetTextColor() );
  //meshActor->SetColorRangeType(vtkMimxMeshActor::BlueToRed, scalarName.c_str(), NULL, meshActor->ComputeMeshScalarRange(scalarName.c_str()));
  meshActor->SetLegendTitle("Modulus");
  
        this->ViewLegendButton->Select( );
        
        if (this->ViewLegendButton->GetSelectedState( ) )
          this->ViewPropertyLegendCallback( 1 );
          
        if (this->EnablePlaneButton->GetSelectedState( ) )
        {
          this->ClippingPlaneCallback( 1 );
          if (this->InvertPlaneButton->GetWidget()->GetSelectedState( ) )
          {
            this->InvertPlaneCallback( 1 );
          }
        }
        
        //std::cout << "Update Done " << std::endl;
        strcpy(this->elementSetSelectionPrevious,Selection);

        if(this->ViewOptionsGroup)
        {
                this->ViewOptionsGroup->SetMeshActor(meshActor);
                this->ViewOptionsGroup->SetElementSetName(Selection);
                this->ViewOptionsGroup->Update();
        }
        
        this->GetMimxMainWindow()->GetRenderWidget()->Render();
}

//-------------------------------------------------------------------------------------
void vtkKWMimxDisplayMatPropGroup::ViewPropertyLegendCallback( int mode )
{
  vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
  vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(
           this->FEMeshList->GetItem(combobox->GetValueIndex( combobox->GetValue() )));

  if (!meshActor)       return;
  
      
  //const char *elsetName = this->ElementSetComboBox->GetWidget()->GetValue();
  
  if ( mode )
  {
          meshActor->SetMeshLegendVisibility(true);
          if (this->ViewOptionsGroup)
          {
          this->ViewOptionsGroup->GetDecimalPrecisionWidget()->SetEnabled(1);
          this->ViewOptionsGroup->GetSpecifyTitleButton()->SetEnabled(1);
          if ( this->ViewOptionsGroup->GetSpecifyTitleButton()->GetWidget()->GetSelectedState() )
            this->ViewOptionsGroup->GetLegendTitle()->SetEnabled(1);
          }
  }
  else
  {
          meshActor->SetMeshLegendVisibility(false);
          if (this->ViewOptionsGroup)
          {
          this->ViewOptionsGroup->GetDecimalPrecisionWidget()->SetEnabled(0);
          this->ViewOptionsGroup->GetSpecifyTitleButton()->SetEnabled(0);
          this->ViewOptionsGroup->GetLegendTitle()->SetEnabled(0);
          }
  }
  
  this->GetMimxMainWindow()->GetRenderWidget()->Render();
}

//-------------------------------------------------------------------------------------
void vtkKWMimxDisplayMatPropGroup::ClippingPlaneCallback( int mode )
{

  vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
  vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(
          this->FEMeshList->GetItem(combobox->GetValueIndex( combobox->GetValue() )));
  
  if (!meshActor)       return;  
  
  //bool invertMode = this->InvertPlaneButton->GetWidget()->GetSelectedState();
  
  const char *elsetName = this->ElementSetComboBox->GetWidget()->GetValue();
         
  if ( mode )
  {
    if (strcmp(elsetName, ALL_SET_STRING) == 0)
          {
          meshActor->EnableMeshCuttingPlane();
 //     meshActor->SetInvertCuttingPlane( invertMode );
      this->InvertPlaneButton->SetEnabled( 1 );
    }
    else
    {
      std::string setName = elsetName;
      meshActor->EnableElementSetCuttingPlane( setName );
      /***VAM***/
      //meshActor->SetInvertCuttingPlane( invertMode );
      this->InvertPlaneButton->SetEnabled( 1 );
    }
    
  }
  else
  {
    if (strcmp(elsetName, ALL_SET_STRING) == 0)
          {
          meshActor->DisableMeshCuttingPlane();
      this->InvertPlaneButton->SetEnabled( 0 );
    }
    else
    {
      std::string setName = elsetName;
      meshActor->DisableElementSetCuttingPlane( setName );
      this->InvertPlaneButton->SetEnabled( 0 );
    }
  }
  this->GetMimxMainWindow()->GetRenderWidget()->Render(); 

        return;
}

//-------------------------------------------------------------------------------------
void vtkKWMimxDisplayMatPropGroup::InvertPlaneCallback( int mode )
{

  vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
  vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(
          this->FEMeshList->GetItem(combobox->GetValueIndex( combobox->GetValue() )));
  
  if (!meshActor)       return;  
  
  meshActor->SetInvertCuttingPlane( static_cast<bool>(mode) );
  
  this->GetMimxMainWindow()->GetRenderWidget()->Render(); 

        return;
}

//----------------------------------------------------------------------------------------
void vtkKWMimxDisplayMatPropGroup::RemovePreviousSelectionDisplay()
{

        if(!strcmp(this->objectSelectionPrevious, ""))  return;
        vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
        vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(
                this->FEMeshList->GetItem(combobox->GetValueIndex( this->objectSelectionPrevious )));
        if(!meshActor)
        {
                return;
        }
        meshActor->SetMeshLegendVisibility(false);
  meshActor->SetMeshScalarVisibility(false);
  meshActor->SetAllElementSetScalarVisibility(true);
        meshActor->SetDisplayMode(vtkMimxMeshActor::DisplayMesh);
  meshActor->ShowHideAllElementSets(0);
  
  const char *elsetName = this->ElementSetComboBox->GetWidget()->GetValue();
    
        if (strcmp(elsetName, ALL_SET_STRING) == 0)
  {
          meshActor->DisableMeshCuttingPlane();
  }
  else
  {
    std::string setName = elsetName;
    meshActor->DisableElementSetCuttingPlane( setName );
  }
  meshActor->HideMesh();
  this->GetMimxMainWindow()->GetViewProperties()->UpdateVisibility();             
        this->GetMimxMainWindow()->GetRenderWidget()->Render(); 
}
//--------------------------------------------------------------------------------------------
void vtkKWMimxDisplayMatPropGroup::DisplayOptionsCallback( )
{
        //vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();

        vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
        const char *name = combobox->GetValue();
        vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(
                this->FEMeshList->GetItem(combobox->GetValueIndex(name)));

  const char *elsetName = this->ElementSetComboBox->GetWidget()->GetValue();
  
        
        if (!this->ViewOptionsGroup)
        {
                this->ViewOptionsGroup = vtkKWMimxViewPropertiesOptionGroup::New();
                this->ViewOptionsGroup->SetApplication( this->GetApplication() );
                this->ViewOptionsGroup->SetMimxMainWindow( this->GetMimxMainWindow() );
                this->ViewOptionsGroup->SetMeshActor(meshActor);
                this->ViewOptionsGroup->SetElementSetName(elsetName);
                this->ViewOptionsGroup->SetArrayName("Youngs_Modulus");
                this->ViewOptionsGroup->Create();
        }
        else
        {
                this->ViewOptionsGroup->SetMeshActor(meshActor);
                this->ViewOptionsGroup->SetElementSetName( elsetName );
                this->ViewOptionsGroup->SetArrayName("Youngs_Modulus");
        }
        
        this->ViewOptionsGroup->Display();
        this->ViewOptionsGroup->ResetValues();
        
}
//--------------------------------------------------------------------------------------------
