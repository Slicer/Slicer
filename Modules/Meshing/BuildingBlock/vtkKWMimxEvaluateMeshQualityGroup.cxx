/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxEvaluateMeshQualityGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.48.2.4 $

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

#include "vtkKWMimxEvaluateMeshQualityGroup.h"

#include "vtkActor.h"
#include "vtkCellData.h"
#include "vtkCellType.h"
#include "vtkDoubleArray.h"
#include "vtkIdList.h"
#include "vtkIntArray.h"
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkUnstructuredGrid.h"

#include "vtkKWApplication.h"
#include "vtkKWCheckButton.h"
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWComboBox.h"
#include "vtkKWComboBoxWithLabel.h"
#include "vtkKWDialog.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWIcon.h"
#include "vtkKWLabel.h"
#include "vtkKWListBox.h"
#include "vtkKWListBoxWithScrollbars.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWPushButton.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWUserInterfacePanel.h"

#include "vtkMeshQualityExtended.h"
#include "vtkMimxErrorCallback.h"
#include "vtkMimxMeshActor.h"

#include "vtkKWMimxMainNotebook.h"
#include "vtkKWMimxViewPropertiesOptionGroup.h"
#include "vtkKWMimxMainUserInterfacePanel.h"

#include <vtksys/SystemTools.hxx>

#include "Resources/mimxClipPlane.h"
#include "Resources/mimxDisplayOptions.h"
#include "Resources/mimxLegend.h"
#include "Resources/mimxSave.h"
#include "Resources/mimxSummaryReport.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxEvaluateMeshQualityGroup);
vtkCxxRevisionMacro(vtkKWMimxEvaluateMeshQualityGroup, "$Revision: 1.48.2.4 $");

//----------------------------------------------------------------------------

vtkKWMimxEvaluateMeshQualityGroup::vtkKWMimxEvaluateMeshQualityGroup()
{
  this->MeshListComboBox = NULL;
  this->NumberOfDistortedEntry = NULL;
  this->NumberOfElementsEntry = NULL;
  this->SummaryFrame = NULL;
  this->QualityMinimumEntry = NULL;
  this->QualityMaximumEntry = NULL;
  this->QualityAverageEntry = NULL;
  this->QualityVarianceEntry = NULL;
  this->SaveButton = NULL;
  this->ViewQualityButton = NULL;
  this->ClippingPlaneButton = NULL;
  this->ViewLegendButton = NULL;
  this->DistortedElementDialog = NULL;
  this->DistortedButtonFrame = NULL;
  this->SaveDistortedButton = NULL;
  this->CancelDistortedButton = NULL;
  this->DistortedElementsReport = NULL;
  this->FileBrowserDialog = NULL;
  this->ButtonFrame = NULL;
  this->QualityTypeButton = NULL;
  this->ViewFrame = NULL;
  this->QualityTypeLabel = NULL;
  this->DistoredListLabel = NULL;
  this->ComponentFrame = NULL;
  this->InvertPlaneButton = NULL;
  this->DistortedElementList = vtkIdList::New();
  this->DistortedMeshQuality = vtkDoubleArray::New();
  this->NumberOfCells = 0;
  this->QualityType = MESH_QUALITY_VOLUME;
  this->minimumQuality = 0.0;
  this->maximumQuality = 1.0;
  this->SummaryFrame = NULL;
  this->SummaryReviewFrame = NULL;
  this->DisplayOptionsButton = NULL;
  this->ViewOptionsGroup = NULL;
  strcpy(this->meshName, "");
  strcpy(this->qualityName, "");
  strcpy(this->PreviousSelection, "");
} 

//----------------------------------------------------------------------------
vtkKWMimxEvaluateMeshQualityGroup::~vtkKWMimxEvaluateMeshQualityGroup()
{
  /* Clean up all allocated Objects */
  if (this->MeshListComboBox)
    this->MeshListComboBox->Delete();
  if (this->NumberOfDistortedEntry)
    this->NumberOfDistortedEntry->Delete();
  if (this->NumberOfElementsEntry)
    this->NumberOfElementsEntry->Delete();
  if (this->SummaryFrame)
    this->SummaryFrame->Delete();
  if (this->SummaryReviewFrame)
    this->SummaryReviewFrame->Delete();
  if (this->QualityMinimumEntry)
    this->QualityMinimumEntry->Delete();
  if (this->QualityMaximumEntry)
    this->QualityMaximumEntry->Delete();
  if (this->QualityAverageEntry)
    this->QualityAverageEntry->Delete();
  if (this->QualityVarianceEntry)
    this->QualityVarianceEntry->Delete();
  if (this->SaveButton)
    this->SaveButton->Delete();
  if (this->DistortedElementDialog)
    this->DistortedElementDialog->Delete();
  if (this->DistortedButtonFrame)
    this->DistortedButtonFrame->Delete();
  if (this->SaveDistortedButton)
    this->SaveDistortedButton->Delete();
  if (this->CancelDistortedButton)
    this->CancelDistortedButton->Delete();
  if (this->DistortedElementsReport)
    this->DistortedElementsReport->Delete();
  if (this->DistortedElementList)
    this->DistortedElementList->Delete();
  if (this->DistortedMeshQuality)
    this->DistortedMeshQuality->Delete();
  if (this->FileBrowserDialog)
    this->FileBrowserDialog->Delete();
  if (this->ButtonFrame)
    this->ButtonFrame->Delete();
  if (this->ViewQualityButton)
    this->ViewQualityButton->Delete();
  if (this->ClippingPlaneButton)
    this->ClippingPlaneButton->Delete();
  if (this->ViewLegendButton)
    this->ViewLegendButton->Delete();
  if (this->QualityTypeButton)
    this->QualityTypeButton->Delete();
  if (this->ViewFrame)
    this->ViewFrame->Delete();
  if (this->QualityTypeLabel)
    this->QualityTypeLabel->Delete();
  if (this->DistoredListLabel)
    this->DistoredListLabel->Delete();
  if (this->ComponentFrame)
    this->ComponentFrame->Delete();
  if (this->InvertPlaneButton)
    this->InvertPlaneButton->Delete();
  if(this->DisplayOptionsButton)
          this->DisplayOptionsButton->Delete();
}

