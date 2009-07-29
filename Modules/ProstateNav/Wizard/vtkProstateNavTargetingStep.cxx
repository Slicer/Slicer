/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#include "vtkProstateNavTargetingStep.h"

#include "vtkProstateNavGUI.h"
#include "vtkProstateNavLogic.h"

#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWLabel.h"
#include "vtkKWMatrixWidget.h"
#include "vtkKWMatrixWidgetWithLabel.h"
#include "vtkKWPushButton.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerFiducialsGUI.h"
#include "vtkSlicerFiducialsLogic.h"
#include "vtkMRMLSelectionNode.h"

#include "vtkMRMLLinearTransformNode.h"
#include "igtlMath.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkProstateNavTargetingStep);
vtkCxxRevisionMacro(vtkProstateNavTargetingStep, "$Revision: 1.1 $");

//----------------------------------------------------------------------------
vtkProstateNavTargetingStep::vtkProstateNavTargetingStep()
{
  //this->SetName("Targeting");
  this->SetTitle("Targeting");
  this->SetDescription("Set target points.");

  this->MainFrame          = NULL;
  this->TargetListFrame    = NULL;
  this->TargetControlFrame = NULL;

  this->MultiColumnList         = NULL;
  this->NeedlePositionMatrix    = NULL;
  this->NeedleOrientationMatrix = NULL;

  this->MoveButton = NULL;
  this->StopButton = NULL;

  this->TitleBackgroundColor[0] = 0.8;
  this->TitleBackgroundColor[1] = 0.8;
  this->TitleBackgroundColor[2] = 0.8;

}

