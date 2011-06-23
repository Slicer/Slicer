/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#include "vtkProstateNavTargetingStep.h"

#include "vtkProstateNavGUI.h"
#include "vtkProstateNavLogic.h"

#include "vtkKWMatrixWidget.h"
#include "vtkKWMatrixWidgetWithLabel.h"
#include "vtkSlicerNodeSelectorWidget.h"

#include "vtkSlicerFiducialsGUI.h"
#include "vtkSlicerFiducialsLogic.h"
#include "vtkMRMLSelectionNode.h"

#include "vtkMRMLLinearTransformNode.h"
#include "igtlMath.h"

#include "vtkMRMLRobotNode.h"

////

#include "vtkProstateNavTargetDescriptor.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkMRMLFiducialListNode.h"
#include "vtkMRMLSliceNode.h"
#include "vtkSlicerVolumesGUI.h"
#include "vtkMRMLInteractionNode.h"

#include "vtkKWFrame.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWFrame.h"
#include "vtkKWEntry.h"
#include "vtkKWLabel.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWEntrySet.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWText.h"
#include "vtkKWPushButton.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWCheckButton.h"

const char TARGET_INDEX_ATTR[]="TARGET_IND";

#define DELETE_IF_NULL_WITH_SETPARENT_NULL(obj) \
  if (obj) \
    { \
    obj->SetParent(NULL); \
    obj->Delete(); \
    obj = NULL; \
    };

// Definition of target list columns
enum
{
  COL_NAME = 0,
  COL_X,
  COL_Y,
  COL_Z,
  COL_NEEDLE,
  COL_OR_W,
  COL_OR_X,
  COL_OR_Y,
  COL_OR_Z,
  COL_COUNT // all valid columns should be inserted above this line
};
static const char* COL_LABELS[COL_COUNT] = { "Name", "R", "A", "S", "Needle", "OrW", "OrX", "OrY", "OrZ" };
static const int COL_WIDTHS[COL_COUNT] = { 8, 6, 6, 6, 10, 6, 6, 6, 6 };


//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkProstateNavTargetingStep);
vtkCxxRevisionMacro(vtkProstateNavTargetingStep, "$Revision: 1.1 $");

//----------------------------------------------------------------------------
vtkProstateNavTargetingStep::vtkProstateNavTargetingStep()
{
  //this->SetName("Targeting");
  this->SetTitle("Targeting");
  this->SetDescription("Set target points.");

  this->MainFrame=NULL;
  
  // TargetPlanning frame
  this->TargetPlanningFrame=NULL;
  this->LoadTargetingVolumeButton=NULL;
  this->VolumeSelectorWidget=NULL;
  this->TargetPlanningFrame=NULL;
  this->ShowCoverageButton=NULL;
  this->AddTargetsOnClickButton=NULL;
  this->NeedleTypeMenuList=NULL; 

  // TargetList frame
  this->TargetListFrame=NULL;
  this->TargetList=NULL;
  this->DeleteButton=NULL;

  // TargetControl frame
  this->TargetControlFrame=NULL;
  this->NeedlePositionMatrix=NULL;
  this->NeedleOrientationMatrix=NULL;
  this->MoveButton=NULL;
  this->StopButton=NULL;

  this->Message=NULL;

  this->TitleBackgroundColor[0] = 0.8;
  this->TitleBackgroundColor[1] = 0.8;
  this->TitleBackgroundColor[2] = 0.8;

  this->ProcessingCallback = false;

  this->TargetPlanListNode=NULL;

  this->ShowTargetOrientation = 0;
}

//----------------------------------------------------------------------------
vtkProstateNavTargetingStep::~vtkProstateNavTargetingStep()
{
  RemoveGUIObservers();

  DELETE_IF_NULL_WITH_SETPARENT_NULL(MainFrame);
  
  // TargetPlanning
  DELETE_IF_NULL_WITH_SETPARENT_NULL(TargetPlanningFrame);
  DELETE_IF_NULL_WITH_SETPARENT_NULL(LoadTargetingVolumeButton);
  DELETE_IF_NULL_WITH_SETPARENT_NULL(VolumeSelectorWidget);
  DELETE_IF_NULL_WITH_SETPARENT_NULL(TargetPlanningFrame);
  DELETE_IF_NULL_WITH_SETPARENT_NULL(ShowCoverageButton);
  DELETE_IF_NULL_WITH_SETPARENT_NULL(AddTargetsOnClickButton);
  DELETE_IF_NULL_WITH_SETPARENT_NULL(NeedleTypeMenuList); 

  // TargetList frame
  DELETE_IF_NULL_WITH_SETPARENT_NULL(TargetListFrame);
  DELETE_IF_NULL_WITH_SETPARENT_NULL(TargetList);
  DELETE_IF_NULL_WITH_SETPARENT_NULL(DeleteButton);

  // TargetControl frame
  DELETE_IF_NULL_WITH_SETPARENT_NULL(TargetControlFrame);
  DELETE_IF_NULL_WITH_SETPARENT_NULL(NeedlePositionMatrix);
  DELETE_IF_NULL_WITH_SETPARENT_NULL(NeedleOrientationMatrix);
  DELETE_IF_NULL_WITH_SETPARENT_NULL(MoveButton);
  DELETE_IF_NULL_WITH_SETPARENT_NULL(StopButton);

  DELETE_IF_NULL_WITH_SETPARENT_NULL(Message);
}

//----------------------------------------------------------------------------
void vtkProstateNavTargetingStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  this->ShowTargetPlanningFrame();
  this->ShowTargetListFrame();
  this->ShowTargetControlFrame();

  AddMRMLObservers();
  
  this->AddGUIObservers();

  EnableAddTargetsOnClickButton(this->AddTargetsOnClickButton->GetSelectedState()==1);
}

