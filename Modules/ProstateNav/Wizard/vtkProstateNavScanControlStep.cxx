#include "vtkProstateNavScanControlStep.h"

#include "vtkProstateNavGUI.h"
#include "vtkProstateNavLogic.h"

#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWPushButton.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerFiducialsGUI.h"
#include "vtkSlicerFiducialsLogic.h"
#include "vtkMRMLSelectionNode.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkProstateNavScanControlStep);
vtkCxxRevisionMacro(vtkProstateNavScanControlStep, "$Revision: 1.1 $");

//----------------------------------------------------------------------------
vtkProstateNavScanControlStep::vtkProstateNavScanControlStep()
{
  this->SetName("2/5. Set Scanner Parameters");
  this->SetDescription("Operate the MRI scanner.");

  this->TargetListFrame  = NULL;
  this->MultiColumnList = NULL;
  this->TargetControlFrame = NULL; 
  this->AddButton        = NULL;
  this->RemoveButton     = NULL;
  this->RemoveAllButton  = NULL;

  /*
  this->FiducialListNodeID = NULL;
  this->FiducialListNode   = NULL;
  */

}

//----------------------------------------------------------------------------
vtkProstateNavScanControlStep::~vtkProstateNavScanControlStep()
{
  
}

//----------------------------------------------------------------------------
void vtkProstateNavScanControlStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkKWWizardWidget *wizardWidget = this->GetGUI()->GetWizardWidget();
  vtkKWWidget *parent = wizardWidget->GetClientArea();


  // -----------------------------------------------------------------
  // Target List Frame

  if (!this->TargetListFrame)
    {
    this->TargetListFrame = vtkKWFrame::New();
    this->TargetListFrame->SetParent(parent);
    this->TargetListFrame->Create();
    }

  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
                this->TargetListFrame->GetWidgetName());

  if (!this->MultiColumnList)
    {
    // add the multicolumn list to show the points
    this->MultiColumnList = vtkKWMultiColumnListWithScrollbars::New();
    this->MultiColumnList->SetParent(TargetListFrame);
    this->MultiColumnList->Create();
    this->MultiColumnList->SetHeight(4);
    //this->MultiColumnList->GetWidget()->SetSelectionTypeToCell();
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
    // make the Name column editable by checkbutton
    this->MultiColumnList->GetWidget()->SetColumnEditWindowToCheckButton(0);
    this->MultiColumnList->GetWidget()->SetCellUpdatedCommand(this, "OnMultiColumnListUpdate");
    this->MultiColumnList->GetWidget()->SetSelectionChangedCommand(this, "OnMultiColumnListSelectionChanged");
    }

  this->Script( "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
                this->MultiColumnList->GetWidgetName());


  // -----------------------------------------------------------------
  // Target Control Frame

  if (!this->TargetControlFrame)
    {
    this->TargetControlFrame = vtkKWFrame::New();
    this->TargetControlFrame->SetParent(TargetListFrame);
    this->TargetControlFrame->Create();
    }

  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
               this->TargetControlFrame->GetWidgetName());

  if (!this->AddButton)
    {
    this->AddButton = vtkKWPushButton::New();
    this->AddButton->SetParent (this->TargetControlFrame);
    this->AddButton->Create();
    this->AddButton->SetText("Add Target");
    this->AddButton->SetBalloonHelpString("Move the robot to the position");
    this->AddButton->AddObserver(vtkKWPushButton::InvokedEvent,
                                  (vtkCommand *)this->GUICallbackCommand);
    }

  if (!this->RemoveButton)
    {
    this->RemoveButton = vtkKWPushButton::New();
    this->RemoveButton->SetParent (this->TargetControlFrame);
    this->RemoveButton->Create();
    this->RemoveButton->SetText("Remove Target");
    this->RemoveButton->SetBalloonHelpString("Move the robot to the position");
    this->RemoveButton->AddObserver(vtkKWPushButton::InvokedEvent,
                                  (vtkCommand *)this->GUICallbackCommand);
    }

  if (!this->RemoveAllButton)
    {
    this->RemoveAllButton = vtkKWPushButton::New();
    this->RemoveAllButton->SetParent (this->TargetControlFrame);
    this->RemoveAllButton->Create();
    this->RemoveAllButton->SetText("Remove All");
    this->RemoveAllButton->SetBalloonHelpString("Move the robot to the position");
    this->RemoveAllButton->AddObserver(vtkKWPushButton::InvokedEvent,
                                  (vtkCommand *)this->GUICallbackCommand);
    }

  this->Script("pack %s %s %s -side left -anchor nw -expand n -padx 2 -pady 2",
               this->AddButton->GetWidgetName(),
               this->RemoveButton->GetWidgetName(),
               this->RemoveAllButton->GetWidgetName());


  // -----------------------------------------------------------------
  // MRML Event Observer

  if (this->GetGUI()->GetFiducialListNodeID())
    {
    this->MRMLScene->SaveStateForUndo(this->GetGUI()->GetFiducialListNode());

    vtkMRMLSelectionNode *selnode = NULL;
    if (this->GetGUI()->GetApplicationLogic())
      {
      selnode = this->GetGUI()->GetApplicationLogic()->GetSelectionNode();
      }
    this->UpdateMRMLObserver(selnode);
    this->SetGUIFromList(this->GetGUI()->GetFiducialListNode());
    }
  else
    {
    vtkErrorMacro ("ShowUserInterface(): Cannot find FiducialListNodeID for Prostate Module." );
    }

}