//----------------------------------------------------------------------------
void vtkKWMimxEvaluateMeshQualityGroup::CreateWidget()
{
  if (this->IsCreated())
    {
    vtkErrorMacro("class already created");
    return;
    }

  // Call the superclass to create the whole widget
  this->Superclass::CreateWidget();

  this->MainFrame->SetParent(this->GetParent());
  this->MainFrame->Create();
  //this->MainFrame->SetLabelText("Mesh Quality");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 0 -fill x", 
    this->MainFrame->GetWidgetName());

  if ( !this->ComponentFrame )
    this->ComponentFrame = vtkKWFrameWithLabel::New();
  this->ComponentFrame->SetParent( this->MainFrame );
  this->ComponentFrame->Create();
  this->ComponentFrame->SetLabelText("Mesh");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 6 -fill x", 
    this->ComponentFrame->GetWidgetName());
    
  if (!this->MeshListComboBox)  
    this->MeshListComboBox = vtkKWComboBoxWithLabel::New();
  this->MeshListComboBox->SetParent(this->ComponentFrame->GetFrame());
  this->MeshListComboBox->Create();
  this->MeshListComboBox->SetLabelText("Mesh: ");
  this->MeshListComboBox->SetLabelWidth(15);
  this->MeshListComboBox->GetWidget()->ReadOnlyOn();
  this->MeshListComboBox->GetWidget()->SetBalloonHelpString("Mesh for quality evaluation");
  this->MeshListComboBox->GetWidget()->SetCommand(this, "SelectionChangedCallback");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
    this->MeshListComboBox->GetWidgetName());

  this->ComponentFrame->CollapseFrame();

  /* Quality Metric */
  if (!this->QualityTypeButton) 
    this->QualityTypeButton = vtkKWMenuButtonWithLabel::New();  
  this->QualityTypeButton->SetParent(this->MainFrame);
  this->QualityTypeButton->Create();
  this->QualityTypeButton->SetLabelText("Metric :");
  this->QualityTypeButton->GetWidget()->SetWidth( 20 );
  this->QualityTypeButton->GetWidget()->GetMenu()->DeleteAllItems();
  this->QualityTypeButton->GetWidget()->GetMenu()->AddRadioButton("Volume",this, "EvaluateMeshQualityApplyCallback 1");
  this->QualityTypeButton->GetWidget()->GetMenu()->AddRadioButton("Edge Collapse",this, "EvaluateMeshQualityApplyCallback 2");
  this->QualityTypeButton->GetWidget()->GetMenu()->AddRadioButton("Jacobian",this, "EvaluateMeshQualityApplyCallback 3");
  this->QualityTypeButton->GetWidget()->GetMenu()->AddRadioButton("Skew",this, "EvaluateMeshQualityApplyCallback 4");
  this->QualityTypeButton->GetWidget()->GetMenu()->AddRadioButton("Angle Out of Bounds",this, "EvaluateMeshQualityApplyCallback 5");
  this->GetApplication()->Script(
    "pack %s -side top -anchor n -padx 2 -pady 6", 
    this->QualityTypeButton->GetWidgetName());
  
  if (!this->SummaryFrame)
          this->SummaryFrame = vtkKWFrame::New();
  this->SummaryFrame->SetParent( this->MainFrame );
  this->SummaryFrame->Create();
  this->SummaryFrame->SetReliefToFlat();
  this->GetApplication()->Script("pack %s -side top -anchor nw -expand n -fill x",
          this->SummaryFrame->GetWidgetName() );    

  vtkKWIcon *summaryIcon = vtkKWIcon::New();
  summaryIcon->SetImage( image_mimxSummaryReport, 
                         image_mimxSummaryReport_width, 
                         image_mimxSummaryReport_height, 
                         image_mimxSummaryReport_pixel_size);

  if (!this->SaveButton)
    this->SaveButton = vtkKWPushButton::New(); 
  this->SaveButton->SetParent(this->SummaryFrame);
  this->SaveButton->Create();
  this->SaveButton->SetImageToIcon( summaryIcon );
  this->SaveButton->SetStateToDisabled();
  this->SaveButton->SetReliefToFlat();
  //this->SaveButton->SetBorderWidth( 2 );
  this->SaveButton->SetCommand(this, "ViewDistortedElemenetsCallback");
  this->SaveButton->SetBalloonHelpString("Quality overview of the mesh for the given metric");
  this->GetApplication()->Script(
          "pack %s -side top -anchor n -expand y -padx 2 -pady 6", 
          this->SaveButton->GetWidgetName());
 
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
        this->DisplayOptionsButton->SetImageToIcon( displayOptionIcon );
        this->DisplayOptionsButton->SetCommand(this, "DisplayOptionsCallback");
        this->DisplayOptionsButton->SetBalloonHelpString("Adjusts the display properties for the quality metric");
        this->DisplayOptionsButton->SetReliefToFlat( ); 
        this->GetApplication()->Script(
                "pack %s -side left -anchor nw -padx 2 -pady 2", 
                this->DisplayOptionsButton->GetWidgetName());
                  
  vtkKWIcon *legendIcon = vtkKWIcon::New();
  legendIcon->SetImage( image_mimxLegend, 
                         image_mimxLegend_width, 
                         image_mimxLegend_height, 
                         image_mimxLegend_pixel_size);
                         
  if (!this->ViewLegendButton)
    this->ViewLegendButton = vtkKWCheckButton::New();
  this->ViewLegendButton->SetParent(this->ViewFrame);
  this->ViewLegendButton->Create();
  this->ViewLegendButton->SetCommand(this, "ViewQualityLegendCallback");
  this->ViewLegendButton->SetImageToIcon( legendIcon );
  this->ViewLegendButton->SetSelectImageToIcon( legendIcon );
  this->ViewLegendButton->SetBalloonHelpString("Toggles display of the legend");
  this->ViewLegendButton->SetBorderWidth( 2 );
  this->ViewLegendButton->SetEnabled( 0 );
  this->GetApplication()->Script(
        "pack %s -side left -anchor nw -padx 2 -pady 2", 
        this->ViewLegendButton->GetWidgetName());
  
  vtkKWIcon *clipPlaneIcon = vtkKWIcon::New();
  clipPlaneIcon->SetImage( image_mimxClipPlane, 
                         image_mimxClipPlane_width, 
                         image_mimxClipPlane_height, 
                         image_mimxClipPlane_pixel_size);
                         
  if(!this->ClippingPlaneButton)        
                this->ClippingPlaneButton = vtkKWCheckButton::New();
        this->ClippingPlaneButton->SetParent(this->ViewFrame);
        this->ClippingPlaneButton->Create();
        this->ClippingPlaneButton->SetBorderWidth(0);
        this->ClippingPlaneButton->SetReliefToGroove();
        this->ClippingPlaneButton->SetEnabled( 0 );
        this->ClippingPlaneButton->SetBorderWidth( 2 );
        this->ClippingPlaneButton->SetImageToIcon( clipPlaneIcon );
  this->ClippingPlaneButton->SetSelectImageToIcon( clipPlaneIcon );
  this->ClippingPlaneButton->SetCommand(this, "ClippingPlaneCallback");
  this->ClippingPlaneButton->SetBalloonHelpString("Activate/deactivate the cutting plane to visualize the mesh cross section");
        this->GetApplication()->Script(
          "pack %s -side left -anchor nw -padx 2 -pady 2", 
                this->ClippingPlaneButton->GetWidgetName());
        
        if(!this->InvertPlaneButton)    
                this->InvertPlaneButton = vtkKWCheckButtonWithLabel::New();
        this->InvertPlaneButton->SetParent(this->ViewFrame);
        this->InvertPlaneButton->Create();
        this->InvertPlaneButton->SetBorderWidth(0);
        this->InvertPlaneButton->SetReliefToGroove();
        this->InvertPlaneButton->SetEnabled( 0 );
        this->InvertPlaneButton->GetWidget()->SetEnabled( 0 );
        this->InvertPlaneButton->SetLabelText( "Invert View" );
        this->InvertPlaneButton->SetBalloonHelpString("Invert the clipping plane widget");
        this->InvertPlaneButton->GetWidget()->SetCommand(this, "InvertPlaneCallback");
        this->GetApplication()->Script(
          "pack %s -side left -anchor w -padx 2 -pady 2", 
                this->InvertPlaneButton->GetWidgetName());
  
  if (!this->ButtonFrame)
    this->ButtonFrame = vtkKWFrame::New();
  this->ButtonFrame->SetParent( this->MainFrame );
  this->ButtonFrame->Create();
  this->GetApplication()->Script("pack %s -side top -anchor nw -expand n -fill x -padx 2 -pady 2",
              this->ButtonFrame->GetWidgetName() );    

  this->ApplyButton->SetParent(this->ButtonFrame);
  this->ApplyButton->Create();
  this->ApplyButton->SetReliefToFlat();

  this->CancelButton->SetParent(this->ButtonFrame);
  this->CancelButton->Create();
  this->CancelButton->SetReliefToFlat();
  this->CancelButton->SetImageToIcon( this->GetMimxMainWindow()->GetCancelButtonIcon() );
  this->CancelButton->SetCommand(this, "EvaluateMeshQualityCancelCallback");
  this->GetApplication()->Script(
    "pack %s -side right -anchor ne -expand y -padx 5 -pady 6", 
    this->CancelButton->GetWidgetName());
    
}