//----------------------------------------------------------------------------
vtkProstateNavTargetingStep::~vtkProstateNavTargetingStep()
{
  if (this->MainFrame)
    {
    this->MainFrame->SetParent(NULL);
    this->MainFrame->Delete();
    this->MainFrame = NULL;
    }
  if (this->TargetListFrame)
    {
    this->TargetListFrame->SetParent(NULL);
    this->TargetListFrame->Delete();
    this->TargetListFrame = NULL;
    }
  if (this->TargetControlFrame)
    {
    this->TargetControlFrame->SetParent(NULL);
    this->TargetControlFrame->Delete();
    this->TargetControlFrame = NULL;
    }
  if (this->MultiColumnList)
    {
    this->MultiColumnList->SetParent(NULL);
    this->MultiColumnList->Delete();
    this->MultiColumnList = NULL;
    }
  if (this->NeedlePositionMatrix)
    {
    this->NeedlePositionMatrix->SetParent(NULL);
    this->NeedlePositionMatrix->Delete();
    this->NeedlePositionMatrix = NULL;
    }
  if (this->NeedleOrientationMatrix)
    {
    this->NeedleOrientationMatrix->SetParent(NULL);
    this->NeedleOrientationMatrix->Delete();
    this->NeedleOrientationMatrix = NULL;
    }
  if (this->MoveButton)
    {
    this->MoveButton->SetParent(NULL);
    this->MoveButton->Delete();
    this->MoveButton = NULL;
    }
  if (this->StopButton)
    {
    this->StopButton->SetParent(NULL);
    this->StopButton->Delete();
    this->StopButton = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkProstateNavTargetingStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkKWWizardWidget *wizardWidget = this->GetGUI()->GetWizardWidget();
  vtkKWWidget *parent = wizardWidget->GetClientArea();

  // -----------------------------------------------------------------
  // Target List Frame
  
  if (!this->MainFrame)
    {
    this->MainFrame = vtkKWFrame::New();
    this->MainFrame->SetParent(parent);
    this->MainFrame->Create();
    }

  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
                this->MainFrame->GetWidgetName());
    
  if (!this->TargetListFrame)
    {
    this->TargetListFrame = vtkKWFrame::New();
    this->TargetListFrame->SetParent(this->MainFrame);
    this->TargetListFrame->Create();
    }

  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
                this->TargetListFrame->GetWidgetName());
  
  if (!this->MultiColumnList)
    {
    this->MultiColumnList = vtkKWMultiColumnListWithScrollbars::New();
    this->MultiColumnList->SetParent(this->TargetListFrame);
    this->MultiColumnList->Create();
    this->MultiColumnList->SetHeight(1);
    this->MultiColumnList->GetWidget()->SetSelectionTypeToRow();
    this->MultiColumnList->GetWidget()->MovableRowsOff();
    this->MultiColumnList->GetWidget()->MovableColumnsOff();

    const char* labels[] =
      { "Name", "X", "Y", "Z", "OrW", "OrX", "OrY", "OrZ" };
    const int widths[] = 
      { 8, 6, 6, 6, 6, 6, 6, 6 };

    for (int col = 0; col < 8; col ++)
      {
      this->MultiColumnList->GetWidget()->AddColumn(labels[col]);
      this->MultiColumnList->GetWidget()->SetColumnWidth(col, widths[col]);
      this->MultiColumnList->GetWidget()->SetColumnAlignmentToLeft(col);
      //this->MultiColumnList->GetWidget()->ColumnEditableOff(col);
      this->MultiColumnList->GetWidget()->ColumnEditableOn(col);
      this->MultiColumnList->GetWidget()->SetColumnEditWindowToSpinBox(col);
      }

    this->MultiColumnList->GetWidget()->SetColumnEditWindowToCheckButton(0);
    this->MultiColumnList->GetWidget()->SetCellUpdatedCommand(this, "OnMultiColumnListUpdate");
    this->MultiColumnList->GetWidget()->SetSelectionChangedCommand(this, "OnMultiColumnListSelectionChanged");

    }

  this->Script( "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
                this->MultiColumnList->GetWidgetName());
  

  if (!this->NeedlePositionMatrix)
    {
    this->NeedlePositionMatrix = vtkKWMatrixWidgetWithLabel::New();
    this->NeedlePositionMatrix->SetParent(this->TargetListFrame);
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

  if (!this->NeedleOrientationMatrix)
    {
    this->NeedleOrientationMatrix = vtkKWMatrixWidgetWithLabel::New();
    this->NeedleOrientationMatrix->SetParent(this->TargetListFrame);
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

  this->Script("pack %s %s -side top -anchor nw -expand n -padx 2 -pady 2",
               this->NeedlePositionMatrix->GetWidgetName(),
               this->NeedleOrientationMatrix->GetWidgetName());


  // -----------------------------------------------------------------
  // Control Frame

  if (!this->TargetControlFrame)
    {
    this->TargetControlFrame = vtkKWFrame::New();
    this->TargetControlFrame->SetParent(this->MainFrame);
    this->TargetControlFrame->Create();
    }

  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
               this->TargetControlFrame->GetWidgetName());
  
  if (!this->MoveButton)
    {
    this->MoveButton = vtkKWPushButton::New();
    this->MoveButton->SetParent (this->TargetControlFrame);
    this->MoveButton->Create();
    this->MoveButton->SetText("Move");
    this->MoveButton->SetBalloonHelpString("Move the robot to the position");
    this->MoveButton->AddObserver(vtkKWPushButton::InvokedEvent,
                                  (vtkCommand *)this->GUICallbackCommand);
    }

  if (!this->StopButton)
    {
    this->StopButton = vtkKWPushButton::New();
    this->StopButton->SetParent (this->TargetControlFrame);
    this->StopButton->Create();
    this->StopButton->SetText("Stop");
    this->StopButton->SetBalloonHelpString("Stop the robot");
    this->StopButton->AddObserver(vtkKWPushButton::InvokedEvent,
                                  (vtkCommand *)this->GUICallbackCommand);
    }

  this->Script("pack %s %s -side left -anchor nw -expand n -padx 2 -pady 2",
               this->MoveButton->GetWidgetName(),
               this->StopButton->GetWidgetName());

  // -----------------------------------------------------------------
  // MRML Event Observers

  UpdateMRMLObserver();

  if (this->GetProstateNavManager() && this->GetGUI())
    {
    vtkSlicerApplication *app = (vtkSlicerApplication *)(this->GetGUI()->GetApplication());
    vtkSlicerFiducialsGUI* fidGUI 
      = vtkSlicerFiducialsGUI::SafeDownCast ( app->GetModuleGUIByName ("Fiducials"));
    
    if (fidGUI)
      {
      vtkMRMLFiducialListNode* fidNode = this->GetProstateNavManager()->GetTargetPlanList();
      if (fidNode)
        {
        fidGUI->Enter();
        fidGUI->SetFiducialListNodeID (fidNode->GetID());
        }
      }
    }
  
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
    if (this->Logic && this->NeedlePositionMatrix && this->NeedleOrientationMatrix)
      {
      float position[3];   // position parameters
      float orientation[4]; // orientation parameters

      vtkKWMatrixWidget* matrix = this->NeedlePositionMatrix->GetWidget();
      position[0] = (float) matrix->GetElementValueAsDouble(0, 0);
      position[1] = (float) matrix->GetElementValueAsDouble(0, 1);
      position[2] = (float) matrix->GetElementValueAsDouble(0, 2);

      matrix = this->NeedleOrientationMatrix->GetWidget();
      orientation[0] = (float) matrix->GetElementValueAsDouble(0, 0);
      orientation[1] = (float) matrix->GetElementValueAsDouble(0, 1);
      orientation[2] = (float) matrix->GetElementValueAsDouble(0, 2);
      orientation[3] = (float) matrix->GetElementValueAsDouble(0, 3);

      vtkMRMLNode* node = this->GetLogic()->GetApplicationLogic()->GetMRMLScene()->
        GetNodeByID(this->GetLogic()->GetRobotTargetNodeID());
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
        this->Logic->RobotMoveTo();
        }
      }
    }

  // -----------------------------------------------------------------
  // Stop Button Pressed

  else if (this->StopButton == vtkKWPushButton::SafeDownCast(caller)
      && event == vtkKWPushButton::InvokedEvent)
    {

    }           

}