//----------------------------------------------------------------------------
void vtkProstateNavTargetingStep::ShowTargetPlanningFrame()
{
  vtkKWWidget *parent = this->GetGUI()->GetWizardWidget()->GetClientArea();

  if (!this->TargetPlanningFrame)
    {
    this->TargetPlanningFrame = vtkKWFrame::New();
    }
  if (!this->TargetPlanningFrame->IsCreated())
    {
    this->TargetPlanningFrame->SetParent(parent);
    this->TargetPlanningFrame->Create();
    }

  this->Script("pack %s -side top -anchor nw -fill x -padx 0 -pady 2",
               this->TargetPlanningFrame->GetWidgetName());
  
  if (!this->LoadTargetingVolumeButton)
    {
     this->LoadTargetingVolumeButton = vtkKWPushButton::New();
    }
  if (!this->LoadTargetingVolumeButton->IsCreated())
    {
    this->LoadTargetingVolumeButton->SetParent(this->TargetPlanningFrame);
    this->LoadTargetingVolumeButton->Create();
    this->LoadTargetingVolumeButton->SetBorderWidth(2);
    this->LoadTargetingVolumeButton->SetReliefToRaised();       
    this->LoadTargetingVolumeButton->SetHighlightThickness(2);
    this->LoadTargetingVolumeButton->SetBackgroundColor(0.85,0.85,0.85);
    this->LoadTargetingVolumeButton->SetActiveBackgroundColor(1,1,1);        
    this->LoadTargetingVolumeButton->SetText( "Load volume");
    this->LoadTargetingVolumeButton->SetBalloonHelpString("Click to load a volume. Need to additionally select the volume to make it the current targeting volume.");
    }

  if (!this->VolumeSelectorWidget)
    {
     this->VolumeSelectorWidget = vtkSlicerNodeSelectorWidget::New();
    }
  if (!this->VolumeSelectorWidget->IsCreated())
    {
    this->VolumeSelectorWidget->SetParent(this->TargetPlanningFrame);
    this->VolumeSelectorWidget->Create();
    this->VolumeSelectorWidget->SetBorderWidth(2);  
    this->VolumeSelectorWidget->SetNodeClass("vtkMRMLVolumeNode", NULL, NULL, NULL);
    this->VolumeSelectorWidget->SetMRMLScene(this->GetLogic()->GetApplicationLogic()->GetMRMLScene());
    this->VolumeSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
    this->VolumeSelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
    this->VolumeSelectorWidget->SetLabelText( "Targeting Volume: ");
    this->VolumeSelectorWidget->SetBalloonHelpString("Select the targeting volume from the current scene.");
    }

  if (!this->ShowCoverageButton)
  {
    this->ShowCoverageButton = vtkKWCheckButton::New();
  } 
  if (!this->ShowCoverageButton->IsCreated()) {
    this->ShowCoverageButton->SetParent(this->TargetPlanningFrame);
    this->ShowCoverageButton->Create();
    this->ShowCoverageButton->SelectedStateOff();
    this->ShowCoverageButton->SetText("Show coverage");
    this->ShowCoverageButton->SetBalloonHelpString("Show coverage volume of the robot");
  }

  if (!this->AddTargetsOnClickButton)
  {
    this->AddTargetsOnClickButton = vtkKWCheckButton::New();
  } 
  if (!this->AddTargetsOnClickButton->IsCreated()) {
    this->AddTargetsOnClickButton->SetParent(this->TargetPlanningFrame);
    this->AddTargetsOnClickButton->Create();
    this->AddTargetsOnClickButton->SelectedStateOff();
    this->AddTargetsOnClickButton->SetText("Add target by image click");
    this->AddTargetsOnClickButton->SetBalloonHelpString("Add a target if image is clicked, with the current needle");
  }

  // add combo box in the frame
  if (!this->NeedleTypeMenuList)
    {
    this->NeedleTypeMenuList = vtkKWMenuButtonWithLabel::New();
    }
  if (!this->NeedleTypeMenuList->IsCreated())
    {
    this->NeedleTypeMenuList->SetParent(this->TargetPlanningFrame);
    this->NeedleTypeMenuList->Create();
    this->NeedleTypeMenuList->SetLabelText("Needle type");
    this->NeedleTypeMenuList->SetBalloonHelpString("Select the needle type");
    }
    
  this->Script("grid %s -row 0 -column 0 -columnspan 2 -padx 2 -pady 2 -sticky ew", this->LoadTargetingVolumeButton->GetWidgetName());
  this->Script("grid %s -row 0 -column 2 -padx 2 -pady 2 -sticky w", this->ShowCoverageButton->GetWidgetName());
  this->Script("grid %s -row 1 -column 0 -columnspan 3 -padx 2 -pady 2 -sticky ew", this->VolumeSelectorWidget->GetWidgetName());
  this->Script("grid %s -row 2 -column 0 -padx 2 -pady 2 -sticky w", this->AddTargetsOnClickButton->GetWidgetName());
  this->Script("grid %s -row 2 -column 1 -columnspan 2 -padx 2 -pady 2 -sticky e", this->NeedleTypeMenuList->GetWidgetName());

}