//----------------------------------------------------------------------------
int vtkKWMimxEvaluateMeshQualityGroup::ViewDistortedElemenetsCallback()
{
  char textValue[128];
  
  if (!this->DistortedElementDialog)
  {
    this->DistortedElementDialog = vtkKWTopLevel::New();
    this->DistortedElementDialog->SetApplication( this->GetApplication() );
    this->DistortedElementDialog->Create();
    this->DistortedElementDialog->SetResizable(1, 1);
    this->DistortedElementDialog->SetSize(300, 350);
    this->DistortedElementDialog->ModalOff( );
    this->DistortedElementDialog->SetMasterWindow(this->GetMimxMainWindow());
  }
  sprintf(textValue, "%s Summary", this->qualityName);
  this->DistortedElementDialog->SetTitle( textValue );
    
  if (!this->SummaryReviewFrame)
  {
    this->SummaryReviewFrame = vtkKWFrame::New();
    this->SummaryReviewFrame->SetParent( this->DistortedElementDialog );
    this->SummaryReviewFrame->Create();
    this->GetApplication()->Script("pack %s -side top -anchor nw -expand n -fill x -padx 2 -pady 2",
                this->SummaryReviewFrame->GetWidgetName() );    
        }

 
  if (!this->NumberOfElementsEntry)
  {
    this->NumberOfElementsEntry = vtkKWEntryWithLabel::New();
    this->NumberOfElementsEntry->SetParent( this->SummaryReviewFrame );
    this->NumberOfElementsEntry->Create();
    this->NumberOfElementsEntry->GetWidget()->ReadOnlyOn();
    this->NumberOfElementsEntry->GetWidget()->SetWidth(10);
    this->NumberOfElementsEntry->SetLabelText("# Elements");
    this->NumberOfElementsEntry->SetBalloonHelpString("Total number of elements in the mesh");
    this->GetApplication()->Script(
          "grid %s -row 0 -column 0 -sticky ne -padx 2 -pady 6", 
          this->NumberOfElementsEntry->GetWidgetName());
  }
  sprintf(textValue, "%d", this->NumberOfCells);
  this->NumberOfElementsEntry->GetWidget()->SetValue(textValue);

 if (!this->NumberOfDistortedEntry)
 {
    this->NumberOfDistortedEntry = vtkKWEntryWithLabel::New();
    this->NumberOfDistortedEntry->SetParent( this->SummaryReviewFrame );
    this->NumberOfDistortedEntry->Create();
    this->NumberOfDistortedEntry->GetWidget()->ReadOnlyOn();
    this->NumberOfDistortedEntry->GetWidget()->SetWidth(10);
    this->NumberOfDistortedEntry->SetLabelText("# Distorted");
    this->NumberOfDistortedEntry->SetBalloonHelpString("Number of distorted elements in the mesh"); 
    this->GetApplication()->Script(
      "grid %s -row 0 -column 1 -sticky ne -padx 2 -pady 6", 
    this->NumberOfDistortedEntry->GetWidgetName());
  }
  sprintf(textValue, "%d", static_cast<int>( this->DistortedElementList->GetNumberOfIds() ) );
  this->NumberOfDistortedEntry->GetWidget()->SetValue(textValue);
   
  if (!this->QualityMinimumEntry)
  {
    this->QualityMinimumEntry = vtkKWEntryWithLabel::New();
    this->QualityMinimumEntry->SetParent( this->SummaryReviewFrame );
    this->QualityMinimumEntry->Create();
    this->QualityMinimumEntry->GetWidget()->ReadOnlyOn();
    this->QualityMinimumEntry->GetWidget()->SetWidth(10);
    this->QualityMinimumEntry->SetLabelText("Minimum");
    this->QualityMinimumEntry->SetBalloonHelpString("Quality minimum value");
    this->GetApplication()->Script(
      "grid %s -row 1 -column 0 -sticky ne -padx 2 -pady 6", 
      this->QualityMinimumEntry->GetWidgetName());
  }
  sprintf(textValue, "%6.3f", this->minimumQuality);
  this->QualityMinimumEntry->GetWidget()->SetValue(textValue);
 
  if (!this->QualityMaximumEntry)
  {
    this->QualityMaximumEntry = vtkKWEntryWithLabel::New();
    this->QualityMaximumEntry->SetParent( this->SummaryReviewFrame );
    this->QualityMaximumEntry->Create();
    this->QualityMaximumEntry->GetWidget()->ReadOnlyOn();
    this->QualityMaximumEntry->GetWidget()->SetWidth(10);
    this->QualityMaximumEntry->SetLabelText("Maximum");
    this->QualityMaximumEntry->SetBalloonHelpString("Quality maximum value");  
    this->GetApplication()->Script(
      "grid %s -row 1 -column 1 -sticky ne -padx 2 -pady 6", 
      this->QualityMaximumEntry->GetWidgetName());
  }
  sprintf(textValue, "%6.3f", this->maximumQuality );
  this->QualityMaximumEntry->GetWidget()->SetValue(textValue);
 
  if (!this->QualityAverageEntry)
  {
    this->QualityAverageEntry = vtkKWEntryWithLabel::New();
    this->QualityAverageEntry->SetParent( this->SummaryReviewFrame );
    this->QualityAverageEntry->Create();
    this->QualityAverageEntry->GetWidget()->ReadOnlyOn();
    this->QualityAverageEntry->GetWidget()->SetWidth(10);
    this->QualityAverageEntry->SetLabelText("Average");
    this->QualityAverageEntry->SetBalloonHelpString("Quality average value");  
    this->GetApplication()->Script(
      "grid %s -row 2 -column 0 -sticky ne -padx 2 -pady 6", 
      this->QualityAverageEntry->GetWidgetName());
  }
  sprintf(textValue, "%6.3f", this->averageQuality);
  this->QualityAverageEntry->GetWidget()->SetValue(textValue);
  
  if (!this->QualityVarianceEntry)
  {
    this->QualityVarianceEntry = vtkKWEntryWithLabel::New();
    this->QualityVarianceEntry->SetParent( this->SummaryReviewFrame );
    this->QualityVarianceEntry->Create();
    this->QualityVarianceEntry->GetWidget()->ReadOnlyOn();
    this->QualityVarianceEntry->GetWidget()->SetWidth(10);
    this->QualityVarianceEntry->SetLabelText("Variance");
    this->QualityVarianceEntry->SetBalloonHelpString("Quality variance");
    this->GetApplication()->Script(
      "grid %s -row 2 -column 1 -sticky ne -padx 2 -pady 6", 
      this->QualityVarianceEntry->GetWidgetName());
  }
  sprintf(textValue, "%6.3f", this->varianceQuality );
  this->QualityVarianceEntry->GetWidget()->SetValue(textValue);  

  if (!this->DistoredListLabel)
  {
    this->DistoredListLabel = vtkKWLabel::New();
    this->DistoredListLabel->SetParent( this->DistortedElementDialog );
    this->DistoredListLabel->Create( );
    this->DistoredListLabel->SetText("Distorted Elements");
    this->GetApplication()->Script(
          "pack %s -side top -anchor center -expand n -fill x -padx 2 -pady 2", 
          this->DistoredListLabel->GetWidgetName());
        }
        
  if (!this->DistortedElementsReport)
  {
    this->DistortedElementsReport = vtkKWListBoxWithScrollbars::New();
    this->DistortedElementsReport->SetParent(this->DistortedElementDialog);
    this->DistortedElementsReport->Create();
    this->DistortedElementsReport->SetBorderWidth(2);
    this->DistortedElementsReport->SetReliefToGroove();
    this->DistortedElementsReport->SetPadX(2);
    this->DistortedElementsReport->SetPadY(2);
    this->DistortedElementsReport->SetWidth(80);
    this->DistortedElementsReport->SetHeight(40);
    this->GetApplication()->Script(
      "pack %s -side top -anchor nw -expand n -fill x -padx 2 -pady 2",
      this->DistortedElementsReport->GetWidgetName()); 
  }

  if (!this->DistortedButtonFrame)
  {
    this->DistortedButtonFrame = vtkKWFrame::New();
    this->DistortedButtonFrame->SetParent( this->DistortedElementDialog );
    this->DistortedButtonFrame->Create();
    this->GetApplication()->Script("pack %s -side top -anchor nw -expand n -fill x -padx 2 -pady 6",
                this->DistortedButtonFrame->GetWidgetName() );    
  }

  if (!this->SaveDistortedButton)
  {
    vtkKWIcon *saveIcon = vtkKWIcon::New();
    saveIcon->SetImage( image_mimxSave, 
                         image_mimxSave_width, 
                         image_mimxSave_height, 
                         image_mimxSave_pixel_size);
                         
    this->SaveDistortedButton = vtkKWPushButton::New();
    this->SaveDistortedButton->SetParent( this->DistortedButtonFrame );
    this->SaveDistortedButton->Create();
    //this->SaveDistortedButton->SetText("Save");
    this->SaveDistortedButton->SetReliefToFlat();
    this->SaveDistortedButton->SetImageToIcon( saveIcon );
    //this->SaveDistortedButton->SetWidth(10);
    this->SaveDistortedButton->SetCommand(this, "DistortedElementDialogSaveCallback");
    this->GetApplication()->Script("pack %s -side left -anchor nw -expand n -fill x -padx 2 -pady 2",
                this->SaveDistortedButton->GetWidgetName() ); 
  }
 
  if (!this->CancelDistortedButton)
  {
    this->CancelDistortedButton = vtkKWPushButton::New();
    this->CancelDistortedButton->SetParent( this->DistortedButtonFrame );
    this->CancelDistortedButton->Create();
    this->CancelDistortedButton->SetReliefToFlat();
    this->CancelDistortedButton->SetImageToIcon( this->GetMimxMainWindow()->GetCancelButtonIcon() );
    this->CancelDistortedButton->SetCommand(this, "DistortedElementDialogCancelCallback");
    this->GetApplication()->Script("pack %s -side right -anchor ne -padx 2 -pady 2",
                this->CancelDistortedButton->GetWidgetName() ); 
  }
  this->DistortedElementsReport->GetWidget()->DeleteAll();

  char formatstring[80];
  
  for (int i=0;i<this->DistortedElementList->GetNumberOfIds();i++)
  {
    vtkIdType thisId = this->DistortedElementList->GetId(i);
    double thisQ = this->DistortedMeshQuality->GetValue(i);
    sprintf(formatstring, "ElementID: %06d, quality: %6.3f", static_cast<int>(thisId), thisQ);
    this->DistortedElementsReport->GetWidget()->Append(formatstring);
  }
  
  this->DistortedElementDialog->Display();
  return 1;
}

