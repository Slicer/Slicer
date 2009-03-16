
#include "vtkObjectFactory.h"
#include "vtkSlicerContextMenuHelper.h"

#include "vtkKWTkUtilities.h"
#include "vtkKWEntry.h"

#include "vtkMRMLDisplayableNode.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerContextMenuHelper );
vtkCxxRevisionMacro ( vtkSlicerContextMenuHelper, "$Revision: 1.0 $" );

//---------------------------------------------------------------------------
vtkSlicerContextMenuHelper::vtkSlicerContextMenuHelper ( ) {
  this->MRMLScene = NULL;
  this->MRMLNode = NULL;
  this->ContextMenu = NULL;
  this->RenameTopLevel = NULL;
}

//---------------------------------------------------------------------------
vtkSlicerContextMenuHelper::~vtkSlicerContextMenuHelper ( ) {
  if (this->MRMLScene)
    {
    this->SetMRMLScene(NULL);
    }
  if (this->ContextMenu)
    {
    this->SetContextMenu(NULL);
    }
  if (this->RenameTopLevel)
    {
    this->RenameTopLevel->SetParent(NULL);
    this->RenameTopLevel->Delete();
    this->RenameTopLevel = NULL;
    }
}

//---------------------------------------------------------------------------
void vtkSlicerContextMenuHelper::PopulateMenu()
{
  if (!this->ContextMenu || !this->MRMLNode || !this->MRMLScene)
    {
    vtkWarningMacro("Trying to populate a menu with null pointers!");
    return;
    }

  this->ContextMenu->AddCommand("Delete", this, "DeleteNodeCallback");
  this->ContextMenu->AddCommand("Rename...", this, "RenameNodeCallback");
  this->ContextMenu->AddCommand("Edit Properties...", this, "EditCallback");
}

//---------------------------------------------------------------------------
void vtkSlicerContextMenuHelper::DeleteNodeCallback()
{
  vtkMRMLNode *node = this->GetMRMLNode();
  if (node != NULL)
    {
    this->GetMRMLScene()->RemoveNode(node);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerContextMenuHelper::ToggleVisibilityCallback()
{
  vtkMRMLDisplayableNode *dnode = vtkMRMLDisplayableNode::SafeDownCast(this->GetMRMLNode());

  if (!dnode)
    {
    return;
    }

  int newVisibilityState = this->ContextMenu->GetItemSelectedState("Toggle Visibility");
  for (int i=0; i<dnode->GetNumberOfDisplayNodes(); i++)
    {
    dnode->GetNthDisplayNode(i)->SetVisibility(newVisibilityState);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerContextMenuHelper::RenameCallback()
{
  if (this->MRMLNode && this->MRMLScene)
    {
    this->PopUpRenameEntry();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerContextMenuHelper::EditCallback()
{
  vtkSlicerApplication *app = vtkSlicerApplication::GetInstance();
  vtkSlicerApplicationGUI *appGUI = app->GetApplicationGUI();

  if (appGUI && this->MRMLNode && this->MRMLScene)
    {
    appGUI->SelectModuleForNode(this->MRMLNode);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerContextMenuHelper::PopUpRenameEntry()
{
  vtkSlicerApplication *app = vtkSlicerApplication::GetInstance();

  if ( !this->RenameTopLevel )
    {
    this->RenameTopLevel = vtkKWTopLevel::New ( );
    this->RenameTopLevel->SetApplication ( app );
    this->RenameTopLevel->SetMasterWindow ( app->GetNthWindow(0) );
    this->RenameTopLevel->Create ( );
    this->RenameTopLevel->HideDecorationOn ( );
    this->RenameTopLevel->Withdraw ( );
    this->RenameTopLevel->SetBorderWidth ( 2 );
    this->RenameTopLevel->SetReliefToGroove ( );

    vtkKWFrame *popUpFrameP = vtkKWFrame::New ( );
    popUpFrameP->SetParent ( this->RenameTopLevel );
    popUpFrameP->Create ( );
    app->Script ( "pack %s -side left -anchor w -padx 2 -pady 2 -fill x -fill y -expand n", popUpFrameP->GetWidgetName ( ) );
    this->RenameEntry = vtkKWEntryWithLabel::New();
    this->RenameEntry->SetParent( popUpFrameP );
    this->RenameEntry->Create();
    this->RenameEntry->SetLabelText( "New name: " );
    popUpFrameP->Delete();

    vtkKWFrame *fP = vtkKWFrame::New();
    fP->SetParent ( popUpFrameP);
    fP->Create();
    this->RenameApply = vtkKWPushButton::New ( );
    this->RenameApply->SetParent ( fP );
    this->RenameApply->Create ( );
    this->RenameApply->SetText ("Apply");
    this->RenameApply->SetCommand (this, "RenameApplyCallback");
    this->RenameCancel = vtkKWPushButton::New();
    this->RenameCancel->SetParent ( fP );
    this->RenameCancel->Create();
    this->RenameCancel->SetText ( "Cancel");
    this->RenameCancel->SetCommand (this, "HideRenameEntry");
    app->Script ( "grid %s -row 0 -column 0 -padx 2 -pady 8", 
      this->RenameEntry->GetWidgetName() );
    app->Script ( "grid %s -row 1 -column 0 -columnspan 1 -pady 8 -sticky ew", 
      fP->GetWidgetName() );
    app->Script ( "pack %s %s -side left -padx 4 -anchor c", 
      RenameCancel->GetWidgetName(), this->RenameApply->GetWidgetName() );
    fP->Delete();
    }

  this->RenameEntry->GetWidget()->SetValue( this->MRMLNode->GetName() );

  // Get the position of the mouse, position the popup
  int x, y;
  vtkKWTkUtilities::GetMousePointerCoordinates(this->ContextMenu, &x, &y);
  this->RenameTopLevel->SetPosition(x, y);
  app->ProcessPendingEvents();
  this->RenameTopLevel->DeIconify();
  this->RenameTopLevel->Raise();
  
}

//---------------------------------------------------------------------------
void vtkSlicerContextMenuHelper::HideRenameEntry()
{
  if ( !this->RenameTopLevel )
    {
    return;
    }
  this->RenameTopLevel->Withdraw();
}

//---------------------------------------------------------------------------
void vtkSlicerContextMenuHelper::RenameApplyCallback()
{
  vtkMRMLNode *node = this->GetMRMLNode();
  if (node != NULL)
    {
    node->SetName( this->RenameEntry->GetWidget()->GetValue() );
    }
}