//----------------------------------------------------------------------------
void vtkProstateNavTargetingStep::ShowTargetListFrame()
{
  vtkKWWidget *parent = this->GetGUI()->GetWizardWidget()->GetClientArea();

  if (!this->TargetListFrame)
    {
    this->TargetListFrame = vtkKWFrame::New();
    }
  if (!this->TargetListFrame->IsCreated())
    {
    this->TargetListFrame->SetParent(parent);
    this->TargetListFrame->Create();
    }
  this->Script("pack %s -side top -anchor nw -expand n -fill x -padx 2 -pady 2",
               this->TargetListFrame->GetWidgetName());

  if (!this->TargetList)
    {
    this->TargetList = vtkKWMultiColumnListWithScrollbars::New();
    this->TargetList->SetParent(this->TargetListFrame);
    this->TargetList->Create();
    this->TargetList->SetHeight(1);
    this->TargetList->GetWidget()->SetSelectionTypeToRow();
    this->TargetList->GetWidget()->SetSelectionBackgroundColor(1,0,0);
    this->TargetList->GetWidget()->MovableRowsOff();
    this->TargetList->GetWidget()->MovableColumnsOff();

    for (int col = 0; col < COL_COUNT; col ++)
      {
      if (this->ShowTargetOrientation || (col!=COL_OR_W && col!=COL_OR_X && col!=COL_OR_Y && col!=COL_OR_Z))
        {
          this->TargetList->GetWidget()->AddColumn(COL_LABELS[col]);
          this->TargetList->GetWidget()->SetColumnWidth(col, COL_WIDTHS[col]);
          this->TargetList->GetWidget()->SetColumnAlignmentToLeft(col);
          this->TargetList->GetWidget()->ColumnEditableOff(col);
        }
      }

    // Set editing options

    this->TargetList->GetWidget()->ColumnEditableOn(COL_NAME);
    this->TargetList->GetWidget()->SetColumnEditWindowToEntry(COL_NAME);

    for (int col = COL_X; col <= COL_Z; col ++)
      {
      this->TargetList->GetWidget()->ColumnEditableOn(col);
      this->TargetList->GetWidget()->SetColumnEditWindowToSpinBox(col);
      }
    if (this->ShowTargetOrientation)
      {
      for (int col = COL_OR_W; col <= COL_OR_Z; col ++)
        {
        this->TargetList->GetWidget()->ColumnEditableOn(col);
        this->TargetList->GetWidget()->SetColumnEditWindowToSpinBox(col);
        }
      }
    }
  this->Script( "pack %s -side top -anchor nw -expand n -fill x -padx 2 -pady 2",
                this->TargetList->GetWidgetName());


  if(!this->DeleteButton)
    {
    this->DeleteButton = vtkKWPushButton::New();
    }
  if(!this->DeleteButton->IsCreated())
    {
    this->DeleteButton->SetParent(this->TargetListFrame);
    this->DeleteButton->SetText("Delete selected target");
    this->DeleteButton->SetBalloonHelpString("Delete selected target point from the target list");
    this->DeleteButton->Create();
    }    
  this->Script("pack %s -side top -anchor ne -padx 2 -pady 4", 
                    this->DeleteButton->GetWidgetName());

}


//----------------------------------------------------------------------------
void vtkProstateNavTargetingStep::ShowTargetControlFrame()
{
  vtkKWWidget *parent = this->GetGUI()->GetWizardWidget()->GetClientArea();

  if (!this->TargetControlFrame)
    {
    this->TargetControlFrame = vtkKWFrame::New();
    }
  if (!this->TargetControlFrame->IsCreated())
    {
    this->TargetControlFrame->SetParent(parent);
    this->TargetControlFrame->Create();
    }
  this->Script("pack %s -side top -anchor nw -expand n -fill x -padx 2 -pady 2",
               this->TargetControlFrame->GetWidgetName());

    if (!this->NeedlePositionMatrix)
    {
    this->NeedlePositionMatrix = vtkKWMatrixWidgetWithLabel::New();
    this->NeedlePositionMatrix->SetParent(this->TargetControlFrame);
    this->NeedlePositionMatrix->Create();
    this->NeedlePositionMatrix->SetLabelText("Position (X, Y, Z):");
    this->NeedlePositionMatrix->ExpandWidgetOff();
    this->NeedlePositionMatrix->GetLabel()->SetWidth(18);
    this->NeedlePositionMatrix->SetBalloonHelpString("Set the needle position");

    vtkKWMatrixWidget *matrix =  this->NeedlePositionMatrix->GetWidget();
    matrix->SetNumberOfColumns(3);
    matrix->SetNumberOfRows(1);
    matrix->SetElementWidth(12);
    matrix->SetRestrictElementValueToDouble();
    matrix->SetElementChangedCommandTriggerToAnyChange();
    }

  if (!this->NeedleOrientationMatrix && this->ShowTargetOrientation)
    {
    this->NeedleOrientationMatrix = vtkKWMatrixWidgetWithLabel::New();
    this->NeedleOrientationMatrix->SetParent(this->TargetControlFrame);
    this->NeedleOrientationMatrix->Create();
    this->NeedleOrientationMatrix->SetLabelText("Orientation (W, X, Y, Z):");
    this->NeedleOrientationMatrix->ExpandWidgetOff();
    this->NeedleOrientationMatrix->GetLabel()->SetWidth(18);
    this->NeedleOrientationMatrix->SetBalloonHelpString("Set the needle orientation");

    vtkKWMatrixWidget *matrix =  this->NeedleOrientationMatrix->GetWidget();
    matrix->SetNumberOfColumns(4);
    matrix->SetNumberOfRows(1);
    matrix->SetElementWidth(12);
    matrix->SetRestrictElementValueToDouble();
    matrix->SetElementChangedCommandTriggerToAnyChange();
    }

  if (this->ShowTargetOrientation)
    {
    this->Script("pack %s %s -side top -anchor nw -expand n -padx 2 -pady 2",
               this->NeedlePositionMatrix->GetWidgetName(),
               this->NeedleOrientationMatrix->GetWidgetName());
    }
  else 
    {
    this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
               this->NeedlePositionMatrix->GetWidgetName());
    }


  if(!this->Message)
    {
    this->Message = vtkKWText::New();
    }
  if(!this->Message->IsCreated())
    {
    this->Message->SetParent(this->TargetControlFrame);
    this->Message->Create();
    this->Message->SetText("Select needle type, then click on image to add a target");      
    this->Message->SetBackgroundColor(0.7, 0.7, 0.95);
    this->Message->SetHeight(6);
    this->Message->SetWrapToWord();
    this->Message->ReadOnlyOn();
    this->Message->SetBorderWidth(2);
    this->Message->SetReliefToGroove();
    this->Message->SetFont("times 12 bold");
    //this->Message->SetForegroundColor(0.0, 1.0, 0.0);
    }
  this->Script("pack %s -side top -anchor nw -expand n -fill x -padx 2 -pady 6", 
                this->Message->GetWidgetName());

  if (!this->MoveButton)
    {
    this->MoveButton = vtkKWPushButton::New();
    this->MoveButton->SetParent (this->TargetControlFrame);
    this->MoveButton->Create();
    this->MoveButton->SetText("Move");
    this->MoveButton->SetBalloonHelpString("Move the robot to the position");
    }

  if (!this->StopButton)
    {
    this->StopButton = vtkKWPushButton::New();
    this->StopButton->SetParent (this->TargetControlFrame);
    this->StopButton->Create();
    this->StopButton->SetText("Stop");
    this->StopButton->SetBalloonHelpString("Stop the robot");
    }

  this->Script("pack %s %s -side left -anchor nw -expand n -padx 2 -pady 2",
               this->MoveButton->GetWidgetName(),
               this->StopButton->GetWidgetName());

}