//----------------------------------------------------------------------------
int vtkKWMimxEvaluateMeshQualityGroup::DistortedElementDialogCancelCallback()
{
  this->DistortedElementDialog->Withdraw();
  return 1;
}

//----------------------------------------------------------------------------
int vtkKWMimxEvaluateMeshQualityGroup::DistortedElementDialogSaveCallback()
{

  if(!this->FileBrowserDialog)
        {
                this->FileBrowserDialog = vtkKWLoadSaveDialog::New() ;
                this->FileBrowserDialog->SaveDialogOn();
                this->FileBrowserDialog->SetApplication(this->GetApplication());
                this->FileBrowserDialog->Create();
                this->FileBrowserDialog->SetTitle ("Save Distorted Elements");
                this->FileBrowserDialog->SetFileTypes ("{{CSV files} {.csv}}");
                this->FileBrowserDialog->SetDefaultExtension (".csv");
        }
        this->FileBrowserDialog->Invoke();
        if(this->FileBrowserDialog->GetStatus() == vtkKWDialog::StatusOK)
        {
                if(this->FileBrowserDialog->GetFileName())
                {
                        const char *filename = this->FileBrowserDialog->GetFileName();
                        ofstream outFile;
      outFile.open(filename);
      if (outFile.fail()) 
      {
        vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
        callback->ErrorMessage("Unable to open the specified file. Check permissions.");
        return 0;
      }
      char formatstring[80];
      sprintf(formatstring, "Metric: %s", this->qualityName);
      outFile << formatstring << std::endl;
      sprintf(formatstring, "Elements: %d, Distorted: %d", this->NumberOfCells, static_cast<int>(this->DistortedElementList->GetNumberOfIds()));
      outFile << formatstring << std::endl;
      sprintf(formatstring, "Minimum: %6.3f", this->minimumQuality);
      outFile << formatstring << std::endl;
      sprintf(formatstring, "Maximum: %6.3f", this->maximumQuality);
      outFile << formatstring << std::endl;
      sprintf(formatstring, "Average: %6.3f", this->averageQuality);
      outFile << formatstring << std::endl;
      sprintf(formatstring, "Variance: %6.3f", this->varianceQuality);
      outFile << formatstring << std::endl;
      
      for (int i=0;i<this->DistortedElementList->GetNumberOfIds();i++)
      {
        vtkIdType thisId = this->DistortedElementList->GetId(i);
        double thisQ = this->DistortedMeshQuality->GetValue(i);
        sprintf(formatstring, "ElementID: %06d, quality: %6.3f", static_cast<int>(thisId), thisQ);
        outFile << formatstring << std::endl;
      }
      outFile.close();
                        
                        return 1;
                }
        }
  
  return 0;
}