//----------------------------------------------------------------------------
void vtkProstateNavTargetingStep::ProcessMRMLEvents(vtkObject *caller,
                                         unsigned long event, void *callData)
{

  vtkMRMLFiducialListNode *node = vtkMRMLFiducialListNode::SafeDownCast(caller);
  if (!node)
    {
    return;
    }

  // -----------------------------------------------------------------
  // Modified Event

  if (node == this->GetProstateNavManager()->GetTargetPlanList()
      && event == vtkCommand::ModifiedEvent)
    {
    SetGUIFromList(this->GetProstateNavManager()->GetTargetPlanList());
    }

  // -----------------------------------------------------------------
  // Node Added Event : when a fiducial is added to the list

  if (node == this->GetProstateNavManager()->GetTargetPlanList()
      && event == vtkMRMLScene::NodeAddedEvent)
    {
    SetGUIFromList(this->GetProstateNavManager()->GetTargetPlanList());
    }

  // -----------------------------------------------------------------
  // Fiducial Modified Event

  else if (node == this->GetProstateNavManager()->GetTargetPlanList()
           && event == vtkMRMLFiducialListNode::FiducialModifiedEvent)
    {
    SetGUIFromList(this->GetProstateNavManager()->GetTargetPlanList());
    }
  
  // -----------------------------------------------------------------
  // Display Modified Event
  
  else if (node == this->GetProstateNavManager()->GetTargetPlanList()
           && event == vtkMRMLFiducialListNode::DisplayModifiedEvent)
    {
    SetGUIFromList(this->GetProstateNavManager()->GetTargetPlanList());
    }
  
}