//----------------------------------------------------------------------------
void vtkProstateNavTargetingStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


//----------------------------------------------------------------------------
void vtkProstateNavTargetingStep::ProcessGUIEvents(vtkObject *caller,
                                          unsigned long event, void *callData)
{

  // -----------------------------------------------------------------
  // Move Button Pressed

  if (this->MoveButton == vtkKWPushButton::SafeDownCast(caller)
      && event == vtkKWPushButton::InvokedEvent)
    {
    if (this->Logic && this->NeedlePositionMatrix)
      {
      float position[3]={0,0,0};   // position parameters
      float orientation[4]={1,0,0,0}; // orientation parameters

      vtkKWMatrixWidget* matrix = this->NeedlePositionMatrix->GetWidget();
      position[0] = (float) matrix->GetElementValueAsDouble(0, 0);
      position[1] = (float) matrix->GetElementValueAsDouble(0, 1);
      position[2] = (float) matrix->GetElementValueAsDouble(0, 2);

      if (this->ShowTargetOrientation && this->NeedleOrientationMatrix)
      {
        matrix = this->NeedleOrientationMatrix->GetWidget();
        orientation[0] = (float) matrix->GetElementValueAsDouble(0, 0);
        orientation[1] = (float) matrix->GetElementValueAsDouble(0, 1);
        orientation[2] = (float) matrix->GetElementValueAsDouble(0, 2);
        orientation[3] = (float) matrix->GetElementValueAsDouble(0, 3);
      }

      vtkMRMLNode* node = this->GetLogic()->GetApplicationLogic()->GetMRMLScene()->GetNodeByID(this->GetProstateNavManager()->GetRobotNode()->GetTargetTransformNodeID());
      vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);

      if (transformNode)
        {
        vtkMatrix4x4* matrix = transformNode->GetMatrixTransformToParent();
        igtl::Matrix4x4 igtlmatrix;

        igtl::QuaternionToMatrix(orientation, igtlmatrix);
        igtlmatrix[0][3] = position[0];
        igtlmatrix[1][3] = position[1];
        igtlmatrix[2][3] = position[2];
        
        matrix->SetElement(0, 0, igtlmatrix[0][0]);
        matrix->SetElement(1, 0, igtlmatrix[1][0]);
        matrix->SetElement(2, 0, igtlmatrix[2][0]);
        matrix->SetElement(3, 0, igtlmatrix[3][0]);

        matrix->SetElement(0, 1, igtlmatrix[0][1]);
        matrix->SetElement(1, 1, igtlmatrix[1][1]);
        matrix->SetElement(2, 1, igtlmatrix[2][1]);
        matrix->SetElement(3, 1, igtlmatrix[3][1]);

        matrix->SetElement(0, 2, igtlmatrix[0][2]);
        matrix->SetElement(1, 2, igtlmatrix[1][2]);
        matrix->SetElement(2, 2, igtlmatrix[2][2]);
        matrix->SetElement(3, 2, igtlmatrix[3][2]);

        matrix->SetElement(0, 3, igtlmatrix[0][3]);
        matrix->SetElement(1, 3, igtlmatrix[1][3]);
        matrix->SetElement(2, 3, igtlmatrix[2][3]);
        matrix->SetElement(3, 3, igtlmatrix[3][3]);
        

        std::cerr << "TARGETPOISITION = "
                  << igtlmatrix[0][3] << ", "
                  << igtlmatrix[1][3] << ", "
                  << igtlmatrix[2][3] << std::endl;

        vtkMatrix4x4* transformToParent = transformNode->GetMatrixTransformToParent();
        transformToParent->DeepCopy(matrix);

        // Send move to command 
        this->GetProstateNavManager()->GetRobotNode()->MoveTo(transformNode->GetID());

        }
      }
    }

  // -----------------------------------------------------------------
  // Stop Button Pressed

  else if (this->StopButton == vtkKWPushButton::SafeDownCast(caller)
      && event == vtkKWPushButton::InvokedEvent)
    {

    }

  /////////

  vtkMRMLProstateNavManagerNode *mrmlNode = this->GetGUI()->GetProstateNavManager();

  if(!mrmlNode)
      return;

  if (this->DeleteButton == vtkKWPushButton::SafeDownCast(caller)
      && event == vtkKWPushButton::InvokedEvent)
    {
      vtkProstateNavTargetDescriptor *targetDesc = mrmlNode->GetTargetDescriptorAtIndex(mrmlNode->GetCurrentTargetIndex());       
      if (this->TargetPlanListNode!=NULL && targetDesc!=NULL)
      {
        int fidIndex=this->TargetPlanListNode->GetFiducialIndex(targetDesc->GetFiducialID());
        if (fidIndex>=0)
        {
          this->TargetPlanListNode->RemoveFiducial(fidIndex);
          mrmlNode->SetCurrentTargetIndex(-1);
          UpdateTargetListGUI();
        }
        else
        {
          vtkErrorMacro("Cannot delete target, fiducial not found");
        }
      }
      else
      {
        vtkErrorMacro("Cannot delete target, fiducial or target descriptor is invalid");
      }
    }

  // load targeting volume dialog button
  if (this->LoadTargetingVolumeButton && this->LoadTargetingVolumeButton == vtkKWPushButton::SafeDownCast(caller) && (event == vtkKWPushButton::InvokedEvent))
    {
    this->GetApplication()->Script("::LoadVolume::ShowDialog");
    }

  // show coverage dialog button
   if (this->ShowCoverageButton && this->ShowCoverageButton == vtkKWCheckButton::SafeDownCast(caller) && (event == vtkKWCheckButton::SelectedStateChangedEvent))
    {
      this->ShowCoverage(this->ShowCoverageButton->GetSelectedState() == 1);
    }

 // activate fiducial placement
 if (this->AddTargetsOnClickButton && this->AddTargetsOnClickButton == vtkKWCheckButton::SafeDownCast(caller) && (event == vtkKWCheckButton::SelectedStateChangedEvent))
  {
    // Activate target fiducials in the Fiducial GUI
    if (this->GetLogic()==NULL)
    {
      vtkErrorMacro("Logic is invalid");
    }    
    else
    {
      EnableAddTargetsOnClickButton(this->AddTargetsOnClickButton->GetSelectedState()==1);
    }

  }

  if (this->NeedleTypeMenuList && this->NeedleTypeMenuList->GetWidget()->GetMenu() == vtkKWMenu::SafeDownCast(caller) && (event == vtkKWMenu::MenuItemInvokedEvent))
    {
      mrmlNode->SetCurrentNeedleIndex(this->NeedleTypeMenuList->GetWidget()->GetMenu()->GetIndexOfSelectedItem());
    }

  if (this->VolumeSelectorWidget == vtkSlicerNodeSelectorWidget::SafeDownCast(caller) &&
    event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) 
  {
    vtkMRMLScalarVolumeNode *volume = vtkMRMLScalarVolumeNode::SafeDownCast(this->VolumeSelectorWidget->GetSelected());
    if (volume != NULL)
    {
      this->GetGUI()->GetLogic()->SelectVolumeInScene(volume, VOL_TARGETING);
      this->AddTargetsOnClickButton->SetSelectedState(1);
    }
  }
}