//----------------------------------------------------------------------------
int vtkKWMimxEvaluateMeshQualityGroup::EvaluateMeshQualityCancelCallback()
{
  cout << "got to EvaluateMeshQualityCancelCallback " << endl;
  cout << "meshname:(" << this->meshName << "), length=" << strlen(this->meshName) << endl;
  cout << "" << endl;
  
  this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName());
  this->MenuGroup->SetMenuButtonsEnabled(1);
  this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(1);
  
  vtkKWComboBox *combobox = this->MeshListComboBox->GetWidget();
  
  // clear out the scalar visibility options and restore the GUI to the mode outside
  // of the quality rendering.  This logic is gated to prevent crashes in the case there
  // is not mesh in the pipeline.
  
  if ( strlen(this->meshName) )
  {    
    vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(
             this->FEMeshList->GetItem(combobox->GetValueIndex( this->meshName )));     
    meshActor->SetMeshScalarVisibility(false);
    meshActor->SetAllElementSetScalarVisibility(false);
    meshActor->SetMeshLegendVisibility(false);
    meshActor->DisableMeshCuttingPlane( );
  }
  
  this->GetMimxMainWindow()->GetRenderWidget()->Render(); 
 
  if(this->ViewOptionsGroup)
          this->ViewOptionsGroup->Withdraw();
   
  return 1;
}