//----------------------------------------------------------------------------
void vtkProstateNavTargetingStep::UpdateMRMLObserver()
{

  if (this->GetProstateNavManager())
    {

    vtkMRMLFiducialListNode* plan      = this->GetProstateNavManager()->GetTargetPlanList();
    vtkMRMLFiducialListNode* completed = this->GetProstateNavManager()->GetTargetCompletedList();

    if (plan && completed)
      {
      
      vtkIntArray *events = vtkIntArray::New();
      events->InsertNextValue(vtkCommand::ModifiedEvent);
      events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
      events->InsertNextValue(vtkMRMLFiducialListNode::DisplayModifiedEvent);
      events->InsertNextValue(vtkMRMLFiducialListNode::FiducialModifiedEvent);
      
      // Set and observe target plan list
      vtkObject *oldNode = (plan);
      this->MRMLObserverManager
        ->SetAndObserveObjectEvents(vtkObjectPointer(&(plan)),(plan),(events));
      if ( oldNode != (plan) )
        {
        this->GetProstateNavManager()->SetAndObserveTargetPlanList(plan);
        this->InvokeEvent (vtkCommand::ModifiedEvent);
        } 

      // Set and observe completed target list
      vtkObject *oldNode2 = (completed);
      this->MRMLObserverManager
        ->SetAndObserveObjectEvents(vtkObjectPointer(&(completed)),(completed),(events));
      if ( oldNode2 != (completed) )
        {
        this->GetProstateNavManager()->SetAndObserveTargetPlanList(completed);
        this->InvokeEvent (vtkCommand::ModifiedEvent);
        } 

      events->Delete();
      }
    }

}