//----------------------------------------------------------------------------
void vtkProstateNavTargetingStep::ProcessMRMLEvents(vtkObject *caller,
                                         unsigned long event, void *callData)
{

  vtkMRMLFiducialListNode *targetNode = vtkMRMLFiducialListNode::SafeDownCast(caller);
  if (targetNode!=NULL && targetNode == this->TargetPlanListNode )
  {
    switch (event)
    {
    case vtkCommand::ModifiedEvent:
    case vtkMRMLScene::NodeAddedEvent: // Node Added Event : when a fiducial is added to the list
    case vtkMRMLScene::NodeRemovedEvent: // Node Removed Event : when a fiducial is reomved from the list
    case vtkMRMLFiducialListNode::FiducialModifiedEvent:
    case vtkMRMLFiducialListNode::DisplayModifiedEvent:
      UpdateTargetListGUI();
      break;
    }
  }

  if ( vtkMRMLScene::SafeDownCast(caller) == this->MRMLScene 
    && event == vtkMRMLScene::NodeAddedEvent )
    {
    vtkMRMLScalarVolumeNode *volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast((vtkMRMLNode*)(callData));

    // Check if the newly added volume is the coverage volume
    // (that shouldn't be used as a targeting volume)
    bool coverageVolumeWasAdded=false;
    vtkMRMLProstateNavManagerNode *managerNode = GetProstateNavManager();
    if (managerNode!=NULL && volumeNode!=NULL)
    {
      if (strcmp(volumeNode->GetName(), ROBOT_COVERAGE_AREA_NODE_NAME)==0 )
      {
        coverageVolumeWasAdded=true;
      }
    }

    if (!coverageVolumeWasAdded && volumeNode!=NULL && this->VolumeSelectorWidget!=NULL && volumeNode!=this->VolumeSelectorWidget->GetSelected() )
      {
      // a new volume is loaded, set as the current targeting volume
      this->VolumeSelectorWidget->SetSelected(volumeNode);
      }
    }

  vtkMRMLProstateNavManagerNode *managerNode = vtkMRMLProstateNavManagerNode::SafeDownCast(caller);
  if (managerNode!=NULL && managerNode==GetProstateNavManager())
    {
    switch (event)
      {
      case vtkMRMLProstateNavManagerNode::CurrentTargetChangedEvent:
        this->GUI->BringTargetToViewIn2DViews(vtkProstateNavGUI::BRING_MARKERS_TO_VIEW_KEEP_CURRENT_ORIENTATION);
        break;
      }
    }

}