//----------------------------------------------------------------------------
void vtkProstateNavScanControlStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkProstateNavScanControlStep::ProcessGUIEvents(vtkObject *caller,
                                         unsigned long event, void *callData)
{

  std::cerr << "vtkProstateNavScanControlStep::ProcessGUIEvents()" << std::endl;

  /*
  vtkMRMLFiducialListNode *activeFiducialListNode 
    = (vtkMRMLFiducialListNode *)this->MRMLScene->GetNodeByID(this->FiducialListNodeID);
  */
  if (!this->GetGUI()->GetFiducialListNode())
    {
    vtkDebugMacro("FiducialList is not exist\n");
    return;
    }

  // -----------------------------------------------------------------
  // Add Button Pressed

  else if (this->AddButton == vtkKWPushButton::SafeDownCast(caller)
      && event == vtkKWPushButton::InvokedEvent)
    {
    this->MRMLScene->SaveStateForUndo();
    int modelIndex = this->GetGUI()->GetFiducialListNode()->AddFiducial();
    if ( modelIndex < 0 ) 
      {
      vtkErrorMacro ("ERROR adding a new fiducial point\n");
      }
    }

  // -----------------------------------------------------------------
  // Remove Button Pressed

  else if (this->RemoveButton == vtkKWPushButton::SafeDownCast(caller)
      && event == vtkKWPushButton::InvokedEvent)
    {
    this->MRMLScene->SaveStateForUndo();    // save state for undo

    // get the row that was last selected
    int numRows = this->MultiColumnList->GetWidget()->GetNumberOfSelectedRows();
    std::cerr << "vtkProstateNavScanControlStep::ProcessGUIEvent():  Remove fiducial " << numRows << std::endl;

    if (numRows == 1)
      {
      int row[1];
      this->MultiColumnList->GetWidget()->GetSelectedRows(row);
      std::cout << "Removing fiducial " << row[0] << endl;
      this->GetGUI()->GetFiducialListNode()->RemoveFiducial(row[0]);
      }
    }


  // -----------------------------------------------------------------
  // Remove All Button Pressed

  else if (this->RemoveAllButton == vtkKWPushButton::SafeDownCast(caller)
           && event == vtkKWPushButton::InvokedEvent)
    {
    this->MRMLScene->SaveStateForUndo();
    this->GetGUI()->GetFiducialListNode()->RemoveAllFiducials();
    }
  
}