//----------------------------------------------------------------------------
int vtkKWMimxEvaluateMeshQualityGroup::EvaluateMeshQualityApplyCallback(int qualityType)
{
  this->QualityType = qualityType;
  
  vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
  
  if(!strcmp(this->MeshListComboBox->GetWidget()->GetValue(),""))
  {
        callback->ErrorMessage("Please select a mesh before evaluating quality");
        return 0;
  }
  
  vtkKWComboBox *combobox = this->MeshListComboBox->GetWidget();
  strcpy(this->meshName, combobox->GetValue());
  int num = combobox->GetValueIndex( this->meshName );
  if(num < 0 || num > combobox->GetNumberOfValues()-1)
  {
    callback->ErrorMessage("Invalid mesh selected for mesh evaluation");
    combobox->SetValue("");
    return 0;
  }

  vtkUnstructuredGrid *ugrid = vtkMimxMeshActor::SafeDownCast(
           this->FEMeshList->GetItem(combobox->GetValueIndex(this->meshName)))->GetDataSet();
  
  vtkMeshQualityExtended *meshQualityFilter = vtkMeshQualityExtended::New();
  meshQualityFilter->SetInput( ugrid );
  
  int meshType = VTK_HEXAHEDRON;
  if (ugrid->GetNumberOfCells() > 0 )
    meshType = ugrid->GetCellType( 0 );
  
  double minMetricValue;
  double maxMetricValue;  
  
  std::string metricFieldName;
  switch ( this->QualityType )
  {
    case MESH_QUALITY_VOLUME: 
      if (meshType == VTK_HEXAHEDRON)
      {
        meshQualityFilter->SetHexQualityMeasureToVolume( ); 
        metricFieldName = "Volume";
        minMetricValue = 0.0;
        maxMetricValue = 10e20;
      }
      else if (meshType == VTK_TETRA)
      {
        meshQualityFilter->SetTetQualityMeasureToVolume( ); 
        metricFieldName = "Volume";
        minMetricValue = 0.0;
        maxMetricValue = 10e20;
      }
      else if (meshType == VTK_QUAD)
      {
        meshQualityFilter->SetQuadQualityMeasureToArea( ); 
        metricFieldName = "Area";
        minMetricValue = 0.0;
        maxMetricValue = 10e20;
      } 
      else
      {
        meshQualityFilter->SetTriangleQualityMeasureToArea( );
        metricFieldName = "Area";
        minMetricValue = 0.0;
        maxMetricValue = 10e20;
      }
      break;
    case MESH_QUALITY_EDGE:
      if (meshType == VTK_HEXAHEDRON)
      {
        meshQualityFilter->SetHexQualityMeasureToEdgeCollapse( ); 
        metricFieldName = "Edge Collapse";
        minMetricValue = 0;
        maxMetricValue = 2;
      }
      else if (meshType == VTK_TETRA)
      {
        meshQualityFilter->SetTetQualityMeasureToEdgeCollapse( ); 
        metricFieldName = "Edge Collapse";
        minMetricValue = 0;
        maxMetricValue = 2;
      }
      else if (meshType == VTK_QUAD)
      {
        meshQualityFilter->SetQuadQualityMeasureToEdgeCollapse( ); 
        metricFieldName = "Edge Collapse";
        minMetricValue = 0;
        maxMetricValue = 2;
      } 
      else
      {
        meshQualityFilter->SetTriangleQualityMeasureToEdgeCollapse( );
        metricFieldName = "Edge Collapse";
        minMetricValue = 0;
        maxMetricValue = 2;
      }
      break;
    case MESH_QUALITY_JACOBIAN:
      if (meshType == VTK_HEXAHEDRON)
      {
        meshQualityFilter->SetHexQualityMeasureToJacobian( );
        metricFieldName = "Jacobian";
        minMetricValue = 0.0;
        maxMetricValue = 10e20; 
      }
      else if (meshType == VTK_TETRA)
      {
        meshQualityFilter->SetTetQualityMeasureToJacobian( ); 
        metricFieldName = "Jacobian";
        minMetricValue = 0.0;
        maxMetricValue = 10e20;
      }
      else if (meshType == VTK_QUAD)
      {
        meshQualityFilter->SetQuadQualityMeasureToJacobian( ); 
        metricFieldName = "Jacobian";
        minMetricValue = 0.0;
        maxMetricValue = 10e20;
      } 
      else
      {
        meshQualityFilter->SetTriangleQualityMeasureToScaledJacobian( );
        metricFieldName = "Jacobian";
        minMetricValue = 0.0;
        maxMetricValue = 10e20;
      }
      break;
    case MESH_QUALITY_SKEW: 
      meshQualityFilter->SetHexQualityMeasureToSkew( );
      if (meshType == VTK_HEXAHEDRON)
      {
        meshQualityFilter->SetHexQualityMeasureToSkew( ); 
        metricFieldName = "Skew";
        minMetricValue = 0.0;
        maxMetricValue = 0.5;
      }
      else if (meshType == VTK_TETRA)
      {
        meshQualityFilter->SetTetQualityMeasureToEdgeRatio( ); 
        metricFieldName = "Edge Ratio";
        minMetricValue = 1.0;
        maxMetricValue = 3.0;
      }
      else if (meshType == VTK_QUAD)
      {
        meshQualityFilter->SetQuadQualityMeasureToSkew( );
        metricFieldName = "Skew"; 
        minMetricValue = 0.5;
        maxMetricValue = 1.0;
      } 
      else
      {
        meshQualityFilter->SetTriangleQualityMeasureToEdgeRatio( );
        metricFieldName = "Edge Ratio";
        minMetricValue = 1.0;
        maxMetricValue = 1.3;
      }
      break;
    case MESH_QUALITY_ANGLE: 
      if (meshType == VTK_HEXAHEDRON)
      {
        meshQualityFilter->SetHexQualityMeasureToAngleOutOfBounds( ); 
        metricFieldName = "Angle Out of Bounds";
        minMetricValue = 0;
        maxMetricValue = 2;
      }
      else if (meshType == VTK_TETRA)
      {
        meshQualityFilter->SetTetQualityMeasureToAngleOutOfBounds( ); 
        metricFieldName = "Angle Out of Bounds";
        minMetricValue = 0;
        maxMetricValue = 2;
      }
      else if (meshType == VTK_QUAD)
      {
        meshQualityFilter->SetQuadQualityMeasureToAngleOutOfBounds( ); 
        metricFieldName = "Angle Out of Bounds";
        minMetricValue = 0;
        maxMetricValue = 2;
      } 
      else
      {
        meshQualityFilter->SetTriangleQualityMeasureToAngleOutOfBounds( );
        metricFieldName = "Angle Out of Bounds";
        minMetricValue = 0;
        maxMetricValue = 2;
      }
      break;
    case MESH_QUALITY_MAX_ANGLE:
          if (meshType == VTK_QUAD)
      {
        meshQualityFilter->SetQuadQualityMeasureToMaxAngle( ); 
        metricFieldName = "Max Angle";
        minMetricValue = 35;
        maxMetricValue = 145;
      } 
      else
      {
        meshQualityFilter->SetTriangleQualityMeasureToMaxAngle( );
        metricFieldName = "Max Angle";
        minMetricValue = 35;
        maxMetricValue = 145;
      }
          break;
        case MESH_QUALITY_MIN_ANGLE:
          if (meshType == VTK_QUAD)
      {
        meshQualityFilter->SetQuadQualityMeasureToMinAngle( ); 
        metricFieldName = "Min Angle";
        minMetricValue = 35;
        maxMetricValue = 145;
      } 
      else
      {
        meshQualityFilter->SetTriangleQualityMeasureToMinAngle( );
        metricFieldName = "Min Angle";
        minMetricValue = 35;
        maxMetricValue = 145;
      }
          break;
        default:
      callback->ErrorMessage("Invalid Metric Specified");
      return 0;
  }
  meshQualityFilter->Update( );
  
  strcpy(this->qualityName, metricFieldName.c_str());
  
  std::string fieldDataArrayName;
  switch ( meshType )
  {
    case VTK_HEXAHEDRON:
      fieldDataArrayName = "Mesh Hexahedron Quality";
      break;
    case VTK_TETRA:
      fieldDataArrayName = "Mesh Tetrahedron Quality";
      break;
    case VTK_QUAD:
      fieldDataArrayName = "Mesh Quadrilateral Quality";
      break;
    case VTK_TRIANGLE:
      fieldDataArrayName = "Mesh Triangle Quality";
      break;
  }
  
  this->minimumQuality = meshQualityFilter->GetOutput()->GetFieldData()->
        GetArray( fieldDataArrayName.c_str() )->GetComponent( 0, 0 );   
  this->maximumQuality = meshQualityFilter->GetOutput()->GetFieldData()->
        GetArray( fieldDataArrayName.c_str() )->GetComponent( 0, 2 );
  this->averageQuality = meshQualityFilter->GetOutput()->GetFieldData()->
         GetArray( fieldDataArrayName.c_str() )->GetComponent( 0, 1 );
  this->varianceQuality = meshQualityFilter->GetOutput()->GetFieldData()->
         GetArray( fieldDataArrayName.c_str() )->GetComponent( 0, 3 );
  
  int badValueCount = 0;
  vtkUnstructuredGrid *mesh = (vtkUnstructuredGrid*) meshQualityFilter->GetOutput();
  this->DistortedElementList->Initialize();
  this->DistortedMeshQuality->Initialize();
  this->NumberOfCells = ugrid->GetNumberOfCells();
  
  for (int i=0; i< ugrid->GetNumberOfCells() ; i++) 
  {
    double thisQ = ((vtkDoubleArray*)(mesh->GetCellData())->GetArray("Quality"))->GetValue(i);

    // negative value is significant, so lets start snooping on values
    if ( (thisQ < minMetricValue) || (thisQ > maxMetricValue) )
    {
        badValueCount++;

      int thisID;
      if (mesh->GetCellData()->GetArray("Element_Numbers") == NULL)
      {
        thisID = i;
      }
      else
      {
        thisID = ((vtkIntArray*)mesh->GetCellData()->GetArray("Element_Numbers"))->GetValue(i);
      }  
      
      this->DistortedElementList->InsertNextId( thisID );
      this->DistortedMeshQuality->InsertNextValue( thisQ );
    }
  } 
  
  
  vtkDoubleArray *qualityArray = (vtkDoubleArray*)(mesh->GetCellData())->GetArray("Quality");
  qualityArray->SetName( metricFieldName.c_str() );
  ugrid->GetCellData()->RemoveArray( metricFieldName.c_str() );
  ugrid->GetCellData()->AddArray( qualityArray );

  
  meshQualityFilter->Delete();
  
  vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(
           this->FEMeshList->GetItem(combobox->GetValueIndex(this->meshName)));
           
  meshActor->SetLegendTextColor( this->GetMimxMainWindow()->GetTextColor() );
  if (this->QualityType == MESH_QUALITY_MAX_ANGLE)
    meshActor->SetColorRangeType(vtkMimxMeshActor::BlueToRed, qualityName, NULL, meshActor->ComputeMeshScalarRange(qualityName));
  else
    meshActor->SetColorRangeType(vtkMimxMeshActor::RedToBlue, qualityName, NULL, meshActor->ComputeMeshScalarRange(qualityName));
  meshActor->SetLegendTitle( qualityName );
  meshActor->SetMeshScalarVisibility(true);
  if(this->ViewOptionsGroup)
  {
        this->ViewOptionsGroup->ResetValues();
        this->ViewOptionsGroup->SetArrayName(qualityName);
  }
 
  this->GetMimxMainWindow()->GetRenderWidget()->Render();
  
  this->SaveButton->SetEnabled( 1 );
  this->ClippingPlaneButton->SetEnabled( 1 );
  this->ViewLegendButton->SetEnabled( 1 );
  this->InvertPlaneButton->SetEnabled( 0 );
  this->InvertPlaneButton->GetWidget()->SetEnabled( 0 );
  this->ViewLegendButton->Select( );
  this->DisplayOptionsButton->SetEnabled( 1 );
             
  this->GetMimxMainWindow()->SetStatusText("Evaluated Mesh Quality");
  
  return 1;      
}