//----------------------------------------------------------------------------
void vtkProstateNavTargetingStep::AddMRMLObservers()
{
  vtkMRMLProstateNavManagerNode* manager=this->GetProstateNavManager();
  if (manager==NULL)
  {
    return;
  }

  vtkMRMLFiducialListNode* plan = manager->GetTargetPlanListNode();
  if (plan)
  {
    vtkSmartPointer<vtkIntArray> events = vtkSmartPointer<vtkIntArray>::New();
    events->InsertNextValue(vtkCommand::ModifiedEvent);
    events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
    events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);    
    events->InsertNextValue(vtkMRMLFiducialListNode::DisplayModifiedEvent);
    events->InsertNextValue(vtkMRMLFiducialListNode::FiducialModifiedEvent);

    // Set and observe target plan list
    //vtkObject *oldNode = this->TargetPlanListNode;
    this->MRMLObserverManager->SetAndObserveObjectEvents(vtkObjectPointer(&(this->TargetPlanListNode)),(plan),(events));
  }
 
  manager->AddObserver(vtkMRMLProstateNavManagerNode::CurrentTargetChangedEvent, this->MRMLCallbackCommand);

  if (this->MRMLScene!=NULL)
  {
    this->MRMLScene->AddObserver(vtkMRMLScene::NodeAddedEvent, this->MRMLCallbackCommand);
  }
}

//----------------------------------------------------------------------------
void vtkProstateNavTargetingStep::RemoveMRMLObservers()
{
  if (this->TargetPlanListNode!=NULL)
  {    
    this->MRMLObserverManager->SetAndObserveObjectEvents(vtkObjectPointer(&(this->TargetPlanListNode)), NULL, NULL);
  }
  vtkMRMLProstateNavManagerNode* manager=this->GetProstateNavManager();
  if (manager!=NULL)
  {
    manager->RemoveObservers(vtkMRMLProstateNavManagerNode::CurrentTargetChangedEvent, this->MRMLCallbackCommand);    
    manager->RemoveObservers(vtkMRMLScene::NodeAddedEvent, this->MRMLCallbackCommand);    
  }
  if (this->MRMLScene!=NULL)
  {
    this->MRMLScene->RemoveObservers(vtkMRMLScene::NodeAddedEvent, this->MRMLCallbackCommand);
  }
}

//---------------------------------------------------------------------------
void vtkProstateNavTargetingStep::OnMultiColumnListUpdate(int row, int col, char * str)
{

  vtkMRMLFiducialListNode* fidList = this->GetProstateNavManager()->GetTargetPlanListNode();

  if (fidList == NULL)
    {
    return;
    }

  bool updated=false;

  // make sure that the row and column exists in the table
  if ((row >= 0) && (row < this->TargetList->GetWidget()->GetNumberOfRows()) &&
      (col >= 0) && (col < this->TargetList->GetWidget()->GetNumberOfColumns()))
    {
      
    // now update the requested value
    if (col == COL_NAME)
      {
      fidList->SetNthFiducialLabelText(row, str);
      updated=true;
      }
    else if (col >= COL_X && col <= COL_Z)
      {
      // get the current xyz
      float * xyz = fidList->GetNthFiducialXYZ(row);
      // now set the new one
      float newCoordinate = atof(str);
      if ( xyz )
        {
        if (col == COL_X)
          {
          fidList->SetNthFiducialXYZ(row, newCoordinate, xyz[1], xyz[2]);
          updated=true;
          }
        if (col == COL_Y)
          {
          fidList->SetNthFiducialXYZ(row, xyz[0], newCoordinate, xyz[2]);
          updated=true;
          }
        if (col == COL_Z)
          {
          fidList->SetNthFiducialXYZ(row, xyz[0], xyz[1], newCoordinate);
          updated=true;
          }
        }            
      }
    else if (this->ShowTargetOrientation && col >= COL_OR_W  && col <= COL_OR_Z)
      {
      float * wxyz = fidList->GetNthFiducialOrientation(row);
      float newCoordinate = atof(str);
      if (col == COL_OR_W)
        {
        fidList->SetNthFiducialOrientation(row, newCoordinate, wxyz[1], wxyz[2], wxyz[3]);
        updated=true;
        }
      if (col == COL_OR_X)
        {
        fidList->SetNthFiducialOrientation(row, wxyz[0], newCoordinate, wxyz[2], wxyz[3]);
        updated=true;
        }
      if (col == COL_OR_Y)
        {
        fidList->SetNthFiducialOrientation(row, wxyz[0], wxyz[1], newCoordinate, wxyz[3]);
        updated=true;
        }
      if (col == COL_OR_Z)
        {
        fidList->SetNthFiducialOrientation(row, wxyz[0], wxyz[1], wxyz[2], newCoordinate);
        updated=true;
        }
      }
    else
      {
      return;
      }
    }
  else
    {
    }
  if (updated)
  {
    this->GetLogic()->UpdateTargetListFromMRML();
  }
}


//---------------------------------------------------------------------------
void vtkProstateNavTargetingStep::OnMultiColumnListSelectionChanged()
{

  vtkMRMLFiducialListNode* fidList = this->GetProstateNavManager()->GetTargetPlanListNode();

  if (fidList == NULL)
    {
    return;
    }

  int numRows = this->TargetList->GetWidget()->GetNumberOfSelectedRows();
  if (numRows == 1)
    {   
    
    int rowIndex = this->TargetList->GetWidget()->GetIndexOfFirstSelectedRow();    
    int targetIndex=this->TargetList->GetWidget()->GetRowAttributeAsInt(rowIndex, TARGET_INDEX_ATTR);
    vtkProstateNavTargetDescriptor* targetDesc=this->GetProstateNavManager()->GetTargetDescriptorAtIndex(targetIndex);    

    if (targetDesc==NULL)
    {
      vtkErrorMacro("Target descriptor not found");
      return;
    }
    // Copy the values to inputs
    vtkKWMatrixWidget* matrix = this->NeedlePositionMatrix->GetWidget();
    double* xyz=targetDesc->GetRASLocation();
    matrix->SetElementValueAsDouble(0, 0, xyz[0]);
    matrix->SetElementValueAsDouble(0, 1, xyz[1]);
    matrix->SetElementValueAsDouble(0, 2, xyz[2]);

    if (this->ShowTargetOrientation && this->NeedleOrientationMatrix)
      {
      matrix = this->NeedleOrientationMatrix->GetWidget();
      double* wxyz=targetDesc->GetRASOrientation();
      matrix->SetElementValueAsDouble(0, 0, wxyz[0]);
      matrix->SetElementValueAsDouble(0, 1, wxyz[1]);
      matrix->SetElementValueAsDouble(0, 2, wxyz[2]);
      matrix->SetElementValueAsDouble(0, 3, wxyz[3]);
      }
          
    this->GetProstateNavManager()->SetCurrentTargetIndex(targetIndex);
    }
}

