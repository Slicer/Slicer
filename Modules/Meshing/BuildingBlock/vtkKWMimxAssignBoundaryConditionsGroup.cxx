/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxAssignBoundaryConditionsGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.58.2.2 $
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

#include "vtkKWMimxAssignBoundaryConditionsGroup.h"

#include "vtkActor.h"
#include "vtkActorCollection.h"
#include "vtkArrowSource.h"
#include "vtkCellData.h"
#include "vtkCollection.h"
#include "vtkConeSource.h"
#include "vtkDoubleArray.h"
#include "vtkFieldData.h"
#include "vtkFloatArray.h"
#include "vtkGlyph3D.h"
#include "vtkIntArray.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPointSet.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkStringArray.h"
#include "vtkUnstructuredGrid.h"


#include "vtkKWApplication.h"
#include "vtkKWCheckButton.h"
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWComboBox.h"
#include "vtkKWComboBoxWithLabel.h"
#include "vtkKWEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWEvent.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWIcon.h"
#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWNotebook.h"
#include "vtkKWOptions.h"
#include "vtkKWPushButton.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWScale.h"
#include "vtkKWSpinBox.h"
#include "vtkKWSpinBoxWithLabel.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWTopLevel.h"
#include "vtkKWTree.h"
#include "vtkKWTreeWithScrollbars.h"
#include "vtkKWMultiColumnList.h"

#include "vtkLinkedListWrapper.h"
#include "vtkMimxBoundaryConditionActor.h"
#include "vtkMimxErrorCallback.h"
#include "vtkMimxMeshActor.h"
#include "vtkMimxUnstructuredGridActor.h"

#include "vtkKWMimxDefineNodeSetGroup.h"
#include "vtkKWMimxMainMenuGroup.h"
#include "vtkKWMimxMainNotebook.h"
#include "vtkKWMimxMainUserInterfacePanel.h"
#include "vtkKWMimxMainWindow.h"

#include <vtksys/stl/algorithm>
#include <vtksys/stl/list>
#include <vtksys/stl/string>

#include "Resources/mimxAddStep.h"
#include "Resources/mimxBCsummary.h"
#include "Resources/mimxDeleteStep.h"
#include "Resources/mimxDisplayBC.h"
#include "Resources/mimxNodeSetDfns.h"
#include "Resources/mimxSmallApply.h"
#include "Resources/mimxStepDef.h"

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD    1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxAssignBoundaryConditionsGroup);
vtkCxxRevisionMacro(vtkKWMimxAssignBoundaryConditionsGroup, "$Revision: 1.58.2.2 $");

//----------------------------------------------------------------------------
vtkKWMimxAssignBoundaryConditionsGroup::vtkKWMimxAssignBoundaryConditionsGroup()
{
  this->ComponentFrame = NULL;
  this->ObjectListComboBox = NULL;
  this->DefineNodeSetPushButton = NULL;
  this->StepNotebook = NULL;
  this->NodeSetComboBox = NULL;
  this->BoundaryConditionTypeComboBox = NULL;
  this->StepFrame = NULL;
  this->StepNumberComboBox = NULL;
  this->AddStepPushButton = NULL;
  this->DeleteStepPushButton = NULL;
  this->DirectionFrame = NULL;
  this->DirectionXEntry = NULL;
  this->DirectionYEntry = NULL;
  this->DirectionZEntry = NULL;
  this->ViewBoundaryConditionsButton = NULL;
  this->CancelStatus = 0;
  this->ViewFrame = NULL;
  this->NodeSetNameArray = vtkStringArray::New();
  this->BoundaryConditionTypeArray = vtkStringArray::New();
  this->GlyphFrame = NULL;
  this->MultiColumnList = NULL;
  this->BoundaryConditionActor = vtkMimxBoundaryConditionActor::New();
  this->OpacityThumbwheel = NULL;
  this->ActorCollection = vtkCollection::New();
  this->GlyphCollection = vtkCollection::New();
  //
  this->GlyphSizeEntry = NULL;
  this->DefineNodeSetDialog = NULL;
  this->GlyphSize = 1.0;
  this->SummaryButton = NULL;
  this->SummaryDialog = NULL;
  this->BCTree = NULL;
  this->CancelDialogButton = NULL;
  this->MeshOpacityScale = NULL;
  this->NodeSetLabel = NULL;
  this->InfoFrame = NULL;
  strcpy(this->nodeSetSelectionPrevious, "");
  strcpy(this->stepNumPrevious, "");
  strcpy(this->boundaryConditionTypePrevious, "");
  this->sizePrevious = 1.0;
  this->startNotebookPage = 1;
  this->endNotebookPage = 1;
  this->currentNotebookPage = 1;
  this->factorX = 1.0;
  this->factorY = 1.0;
  this->factorZ = 1.0;
  this->BoundaryConditionArray = vtkStringArray::New();
  this->BoundaryConditionArray->InsertNextValue("Force");
  this->BoundaryConditionArray->InsertNextValue("Displacement");
  this->BoundaryConditionArray->InsertNextValue("Rotation");
  this->BoundaryConditionArray->InsertNextValue("Moment");  
  
  this->StepControlDialog = NULL;
  this->StepParameterLabel = NULL;
  this->StepParameterFrame = NULL;
  this->StepIncrementEntry = NULL;
  this->NlGeomButton = NULL;
  this->UnsymmButton = NULL;
  this->AnalysisTypeMenu = NULL;
  this->TimeIncrementEntry = NULL;
  this->NodePrintButton = NULL;
  this->NodePrintFrame = NULL;
  this->NodeSetMenu = NULL;
  this->NodePrintFrequencyEntry = NULL;
  this->NodePrintSummaryButton = NULL;
  this->NodePrintSummaryButton = NULL;
  this->ElementPrintButton = NULL;
  this->ElementSetMenu = NULL;
  this->ElementPrintFrequencyEntry = NULL;
  this->ElementPrintPositionMenu = NULL;
  this->ElementPrintSummaryButton = NULL;
  this->ElementPrintTotalButton = NULL;
  this->NodeOutputButton = NULL;
  this->ElementOutputButton = NULL;
  this->ElementOutputPositionMenu = NULL;
  this->StepControlApplyButton = NULL;
  this->StepControlCancelButton = NULL;
  this->ElementPositionSetMenu = NULL;
  this->DefineControlPushButton = NULL;
  this->NodePrintVariablesEntry = NULL;
  this->NodeOutputVariablesEntry = NULL;
  this->ElementPrintVariablesEntry = NULL;
  this->ElementOutputVariablesEntry = NULL;
  this->NodePrintTotalButton = NULL;
  this->ElementPrintFrame = NULL;
  this->SubHeadingLabel = NULL;
  this->SubHeadingEntry = NULL;
  this->SubHeadingFrame = NULL;
  this->NodeSetOutputApplyButton = NULL;
  this->ElementSetOutputApplyButton = NULL;
  this->ButtonFrame = NULL;
  this->AmplitudeTypeMenu = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxAssignBoundaryConditionsGroup::~vtkKWMimxAssignBoundaryConditionsGroup()
{
  if(this->ComponentFrame)
    this->ComponentFrame->Delete();
  if(this->ObjectListComboBox)
    this->ObjectListComboBox->Delete();
  if(this->NodeSetComboBox)
    this->NodeSetComboBox->Delete();
  if(this->BoundaryConditionTypeComboBox)
    this->BoundaryConditionTypeComboBox->Delete();
  if(this->StepFrame)
    this->StepFrame->Delete();
  if(this->StepNumberComboBox)
    this->StepNumberComboBox->Delete();
  if(this->AddStepPushButton)
    this->AddStepPushButton->Delete();
  if(this->DirectionFrame)
    this->DirectionFrame->Delete();
  if(this->DirectionXEntry)
    this->DirectionXEntry->Delete();
  if(this->DirectionYEntry)
    this->DirectionYEntry->Delete();
  if(this->DirectionZEntry)
    this->DirectionZEntry->Delete();  
  if(this->ViewBoundaryConditionsButton)
    this->ViewBoundaryConditionsButton->Delete();
  if(this->ViewFrame)
    this->ViewFrame->Delete();
  if(this->GlyphFrame)
    this->GlyphFrame->Delete();
  if(this->MultiColumnList)
    this->MultiColumnList->Delete();
  if (this->BoundaryConditionActor)
    this->BoundaryConditionActor->Delete();
  if(this->OpacityThumbwheel)
    this->OpacityThumbwheel->Delete();
 
  if(this->GlyphSizeEntry)
    this->GlyphSizeEntry->Delete();
  if (this->DefineNodeSetPushButton)
    this->DefineNodeSetPushButton->Delete();
  if (this->StepNotebook)
    this->StepNotebook->Delete();
  if (this->DefineNodeSetDialog)
    this->DefineNodeSetDialog->Delete();
  if (this->DeleteStepPushButton)
    this->DeleteStepPushButton->Delete();
  if (this->SummaryButton)
    this->SummaryButton->Delete();
  if (this->SummaryDialog)
    this->SummaryDialog->Delete();
  if (this->BCTree)
    this->BCTree->Delete();
  if (this->CancelDialogButton)
    this->CancelDialogButton->Delete();
  if (this->MeshOpacityScale)
    this->MeshOpacityScale->Delete();
  if ( this->NodeSetLabel )
    this->NodeSetLabel->Delete();
  if ( this->InfoFrame )
    this->InfoFrame->Delete();
  if ( this->SubHeadingLabel )
    this->SubHeadingLabel->Delete();
  if ( this->SubHeadingEntry )
    this->SubHeadingEntry->Delete();
  if (this->SubHeadingFrame)
    this->SubHeadingFrame->Delete();
  this->BoundaryConditionActor->Delete();
  this->BoundaryConditionTypeArray->Delete();
  this->NodeSetNameArray->Delete();
  this->BoundaryConditionArray->Delete();
  if ( this->StepControlDialog )
    this->StepControlDialog->Delete();
  if ( this->StepParameterLabel )
    this->StepParameterLabel->Delete();
  if ( this->StepParameterFrame )
    this->StepParameterFrame->Delete();
  if ( this->StepIncrementEntry )
    this->StepIncrementEntry->Delete();
  if ( this->NlGeomButton )
    this->NlGeomButton->Delete();
  if ( this->UnsymmButton )
    this->UnsymmButton->Delete();
  if ( this->AnalysisTypeMenu )
    this->AnalysisTypeMenu->Delete();
  if ( this->TimeIncrementEntry )
    this->TimeIncrementEntry->Delete();
  if ( this->NodePrintButton )
    this->NodePrintButton->Delete();
  if ( this->NodePrintFrame )
    this->NodePrintFrame->Delete();
  if ( this->NodeSetMenu )
    this->NodeSetMenu->Delete();
  if ( this->NodePrintFrequencyEntry )
    this->NodePrintFrequencyEntry->Delete();
  if ( this->NodePrintSummaryButton )
    this->NodePrintSummaryButton->Delete();
  if ( this->NodePrintSummaryButton )
    this->NodePrintSummaryButton->Delete();
  if ( this->ElementPrintButton )
    this->ElementPrintButton->Delete();
  if ( this->ElementSetMenu )
    this->ElementSetMenu->Delete();
  if ( this->ElementPrintFrequencyEntry )
    this->ElementPrintFrequencyEntry->Delete();
  if ( this->ElementPrintPositionMenu )
    this->ElementPrintPositionMenu->Delete();
  if ( this->ElementPrintSummaryButton )
    this->ElementPrintSummaryButton->Delete();
  if ( this->ElementPrintTotalButton )
    this->ElementPrintTotalButton->Delete();
  if ( this->NodeOutputButton )
    this->NodeOutputButton->Delete();
  if ( this->ElementOutputButton )
    this->ElementOutputButton->Delete();
  if ( this->ElementOutputPositionMenu )
    this->ElementOutputPositionMenu->Delete();
  if ( this->StepControlApplyButton )
    this->StepControlApplyButton->Delete();
  if ( this->StepControlCancelButton )
    this->StepControlCancelButton->Delete();
  if ( this->ElementPositionSetMenu )
    this->ElementPositionSetMenu->Delete();
  if ( this->DefineControlPushButton )
    this->DefineControlPushButton->Delete();
  if ( this->NodePrintVariablesEntry )
    this->NodePrintVariablesEntry->Delete();
  if ( this->NodePrintTotalButton )
    this->NodePrintTotalButton->Delete();
  if ( this->ElementPrintFrame )
    this->ElementPrintFrame->Delete();
  if(this->NodeSetOutputApplyButton)
    this->NodeSetOutputApplyButton->Delete();
  if(this->ElementSetOutputApplyButton)
    this->ElementSetOutputApplyButton->Delete();
  if(this->ButtonFrame)
    this->ButtonFrame->Delete();
  if (this->AmplitudeTypeMenu)
    this->AmplitudeTypeMenu->Delete();
  if (this->ActorCollection)
    {
    this->ActorCollection->Delete();
    }
  if (this->GlyphCollection)
    {
    this->GlyphCollection->Delete();
    }

}
//----------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::CreateWidget()
{
  if(this->IsCreated())
    {
    vtkErrorMacro("class already created");
    return;
    }

  this->Superclass::CreateWidget();
  
  this->MainFrame->SetParent(this->GetParent());
  this->MainFrame->Create();
  //this->MainFrame->SetLabelText("Boundary Conditions");

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 0 -fill x", 
    this->MainFrame->GetWidgetName());

  if (!this->ComponentFrame)    
    this->ComponentFrame = vtkKWFrameWithLabel::New();
  this->ComponentFrame->SetParent(this->MainFrame);
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
  ObjectListComboBox->SetLabelWidth(15);
  ObjectListComboBox->GetWidget()->ReadOnlyOn();
  ObjectListComboBox->GetWidget()->SetCommand(this, "SelectionChangedCallback");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
    ObjectListComboBox->GetWidgetName());
  
  vtkKWIcon *defineNodeSetIcon = vtkKWIcon::New();
  defineNodeSetIcon->SetImage(  image_mimxNodeSetDfns, 
                                image_mimxNodeSetDfns_width, 
                                image_mimxNodeSetDfns_height, 
                                image_mimxNodeSetDfns_pixel_size);
  
  
  if (!this->DefineNodeSetPushButton)    
    this->DefineNodeSetPushButton = vtkKWPushButton::New();
  this->DefineNodeSetPushButton->SetParent(this->MainFrame);
  this->DefineNodeSetPushButton->Create();
 
  this->DefineNodeSetPushButton->SetBorderWidth( 2 );
  this->DefineNodeSetPushButton->SetReliefToFlat( );
  this->DefineNodeSetPushButton->SetImageToIcon( defineNodeSetIcon );
  this->DefineNodeSetPushButton->SetCommand(this, "DefineNodeSetCallback");
  this->GetApplication()->Script(
    "pack %s -side top -anchor n -expand y -padx 2 -pady 6",
    this->DefineNodeSetPushButton->GetWidgetName());
  defineNodeSetIcon->Delete();
  this->DefineNodeSetPushButton->SetEnabled( 0 );
    
  if (!this->StepNotebook)
    this->StepNotebook = vtkKWNotebook::New();
  this->StepNotebook->SetParent(this->MainFrame);
  this->StepNotebook->Create();
  this->StepNotebook->SetReliefToFlat();
  this->StepNotebook->AddPage("<");
  this->StepNotebook->AddPage(">");
  this->AddCallbackCommandObserver(
    this->StepNotebook, vtkKWEvent::NotebookRaisePageEvent);
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 0 -fill x", 
    this->StepNotebook->GetWidgetName());
           
  //step details
  if(!this->StepFrame)
    this->StepFrame = vtkKWFrame::New();
  this->StepFrame->SetParent(this->MainFrame);
  this->StepFrame->Create();
  //this->StepFrame->SetLabelText("Step Details");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
    this->StepFrame->GetWidgetName());

  vtkKWIcon *addStepIcon = vtkKWIcon::New();
  addStepIcon->SetImage(  image_mimxAddStep, 
                          image_mimxAddStep_width, 
                          image_mimxAddStep_height, 
                          image_mimxAddStep_pixel_size);
                          
  if (!this->AddStepPushButton)    
    this->AddStepPushButton = vtkKWPushButton::New();
  this->AddStepPushButton->SetParent(this->StepFrame);
  this->AddStepPushButton->Create();
  this->AddStepPushButton->SetImageToIcon( addStepIcon );
  this->AddStepPushButton->SetCommand(this, "AddStepNumberCallback");
  this->AddStepPushButton->SetReliefToFlat( );
  this->AddStepPushButton->SetBalloonHelpString("Add a step");
  this->GetApplication()->Script(
    "pack %s -side right -anchor ne -expand y -padx 2 -pady 2",
    this->AddStepPushButton->GetWidgetName());
  addStepIcon->Delete();

  vtkKWIcon *deleteStepIcon = vtkKWIcon::New();
  deleteStepIcon->SetImage(  image_mimxDeleteStep, 
                             image_mimxDeleteStep_width, 
                             image_mimxDeleteStep_height, 
                             image_mimxDeleteStep_pixel_size);
                          
  if (!this->DeleteStepPushButton)    
    this->DeleteStepPushButton = vtkKWPushButton::New();
  this->DeleteStepPushButton->SetParent(this->StepFrame);
  this->DeleteStepPushButton->Create();
  this->DeleteStepPushButton->SetImageToIcon( deleteStepIcon );
  this->DeleteStepPushButton->SetCommand(this, "DeleteStepNumberCallback");
  this->DeleteStepPushButton->SetReliefToFlat( );
  this->DeleteStepPushButton->SetBalloonHelpString("Remove the selected step");
  this->GetApplication()->Script(
    "pack %s -side left -anchor nw -expand y -padx 2 -pady 2",
    this->DeleteStepPushButton->GetWidgetName());
  deleteStepIcon->Delete();
     
  if (!this->SubHeadingFrame)
    this->SubHeadingFrame = vtkKWFrame::New();
  this->SubHeadingFrame->SetParent(this->MainFrame);
  this->SubHeadingFrame->Create();
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 10 -fill x", 
    this->SubHeadingFrame->GetWidgetName());
     
  if(!this->SubHeadingLabel)    
    this->SubHeadingLabel = vtkKWLabel::New();
  this->SubHeadingLabel->SetParent(this->SubHeadingFrame);
  this->SubHeadingLabel->Create();
  this->SubHeadingLabel->SetText("Subheading : ");
  this->GetApplication()->Script(
    "grid %s -row 0 -column 0 -sticky e -padx 2 -pady 2", 
    this->SubHeadingLabel->GetWidgetName());
    
  
  if(!this->SubHeadingEntry)    
    this->SubHeadingEntry = vtkKWEntry::New();
  this->SubHeadingEntry->SetParent(this->SubHeadingFrame);
  this->SubHeadingEntry->Create();
  this->SubHeadingEntry->SetWidth( 30 );
  this->GetApplication()->Script(
    "grid %s -row 0 -column 1 -sticky w -padx 2 -pady 2", 
    this->SubHeadingEntry->GetWidgetName());
    
  this->GetApplication()->Script(
    "grid columnconfigure %s 0 -weight  1",
    this->SubHeadingFrame->GetWidgetName());
  this->GetApplication()->Script(
    "grid columnconfigure %s 1 -weight  4",
    this->SubHeadingFrame->GetWidgetName());
      
  if (!this->InfoFrame)
    this->InfoFrame = vtkKWFrame::New();
  this->InfoFrame->SetParent(this->MainFrame);
  this->InfoFrame->Create();
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 6 -fill x", 
    this->InfoFrame->GetWidgetName());
     
  // Boundary Condition Type
  if(!this->BoundaryConditionTypeComboBox)    
    this->BoundaryConditionTypeComboBox = vtkKWMenuButton::New();
  this->BoundaryConditionTypeComboBox->SetParent(this->InfoFrame);
  this->BoundaryConditionTypeComboBox->Create();
  this->BoundaryConditionTypeComboBox->SetWidth( 25 );
  this->BoundaryConditionTypeComboBox->GetMenu()->AddRadioButton(
    "Force",this, "BoundaryConditionTypeSelectionChangedCallback Force");
  this->BoundaryConditionTypeComboBox->GetMenu()->AddRadioButton(
    "Displacement",this, "BoundaryConditionTypeSelectionChangedCallback Displacement");        
  this->BoundaryConditionTypeComboBox->GetMenu()->AddRadioButton(
    "Rotation",this, "BoundaryConditionTypeSelectionChangedCallback Rotation");    
  this->BoundaryConditionTypeComboBox->GetMenu()->AddRadioButton(
    "Moment",this, "BoundaryConditionTypeSelectionChangedCallback Moment");
  this->BoundaryConditionTypeComboBox->SetValue("Force");
  this->GetApplication()->Script(
    "grid %s -row 0 -column 1 -sticky w -padx 2 -pady 2", 
    this->BoundaryConditionTypeComboBox->GetWidgetName());
      
  // for Node Set listing
  if(!this->NodeSetLabel)    
    this->NodeSetLabel = vtkKWLabel::New();
  this->NodeSetLabel->SetParent(this->InfoFrame);
  this->NodeSetLabel->Create();
  this->NodeSetLabel->SetText("Node Set : ");
  this->GetApplication()->Script(
    "grid %s -row 1 -column 0 -sticky e -padx 2 -pady 2", 
    NodeSetLabel->GetWidgetName());
      
  if(!this->NodeSetComboBox)    
    this->NodeSetComboBox = vtkKWComboBox::New();
  this->NodeSetComboBox->SetParent(this->InfoFrame);
  this->NodeSetComboBox->Create();
  //this->NodeSetComboBox->SetLabelText("Node Set : ");
  this->NodeSetComboBox->ReadOnlyOn();
  this->NodeSetComboBox->SetWidth( 29 );
  //this->NodeSetComboBox->SetLabelWidth( 10 );
  this->NodeSetComboBox->SetCommand(this, "NodeSetChangedCallback");
  this->GetApplication()->Script(
    "grid %s -row 1 -column 1 -sticky w -padx 2 -pady 2", 
    NodeSetComboBox->GetWidgetName());
  
  this->GetApplication()->Script(
    "grid columnconfigure %s 0 -weight  1",
    this->InfoFrame->GetWidgetName());
  this->GetApplication()->Script(
    "grid columnconfigure %s 1 -weight  4",
    this->InfoFrame->GetWidgetName());

  //direction details
  if(!this->DirectionFrame)
    this->DirectionFrame = vtkKWFrame::New();
  this->DirectionFrame->SetParent(this->MainFrame);
  this->DirectionFrame->Create();
  //this->DirectionFrame->SetLabelText("Along");
  this->GetApplication()->Script(
    "pack %s -side top -anchor n -expand y -padx 2 -pady 6", 
    this->DirectionFrame->GetWidgetName());

  //direction choice X
  if (!this->DirectionXEntry)
    this->DirectionXEntry = vtkKWEntryWithLabel::New();
  this->DirectionXEntry->SetParent(this->DirectionFrame);
  this->DirectionXEntry->Create();
  this->DirectionXEntry->SetLabelText("X : ");
  this->DirectionXEntry->GetWidget()->SetRestrictValueToDouble();
  this->DirectionXEntry->GetWidget()->SetWidth(7);
  this->GetApplication()->Script(
    "pack %s -side left -anchor n -padx 6 -pady 2", 
    this->DirectionXEntry->GetWidgetName());

  //Y
  if (!this->DirectionYEntry)
    this->DirectionYEntry = vtkKWEntryWithLabel::New();
  this->DirectionYEntry->SetParent(this->DirectionFrame);
  this->DirectionYEntry->Create();
  this->DirectionYEntry->SetLabelText("Y : ");
  this->DirectionYEntry->GetWidget()->SetRestrictValueToDouble();
  this->DirectionYEntry->GetWidget()->SetWidth(7);
  this->GetApplication()->Script(
    "pack %s -side left -anchor n -padx 6 -pady 2", 
    this->DirectionYEntry->GetWidgetName());
  //Z
  if (!this->DirectionZEntry)
    this->DirectionZEntry = vtkKWEntryWithLabel::New();
  this->DirectionZEntry->SetParent(this->DirectionFrame);
  this->DirectionZEntry->Create();
  this->DirectionZEntry->SetLabelText("Z : ");
  this->DirectionZEntry->GetWidget()->SetRestrictValueToDouble();
  this->DirectionZEntry->GetWidget()->SetWidth(7);
  this->GetApplication()->Script(
    "pack %s -side left -anchor n -padx 6 -pady 2", 
    this->DirectionZEntry->GetWidgetName());

  if (!this->ButtonFrame)
    this->ButtonFrame = vtkKWFrame::New();
  this->ButtonFrame->SetParent(this->MainFrame);
  this->ButtonFrame->Create();
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 6 -fill x", 
    this->ButtonFrame->GetWidgetName());
     
  this->ApplyButton->SetParent(this->ButtonFrame);
  this->ApplyButton->Create();
  this->ApplyButton->SetReliefToFlat();
  this->ApplyButton->SetImageToIcon( this->GetMimxMainWindow()->GetApplyButtonIcon() );
  //this->ApplyButton->SetText("Apply");
  this->ApplyButton->SetCommand(this, "AssignBoundaryConditionsApplyCallback");
  this->GetApplication()->Script(
    "pack %s -side left -anchor nw -expand y -padx 5 -pady 6", 
    this->ApplyButton->GetWidgetName());

  this->CancelButton->SetParent(this->ButtonFrame);
  this->CancelButton->Create();
  this->CancelButton->SetReliefToFlat();
  this->CancelButton->SetImageToIcon( this->GetMimxMainWindow()->GetCancelButtonIcon() );
  //this->CancelButton->SetText("Cancel");
  this->CancelButton->SetCommand(this, "AssignBoundaryConditionsCancelCallback");
  this->GetApplication()->Script(
    "pack %s -side right -anchor ne -expand y -padx 5 -pady 6", 
    this->CancelButton->GetWidgetName());

  
  if (!this->ViewFrame)
    this->ViewFrame = vtkKWFrameWithLabel::New();
  this->ViewFrame->SetParent( this->MainFrame );
  this->ViewFrame->Create();
  this->ViewFrame->SetLabelText("Load/BC Display");
  this->ViewFrame->CollapseFrame();
  this->GetApplication()->Script("pack %s -side top -anchor nw -expand n -fill x -padx 2 -pady 2",
                                 this->ViewFrame->GetWidgetName() );    
  // this->ViewFrame->CollapseFrame();

  if(!this->MultiColumnList)
    this->MultiColumnList = vtkKWMultiColumnListWithScrollbars::New();
  this->MultiColumnList->SetParent(this->ViewFrame->GetFrame());
  this->MultiColumnList->Create();
  this->MultiColumnList->GetWidget()->ClearStripeBackgroundColor();
  this->MultiColumnList->SetHorizontalScrollbarVisibility(0);
  this->MultiColumnList->GetWidget()->SetSelectionBackgroundColor(1.0,1.0,1.0);
  this->MultiColumnList->GetWidget()->SetHeight(4);
  
  
  // Add the columns 
  int col_index;
  col_index = this->MultiColumnList->GetWidget()->AddColumn(NULL);
  this->MultiColumnList->GetWidget()->SetColumnFormatCommandToEmptyOutput(col_index);
  this->MultiColumnList->GetWidget()->SetColumnLabelImageToPredefinedIcon(
    col_index, vtkKWIcon::IconEye);
  this->MultiColumnList->GetWidget()->SetColumnWidth( col_index, 2);
  
  col_index = this->MultiColumnList->GetWidget()->AddColumn("Node Set Name");
  this->MultiColumnList->GetWidget()->SetColumnWidth( col_index, 20);

  col_index = this->MultiColumnList->GetWidget()->AddColumn("Type");
  this->MultiColumnList->GetWidget()->SetColumnWidth( col_index, 10);
  this->MultiColumnList->GetWidget()->SetSortArrowVisibility(0);
  this->MultiColumnList->GetWidget()->ColumnSeparatorsVisibilityOff();
  this->MultiColumnList->GetWidget()->SetHeight( 2 );
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 2 -fill x", 
    this->MultiColumnList->GetWidgetName());
      
