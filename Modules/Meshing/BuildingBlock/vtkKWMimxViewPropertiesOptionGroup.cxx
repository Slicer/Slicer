/*=========================================================================
Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxViewPropertiesOptionGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:40 $
Version:   $Revision: 1.14.2.3 $

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

#include "vtkKWMimxViewPropertiesOptionGroup.h"

#include "vtkObjectFactory.h"

#include "vtkKWApplication.h"
#include "vtkKWCheckButton.h"
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWComboBox.h"
#include "vtkKWComboBoxWithLabel.h"
#include "vtkKWEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWIcon.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWPushButton.h"
#include "vtkKWRenderWidget.h"

#include "vtkMimxMeshActor.h"


#include "Resources/mimxCancel.h"

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD                 1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxViewPropertiesOptionGroup);
vtkCxxRevisionMacro(vtkKWMimxViewPropertiesOptionGroup, "$Revision: 1.14.2.3 $");

//----------------------------------------------------------------------------
vtkKWMimxViewPropertiesOptionGroup::vtkKWMimxViewPropertiesOptionGroup()
{
        this->MeshActor = NULL;
        this->MimxMainWindow = NULL;
        this->ElementSetName = NULL;
        this->ArrayName = NULL;
        this->RangeFrame = NULL;
        this->RangeMaximum = NULL;
        this->RangeMinimum = NULL;
        this->UpdateRange = NULL;
        this->SpecifyRangeButton = NULL;
        this->OptionsFrame = NULL;
        this->LegendFrame = NULL;
        this->ColorMenuButton = NULL;
        this->DecimalPrecisionWidget = NULL;
        this->SpecifyTitleButton = NULL;
        this->LegendTitle = NULL;
        this->CloseButton = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxViewPropertiesOptionGroup::~vtkKWMimxViewPropertiesOptionGroup()
{
        if (this->ColorMenuButton)
                this->ColorMenuButton->Delete();
        if (this->DecimalPrecisionWidget)
                this->DecimalPrecisionWidget->Delete();
        if (this->SpecifyTitleButton)
                this->SpecifyTitleButton->Delete();
        if (this->LegendTitle)
                this->LegendTitle->Delete();
        if(this->RangeFrame)
                this->RangeFrame->Delete();
        if(this->RangeMinimum)
                this->RangeMinimum->Delete();
        if(this->RangeMaximum)
                this->RangeMaximum->Delete();
        if(this->UpdateRange)
                this->UpdateRange->Delete();
        if(this->SpecifyRangeButton)
                this->SpecifyRangeButton->Delete();
        if(this->OptionsFrame)
                this->OptionsFrame->Delete();
  if (this->CloseButton)
                this->CloseButton->Delete();
}
//----------------------------------------------------------------------------
void vtkKWMimxViewPropertiesOptionGroup::CreateWidget()
{

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }
  
  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();

  this->SetResizable(1, 1);
  this->SetMinimumSize(250, 220);
  this->ModalOn( );
  this->SetMasterWindow(this->MimxMainWindow);
  this->SetTitle("Display Options");

  if(!this->OptionsFrame)
          this->OptionsFrame = vtkKWFrame::New();
  this->OptionsFrame->SetParent(this);
  this->OptionsFrame->Create();
  this->GetApplication()->Script(
          "pack %s -side top -anchor nw -expand n -padx 2 -pady 0 -fill x", 
          this->OptionsFrame->GetWidgetName());

  if (!this->ColorMenuButton)   
                this->ColorMenuButton = vtkKWMenuButtonWithLabel::New();
        this->ColorMenuButton->SetParent(this->OptionsFrame);
        this->ColorMenuButton->Create();
        this->ColorMenuButton->SetBorderWidth(0);
        this->ColorMenuButton->SetReliefToGroove();
        this->ColorMenuButton->SetLabelText("Spectrum :");
        this->ColorMenuButton->SetPadX(2);
        this->ColorMenuButton->SetPadY(2);
        this->ColorMenuButton->GetWidget()->SetWidth(20);
        this->GetApplication()->Script("pack %s -side top -anchor n -padx 2 -pady 15", 
                this->ColorMenuButton->GetWidgetName());
        
        this->ColorMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "Red to Blue",this, "ColorModeCallback 1");
  this->ColorMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "Blue to Red",this, "ColorModeCallback 2");
  this->ColorMenuButton->GetWidget()->SetValue("Blue to Red");
  
  if(!this->RangeFrame)
          this->RangeFrame = vtkKWFrameWithLabel::New();
  this->RangeFrame->SetParent(this);
  this->RangeFrame->Create();
  this->RangeFrame->SetLabelText("Color Scale");
  this->GetApplication()->Script(
          "pack %s -side top -anchor nw -expand n -padx 2 -pady 0 -fill x", 
          this->RangeFrame->GetWidgetName());

  if (!this->SpecifyRangeButton)
          this->SpecifyRangeButton = vtkKWCheckButtonWithLabel::New();
  this->SpecifyRangeButton->SetParent(this->RangeFrame->GetFrame());
  this->SpecifyRangeButton->Create();
  this->SpecifyRangeButton->GetWidget()->SetCommand(this, "SpecifyRangeCallback");
  this->SpecifyRangeButton->GetWidget()->SetText("Specify ");
  //  this->ViewLegendButton->GetWidget()->SetEnabled( 0 );
  this->GetApplication()->Script(
          "grid %s -row 0 -column 0 -sticky nw -padx 2 -pady 2", 
          this->SpecifyRangeButton->GetWidgetName());

  if (!this->RangeMinimum)
          this->RangeMinimum = vtkKWEntryWithLabel::New();
  this->RangeMinimum->SetParent(this->RangeFrame->GetFrame());
  this->RangeMinimum->Create();
  this->RangeMinimum->SetWidth(5);
  this->RangeMinimum->SetLabelText("Minimum : ");
  this->RangeMinimum->GetWidget()->SetValue("");
  this->RangeMinimum->GetWidget()->SetRestrictValueToDouble();
  this->GetApplication()->Script(
          "grid %s -row 0 -column 1 -sticky nw -padx 2 -pady 2", 
          this->RangeMinimum->GetWidgetName());
  this->RangeMinimum->SetEnabled(0);

  if (!this->RangeMaximum)
          this->RangeMaximum = vtkKWEntryWithLabel::New();
  this->RangeMaximum->SetParent(this->RangeFrame->GetFrame());
  this->RangeMaximum->Create();
  this->RangeMaximum->SetWidth(5);
  this->RangeMaximum->SetLabelText("Maximum : ");
  this->RangeMaximum->GetWidget()->SetValue("");
  this->RangeMaximum->GetWidget()->SetRestrictValueToDouble();
  this->GetApplication()->Script(
          "grid %s -row 0 -column 2 -sticky nw -padx 2 -pady 2", 
          this->RangeMaximum->GetWidgetName());
  this->RangeMaximum->SetEnabled(0);

  if(!this->UpdateRange)
          this->UpdateRange = vtkKWPushButton::New();
  this->UpdateRange->SetParent(this->RangeFrame->GetFrame());
  this->UpdateRange->Create();
  this->UpdateRange->SetText("Update");
  this->UpdateRange->SetCommand(this, "UpdateRangeCallback");
  this->GetApplication()->Script(
          "grid %s -row 0 -column 3 -sticky nw -padx 2 -pady 2", 
          this->UpdateRange->GetWidgetName());
  this->UpdateRange->SetEnabled(0);
  
  if(!this->LegendFrame)
          this->LegendFrame = vtkKWFrameWithLabel::New();
  this->LegendFrame->SetParent(this);
  this->LegendFrame->Create();
  this->LegendFrame->SetLabelText("Legend");
  this->GetApplication()->Script(
          "pack %s -side top -anchor nw -expand n -padx 2 -pady 0 -fill x", 
          this->LegendFrame->GetWidgetName());
        
        if (!this->DecimalPrecisionWidget)
    this->DecimalPrecisionWidget = vtkKWComboBoxWithLabel::New();
  this->DecimalPrecisionWidget->SetParent( this->LegendFrame->GetFrame() );
  this->DecimalPrecisionWidget->Create();
  this->DecimalPrecisionWidget->SetLabelText("Floating Point Precision: ");
  this->DecimalPrecisionWidget->SetLabelWidth( 20 );
  this->DecimalPrecisionWidget->GetWidget()->SetWidth( 20 );
  this->DecimalPrecisionWidget->GetWidget()->SetCommand(this, "SetDecimalPrecisionCallback");
  char tmpStr[128];
  for (int i = 0; i < 7 ; i++)
  {
          sprintf(tmpStr,"%d",i);
          this->DecimalPrecisionWidget->GetWidget()->AddValue(tmpStr);
  }
  this->GetApplication()->Script("grid %s -row 0 -column 1 -sticky nw -padx 2 -pady 2", 
    this->DecimalPrecisionWidget->GetWidgetName());
  this->DecimalPrecisionWidget->GetWidget()->SetValue("3");
  
  if (!this->SpecifyTitleButton)
          this->SpecifyTitleButton = vtkKWCheckButtonWithLabel::New();
  this->SpecifyTitleButton->SetParent(this->LegendFrame->GetFrame());
  this->SpecifyTitleButton->Create();
  this->SpecifyTitleButton->GetWidget()->SetCommand(this, "SpecifyTitleCallback");
  this->SpecifyTitleButton->GetWidget()->SetText("Specify ");
  this->GetApplication()->Script(
          "grid %s -row 1 -column 0 -sticky nw -padx 2 -pady 2", 
          this->SpecifyTitleButton->GetWidgetName());
          
  if(!this->LegendTitle)
          this->LegendTitle = vtkKWEntryWithLabel::New();
  this->LegendTitle->SetParent( this->LegendFrame->GetFrame() );
  this->LegendTitle->Create();
  this->LegendTitle->SetLabelText("Title");
  this->LegendTitle->SetLabelWidth( 20 );
  this->LegendTitle->GetWidget()->SetWidth( 25 );
  this->LegendTitle->SetEnabled(0);
  this->LegendTitle->GetWidget()->SetCommand(this, "SetLegendTitleCallback");
  this->GetApplication()->Script(
          "grid %s -row 1 -column 1 -sticky nw -padx 2 -pady 2", 
          this->LegendTitle->GetWidgetName());
        
        vtkKWIcon *cancelIcon = vtkKWIcon::New();
  cancelIcon->SetImage( image_mimxCancel, 
                              image_mimxCancel_width, 
                              image_mimxCancel_height, 
                              image_mimxCancel_pixel_size);
                              
        if (!this->CloseButton)
    this->CloseButton = vtkKWPushButton::New();
  this->CloseButton->SetParent( this );
  this->CloseButton->Create();
  //this->CloseButton->SetText("Close");
  this->CloseButton->SetReliefToFlat();
  this->CloseButton->SetImageToIcon( cancelIcon );
  this->CloseButton->SetCommand(this, "Withdraw");
  this->GetApplication()->Script(
          "pack %s -side right -anchor ne -padx 2 -pady 2", 
          this->CloseButton->GetWidgetName());

  this->AddBinding("<Escape>", this, "Withdraw");
    
            
}
//----------------------------------------------------------------------------
void vtkKWMimxViewPropertiesOptionGroup::Update()
{
        this->SpecifyTitleButton->GetWidget()->SetSelectedState( 0 );
        this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxViewPropertiesOptionGroup::UpdateEnableState()
{
        this->Superclass::UpdateEnableState();
}
//------------------------------------------------------------------------------
void vtkKWMimxViewPropertiesOptionGroup::PrintSelf(ostream& os, vtkIndent indent)
{
        this->Superclass::PrintSelf(os,indent);
}
//------------------------------------------------------------------------------
void vtkKWMimxViewPropertiesOptionGroup::DeselectAllButtons()
{
        this->SpecifyTitleButton->GetWidget()->SetSelectedState( 0 );
}
//-------------------------------------------------------------------------------
void vtkKWMimxViewPropertiesOptionGroup::ColorModeCallback(int mode)
{
        int RangeType = 0;
        switch (mode)
        {
          case 1: RangeType = vtkMimxMeshActor::RedToBlue; break;
          case 2: RangeType = vtkMimxMeshActor::BlueToRed; break;
        }
        
        double *range = this->GetRange();
        this->MeshActor->SetColorRangeType(RangeType, this->ArrayName, this->ElementSetName, range);
        this->MimxMainWindow->RenderWidget->Render();
}
//-------------------------------------------------------------------------------
void vtkKWMimxViewPropertiesOptionGroup::UpdateRangeCallback()
{
        double *range = this->GetRange();
        if(this->MeshActor->GetDisplayMode() == vtkMimxMeshActor::DisplayMesh)
        {
                this->MeshActor->GenerateMeshMapperLookUpTable(this->ArrayName, range);
        }
        else
        {
                this->MeshActor->GenerateElementSetMapperLookUpTable(
                        this->ElementSetName, this->ArrayName, range);
        }
        this->MimxMainWindow->RenderWidget->Render();
//      delete range;
}
//---------------------------------------------------------------------------------
void vtkKWMimxViewPropertiesOptionGroup::SpecifyRangeCallback(int mode)
{
        this->RangeMaximum->SetEnabled(mode);
        this->RangeMinimum->SetEnabled(mode);
        this->UpdateRange->SetEnabled(mode);
        if (mode == 0)
        {
          double *range;
        if(this->MeshActor->GetDisplayMode() == vtkMimxMeshActor::DisplayMesh)
                {
                        range = this->MeshActor->ComputeMeshScalarRange(this->ArrayName);
                        this->MeshActor->GenerateMeshMapperLookUpTable(this->ArrayName, range);
                }
                else
                {
                        range = this->MeshActor->ComputeElementSetScalarRange(this->ElementSetName, this->ArrayName);
                        this->MeshActor->GenerateElementSetMapperLookUpTable(
                            this->ElementSetName, this->ArrayName, range);
                }
                this->RangeMinimum->GetWidget()->SetValueAsDouble(range[0]);
                this->RangeMaximum->GetWidget()->SetValueAsDouble(range[1]);
                this->MimxMainWindow->RenderWidget->Render();
        }
}
//---------------------------------------------------------------------------------
void vtkKWMimxViewPropertiesOptionGroup::SpecifyTitleCallback(int mode)
{
        this->LegendTitle->SetEnabled(mode);
}
//---------------------------------------------------------------------------------
double* vtkKWMimxViewPropertiesOptionGroup::GetRange()
{
        double *range;
        if(this->SpecifyRangeButton->GetWidget()->GetSelectedState())
        {
                range = new double[2];
                range[0] = this->RangeMinimum->GetWidget()->GetValueAsDouble();
                range[1] = this->RangeMaximum->GetWidget()->GetValueAsDouble();
        }
        else
        {
                if(this->MeshActor->GetDisplayMode() == vtkMimxMeshActor::DisplayMesh)
                {
                        range = this->MeshActor->ComputeMeshScalarRange(this->ArrayName);
                }
                else
                {
                        range = this->MeshActor->ComputeElementSetScalarRange(this->ElementSetName, this->ArrayName);
                }
                this->RangeMinimum->GetWidget()->SetValueAsDouble(range[0]);
                this->RangeMaximum->GetWidget()->SetValueAsDouble(range[1]);
        }
        return range;
}

//---------------------------------------------------------------------------------
void vtkKWMimxViewPropertiesOptionGroup::SetDecimalPrecisionCallback( char *charValue )
{
  //std::cout << "Precision: " << charValue << std::endl;
  if ( strlen(charValue) )
        {
          int precision = atoi(charValue);
          if ( this->MeshActor )
          {
          this->MeshActor->SetLegendPrecision( precision );
          this->MimxMainWindow->RenderWidget->Render();
          }
  }
}
//---------------------------------------------------------------------------------
void vtkKWMimxViewPropertiesOptionGroup::SetLegendTitleCallback( char *title )
{
  //std::cout << "Title: " << title << std::endl;
  if ( strlen(title) )
        {
          if ( this->MeshActor )
          {
          this->MeshActor->SetLegendTitle( title );
          this->MimxMainWindow->RenderWidget->Render();
          }
  }
}
//---------------------------------------------------------------------------------
void vtkKWMimxViewPropertiesOptionGroup::ResetValues()
{
         int colorType = vtkMimxMeshActor::RedToBlue;
         
         if ( this->MeshActor )
           colorType = this->MeshActor->GetColorRangeType();
           
         if(this->ColorMenuButton)
         {
                 switch ( colorType )
                 {
                   case vtkMimxMeshActor::RedToBlue:
                     this->ColorMenuButton->GetWidget()->SetValue("Red to Blue"); 
                     break;
                   case vtkMimxMeshActor::BlueToRed: 
                     this->ColorMenuButton->GetWidget()->SetValue("Blue to Red"); 
                     break;
                 }
         }
         
         if(this->SpecifyRangeButton)
         {
                 if (this->SpecifyRangeButton->GetWidget()->GetSelectedState())
                         this->SpecifyRangeButton->GetWidget()->SetSelectedState(0);
         }
         double range[2];
         if ( this->MeshActor )
           this->MeshActor->GetCurrentScalarRange( range );
         
         if ((this->RangeMaximum) && ( this->MeshActor ))
         {
                 this->RangeMaximum->GetWidget()->SetValueAsDouble(range[1]);
         }
         else
         {
         this->RangeMaximum->GetWidget()->SetValue("");
         }
         
         if ((this->RangeMinimum) && ( this->MeshActor ))
         {
                 this->RangeMinimum->GetWidget()->SetValueAsDouble( range[0] );
         }
         else
         {
         this->RangeMinimum->GetWidget()->SetValue("");
         }
         if ( ( this->LegendTitle ) && ( this->MeshActor ) )
         {
                 this->LegendTitle->GetWidget()->SetValue( this->MeshActor->GetLegendTitle() );
   }
   else
   {
     this->LegendTitle->GetWidget()->SetValue( "" );
   }
}
//---------------------------------------------------------------------------------