//----------------------------------------------------------------------------
void vtkProstateNavTargetingStep::UpdateTargetListGUI()
{
  vtkMRMLFiducialListNode* activeFiducialListNode=NULL;
  if (this->GetProstateNavManager()!=NULL)
  {
    activeFiducialListNode=this->GetProstateNavManager()->GetTargetPlanListNode();
  }

  if (activeFiducialListNode == NULL)    //clear out the list box
    {
    if (this->TargetList)
      {
      if (this->TargetList->GetWidget()->GetNumberOfRows() != 0)
        {
        this->TargetList->GetWidget()->DeleteAllRows();
        }
      }
    return;
    }
  
  // create new target points, if necessary
  this->GetLogic()->UpdateTargetListFromMRML();

  vtkMRMLProstateNavManagerNode *manager = this->GetGUI()->GetProstateNavManager();
  if (!manager)
  {
    return;
  }

  //int numPoints = activeFiducialListNode->GetNumberOfFiducials();
  int numPoints = manager->GetTotalNumberOfTargets();

  bool deleteFlag = true;

  if (numPoints != this->TargetList->GetWidget()->GetNumberOfRows())
    {
    // clear out the multi column list box and fill it in with the
    // new list
    this->TargetList->GetWidget()->DeleteAllRows();
    }
  else
    {
    deleteFlag = false;
    }
        
  double *xyz;
  double *wxyz;

  // Precision of the target position and orientation display
  const int POSITION_PRECISION_DIGITS=1;
  const double POSITION_PRECISION_TOLERANCE=0.1/2.0;

  for (int row = 0; row < numPoints; row++)
    {      
    int targetIndex=row;
    vtkProstateNavTargetDescriptor* target = manager->GetTargetDescriptorAtIndex(targetIndex);

    if (deleteFlag)
      {
      // add a row for this point
      this->TargetList->GetWidget()->AddRow();
      }
    this->TargetList->GetWidget()->SetRowAttributeAsInt(row, TARGET_INDEX_ATTR, targetIndex);

    xyz=target->GetRASLocation();
    wxyz=target->GetRASOrientation();

    if (xyz == NULL)
      {
      vtkErrorMacro ("UpdateTargetListGUI: ERROR: got null xyz for point " << row << endl);
      }

    if (target->GetName().compare(this->TargetList->GetWidget()->GetCellText(row,COL_NAME)) != 0)
        {
          this->TargetList->GetWidget()->SetCellText(row,COL_NAME,target->GetName().c_str());
        }               

    // selected
    vtkKWMultiColumnList* columnList = this->TargetList->GetWidget();
    if (xyz != NULL)
      {
      for (int i = 0; i < 3; i ++) // for position (x, y, z)
        {
        if (deleteFlag || fabs(columnList->GetCellTextAsDouble(row,COL_X+i)-xyz[i])>POSITION_PRECISION_TOLERANCE)
          {
          std::ostrstream os;    
          os << std::setiosflags(ios::fixed | ios::showpoint) << std::setprecision(POSITION_PRECISION_DIGITS);
          os << xyz[i] << std::ends;
          columnList->SetCellText(row,COL_X+i,os.str());
          os.rdbuf()->freeze();
          }
        }
      }
    if (this->ShowTargetOrientation && wxyz != NULL)
      {
      for (int i = 0; i < 4; i ++) // for orientation (w, x, y, z)
        {
        if (deleteFlag || fabs(columnList->GetCellTextAsDouble(row, COL_OR_W+i)-wxyz[i])>POSITION_PRECISION_TOLERANCE)
          {
          std::ostrstream os;    
          os << std::setiosflags(ios::fixed | ios::showpoint) << std::setprecision(POSITION_PRECISION_DIGITS);
          os << wxyz[i] << std::ends;
          columnList->SetCellText(row,COL_OR_W+i,os.str());
          os.rdbuf()->freeze();
          }
        }
      }

    if (target->GetNeedleTypeString().compare(this->TargetList->GetWidget()->GetCellText(row,COL_NEEDLE)) != 0)
    {
      this->TargetList->GetWidget()->SetCellText(row,COL_NEEDLE,target->GetNeedleTypeString().c_str());
    }

    }  

}



