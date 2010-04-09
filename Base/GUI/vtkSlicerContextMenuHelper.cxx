
#include "vtkObjectFactory.h"
#include "vtkCollection.h"

#include "vtkSlicerContextMenuHelper.h"

#include "vtkKWTkUtilities.h"
#include "vtkKWEntry.h"
#include "vtkKWMessageDialog.h"

#include "vtkMRMLDisplayableNode.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerContextMenuHelper );
vtkCxxRevisionMacro ( vtkSlicerContextMenuHelper, "$Revision$" );

//---------------------------------------------------------------------------
vtkSlicerContextMenuHelper::vtkSlicerContextMenuHelper ( ) {
  this->MRMLScene = NULL;
  this->MRMLNode = NULL;
  this->ContextMenu = NULL;
  this->RenameEntry = NULL;
  this->RenameTopLevel = NULL;
  this->DeleteItem = -1;
  this->RenameItem = -1;
  this->EditItem = -1;
}

//---------------------------------------------------------------------------
vtkSlicerContextMenuHelper::~vtkSlicerContextMenuHelper ( ) {
  if (this->MRMLScene)
    {
    this->SetMRMLScene(NULL);
    }
  if (this->MRMLNode)
    {
    this->SetMRMLNode(NULL);
    }
  if (this->ContextMenu)
    {
    this->SetContextMenu(NULL);
    }
  if (this->RenameEntry)
    {
    this->RenameEntry->SetParent(NULL);
    this->RenameEntry->Delete();
    this->RenameEntry = NULL;
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
  if (!this->ContextMenu || !this->MRMLScene)
    {
    vtkWarningMacro("Trying to populate a menu with null pointers!");
    return;
    }

  this->RenameItem = this->ContextMenu->AddCommand("Rename...", this, "RenameNodeCallback");
  this->EditItem = this->ContextMenu->AddCommand("Edit Properties...", this, "EditCallback");
  this->DeleteItem = this->ContextMenu->AddCommand("Delete...", this, "DeleteNodeCallback");
}

//---------------------------------------------------------------------------
void vtkSlicerContextMenuHelper::DeleteNodeCallback()
{
  vtkMRMLNode *node = this->GetMRMLNode();
  vtkSlicerApplication *app = vtkSlicerApplication::GetInstance();
  if (node != NULL)
    {
    std::string message = std::string("Delete ") + std::string(node->GetName()) + std::string("?"); 
    if ( vtkKWMessageDialog::PopupYesNo( 
          app, this->ContextMenu->GetParentTopLevel(), 
          "Delete Node", message.c_str(),
          vtkKWMessageDialog::WarningIcon | 
          vtkKWMessageDialog::InvokeAtPointer) )
      {
      this->GetMRMLScene()->RemoveNode(node);
      }
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
void vtkSlicerContextMenuHelper::RenameNodeCallback()
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

  if (appGUI && this->MRMLNode)
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
    this->RenameTopLevel->ModalOn();
    this->RenameTopLevel->Create ( );
    this->RenameTopLevel->SetMasterWindow ( app->GetApplicationGUI()->GetMainSlicerWindow() );
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
    this->RenameEntry->GetWidget()->SetCommandTrigger(vtkKWEntry::TriggerOnReturnKey); 
    this->RenameEntry->GetWidget()->SetCommand (this, "RenameApplyCallback");
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

  this->RenameEntry->GetWidget()->SelectAll();
  this->RenameEntry->GetWidget()->Focus();
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
    std::string name(this->RenameEntry->GetWidget()->GetValue());
    if (this->MRMLScene->GetNodesByName(name.c_str())->GetNumberOfItems() > 0) 
      {
      /*** For some reason the dilog below doesn not dissapera or causes crash, removing for now
      vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
      dialog->SetParent (  this->RenameTopLevel );
      dialog->SetStyleToMessage();
      std::string msg = std::string("Node with the name ") + 
        std::string(this->RenameEntry->GetWidget()->GetValue()) + std::string(" already exists");
      dialog->SetText(msg.c_str());
      dialog->Create ( );

      vtkSlicerApplication *app = vtkSlicerApplication::GetInstance();
      dialog->SetMasterWindow( app->GetApplicationGUI()->GetMainSlicerWindow() );

      dialog->ModalOn();
      dialog->Invoke();
      dialog->Delete();
      ****/
      // make sure name is unique instead
      //name = std::string(this->MRMLScene->GetUniqueNameByString(name.c_str()));
      }
    node->SetName( name.c_str());
    this->MRMLScene->InvokeEvent(vtkMRMLScene::NodeAddedEvent, node);
    //return;
    }
  this->HideRenameEntry();
}

//---------------------------------------------------------------------------
void vtkSlicerContextMenuHelper::UpdateMenuState()
{
  if ( this->GetMRMLNode() )
    {
    this->ContextMenu->SetItemState(this->DeleteItem, 1);
    this->ContextMenu->SetItemState(this->RenameItem, 1);
    this->ContextMenu->SetItemState(this->EditItem, 1);
    }
  else
    {
    this->ContextMenu->SetItemState(this->DeleteItem, 0);
    this->ContextMenu->SetItemState(this->RenameItem, 0);
    this->ContextMenu->SetItemState(this->EditItem, 0);
    }
}