//----------------------------------------------------------------------------
void vtkProstateNavScanControlStep::ProcessMRMLEvents(vtkObject *caller,
                                         unsigned long event, void *callData)
{
  std::cerr << "vtkProstateNavScanControlStep::ProcessMRMLEvents() is called. " << std::cerr;

  vtkMRMLSelectionNode *selnode;
  if (this->GetGUI()->GetApplicationLogic())
    {
    selnode = this->GetGUI()->GetApplicationLogic()->GetSelectionNode();
    }


  // -----------------------------------------------------------------
  // Update MRML Observer

  if (selnode != NULL 
      && vtkMRMLSelectionNode::SafeDownCast(caller) == selnode
      && event == vtkCommand::ModifiedEvent)
    {

    if (selnode->GetActiveFiducialListID() != NULL &&
        this->GetGUI()->GetFiducialListNodeID() != NULL)
      {
      if (strcmp(selnode->GetActiveFiducialListID(),
                 this->GetGUI()->GetFiducialListNodeID()) != 0)
        {
        if (!selnode->GetActiveFiducialListID())
          {
          this->UpdateMRMLObserver(selnode);
          }
        }
      }
    }

  // -----------------------------------------------------------------
  // Fiducial Modified

  if (event == vtkCommand::WidgetValueChangedEvent)
    {
    vtkDebugMacro("got a widget value changed event... the list node was changed.\n");
    }

  vtkMRMLFiducialListNode *node = vtkMRMLFiducialListNode::SafeDownCast(caller);
  if (!node)
    {
    return;
    }
  /*
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkSlicerFiducialsGUI *fidGUI
    = (vtkSlicerFiducialsGUI*)app->GetModuleGUIByName("Fiducials");
  vtkMRMLFiducialListNode *activeFiducialListNode 
    = (vtkMRMLFiducialListNode *)this->MRMLScene->GetNodeByID(fidGUI->GetFiducialListNodeID());
  */
  
  // -----------------------------------------------------------------
  // Modified Event

  if (node == this->GetGUI()->GetFiducialListNode() && event == vtkCommand::ModifiedEvent)
    {
    SetGUIFromList(this->GetGUI()->GetFiducialListNode());
    }

  // -----------------------------------------------------------------
  // Fiducial Modified Event

  else if (node == this->GetGUI()->GetFiducialListNode() 
           && event == vtkMRMLFiducialListNode::FiducialModifiedEvent)
    {
    SetGUIFromList(this->GetGUI()->GetFiducialListNode());
    }
  
  // -----------------------------------------------------------------
  // Display Modified Event

  else if (node == this->GetGUI()->GetFiducialListNode()
           && event == vtkMRMLFiducialListNode::DisplayModifiedEvent)
    {
    SetGUIFromList(this->GetGUI()->GetFiducialListNode());
    }

}

//----------------------------------------------------------------------------
void vtkProstateNavScanControlStep::UpdateMRMLObserver(vtkMRMLSelectionNode* selnode)
{

  std::cerr << "vtkProstateNavScanControlStep::UpdateMRMLObserver()" << std::endl;

  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkSlicerFiducialsGUI *fidGUI
    = (vtkSlicerFiducialsGUI*) app->GetModuleGUIByName("Fiducials");

  vtkMRMLFiducialListNode* fidlist = 
    vtkMRMLFiducialListNode::SafeDownCast(this->MRMLScene
                                          ->GetNodeByID(this->GetGUI()->GetFiducialListNodeID()));
  if (selnode != NULL)
    {
    std::cerr << "selnode != 0;" << std::endl;
    // is the active fid list id out of synch with our selection?
    vtkIntArray *events = vtkIntArray::New();
    events->InsertNextValue(vtkCommand::ModifiedEvent);
    events->InsertNextValue(vtkMRMLFiducialListNode::DisplayModifiedEvent);
    events->InsertNextValue(vtkMRMLFiducialListNode::FiducialModifiedEvent);

    vtkObject *oldNode = (fidlist);
    vtkObject *pNode   = this->GetGUI()->GetFiducialListNode();
    this->MRMLObserverManager
      ->SetAndObserveObjectEvents(vtkObjectPointer(&(fidlist)),(fidlist),(events));
    if ( oldNode != (fidlist) )
      {
      this->GetGUI()->SetFiducialListNode(fidlist);
      this->InvokeEvent (vtkCommand::ModifiedEvent);
      } 
    }
}


//---------------------------------------------------------------------------
void vtkProstateNavScanControlStep::OnMultiColumnListUpdate(int row, int col, char * str)
{

  vtkMRMLFiducialListNode* fidList = this->GetGUI()->GetFiducialListNode();

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
void vtkProstateNavScanControlStep::OnMultiColumnListSelectionChanged()
{

  vtkMRMLFiducialListNode* fidList = this->GetGUI()->GetFiducialListNode();

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
    }
}


//----------------------------------------------------------------------------
void vtkProstateNavScanControlStep::SetGUIFromList(vtkMRMLFiducialListNode * activeFiducialListNode)
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
    /*
    if (deleteFlag ||
        this->MultiColumnList->GetWidget()->GetCellTextAsInt(row,this->SelectedColumn)
        != (activeFiducialListNode->GetNthFiducialSelected(row) ? 1 : 0))
      {
      this->MultiColumnList->GetWidget()
        ->SetCellTextAsInt(row,this->SelectedColumn,
                           (activeFiducialListNode->GetNthFiducialSelected(row) ? 1 : 0));
      this->MultiColumnList->GetWidget()->SetCellWindowCommandToCheckButton(row,this->SelectedColumn);
      }
    */
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