//----------------------------------------------------------------------------
void vtkKWMimxEvaluateMeshQualityGroup::Update()
{
        this->UpdateEnableState();
}

//---------------------------------------------------------------------------
void vtkKWMimxEvaluateMeshQualityGroup::UpdateEnableState()
{
        this->UpdateObjectLists();
        this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
void vtkKWMimxEvaluateMeshQualityGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//------------------------------------------------------------------------------
void vtkKWMimxEvaluateMeshQualityGroup::UpdateObjectLists()
{ 
  int item = 
     this->UpdateMeshComboBox( this->MeshListComboBox->GetWidget() );
  /*
  this->MeshListComboBox->GetWidget()->DeleteAllValues();
  
  int defaultItem = -1;
  for (int i = 0; i < this->FEMeshList->GetNumberOfItems(); i++)
  {
    this->MeshListComboBox->GetWidget()->AddValue(
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
  */  
  if (item != -1)
  {
    //this->MeshListComboBox->GetWidget()->SetValue(
    //      this->FEMeshList->GetItem(defaultItem)->GetFileName());
    strcpy(this->PreviousSelection, this->FEMeshList->GetItem(item)->GetFileName());
    this->SelectionChangedCallback(this->FEMeshList->GetItem(item)->GetFileName());
    this->QualityTypeButton->GetWidget()->SetStateToNormal();
  }
  else
  {
    this->QualityTypeButton->GetWidget()->SetStateToDisabled();
  }
  
}

//------------------------------------------------------------------------------
void vtkKWMimxEvaluateMeshQualityGroup::ClearStatsEntry()
{ 
  this->ViewLegendButton->SetEnabled( 0 );
  this->ClippingPlaneButton->SetEnabled( 0 );
  this->SaveButton->SetEnabled( 0 );
  this->InvertPlaneButton->SetEnabled( 0 );
  this->InvertPlaneButton->GetWidget()->SetEnabled( 0 );
  this->ViewLegendButton->Deselect( );
  this->ClippingPlaneButton->Deselect( );
  this->InvertPlaneButton->GetWidget()->Deselect( );
  this->QualityTypeButton->GetWidget()->SetValue("");
  this->DisplayOptionsButton->SetEnabled( 0 );
}

//------------------------------------------------------------------------------
int vtkKWMimxEvaluateMeshQualityGroup::ClippingPlaneCallback(int mode)
{
  vtkKWComboBox *combobox = this->MeshListComboBox->GetWidget();
  vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(
           this->FEMeshList->GetItem(combobox->GetValueIndex( this->meshName )));
  
  if ( meshActor == NULL )
    return 1;
      
  if ( mode )
  {
    meshActor->EnableMeshCuttingPlane();
    if ( this->InvertPlaneButton->GetWidget()->GetSelectedState() )
      meshActor->SetInvertCuttingPlane( true );
    else
      meshActor->SetInvertCuttingPlane( false );
    this->InvertPlaneButton->SetEnabled( 1 );
    this->InvertPlaneButton->GetWidget()->SetEnabled( 1 );
  }
  else 
  {
    meshActor->DisableMeshCuttingPlane();
    this->InvertPlaneButton->GetWidget()->SetState( 0 );
    this->InvertPlaneButton->SetEnabled( 0 );
  }
  
  this->GetMimxMainWindow()->GetRenderWidget()->Render(); 
  return 1;
}

//------------------------------------------------------------------------------
int vtkKWMimxEvaluateMeshQualityGroup::InvertPlaneCallback(int mode)
{
  vtkKWComboBox *combobox = this->MeshListComboBox->GetWidget();
  vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(
           this->FEMeshList->GetItem(combobox->GetValueIndex( this->meshName )));
  if(!meshActor)        return 0;
           
  meshActor->SetInvertCuttingPlane( static_cast<bool>(mode) );

  this->GetMimxMainWindow()->GetRenderWidget()->Render(); 
  return 1;
}
//------------------------------------------------------------------------------
int vtkKWMimxEvaluateMeshQualityGroup::ViewQualityLegendCallback(int mode)
{
  vtkKWComboBox *combobox = this->MeshListComboBox->GetWidget();
  
  if ( strlen(this->meshName) )
  {
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
  return 1;
}
//------------------------------------------------------------------------------
void vtkKWMimxEvaluateMeshQualityGroup::SelectionChangedCallback(const char* selection)
{
  
  if ( (strcmp(selection,"") != 0) && (strcmp(selection, this->PreviousSelection) != 0) )
        {
          vtkKWComboBox *combobox = this->MeshListComboBox->GetWidget();
          vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(
             this->FEMeshList->GetItem(combobox->GetValueIndex( this->PreviousSelection )));     
    meshActor->SetMeshScalarVisibility(false);
    meshActor->SetMeshLegendVisibility(false);
    meshActor->DisableMeshCuttingPlane( );
    this->ClearStatsEntry();
          strcpy(this->PreviousSelection, selection);
          if(this->ViewOptionsGroup)
          {
                  this->ViewOptionsGroup->ResetValues();
                  this->ViewOptionsGroup->SetMeshActor(meshActor);
          }
          meshActor->HideMesh();
    this->GetMimxMainWindow()->GetViewProperties()->UpdateVisibility();     
        }
        
        if (strcmp(selection,"") != 0)
        {
        vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(
            this->FEMeshList->GetItem(this->MeshListComboBox->GetWidget()->GetValueIndex(selection)));   
        vtkUnstructuredGrid *ugrid = meshActor->GetDataSet();
    meshActor->ShowMesh();
    this->GetMimxMainWindow()->GetViewProperties()->UpdateVisibility();     
    
    int meshType = VTK_HEXAHEDRON;
    if (ugrid->GetNumberOfCells() > 0 )
      meshType = ugrid->GetCellType( 0 );
    
    this->QualityTypeButton->GetWidget()->GetMenu()->DeleteAllItems();
    switch (meshType)
    {
      case VTK_HEXAHEDRON:
        this->QualityTypeButton->GetWidget()->GetMenu()->AddRadioButton("Volume",this, "EvaluateMeshQualityApplyCallback 1");
        this->QualityTypeButton->GetWidget()->GetMenu()->AddRadioButton("Edge Collapse",this, "EvaluateMeshQualityApplyCallback 2");
        this->QualityTypeButton->GetWidget()->GetMenu()->AddRadioButton("Jacobian",this, "EvaluateMeshQualityApplyCallback 3");
        this->QualityTypeButton->GetWidget()->GetMenu()->AddRadioButton("Skew",this, "EvaluateMeshQualityApplyCallback 4");
        this->QualityTypeButton->GetWidget()->GetMenu()->AddRadioButton("Angle Out of Bounds",this, "EvaluateMeshQualityApplyCallback 5");
        break;
      case VTK_TETRA:
        this->QualityTypeButton->GetWidget()->GetMenu()->AddRadioButton("Volume",this, "EvaluateMeshQualityApplyCallback 1");
        this->QualityTypeButton->GetWidget()->GetMenu()->AddRadioButton("Edge Collapse",this, "EvaluateMeshQualityApplyCallback 2");
        this->QualityTypeButton->GetWidget()->GetMenu()->AddRadioButton("Jacobian",this, "EvaluateMeshQualityApplyCallback 3");
        this->QualityTypeButton->GetWidget()->GetMenu()->AddRadioButton("Edge Ratio",this, "EvaluateMeshQualityApplyCallback 4");
        this->QualityTypeButton->GetWidget()->GetMenu()->AddRadioButton("Angle Out of Bounds",this, "EvaluateMeshQualityApplyCallback 5");
        break;
      case VTK_QUAD:
        this->QualityTypeButton->GetWidget()->GetMenu()->AddRadioButton("Area",this, "EvaluateMeshQualityApplyCallback 1");
        this->QualityTypeButton->GetWidget()->GetMenu()->AddRadioButton("Edge Collapse",this, "EvaluateMeshQualityApplyCallback 2");
        this->QualityTypeButton->GetWidget()->GetMenu()->AddRadioButton("Jacobian",this, "EvaluateMeshQualityApplyCallback 3");
        this->QualityTypeButton->GetWidget()->GetMenu()->AddRadioButton("Skew",this, "EvaluateMeshQualityApplyCallback 4");
        this->QualityTypeButton->GetWidget()->GetMenu()->AddRadioButton("Min Angle",this, "EvaluateMeshQualityApplyCallback 6");
                this->QualityTypeButton->GetWidget()->GetMenu()->AddRadioButton("Max Angle",this, "EvaluateMeshQualityApplyCallback 7");
        break;
      case VTK_TRIANGLE:
        this->QualityTypeButton->GetWidget()->GetMenu()->AddRadioButton("Area",this, "EvaluateMeshQualityApplyCallback 1");
        this->QualityTypeButton->GetWidget()->GetMenu()->AddRadioButton("Edge Collapse",this, "EvaluateMeshQualityApplyCallback 2");
        this->QualityTypeButton->GetWidget()->GetMenu()->AddRadioButton("Jacobian",this, "EvaluateMeshQualityApplyCallback 3");
        this->QualityTypeButton->GetWidget()->GetMenu()->AddRadioButton("Edge Ratio",this, "EvaluateMeshQualityApplyCallback 4");
        this->QualityTypeButton->GetWidget()->GetMenu()->AddRadioButton("Min Angle",this, "EvaluateMeshQualityApplyCallback 6");
                this->QualityTypeButton->GetWidget()->GetMenu()->AddRadioButton("Max Angle",this, "EvaluateMeshQualityApplyCallback 7");
        break;
    }
  
  } 
}
//--------------------------------------------------------------------------------
void vtkKWMimxEvaluateMeshQualityGroup::DisplayOptionsCallback( )
{
        //vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();

        vtkKWComboBox *combobox = this->MeshListComboBox->GetWidget();
        const char *name = combobox->GetValue();
        vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(
                this->FEMeshList->GetItem(combobox->GetValueIndex(name)));

        const char *elsetName = NULL;


        if (!this->ViewOptionsGroup)
        {
                this->ViewOptionsGroup = vtkKWMimxViewPropertiesOptionGroup::New();
                this->ViewOptionsGroup->SetApplication( this->GetApplication() );
                this->ViewOptionsGroup->SetMimxMainWindow( this->GetMimxMainWindow() );
                this->ViewOptionsGroup->SetMeshActor(meshActor);
                this->ViewOptionsGroup->SetElementSetName(elsetName);
                this->ViewOptionsGroup->SetArrayName(qualityName);
                this->ViewOptionsGroup->Create();
        }
        else
        {
                this->ViewOptionsGroup->SetMeshActor(meshActor);
                this->ViewOptionsGroup->SetElementSetName( elsetName );
                this->ViewOptionsGroup->SetArrayName(qualityName);
        }

        this->ViewOptionsGroup->Display();
        this->ViewOptionsGroup->ResetValues();

}
//--------------------------------------------------------------------------------------------