#if 0
  vtkKWIcon *viewBCIcon = vtkKWIcon::New();
  viewBCIcon->SetImage(   image_mimxDisplayBC, 
                          image_mimxDisplayBC_width, 
                          image_mimxDisplayBC_height, 
                          image_mimxDisplayBC_pixel_size);
                          
  if (!this->ViewBoundaryConditionsButton)
    this->ViewBoundaryConditionsButton = vtkKWCheckButtonWithLabel::New();
  this->ViewBoundaryConditionsButton->SetParent(this->ViewFrame->GetFrame());
  this->ViewBoundaryConditionsButton->Create();
  this->ViewBoundaryConditionsButton->GetWidget()->SetCommand(this, "ViewBoundaryConditionsCallback");
  //this->ViewBoundaryConditionsButton->GetWidget()->SetText("View Boundary Conditions");
  this->ViewBoundaryConditionsButton->GetWidget()->SetBalloonHelpString( "View boundary conditions" );
  this->ViewBoundaryConditionsButton->GetWidget()->SetBorderWidth( 2 );
  this->ViewBoundaryConditionsButton->GetWidget()->SetImageToIcon( viewBCIcon );
  this->ViewBoundaryConditionsButton->GetWidget()->SetSelectImageToIcon( viewBCIcon );
  //  this->ViewPropertyButton->GetWidget()->SetEnabled( 0 );
  this->GetApplication()->Script(
    "pack %s -side left -anchor w -expand n -padx 2 -pady 2", 
    this->ViewBoundaryConditionsButton->GetWidgetName());
  viewBCIcon->Delete();
#endif

  if (!this->GlyphFrame)    
    this->GlyphFrame = vtkKWFrame::New();
  this->GlyphFrame->SetParent(this->ViewFrame->GetFrame());
  this->GlyphFrame->Create();
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 2 -fill x", 
    this->GlyphFrame->GetWidgetName());
    
  vtkKWIcon *summaryIcon = vtkKWIcon::New();
  summaryIcon->SetImage(  image_mimxBCsummary, 
                          image_mimxBCsummary_width, 
                          image_mimxBCsummary_height, 
                          image_mimxBCsummary_pixel_size);
  if (!this->SummaryButton)
    this->SummaryButton = vtkKWPushButton::New();
  this->SummaryButton->SetParent(this->GlyphFrame);
  this->SummaryButton->Create();
  //this->SummaryButton->SetText("Summarize");
  this->SummaryButton->SetImageToIcon( summaryIcon );
  this->SummaryButton->SetReliefToFlat( );
  this->SummaryButton->SetCommand(this, "SummarizeCallback");
  this->GetApplication()->Script(
    "grid %s -row 0 -column 0 -sticky w -padx 1 -pady 2", 
    this->SummaryButton->GetWidgetName());
      
  if (!this->GlyphSizeEntry)
    this->GlyphSizeEntry = vtkKWEntryWithLabel::New();
  this->GlyphSizeEntry->SetParent(this->GlyphFrame);
  this->GlyphSizeEntry->Create();
  this->GlyphSizeEntry->SetLabelText("Scale :");
  this->GlyphSizeEntry->GetWidget()->SetRestrictValueToDouble();
  this->GlyphSizeEntry->GetWidget()->SetValueAsDouble(1.0);
  this->GlyphSizeEntry->GetWidget()->SetWidth(7);
  this->GlyphSizeEntry->GetWidget()->SetCommand(this, "GlyphSizeChangedCallback");
  this->GetApplication()->Script(
    "grid %s -row 0 -column 1 -sticky w -padx 1 -pady 2", 
    this->GlyphSizeEntry->GetWidgetName());
    
  
  if(!this->OpacityThumbwheel)
    this->OpacityThumbwheel = vtkKWSpinBoxWithLabel::New();
  this->OpacityThumbwheel->SetParent(this->GlyphFrame);
  this->OpacityThumbwheel->Create();
  this->OpacityThumbwheel->GetWidget()->SetRange(0.0, 1.0);
  this->OpacityThumbwheel->GetWidget()->SetValue(1.0);
  this->OpacityThumbwheel->GetWidget()->SetIncrement(0.05);
  this->OpacityThumbwheel->GetWidget()->SetValueFormat("%3.2f");
  this->OpacityThumbwheel->GetWidget()->WrapOff();
  this->OpacityThumbwheel->GetWidget()->SetWidth(6);
  this->OpacityThumbwheel->SetLabelWidth(8);
  this->OpacityThumbwheel->GetLabel()->SetText("Opacity :");
  this->OpacityThumbwheel->GetWidget()->SetCommand(this, "OpacityCallback");
  this->GetApplication()->Script(
    "grid %s -row 0 -column 2 -sticky w -padx 1 -pady 2", 
    this->OpacityThumbwheel->GetWidgetName());