//---------------------------------------------------------------------------
void vtkProstateNavTargetingStep::OnMultiColumnListUpdate(int row, int col, char * str)
{

  vtkMRMLFiducialListNode* fidList = this->GetProstateNavManager()->GetTargetPlanList();

  if (fidList == NULL)
    {
    return;
    }

  // make sure that the row and column exists in the table
  if ((row >= 0) && (row < this->MultiColumnList->GetWidget()->GetNumberOfRows()) &&
      (col >= 0) && (col < this->MultiColumnList->GetWidget()->GetNumberOfColumns()))
    {
      
    // now update the requested value
    if (col == 0)
      {
      fidList->SetNthFiducialLabelText(row, str);
      }
    else if (col >= 1 && col <= 3)
      {
      // get the current xyz
      float * xyz = fidList->GetNthFiducialXYZ(row);
      // now set the new one
      float newCoordinate = atof(str);
      if ( xyz )
        {
        if (col == 1)
          {
          fidList->SetNthFiducialXYZ(row, newCoordinate, xyz[1], xyz[2]);
          }
        if (col == 2)
          {
          fidList->SetNthFiducialXYZ(row, xyz[0], newCoordinate, xyz[2]);
          }
        if (col == 3)
          {
          fidList->SetNthFiducialXYZ(row, xyz[0], xyz[1], newCoordinate);
          }
        }            
      }
    else if (col >= 4  && col <= 7)
      {
      float * wxyz = fidList->GetNthFiducialOrientation(row);
      float newCoordinate = atof(str);
      if (col == 4)
        {
        fidList->SetNthFiducialOrientation(row, newCoordinate, wxyz[1], wxyz[2], wxyz[3]);
        }
      if (col == 5)
        {
        fidList->SetNthFiducialOrientation(row, wxyz[0], newCoordinate, wxyz[2], wxyz[3]);
        }
      if (col == 6)
        {
        fidList->SetNthFiducialOrientation(row, wxyz[0], wxyz[1], newCoordinate, wxyz[3]);
        }
      if (col == 7)
        {
        fidList->SetNthFiducialOrientation(row, wxyz[0], wxyz[1], wxyz[2], newCoordinate);
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
}


//---------------------------------------------------------------------------
void vtkProstateNavTargetingStep::OnMultiColumnListSelectionChanged()
{

  vtkMRMLFiducialListNode* fidList = this->GetProstateNavManager()->GetTargetPlanList();

  if (fidList == NULL)
    {
    return;
    }

  if (this->MRMLScene)
    {
    this->MRMLScene->SaveStateForUndo();
    }

  int numRows = this->MultiColumnList->GetWidget()->GetNumberOfSelectedRows();
  if (numRows == 1)
    {
    for (int i = 0; i < fidList->GetNumberOfFiducials(); i ++)
      {
      fidList->SetNthFiducialSelected(i, false);
      }
    int row[1];
    this->MultiColumnList->GetWidget()->GetSelectedRows(row);
    fidList->SetNthFiducialSelected(row[0], true);

    // Copy the values to inputs
    vtkKWMatrixWidget* matrix = this->NeedlePositionMatrix->GetWidget();
    float* xyz = fidList->GetNthFiducialXYZ(row[0]);
    matrix->SetElementValueAsDouble(0, 0, xyz[0]);
    matrix->SetElementValueAsDouble(0, 1, xyz[1]);
    matrix->SetElementValueAsDouble(0, 2, xyz[2]);

    matrix = this->NeedleOrientationMatrix->GetWidget();
    float* wxyz = fidList->GetNthFiducialOrientation(row[0]);
    matrix->SetElementValueAsDouble(0, 0, wxyz[0]);
    matrix->SetElementValueAsDouble(0, 1, wxyz[1]);
    matrix->SetElementValueAsDouble(0, 2, wxyz[2]);
    matrix->SetElementValueAsDouble(0, 3, wxyz[3]);

    }
}


//----------------------------------------------------------------------------
void vtkProstateNavTargetingStep::SetGUIFromList(vtkMRMLFiducialListNode * activeFiducialListNode)
{

  if (activeFiducialListNode == NULL)    //clear out the list box
    {
    if (this->MultiColumnList)
      {
      if (this->MultiColumnList->GetWidget()->GetNumberOfRows() != 0)
        {
        this->MultiColumnList->GetWidget()->DeleteAllRows();
        }
      }
    return;
    }
    
  int numPoints = activeFiducialListNode->GetNumberOfFiducials();
  bool deleteFlag = true;

  if (numPoints != this->MultiColumnList->GetWidget()->GetNumberOfRows())
    {
    // clear out the multi column list box and fill it in with the
    // new list
    this->MultiColumnList->GetWidget()->DeleteAllRows();
    }
  else
    {
    deleteFlag = false;
    }
        
  float *xyz;
  float *wxyz;

  for (int row = 0; row < numPoints; row++)
    {
    if (deleteFlag)
      {
      // add a row for this point
      this->MultiColumnList->GetWidget()->AddRow();
      }

    xyz = activeFiducialListNode->GetNthFiducialXYZ(row);
    wxyz = activeFiducialListNode->GetNthFiducialOrientation(row);

    if (xyz == NULL)
      {
      vtkErrorMacro ("SetGUIFromList: ERROR: got null xyz for point " << row << endl);
      }

    if (activeFiducialListNode->GetNthFiducialLabelText(row) != NULL)
      {
      if (strcmp(this->MultiColumnList->GetWidget()->GetCellText(row,0),
                 activeFiducialListNode->GetNthFiducialLabelText(row)) != 0)
        {
        this->MultiColumnList->GetWidget()
          ->SetCellText(row,0,activeFiducialListNode->GetNthFiducialLabelText(row));
        }               
      }
    else
      {
      if (strcmp(this->MultiColumnList->GetWidget()->GetCellText(row, 0), "(none)") != 0)
        {
        this->MultiColumnList->GetWidget()->SetCellText(row,0,"(none)");
        }
      }

    // selected
    vtkKWMultiColumnList* columnList = this->MultiColumnList->GetWidget();
    if (xyz != NULL)
      {
      for (int i = 0; i < 3; i ++) // for position (x, y, z)
        {
        if (deleteFlag || columnList->GetCellTextAsDouble(row,1+i) != xyz[i])
          {
          columnList->SetCellTextAsDouble(row,1+i,xyz[i]);
          }
        }
      }
    if (wxyz != NULL)
      {
      for (int i = 0; i < 4; i ++) // for orientation (w, x, y, z)
        {
        if (deleteFlag || columnList->GetCellTextAsDouble(row, 4+i) != wxyz[i])
          {
          columnList->SetCellTextAsDouble(row,4+i,wxyz[i]);
          }
        }
      }
    }

  vtkDebugMacro("Now going to update GUI from the logic's active list");

}