//-----------------------------------------------------------------------------
void vtkProstateNavTargetingStep::AddGUIObservers()
{
  this->RemoveGUIObservers();

  if (this->LoadTargetingVolumeButton)
    {
    this->LoadTargetingVolumeButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand); 
    }
  if (this->VolumeSelectorWidget)
    {
    this->VolumeSelectorWidget->AddObserver ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand);  
    }
  if (this->ShowCoverageButton)
    {
      this->ShowCoverageButton->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    }  
  if (this->AddTargetsOnClickButton)
    {
      this->AddTargetsOnClickButton->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    }  
  if (this->NeedleTypeMenuList)
    {
    this->NeedleTypeMenuList->GetWidget()->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->DeleteButton)
    {
    this->DeleteButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->MoveButton)
    {
    this->MoveButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->StopButton)
    {
    this->StopButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->TargetList)
    {
    this->TargetList->GetWidget()->SetCellUpdatedCommand(this, "OnMultiColumnListUpdate");
    this->TargetList->GetWidget()->SetSelectionChangedCommand(this, "OnMultiColumnListSelectionChanged");
    }
}
//-----------------------------------------------------------------------------
void vtkProstateNavTargetingStep::RemoveGUIObservers()
{
  if (this->LoadTargetingVolumeButton)
    {
    this->LoadTargetingVolumeButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand); 
    }
  if (this->VolumeSelectorWidget)
    {
    this->VolumeSelectorWidget->RemoveObserver ((vtkCommand *)this->GUICallbackCommand);  
    }
  if (this->ShowCoverageButton)
    {
    this->ShowCoverageButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }  
  if (this->AddTargetsOnClickButton)
    {
      this->AddTargetsOnClickButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }  
  if (this->NeedleTypeMenuList)
    {
      this->NeedleTypeMenuList->GetWidget()->GetMenu()->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->DeleteButton)
    {
    this->DeleteButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->MoveButton)
    {
    this->MoveButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->StopButton)
    {
    this->StopButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->TargetList)
    {
    this->TargetList->GetWidget()->SetCellUpdatedCommand(this, "");
    this->TargetList->GetWidget()->SetSelectionChangedCommand(this, "");
    }
}

//--------------------------------------------------------------------------------
void vtkProstateNavTargetingStep::UpdateGUI()
{
  vtkMRMLProstateNavManagerNode *mrmlNode = this->GetGUI()->GetProstateNavManager();

  if (!mrmlNode)
  {
    return;
  }

  
  const char* volNodeID = mrmlNode->GetTargetingVolumeNodeID();
  vtkMRMLScalarVolumeNode *volNode=vtkMRMLScalarVolumeNode::SafeDownCast(this->GetLogic()->GetApplicationLogic()->GetMRMLScene()->GetNodeByID(volNodeID));
  if ( volNode )
  {
    this->VolumeSelectorWidget->UpdateMenu();
    this->VolumeSelectorWidget->SetSelected( volNode );
  }

  // Display information about the currently selected target descriptor    
  if (this->Message)
  {    
    vtkMRMLRobotNode* robot=NULL;
    if (this->GetProstateNavManager()!=NULL)
    {
      robot=this->GetProstateNavManager()->GetRobotNode();
    }
    vtkProstateNavTargetDescriptor *targetDesc = mrmlNode->GetTargetDescriptorAtIndex(mrmlNode->GetCurrentTargetIndex()); 

    if (robot!=NULL && targetDesc!=NULL)
    {
      std::string info=robot->GetTargetInfoText(targetDesc);
      this->Message->SetText(info.c_str());
    }
    else
    {
      // no target info available for the current robot with the current target
      this->Message->SetText("");
    }

  }

  UpdateTargetListGUI();

  if (this->NeedleTypeMenuList)
    {
    this->NeedleTypeMenuList->GetWidget()->GetMenu()->DeleteAllItems();
    for (int i = 0; i < mrmlNode->GetNumberOfNeedles(); i++)
      {
      std::ostrstream needleTitle;
      needleTitle << mrmlNode->GetNeedleDescription(i) << " <" << mrmlNode->GetNeedleType(i) <<"> ("
        <<mrmlNode->GetNeedleOvershoot(i)<<"mm overshoot, "
        <<mrmlNode->GetNeedleLength(i)<<"mm length"
        << ")" << std::ends;      
      this->NeedleTypeMenuList->GetWidget()->GetMenu()->AddRadioButton(needleTitle.str());
      needleTitle.rdbuf()->freeze();
      needleTitle.clear();
      }
    int needleIndex=mrmlNode->GetCurrentNeedleIndex();
    this->NeedleTypeMenuList->GetWidget()->GetMenu()->SelectItem(needleIndex);
    }
}

// return:
//  0=error
//----------------------------------------------------------------------------
void vtkProstateNavTargetingStep::ShowCoverage(bool show) 
{
  // :TODO: show/hide depending on show parameter value

  vtkProstateNavLogic *logic=this->GetGUI()->GetLogic();
  if (!logic)
  {
    vtkErrorMacro("Invalid logic object");
    return;
  }
  logic->ShowCoverage(show);
}

//----------------------------------------------------------------------------
void vtkProstateNavTargetingStep::HideUserInterface()
{
  Superclass::HideUserInterface();
  TearDownGUI();
}

//----------------------------------------------------------------------------------------
void vtkProstateNavTargetingStep::TearDownGUI()
{
  RemoveMRMLObservers();
  RemoveGUIObservers();
}

//----------------------------------------------------------------------------
void vtkProstateNavTargetingStep::EnableAddTargetsOnClickButton(bool enable)
{
  if (this->GetProstateNavManager()==NULL)
  {
    return;
  }
  vtkMRMLFiducialListNode* fidNode = this->GetProstateNavManager()->GetTargetPlanListNode();
  GetLogic()->SetCurrentFiducialList(fidNode);
  GetLogic()->SetMouseInteractionMode( (enable) ? 
    vtkMRMLInteractionNode::Place : 
    vtkMRMLInteractionNode::ViewTransform
    );
}

//----------------------------------------------------------------------------
void vtkProstateNavTargetingStep::SetShowTargetOrientation(int show)
{
  if (this->TargetList)
  {
    vtkErrorMacro("ShowTargetOrientation cannot be changed after the GUI has been built");
    return;
  }
  this->ShowTargetOrientation = show;  
}