//  this->OpacityThumbwheel->SetEnabled(0);

        
  vtkKWIcon *stepDefIcon = vtkKWIcon::New();
  stepDefIcon->SetImage(  image_mimxStepDef, 
                          image_mimxStepDef_width, 
                          image_mimxStepDef_height, 
                          image_mimxStepDef_pixel_size);
                          
  if (!this->DefineControlPushButton)    
    this->DefineControlPushButton = vtkKWPushButton::New();
  this->DefineControlPushButton->SetParent(this->MainFrame);
  this->DefineControlPushButton->Create();
  this->DefineControlPushButton->SetReliefToFlat( );
  this->DefineControlPushButton->SetImageToIcon( stepDefIcon );
  this->DefineControlPushButton->SetCommand(this, "StepControlCallback");
  this->GetApplication()->Script(
    "pack %s -side top -anchor n -expand y -padx 2 -pady 6",
    this->DefineControlPushButton->GetWidgetName());
  stepDefIcon->Delete();
  this->BoundaryConditionActor->SetRenderer( this->GetMimxMainWindow()->GetRenderWidget()->GetRenderer() );
  
}
//----------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::Update()
{
  this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::UpdateEnableState()
{
  this->UpdateObjectLists();
  this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
int vtkKWMimxAssignBoundaryConditionsGroup::AssignBoundaryConditionsApplyCallback()
{
  vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
  if(!strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
    {
    callback->ErrorMessage("Mesh selection required");
    return 0;
    }

  vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
  const char *name = combobox->GetValue();

  int num = combobox->GetValueIndex(name);
  if(num < 0 || num > combobox->GetNumberOfValues()-1)
    {
    callback->ErrorMessage("Invalid mesh selected");
    combobox->SetValue("");
    return 0;
    }
    
  vtkUnstructuredGrid *ugrid = vtkMimxMeshActor::SafeDownCast(
    this->FEMeshList->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
    
  const char *NodeSetname = this->NodeSetComboBox->GetValue();

  if(!strcmp(NodeSetname,""))
    {
    callback->ErrorMessage("Choose valid node set name");
    return 0;
    }

  const char *boundaryconditiontype = 
    this->BoundaryConditionTypeComboBox->GetValue();
  int bcType;
  if (strcmp(boundaryconditiontype, "Force") == 0)
    bcType = vtkMimxBoundaryConditionActor::Force;
  else if (strcmp(boundaryconditiontype, "Displacement") == 0)
    bcType = vtkMimxBoundaryConditionActor::Displacement;
  else if (strcmp(boundaryconditiontype, "Rotation") == 0)
    bcType = vtkMimxBoundaryConditionActor::Rotation;
  else
    bcType = vtkMimxBoundaryConditionActor::Moment;
  const char *entryx = this->DirectionXEntry->GetWidget()->GetValue();
  const char *entryy = this->DirectionYEntry->GetWidget()->GetValue();
  const char *entryz = this->DirectionZEntry->GetWidget()->GetValue();

  if(!strcmp(boundaryconditiontype,""))
    {
    callback->ErrorMessage("Choose Boundary condition type");
    return 0;
    }
  //int status = 0;

  /***Add the Specified Boundary Condition***/
  int activePage = this->GetNotebookStep();
  char stepnum[16];
  sprintf(stepnum, "%d", activePage);  
  char boundname[256];

  this->ConcatenateStrings("Step", stepnum, NodeSetname, 
                           boundaryconditiontype, "X", boundname);
    
  vtkFloatArray *boundarray = vtkFloatArray::SafeDownCast(
    ugrid->GetFieldData()->GetArray(boundname));
  if(boundarray)
    {
    ugrid->GetFieldData()->RemoveArray(boundname);
    this->BoundaryConditionActor->DeleteBoundaryConditionActor(boundname);
    }
    
  if(strcmp(entryx,""))
    {
    // create a new field data entry
    double entry = this->DirectionXEntry->GetWidget()->GetValueAsDouble();
    if (entry == 0.0 &&(!strcmp(boundaryconditiontype, "Force") || 
                        !strcmp(boundaryconditiontype, "Moment")))
      {
      }
    else
      {
      boundarray = vtkFloatArray::New();
      boundarray->SetName(boundname);
      boundarray->InsertNextValue(entry);
      ugrid->GetFieldData()->AddArray(boundarray);
      boundarray->Delete();
 
      vtkIntArray *BoundCond = vtkIntArray::SafeDownCast(
        ugrid->GetFieldData()->GetArray("Boundary_Condition_Number_Of_Steps"));
      if (activePage == BoundCond->GetValue(0)+1)
        {
        BoundCond->SetValue(0, activePage);
        }
      }
    }


  this->ConcatenateStrings("Step", stepnum, NodeSetname, 
                           boundaryconditiontype, "Y", boundname);

  boundarray = vtkFloatArray::SafeDownCast(
    ugrid->GetFieldData()->GetArray(boundname));
  if(boundarray)
    {
    ugrid->GetFieldData()->RemoveArray(boundname);
    this->BoundaryConditionActor->DeleteBoundaryConditionActor(boundname);
    }

  if(strcmp(entryy,""))
    {
    // create a new field data entry
    double entry = this->DirectionYEntry->GetWidget()->GetValueAsDouble();
    if(entry == 0.0 &&(!strcmp(boundaryconditiontype, "Force") || 
                       !strcmp(boundaryconditiontype, "Moment")))
      {
      }
    else
      {
      boundarray = vtkFloatArray::New();
      boundarray->SetName(boundname);
      boundarray->InsertNextValue(entry);
      ugrid->GetFieldData()->AddArray(boundarray);
      boundarray->Delete();
      vtkIntArray *BoundCond = vtkIntArray::SafeDownCast(
        ugrid->GetFieldData()->GetArray("Boundary_Condition_Number_Of_Steps"));

      if (activePage == BoundCond->GetValue(0)+1)
        {
        BoundCond->SetValue(0, activePage);
        }
      }
    }

  this->ConcatenateStrings("Step", stepnum, NodeSetname, 
                           boundaryconditiontype, "Z", boundname);

  boundarray = vtkFloatArray::SafeDownCast(
    ugrid->GetFieldData()->GetArray(boundname));
  if(boundarray)
    {
    ugrid->GetFieldData()->RemoveArray(boundname);
    this->BoundaryConditionActor->DeleteBoundaryConditionActor(boundname);
    }

  if(strcmp(entryz,""))
    {
    // create a new field data entry
    double entry = this->DirectionZEntry->GetWidget()->GetValueAsDouble();
    if(entry == 0.0 &&(!strcmp(boundaryconditiontype, "Force") || 
                       !strcmp(boundaryconditiontype, "Moment")))
      {
      }
    else
      {
      boundarray = vtkFloatArray::New();
      boundarray->SetName(boundname);
      boundarray->InsertNextValue(entry);
      ugrid->GetFieldData()->AddArray(boundarray);
      boundarray->Delete();
      // update the number of steps in the boundary condition
      vtkIntArray *BoundCond = vtkIntArray::SafeDownCast(
        ugrid->GetFieldData()->GetArray("Boundary_Condition_Number_Of_Steps"));

      if(activePage == BoundCond->GetValue(0)+1)
        {
        BoundCond->SetValue(0, activePage);
        }
      }
    }
    
  /***VAM***/
  this->BoundaryConditionActor->AddBoundaryConditionActor(stepnum, 
                                                          NodeSetname, boundaryconditiontype, boundname);
  this->BoundaryConditionActor->ShowBoundaryCondition(atoi(stepnum), NodeSetname, bcType);
  //ShowAllStepActors(atoi(stepnum));
  this->UpdateBCVisibilityList();
  strcpy(this->objectSelectionPrevious, "");
  this->GetMimxMainWindow()->GetRenderWidget()->Render();
    
  //    store sub-heading definitions
  vtkStringArray *subheading = vtkStringArray::New();
  subheading->InsertNextValue(this->SubHeadingEntry->GetValue());
  char step[256];
  strcpy(step, "Step_");    strcat(step, stepnum);
  strcat(step, "_SubHeading");
  subheading->SetName(step);
  ugrid->GetFieldData()->AddArray(subheading);
  subheading->Delete();
  return 1;
}
//----------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::AssignBoundaryConditionsCancelCallback()
{
//  this->MainFrame->UnpackChildren();
  this->CancelStatus = 1;
  this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName());
  this->MenuGroup->SetMenuButtonsEnabled(1);
  this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(1);
  vtkKWRenderWidget *ren = this->GetMimxMainWindow()->GetRenderWidget();
  /***VAM***/
  //this->DeleteBoundaryConditionActors();
  this->BoundaryConditionActor->DeleteAllBoundaryConditionActors();
  this->MultiColumnList->GetWidget()->DeleteAllRows();
  ren->Render();
  this->CancelStatus = 0;
}
//-----------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::UpdateObjectLists()
{
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
  if ((defaultItem == -1) && (this->FEMeshList->GetNumberOfItems() > 0))
    defaultItem = this->FEMeshList->GetNumberOfItems()-1;
    
  if (defaultItem != -1)
    {
    ObjectListComboBox->GetWidget()->SetValue(
      this->FEMeshList->GetItem(defaultItem)->GetFileName());
    this->DefineControlPushButton->SetEnabled(1);
    }
  else
    {
    ObjectListComboBox->GetWidget()->SetValue("");
    this->DefineNodeSetPushButton->SetEnabled(0);
    this->DefineControlPushButton->SetEnabled(0);
    }
  strcpy(this->objectSelectionPrevious, "");
  strcpy(this->nodeSetSelectionPrevious, "");
  strcpy(this->stepNumPrevious, "");
  strcpy(this->boundaryConditionTypePrevious, "");

  this->SelectionChangedCallback(ObjectListComboBox->GetWidget()->GetValue());
}
//--------------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::SelectionChangedCallback(const char *Selection)
{
  this->NodeSetComboBox->DeleteAllValues();
  this->NodeSetComboBox->SetValue("");
  if(this->CancelStatus)    return;
  if(!strcmp(this->objectSelectionPrevious, this->ObjectListComboBox->GetWidget()->GetValue()))
    return;
  if(!strcmp(Selection,""))
    {
    return;
    }
  this->DefineNodeSetPushButton->SetEnabled(1);
  strcpy(this->objectSelectionPrevious, this->ObjectListComboBox->GetWidget()->GetValue());

  vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
  vtkUnstructuredGrid *ugrid = vtkMimxMeshActor::SafeDownCast(
    this->FEMeshList->GetItem(combobox->GetValueIndex(Selection)))->GetDataSet();
  //    
  vtkIntArray *BoundCond = vtkIntArray::SafeDownCast(
    ugrid->GetFieldData()->GetArray("Boundary_Condition_Number_Of_Steps"));
  int NumberOfSteps;
  if(!BoundCond)
    {
    BoundCond = vtkIntArray::New();
    BoundCond->SetNumberOfValues(1);
    BoundCond->SetValue(0,1);
    BoundCond->SetName("Boundary_Condition_Number_Of_Steps");
    ugrid->GetFieldData()->AddArray(BoundCond);
    BoundCond->Delete();
    NumberOfSteps = 1;
    }
  else
    {
    NumberOfSteps = BoundCond->GetValue(0);
    }
  char numentries[16];
  int numberOfPages = this->StepNotebook->GetNumberOfPages();
  this->StepNotebook->RemovePage(">");
  for (int i=0;i<numberOfPages-2;i++)
    {
    sprintf(numentries, "STEP %d", i+1);
    this->StepNotebook->RemovePage(numentries);
    }
    
  if(!NumberOfSteps)
    {
    sprintf(numentries, "STEP %d", 1);
    this->StepNotebook->AddPage(numentries);
    }
  else
    {
    for (int i=0;i<NumberOfSteps;i++)
      {
      sprintf(numentries, "STEP %d", i+1);
      this->StepNotebook->AddPage(numentries);
      }
    }
  this->StepNotebook->AddPage(">");
  char firstpage[16];
  sprintf(firstpage, "STEP %d", 1);
  this->StepNotebook->RaisePage(firstpage);
  this->GenerateStepParameters(1);    
  strcpy(this->nodeSetSelectionPrevious, "");
  strcpy(this->stepNumPrevious, "");
  strcpy(this->boundaryConditionTypePrevious, "");

  sprintf(numentries, "%d", NumberOfSteps);
  int item = combobox->GetValueIndex(Selection);
  vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(this->FEMeshList->GetItem(item));
  meshActor->ShowMesh();
  this->GlyphSize = meshActor->GetAverageEdgeLength();
  this->GetMimxMainWindow()->GetRenderWidget()->Render();
  this->GetMimxMainWindow()->GetViewProperties()->UpdateVisibility();
    
  vtkStringArray *strarray = vtkStringArray::SafeDownCast(
    ugrid->GetFieldData()->GetAbstractArray("Node_Set_Names"));

  int numarrrays;
  if (strarray == NULL)
    numarrrays = 0;
  else
    numarrrays = strarray->GetNumberOfValues();
      
  this->NodeSetComboBox->DeleteAllValues();
  for (int i=0; i<numarrrays; i++)
    {
    this->NodeSetComboBox->AddValue(
      strarray->GetValue(i));
    }
    
  if (strarray == NULL)
    this->NodeSetComboBox->SetValue("");
  else
    this->NodeSetComboBox->SetValue(strarray->GetValue(numarrrays-1));
  this->GetValue();
    
  this->BoundaryConditionActor->SetDataSet( meshActor->GetDataSet() );
  this->BoundaryConditionActor->SetStep( this->GetNotebookStep());
  this->BoundaryConditionActor->ShowAllStepActors( this->GetNotebookStep() );
  this->UpdateBCVisibilityList();
  this->GetMimxMainWindow()->GetRenderWidget()->Render();
}
//-------------------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::NodeSetChangedCallback(const char *Selection)
{
  if(this->CancelStatus)    return;
    
  if(!strcmp(Selection,""))
    {
    return;
    }

  if(!strcmp(this->nodeSetSelectionPrevious,this->NodeSetComboBox->GetValue()))
    {
    return;
    }
  strcpy(this->nodeSetSelectionPrevious,this->NodeSetComboBox->GetValue());
  if(strcmp(this->BoundaryConditionTypeComboBox->GetValue(), ""))
    {
    this->BoundaryConditionTypeComboBox->SetValue(
      this->BoundaryConditionTypeComboBox->GetValue());
    }
  else
    {
    this->BoundaryConditionTypeComboBox->SetValue("Force");
    }
  this->BoundaryConditionTypeSelectionChangedCallback(
    this->BoundaryConditionTypeComboBox->GetValue());
}
//----------------------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::
BoundaryConditionTypeSelectionChangedCallback(const char *Selection)
{
  if(this->CancelStatus)    return;
  if(!strcmp(Selection,""))
    {
    return;
    }
  this->GetValue();
}
//----------------------------------------------------------------------------------------
int vtkKWMimxAssignBoundaryConditionsGroup::GetValue()
{
  // list all the three values.
  if(!strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
    {
    return 0;
    }

  if(!strcmp(this->NodeSetComboBox->GetValue(),""))
    {
    return 0;
    }
    
  vtkUnstructuredGrid *ugrid = vtkMimxMeshActor::SafeDownCast(
    this->FEMeshList->GetItem(this->ObjectListComboBox->GetWidget()->GetValueIndex(
                                ObjectListComboBox->GetWidget()->GetValue())))->GetDataSet();

  // stored value for direction
  int status = 0;
  // X
  char name[256];
  strcpy(name, "Step_");
  /***VAM***/
  int activePage = this->GetNotebookStep();
  char stepNumberStr[16];
  sprintf(stepNumberStr, "%d", activePage);
  strcat(name, stepNumberStr);
  strcat(name,"_");
  strcat(name, this->NodeSetComboBox->GetValue());
  strcat(name, "_");
  strcat(name, this->BoundaryConditionTypeComboBox->GetValue());
  strcat(name,"_");
  strcat(name, "X");

  vtkDataArray *dataarray = ugrid->GetFieldData()->GetArray(name);
  float Value = 0.0;
  if(dataarray)
    {
    Value = vtkFloatArray::SafeDownCast(dataarray)->GetValue(0);
    this->DirectionXEntry->GetWidget()->SetValueAsDouble(Value);
    status = 1;
    }
  else{
  this->DirectionXEntry->GetWidget()->SetValue("");
  }

  // Y
  //name[256];
  strcpy(name, "Step_");
  strcat(name, stepNumberStr);
  strcat(name,"_");
  strcat(name, this->NodeSetComboBox->GetValue());
  strcat(name, "_");
  strcat(name, this->BoundaryConditionTypeComboBox->GetValue());
  strcat(name,"_");
  strcat(name, "Y");

  dataarray = ugrid->GetFieldData()->GetArray(name);
  Value = 0.0;
  if(dataarray)
    {
    Value = vtkFloatArray::SafeDownCast(dataarray)->GetValue(0);
    this->DirectionYEntry->GetWidget()->SetValueAsDouble(Value);
    status = 1;
    }
  else{
  this->DirectionYEntry->GetWidget()->SetValue("");
  }

  // Z
  //name[256];
  strcpy(name, "Step_");
  /***VAM***/
  strcat(name, stepNumberStr);
  strcat(name,"_");
  strcat(name, this->NodeSetComboBox->GetValue());
  strcat(name, "_");
  strcat(name, this->BoundaryConditionTypeComboBox->GetValue());
  strcat(name,"_");
  strcat(name, "Z");

  dataarray = ugrid->GetFieldData()->GetArray(name);
  Value = 0.0;
  if(dataarray)
    {
    Value = vtkFloatArray::SafeDownCast(dataarray)->GetValue(0);
    this->DirectionZEntry->GetWidget()->SetValueAsDouble(Value);
    status = 1;
    }
  else{
  this->DirectionZEntry->GetWidget()->SetValue("");
  }
  return status;
}
//-----------------------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::AddStepNumberCallback()
{
  if (this->CancelStatus)    return;
  vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
  // check if the given step number has any boundary conditions
  if(!strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
    {
    callback->ErrorMessage("Mesh selection required");
    return;
    }

  vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
  const char *name = combobox->GetValue();

  int num = combobox->GetValueIndex(name);
  if(num < 0 || num > combobox->GetNumberOfValues()-1)
    {
    callback->ErrorMessage("Choose valid FE Mesh");
    combobox->SetValue("");
    return;
    }

  vtkUnstructuredGrid *ugrid = vtkMimxMeshActor::SafeDownCast(
    this->FEMeshList->GetItem(combobox->GetValueIndex(name)))->GetDataSet();

  if(!strcmp(this->NodeSetComboBox->GetValue(),""))
    {
    callback->ErrorMessage("Select node set");
    return;
    }
    
  int emptyStep = this->IsStepEmpty(ugrid);
  if (emptyStep > 0)
    {
    char errorString[256];
    sprintf(errorString, "Cannot add a new step because step %d is empty", emptyStep);
    callback->ErrorMessage( errorString );
    return;
    }

  char numentries[32];
  /***VAM***/
  int numberOfPages = this->StepNotebook->GetNumberOfPages();
  sprintf(numentries, "STEP %d", numberOfPages-1);
  this->StepNotebook->AddPage(numentries);
  this->StepNotebook->HideAllPages();
    
  int showNumberOfPages;
  if ( numberOfPages > 5 )
    {
    showNumberOfPages = 4;
    this->endNotebookPage = numberOfPages-1;
    this->startNotebookPage = this->endNotebookPage - 4;
    }
  else
    {
    showNumberOfPages = numberOfPages - 1;
    this->endNotebookPage = showNumberOfPages;
    this->startNotebookPage = this->endNotebookPage - showNumberOfPages;
    }
    
    
  this->StepNotebook->ShowPage("<");
  for (int i=this->startNotebookPage;i<this->endNotebookPage;i++)
    {
    char stepNumberStr[16];
    sprintf(stepNumberStr, "STEP %d", i+1);
    //std::cout << "Show : " << stepNumberStr << std::endl;
    this->StepNotebook->ShowPage( stepNumberStr );
    }
  this->StepNotebook->ShowPage(">");
  this->StepNotebook->RaisePage(numentries);
  this->StepNumberChangedCallback( numentries );
}
//-----------------------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::DeleteStepNumberCallback()
{
  if (this->CancelStatus)    return;
  vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
  // check if the given step number has any boundary conditions
  if(!strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
    {
    callback->ErrorMessage("Mesh selection required");
    return;
    }

  vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
  std::string name = combobox->GetValue();
    
  int num = combobox->GetValueIndex(name.c_str());
  if(num < 0 || num > combobox->GetNumberOfValues()-1)
    {
    callback->ErrorMessage("Invalid Mesh was selected");
    combobox->SetValue("");
    return;
    }

  vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(
    this->FEMeshList->GetItem(combobox->GetValueIndex(name.c_str())));
        
  //vtkUnstructuredGrid *ugrid = meshActor->GetDataSet();
    
  int activePage = this->GetNotebookStep();
  /* Check Page Index */
  char dialogMessage[256];
  sprintf(dialogMessage, "Do you really want to delete step number %d?", activePage);

  vtkKWMessageDialog *Dialog = vtkKWMessageDialog::New();
  Dialog->SetStyleToOkCancel();
  Dialog->SetApplication(this->GetApplication());
  Dialog->Create();
  Dialog->SetTitle("Confirm Delete Step");
  Dialog->SetText(dialogMessage);
  Dialog->Invoke();
  if(Dialog->GetStatus() != vtkKWMessageDialog::StatusOK)
    {
    Dialog->Delete();
    return;
    }

  meshActor->DeleteBoundaryConditionStep( activePage );
  Dialog->Delete();      
  strcpy(this->objectSelectionPrevious, "");
  this->SelectionChangedCallback(name.c_str());
        
  return;    
}

//-----------------------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::StepNumberChangedCallback( const char *StepNum )
{
  if(this->CancelStatus)    return;

  vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
  if(!strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
    {
    callback->ErrorMessage("FE Mesh selection required");
    return;
    }
  if ( StepNum != NULL)
    strcpy(this->stepNumPrevious, StepNum);
  else
    strcpy(this->stepNumPrevious, "");
    
  int stepNumber;
  if(strstr(StepNum, "STEP"))
    sscanf(StepNum, "STEP %d", &stepNumber);
  else
    stepNumber = atoi(StepNum);

  if (stepNumber < 0)  stepNumber = 0;
  this->BoundaryConditionActor->SetStep( stepNumber );
  this->BoundaryConditionActor->ShowAllStepActors( stepNumber );
  this->UpdateBCVisibilityList();
  this->GenerateStepParameters(stepNumber);    
  this->GetValue();
  this->GetMimxMainWindow()->GetRenderWidget()->Render();    
}
//-----------------------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::ConcatenateStrings(
  const char* Step, const char* Num, const char* NodeSetName, 
  const char* Type, const char* Direction, char *Name)
{
  strcpy(Name, Step);
  strcat(Name, "_");
  strcat(Name,Num);
  strcat(Name, "_");
  strcat(Name, NodeSetName);
  strcat(Name, "_");
  strcat(Name,Type);
  strcat(Name, "_");
  strcat(Name, Direction);
}
//------------------------------------------------------------------------------------------
int vtkKWMimxAssignBoundaryConditionsGroup::IsStepEmpty(vtkUnstructuredGrid *ugrid)
{
  vtkStringArray *nodesetnamestring = vtkStringArray::SafeDownCast(
    ugrid->GetFieldData()->GetAbstractArray("Node_Set_Names"));
  if(!nodesetnamestring)    return 0;
    
  /***VAM***/
  int numberOfSteps = this->StepNotebook->GetNumberOfPages() - 2;

  for (int j=1;j<=numberOfSteps;j++)
    {
    char stepnum[32];
    sprintf(stepnum, "%d", j);
    bool stepBC = false;
    for(int i=0; i<nodesetnamestring->GetNumberOfValues(); i++)
      {
      const char* nodesetname =  nodesetnamestring->GetValue(i);
      char Concatenate[256];
      this->ConcatenateStrings("Step", stepnum, nodesetname, "Force", "X", Concatenate);
      if(ugrid->GetFieldData()->GetArray(Concatenate))    stepBC = true;
      this->ConcatenateStrings("Step", stepnum, nodesetname, "Force", "Y", Concatenate);
      if(ugrid->GetFieldData()->GetArray(Concatenate))    stepBC = true;
      this->ConcatenateStrings("Step", stepnum, nodesetname, "Force", "Z", Concatenate);
      if(ugrid->GetFieldData()->GetArray(Concatenate))    stepBC = true;
          
      this->ConcatenateStrings("Step", stepnum, nodesetname, "Displacement", "X", Concatenate);
      if(ugrid->GetFieldData()->GetArray(Concatenate))    stepBC = true;
      this->ConcatenateStrings("Step", stepnum, nodesetname, "Displacement", "Y", Concatenate);
      if(ugrid->GetFieldData()->GetArray(Concatenate))    stepBC = true;
      this->ConcatenateStrings("Step", stepnum, nodesetname, "Displacement", "Z", Concatenate);
      if(ugrid->GetFieldData()->GetArray(Concatenate))    stepBC = true;
          
      this->ConcatenateStrings("Step", stepnum, nodesetname, "Rotation", "X", Concatenate);
      if(ugrid->GetFieldData()->GetArray(Concatenate))    return 0;
      this->ConcatenateStrings("Step", stepnum, nodesetname, "Rotation", "Y", Concatenate);
      if(ugrid->GetFieldData()->GetArray(Concatenate))    stepBC = true;
      this->ConcatenateStrings("Step", stepnum, nodesetname, "Rotation", "Z", Concatenate);
      if(ugrid->GetFieldData()->GetArray(Concatenate))    stepBC = true;
          
      this->ConcatenateStrings("Step", stepnum, nodesetname, "Moment", "X", Concatenate);
      if(ugrid->GetFieldData()->GetArray(Concatenate))    stepBC = true;
      this->ConcatenateStrings("Step", stepnum, nodesetname, "Moment", "Y", Concatenate);
      if(ugrid->GetFieldData()->GetArray(Concatenate))    stepBC = true;
      this->ConcatenateStrings("Step", stepnum, nodesetname, "Moment", "Z", Concatenate);
      if(ugrid->GetFieldData()->GetArray(Concatenate))    stepBC = true;
      }
    if (stepBC == false) return j;
    }
  return 0;
}
//-----------------------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::ViewBoundaryConditionsCallback(int vtkNotUsed(Mode))
{
}
//-----------------------------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::GlyphSizeChangedCallback(const char *vtkNotUsed(Entry))
{
  if(this->CancelStatus)    return;
  double scaleValue = this->GlyphSizeEntry->GetWidget()->GetValueAsDouble( );
  this->BoundaryConditionActor->SetScaleFactor(scaleValue);
  this->MimxMainWindow->GetRenderWidget()->Render();
}
//------------------------------------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::ProcessCallbackCommandEvents(vtkObject *caller,
                                                                          unsigned long event,
                                                                          void *calldata)
{
  const char **cargs = (const char**)calldata;

  bool adjustTabs = false;
  bool raiseTab = false;
  
  int numberOfPages = this->StepNotebook->GetNumberOfPages();
  int showNumberOfPages;
  
  if ( numberOfPages > 6 )
    showNumberOfPages = 4;
  else
    showNumberOfPages = numberOfPages - 2;
  
  
  int raisePageIndex = 1;
  switch (event)
    {
    case vtkKWEvent::NotebookRaisePageEvent:
   
      if (strcmp(cargs[0], "<") == 0)
        {
        if ( this->startNotebookPage > 0 )
          {
          this->startNotebookPage -= 1;
          this->endNotebookPage = this->startNotebookPage + showNumberOfPages;
          adjustTabs = true;
          raiseTab = true;
          raisePageIndex = this->startNotebookPage+1;
          }
        else
          {
          raiseTab = true;
          raisePageIndex = this->startNotebookPage+1;
          }
        }
      else if (strcmp(cargs[0], ">") == 0)
        {
        //std::cout << ">>>>>>>>>>>>>>>>>>>" << std::endl;
        if ( this->endNotebookPage < numberOfPages-2 ) 
          {  
          this->endNotebookPage += 1;
          this->startNotebookPage = this->endNotebookPage - showNumberOfPages;
          adjustTabs = true;
          raiseTab = true;
          raisePageIndex = this->endNotebookPage;
          }
        else
          {
          raiseTab = true;
          raisePageIndex = this->endNotebookPage;
          }
        }

    } 
  
  if ( adjustTabs )
    {    
    this->StepNotebook->HideAllPages();
    this->StepNotebook->ShowPage("<");
    for (int i=this->startNotebookPage;i<this->endNotebookPage;i++)
      {
      char stepNumberStr[16];
      sprintf(stepNumberStr, "STEP %d", i+1);
      this->StepNotebook->ShowPage( stepNumberStr );
      //std::cout << "Process Show Page : " << stepNumberStr << std::endl;
      }
    this->StepNotebook->ShowPage(">");
    }
  
  if ( raiseTab )
    {
    char stepNumberStr[16];
    sprintf(stepNumberStr, "STEP %d", raisePageIndex);
    this->StepNotebook->RaisePage( stepNumberStr );
    this->StepNumberChangedCallback(stepNumberStr);
    }  

  if ( strstr(cargs[0], "STEP") )
    {
    char stepStr[16];
    char stepNumberStr[16];
    sscanf(cargs[0], "%s %s", stepStr, stepNumberStr);
    //std::cout << stepStr << " " << stepNumberStr << std::endl;
    this->StepNumberChangedCallback(stepNumberStr);
    }  
  //int activeStep = this->GetNotebookStep();
 
  this->Superclass::ProcessCallbackCommandEvents(caller, event, calldata);
}
//------------------------------------------------------------------------------------------------------
int vtkKWMimxAssignBoundaryConditionsGroup::GetNotebookStep()
{
  int activePage = this->StepNotebook->GetRaisedPageId();
  const char *pageTitle = this->StepNotebook->GetPageTitle( activePage );
  int stepNumber;
  sscanf(pageTitle, "STEP %d", &stepNumber);
  if (stepNumber < 0)  stepNumber = 0;
  
  return stepNumber;
}
//-----------------------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::DefineNodeSetCallback()
{
  if (this->DefineNodeSetDialog == NULL)
    {
    this->DefineNodeSetDialog = vtkKWMimxDefineNodeSetGroup::New();
    this->DefineNodeSetDialog->SetApplication( this->GetApplication() );
    this->DefineNodeSetDialog->SetMimxMainWindow(this->GetMimxMainWindow());
    this->DefineNodeSetDialog->SetNodeSetCombobox(this->NodeSetComboBox);
    this->DefineNodeSetDialog->Create( );
    }
    
  vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
  const char *name = combobox->GetValue();
  vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(
    this->FEMeshList->GetItem(combobox->GetValueIndex(name)));
        
  this->DefineNodeSetDialog->SetMeshActor( meshActor );
  this->DefineNodeSetDialog->Display();
    
}
//------------------------------------------------------------------------------------------------------
double vtkKWMimxAssignBoundaryConditionsGroup::GetValue(int Direction, 
                                                        const char *BoundaryCondition, const char *NodeSetName)
{
  // list all the three values.
  if(!strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
    {
    return 0;
    }

  vtkUnstructuredGrid *ugrid = vtkMimxMeshActor::SafeDownCast(
    this->FEMeshList->GetItem(this->ObjectListComboBox->GetWidget()->GetValueIndex(
                                ObjectListComboBox->GetWidget()->GetValue())))->GetDataSet();

  char Dir[16];    
  if(Direction == 1)    strcpy(Dir, "X");
  if(Direction == 2)    strcpy(Dir, "Y");
  if(Direction == 3)    strcpy(Dir, "Z");

  char name[256];
  strcpy(name, "Step_");
  int activePage = this->GetNotebookStep();
  char stepNumberStr[16];
  sprintf(stepNumberStr, "%d", activePage);
  strcat(name, stepNumberStr);
  strcat(name,"_");
  strcat(name, NodeSetName);
  strcat(name, "_");
  strcat(name, BoundaryCondition);
  strcat(name,"_");
  strcat(name, Dir);

  vtkDataArray *dataarray = ugrid->GetFieldData()->GetArray(name);
  double Value = VTK_DOUBLE_MAX;
  if(dataarray)
    {
    Value = vtkFloatArray::SafeDownCast(dataarray)->GetValue(0);
    if(Direction == 1)
      this->DirectionXEntry->GetWidget()->SetValueAsDouble(Value);
    if(Direction == 2)
      this->DirectionYEntry->GetWidget()->SetValueAsDouble(Value);
    if(Direction == 3)
      this->DirectionZEntry->GetWidget()->SetValueAsDouble(Value);
    }
  else
    {
    if(Direction == 1)
      this->DirectionXEntry->GetWidget()->SetValue("");
    if(Direction == 2)
      this->DirectionYEntry->GetWidget()->SetValue("");
    if(Direction == 3)
      this->DirectionZEntry->GetWidget()->SetValue("");
    }
  return Value;
}
//-----------------------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::SummarizeCallback()
{
  
  if (!this->SummaryDialog)
    {
    this->SummaryDialog = vtkKWTopLevel::New();
    this->SummaryDialog->SetApplication( this->GetApplication() );
    this->SummaryDialog->Create();
    this->SummaryDialog->SetResizable(1, 1);
    this->SummaryDialog->SetSize(300, 500);
    this->SummaryDialog->ModalOn( );
    this->SummaryDialog->SetMasterWindow(this->GetMimxMainWindow());
    this->SummaryDialog->SetTitle( "BC/Load Summary" );
    }

  if (!this->BCTree)
    {
    this->BCTree = vtkKWTreeWithScrollbars::New();
    this->BCTree->SetParent( this->SummaryDialog );
    this->BCTree->Create();
    this->GetApplication()->Script("pack %s -side top -anchor nw -fill both -expand true -padx 2 -pady 2",
                                   this->BCTree->GetWidgetName() );    
    }
  vtkKWTree *treeWidget = this->BCTree->GetWidget();
    
  std::string meshName = this->ObjectListComboBox->GetWidget()->GetValue();
  treeWidget->DeleteAllNodes();
  treeWidget->AddNode(NULL, "Mesh", meshName.c_str());
    
  vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
  int num = combobox->GetValueIndex( meshName.c_str() );
  if(num < 0 || num > combobox->GetNumberOfValues()-1)
    {
    //callback->ErrorMessage("Invalid mesh selected");
    combobox->SetValue("");
    return;
    }
    
  vtkUnstructuredGrid *ugrid = vtkMimxMeshActor::SafeDownCast(
    this->FEMeshList->GetItem(num))->GetDataSet();
      
  vtkStringArray *nodesetnamestring = vtkStringArray::SafeDownCast(
    ugrid->GetFieldData()->GetAbstractArray("Node_Set_Names"));
  if(!nodesetnamestring)    return;
    
  /***VAM***/
  int numberOfSteps = this->StepNotebook->GetNumberOfPages() - 2;

  for (int j=1;j<=numberOfSteps;j++)
    {
    char stepnum[32];
    sprintf(stepnum, "%d", j);
    char stepText[32];
    sprintf(stepText, "STEP %d", j);
    treeWidget->AddNode("Mesh", stepnum, stepText);
    
    for(int i=0; i<nodesetnamestring->GetNumberOfValues(); i++)
      {
      const char* nodesetname =  nodesetnamestring->GetValue(i);
      char nodeSetText[256];
      char bcText[256];
      char bcId[256];
      char Concatenate[256];
          
      sprintf(nodeSetText, "%s%d", nodesetname, j);
      treeWidget->AddNode(stepnum, nodeSetText, nodesetname);
      treeWidget->OpenFirstNode();
      this->ConcatenateStrings("Step", stepnum, nodesetname, "Force", "X", Concatenate);
      vtkFloatArray *bcArray = vtkFloatArray::SafeDownCast(ugrid->GetFieldData()->GetArray(Concatenate));
      if ( bcArray )
        {
        float bcValue = bcArray->GetValue(0);
        sprintf(bcText, "Force X: %f", bcValue);
        sprintf(bcId, "ForceX%d%d", i,j);
        treeWidget->AddNode(nodeSetText, bcId, bcText);
        }
      this->ConcatenateStrings("Step", stepnum, nodesetname, "Force", "Y", Concatenate);
      bcArray = vtkFloatArray::SafeDownCast(ugrid->GetFieldData()->GetArray(Concatenate));
      if ( bcArray )
        {
        float bcValue = bcArray->GetValue(0);
        sprintf(bcText, "Force Y: %f", bcValue);
        sprintf(bcId, "ForceY%d%d", i,j);
        treeWidget->AddNode(nodeSetText, bcId, bcText);
        }
      this->ConcatenateStrings("Step", stepnum, nodesetname, "Force", "Z", Concatenate);
      bcArray = vtkFloatArray::SafeDownCast(ugrid->GetFieldData()->GetArray(Concatenate));
      if ( bcArray )
        {
        float bcValue = bcArray->GetValue(0);
        sprintf(bcText, "Force Z: %f", bcValue);
        sprintf(bcId, "ForceZ%d%d", i,j);
        treeWidget->AddNode(nodeSetText, bcId, bcText);
        }
          
      this->ConcatenateStrings("Step", stepnum, nodesetname, "Displacement", "X", Concatenate);
      bcArray = vtkFloatArray::SafeDownCast(ugrid->GetFieldData()->GetArray(Concatenate));
      if ( bcArray )
        {
        float bcValue = bcArray->GetValue(0);
        sprintf(bcText, "Displacement X: %f", bcValue);
        sprintf(bcId, "DisplacementX%d%d", i,j);
        treeWidget->AddNode(nodeSetText, bcId, bcText);
        }
      this->ConcatenateStrings("Step", stepnum, nodesetname, "Displacement", "Y", Concatenate);
      bcArray = vtkFloatArray::SafeDownCast(ugrid->GetFieldData()->GetArray(Concatenate));
      if ( bcArray )
        {
        float bcValue = bcArray->GetValue(0);
        sprintf(bcText, "Displacement Y: %f", bcValue);
        sprintf(bcId, "DisplacementY%d%d", i,j);
        treeWidget->AddNode(nodeSetText, bcId, bcText);
        }
      this->ConcatenateStrings("Step", stepnum, nodesetname, "Displacement", "Z", Concatenate);
      bcArray = vtkFloatArray::SafeDownCast(ugrid->GetFieldData()->GetArray(Concatenate));
      if ( bcArray )
        {
        float bcValue = bcArray->GetValue(0);
        sprintf(bcText, "Displacement Z: %f", bcValue);
        sprintf(bcId, "DisplacementZ%d%d", i,j);
        treeWidget->AddNode(nodeSetText, bcId, bcText);
        }
          
      this->ConcatenateStrings("Step", stepnum, nodesetname, "Rotation", "X", Concatenate);
      bcArray = vtkFloatArray::SafeDownCast(ugrid->GetFieldData()->GetArray(Concatenate));
      if ( bcArray )
        {
        float bcValue = bcArray->GetValue(0);
        sprintf(bcText, "Rotation X: %f", bcValue);
        sprintf(bcId, "RotationX%d%d", i,j);
        treeWidget->AddNode(nodeSetText, bcId, bcText);
        }
      this->ConcatenateStrings("Step", stepnum, nodesetname, "Rotation", "Y", Concatenate);
      bcArray = vtkFloatArray::SafeDownCast(ugrid->GetFieldData()->GetArray(Concatenate));
      if ( bcArray )
        {
        float bcValue = bcArray->GetValue(0);
        sprintf(bcText, "Rotation Y: %f", bcValue);
        sprintf(bcId, "RotationY%d%d", i,j);
        treeWidget->AddNode(nodeSetText, bcId, bcText);
        }
      this->ConcatenateStrings("Step", stepnum, nodesetname, "Rotation", "Z", Concatenate);
      bcArray = vtkFloatArray::SafeDownCast(ugrid->GetFieldData()->GetArray(Concatenate));
      if ( bcArray )
        {
        float bcValue = bcArray->GetValue(0);
        sprintf(bcText, "Rotation Z: %f", bcValue);
        sprintf(bcId, "RotationZ%d%d", i,j);
        treeWidget->AddNode(nodeSetText, bcId, bcText);
        }
          
      this->ConcatenateStrings("Step", stepnum, nodesetname, "Moment", "X", Concatenate);
      bcArray = vtkFloatArray::SafeDownCast(ugrid->GetFieldData()->GetArray(Concatenate));
      if ( bcArray )
        {
        float bcValue = bcArray->GetValue(0);
        sprintf(bcText, "Moment X: %f", bcValue);
        sprintf(bcId, "MomentX%d%d", i,j);
        treeWidget->AddNode(nodeSetText, bcId, bcText);
        }
      this->ConcatenateStrings("Step", stepnum, nodesetname, "Moment", "Y", Concatenate);
      bcArray = vtkFloatArray::SafeDownCast(ugrid->GetFieldData()->GetArray(Concatenate));
      if ( bcArray )
        {
        float bcValue = bcArray->GetValue(0);
        sprintf(bcText, "Moment Y: %f", bcValue);
        sprintf(bcId, "MomentY%d%d", i,j);
        treeWidget->AddNode(nodeSetText, bcId, bcText);
        }
      this->ConcatenateStrings("Step", stepnum, nodesetname, "Moment", "Z", Concatenate);
      bcArray = vtkFloatArray::SafeDownCast(ugrid->GetFieldData()->GetArray(Concatenate));
      if ( bcArray )
        {
        float bcValue = bcArray->GetValue(0);
        sprintf(bcText, "Moment Z: %f", bcValue);
        sprintf(bcId, "MomentZ%d%d", i,j);
        treeWidget->AddNode(nodeSetText, bcId, bcText);
        }
      }
    }
  
  if (!this->CancelDialogButton)
    {
    this->CancelDialogButton = vtkKWPushButton::New();
    this->CancelDialogButton->SetParent( this->SummaryDialog );
    this->CancelDialogButton->Create();
    this->CancelDialogButton->SetReliefToFlat();
    this->CancelDialogButton->SetImageToIcon( this->GetMimxMainWindow()->GetCancelButtonIcon() );
    this->CancelDialogButton->SetCommand(this, "SummaryDialogCancelCallback");
    this->GetApplication()->Script("pack %s -side bottom -anchor se -padx 2 -pady 2",
                                   this->CancelDialogButton->GetWidgetName() ); 
    }
  
  this->SummaryDialog->Display();
  return;
}
//----------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::DialogCancelCallback()
{
  this->StepControlDialog->Withdraw();
  return;
}
//----------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::SummaryDialogCancelCallback()
{
  this->SummaryDialog->Withdraw();
  return;
}
//----------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::DeleteBoundaryConditionActors()
{
  int i, j;
  int numItems = this->ActorCollection->GetNumberOfItems();
  vtkKWRenderWidget *renderWidget = NULL;
  if(this->MimxMainWindow)
    renderWidget = this->GetMimxMainWindow()->GetRenderWidget();
  for (i=0; i<numItems; i++)
    {
    vtkActorCollection *actorGroup = vtkActorCollection::SafeDownCast(this->ActorCollection->GetItemAsObject(i));
    vtkCollection *glyphGroup = vtkCollection::SafeDownCast(this->GlyphCollection->GetItemAsObject(i));
    int numActors = actorGroup->GetNumberOfItems();
    if(renderWidget)
      {
      for (j=0; j<numActors; j++)
        {
        renderWidget->RemoveViewProp(vtkActor::SafeDownCast(actorGroup->GetItemAsObject(j)));
        }
      }
    for (j=0; j<numActors; j++)
      {
      vtkActor::SafeDownCast(actorGroup->GetItemAsObject(j))->Delete();
      vtkGlyph3D::SafeDownCast(glyphGroup->GetItemAsObject(j))->Delete();
      }
    this->ActorCollection->GetItemAsObject(i)->Delete();
    this->GlyphCollection->GetItemAsObject(i)->Delete();
    }
  //
  if(numItems)
    {
    do 
      {
      this->ActorCollection->RemoveItem(numItems-1);
      this->GlyphCollection->RemoveItem(numItems-1);
      numItems --;
      }
    while (numItems);
    }
  //
  this->BoundaryConditionTypeArray->Initialize();
  this->NodeSetNameArray->Initialize();
  this->MultiColumnList->GetWidget()->DeleteAllRows();
  if(renderWidget)    renderWidget->Render();
}
//----------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::CreateBoundaryConditionActors()
{
  this->DeleteBoundaryConditionActors();
  vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
  if(!strcmp(combobox->GetValue(), ""))
    {
    return;
    }
    
  int i, j;
  int num = combobox->GetValueIndex( combobox->GetValue() );

  vtkUnstructuredGrid *ugrid = vtkMimxMeshActor::SafeDownCast(
    this->FEMeshList->GetItem(num))->GetDataSet();

  vtkStringArray *nodesetnamestring = vtkStringArray::SafeDownCast(
    ugrid->GetFieldData()->GetAbstractArray("Node_Set_Names"));
  if (!nodesetnamestring)
    {
    /***VAM - Add error callback ***/
    return; 
    }
  
  for (i=0; i<nodesetnamestring->GetNumberOfValues(); i++)
    {
    const char *nodesetname = nodesetnamestring->GetValue(i);
    for (j=0; j<this->BoundaryConditionArray->GetNumberOfValues(); j++)
      {
      const char *boundCondType = this->BoundaryConditionArray->GetValue(j);
      this->CreateBoundaryConditionActor(nodesetname, boundCondType);
      }
    }
  this->GetMimxMainWindow()->GetRenderWidget()->Render();
}
//----------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::ComputeBoundaryCondRepresentation(
  const char *BoundaryCondition, const char *NodeSetName, double ValX, double ValY, double ValZ )
{ 
  const char *meshName = this->ObjectListComboBox->GetWidget()->GetValue();

  vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
  // check if the given step number has any boundary conditions
  if(!strcmp(meshName,""))
    {
    callback->ErrorMessage("FE Mesh selection required");
    return;
    }

  int activePage = this->GetNotebookStep();
  char stepNum[16]; 
  sprintf(stepNum, "%d", activePage);

  char boundnameX[256];
  char boundnameY[256];
  char boundnameZ[256];

  this->ConcatenateStrings("Step", stepNum, NodeSetName, BoundaryCondition, "X", boundnameX);
  this->ConcatenateStrings("Step", stepNum, NodeSetName, BoundaryCondition, "Y", boundnameY);
  this->ConcatenateStrings("Step", stepNum, NodeSetName, BoundaryCondition, "Z", boundnameZ);

  vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(this->FEMeshList->GetItem(
                                                                 this->ObjectListComboBox->GetWidget()->GetValueIndex(meshName)));

  vtkUnstructuredGrid *ugrid = meshActor->GetDataSet();

  if(!meshActor->GetIsAverageEdgeLengthCalculated())    meshActor->CalculateAverageEdgeLength();

  vtkPointSet *pointSetX = meshActor->GetPointSetOfNodeSet(NodeSetName);

  if(!pointSetX->GetNumberOfPoints())    return;

  double factor = this->GlyphSizeEntry->GetWidget()->GetValueAsDouble();
  //
  double xmag = VTK_DOUBLE_MAX, ymag = VTK_DOUBLE_MAX, zmag = VTK_DOUBLE_MAX;
  double xmagabs = VTK_DOUBLE_MAX, ymagabs = VTK_DOUBLE_MAX, zmagabs = VTK_DOUBLE_MAX;
  double max = VTK_DOUBLE_MAX;

  if(ValX != VTK_DOUBLE_MAX)
    {
    xmagabs = fabs(ValX);
    xmag = ValX;
    max = xmagabs;    
    }
  if(ValY != VTK_DOUBLE_MAX)
    {
    ymagabs = fabs(ValY);
    ymag = ValY;
    if(max == VTK_DOUBLE_MAX)
      {
      max = ymagabs;
      }
    else
      {
      if(max < ymagabs)
        {
        max = ymagabs;
        }
      }
    }
  if(ValZ != VTK_DOUBLE_MAX)
    {
    zmagabs = fabs(ValZ);
    zmag = ValZ;
    if(max == VTK_DOUBLE_MAX)
      {
      max = zmagabs;
      }
    else
      {
      if(max < zmagabs)
        {
        max = zmagabs;
        }
      }
    }
  //
  //    double this->factorX = 0.0, this->factorY = 0.0, this->factorZ = 0.0;
  if(max != VTK_DOUBLE_MAX && max != 0.0)
    {
    if(xmagabs != VTK_DOUBLE_MAX)
      {
      this->factorX = xmagabs/max;
      }
    if(ymagabs != VTK_DOUBLE_MAX)
      {
      this->factorY = ymagabs/max;
      }
    if(zmagabs != VTK_DOUBLE_MAX)
      {
      this->factorZ = zmagabs/max;
      }
    }

  vtkActorCollection *actorGroup = vtkActorCollection::New();
  vtkCollection *glyphGroup = vtkCollection::New();

  if(ValX != VTK_DOUBLE_MAX)
    {
    vtkArrowSource *arrowSourceX = vtkArrowSource::New();

    vtkConeSource *coneSourceX = vtkConeSource::New();
    coneSourceX->SetHeight(coneSourceX->GetHeight()/3.0);
    coneSourceX->SetRadius(coneSourceX->GetRadius()/3.0);
    vtkActor *GlyphActorX = vtkActor::New();
    vtkGlyph3D *GlyphX = vtkGlyph3D::New();
    pointSetX = vtkUnstructuredGrid::New();
    pointSetX->DeepCopy(meshActor->GetPointSetOfNodeSet(NodeSetName));
    GlyphX->SetInput(pointSetX);

    vtkFloatArray *vectorarray = vtkFloatArray::New();
    vectorarray->SetNumberOfComponents(3);
    vectorarray->SetNumberOfTuples(pointSetX->GetNumberOfPoints());
    int i;
    for (i=0; i<pointSetX->GetNumberOfPoints(); i++)
      {
      float x[3];
      if(xmag >= 0.0)
        {
        x[0] = 1.0; x[1] = 0.0; x[2] = 0.0;
        }
      else
        {
        x[0] = -1.0; x[1] = 0.0; x[2] = 0.0;
        }
      vectorarray->SetTuple(i, x);
      }
    pointSetX->GetPointData()->SetVectors(vectorarray);

    vtkDataArray *dataarray = ugrid->GetFieldData()->GetArray(boundnameX);
    if(dataarray)
      {
      float Value = vtkFloatArray::SafeDownCast(dataarray)->GetValue(0);
      if(Value == 0.0 && !strcmp(BoundaryCondition, "Displacement"))    
        {
        this->factorX = 1.0;
        GlyphX->SetSource(coneSourceX->GetOutput());
        GlyphX->SetScaleFactor(this->GlyphSize*factor*this->factorX);
        //
        //double scaleFactorX = this->GlyphSize*factor*this->factorX;
        double offSet = coneSourceX->GetHeight()*this->GlyphSize*factor*this->factorX/2.0;
        for (i=0; i<pointSetX->GetNumberOfPoints(); i++)
          {
          double x[3];
          pointSetX->GetPoint(i, x);
          x[0] = x[0] - offSet;
          pointSetX->GetPoints()->SetPoint(i, x);
          }
        //
        }
      else 
        {
        GlyphX->SetSource(arrowSourceX->GetOutput());
        GlyphX->SetScaleFactor(this->GlyphSize*factor*this->factorX);
        }
      }
    else{
    GlyphX->SetSource(arrowSourceX->GetOutput());
    GlyphX->SetScaleFactor(this->GlyphSize*factor*this->factorX);
    }
    GlyphX->Update();

    vtkPolyDataMapper *glyphMapperX = vtkPolyDataMapper::New();
    glyphMapperX->SetInput(GlyphX->GetOutput());

    GlyphActorX->SetMapper(glyphMapperX);
    GlyphActorX->GetProperty()->SetColor(1.0, 0.0, 0.0);
    pointSetX->Delete();
    arrowSourceX->Delete();
    coneSourceX->Delete();
    glyphMapperX->Delete();
    vectorarray->Delete();
    actorGroup->AddItem(GlyphActorX);
    glyphGroup->AddItem(GlyphX);
    GlyphActorX->Delete();
    GlyphX->Delete();
    }

  if(ugrid->GetFieldData()->GetArray(boundnameY))
    {
    vtkPointSet *pointSetY = vtkUnstructuredGrid::New();
    pointSetY->DeepCopy(meshActor->GetPointSetOfNodeSet(NodeSetName));

    vtkArrowSource *arrowSourceY = vtkArrowSource::New();
    vtkConeSource *coneSourceY = vtkConeSource::New();
    coneSourceY->SetHeight(coneSourceY->GetHeight()/3.0);
    coneSourceY->SetRadius(coneSourceY->GetRadius()/3.0);
    vtkActor *GlyphActorY = vtkActor::New();

    vtkFloatArray *vectorarray = vtkFloatArray::New();
    vectorarray->SetNumberOfComponents(3);
    vectorarray->SetNumberOfTuples(pointSetY->GetNumberOfPoints());
    int i;
    for (i=0; i<pointSetY->GetNumberOfPoints(); i++)
      {
      float x[3];
      if(ymag >= 0.0)
        {
        x[0] = 0.0; x[1] = 1.0; x[2] = 0.0;
        }
      else
        {
        x[0] = 0.0; x[1] = -1.0; x[2] = 0.0;
        }
      vectorarray->SetTuple(i, x);
      }
    pointSetY->GetPointData()->SetVectors(vectorarray);

    vtkGlyph3D *GlyphY = vtkGlyph3D::New();
    GlyphY->SetInput(pointSetY);
    vtkDataArray *dataarray = ugrid->GetFieldData()->GetArray(boundnameY);
    if(dataarray)
      {
      float Value = vtkFloatArray::SafeDownCast(dataarray)->GetValue(0);
      if(Value == 0.0 && !strcmp(BoundaryCondition, "Displacement"))    
        {
        this->factorY = 1.0;
        GlyphY->SetSource(coneSourceY->GetOutput());
        GlyphY->SetScaleFactor(this->GlyphSize*factor*this->factorY);
        //
        //double scaleFactorY = this->GlyphSize*factor*this->factorY;
        double offSet = coneSourceY->GetHeight()*this->GlyphSize*factor*this->factorY/2.0;
        for (i=0; i<pointSetY->GetNumberOfPoints(); i++)
          {
          double x[3];
          pointSetY->GetPoint(i, x);
          x[1] = x[1] - offSet;
          pointSetY->GetPoints()->SetPoint(i, x);
          }
        //
        }
      else 
        {
        GlyphY->SetSource(arrowSourceY->GetOutput());
        GlyphY->SetScaleFactor(this->GlyphSize*factor*this->factorY);
        }
      }
    else{
    GlyphY->SetSource(arrowSourceY->GetOutput());
    GlyphY->SetScaleFactor(this->GlyphSize*factor*this->factorY);
    }
    GlyphY->SetVectorMode(1);
    GlyphY->SetVectorModeToUseVector();
    GlyphY->Update();

    vtkPolyDataMapper *glyphMapperY = vtkPolyDataMapper::New();
    glyphMapperY->SetInput(GlyphY->GetOutput());

    GlyphActorY->SetMapper(glyphMapperY);
    GlyphActorY->GetProperty()->SetColor(0.0, 1.0, 0.0);
    arrowSourceY->Delete();
    coneSourceY->Delete();
    glyphMapperY->Delete();
    vectorarray->Delete();
    pointSetY->Delete();
    actorGroup->AddItem(GlyphActorY);
    glyphGroup->AddItem(GlyphY);
    GlyphActorY->Delete();
    GlyphY->Delete();
    }

  if(ugrid->GetFieldData()->GetArray(boundnameZ))
    {
    vtkPointSet *pointSetZ = vtkUnstructuredGrid::New();
    pointSetZ->DeepCopy(meshActor->GetPointSetOfNodeSet(NodeSetName));

    vtkFloatArray *vectorarray = vtkFloatArray::New();
    vectorarray->SetNumberOfComponents(3);
    vectorarray->SetNumberOfTuples(pointSetZ->GetNumberOfPoints());

    int i;
    for (i=0; i<pointSetZ->GetNumberOfPoints(); i++)
      {
      float x[3];
      if(zmag >= 0.0)
        {
        x[0] = 0.0; x[1] = 0.0; x[2] = 1.0;
        }
      else
        {
        x[0] = 0.0; x[1] = 0.0; x[2] = -1.0;
        }
      vectorarray->SetTuple(i, x);
      }
    pointSetZ->GetPointData()->SetVectors(vectorarray);

    vtkArrowSource *arrowSourceZ = vtkArrowSource::New();
    vtkConeSource *coneSourceZ = vtkConeSource::New();
    coneSourceZ->SetHeight(coneSourceZ->GetHeight()/3.0);
    coneSourceZ->SetRadius(coneSourceZ->GetRadius()/3.0);
    vtkActor *GlyphActorZ = vtkActor::New();

    vtkGlyph3D *GlyphZ = vtkGlyph3D::New();
    GlyphZ->SetInput(pointSetZ);
    vtkDataArray *dataarray = ugrid->GetFieldData()->GetArray(boundnameZ);
    if(dataarray)
      {
      float Value = vtkFloatArray::SafeDownCast(dataarray)->GetValue(0);
      if(Value == 0.0 && !strcmp(BoundaryCondition, "Displacement"))    
        {
        this->factorZ = 1.0;
        GlyphZ->SetSource(coneSourceZ->GetOutput());
        GlyphZ->SetScaleFactor(this->GlyphSize*factor*this->factorZ);
        //
        //double scaleFactorZ = this->GlyphSize*factor*this->factorZ;
        double offSet = coneSourceZ->GetHeight()*this->GlyphSize*factor*this->factorZ/2.0;
        for (i=0; i<pointSetZ->GetNumberOfPoints(); i++)
          {
          double x[3];
          pointSetZ->GetPoint(i, x);
          x[2] = x[2] - offSet;
          pointSetZ->GetPoints()->SetPoint(i, x);
          }
        //
        }
      else 
        {
        GlyphZ->SetSource(arrowSourceZ->GetOutput());
        GlyphZ->SetScaleFactor(this->GlyphSize*factor*this->factorZ);
        }
      }
    else{
    GlyphZ->SetSource(arrowSourceZ->GetOutput());
    GlyphZ->SetScaleFactor(this->GlyphSize*factor*this->factorZ);
    }
    GlyphZ->SetVectorMode(1);
    GlyphZ->SetVectorModeToUseVector();
    GlyphZ->Update();

    vtkPolyDataMapper *glyphMapperZ = vtkPolyDataMapper::New();
    glyphMapperZ->SetInput(GlyphZ->GetOutput());

    GlyphActorZ->SetMapper(glyphMapperZ);
    GlyphActorZ->GetProperty()->SetColor(0.0, 0.0, 1.0);
    arrowSourceZ->Delete();
    coneSourceZ->Delete();
    glyphMapperZ->Delete();
    pointSetZ->Delete();
    vectorarray->Delete();
    actorGroup->AddItem(GlyphActorZ);
    glyphGroup->AddItem(GlyphZ);
    GlyphActorZ->Delete();
    GlyphZ->Delete();
    }

  this->ActorCollection->AddItem(actorGroup);
  this->GlyphCollection->AddItem(glyphGroup);
  actorGroup->Delete();
  glyphGroup->Delete();
}
//----------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::ShowBoundaryConditionRepresentation(int ConditionNum)
{
  std::string nodeSetName = this->MultiColumnList->GetWidget()->GetCellText(ConditionNum, 1);
  std::string typeStr = this->MultiColumnList->GetWidget()->GetCellText(ConditionNum, 2);
  int state = this->MultiColumnList->GetWidget()->GetCellWindowAsCheckButton(ConditionNum, 0)->GetSelectedState();
  int step = this->GetNotebookStep();
  int type;
  if (typeStr == "Force")
    type = vtkMimxBoundaryConditionActor::Force;
  else if (typeStr == "Displacement")
    type = vtkMimxBoundaryConditionActor::Displacement;
  else if (typeStr == "Rotation")
    type = vtkMimxBoundaryConditionActor::Rotation;
  else
    type = vtkMimxBoundaryConditionActor::Moment;
    
  if ( state )
    this->BoundaryConditionActor->ShowBoundaryCondition( step, nodeSetName.c_str(), type);
  else
    this->BoundaryConditionActor->HideBoundaryCondition( step, nodeSetName.c_str(), type);
}
//----------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::HideBoundaryConditionRepresentation(int ConditionNum)
{
  vtkActorCollection *actorGroup = vtkActorCollection::SafeDownCast(
    this->ActorCollection->GetItemAsObject(ConditionNum));
  int numActors = actorGroup->GetNumberOfItems();
  int i;
  for (i=0; i< numActors; i++)
    {
    this->GetMimxMainWindow()->GetRenderWidget()->RemoveViewProp(
      vtkActor::SafeDownCast(actorGroup->GetItemAsObject(i)));
    }
  this->MultiColumnList->GetWidget()->GetCellWindowAsCheckButton(ConditionNum, 0)->SetSelectedState(0);
  this->GetMimxMainWindow()->GetRenderWidget()->Render();
}
//----------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::UpdateBCVisibilityList( )
{
  char stepString[128];
  int stepNum = this->GetNotebookStep();
  sprintf(stepString, "Step_%d_", stepNum);
  
  std::string meshName = this->ObjectListComboBox->GetWidget()->GetValue();
  
  vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(this->FEMeshList->GetItem(
                                                                 this->ObjectListComboBox->GetWidget()->GetValueIndex(meshName.c_str())));
                
  this->MultiColumnList->GetWidget()->DeleteAllRows();
  
  vtkFieldData *fieldData = meshActor->GetDataSet()->GetFieldData();
  if ( fieldData )
    {
    for (int i=0;i<fieldData->GetNumberOfArrays();i++)
      {
      std::string arrayName = fieldData->GetArrayName(i);
      if (  arrayName.find(stepString) == 0 )
        {
        const char *nodeSetName;
        const char *stepNumber;
        const char *bcType;
        const char *axis;
        bool visibility; 

        BoundaryConditionProperty *p = this->BoundaryConditionActor->GetBoundaryCondition(arrayName.c_str());
        if(p)
          {
          stepNumber = p->StepNumber;
          bcType = p->bcType;
          axis = p->Axis;
          nodeSetName = p->nodeSet;
          visibility = p->IsVisible;
          bool found = false;
            
          for (int j=0;j<i;j++)
            {
            std::string prevArrayName = fieldData->GetArrayName(j); 
            char prevString[128];
            sprintf(prevString, "Step_%s_%s_%s", stepNumber, nodeSetName, bcType);
            if (  prevArrayName.find(prevString) == 0 )
              {
              found = true;
              }
            }
            
          if ( found == false )
            {
            int rowIndex = this->MultiColumnList->GetWidget()->GetNumberOfRows();
            if (visibility)
              this->MultiColumnList->GetWidget()->InsertCellTextAsInt(rowIndex,0,1);
            else
              this->MultiColumnList->GetWidget()->InsertCellTextAsInt(rowIndex,0,0);
            this->MultiColumnList->GetWidget()->SetCellWindowCommandToCheckButton(rowIndex,0);
            this->MultiColumnList->GetWidget()->GetCellWindowAsCheckButton(rowIndex,0)->SetCommand(this, "UpdateVisibility");
            this->MultiColumnList->GetWidget()->InsertCellText(rowIndex, 1, nodeSetName);
            this->MultiColumnList->GetWidget()->InsertCellText(rowIndex, 2, bcType);
            }
          }
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::CreateBoundaryConditionActor(
  const char *NodeSetName, const char *BoundaryConditionType)
{
  double xVal = this->GetValue(1, BoundaryConditionType, NodeSetName);
  double yVal = this->GetValue(2, BoundaryConditionType, NodeSetName);
  double zVal = this->GetValue(3, BoundaryConditionType, NodeSetName);
    
  if(xVal != VTK_DOUBLE_MAX || yVal != VTK_DOUBLE_MAX || zVal != VTK_DOUBLE_MAX )
    {
    this->ComputeBoundaryCondRepresentation(BoundaryConditionType, NodeSetName, xVal, yVal, zVal);
    this->BoundaryConditionTypeArray->InsertNextValue(BoundaryConditionType);
    this->NodeSetNameArray->InsertNextValue(NodeSetName);
    //this->MultiColumnList->GetWidget()->AddRow();
    int rowIndex = this->MultiColumnList->GetWidget()->GetNumberOfRows();
    this->MultiColumnList->GetWidget()->InsertCellTextAsInt(rowIndex,0,1);
    this->MultiColumnList->GetWidget()->SetCellWindowCommandToCheckButton(rowIndex,0);
    this->MultiColumnList->GetWidget()->GetCellWindowAsCheckButton(rowIndex,0)->SetCommand(this, "UpdateVisibility");
    this->MultiColumnList->GetWidget()->InsertCellText(rowIndex, 1, NodeSetName);
    this->MultiColumnList->GetWidget()->InsertCellText(rowIndex, 2, BoundaryConditionType);
    this->ShowBoundaryConditionRepresentation(this->NodeSetNameArray->GetNumberOfValues()-1);
    }
}
//----------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::DeleteBoundaryConditionActor(
  const char *NodeSetName, const char *BoundaryConditionType)
{
  int i, j;
  int numItems = this->ActorCollection->GetNumberOfItems();
  for (i=0; i< numItems; i++)
    {
    char listNodeSetName[256];
    const char *listBoundaryConditionType = this->MultiColumnList->GetWidget()->GetCellText(i, 1);
    strcpy(listNodeSetName, listBoundaryConditionType);
    listBoundaryConditionType = this->MultiColumnList->GetWidget()->GetCellText(i, 2);
    if(!strcmp(NodeSetName, listNodeSetName) && 
       !strcmp(listBoundaryConditionType, BoundaryConditionType))
      {
      break;
      }
    }
  if(i == numItems)    return;
  vtkKWRenderWidget *renderWidget = NULL;
  if(this->MimxMainWindow)
    renderWidget = this->GetMimxMainWindow()->GetRenderWidget();

  vtkActorCollection *actorGroup = vtkActorCollection::SafeDownCast(this->ActorCollection->GetItemAsObject(i));
  vtkCollection *glyphGroup = vtkCollection::SafeDownCast(this->GlyphCollection->GetItemAsObject(i));
  int numActors = actorGroup->GetNumberOfItems();
  if(renderWidget)
    {
    for (j=0; j<numActors; j++)
      {
      renderWidget->RemoveViewProp(vtkActor::SafeDownCast(actorGroup->GetItemAsObject(j)));
      }
    }
  for (j=0; j<numActors; j++)
    {
    vtkActor::SafeDownCast(actorGroup->GetItemAsObject(j))->Delete();
    vtkGlyph3D::SafeDownCast(glyphGroup->GetItemAsObject(j))->Delete();
    }
  this->ActorCollection->GetItemAsObject(i)->Delete();
  this->ActorCollection->RemoveItem(i);
  this->GlyphCollection->GetItemAsObject(i)->Delete();
  this->GlyphCollection->RemoveItem(i);

  vtkStringArray *temparraynode = vtkStringArray::New();
  vtkStringArray *temparraybound =  vtkStringArray::New();

  temparraynode->DeepCopy(this->NodeSetNameArray);
  this->NodeSetNameArray->Initialize();
    
  temparraybound->DeepCopy(this->BoundaryConditionTypeArray);
  temparraybound->Initialize();

  for (j=0; j<temparraybound->GetNumberOfTuples(); j++)
    {
    if(j != i)
      {
      this->NodeSetNameArray->InsertNextValue(temparraynode->GetValue(j));
      this->BoundaryConditionTypeArray->InsertNextValue(temparraybound->GetValue(j));
      }
    }
  temparraynode->Delete();
  temparraybound->Delete();
  this->MultiColumnList->GetWidget()->DeleteRow(i);
  if(renderWidget)    renderWidget->Render();    
}
//----------------------------------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::ModifyBoundaryConditionActor(
  const char *NodeSetName, const char *BoundaryConditionType)
{
  int i;
  int numItems = this->ActorCollection->GetNumberOfItems();
  for (i=0; i< numItems; i++)
    {
    char listNodeSetName[256];
    const char *listBoundaryConditionType = this->MultiColumnList->GetWidget()->GetCellText(i, 1);
    strcpy(listNodeSetName, listBoundaryConditionType);
    listBoundaryConditionType = this->MultiColumnList->GetWidget()->GetCellText(i, 2);
    if(!strcmp(NodeSetName, listNodeSetName) && 
       !strcmp(listBoundaryConditionType, BoundaryConditionType))
      {
      break;
      }
    }
  if(i == numItems)
    {
    this->CreateBoundaryConditionActor(NodeSetName, BoundaryConditionType);
    }
  else
    {
    double ValX = this->GetValue(1, BoundaryConditionType, NodeSetName);
    double ValY = this->GetValue(2, BoundaryConditionType, NodeSetName);
    double ValZ = this->GetValue(3, BoundaryConditionType, NodeSetName);
    if(ValX != VTK_DOUBLE_MAX || ValY != VTK_DOUBLE_MAX || ValZ != VTK_DOUBLE_MAX )
      {
      const char *meshName = this->ObjectListComboBox->GetWidget()->GetValue();

      int activePage = this->GetNotebookStep();
      char stepNum[16]; 
      sprintf(stepNum, "%d", activePage);

      char boundnameX[256];
      char boundnameY[256];
      char boundnameZ[256];

      this->ConcatenateStrings("Step", stepNum, NodeSetName, BoundaryConditionType, "X", boundnameX);
      this->ConcatenateStrings("Step", stepNum, NodeSetName, BoundaryConditionType, "Y", boundnameY);
      this->ConcatenateStrings("Step", stepNum, NodeSetName, BoundaryConditionType, "Z", boundnameZ);

      vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(this->FEMeshList->GetItem(
                                                                     this->ObjectListComboBox->GetWidget()->GetValueIndex(meshName)));

      vtkUnstructuredGrid *ugrid = meshActor->GetDataSet();

      double factor = this->GlyphSizeEntry->GetWidget()->GetValueAsDouble();
      //
      double xmag = VTK_DOUBLE_MAX, ymag = VTK_DOUBLE_MAX, zmag = VTK_DOUBLE_MAX;
      double xmagabs = VTK_DOUBLE_MAX, ymagabs = VTK_DOUBLE_MAX, zmagabs = VTK_DOUBLE_MAX;
      double max = VTK_DOUBLE_MAX;

      if(ValX != VTK_DOUBLE_MAX)
        {
        xmagabs = fabs(ValX);
        xmag = ValX;
        max = xmagabs;    
        }
      if(ValY != VTK_DOUBLE_MAX)
        {
        ymagabs = fabs(ValY);
        ymag = ValY;
        if(max == VTK_DOUBLE_MAX)
          {
          max = ymagabs;
          }
        else
          {
          if(max < ymagabs)
            {
            max = ymagabs;
            }
          }
        }
      if(ValZ != VTK_DOUBLE_MAX)
        {
        zmagabs = fabs(ValZ);
        zmag = ValZ;
        if(max == VTK_DOUBLE_MAX)
          {
          max = zmagabs;
          }
        else
          {
          if(max < zmagabs)
            {
            max = zmagabs;
            }
          }
        }
      //
      //    double this->factorX = 0.0, this->factorY = 0.0, this->factorZ = 0.0;
      if(max != VTK_DOUBLE_MAX && max != 0.0)
        {
        if(xmagabs != VTK_DOUBLE_MAX)
          {
          this->factorX = xmagabs/max;
          }
        if(ymagabs != VTK_DOUBLE_MAX)
          {
          this->factorY = ymagabs/max;
          }
        if(zmagabs != VTK_DOUBLE_MAX)
          {
          this->factorZ = zmagabs/max;
          }
        }

      vtkActorCollection *actorGroup = vtkActorCollection::SafeDownCast(this->ActorCollection->GetItemAsObject(i));
      vtkCollection *glyphGroup = vtkCollection::SafeDownCast(this->GlyphCollection->GetItemAsObject(i));
      int numActors = actorGroup->GetNumberOfItems();
      int j;
      vtkKWRenderWidget *renderWidget = this->GetMimxMainWindow()->GetRenderWidget();

      for (j=0; j<numActors; j++)
        {
        renderWidget->RemoveViewProp(vtkActor::SafeDownCast(actorGroup->GetItemAsObject(j)));
        }
      for (j=0; j<numActors; j++)
        {
        vtkActor::SafeDownCast(actorGroup->GetItemAsObject(j))->Delete();
        vtkGlyph3D::SafeDownCast(glyphGroup->GetItemAsObject(j))->Delete();
        }
      actorGroup->RemoveAllItems();
      glyphGroup->RemoveAllItems();

      vtkPointSet *pointSetX;

      if(ValX != VTK_DOUBLE_MAX)
        {
        vtkArrowSource *arrowSourceX = vtkArrowSource::New();

        vtkConeSource *coneSourceX = vtkConeSource::New();
        coneSourceX->SetHeight(coneSourceX->GetHeight()/3.0);
        coneSourceX->SetRadius(coneSourceX->GetRadius()/3.0);
        vtkActor *GlyphActorX = vtkActor::New();
        vtkGlyph3D *GlyphX = vtkGlyph3D::New();
        pointSetX = vtkUnstructuredGrid::New();
        pointSetX->DeepCopy(meshActor->GetPointSetOfNodeSet(NodeSetName));
        GlyphX->SetInput(pointSetX);

        vtkFloatArray *vectorarray = vtkFloatArray::New();
        vectorarray->SetNumberOfComponents(3);
        vectorarray->SetNumberOfTuples(pointSetX->GetNumberOfPoints());
        for (i=0; i<pointSetX->GetNumberOfPoints(); i++)
          {
          float x[3];
          if(xmag >= 0.0)
            {
            x[0] = 1.0; x[1] = 0.0; x[2] = 0.0;
            }
          else
            {
            x[0] = -1.0; x[1] = 0.0; x[2] = 0.0;
            }
          vectorarray->SetTuple(i, x);
          }
        pointSetX->GetPointData()->SetVectors(vectorarray);

        vtkDataArray *dataarray = ugrid->GetFieldData()->GetArray(boundnameX);
        if(dataarray)
          {
          float Value = vtkFloatArray::SafeDownCast(dataarray)->GetValue(0);
          if(Value == 0.0 && !strcmp(BoundaryConditionType, "Displacement"))    
            {
            this->factorX = 1.0;
            GlyphX->SetSource(coneSourceX->GetOutput());
            GlyphX->SetScaleFactor(this->GlyphSize*factor*this->factorX);
            //
            //double scaleFactorX = this->GlyphSize*factor*this->factorX;
            double offSet = coneSourceX->GetHeight()*this->GlyphSize*factor*this->factorX/2.0;
            for (i=0; i<pointSetX->GetNumberOfPoints(); i++)
              {
              double x[3];
              pointSetX->GetPoint(i, x);
              x[0] = x[0] - offSet;
              pointSetX->GetPoints()->SetPoint(i, x);
              }
            //
            }
          else 
            {
            GlyphX->SetSource(arrowSourceX->GetOutput());
            GlyphX->SetScaleFactor(this->GlyphSize*factor*this->factorX);
            }
          }
        else{
        GlyphX->SetSource(arrowSourceX->GetOutput());
        GlyphX->SetScaleFactor(this->GlyphSize*factor*this->factorX);
        }
        GlyphX->Update();

        vtkPolyDataMapper *glyphMapperX = vtkPolyDataMapper::New();
        glyphMapperX->SetInput(GlyphX->GetOutput());

        GlyphActorX->SetMapper(glyphMapperX);
        GlyphActorX->GetProperty()->SetColor(1.0, 0.0, 0.0);
        vectorarray->Delete();
        pointSetX->Delete();
        arrowSourceX->Delete();
        coneSourceX->Delete();
        glyphMapperX->Delete();
        actorGroup->AddItem(GlyphActorX);
        glyphGroup->AddItem(GlyphX);
        GlyphActorX->Delete();
        GlyphX->Delete();
        }

      if(ugrid->GetFieldData()->GetArray(boundnameY))
        {
        vtkPointSet *pointSetY = vtkUnstructuredGrid::New();
        pointSetY->DeepCopy(meshActor->GetPointSetOfNodeSet(NodeSetName));

        vtkArrowSource *arrowSourceY = vtkArrowSource::New();
        vtkConeSource *coneSourceY = vtkConeSource::New();
        coneSourceY->SetHeight(coneSourceY->GetHeight()/3.0);
        coneSourceY->SetRadius(coneSourceY->GetRadius()/3.0);
        vtkActor *GlyphActorY = vtkActor::New();

        vtkFloatArray *vectorarray = vtkFloatArray::New();
        vectorarray->SetNumberOfComponents(3);
        vectorarray->SetNumberOfTuples(pointSetY->GetNumberOfPoints());
        for (i=0; i<pointSetY->GetNumberOfPoints(); i++)
          {
          float x[3];
          if(ymag >= 0.0)
            {
            x[0] = 0.0; x[1] = 1.0; x[2] = 0.0;
            }
          else
            {
            x[0] = 0.0; x[1] = -1.0; x[2] = 0.0;
            }
          vectorarray->SetTuple(i, x);
          }
        pointSetY->GetPointData()->SetVectors(vectorarray);

        vtkGlyph3D *GlyphY = vtkGlyph3D::New();
        GlyphY->SetInput(pointSetY);
        vtkDataArray *dataarray = ugrid->GetFieldData()->GetArray(boundnameY);
        if(dataarray)
          {
          float Value = vtkFloatArray::SafeDownCast(dataarray)->GetValue(0);
          if(Value == 0.0 && !strcmp(BoundaryConditionType, "Displacement"))    
            {
            this->factorY = 1.0;
            GlyphY->SetSource(coneSourceY->GetOutput());
            GlyphY->SetScaleFactor(this->GlyphSize*factor*this->factorY);
            //
            //double scaleFactorY = this->GlyphSize*factor*this->factorY;
            double offSet = coneSourceY->GetHeight()*this->GlyphSize*factor*this->factorY/2.0;
            for (i=0; i<pointSetY->GetNumberOfPoints(); i++)
              {
              double x[3];
              pointSetY->GetPoint(i, x);
              x[1] = x[1] - offSet;
              pointSetY->GetPoints()->SetPoint(i, x);
              }
            //
            }
          else 
            {
            GlyphY->SetSource(arrowSourceY->GetOutput());
            GlyphY->SetScaleFactor(this->GlyphSize*factor*this->factorY);
            }
          }
        else{
        GlyphY->SetSource(arrowSourceY->GetOutput());
        GlyphY->SetScaleFactor(this->GlyphSize*factor*this->factorY);
        }
        GlyphY->SetVectorMode(1);
        GlyphY->SetVectorModeToUseVector();
        GlyphY->Update();

        vtkPolyDataMapper *glyphMapperY = vtkPolyDataMapper::New();
        glyphMapperY->SetInput(GlyphY->GetOutput());

        GlyphActorY->SetMapper(glyphMapperY);
        GlyphActorY->GetProperty()->SetColor(0.0, 1.0, 0.0);
        arrowSourceY->Delete();
        coneSourceY->Delete();
        glyphMapperY->Delete();
        vectorarray->Delete();
        pointSetY->Delete();
        actorGroup->AddItem(GlyphActorY);
        glyphGroup->AddItem(GlyphY);
        GlyphActorY->Delete();
        GlyphY->Delete();
        }

      if(ugrid->GetFieldData()->GetArray(boundnameZ))
        {
        vtkPointSet *pointSetZ = vtkUnstructuredGrid::New();
        pointSetZ->DeepCopy(meshActor->GetPointSetOfNodeSet(NodeSetName));

        vtkFloatArray *vectorarray = vtkFloatArray::New();
        vectorarray->SetNumberOfComponents(3);
        vectorarray->SetNumberOfTuples(pointSetZ->GetNumberOfPoints());

        for (i=0; i<pointSetZ->GetNumberOfPoints(); i++)
          {
          float x[3];
          if(zmag >= 0.0)
            {
            x[0] = 0.0; x[1] = 0.0; x[2] = 1.0;
            }
          else
            {
            x[0] = 0.0; x[1] = 0.0; x[2] = -1.0;
            }
          vectorarray->SetTuple(i, x);
          }
        pointSetZ->GetPointData()->SetVectors(vectorarray);

        vtkArrowSource *arrowSourceZ = vtkArrowSource::New();
        vtkConeSource *coneSourceZ = vtkConeSource::New();
        coneSourceZ->SetHeight(coneSourceZ->GetHeight()/3.0);
        coneSourceZ->SetRadius(coneSourceZ->GetRadius()/3.0);
        vtkActor *GlyphActorZ = vtkActor::New();

        vtkGlyph3D *GlyphZ = vtkGlyph3D::New();
        GlyphZ->SetInput(pointSetZ);
        vtkDataArray *dataarray = ugrid->GetFieldData()->GetArray(boundnameZ);
        if(dataarray)
          {
          float Value = vtkFloatArray::SafeDownCast(dataarray)->GetValue(0);
          if(Value == 0.0 && !strcmp(BoundaryConditionType, "Displacement"))    
            {
            this->factorZ = 1.0;
            GlyphZ->SetSource(coneSourceZ->GetOutput());
            GlyphZ->SetScaleFactor(this->GlyphSize*factor*this->factorZ);
            //
            //double scaleFactorZ = this->GlyphSize*factor*this->factorZ;
            double offSet = coneSourceZ->GetHeight()*this->GlyphSize*factor*this->factorZ/2.0;
            for (i=0; i<pointSetZ->GetNumberOfPoints(); i++)
              {
              double x[3];
              pointSetZ->GetPoint(i, x);
              x[2] = x[2] - offSet;
              pointSetZ->GetPoints()->SetPoint(i, x);
              }
            //
            }
          else 
            {
            GlyphZ->SetSource(arrowSourceZ->GetOutput());
            GlyphZ->SetScaleFactor(this->GlyphSize*factor*this->factorZ);
            }
          }
        else{
        GlyphZ->SetSource(arrowSourceZ->GetOutput());
        GlyphZ->SetScaleFactor(this->GlyphSize*factor*this->factorZ);
        }
        GlyphZ->SetVectorMode(1);
        GlyphZ->SetVectorModeToUseVector();
        GlyphZ->Update();

        vtkPolyDataMapper *glyphMapperZ = vtkPolyDataMapper::New();
        glyphMapperZ->SetInput(GlyphZ->GetOutput());

        GlyphActorZ->SetMapper(glyphMapperZ);
        GlyphActorZ->GetProperty()->SetColor(0.0, 0.0, 1.0);
        arrowSourceZ->Delete();
        coneSourceZ->Delete();
        glyphMapperZ->Delete();
        pointSetZ->Delete();
        vectorarray->Delete();
        actorGroup->AddItem(GlyphActorZ);
        glyphGroup->AddItem(GlyphZ);
        GlyphActorZ->Delete();
        GlyphZ->Delete();
        }
      }    
    this->ShowBoundaryConditionRepresentation(i);
    }
}
//----------------------------------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::UpdateVisibility(int vtkNotUsed(Mode))
{
  int numItems = this->MultiColumnList->GetWidget()->GetNumberOfRows();
  int stepNumber = this->GetNotebookStep();
  for (int i=0; i<numItems; i++)
    {
    std::string nodeSetName = this->MultiColumnList->GetWidget()->GetCellText(i, 1);
    std::string type = this->MultiColumnList->GetWidget()->GetCellText(i, 2);
    int bcType;
    if (type == "Force")
      bcType = vtkMimxBoundaryConditionActor::Force;
    else if (type == "Displacement")
      bcType = vtkMimxBoundaryConditionActor::Displacement;
    else if (type == "Rotation")
      bcType = vtkMimxBoundaryConditionActor::Rotation;
    else
      bcType = vtkMimxBoundaryConditionActor::Moment;
    if (this->MultiColumnList->GetWidget()->GetCellWindowAsCheckButton(i, 0)->GetSelectedState())
      {
      //this->ShowBoundaryConditionRepresentation(i);
      this->BoundaryConditionActor->ShowBoundaryCondition(stepNumber, nodeSetName.c_str(), bcType);
      }
    else
      {
      //this->HideBoundaryConditionRepresentation(i);
      this->BoundaryConditionActor->HideBoundaryCondition(stepNumber, nodeSetName.c_str(), bcType);
      }
    }
  this->GetMimxMainWindow()->GetRenderWidget()->Render();    
}
//-------------------------------------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::ModifyBoundaryConditionActors()
{
  int i;
  int numItems = this->MultiColumnList->GetWidget()->GetNumberOfRows();
  for (i=0; i< numItems; i++)
    {
    char NodeSetName[256], BoundaryConditionType[256];
    const char *listBoundaryConditionType = this->MultiColumnList->GetWidget()->GetCellText(i, 1);
    strcpy(NodeSetName, listBoundaryConditionType);
    listBoundaryConditionType = this->MultiColumnList->GetWidget()->GetCellText(i, 2);
    strcpy(BoundaryConditionType, listBoundaryConditionType);
    this->ModifyBoundaryConditionActor(NodeSetName, BoundaryConditionType);
    }
}
//-------------------------------------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::OpacityCallback(double Val)
{
  vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
  if(!strcmp(combobox->GetValue(), ""))
    {
    return;
    }

  int num = combobox->GetValueIndex( combobox->GetValue() );
  vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(this->FEMeshList->GetItem(num));
  meshActor->SetMeshOpacity(Val);
  this->MimxMainWindow->GetRenderWidget()->Render();
}
//#if 0
//-----------------------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::StepControlCallback()
{
  vtkKWIcon *applyIcon = vtkKWIcon::New();
  applyIcon->SetImage(  image_mimxSmallApply, 
                        image_mimxSmallApply_width, 
                        image_mimxSmallApply_height, 
                        image_mimxSmallApply_pixel_size);
  
  /*************** Step Control Dialog ***************/
  if (!this->StepControlDialog)
    {
    this->StepControlDialog = vtkKWTopLevel::New();
    this->StepControlDialog->SetApplication( this->GetApplication() );
    this->StepControlDialog->Create();
    this->StepControlDialog->SetResizable(1, 1);
    this->StepControlDialog->SetSize(300, 810);
    this->StepControlDialog->SetMinimumSize(300, 810);
    this->StepControlDialog->ModalOn( );
    this->StepControlDialog->SetMasterWindow(this->GetMimxMainWindow());
    this->StepControlDialog->SetTitle( "Step Definitions" );
    }
  
  /*************** Step Control Parameters ***************/
  if (!this->StepParameterFrame)
    {
    this->StepParameterFrame = vtkKWFrameWithLabel::New();
    this->StepParameterFrame->SetParent( this->StepControlDialog );
    this->StepParameterFrame->Create();
    //this->StepParameterFrame->AllowFrameToCollapseOff();
    this->StepParameterFrame->SetLabelText("*STEP");
    this->GetApplication()->Script(
      "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", 
      this->StepParameterFrame->GetWidgetName());
    }
  if (!this->StepIncrementEntry)
    {
    this->StepIncrementEntry = vtkKWEntryWithLabel::New();
    this->StepIncrementEntry->SetParent( this->StepParameterFrame->GetFrame() );
    this->StepIncrementEntry->Create();
    this->StepIncrementEntry->SetLabelText("INC= ");
    this->StepIncrementEntry->GetWidget()->SetRestrictValueToInteger();
    this->StepIncrementEntry->GetWidget()->SetValueAsInt(100);
    this->StepIncrementEntry->GetWidget()->SetWidth(15);
    this->GetApplication()->Script(
      "pack %s -side top -anchor nw -padx 10 -pady 2", 
      this->StepIncrementEntry->GetWidgetName());
    } 
    
  if (!this->AmplitudeTypeMenu)
    {
    this->AmplitudeTypeMenu = vtkKWMenuButtonWithLabel::New();
    this->AmplitudeTypeMenu->SetParent(this->StepParameterFrame->GetFrame());
    this->AmplitudeTypeMenu->Create();
    this->AmplitudeTypeMenu->SetLabelText("AMPLITUDE=");
    this->AmplitudeTypeMenu->GetWidget()->SetWidth( 25 );
    this->AmplitudeTypeMenu->GetWidget()->GetMenu()->AddRadioButton("Default");
    this->AmplitudeTypeMenu->GetWidget()->GetMenu()->AddRadioButton("Step");
    this->AmplitudeTypeMenu->GetWidget()->GetMenu()->AddRadioButton("Ramp");
    this->AmplitudeTypeMenu->GetWidget()->SetValue("Default");
    this->GetApplication()->Script(
      "pack %s -side top -anchor nw -padx 10 -pady 2", 
      this->AmplitudeTypeMenu->GetWidgetName());
    }
  if (!this->NlGeomButton)
    {
    this->NlGeomButton = vtkKWCheckButtonWithLabel::New();
    this->NlGeomButton->SetParent( this->StepParameterFrame->GetFrame() );
    this->NlGeomButton->Create();
    this->NlGeomButton->SetLabelText("NLGEOM");
    this->NlGeomButton->SetLabelPositionToRight();
    this->GetApplication()->Script(
      "pack %s -side top -anchor nw -padx 10 -pady 2", 
      this->NlGeomButton->GetWidgetName());
    } 
    
  if (!this->UnsymmButton)
    {
    this->UnsymmButton = vtkKWCheckButtonWithLabel::New();
    this->UnsymmButton->SetParent(this->StepParameterFrame->GetFrame());
    this->UnsymmButton->Create();
    this->UnsymmButton->SetLabelText("UNSYMM");
    this->UnsymmButton->SetLabelPositionToRight();
    this->GetApplication()->Script(
      "pack %s -side top -anchor nw -padx 10 -pady 2", 
      this->UnsymmButton->GetWidgetName());
    } 
    
  if (!this->AnalysisTypeMenu)
    {
    this->AnalysisTypeMenu = vtkKWMenuButton::New();
    this->AnalysisTypeMenu->SetParent(this->StepParameterFrame->GetFrame());
    this->AnalysisTypeMenu->Create();
    this->AnalysisTypeMenu->SetWidth( 25 );
    this->AnalysisTypeMenu->GetMenu()->AddRadioButton("Static Analysis");
    this->AnalysisTypeMenu->GetMenu()->AddRadioButton("Dynamic Analysis");
    this->AnalysisTypeMenu->SetValue("Static Analysis");
    this->GetApplication()->Script(
      "pack %s -side top -anchor nw -padx 10 -pady 2", 
      this->AnalysisTypeMenu->GetWidgetName());
    }
    
  if (!this->TimeIncrementEntry)
    {
    this->TimeIncrementEntry = vtkKWEntryWithLabel::New();
    this->TimeIncrementEntry->SetParent(this->StepParameterFrame->GetFrame());
    this->TimeIncrementEntry->Create();
    this->TimeIncrementEntry->GetWidget()->SetWidth(15);
    this->TimeIncrementEntry->GetWidget()->SetValue("1.0,1.0");
    this->TimeIncrementEntry->SetLabelText("Time Increment Control: ");
    this->TimeIncrementEntry->GetWidget()->SetBalloonHelpString("Enter the starting, ending, min, and max increment separated by commas");
    this->GetApplication()->Script(
      "pack %s -side top -anchor nw -padx 10 -pady 2", 
      this->TimeIncrementEntry->GetWidgetName());
    }
  if (!this->StepControlApplyButton)
    {
    this->StepControlApplyButton = vtkKWPushButton::New();
    this->StepControlApplyButton->SetParent( this->StepParameterFrame->GetFrame());
    this->StepControlApplyButton->Create();
    this->StepControlApplyButton->SetReliefToFlat();
    this->StepControlApplyButton->SetImageToIcon( applyIcon );
    this->StepControlApplyButton->SetCommand(this, "DialogApplyCallback");
    this->GetApplication()->Script("pack %s -side top -anchor ne -padx 2 -pady 2",
                                   this->StepControlApplyButton->GetWidgetName() ); 
    }    
  /*************** Node Control Parameters ***************/
  if (!this->NodePrintFrame)
    {
    this->NodePrintFrame = vtkKWFrameWithLabel::New();
    this->NodePrintFrame->SetParent( this->StepControlDialog );
    this->NodePrintFrame->Create();
    //this->NodePrintFrame->AllowFrameToCollapseOff();
    this->NodePrintFrame->SetLabelText("Nodes");
    this->GetApplication()->Script(
      "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", 
      this->NodePrintFrame->GetWidgetName());
    }
  
  if (!this->NodeSetMenu)
    {
    this->NodeSetMenu = vtkKWComboBoxWithLabel::New();
    this->NodeSetMenu->SetParent(this->NodePrintFrame->GetFrame());
    this->NodeSetMenu->Create();
    this->NodeSetMenu->GetWidget()->SetWidth(25);
    this->NodeSetMenu->SetLabelText( "NSET= " );
    this->NodeSetMenu->GetWidget()->SetReadOnly(1);
    this->NodeSetMenu->GetWidget()->SetCommand(this, "OutputNodeSetChangedCallback");
    this->GetApplication()->Script(
      "pack %s -side top -anchor nw -padx 2 -pady 2", 
      this->NodeSetMenu->GetWidgetName());
    }
// populate the node set list    
  vtkUnstructuredGrid *ugrid = this->IsMeshValid();
  if(ugrid)
    {
    this->NodeSetMenu->GetWidget()->DeleteAllValues();
    vtkStringArray *stringarray = vtkStringArray::SafeDownCast(
      ugrid->GetFieldData()->GetAbstractArray("Node_Set_Names"));
    if(stringarray)
      {
      int i;
      for (i=0; i<stringarray->GetNumberOfTuples(); i++)
        {
        this->NodeSetMenu->GetWidget()->AddValue(stringarray->GetValue(i));
        }
      this->NodeSetMenu->GetWidget()->SetValue(stringarray->GetValue(0));
      }
    }
  //
  if (!this->NodePrintButton)
    {
    this->NodePrintButton = vtkKWCheckButtonWithLabel::New();
    this->NodePrintButton->SetParent(this->NodePrintFrame->GetFrame());
    this->NodePrintButton->Create();
    this->NodePrintButton->SetLabelText("*NODE PRINT,");
    this->NodePrintButton->SetLabelPositionToRight();
    this->NodePrintButton->GetWidget()->SetCommand(this, "NodePrintCallback");
    this->GetApplication()->Script(
      "pack %s -side top -anchor nw -padx 20 -pady 2", 
      this->NodePrintButton->GetWidgetName());
    } 
    
  if (!this->NodePrintFrequencyEntry)
    {
    this->NodePrintFrequencyEntry = vtkKWEntryWithLabel::New();
    this->NodePrintFrequencyEntry->SetParent(this->NodePrintFrame->GetFrame());
    this->NodePrintFrequencyEntry->Create();
    this->NodePrintFrequencyEntry->SetLabelText("FREQUENCY= ");
    this->NodePrintFrequencyEntry->GetWidget()->SetRestrictValueToInteger();
    this->NodePrintFrequencyEntry->GetWidget()->SetValue("1");
    this->GetApplication()->Script(
      "pack %s -side top -anchor nw -padx 40 -pady 2", 
      this->NodePrintFrequencyEntry->GetWidgetName());
    this->NodePrintFrequencyEntry->SetEnabled(0);
    }
  
  if (!this->NodePrintSummaryButton)
    {
    this->NodePrintSummaryButton = vtkKWCheckButtonWithLabel::New();
    this->NodePrintSummaryButton->SetParent(this->NodePrintFrame->GetFrame());
    this->NodePrintSummaryButton->Create();
    this->NodePrintSummaryButton->SetLabelText("SUMMARY");
    this->NodePrintSummaryButton->SetLabelPositionToRight();
    this->GetApplication()->Script(
      "pack %s -side top -anchor nw -padx 40 -pady 2", 
      this->NodePrintSummaryButton->GetWidgetName());
    this->NodePrintSummaryButton->SetEnabled(0);
    } 
    
  if (!this->NodePrintTotalButton)
    {
    this->NodePrintTotalButton = vtkKWCheckButtonWithLabel::New();
    this->NodePrintTotalButton->SetParent(this->NodePrintFrame->GetFrame());
    this->NodePrintTotalButton->Create();
    this->NodePrintTotalButton->SetLabelText("TOTALS");
    this->NodePrintTotalButton->SetLabelPositionToRight();
    this->GetApplication()->Script(
      "pack %s -side top -anchor nw -padx 40 -pady 2", 
      this->NodePrintTotalButton->GetWidgetName());
    this->NodePrintTotalButton->SetEnabled(0);
    } 
  
  if (!this->NodePrintVariablesEntry)
    {
    this->NodePrintVariablesEntry = vtkKWEntryWithLabel::New();
    this->NodePrintVariablesEntry->SetParent(this->NodePrintFrame->GetFrame());
    this->NodePrintVariablesEntry->Create();
    this->NodePrintVariablesEntry->SetLabelText("Variable(s) of interest: ");
    this->GetApplication()->Script(
      "pack %s -side top -anchor nw -padx 40 -pady 2 -fill x", 
      this->NodePrintVariablesEntry->GetWidgetName());
    this->NodePrintVariablesEntry->SetEnabled(0);
    }
    
  if (!this->NodeOutputButton)
    {
    this->NodeOutputButton = vtkKWCheckButtonWithLabel::New();
    this->NodeOutputButton->SetParent( this->NodePrintFrame->GetFrame() );
    this->NodeOutputButton->Create();
    this->NodeOutputButton->SetLabelText("*NODE OUTPUT,");
    this->NodeOutputButton->SetLabelPositionToRight();
    this->NodeOutputButton->GetWidget()->SetCommand(this, "NodeOutputCallback");
    this->GetApplication()->Script(
      "pack %s -side top -anchor nw -padx 20 -pady 2", 
      this->NodeOutputButton->GetWidgetName());
    } 
    
  if (!this->NodeOutputVariablesEntry)
    {
    this->NodeOutputVariablesEntry = vtkKWEntryWithLabel::New();
    this->NodeOutputVariablesEntry->SetParent(this->NodePrintFrame->GetFrame());
    this->NodeOutputVariablesEntry->Create();
    this->NodeOutputVariablesEntry->SetLabelText("Variable(s) of interest: ");
    this->GetApplication()->Script(
      "pack %s -side top -anchor nw -padx 40 -pady 2 -fill x", 
      this->NodeOutputVariablesEntry->GetWidgetName());
    this->NodeOutputVariablesEntry->SetEnabled(0);
    }
    
  if (!this->NodeSetOutputApplyButton)
    {
    this->NodeSetOutputApplyButton = vtkKWPushButton::New();
    this->NodeSetOutputApplyButton->SetParent( this->NodePrintFrame->GetFrame());
    this->NodeSetOutputApplyButton->Create();
    this->NodeSetOutputApplyButton->SetReliefToFlat();
    this->NodeSetOutputApplyButton->SetImageToIcon( applyIcon );
    this->NodeSetOutputApplyButton->SetCommand(this, "NodeSetOutputApplyCallback");
    this->GetApplication()->Script("pack %s -side top -anchor ne -padx 2 -pady 2",
                                   this->NodeSetOutputApplyButton->GetWidgetName() ); 
    }    
  this->OutputNodeSetChangedCallback(this->NodeSetMenu->GetWidget()->GetValue());
  /*************** Element Control Parameters ***************/
  if (!this->ElementPrintFrame)
    {
    this->ElementPrintFrame = vtkKWFrameWithLabel::New();
    this->ElementPrintFrame->SetParent( this->StepControlDialog );
    this->ElementPrintFrame->Create();
    //this->ElementPrintFrame->AllowFrameToCollapseOff();
    this->ElementPrintFrame->SetLabelText("Elements");
    this->GetApplication()->Script(
      "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", 
      this->ElementPrintFrame->GetWidgetName());
    }
  
  if (!this->ElementSetMenu)
    {
    this->ElementSetMenu = vtkKWComboBoxWithLabel::New();
    this->ElementSetMenu->SetParent(this->ElementPrintFrame->GetFrame());
    this->ElementSetMenu->Create();
    this->ElementSetMenu->GetWidget()->SetWidth(25);
    this->ElementSetMenu->SetLabelText( "ELSET=" );
    this->ElementSetMenu->GetWidget()->SetCommand(this, "OutputElementSetChangedCallback");
    this->ElementSetMenu->GetWidget()->SetReadOnly(1);
    this->GetApplication()->Script(
      "pack %s -side top -anchor nw -padx 2 -pady 2", 
      this->ElementSetMenu->GetWidgetName());
    }
  // populate the element set list    
  if (ugrid)
    {
    this->ElementSetMenu->GetWidget()->DeleteAllValues();
    vtkStringArray *stringarray = vtkStringArray::SafeDownCast(
      ugrid->GetFieldData()->GetAbstractArray("Element_Set_Names"));
    if(stringarray)
      {
      int i;
      for (i=0; i<stringarray->GetNumberOfTuples(); i++)
        {
        this->ElementSetMenu->GetWidget()->AddValue(stringarray->GetValue(i));
        }
      this->ElementSetMenu->GetWidget()->SetValue(stringarray->GetValue(0));
      }
    }
  //
  if (!this->ElementPrintButton)
    {
    this->ElementPrintButton = vtkKWCheckButtonWithLabel::New();
    this->ElementPrintButton->SetParent(this->ElementPrintFrame->GetFrame());
    this->ElementPrintButton->Create();
    this->ElementPrintButton->SetLabelText("*EL PRINT,");
    this->ElementPrintButton->SetLabelPositionToRight();
    this->ElementPrintButton->GetWidget()->SetCommand(this, "ElementPrintCallback");
    this->GetApplication()->Script(
      "pack %s -side top -anchor nw -padx 20 -pady 2", 
      this->ElementPrintButton->GetWidgetName());
    }
  if (!this->ElementPrintFrequencyEntry)
    {
    this->ElementPrintFrequencyEntry = vtkKWEntryWithLabel::New();
    this->ElementPrintFrequencyEntry->SetParent(this->ElementPrintFrame->GetFrame());
    this->ElementPrintFrequencyEntry->Create();
    this->ElementPrintFrequencyEntry->SetLabelText("FREQUENCY= ");
    this->ElementPrintFrequencyEntry->GetWidget()->SetRestrictValueToInteger();
    this->ElementPrintFrequencyEntry->GetWidget()->SetValueAsInt(1);
    this->GetApplication()->Script(
      "pack %s -side top -anchor nw -padx 40 -pady 2", 
      this->ElementPrintFrequencyEntry->GetWidgetName());
    this->ElementPrintFrequencyEntry->SetEnabled(0);
    }
    
  if (!this->ElementPositionSetMenu)
    {
    this->ElementPositionSetMenu = vtkKWMenuButtonWithLabel::New();
    this->ElementPositionSetMenu->SetParent(this->ElementPrintFrame->GetFrame());
    this->ElementPositionSetMenu->Create();
    this->ElementPositionSetMenu->GetWidget()->SetWidth(25);
    this->ElementPositionSetMenu->SetLabelText( "POSITION= " );
    this->ElementPositionSetMenu->GetWidget()->GetMenu()->AddRadioButton("Averaged at Nodes");
    this->ElementPositionSetMenu->GetWidget()->GetMenu()->AddRadioButton("Centroidal");
    this->ElementPositionSetMenu->GetWidget()->GetMenu()->AddRadioButton("Integration Points");
    this->ElementPositionSetMenu->GetWidget()->GetMenu()->AddRadioButton("Nodes");
    this->ElementPositionSetMenu->GetWidget()->SetValue("Centroidal");
    this->GetApplication()->Script(
      "pack %s -side top -anchor nw -padx 40 -pady 2", 
      this->ElementPositionSetMenu->GetWidgetName());
    this->ElementPositionSetMenu->SetEnabled(0);
    }
    
  if (!this->ElementPrintSummaryButton)
    {
    this->ElementPrintSummaryButton = vtkKWCheckButtonWithLabel::New();
    this->ElementPrintSummaryButton->SetParent(this->ElementPrintFrame->GetFrame());
    this->ElementPrintSummaryButton->Create();
    this->ElementPrintSummaryButton->SetLabelText("SUMMARY");
    this->ElementPrintSummaryButton->SetLabelPositionToRight();
    this->GetApplication()->Script(
      "pack %s -side top -anchor nw -padx 40 -pady 2", 
      this->ElementPrintSummaryButton->GetWidgetName());
    this->ElementPrintSummaryButton->SetEnabled(0);
    } 
    
  if (!this->ElementPrintTotalButton)
    {
    this->ElementPrintTotalButton = vtkKWCheckButtonWithLabel::New();
    this->ElementPrintTotalButton->SetParent(this->ElementPrintFrame->GetFrame());
    this->ElementPrintTotalButton->Create();
    this->ElementPrintTotalButton->SetLabelText("TOTALS");
    this->ElementPrintTotalButton->SetLabelPositionToRight();
    this->GetApplication()->Script(
      "pack %s -side top -anchor nw -padx 40 -pady 2", 
      this->ElementPrintTotalButton->GetWidgetName());
    this->ElementPrintTotalButton->SetEnabled(0);
    }
  
  if (!this->ElementPrintVariablesEntry)
    {
    this->ElementPrintVariablesEntry = vtkKWEntryWithLabel::New();
    this->ElementPrintVariablesEntry->SetParent(this->ElementPrintFrame->GetFrame());
    this->ElementPrintVariablesEntry->Create();
    this->ElementPrintVariablesEntry->SetLabelText("Variable(s) of interest: ");
    this->GetApplication()->Script(
      "pack %s -side top -anchor nw -padx 40 -pady 2 -fill x", 
      this->ElementPrintVariablesEntry->GetWidgetName());
    this->ElementPrintVariablesEntry->SetEnabled(0);
    }
    
  if (!this->ElementOutputButton)
    {
    this->ElementOutputButton = vtkKWCheckButtonWithLabel::New();
    this->ElementOutputButton->SetParent( this->ElementPrintFrame->GetFrame() );
    this->ElementOutputButton->Create();
    this->ElementOutputButton->SetLabelText("*ELEMENT OUTPUT,");
    this->ElementOutputButton->SetLabelPositionToRight();
    this->ElementOutputButton->GetWidget()->SetCommand(this, "ElementOutputCallback");
    this->GetApplication()->Script(
      "pack %s -side top -anchor nw -padx 20 -pady 2", 
      this->ElementOutputButton->GetWidgetName());
    } 
  
  if (!this->ElementOutputPositionMenu)
    {
    this->ElementOutputPositionMenu = vtkKWMenuButtonWithLabel::New();
    this->ElementOutputPositionMenu->SetParent(this->ElementPrintFrame->GetFrame());
    this->ElementOutputPositionMenu->Create();
    this->ElementOutputPositionMenu->GetWidget()->SetWidth(25);
    this->ElementOutputPositionMenu->SetLabelText( "POSITION= " );
    this->ElementOutputPositionMenu->GetWidget()->GetMenu()->AddRadioButton("Centroidal");
    this->ElementOutputPositionMenu->GetWidget()->GetMenu()->AddRadioButton("Integration Points");
    this->ElementOutputPositionMenu->GetWidget()->GetMenu()->AddRadioButton("Nodes");
    this->ElementOutputPositionMenu->GetWidget()->SetValue("Centroidal");
    this->GetApplication()->Script(
      "pack %s -side top -anchor nw -padx 40 -pady 2", 
      this->ElementOutputPositionMenu->GetWidgetName());
    this->ElementOutputPositionMenu->SetEnabled(0);
    }
    
  if (!this->ElementOutputVariablesEntry)
    {
    this->ElementOutputVariablesEntry = vtkKWEntryWithLabel::New();
    this->ElementOutputVariablesEntry->SetParent(this->ElementPrintFrame->GetFrame());
    this->ElementOutputVariablesEntry->Create();
    this->ElementOutputVariablesEntry->SetLabelText("Variable(s) of interest: ");
    this->GetApplication()->Script(
      "pack %s -side top -anchor nw -padx 40 -pady 2 -fill x", 
      this->ElementOutputVariablesEntry->GetWidgetName());
    this->ElementOutputVariablesEntry->SetEnabled(0);
    }
  
  if (!this->ElementSetOutputApplyButton)
    {
    this->ElementSetOutputApplyButton = vtkKWPushButton::New();
    this->ElementSetOutputApplyButton->SetParent( this->ElementPrintFrame->GetFrame());
    this->ElementSetOutputApplyButton->Create();
    this->ElementSetOutputApplyButton->SetReliefToFlat();
    this->ElementSetOutputApplyButton->SetImageToIcon( applyIcon );
    this->ElementSetOutputApplyButton->SetCommand(this, "ElementSetOutputApplyCallback");
    this->GetApplication()->Script("pack %s -side top -anchor ne -padx 2 -pady 2",
                                   this->ElementSetOutputApplyButton->GetWidgetName() ); 
    }
  this->OutputElementSetChangedCallback(this->ElementSetMenu->GetWidget()->GetValue());

  /**************************************************************/
  if (!this->StepControlCancelButton)
    {
    this->StepControlCancelButton = vtkKWPushButton::New();
    this->StepControlCancelButton->SetParent( this->StepControlDialog );
    this->StepControlCancelButton->Create();
    this->StepControlCancelButton->SetReliefToFlat();
    this->StepControlCancelButton->SetImageToIcon( this->GetMimxMainWindow()->GetCancelButtonIcon() );
    this->StepControlCancelButton->SetCommand(this, "DialogCancelCallback");
    this->GetApplication()->Script("pack %s -side top -anchor n -padx 2 -pady 2",
                                   this->StepControlCancelButton->GetWidgetName() ); 
    }
  
  this->AssignValuesToStepWindow(this->GetNotebookStep());
  this->StepControlDialog->Display();
  applyIcon->Delete();
  return;
}
//---------------------------------------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::NodePrintCallback(int mode)
{
  if ( mode )
    {
    this->NodePrintFrequencyEntry->SetEnabled(1);
    this->NodePrintSummaryButton->SetEnabled(1);
    this->NodePrintTotalButton->SetEnabled(1);
    this->NodePrintVariablesEntry->SetEnabled(1);
    }
  else
    {
    this->NodePrintFrequencyEntry->SetEnabled(0);
    this->NodePrintSummaryButton->SetEnabled(0);
    this->NodePrintTotalButton->SetEnabled(0);
    this->NodePrintVariablesEntry->SetEnabled(0);
    }
}
//---------------------------------------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::NodeOutputCallback(int mode)
{
  if ( mode )
    {
    this->NodeOutputVariablesEntry->SetEnabled(1);
    }
  else
    {
    this->NodeOutputVariablesEntry->SetEnabled(0);
    }
}
//---------------------------------------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::ElementPrintCallback(int mode)
{
  if ( mode )
    {
    this->ElementPrintFrequencyEntry->SetEnabled(1);
    this->ElementPositionSetMenu->SetEnabled(1);
    this->ElementPrintSummaryButton->SetEnabled(1);
    this->ElementPrintTotalButton->SetEnabled(1);
    this->ElementPrintVariablesEntry->SetEnabled(1);
    }
  else
    {
    this->ElementPrintFrequencyEntry->SetEnabled(0);
    this->ElementPositionSetMenu->SetEnabled(0);
    this->ElementPrintSummaryButton->SetEnabled(0);
    this->ElementPrintTotalButton->SetEnabled(0);
    this->ElementPrintVariablesEntry->SetEnabled(0);
    }
}
//---------------------------------------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::ElementOutputCallback(int mode)
{
  if ( mode )
    {
    this->ElementOutputPositionMenu->SetEnabled(1);
    this->ElementOutputVariablesEntry->SetEnabled(1);
    }
  else
    {
    this->ElementOutputPositionMenu->SetEnabled(0);
    this->ElementOutputVariablesEntry->SetEnabled(0);
    }
}
//---------------------------------------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::DialogApplyCallback()
{
  vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();

  vtkUnstructuredGrid *ugrid = this->IsMeshValid();
  if(!ugrid)    return;
  //
  int activePage = this->GetNotebookStep();
  if(!activePage)
    {
    callback->ErrorMessage("Invalid Step Number");
    return;
    }
  // step input parameters
  vtkStringArray *stepinput = vtkStringArray::New();
    
  char step[256];
  char stepnum[16];
  sprintf(stepnum, "%d", activePage); 
  strcpy(step, "Step");    strcat(step, "_");strcat(step, stepnum);
  strcat(step, "_");    strcat(step, "Input_Parameters");
  stepinput->SetName(step);

  stepinput->InsertNextValue(this->StepIncrementEntry->GetWidget()->GetValue());
  stepinput->InsertNextValue(this->AmplitudeTypeMenu->GetWidget()->GetValue());
  if(this->NlGeomButton->GetWidget()->GetSelectedState())
    {
    stepinput->InsertNextValue("YES");
    }
  else
    {
    stepinput->InsertNextValue("NO");
    }

  if(this->UnsymmButton->GetWidget()->GetSelectedState())
    {
    stepinput->InsertNextValue("YES");
    }
  else
    {
    stepinput->InsertNextValue("NO");
    }

  const char *analysistype = this->AnalysisTypeMenu->GetValue();
    
  if(!strcmp(analysistype, "Static Analysis"))
    stepinput->InsertNextValue("STATIC");
  else
    stepinput->InsertNextValue("DYNAMIC");

  stepinput->InsertNextValue(this->TimeIncrementEntry->GetWidget()->GetValue());
  ugrid->GetFieldData()->AddArray(stepinput);
  stepinput->Delete();
}
//-------------------------------------------------------------------------------------------
vtkUnstructuredGrid* vtkKWMimxAssignBoundaryConditionsGroup::IsMeshValid()
{
  // check the validity of the selected mesh
  vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
  if(!strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
    {
    callback->ErrorMessage("Mesh selection required");
    return NULL;
    }

  vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
  const char *name = combobox->GetValue();

  int num = combobox->GetValueIndex(name);
  if(num < 0 || num > combobox->GetNumberOfValues()-1)
    {
    callback->ErrorMessage("Invalid mesh selected");
    combobox->SetValue("");
    return NULL;
    }

  vtkUnstructuredGrid *ugrid = vtkMimxMeshActor::SafeDownCast(
    this->FEMeshList->GetItem(combobox->GetValueIndex(name)))->GetDataSet();

  return ugrid;
}
//----------------------------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::NodeSetOutputApplyCallback()
{
  vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
  if(!strcmp(this->NodeSetMenu->GetWidget()->GetValue(),""))
    {
    callback->ErrorMessage("Node Set selection required");
    return ;
    }
  const char *nodesetname = this->NodeSetMenu->GetWidget()->GetValue();

  vtkUnstructuredGrid *ugrid = this->IsMeshValid();
  if(!ugrid)    return;
  //
  int activePage = this->GetNotebookStep();
  if(!activePage)
    {
    callback->ErrorMessage("Invalid Step Number");
    return;
    }
  // step step print parameters

  char step[256];
  char stepnum[16];
  sprintf(stepnum, "%d", activePage); 
  strcpy(step, "Step");    strcat(step, "_");strcat(step, stepnum);
  strcat(step, "_");    strcat(step, "Node_Set_");
  strcat(step, nodesetname); strcat(step, "_Print");
  if(this->NodePrintButton->GetWidget()->GetSelectedState())
    {
    vtkStringArray *stepinput = vtkStringArray::New();
    stepinput->SetName(step);
    stepinput->InsertNextValue( "YES" );
    std::string tmpStr = this->NodePrintFrequencyEntry->GetWidget()->GetValue();
    int frequencyValue = this->NodePrintFrequencyEntry->GetWidget()->GetValueAsInt();
    if (tmpStr == "")
      {
      callback->ErrorMessage("Please specify a freqency for printing.");
      stepinput->Delete();
      return;
      }
    if (frequencyValue < 0)
      {
      callback->ErrorMessage("Please specify a value greater than or equal to 0 for print freqency.");
      stepinput->Delete();
      return;
      }
    stepinput->InsertNextValue( tmpStr );
    if(this->NodePrintSummaryButton->GetWidget()->GetSelectedState())
      {
      stepinput->InsertNextValue("YES");
      }
    else
      {
      stepinput->InsertNextValue("NO");
      }
    if(this->NodePrintTotalButton->GetWidget()->GetSelectedState())
      {
      stepinput->InsertNextValue("YES");
      }
    else
      {
      stepinput->InsertNextValue("NO");
      }
    stepinput->InsertNextValue(this->NodePrintVariablesEntry->GetWidget()->GetValue());
    ugrid->GetFieldData()->AddArray(stepinput);
    stepinput->Delete();
    }
  else
    {
    vtkStringArray *stepinput = vtkStringArray::New();
    stepinput->SetName(step);
    stepinput->InsertNextValue( "NO" );
    ugrid->GetFieldData()->AddArray(stepinput);
    stepinput->Delete();
    }

  //stepnum[16];
  sprintf(stepnum, "%d", activePage); 
  strcpy(step, "Step");    strcat(step, "_");strcat(step, stepnum);
  strcat(step, "_");    strcat(step, "Node_Set_");
  strcat(step, nodesetname); strcat(step, "_Output");
  if(this->NodeOutputButton->GetWidget()->GetSelectedState())
    {
    vtkStringArray *stepinput = vtkStringArray::New();
    stepinput->SetName(step);
    stepinput->InsertNextValue( "YES" );
    stepinput->InsertNextValue(this->NodeOutputVariablesEntry->GetWidget()->GetValue());
    ugrid->GetFieldData()->AddArray(stepinput);
    stepinput->Delete();
    }
  else
    {
    vtkStringArray *stepinput = vtkStringArray::New();
    stepinput->SetName(step);
    stepinput->InsertNextValue( "NO" );
    ugrid->GetFieldData()->AddArray(stepinput);
    stepinput->Delete();
    }

  this->NodePrintButton->SetEnabled( 0 );
  this->NodePrintFrequencyEntry->SetEnabled( 0 );
  this->NodePrintSummaryButton->SetEnabled( 0 );
  this->NodePrintTotalButton->SetEnabled( 0 );
  this->NodePrintVariablesEntry->SetEnabled( 0 );
  this->NodeOutputButton->SetEnabled( 0 );
  this->NodeOutputVariablesEntry->SetEnabled( 0 );
}
//----------------------------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::ElementSetOutputApplyCallback()
{
  vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
  if(!strcmp(this->ElementSetMenu->GetWidget()->GetValue(),""))
    {
    callback->ErrorMessage("Element Set selection required");
    return ;
    }
  const char *ElementSetname = this->ElementSetMenu->GetWidget()->GetValue();

  vtkUnstructuredGrid *ugrid = this->IsMeshValid();
  if(!ugrid)    return;
  //
  int activePage = this->GetNotebookStep();
  if(!activePage)
    {
    callback->ErrorMessage("Invalid Step Number");
    return;
    }
  // step step print parameters

  char step[256];
  char stepnum[16];
  sprintf(stepnum, "%d", activePage); 
  strcpy(step, "Step");    strcat(step, "_");strcat(step, stepnum);
  strcat(step, "_");    strcat(step, "Element_Set_");
  strcat(step, ElementSetname); strcat(step, "_Print");
  if(this->ElementPrintButton->GetWidget()->GetSelectedState())
    {
    vtkStringArray *stepinput = vtkStringArray::New();
    stepinput->SetName(step);
    stepinput->InsertNextValue( "YES" );
    std::string tmpStr = this->ElementPrintFrequencyEntry->GetWidget()->GetValue();
    int frequencyValue = this->ElementPrintFrequencyEntry->GetWidget()->GetValueAsInt();
    if (tmpStr == "")
      {
      callback->ErrorMessage("Please specify a freqency for printing.");
      stepinput->Delete();
      return;
      }
    if (frequencyValue < 0)
      {
      callback->ErrorMessage("Please specify a value greater than or equal to 0 for print freqency.");
      stepinput->Delete();
      return;
      }
    stepinput->InsertNextValue( tmpStr );
    stepinput->InsertNextValue(this->ElementPositionSetMenu->GetWidget()->GetValue());
    if(this->ElementPrintSummaryButton->GetWidget()->GetSelectedState())
      {
      stepinput->InsertNextValue("YES");
      }
    else
      {
      stepinput->InsertNextValue("NO");
      }
    if(this->ElementPrintTotalButton->GetWidget()->GetSelectedState())
      {
      stepinput->InsertNextValue("YES");
      }
    else
      {
      stepinput->InsertNextValue("NO");
      }
    stepinput->InsertNextValue(this->ElementPrintVariablesEntry->GetWidget()->GetValue());
    ugrid->GetFieldData()->AddArray(stepinput);
    stepinput->Delete();
    }
  else
    {
    vtkStringArray *stepinput = vtkStringArray::New();
    stepinput->SetName(step);
    stepinput->InsertNextValue( "NO" );
    ugrid->GetFieldData()->AddArray(stepinput);
    stepinput->Delete();
    }

  //stepnum[16];
  sprintf(stepnum, "%d", activePage); 
  strcpy(step, "Step");    strcat(step, "_");strcat(step, stepnum);
  strcat(step, "_");    strcat(step, "Element_Set_");
  strcat(step, ElementSetname); strcat(step, "_Output");
  if(this->ElementOutputButton->GetWidget()->GetSelectedState())
    {
    vtkStringArray *stepinput = vtkStringArray::New();
    stepinput->SetName(step);
    stepinput->InsertNextValue( "YES" );
    stepinput->InsertNextValue(this->ElementOutputPositionMenu->GetWidget()->GetValue());
    stepinput->InsertNextValue(this->ElementOutputVariablesEntry->GetWidget()->GetValue());
    ugrid->GetFieldData()->AddArray(stepinput);
    stepinput->Delete();
    }
  else
    {
    vtkStringArray *stepinput = vtkStringArray::New();
    stepinput->SetName(step);
    stepinput->InsertNextValue( "NO" );
    ugrid->GetFieldData()->AddArray(stepinput);
    stepinput->Delete();
    }
  this->ElementPrintButton->SetEnabled(0);
  this->ElementPrintFrequencyEntry->SetEnabled(0);
  this->ElementPositionSetMenu->SetEnabled(0);
  this->ElementPrintSummaryButton->SetEnabled(0);
  this->ElementPrintTotalButton->SetEnabled(0);
  this->ElementPrintVariablesEntry->SetEnabled(0);
  this->ElementOutputButton->SetEnabled(0);
  this->ElementOutputPositionMenu->SetEnabled(0);
  this->ElementOutputVariablesEntry->SetEnabled(0);
}
//----------------------------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::GenerateStepParameters(int StepNum)
{
  vtkUnstructuredGrid *ugrid = this->IsMeshValid();
  if(!ugrid)    return;
  // obtain subheading values if present
  char subheading[256];
  char stepnum[16];
  sprintf(stepnum, "%d", StepNum); 
  strcpy(subheading, "Step_");    strcat(subheading, stepnum);
  strcat(subheading, "_SubHeading");
  if(ugrid->GetFieldData()->GetAbstractArray(subheading))
    {
    vtkStringArray *stringarray = vtkStringArray::SafeDownCast(
      ugrid->GetFieldData()->GetAbstractArray(subheading));
    this->SubHeadingEntry->SetValue(stringarray->GetValue(0));
    }
  else
    {
    this->SubHeadingEntry->SetValue("");
    }
  // step input parameters
  char step[256];
  strcpy(step, "Step");    strcat(step, "_");strcat(step, stepnum);
  strcat(step, "_");    strcat(step, "Input_Parameters");

  if(ugrid->GetFieldData()->GetAbstractArray(step))    return;
    
  if (StepNum == 1)
    {
    vtkStringArray *stepinput = vtkStringArray::New();
    stepinput->SetName(step);
    stepinput->InsertNextValue("100");
    stepinput->InsertNextValue("Default");
    stepinput->InsertNextValue("YES");
    stepinput->InsertNextValue("YES");
    stepinput->InsertNextValue("STATIC");
    stepinput->InsertNextValue("1.0, 1.0");
    ugrid->GetFieldData()->AddArray(stepinput);
    stepinput->Delete();
    }
  else
    {
    vtkStringArray *stringarray = this->GetLowerStepWithParameters(StepNum, ugrid);
    vtkStringArray *stepinput = vtkStringArray::New();
    stepinput->SetName(step);
    stepinput->DeepCopy(stringarray);
    ugrid->GetFieldData()->AddArray(stepinput);
    stepinput->Delete();
    this->CopyLowerStepNodeAndElementOutput(StepNum, ugrid);
    }
}
//----------------------------------------------------------------------------------------------
vtkStringArray* vtkKWMimxAssignBoundaryConditionsGroup::GetLowerStepWithParameters(
  int StepNum, vtkUnstructuredGrid *ugrid)
{
  int i;
  for (i=StepNum-1; i>0; i--)
    {
    char step[256];
    char stepnum[16];
    sprintf(stepnum, "%d", i); 
    strcpy(step, "Step");    strcat(step, "_");strcat(step, stepnum);
    strcat(step, "_");    strcat(step, "Input_Parameters");
    if(ugrid->GetFieldData()->GetAbstractArray(step))
      {
      return vtkStringArray::SafeDownCast(
        ugrid->GetFieldData()->GetAbstractArray(step));
      }
    }
  return NULL;
}
//-----------------------------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::AssignValuesToStepWindow(int StepNum)
{
  vtkUnstructuredGrid *ugrid = this->IsMeshValid();
  if(!ugrid)    return;
  // step input parameters
  char step[256];
  char stepnum[16];
  sprintf(stepnum, "%d", StepNum); 
  strcpy(step, "Step");    strcat(step, "_");strcat(step, stepnum);
  strcat(step, "_");    strcat(step, "Input_Parameters");

  vtkStringArray *stringarray = vtkStringArray::SafeDownCast(ugrid->GetFieldData()->GetAbstractArray(step));
  this->StepIncrementEntry->GetWidget()->SetValue(stringarray->GetValue(0));
  this->AmplitudeTypeMenu->GetWidget()->SetValue(stringarray->GetValue(1));
  if(stringarray->GetValue(2) == "YES")
    {
    this->NlGeomButton->GetWidget()->SetSelectedState(1);
    }
  else
    {
    this->NlGeomButton->GetWidget()->SetSelectedState(0);
    }
  if(stringarray->GetValue(3) == "YES")
    {
    this->UnsymmButton->GetWidget()->SetSelectedState(1);
    }
  else
    {
    this->UnsymmButton->GetWidget()->SetSelectedState(0);
    }

  if(stringarray->GetValue(4) == "STATIC")
    this->AnalysisTypeMenu->SetValue("Static Analysis");
  else
    this->AnalysisTypeMenu->SetValue("Dynamic Analysis");
    
  this->TimeIncrementEntry->GetWidget()->SetValue(stringarray->GetValue(5));

  // node and element print and output entries
  this->OutputNodeSetChangedCallback(this->NodeSetMenu->GetWidget()->GetValue());
  this->OutputElementSetChangedCallback(this->ElementSetMenu->GetWidget()->GetValue());
}
//--------------------------------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::OutputNodeSetChangedCallback(const char *Entry)
{
  if (strlen(Entry) == 0)    return;
    
  if (strlen(this->NodeSetMenu->GetWidget()->GetValue()) == 0)
    {
    return;
    }
  const char *nodesetname = this->NodeSetMenu->GetWidget()->GetValue();

  vtkUnstructuredGrid *ugrid = this->IsMeshValid();
  if (!ugrid)    return;
  //
  int activePage = this->GetNotebookStep();
  if (!activePage)
    {
    return;
    }
  // step step print parameters
  char step[256];
  char stepnum[16];
  sprintf(stepnum, "%d", activePage); 
  strcpy(step, "Step");    strcat(step, "_");strcat(step, stepnum);
  strcat(step, "_");    strcat(step, "Node_Set_");
  strcat(step, nodesetname); strcat(step, "_Print");
  if (ugrid->GetFieldData()->GetAbstractArray(step))
    {
    vtkStringArray *stringarray = vtkStringArray::SafeDownCast(
      ugrid->GetFieldData()->GetAbstractArray(step));
        
    if (stringarray->GetValue(0) == "YES")
      {
      this->NodePrintButton->GetWidget()->SetSelectedState(1);
      this->NodePrintFrequencyEntry->GetWidget()->SetValue(stringarray->GetValue(1));
      if (stringarray->GetValue(2) == "YES")
        this->NodePrintSummaryButton->GetWidget()->SetSelectedState(1);
      else
        this->NodePrintSummaryButton->GetWidget()->SetSelectedState(0);

      if (stringarray->GetValue(3) == "YES")
        this->NodePrintTotalButton->GetWidget()->SetSelectedState(1);
      else
        this->NodePrintTotalButton->GetWidget()->SetSelectedState(0);    

      this->NodePrintVariablesEntry->GetWidget()->SetValue(stringarray->GetValue(4));
      }
    else
      {
      this->NodePrintButton->GetWidget()->SetSelectedState(0);
      }
    }
  else
    {
    this->NodePrintButton->GetWidget()->SetSelectedState(1);
    this->NodePrintFrequencyEntry->GetWidget()->SetValue( "1" );
    this->NodePrintSummaryButton->GetWidget()->SetSelectedState(1);
    this->NodePrintTotalButton->GetWidget()->SetSelectedState(1);
    this->NodePrintVariablesEntry->GetWidget()->SetValue( "U,RF" );
    }

  //stepnum[16];
  //step[256];
  sprintf(stepnum, "%d", activePage); 
  strcpy(step, "Step");    strcat(step, "_");strcat(step, stepnum);
  strcat(step, "_");    strcat(step, "Node_Set_");
  strcat(step, nodesetname); strcat(step, "_Output");
  if (ugrid->GetFieldData()->GetAbstractArray(step))
    {
    vtkStringArray *stringarray = vtkStringArray::SafeDownCast( ugrid->GetFieldData()->GetAbstractArray(step));
    if (stringarray->GetValue(0) == "YES" )
      {
      this->NodeOutputButton->GetWidget()->SetSelectedState(1);          
      this->NodeOutputVariablesEntry->GetWidget()->SetValue(stringarray->GetValue(1));
      }
    else
      {
      this->NodeOutputButton->GetWidget()->SetSelectedState(0);
      }
    }
  else
    {
    this->NodeOutputButton->GetWidget()->SetSelectedState(1);
    this->NodeOutputVariablesEntry->GetWidget()->SetValue( "ALL" );
    this->NodeOutputButton->SetEnabled( 1 );
    this->NodeOutputVariablesEntry->SetEnabled( 1 );
    }
        
  this->NodePrintButton->SetEnabled( 1 );
  if ( this->NodePrintButton->GetWidget()->GetSelectedState( ) )
    {
    this->NodePrintFrequencyEntry->SetEnabled( 1 );
    this->NodePrintSummaryButton->SetEnabled( 1 );
    this->NodePrintTotalButton->SetEnabled( 1 );
    this->NodePrintVariablesEntry->SetEnabled( 1 );
    }
  else
    {
    this->NodePrintFrequencyEntry->SetEnabled( 0 );
    this->NodePrintSummaryButton->SetEnabled( 0 );
    this->NodePrintTotalButton->SetEnabled( 0 );
    this->NodePrintVariablesEntry->SetEnabled( 0 );
    }
  this->NodeOutputButton->SetEnabled( 1 );
  if ( this->NodeOutputButton->GetWidget()->GetSelectedState( ) )
    {
    this->NodeOutputVariablesEntry->SetEnabled( 1 );
    }
  else
    {
    this->NodeOutputVariablesEntry->SetEnabled( 0 );
    }
  
}
//--------------------------------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::OutputElementSetChangedCallback(const char *Entry)
{
  if (strlen(Entry) == 0)    return;

  const char *ElementSetname = this->ElementSetMenu->GetWidget()->GetValue();
  if (strlen(ElementSetname) == 0)    return;
  vtkUnstructuredGrid *ugrid = this->IsMeshValid();
  if(!ugrid)    return;
  //
  int activePage = this->GetNotebookStep();
  // step step print parameters

  char step[256];
  char stepnum[16];
  sprintf(stepnum, "%d", activePage); 
  strcpy(step, "Step");    strcat(step, "_");strcat(step, stepnum);
  strcat(step, "_");    strcat(step, "Element_Set_");
  strcat(step, ElementSetname); strcat(step, "_Print");
  if(ugrid->GetFieldData()->GetAbstractArray(step))
    {
    vtkStringArray *stringarray = vtkStringArray::SafeDownCast(
      ugrid->GetFieldData()->GetAbstractArray(step));
    if (stringarray->GetValue(0) == "YES")
      {
      this->ElementPrintButton->GetWidget()->SetSelectedState(1);
      this->ElementPrintFrequencyEntry->GetWidget()->SetValue(stringarray->GetValue(1));
      this->ElementPositionSetMenu->GetWidget()->SetValue(stringarray->GetValue(2));
      if(stringarray->GetValue(3) == "YES")
        this->ElementPrintSummaryButton->GetWidget()->SetSelectedState(1);
      else
        this->ElementPrintSummaryButton->GetWidget()->SetSelectedState(0);

      if(stringarray->GetValue(4) == "YES")
        this->ElementPrintTotalButton->GetWidget()->SetSelectedState(1);
      else
        this->ElementPrintTotalButton->GetWidget()->SetSelectedState(0);
      this->ElementPrintVariablesEntry->GetWidget()->SetValue(stringarray->GetValue(5));
      }
    else
      {
      this->ElementPrintButton->GetWidget()->SetSelectedState(0);
      }
    }
  else
    {
    this->ElementPrintButton->GetWidget()->SetSelectedState(1);
    this->ElementPrintTotalButton->GetWidget()->SetSelectedState(1);
    this->ElementPrintSummaryButton->GetWidget()->SetSelectedState(1);
    this->ElementPrintFrequencyEntry->GetWidget()->SetValue( "1" );
    this->ElementPositionSetMenu->GetWidget()->SetValue( "Centroidal" );
    this->ElementPrintVariablesEntry->GetWidget()->SetValue( "S,E" );
    }

  //stepnum[16];
  //step[256];
  sprintf(stepnum, "%d", activePage); 
  strcpy(step, "Step");    strcat(step, "_");strcat(step, stepnum);
  strcat(step, "_");    strcat(step, "Element_Set_");
  strcat(step, ElementSetname); strcat(step, "_Output");
  if(ugrid->GetFieldData()->GetAbstractArray(step))
    {
    vtkStringArray *stringarray = vtkStringArray::SafeDownCast(
      ugrid->GetFieldData()->GetAbstractArray(step));
    if (stringarray->GetValue(0) == "YES")
      {
      this->ElementOutputButton->GetWidget()->SetSelectedState(1);
      this->ElementOutputPositionMenu->GetWidget()->SetValue(stringarray->GetValue(1));
      this->ElementOutputVariablesEntry->GetWidget()->SetValue(stringarray->GetValue(2));
      }
    else
      {
      this->ElementOutputButton->GetWidget()->SetSelectedState(0);
      }
    }
  else
    {
    this->ElementOutputButton->GetWidget()->SetSelectedState( 1 );
    this->ElementOutputPositionMenu->GetWidget()->SetValue( "Centroidal" );
    this->ElementOutputVariablesEntry->GetWidget()->SetValue( "ALL" );
    }
    
  this->ElementPrintButton->SetEnabled(1);
  if (this->ElementPrintButton->GetWidget()->GetSelectedState())
    {
    this->ElementPrintFrequencyEntry->SetEnabled(1);
    this->ElementPositionSetMenu->SetEnabled(1);
    this->ElementPrintSummaryButton->SetEnabled(1);
    this->ElementPrintTotalButton->SetEnabled(1);
    this->ElementPrintVariablesEntry->SetEnabled(1);
    }
  else
    {
    this->ElementPrintFrequencyEntry->SetEnabled(0);
    this->ElementPositionSetMenu->SetEnabled(0);
    this->ElementPrintSummaryButton->SetEnabled(0);
    this->ElementPrintTotalButton->SetEnabled(0);
    this->ElementPrintVariablesEntry->SetEnabled(0);
    }
  
  this->ElementOutputButton->SetEnabled(1);
  if (this->ElementOutputButton->GetWidget()->GetSelectedState())
    {
    this->ElementOutputPositionMenu->SetEnabled(1);
    this->ElementOutputVariablesEntry->SetEnabled(1);
    }
  else
    {
    this->ElementOutputPositionMenu->SetEnabled(0);
    this->ElementOutputVariablesEntry->SetEnabled(0);
    }
  
}
//--------------------------------------------------------------------------------------------------
void vtkKWMimxAssignBoundaryConditionsGroup::CopyLowerStepNodeAndElementOutput(
  int StepNum, vtkUnstructuredGrid *Ugrid)
{
  if(StepNum == 1)    return;
  // start with node sets
  int i;
  int prevStep = StepNum -1;
  vtkStringArray *elsetarray = vtkStringArray::SafeDownCast(
    Ugrid->GetFieldData()->GetAbstractArray("Element_Set_Names"));
  if(!elsetarray)    return;

  vtkStringArray *nodesetarray = vtkStringArray::SafeDownCast(
    Ugrid->GetFieldData()->GetAbstractArray("Node_Set_Names"));
  if(!nodesetarray)    return;

  int numNodeSets = nodesetarray->GetNumberOfValues();
  int numElementSets = elsetarray->GetNumberOfValues();
  //
  for (i=0; i<numNodeSets; i++)
    {
    // node print definitions
    const char *nodesetname = nodesetarray->GetValue(i);
    char step[256];
    char stepnum[16];
    sprintf(stepnum, "%d", prevStep); 
    strcpy(step, "Step");    strcat(step, "_");strcat(step, stepnum);
    strcat(step, "_");    strcat(step, "Node_Set_");
    strcat(step, nodesetname); strcat(step, "_Print");
    if(Ugrid->GetFieldData()->GetAbstractArray(step))
      {
      vtkStringArray *stringarray = vtkStringArray::SafeDownCast(
        Ugrid->GetFieldData()->GetAbstractArray(step));
      //step[256];
      //stepnum[16];
      sprintf(stepnum, "%d", StepNum);
      strcpy(step, "Step");    strcat(step, "_");strcat(step, stepnum);
      strcat(step, "_");    strcat(step, "Node_Set_");
      strcat(step, nodesetname); strcat(step, "_Print");
      vtkStringArray *nodearray = vtkStringArray::New();
      nodearray->SetName(step);
      nodearray->DeepCopy(stringarray);
      Ugrid->GetFieldData()->AddArray(nodearray);
      nodearray->Delete();
      }
    //stepnum[16];
    //step[256];
    sprintf(stepnum, "%d", prevStep); 
    strcpy(step, "Step");    strcat(step, "_");strcat(step, stepnum);
    strcat(step, "_");    strcat(step, "Node_Set_");
    strcat(step, nodesetname); strcat(step, "_Output");
    if(Ugrid->GetFieldData()->GetAbstractArray(step))
      {
      vtkStringArray *stringarray = vtkStringArray::SafeDownCast(
        Ugrid->GetFieldData()->GetAbstractArray(step));
      //stepnum[16];
      //step[256];
      sprintf(stepnum, "%d", StepNum); 
      strcpy(step, "Step");    strcat(step, "_");strcat(step, stepnum);
      strcat(step, "_");    strcat(step, "Node_Set_");
      strcat(step, nodesetname); strcat(step, "_Output");
      vtkStringArray *nodearray = vtkStringArray::New();
      nodearray->SetName(step);
      nodearray->DeepCopy(stringarray);
      Ugrid->GetFieldData()->AddArray(nodearray);
      nodearray->Delete();
      }
    }
  // element sets
  for (i=0; i<numElementSets; i++)
    {
    // element print definitions
    const char *elementsetname = elsetarray->GetValue(i);
    char step[256];
    char stepnum[16];
    sprintf(stepnum, "%d", prevStep); 
    strcpy(step, "Step");    strcat(step, "_");strcat(step, stepnum);
    strcat(step, "_");    strcat(step, "Element_Set_");
    strcat(step, elementsetname); strcat(step, "_Print");
    if(Ugrid->GetFieldData()->GetAbstractArray(step))
      {
      vtkStringArray *stringarray = vtkStringArray::SafeDownCast(
        Ugrid->GetFieldData()->GetAbstractArray(step));
      //step[256];
      //stepnum[16];
      sprintf(stepnum, "%d", StepNum);
      strcpy(step, "Step");    strcat(step, "_");strcat(step, stepnum);
      strcat(step, "_");    strcat(step, "Element_Set_");
      strcat(step, elementsetname); strcat(step, "_Print");
      vtkStringArray *elementarray = vtkStringArray::New();
      elementarray->SetName(step);
      elementarray->DeepCopy(stringarray);
      Ugrid->GetFieldData()->AddArray(elementarray);
      elementarray->Delete();
      }
    //stepnum[16];
    //step[256];
    sprintf(stepnum, "%d", prevStep); 
    strcpy(step, "Step");    strcat(step, "_");strcat(step, stepnum);
    strcat(step, "_");    strcat(step, "Element_Set_");
    strcat(step, elementsetname); strcat(step, "_Output");
    if(Ugrid->GetFieldData()->GetAbstractArray(step))
      {
      vtkStringArray *stringarray = vtkStringArray::SafeDownCast(
        Ugrid->GetFieldData()->GetAbstractArray(step));
      //stepnum[16];
      //step[256];
      sprintf(stepnum, "%d", StepNum); 
      strcpy(step, "Step");    strcat(step, "_");strcat(step, stepnum);
      strcat(step, "_");    strcat(step, "Element_Set_");
      strcat(step, elementsetname); strcat(step, "_Output");
      vtkStringArray *elementarray = vtkStringArray::New();
      elementarray->SetName(step);
      elementarray->DeepCopy(stringarray);
      Ugrid->GetFieldData()->AddArray(elementarray);
      elementarray->Delete();
      }
    }
}
//--------------------------------------------------------------------------------------------------
