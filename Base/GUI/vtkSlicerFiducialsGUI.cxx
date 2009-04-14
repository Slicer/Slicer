#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkKWWidget.h"
#include "vtkSlicerModelsGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerFoundationIcons.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkMRMLViewNode.h"

#include "vtkKWMessage.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"

#include "vtkKWFrameWithLabel.h"

#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"

#include "vtkSlicerFiducialsGUI.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerFiducialsGUI );
vtkCxxRevisionMacro ( vtkSlicerFiducialsGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerFiducialsGUI::vtkSlicerFiducialsGUI ( )
{
    this->Logic = NULL;
    this->FiducialListSelectorWidget = NULL;
    this->FiducialListNodeID = NULL; // "(none)";
    this->FiducialListNode = NULL; // "(none)";

    this->MeasurementLabel = NULL;
    
    this->AddFiducialButton = NULL;
    this->RemoveFiducialButton = NULL;
    this->RemoveFiducialsInListButton = NULL;
    this->RemoveAllFiducialsButton = NULL;
    this->LockAllFiducialsButton = NULL;
    this->UnlockAllFiducialsButton = NULL;
    
    this->SelectAllFiducialsButton = NULL;
    this->DeselectAllFiducialsButton = NULL;
    this->SelectAllFiducialsInListButton = NULL;
    this->DeselectAllFiducialsInListButton = NULL;
    
    this->VisibilityToggle = NULL;

    this->MoveSelectedFiducialUpButton = NULL;
    this->MoveSelectedFiducialDownButton = NULL;
    
    this->ListColorButton = NULL;
    this->ListSelectedColorButton = NULL;
    this->ListSymbolScale = NULL;
    this->ListSymbolTypeMenu = NULL;
    this->ListTextScale = NULL;
    this->ListOpacity = NULL;
    this->ListAmbient = NULL;
    this->ListDiffuse = NULL;
    this->ListSpecular = NULL;
    this->ListPower = NULL;
    
    this->AllLockMenuButton = NULL;
    this->ListLockMenuButton = NULL;
    this->AllVisibilityMenuButton = NULL;
    this->ListVisibilityMenuButton = NULL;
    
    this->MultiColumnList = NULL;

    this->BIRNLabel = NULL;
    this->NCIGTLabel = NULL;
    this->NAMICLabel = NULL;
    this->NACLabel = NULL;
    
//    this->DebugOn();
}


//---------------------------------------------------------------------------
vtkSlicerFiducialsGUI::~vtkSlicerFiducialsGUI ( )
{

  this->SetModuleLogic ( NULL );

  if ( this->AllLockMenuButton)
    {
    this->AllLockMenuButton->SetParent ( NULL );
    this->AllLockMenuButton->Delete();
    this->AllLockMenuButton = NULL;
    }
  if ( this->ListLockMenuButton )
    {
    this->ListLockMenuButton->SetParent ( NULL );
    this->ListLockMenuButton->Delete();
    this->ListLockMenuButton = NULL;
    }
  if ( this->AllVisibilityMenuButton )
    {
    this->AllVisibilityMenuButton->SetParent ( NULL );
    this->AllVisibilityMenuButton->Delete();
    this->AllVisibilityMenuButton = NULL;
    }
  if ( this->ListVisibilityMenuButton )
    {
    this->ListVisibilityMenuButton->SetParent ( NULL );
    this->ListVisibilityMenuButton->Delete();
    this->ListVisibilityMenuButton = NULL;
    }
  if (this->FiducialListSelectorWidget)
    {
    this->FiducialListSelectorWidget->SetParent(NULL);
    this->FiducialListSelectorWidget->Delete();
    this->FiducialListSelectorWidget = NULL;
    }
  if (this->MeasurementLabel)
    {
    this->MeasurementLabel->SetParent(NULL);
    this->MeasurementLabel->Delete();
    this->MeasurementLabel = NULL;
    }
  if (this->AddFiducialButton )
    {
    this->AddFiducialButton->SetParent (NULL );
    this->AddFiducialButton->Delete ( );
    this->AddFiducialButton = NULL;
    }
  if (this->RemoveFiducialButton )
    {
    this->RemoveFiducialButton->SetParent (NULL );
    this->RemoveFiducialButton->Delete ( );
    this->RemoveFiducialButton = NULL;
    }
  if (this->RemoveAllFiducialsButton )
    {
    this->RemoveAllFiducialsButton->SetParent (NULL );
    this->RemoveAllFiducialsButton->Delete ( );
    this->RemoveAllFiducialsButton = NULL;
    }
  if (this->LockAllFiducialsButton )
    {
    this->LockAllFiducialsButton->SetParent (NULL );
    this->LockAllFiducialsButton->Delete ( );
    this->LockAllFiducialsButton = NULL;
    }
  if (this->UnlockAllFiducialsButton )
    {
    this->UnlockAllFiducialsButton->SetParent (NULL );
    this->UnlockAllFiducialsButton->Delete ( );
    this->UnlockAllFiducialsButton = NULL;
    }
  if (this->RemoveFiducialsInListButton )
    {
    this->RemoveFiducialsInListButton->SetParent (NULL );
    this->RemoveFiducialsInListButton->Delete ( );
    this->RemoveFiducialsInListButton = NULL;
    }
  if (this->SelectAllFiducialsButton)
    {
    this->SelectAllFiducialsButton->SetParent(NULL);
    this->SelectAllFiducialsButton->Delete();
    this->SelectAllFiducialsButton = NULL;
    }
  if (this->DeselectAllFiducialsButton)
    {
    this->DeselectAllFiducialsButton->SetParent(NULL);
    this->DeselectAllFiducialsButton->Delete();
    this->DeselectAllFiducialsButton = NULL;
    }
  if (this->SelectAllFiducialsInListButton)
    {
    this->SelectAllFiducialsInListButton->SetParent(NULL);
    this->SelectAllFiducialsInListButton->Delete();
    this->SelectAllFiducialsInListButton = NULL;
    }
  if (this->DeselectAllFiducialsInListButton)
    {
    this->DeselectAllFiducialsInListButton->SetParent(NULL);
    this->DeselectAllFiducialsInListButton->Delete();
    this->DeselectAllFiducialsInListButton = NULL;
    }
  if (this->VisibilityToggle)
    {
    this->VisibilityToggle->SetParent(NULL);
    this->VisibilityToggle->Delete();
    this->VisibilityToggle = NULL;
    }
  if (this->ListColorButton)
    {
    this->ListColorButton->SetParent(NULL);
    this->ListColorButton->Delete();
    this->ListColorButton = NULL;
    }
  if (this->ListSelectedColorButton)
    {
    this->ListSelectedColorButton->SetParent(NULL);
    this->ListSelectedColorButton->Delete();
    this->ListSelectedColorButton = NULL;
    }
  if (this->ListSymbolScale)
    {
    this->ListSymbolScale->SetParent(NULL);
    this->ListSymbolScale->Delete();
    this->ListSymbolScale = NULL;
    }
  if (this->ListSymbolTypeMenu)
    {
    this->ListSymbolTypeMenu->SetParent(NULL);
    this->ListSymbolTypeMenu->Delete();
    this->ListSymbolTypeMenu = NULL;
    }
  if (this->ListTextScale)
    {
    this->ListTextScale->SetParent(NULL);
    this->ListTextScale->Delete();
    this->ListTextScale = NULL;
    }
  if (this->ListOpacity)
    {
    this->ListOpacity->SetParent(NULL);
    this->ListOpacity->Delete();
    this->ListOpacity = NULL;
    }
  if (this->ListAmbient)
    {
    this->ListAmbient->SetParent(NULL);
    this->ListAmbient->Delete();
    this->ListAmbient = NULL;
    }
  if (this->ListDiffuse)
    {
    this->ListDiffuse->SetParent(NULL);
    this->ListDiffuse->Delete();
    this->ListDiffuse = NULL;
    }
   if (this->ListSpecular)
    {
    this->ListSpecular->SetParent(NULL);
    this->ListSpecular->Delete();
    this->ListSpecular = NULL;
    }
   if (this->ListPower)
    {
    this->ListPower->SetParent(NULL);
    this->ListPower->Delete();
    this->ListPower = NULL;
    }
  if (this->MoveSelectedFiducialUpButton)
    {
    this->MoveSelectedFiducialUpButton->SetParent(NULL);
    this->MoveSelectedFiducialUpButton->Delete();
    this->MoveSelectedFiducialUpButton = NULL;
    }
  if (this->MoveSelectedFiducialDownButton)
    {
    this->MoveSelectedFiducialDownButton->SetParent(NULL);
    this->MoveSelectedFiducialDownButton->Delete();
    this->MoveSelectedFiducialDownButton = NULL;
    }
  if (this->MultiColumnList)
    {
    this->MultiColumnList->SetParent(NULL);
    this->MultiColumnList->Delete();
    this->MultiColumnList = NULL;
    }
  if ( this->NACLabel )
    {
    this->NACLabel->SetParent ( NULL );
    this->NACLabel->Delete();
    this->NACLabel = NULL;
    }
  if ( this->NAMICLabel )
    {
    this->NAMICLabel->SetParent ( NULL );
    this->NAMICLabel->Delete();
    this->NAMICLabel = NULL;
    }
  if ( this->NCIGTLabel )
    {
    this->NCIGTLabel->SetParent ( NULL );
    this->NCIGTLabel->Delete();
    this->NCIGTLabel = NULL;
    }
  if ( this->BIRNLabel )
    {
    this->BIRNLabel->SetParent ( NULL );
    this->BIRNLabel->Delete();
    this->BIRNLabel = NULL;
    }

  //this->SetFiducialListNodeID("(none)");
  this->SetFiducialListNodeID(NULL);
  vtkSetMRMLNodeMacro(this->FiducialListNode, NULL);

}


//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerFiducialsGUI: " << this->GetClassName ( ) << "\n";
    os << indent << "Logic: " << this->GetLogic ( ) << "\n";
    // print widgets?
}



//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::RemoveGUIObservers ( )
{
    vtkDebugMacro("vtkSlicerFiducialsGUI: RemoveGUIObservers\n");
    this->FiducialListSelectorWidget->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->AddFiducialButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->RemoveFiducialButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->RemoveFiducialsInListButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->RemoveAllFiducialsButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->SelectAllFiducialsButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->DeselectAllFiducialsButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->SelectAllFiducialsInListButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->DeselectAllFiducialsInListButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

    this->MoveSelectedFiducialUpButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->MoveSelectedFiducialDownButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->VisibilityToggle->GetWidget()->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ListColorButton->RemoveObservers (vtkKWChangeColorButton::ColorChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->ListSelectedColorButton->RemoveObservers (vtkKWChangeColorButton::ColorChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->ListSymbolScale->RemoveObservers (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->ListSymbolTypeMenu->GetWidget()->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->ListTextScale->RemoveObservers (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->ListOpacity->RemoveObservers (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->ListAmbient->RemoveObservers (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->ListDiffuse->RemoveObservers (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->ListSpecular->RemoveObservers (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->ListPower->RemoveObservers (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    
    this->AllLockMenuButton->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ListLockMenuButton->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );    
    this->AllVisibilityMenuButton->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ListVisibilityMenuButton->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );    

    this->RemoveObservers (vtkSlicerFiducialsGUI::FiducialListIDModifiedEvent, (vtkCommand *)this->GUICallbackCommand);    

    if (this->MRMLScene)
      {
      this->MRMLScene->RemoveObservers(vtkMRMLScene::NodeRemovedEvent, (vtkCommand *)this->MRMLCallbackCommand);
      this->MRMLScene->RemoveObservers(vtkMRMLScene::NodeAddedEvent, (vtkCommand *)this->MRMLCallbackCommand);
      this->MRMLScene->RemoveObservers(vtkMRMLScene::SceneCloseEvent, (vtkCommand *)this->MRMLCallbackCommand);
      }
}


//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::AddGUIObservers ( )
{
    vtkDebugMacro("vtkSlicerFiducialsGUI: AddGUIObservers\n");
    this->FiducialListSelectorWidget->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->AddFiducialButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
    this->RemoveFiducialButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
    this->RemoveFiducialsInListButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
    this->RemoveAllFiducialsButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
    this->SelectAllFiducialsButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->DeselectAllFiducialsButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->SelectAllFiducialsInListButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->DeselectAllFiducialsInListButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

    this->MoveSelectedFiducialUpButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->MoveSelectedFiducialDownButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->VisibilityToggle->GetWidget()->AddObserver (vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
    this->ListColorButton->AddObserver (vtkKWChangeColorButton::ColorChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ListSelectedColorButton->AddObserver (vtkKWChangeColorButton::ColorChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ListSymbolScale->AddObserver (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->ListSymbolTypeMenu->GetWidget()->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->ListTextScale->AddObserver (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->ListOpacity->AddObserver (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->ListAmbient->AddObserver (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->ListDiffuse->AddObserver (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->ListSpecular->AddObserver (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->ListPower->AddObserver (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    
    this->AllLockMenuButton->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ListLockMenuButton->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );    
    this->AllVisibilityMenuButton->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ListVisibilityMenuButton->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );    

    this->AddObserver(vtkSlicerFiducialsGUI::FiducialListIDModifiedEvent, (vtkCommand *)this->GUICallbackCommand);

    // observe the scene for node deleted events
    if (this->MRMLScene)
      {
      if (this->MRMLScene->HasObserver(vtkMRMLScene::NodeRemovedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
        {
        this->MRMLScene->AddObserver(vtkMRMLScene::NodeRemovedEvent, (vtkCommand *)this->MRMLCallbackCommand);
        }
      if (this->MRMLScene->HasObserver(vtkMRMLScene::NodeAddedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
        {
        this->MRMLScene->AddObserver(vtkMRMLScene::NodeAddedEvent, (vtkCommand *)this->MRMLCallbackCommand);
        }
       if (this->MRMLScene->HasObserver(vtkMRMLScene::SceneCloseEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
        {
        this->MRMLScene->AddObserver(vtkMRMLScene::SceneCloseEvent, (vtkCommand *)this->MRMLCallbackCommand);
        }
      }
        
}



//--------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::ProcessGUIEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
{
  int numnodes, nn;
  vtkMRMLFiducialListNode *flNode;
  
  vtkDebugMacro("vtkSlicerFiducialsGUI::ProcessGUIEvents: event = " << event);
  
    // process fiducial list node selector events
    vtkSlicerNodeSelectorWidget *fidListSelector = 
        vtkSlicerNodeSelectorWidget::SafeDownCast(caller);    
    
    if (fidListSelector == this->FiducialListSelectorWidget &&
        event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent )
      {
      vtkDebugMacro("vtkSlicerFiducialsGUI: ProcessGUIEvent Node Selector Event: " << event << ".\n");
      vtkMRMLFiducialListNode *fidList =
        vtkMRMLFiducialListNode::SafeDownCast(this->FiducialListSelectorWidget->GetSelected());
      if (fidList != NULL)
        {
        this->SetFiducialListNode(fidList);
        }
      else
        {
        vtkDebugMacro("vtkSlicerFiducialsGUI: ProcessGUIEvent: the selected node is null!");
        }
      return;
    }
    vtkMRMLFiducialListNode *fidList =
        vtkMRMLFiducialListNode::SafeDownCast(caller);
    if (fidList == this->MRMLScene->GetNodeByID(this->GetFiducialListNodeID()) &&
        event == vtkCommand::ModifiedEvent)
    {
        return;
    }
    
  // get the currently displayed list
  
  // is there one list?
  vtkMRMLFiducialListNode *activeFiducialListNode = (vtkMRMLFiducialListNode *)this->MRMLScene->GetNodeByID(this->GetFiducialListNodeID());
  if (activeFiducialListNode == NULL)
  {
      vtkDebugMacro ("ERROR: No Fiducial List, adding one first!\n");
      vtkMRMLFiducialListNode *newList = this->GetLogic()->AddFiducialList();      
      if (newList != NULL)
        {
        this->SetFiducialListNodeID(newList->GetID());
        newList->Delete();
        }
      else
        {
        vtkErrorMacro("Unable to add a new fid list via the logic\n");
        }
      // now get the newly active node 
      activeFiducialListNode = (vtkMRMLFiducialListNode *)this->MRMLScene->GetNodeByID(this->GetFiducialListNodeID());
      if (activeFiducialListNode == NULL)
        {
        vtkErrorMacro ("ERROR adding a new fiducial list for the point...\n");
        return;
        }
  }
  vtkKWPushButton *button = vtkKWPushButton::SafeDownCast(caller);
  if (button != NULL && event ==  vtkKWPushButton::InvokedEvent)
    {
    if (button == this->AddFiducialButton)
      {
      vtkDebugMacro("vtkSlicerFiducialsGUI: ProcessGUIEvent: Add Fiducial Button event: " << event << ".\n");
      // save state for undo
      this->MRMLScene->SaveStateForUndo(activeFiducialListNode);

      // add a fiducial, get the index of the new fiducial
      int modelIndex = activeFiducialListNode->AddFiducial();
      if ( modelIndex < 0 ) 
        {
        // TODO: generate an error...
        vtkErrorMacro ("ERROR adding a new fiducial point\n");
        return;
        }
      }
    else if (button == this->RemoveFiducialButton)
      {
      vtkDebugMacro("vtkSlicerFiducialsGUI: ProcessGUIEvent: Remove Fiducial Button event: " << event << ".\n");
      // check to see if should confirm
      const char * confirmDelete = ((vtkSlicerApplication *)this->GetApplication())->GetConfirmDelete();
      int confirmDeleteFlag = 0;
      if (confirmDelete != NULL &&
          strncmp(confirmDelete, "1", 1) == 0)
        {
        vtkDebugMacro("vtkSlicerFiducialsGUI: ProcessGUIEvent: confirm delete flag is 1\n");
        confirmDeleteFlag = 1;
        }
      else
        {
        vtkDebugMacro("Not confirming deletes, confirmDelete = '" << confirmDelete << "'\n");
        }
      // save state for undo
      this->MRMLScene->SaveStateForUndo(activeFiducialListNode);
      
      // get the row that was last selected
      int numRows = this->MultiColumnList->GetWidget()->GetNumberOfSelectedRows();
      if (numRows == 1)
        {
        int row[1];
        this->MultiColumnList->GetWidget()->GetSelectedRows(row);
        
        if (confirmDeleteFlag)
          {
          // confirm that really want to remove this fiducial
          std::cout << "Removing fiducial " << row[0] << endl;
          }
        
        // then remove that fiducial by index
        activeFiducialListNode->RemoveFiducial(row[0]);
        }
      else
        {
        vtkErrorMacro ("Selected rows (" << numRows << ") not 1, just pick one to delete for now\n");
        return;
        }
      }
    else if (button == this->RemoveFiducialsInListButton)
      {
      vtkDebugMacro("vtkSlicerFiducialsGUI: ProcessGUIEvent: Remove Fiducials In List Button event: " << event << ".\n");
      // save state for undo
      this->MRMLScene->SaveStateForUndo(activeFiducialListNode);
      activeFiducialListNode->RemoveAllFiducials();
      }
    else if (button == this->RemoveAllFiducialsButton)
      {
      vtkDebugMacro("vtkSlicerFiducialsGUI: ProcessGUIEvent: Remove Fiducials Button event: " << event << ".\n");
      // save state for undo
      this->MRMLScene->SaveStateForUndo();
      numnodes = this->MRMLScene->GetNumberOfNodesByClass ( "vtkMRMLFiducialListNode" );
      for ( nn=0; nn<numnodes; nn++ )
        {
        flNode = vtkMRMLFiducialListNode::SafeDownCast (this->MRMLScene->GetNthNodeByClass ( nn, "vtkMRMLFiducialListNode" ));
        if ( flNode != NULL )
          {
          flNode->RemoveAllFiducials();
          }
        }
      //--- TODO: now delete the node...
      }
    else if ( button == this->VisibilityToggle->GetWidget() )
      {
      if ( activeFiducialListNode->GetVisibility() > 0 )
        {
        activeFiducialListNode->SetVisibility ( 0 );
        }
      else
        {
        activeFiducialListNode->SetVisibility ( 1 );
        }
      }
    else if (button == this->SelectAllFiducialsButton)
      {
      vtkDebugMacro("vtkSlicerFiducialsGUI: ProcessGUIEvent: Select all Fiducials Button event: " << event << ".\n");
      // save state for undo
      this->MRMLScene->SaveStateForUndo();
      numnodes = this->MRMLScene->GetNumberOfNodesByClass ( "vtkMRMLFiducialListNode" );
      for ( nn=0; nn<numnodes; nn++ )
        {
        flNode = vtkMRMLFiducialListNode::SafeDownCast (this->MRMLScene->GetNthNodeByClass ( nn, "vtkMRMLFiducialListNode" ));
        if ( flNode != NULL )
          {
          flNode->SetAllFiducialsSelected(1);
          }
        }
      }
    else if (button == this->DeselectAllFiducialsButton)
      {
      vtkDebugMacro("vtkSlicerFiducialsGUI: ProcessGUIEvent: Deselect all Fiducials Button event: " << event << ".\n");
      // save state for undo
      this->MRMLScene->SaveStateForUndo();
      numnodes = this->MRMLScene->GetNumberOfNodesByClass ( "vtkMRMLFiducialListNode" );
      for ( nn=0; nn<numnodes; nn++ )
        {
        flNode = vtkMRMLFiducialListNode::SafeDownCast (this->MRMLScene->GetNthNodeByClass ( nn, "vtkMRMLFiducialListNode" ));
        if ( flNode != NULL )
          {
          flNode->SetAllFiducialsSelected(0);
          }
        }
      }
    else if (button == this->SelectAllFiducialsInListButton)
      {
      vtkDebugMacro("vtkSlicerFiducialsGUI: ProcessGUIEvent: Select Fiducials In List Button event: " << event << ".\n");
      // save state for undo
      this->MRMLScene->SaveStateForUndo(activeFiducialListNode);
      activeFiducialListNode->SetAllFiducialsSelected(1);
      }
    else if (button == this->DeselectAllFiducialsInListButton)
      {
      vtkDebugMacro("vtkSlicerFiducialsGUI: ProcessGUIEvent: Deselect all Fiducials In List Button event: " << event << ".\n");
      // save state for undo
      this->MRMLScene->SaveStateForUndo(activeFiducialListNode);
      activeFiducialListNode->SetAllFiducialsSelected(0);
      }
    else if (button == this->MoveSelectedFiducialUpButton)
      {
      vtkDebugMacro("vtkSlicerFiducialsGUI: ProcessGUIEvent: Move Selected Fiducial Up Button event: " << event << ".\n");
      // get the index of the selected fiducial
      int selectedIndex =  this->MultiColumnList->GetWidget()->GetIndexOfFirstSelectedRow();
      if (selectedIndex == -1)
        {
        vtkErrorMacro("Select a fiducial first...");
        }
      else
        {
        // save state for undo
        this->MRMLScene->SaveStateForUndo(activeFiducialListNode);
        int newIndex = activeFiducialListNode->MoveFiducialUp(selectedIndex);
        if (newIndex == -1)
          {
          vtkErrorMacro("Failed to move fiducial " << selectedIndex << " up");
          }
        else
          {
          // adjust the selection so that the same fid is still selected
          // first get the selected cell (no mult selections)
          int rows[1], cols[1];
          this->MultiColumnList->GetWidget()->GetSelectedCells(rows, cols);
          this->MultiColumnList->GetWidget()->DeselectRow(selectedIndex);
          this->MultiColumnList->GetWidget()->SelectCell(newIndex, cols[0]);
          }
        }
      }
    else if (button == this->MoveSelectedFiducialDownButton)
      {
      vtkDebugMacro("vtkSlicerFiducialsGUI: ProcessGUIEvent: Move Selected Fiducial Down Button event: " << event << ".\n");
      // get the index of the selected fiducial
      int selectedIndex =  this->MultiColumnList->GetWidget()->GetIndexOfFirstSelectedRow();
      if (selectedIndex == -1)
        {
        vtkErrorMacro("Select a fiducial first...");
        }
      else
        {
        // save state for undo
        this->MRMLScene->SaveStateForUndo(activeFiducialListNode);
        int newIndex = activeFiducialListNode->MoveFiducialDown(selectedIndex);
        if (newIndex == -1)
         {
         vtkErrorMacro("Failed to move fiducial " << selectedIndex << " down");
         }
        else
          {
          // adjust the selection so that the same fid is still selected
          // first get the selected cell (no mult selections)
          int rows[1], cols[1];
          this->MultiColumnList->GetWidget()->GetSelectedCells(rows, cols);
          this->MultiColumnList->GetWidget()->DeselectRow(selectedIndex);
          this->MultiColumnList->GetWidget()->SelectCell(newIndex, cols[0]);
          }
        }
      }
    }

  // list colour
  vtkKWChangeColorButton *colorButton = vtkKWChangeColorButton::SafeDownCast(caller);
  if (colorButton != NULL && event == vtkKWChangeColorButton::ColorChangedEvent)
    {
    if (colorButton == this->ListColorButton)
      {
      // save state for undo
      this->MRMLScene->SaveStateForUndo(activeFiducialListNode);
      vtkDebugMacro("ProcessGUIEvents: list colour button change event\n");
      // change the colour
      activeFiducialListNode->SetColor(this->ListColorButton->GetColor());
      }
    else if (colorButton == this->ListSelectedColorButton)
      {
      // save state for undo
      this->MRMLScene->SaveStateForUndo(activeFiducialListNode);
      vtkDebugMacro("ProcessGUIEvents: list selected colour button change event\n");
      // change the selected colour
      activeFiducialListNode->SetSelectedColor(this->ListSelectedColorButton->GetColor());
      }
    }
  
  // list symbol and text sizes
  vtkKWScaleWithEntry *scale = vtkKWScaleWithEntry::SafeDownCast(caller);
  if (scale != NULL && event == vtkKWScale::ScaleValueChangedEvent)
    {
    // save state for undo
    this->MRMLScene->SaveStateForUndo(activeFiducialListNode);
    if (scale == this->ListSymbolScale)
      {
      activeFiducialListNode->SetSymbolScale(this->ListSymbolScale->GetValue());
      }
    else if (scale == this->ListTextScale)
      {
      activeFiducialListNode->SetTextScale(this->ListTextScale->GetValue());
      }
    else if (scale == this->ListOpacity)
      {
      activeFiducialListNode->SetOpacity(this->ListOpacity->GetValue());
      }
    else if (scale == this->ListAmbient)
      {
      activeFiducialListNode->SetAmbient(this->ListAmbient->GetValue());
      }
    else if (scale == this->ListDiffuse)
      {
      activeFiducialListNode->SetDiffuse(this->ListDiffuse->GetValue());
      }
    else if (scale == this->ListSpecular)
      {
      activeFiducialListNode->SetSpecular(this->ListSpecular->GetValue());
      }
    else if (scale == this->ListPower)
      {
      activeFiducialListNode->SetPower(this->ListPower->GetValue());
      }
    }

  vtkKWMenu *menu = vtkKWMenu::SafeDownCast ( caller );
  if ( menu != NULL && event == vtkKWMenu::MenuItemInvokedEvent )
    {
    if ( menu == this->AllLockMenuButton->GetMenu() )
      {
      // set lock on all fiducials in all lists.
      if ( menu->GetItemSelectedState ( "Lock" ) == 1 )
        {
        this->ModifyAllLock(1);
        }
      if ( menu->GetItemSelectedState ( "Unlock" ) == 1 )
        {
        this->ModifyAllLock(0);
        }
      }
    else if ( menu == this->ListLockMenuButton->GetMenu() )
      {
      // set lock on all fiducials in selected list
      if ( menu->GetItemSelectedState ( "Lock" ) == 1 )
        {
        this->ModifyListLock(1);
        }
      if ( menu->GetItemSelectedState ( "Unlock" ) == 1 )
        {
        this->ModifyListLock(0);
        }
      }
    else if ( menu == this->AllVisibilityMenuButton->GetMenu() )
      {
      // set visibility on all fiducials in all lists.
      if ( menu->GetItemSelectedState ( "Visible" ) == 1 )
        {
        this->ModifyAllVisibility (1 );
        }
      if ( menu->GetItemSelectedState ( "Invisible" ) == 1 )
        {
        this->ModifyAllVisibility (0 );
        }
      }
    else if ( menu == this->ListVisibilityMenuButton->GetMenu() )
      {
      // set visibility on all fiducials in selected list.
      if ( menu->GetItemSelectedState ( "Visible" ) == 1 )
        {
        this->ModifyListVisibility( 1 );
        }
      if ( menu->GetItemSelectedState ( "Invisible" ) == 1 )
        {
        this->ModifyListVisibility( 0 );
        }
      }    
    }

  // list symbol type
  if (this->ListSymbolTypeMenu->GetWidget()->GetMenu() ==  vtkKWMenu::SafeDownCast(caller) &&
      event == vtkKWMenu::MenuItemInvokedEvent)
    {
    // save state for undo
    this->MRMLScene->SaveStateForUndo(activeFiducialListNode);
    vtkDebugMacro("Changing list glyph type to " << this->ListSymbolTypeMenu->GetWidget()->GetValue() << endl);
    activeFiducialListNode->SetGlyphTypeFromString(this->ListSymbolTypeMenu->GetWidget()->GetValue());
    }
  return;
}


//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::ModifyAllLock( int lockState)
{

  if ( this->MRMLScene == NULL )
    {
    vtkErrorMacro ( "ModifyAllLock: got NULL MRMLScene." );
    return;
    }
  if ( lockState != 0 && lockState != 1 )
    {
    vtkErrorMacro ( "ModifyAllLock: got bad value for lock state; should be 0 or 1" );
    return;
    }
  
  vtkMRMLFiducialListNode *flNode;
  
  // save state for undo:
  // maybe we should just make a list of all the fiducialListNodes
  // and save their state here instead of the entire scene?
  this->MRMLScene->SaveStateForUndo();
  int numnodes = this->MRMLScene->GetNumberOfNodesByClass ( "vtkMRMLFiducialListNode" );
  for ( int nn=0; nn<numnodes; nn++ )
    {
    flNode = vtkMRMLFiducialListNode::SafeDownCast (this->MRMLScene->GetNthNodeByClass ( nn, "vtkMRMLFiducialListNode" ));
    if ( flNode != NULL )
      {
      flNode->SetLocked(lockState);
      }
    }

  // and update the entire column of the multicolumn list to match the active list node.
  vtkMRMLFiducialListNode *activeFiducialListNode = (vtkMRMLFiducialListNode *)this->MRMLScene->GetNodeByID(this->GetFiducialListNodeID());
  if ( activeFiducialListNode == NULL )
    {
    vtkErrorMacro ( "ModifyAllLock: got NULL activeFiducialListNode." );
    return;
    }
  if ( this->MultiColumnList->GetWidget() == NULL )
    {
    vtkErrorMacro ( "ModifyAllLock: got NULL multicolumnlist." );
    return;
    }
  int numPoints = this->MultiColumnList->GetWidget()->GetNumberOfRows();
  for ( int i = 0; i < numPoints; i++ )
    {
    if (this->MultiColumnList->GetWidget()->GetCellTextAsInt(i, this->LockColumn) != activeFiducialListNode->GetLocked() )
      {
      this->MultiColumnList->GetWidget()->SetCellEditable( i, this->LockColumn, 1 );
        this->MultiColumnList->GetWidget()->SetCellEnabledAttribute (i,  this->LockColumn, 1 );
      this->MultiColumnList->GetWidget()->ColumnEditableOn( this->LockColumn );
      this->MultiColumnList->GetWidget()->SetCellTextAsInt(i, this->LockColumn, activeFiducialListNode->GetLocked() );
      this->MultiColumnList->GetWidget()->SetCellWindowCommandToCheckButton(i, this->LockColumn);
      this->MultiColumnList->GetWidget()->SetCellEditable( i, this->LockColumn, 0 );
      this->MultiColumnList->GetWidget()->SetCellEnabledAttribute (i,  this->LockColumn, 0 );
      this->MultiColumnList->GetWidget()->ColumnEditableOff( this->LockColumn );
      }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::ModifyListLock( int lockState )
{
  if ( this->MRMLScene == NULL )
    {
    vtkErrorMacro ( "ModifyListLock: got NULL MRMLScene." );
    return;
    }
  if ( lockState != 0 && lockState != 1 )
    {
    vtkErrorMacro ( "ModifyListLock: got bad value for lock state; should be 0 or 1" );
    return;
    }

  vtkMRMLFiducialListNode *activeFiducialListNode = (vtkMRMLFiducialListNode *)this->MRMLScene->GetNodeByID(this->GetFiducialListNodeID());
  if ( activeFiducialListNode == NULL )
    {
    vtkErrorMacro ( "ModifyListLock: got NULL activeFiducialListNode." );
    return;
    }
  
  // save state of node for undo
  this->MRMLScene->SaveStateForUndo(activeFiducialListNode);
  activeFiducialListNode->SetLocked( lockState );

  // and update the entire column of the multicolumn list to match the active list node.
  if ( this->MultiColumnList->GetWidget() == NULL )
    {
    vtkErrorMacro ( "ModifyListLock: got NULL multicolumnlist." );
    return;
    }
  int numPoints = this->MultiColumnList->GetWidget()->GetNumberOfRows();
  for ( int i = 0; i < numPoints; i++ )
    {
    if (this->MultiColumnList->GetWidget()->GetCellTextAsInt(i, this->LockColumn) != activeFiducialListNode->GetLocked() )
      {
      this->MultiColumnList->GetWidget()->SetCellEditable( i, this->LockColumn, 1 );
      this->MultiColumnList->GetWidget()->SetCellEnabledAttribute (i,  this->LockColumn, 1 );
      this->MultiColumnList->GetWidget()->ColumnEditableOn( this->LockColumn );
      this->MultiColumnList->GetWidget()->SetCellTextAsInt(i, this->LockColumn, activeFiducialListNode->GetLocked() );
      this->MultiColumnList->GetWidget()->SetCellWindowCommandToCheckButton(i, this->LockColumn);
      this->MultiColumnList->GetWidget()->SetCellEditable( i, this->LockColumn, 0 );
      this->MultiColumnList->GetWidget()->SetCellEnabledAttribute (i,  this->LockColumn, 0 );
      this->MultiColumnList->GetWidget()->ColumnEditableOff( this->LockColumn );

      }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::ModifyAllVisibility( int visibilityState)
{
  if ( this->MRMLScene == NULL )
    {
    vtkErrorMacro ( "ModifyAllVisibility: got NULL MRMLScene." );
    return;
    }
  if ( visibilityState != 0 && visibilityState != 1 )
    {
    vtkErrorMacro ( "ModifyAllVisibility: got bad value for lock state; should be 0 or 1" );
    return;
    }
  
  vtkMRMLFiducialListNode *flNode;
  
  // save state for undo:
  // maybe we should just make a list of all the fiducialListNodes
  // and save their state here instead of the entire scene?
  this->MRMLScene->SaveStateForUndo();
  int numnodes = this->MRMLScene->GetNumberOfNodesByClass ( "vtkMRMLFiducialListNode" );
  for ( int nn=0; nn<numnodes; nn++ )
    {
    flNode = vtkMRMLFiducialListNode::SafeDownCast (this->MRMLScene->GetNthNodeByClass ( nn, "vtkMRMLFiducialListNode" ));
    if ( flNode != NULL )
      {
      flNode->SetAllFiducialsVisibility ( visibilityState );
      flNode->SetVisibility ( visibilityState );
      }
    }
  
  // update the fiducial visibility parameter in the view node too.
  // TODO: when there are multiple views, use active view instead of 0th.
  // vtkMRMLViewNode *vn = vtkMRMLViewNode::SafeDownCast(
  // this->GetMRMLScene()->GetNthNodeByClass ( 0, "vtkMRMLViewNode"));
  // if (vn != NULL )
  // {
  // vn->SetFiducialsVisible ( activeFiducialListNode->GetVisibility());
  // }


  // update the icon in the DISPLAY panel to match the active list node..
  vtkMRMLFiducialListNode *activeFiducialListNode = (vtkMRMLFiducialListNode *)this->MRMLScene->GetNodeByID(this->GetFiducialListNodeID());
  if ( activeFiducialListNode == NULL )
    {
    vtkErrorMacro ( "ModifyAllVisibility: got NULL activeFiducialListNode." );
    return;
    }

  if ( this->GetApplicationGUI() == NULL )
    {
    vtkErrorMacro ( "ModifyAllVisibility: Got NULL ApplicationGUI" );
    return;
    }
  if ( this->GetApplicationGUI()->GetSlicerFoundationIcons() == NULL )
    {
    vtkErrorMacro ( "ModifyAllVisibility: Got NULL SlicerFoundationIcons." );
    return;
    }
  if (activeFiducialListNode->GetVisibility() > 0)
    {
    this->GetVisibilityToggle()->GetWidget()->SetImageToIcon(this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerCheckedVisibleIcon() );
    }
  else
    {
    this->GetVisibilityToggle()->GetWidget()->SetImageToIcon(this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerInvisibleIcon() );
    }

}


//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::ModifyListVisibility( int visibilityState)
{
  if ( this->MRMLScene == NULL )
    {
    vtkErrorMacro ( "ModifyListVisibility: got NULL MRMLScene." );
    return;
    }
  if ( visibilityState != 0 && visibilityState != 1 )
    {
    vtkErrorMacro ( "ModifyListVisibility: got bad value for lock state; should be 0 or 1" );
    return;
    }

  vtkMRMLFiducialListNode *activeFiducialListNode = (vtkMRMLFiducialListNode *)this->MRMLScene->GetNodeByID(this->GetFiducialListNodeID());
  if ( activeFiducialListNode == NULL )
    {
    vtkErrorMacro ( "ModifyListVisibility: got NULL activeFiducialListNode." );
    return;
    }
  
  // save state of node for undo
  this->MRMLScene->SaveStateForUndo(activeFiducialListNode);

  // change the visibility on the list
  activeFiducialListNode->SetAllFiducialsVisibility ( visibilityState );
  activeFiducialListNode->SetVisibility( visibilityState );

  // update the fiducial visibility parameter in the view node too.
  // TODO: when there are multiple views, use active view instead of 0th.
  // vtkMRMLViewNode *vn = vtkMRMLViewNode::SafeDownCast(
  // this->GetMRMLScene()->GetNthNodeByClass ( 0, "vtkMRMLViewNode"));
  // if (vn != NULL )
  // {
  // vn->SetFiducialsVisible ( activeFiducialListNode->GetVisibility());
  // }

  // dis/enable the Visibile column depending on if the list is visible?
  //this->MultiColumnList->GetWidget()->SetColumnEditable(this->VisibilityColumn, (activeFiducialListNode->GetVisibility() > 0 ? 1 : 0));
                                                           
  // update the icon in the DISPLAY panel
  if ( this->GetApplicationGUI() == NULL )
    {
    vtkErrorMacro ( "ModifyListVisibility: Got NULL ApplicationGUI" );
    return;
    }
  if ( this->GetApplicationGUI()->GetSlicerFoundationIcons() == NULL )
    {
    vtkErrorMacro ( "ModifyListVisibility: Got NULL SlicerFoundationIcons." );
    return;
    }
  if (activeFiducialListNode->GetVisibility() > 0)
    {
    this->GetVisibilityToggle()->GetWidget()->SetImageToIcon(this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerCheckedVisibleIcon() );
    }
  else
    {
    this->GetVisibilityToggle()->GetWidget()->SetImageToIcon(this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerInvisibleIcon() );
    }
}




//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::ProcessLogicEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::ProcessMRMLEvents ( vtkObject *caller,
                                             unsigned long event, void *callData )
{
    
  vtkDebugMacro("vtkSlicerFiducialsGUI::ProcessMRMLEvents: event = " << event << ".\n");
  
  if (event == vtkCommand::WidgetValueChangedEvent)
    {
    vtkDebugMacro("got a widget value changed event... the list node was changed.\n");
    }
  
  // first check to see if there was a fiducial list node deleted
  if (vtkMRMLScene::SafeDownCast(caller) != NULL &&
      vtkMRMLScene::SafeDownCast(caller) == this->MRMLScene &&
      event == vtkMRMLScene::NodeRemovedEvent)
    {
    vtkDebugMacro("vtkSlicerFiducialsGUI::ProcessGUIEvents: got a node deleted event on scene");
    // check to see if it was the current node that was deleted
    if (callData != NULL)
      {
      vtkMRMLNode *delNode = (vtkMRMLNode *)callData;
      if (delNode != NULL &&
          delNode->GetID() == this->GetFiducialListNodeID())
        {
        vtkDebugMacro("My node got deleted " << this->GetFiducialListNodeID());
//        this->SetFiducialListNodeID(NULL);
        }
      }
    }

    // did the selected node get modified?
    if (this->ApplicationLogic)
      {
      vtkMRMLSelectionNode *selnode = this->ApplicationLogic->GetSelectionNode();
      if (selnode != NULL
          && vtkMRMLSelectionNode::SafeDownCast(caller) == selnode
          && event == vtkCommand::ModifiedEvent)
        {
        vtkDebugMacro("The selection node changed\n");
        // is the active fid list id out of synch with our selection?
        if (selnode->GetActiveFiducialListID() != NULL &&
            this->GetFiducialListNodeID() != NULL)
          {
          if (strcmp(selnode->GetActiveFiducialListID(), this->GetFiducialListNodeID()) != 0)
            {
            vtkDebugMacro("Updating the fid gui's fid list node id\n");
            this->SetFiducialListNodeID(selnode->GetActiveFiducialListID());
            }
          }
        }
      }

    
    vtkMRMLFiducialListNode *node = vtkMRMLFiducialListNode::SafeDownCast(caller);
    vtkMRMLFiducialListNode *activeFiducialListNode = (vtkMRMLFiducialListNode *)this->MRMLScene->GetNodeByID(this->GetFiducialListNodeID());
    
    // check for a node added event
    if (//vtkMRMLScene::SafeDownCast(caller) != NULL &&
        //vtkMRMLScene::SafeDownCast(caller) == this->MRMLScene &&
        node != NULL &&
        event == vtkMRMLScene::NodeAddedEvent)
      {
      vtkDebugMacro("vtkSlicerFiducialsGUI::ProcessGUIEvents: got a node added event on scene");
      // check to see if it was a fid node    
      if (callData != NULL)
        {
        //vtkMRMLNode *addNode = (vtkMRMLNode *)callData;
        vtkMRMLFiducialListNode *addNode = reinterpret_cast<vtkMRMLFiducialListNode*>(callData);
        if (addNode != NULL &&
            addNode->IsA("vtkMRMLFiducialListNode"))
          {
          vtkDebugMacro("Got a node added event on a fiducial list node " << addNode->GetID());
          // is it the currently active one?
          if (addNode == activeFiducialListNode)
            {
            SetGUIFromList(activeFiducialListNode);
            return;
            }
          }
        }
      }

    if (node == activeFiducialListNode)
      {
      if (event == vtkCommand::ModifiedEvent || event == vtkMRMLScene::NodeAddedEvent || event == vtkMRMLScene::NodeRemovedEvent)
        {
        vtkDebugMacro("Modified or node added or removed event on the fiducial list node.\n");
        if (node == NULL)
          {
          vtkDebugMacro("\tBUT: the node is null\n");
          return;
          }
        vtkDebugMacro("ProcessMRMLEvents: \t\tUpdating the GUI\n");
        // update the table
        SetGUIFromList(activeFiducialListNode);
        return;
        }
      else if ( event == vtkMRMLFiducialListNode::FiducialModifiedEvent)
        {
        vtkDebugMacro("\tfiducial modified event on the active fiducial list.");
        if (node == NULL)
          {
          return;
          }
        vtkDebugMacro("ProcessMRMLEvents: setting the gui from the acitve fid list node");
        SetGUIFromList(activeFiducialListNode);
        return;
        }
      else if (event == vtkMRMLFiducialListNode::DisplayModifiedEvent)
        {
        vtkDebugMacro("vtkSlicerFiducialsGUI::ProcessMRMLEvents: DisplayModified event on the fiducial list node...\n");
        this->SetGUIDisplayFrameFromList(activeFiducialListNode);
        }
      } // end of events on the active fiducial list node

    if (node == vtkMRMLFiducialListNode::SafeDownCast(this->FiducialListSelectorWidget->GetSelected()) && event == vtkCommand::ModifiedEvent)
      {
      vtkDebugMacro("\tmodified event on the fiducial list selected node.\n");
      if (activeFiducialListNode !=  vtkMRMLFiducialListNode::SafeDownCast(this->FiducialListSelectorWidget->GetSelected()))
        {
        // select it first off
        this->SetFiducialListNode(vtkMRMLFiducialListNode::SafeDownCast(this->FiducialListSelectorWidget->GetSelected()));
        }
      vtkDebugMacro("Setting gui from list after display modified event");
      SetGUIFromList(activeFiducialListNode);
      return;        
      }    
    

    // did the scene close?
    if (event == vtkMRMLScene::SceneCloseEvent)
      {
      vtkDebugMacro("Scene closing, deleting rows");
      if (this->MultiColumnList && this->MultiColumnList->GetWidget())
        {
        this->MultiColumnList->GetWidget()->DeleteAllRows();
        }
      }
    vtkDebugMacro("vtkSlicerFiducialsGUI: Done processing mrml events...");
//    std::cerr << "vtkSlicerFiducialsGUI::ProcessMRMLEvents  mismatched
//    caller and event (" << event << ")\n";
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::UpdateGUI()
{
    // is it built

    // get the selected list
    if (this->ApplicationLogic)
      {
      vtkMRMLSelectionNode *selnode = this->ApplicationLogic->GetSelectionNode();
      if (selnode->GetActiveFiducialListID() != NULL)
        {
        if (this->GetFiducialListNodeID() != NULL)
          {
          if (strcmp(selnode->GetActiveFiducialListID(), this->GetFiducialListNodeID()) != 0)
            {
            this->SetFiducialListNodeID(selnode->GetActiveFiducialListID());
            }
          }
        else
          {
          // it wasn't set yet
          this->SetFiducialListNodeID(selnode->GetActiveFiducialListID());
          }
        }
      else
        {
        vtkDebugMacro("UpdateGUI: the selection node is null, can't get the scene's currently active fid list");
        }
      }
    
    // update the gui
    if (this->GetFiducialListNodeID() != NULL)
      {
      vtkDebugMacro("Setting gui from the scene's fid list poitned to by this fid list node id");
      SetGUIFromList((vtkMRMLFiducialListNode *)this->MRMLScene->GetNodeByID(this->GetFiducialListNodeID()));
      }
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::SetGUIFromList(vtkMRMLFiducialListNode * activeFiducialListNode)
{
    
    if (activeFiducialListNode == NULL)
      {      
      //clear out the list box
      if (this->MultiColumnList)
        {
        if (this->MultiColumnList->GetWidget()->GetNumberOfRows() != 0)
          {
          vtkDebugMacro("SetGUIFromList: passed in active fid list node is null, clearing outthe list box");
          this->MultiColumnList->GetWidget()->DeleteAllRows();
          }
        }
      vtkDebugMacro("SetGUIFromList: passed in active fid list node is null, returning");
      return;
      }

    // make sure that it's selected
    if (this->FiducialListSelectorWidget->GetSelected() == NULL)
      {
      vtkDebugMacro("Null selected fid list, selecting it");
      this->FiducialListSelectorWidget->SetSelected(activeFiducialListNode);
      return;
      }
    if (activeFiducialListNode && this->FiducialListSelectorWidget->GetSelected() &&
        strcmp(activeFiducialListNode->GetName(),
               this->FiducialListSelectorWidget->GetSelected()->GetName()) != 0)
      {
      vtkDebugMacro("SetGUIFromList: input list " << activeFiducialListNode->GetName() << " doesn't match selector widget value: " << this->FiducialListSelectorWidget->GetSelected()->GetName());
      //this->FiducialListSelectorWidget->GetWidget()->GetWidget()->SetValue(activeFiducialListNode->GetName());
      this->FiducialListSelectorWidget->SetSelected(activeFiducialListNode);
      vtkDebugMacro("... returning, hoping for a invoke event");
      return;
      }
    
    int numPoints = activeFiducialListNode->GetNumberOfFiducials();
    bool deleteFlag = true;

    vtkDebugMacro("SetGUIFromList: have " << numPoints << " points in the list");
    
    if (numPoints != this->MultiColumnList->GetWidget()->GetNumberOfRows())
      {
      vtkDebugMacro("SetGUIFromList: numPoints " << numPoints << " doesn't match number of rows " << this->MultiColumnList->GetWidget()->GetNumberOfRows() << ", so deleting all of them and starting from scratch\n");
      
      // clear out the multi column list box and fill it in with the
      // new list
      this->MultiColumnList->GetWidget()->DeleteAllRows();
      }
    else
      {
      deleteFlag = false;
      }

    // a row for each point
        
    float *xyz;
    float *wxyz;
    for (int row = 0; row < numPoints; row++)
      {
      // add a row for this point
      if (deleteFlag)
        {
        vtkDebugMacro("SetGUIFromList: Adding point " << row << " to the table" << endl);
        this->MultiColumnList->GetWidget()->AddRow();
        }
      vtkDebugMacro("SetGUIFromList: getting " << row << "th fiducial xyz - total fids = " << numPoints);
      // now populate it
      xyz = activeFiducialListNode->GetNthFiducialXYZ(row);
      if (xyz == NULL)
        {
        vtkErrorMacro ("SetGUIFromList: ERROR: got null xyz for point " << row << endl);
        }
      vtkDebugMacro("Getting nth fiducial orientation");            
      wxyz = activeFiducialListNode->GetNthFiducialOrientation(row);
      
      if (activeFiducialListNode->GetNthFiducialLabelText(row) != NULL)
        {
        if (strcmp(this->MultiColumnList->GetWidget()->GetCellText(row,this->NameColumn), activeFiducialListNode->GetNthFiducialLabelText(row)) != 0)
          {
          this->MultiColumnList->GetWidget()->SetCellText(row,this->NameColumn,activeFiducialListNode->GetNthFiducialLabelText(row));
          }               
        }
      else
        {
        if (strcmp(this->MultiColumnList->GetWidget()->GetCellText(row,this->NameColumn), "(none)") != 0)
          {
          this->MultiColumnList->GetWidget()->SetCellText(row,this->NameColumn,"(none)");
          }
        }
      // selected
      if (deleteFlag || this->MultiColumnList->GetWidget()->GetCellTextAsInt(row,this->SelectedColumn) != (activeFiducialListNode->GetNthFiducialSelected(row) ? 1 : 0))
        {
        this->MultiColumnList->GetWidget()->SetCellTextAsInt(row,this->SelectedColumn,(activeFiducialListNode->GetNthFiducialSelected(row) ? 1 : 0));
        this->MultiColumnList->GetWidget()->SetCellWindowCommandToCheckButton(row,this->SelectedColumn);
        this->UpdateMeasurementLabel();
        }
      // locked
      if (deleteFlag || this->MultiColumnList->GetWidget()->GetCellTextAsInt(row,this->LockColumn) != (activeFiducialListNode->GetLocked() ? 1 : 0))
        {
        // -- trying to keep this from being editable.
        this->MultiColumnList->GetWidget()->SetCellEditable( row, this->LockColumn, 1 );
        this->MultiColumnList->GetWidget()->ColumnEditableOn( this->LockColumn );
        this->MultiColumnList->GetWidget()->SetCellTextAsInt(row, this->LockColumn,(activeFiducialListNode->GetLocked() ? 1 : 0));
        this->MultiColumnList->GetWidget()->SetCellWindowCommandToCheckButton(row,this->LockColumn);
        this->MultiColumnList->GetWidget()->SetCellEditable( row, this->LockColumn, 0 );
        this->MultiColumnList->GetWidget()->ColumnEditableOff( this->LockColumn );
        this->MultiColumnList->GetWidget()->SetCellEnabledAttribute (row,  this->LockColumn, 0 );
        }
      // visible
      if (deleteFlag || this->MultiColumnList->GetWidget()->GetCellTextAsInt(row,this->VisibilityColumn) != (activeFiducialListNode->GetNthFiducialVisibility(row) ? 1 : 0))
        {
        this->MultiColumnList->GetWidget()->SetCellTextAsInt(row,this->VisibilityColumn,(activeFiducialListNode->GetNthFiducialVisibility(row) ? 1 : 0));
        this->MultiColumnList->GetWidget()->SetCellWindowCommandToCheckButton(row,this->VisibilityColumn);
        }
      if (xyz != NULL)
        {
        // always set it if it's a new row added because all were
        // deleted, because the numerical default is 0
        if (deleteFlag || this->MultiColumnList->GetWidget()->GetCellTextAsDouble(row,this->XColumn) != xyz[0])
          {
          this->MultiColumnList->GetWidget()->SetCellTextAsDouble(row,this->XColumn,xyz[0]);
          } 
        if (deleteFlag || this->MultiColumnList->GetWidget()->GetCellTextAsDouble(row,this->YColumn) != xyz[1])
          {
          this->MultiColumnList->GetWidget()->SetCellTextAsDouble(row,this->YColumn,xyz[1]);
          }
        if (deleteFlag || this->MultiColumnList->GetWidget()->GetCellTextAsDouble(row,this->ZColumn) != xyz[2])
          {
          this->MultiColumnList->GetWidget()->SetCellTextAsDouble(row,this->ZColumn,xyz[2]);
          }
        this->UpdateMeasurementLabel();
        }
      if (wxyz != NULL)
        {
        if (deleteFlag || this->MultiColumnList->GetWidget()->GetCellTextAsDouble(row,this->OrWColumn) != wxyz[0])
          {
          this->MultiColumnList->GetWidget()->SetCellTextAsDouble(row,this->OrWColumn,wxyz[0]);
          }
        if (deleteFlag || this->MultiColumnList->GetWidget()->GetCellTextAsDouble(row,this->OrXColumn) != wxyz[1])
          {
          this->MultiColumnList->GetWidget()->SetCellTextAsDouble(row,this->OrXColumn,wxyz[1]);
          }
        if (deleteFlag || this->MultiColumnList->GetWidget()->GetCellTextAsDouble(row,this->OrYColumn) != wxyz[2])
          {
          this->MultiColumnList->GetWidget()->SetCellTextAsDouble(row,this->OrYColumn,wxyz[2]);
          }
        if (deleteFlag || this->MultiColumnList->GetWidget()->GetCellTextAsDouble(row,this->OrZColumn) != wxyz[3])
          {
          this->MultiColumnList->GetWidget()->SetCellTextAsDouble(row,this->OrZColumn,wxyz[3]);
          }
        }
      }
    
    vtkDebugMacro("Now going to update GUI from the logic's active list");
    // update the visibility, color, scale buttons to match the displayed list's
    if (activeFiducialListNode == NULL)
      {
      vtkErrorMacro ("vtkSlicerFiducialsGUI::SetGUIFromList: ERROR: no active fiducial list node in the gui class!\n");                
      return;
      }
    this->SetGUIDisplayFrameFromList(activeFiducialListNode);
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::SetGUIDisplayFrameFromList(vtkMRMLFiducialListNode * activeFiducialListNode)
{
    vtkDebugMacro(<< "\tupdating the visibility button\n");

    // color
    vtkDebugMacro(<< "\tupdating the colour\n");
    double *nodeColor = activeFiducialListNode->GetColor();
    if (this->ListColorButton != NULL)
      {
      double *buttonColor = this->ListColorButton->GetColor();
      if (nodeColor != NULL && buttonColor != NULL && 
          (nodeColor[0] != buttonColor[0] ||
           nodeColor[1] != buttonColor[1] ||
           nodeColor[2] != buttonColor[2]))
        {
        vtkDebugMacro("Updating list color button\n");
        this->ListColorButton->SetColor(nodeColor);
        }
      }
    else
      {
      vtkErrorMacro("No colour button!\n");
      }
    
    // selected color
    vtkDebugMacro(<< "\tupdating the selected colour\n");
    double *nodeSelectedColor = activeFiducialListNode->GetSelectedColor();
    if (this->ListSelectedColorButton != NULL)
      {
      double *buttonSelectedColor = this->ListSelectedColorButton->GetColor();
      if (nodeSelectedColor != NULL && buttonSelectedColor != NULL && 
          (nodeSelectedColor[0] != buttonSelectedColor[0] ||
           nodeSelectedColor[1] != buttonSelectedColor[1] ||
           nodeSelectedColor[2] != buttonSelectedColor[2]))
        {
        vtkDebugMacro("Updating list selected color button\n");
        this->ListSelectedColorButton->SetColor(nodeSelectedColor);
        }
      }
    else
      {
      vtkErrorMacro("No selected colour button!\n");
      }
    
    // symbol scale
    vtkDebugMacro(<< "\tupdating the symbol scale");
    double scale = activeFiducialListNode->GetSymbolScale();
    if (this->ListSymbolScale != NULL &&
        scale != this->ListSymbolScale->GetValue())
      {
      this->ListSymbolScale->SetValue(scale);
      }
    
    
    // symbol glyph type
    vtkDebugMacro("\tupdating the symbol glyph type to " << activeFiducialListNode->GetGlyphType() << endl);
    const char * glyphType = activeFiducialListNode->GetGlyphTypeAsString();
    if (this->ListSymbolTypeMenu != NULL &&
        glyphType != this->ListSymbolTypeMenu->GetWidget()->GetValue())
      {
      vtkDebugMacro("\t\tmenu value was " << this->ListSymbolTypeMenu->GetWidget()->GetValue() << endl);
      this->ListSymbolTypeMenu->GetWidget()->SetValue(glyphType);
      }
    
    // text scale
    vtkDebugMacro(<< "\tupdating the text scale.");
    scale = activeFiducialListNode->GetTextScale();
    if (this->ListTextScale != NULL &&
        scale != this->ListTextScale->GetValue())
      {
      this->ListTextScale->SetValue(scale);
      }
    
    // opacity
    vtkDebugMacro(<< "\tupdating the opacity");
    scale = activeFiducialListNode->GetOpacity();
    if (this->ListOpacity != NULL &&
        scale != this->ListOpacity->GetValue())
      {
      this->ListOpacity->SetValue(scale);
      }

    // ambient
    scale = activeFiducialListNode->GetAmbient();
    if (this->ListAmbient != NULL &&
        scale != this->ListAmbient->GetValue())
      {
      this->ListAmbient->SetValue(scale);
      }
    // diffuse
    scale = activeFiducialListNode->GetDiffuse();
    if (this->ListDiffuse != NULL &&
        scale != this->ListDiffuse->GetValue())
      {
      this->ListDiffuse->SetValue(scale);
      }
    // specular
    scale = activeFiducialListNode->GetSpecular();
    if (this->ListSpecular != NULL &&
        scale != this->ListSpecular->GetValue())
      {
      this->ListSpecular->SetValue(scale);
      }
    // power
    scale = activeFiducialListNode->GetPower();
    if (this->ListPower != NULL &&
        scale != this->ListPower->GetValue())
      {
      this->ListPower->SetValue(scale);
      }
    
    // visibility
    if ( this->GetVisibilityToggle() == NULL )
      {
      vtkErrorMacro ( "SetGUIDisplayFrameFromList: Got NULL ApplicationGUI" );
      return;
      }
    if ( this->GetApplicationGUI() == NULL )
      {
      vtkErrorMacro ( "SetGUIDisplayFrameFromList: Got NULL ApplicationGUI" );
      return;
      }
    if ( this->GetApplicationGUI()->GetSlicerFoundationIcons() == NULL )
      {
      vtkErrorMacro ( "SetGUIDisplayFrameFromList: Got NULL SlicerFoundationIcons." );
      return;
      }
    if (activeFiducialListNode->GetVisibility() > 0)
      {
      this->GetVisibilityToggle()->GetWidget()->SetImageToIcon(this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerCheckedVisibleIcon() );
      }
    else
      {
      this->GetVisibilityToggle()->GetWidget()->SetImageToIcon(this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerInvisibleIcon() );
      }
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::CreateModuleEventBindings ( )
{
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::ReleaseModuleEventBindings ( )
{
  
}


//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::Enter ( vtkMRMLNode *node )
{
  if ( this->Built == false )
    {
    this->BuildGUI();
    this->Built = true;
    this->AddGUIObservers();
    }

  vtkMRMLFiducialListNode *fiducialListNode = vtkMRMLFiducialListNode::SafeDownCast(node);
  if ( fiducialListNode )
    {
    this->FiducialListSelectorWidget->SetSelected( fiducialListNode );
    }

  this->CreateModuleEventBindings();
  this->UpdateGUI();
}



//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::Exit ( )
{
  this->ReleaseModuleEventBindings();
}


//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::TearDownGUI ( )
{
  this->Exit();
  if ( this->Built )
    {
    this->RemoveGUIObservers();
    }
}



//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::BuildGUI ( )
{
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

    // ---
    // MODULE GUI FRAME 
    // configure a page for a model loading UI for now.
    // later, switch on the modulesButton in the SlicerControlGUI
    // ---
    // create a page
    this->UIPanel->AddPage ( "Fiducials", "Fiducials", NULL );
    
    // Define your help text and build the help frame here.
    const char *help = "The Fiducials Module creates and manages lists of Fiducial points. <a>http://www.slicer.org/slicerWiki/index.php/Modules:Fiducials-Documentation</a>.\nClick on the tool bar icon of an arrow pointing to a starburst fiducial to enter the 'place a new object mode', then click on 3D models or on 2D slices.\nYou can also place fiducials while in 'tranform view' mode by positioning the mouse over a 2D slice plane in the Slice view windows (it must be the active window) and pressing the 'P' key. You can then click and drag the fiducial using the mouse in 'transform view' mode.\nYou can reset the positions of the fiducials in the table below, and adjust selection (fiducials must be selected if they are to be passed into a command line module).\nTo align slices with fiducials, move the fiducial while holding down the Control key.\nYou can right click in a row to align slices to that fiducial, or in the 2d slice windows you can use the '`' key to jump to the next fiducial, Shift-` to jump backwards through the list.\nUse the backspace or delete key to delete a fiducial over which you are hovering in 2D.\nThe distance between the first two selected fiducials in the list will be computed automatically and appear in a label below the list of fiducials.";
    const char *about = "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details. ";
    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "Fiducials" );
    this->BuildHelpAndAboutFrame ( page, help, about );
    this->NACLabel = vtkKWLabel::New();
    this->NACLabel->SetParent ( this->GetLogoFrame() );
    this->NACLabel->Create();
    this->NACLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetNACLogo() );

    this->NAMICLabel = vtkKWLabel::New();
    this->NAMICLabel->SetParent ( this->GetLogoFrame() );
    this->NAMICLabel->Create();
    this->NAMICLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetNAMICLogo() );    

    this->NCIGTLabel = vtkKWLabel::New();
    this->NCIGTLabel->SetParent ( this->GetLogoFrame() );
    this->NCIGTLabel->Create();
    this->NCIGTLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetNCIGTLogo() );
    
    this->BIRNLabel = vtkKWLabel::New();
    this->BIRNLabel->SetParent ( this->GetLogoFrame() );
    this->BIRNLabel->Create();
    this->BIRNLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetBIRNLogo() );
    app->Script ( "grid %s -row 0 -column 0 -padx 2 -pady 2 -sticky w", this->NAMICLabel->GetWidgetName());
    app->Script ("grid %s -row 0 -column 1 -padx 2 -pady 2 -sticky w",  this->NACLabel->GetWidgetName());
    app->Script ( "grid %s -row 1 -column 0 -padx 2 -pady 2 -sticky w",  this->BIRNLabel->GetWidgetName());
    app->Script ( "grid %s -row 1 -column 1 -padx 2 -pady 2 -sticky w",  this->NCIGTLabel->GetWidgetName());                  

    //---
    // FIDUCIAL LIST FRAME
    //---
    vtkSlicerModuleCollapsibleFrame *fiducialListFrame = vtkSlicerModuleCollapsibleFrame::New();
    fiducialListFrame->SetParent ( page );
    fiducialListFrame->Create();
    fiducialListFrame->SetLabelText ("Create & Select Fiducial Lists" );
    fiducialListFrame->ExpandFrame();
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  fiducialListFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("Fiducials")->GetWidgetName());

    //---
    //--- create some temporary frames and a label for grid packaging.
    //---
    vtkKWFrame *f = vtkKWFrame::New();
    f->SetParent ( fiducialListFrame->GetFrame() );
    f->Create();
    vtkKWFrame *fA = vtkKWFrame::New();
    fA->SetParent ( f );
    fA->Create();
    vtkKWFrame *fB = vtkKWFrame::New();
    fB->SetParent ( f );
    fB->Create();
    //---
    //--- Here we are creating a 'stand-in' label for
    //--- the node selector widget because we want
    //--- to align the button row above with the left
    //--- side of the widget's menubutton, and we
    //--- cant predict exactly where that will be with
    //--- it's own label in place. So we'll set it's label
    //--- to have null text, and use this label in its
    //--- place.
    //---
    vtkKWLabel *lA = vtkKWLabel::New();
    lA->SetParent ( f );
    lA->Create();
    lA->SetText ( "Fiducial List:");
    
    // add a select all fiducials on this list button
    this->SelectAllFiducialsButton = vtkKWPushButton::New ( );
    this->SelectAllFiducialsButton->SetParent ( fA );
    this->SelectAllFiducialsButton->Create ( );
    this->SelectAllFiducialsButton->SetImageToIcon ( this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerFiducialsSelectAllIcon() );
    this->SelectAllFiducialsButton->SetReliefToFlat();
    this->SelectAllFiducialsButton->SetBorderWidth ( 0 );
    this->SelectAllFiducialsButton->SetBalloonHelpString("Select all fiducial points from all fiducial lists.");

     // add a deselect all fiducials on this list button
    this->DeselectAllFiducialsButton = vtkKWPushButton::New ( );
    this->DeselectAllFiducialsButton->SetParent ( fA );
    this->DeselectAllFiducialsButton->Create ( );
    this->DeselectAllFiducialsButton->SetImageToIcon ( this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerFiducialsSelectNoneIcon() );
    this->DeselectAllFiducialsButton->SetReliefToFlat();
    this->DeselectAllFiducialsButton->SetBorderWidth ( 0 );
    this->DeselectAllFiducialsButton->SetBalloonHelpString("Deselect all fiducial points from all fiducial lists.");

    // remove all fiducials from all lists and remove fiducial lists  button
    this->RemoveAllFiducialsButton = vtkKWPushButton::New ( );
    this->RemoveAllFiducialsButton->SetParent ( fA );
    this->RemoveAllFiducialsButton->Create ( );
    this->RemoveAllFiducialsButton->SetImageToIcon ( this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerFiducialsDeleteAllIcon() );
    this->RemoveAllFiducialsButton->SetReliefToFlat();
    this->RemoveAllFiducialsButton->SetBorderWidth ( 0 );
    this->RemoveAllFiducialsButton->SetBalloonHelpString("Remove all fiducial points from all fiducial lists.");

    //---
    //--- create all lock menu button and set up menu
    //---
    this->AllLockMenuButton = vtkKWMenuButton::New();
    this->AllLockMenuButton->SetParent ( fB);
    this->AllLockMenuButton->Create();
    this->AllLockMenuButton->SetBorderWidth(0);
    this->AllLockMenuButton->SetReliefToFlat();
    this->AllLockMenuButton->IndicatorVisibilityOff();
    this->AllLockMenuButton->SetImageToIcon ( this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerLockOrUnlockIcon() );
    this->AllLockMenuButton->SetBalloonHelpString ( "Lock or unlock all fiducials in all fiducial lists." );
    this->AllLockMenuButton->GetMenu()->AddRadioButton ( "Lock");
    int index = this->AllLockMenuButton->GetMenu()->GetIndexOfItem ("Lock");
    this->AllLockMenuButton->GetMenu()->SetItemImageToIcon (index, this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerLockIcon()  );
    this->AllLockMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
    this->AllLockMenuButton->GetMenu()->SetItemIndicatorVisibility ( index, 0);
    this->AllLockMenuButton->GetMenu()->AddRadioButton ( "Unlock");
    index = this->AllLockMenuButton->GetMenu()->GetIndexOfItem ("Unlock");
    this->AllLockMenuButton->GetMenu()->SetItemImageToIcon (index, this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerUnlockIcon()  );
    this->AllLockMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
    this->AllLockMenuButton->GetMenu()->SetItemIndicatorVisibility ( index, 0);
    this->AllLockMenuButton->GetMenu()->AddSeparator();
    this->AllLockMenuButton->GetMenu()->AddRadioButton ( "close");
    index = this->AllLockMenuButton->GetMenu()->GetIndexOfItem ("close");
    this->AllLockMenuButton->GetMenu()->SetItemIndicatorVisibility ( index, 0);

    //---
    //--- create all visibility menu button and set up menu
    //---    
    this->AllVisibilityMenuButton = vtkKWMenuButton::New();
    this->AllVisibilityMenuButton->SetParent ( fB );
    this->AllVisibilityMenuButton->Create();
    this->AllVisibilityMenuButton->SetBorderWidth(0);
    this->AllVisibilityMenuButton->SetReliefToFlat();
    this->AllVisibilityMenuButton->IndicatorVisibilityOff();
    this->AllVisibilityMenuButton->SetImageToIcon ( this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerVisibleOrInvisibleIcon() );
    this->AllVisibilityMenuButton->SetBalloonHelpString ( "Set hide or expose all fiducial lists, and set visibility on all fiducials in them." );
    this->AllVisibilityMenuButton->GetMenu()->AddRadioButton ( "Visible");
    index = this->AllVisibilityMenuButton->GetMenu()->GetIndexOfItem ("Visible");
    this->AllVisibilityMenuButton->GetMenu()->SetItemImageToIcon (index, this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerVisibleIcon()  );
    this->AllVisibilityMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
    this->AllVisibilityMenuButton->GetMenu()->SetItemIndicatorVisibility ( index, 0);
    this->AllVisibilityMenuButton->GetMenu()->AddRadioButton ( "Invisible");
    index = this->AllVisibilityMenuButton->GetMenu()->GetIndexOfItem ("Invisible");
    this->AllVisibilityMenuButton->GetMenu()->SetItemImageToIcon (index, this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerInvisibleIcon()  );
    this->AllVisibilityMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
    this->AllVisibilityMenuButton->GetMenu()->SetItemIndicatorVisibility ( index, 0);
    this->AllVisibilityMenuButton->GetMenu()->AddSeparator();
    this->AllVisibilityMenuButton->GetMenu()->AddRadioButton ( "close");
    index = this->AllVisibilityMenuButton->GetMenu()->GetIndexOfItem ("close");
    this->AllVisibilityMenuButton->GetMenu()->SetItemIndicatorVisibility ( index, 0);
    
    //---
    //--- create node selector
    //---
    this->FiducialListSelectorWidget = vtkSlicerNodeSelectorWidget::New();
    this->FiducialListSelectorWidget->SetParent( f );
    this->FiducialListSelectorWidget->Create();
    this->FiducialListSelectorWidget->SetNodeClass("vtkMRMLFiducialListNode", NULL, NULL, NULL);
    this->FiducialListSelectorWidget->NewNodeEnabledOn();
    this->FiducialListSelectorWidget->SetMRMLScene(this->GetMRMLScene());
    this->FiducialListSelectorWidget->SetBorderWidth(2);
    this->FiducialListSelectorWidget->SetPadX(2);
    this->FiducialListSelectorWidget->SetPadY(2);
    this->FiducialListSelectorWidget->GetWidget()->SetWidth(24);
    this->FiducialListSelectorWidget->SetLabelText( "");
    this->FiducialListSelectorWidget->SetBalloonHelpString("Select a fiducial list from the current MRML scene.");

    //---
    //--- package these up to look nice.
    //---
    app->Script( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2", f->GetWidgetName() );

    app->Script ( "grid %s -row 0 -column 1 -sticky w -padx 8 -pady 2", fA->GetWidgetName() );
    app->Script ( "grid %s -row 0 -column 2 -sticky e -padx 4 -pady 2", fB->GetWidgetName() );
    app->Script ( "grid %s -row 1 -column 0 -sticky w -padx 1 -pady 2", lA->GetWidgetName() );
    app->Script ( "grid %s -row 1 -column 1 -columnspan 2 -sticky ew -padx 0 -pady 2",
                  this->FiducialListSelectorWidget->GetWidgetName() );
    app->Script ( "grid columnconfigure %s 0 -weight 0", f->GetWidgetName() );
    app->Script ( "grid columnconfigure %s 1 -weight 0", f->GetWidgetName() );
    app->Script ( "grid columnconfigure %s 2 -weight 1", f->GetWidgetName() );

    app->Script("pack %s %s %s -side left -anchor w -padx 2 -pady 2", 
                this->SelectAllFiducialsButton->GetWidgetName(),
                this->DeselectAllFiducialsButton->GetWidgetName(),
                this->RemoveAllFiducialsButton->GetWidgetName() );
    app->Script("pack %s %s -side left -anchor e -padx 2 -pady 2", 
                this->AllLockMenuButton->GetWidgetName(),
                this->AllVisibilityMenuButton->GetWidgetName() );


    //---
    // FIDUCIAL FRAME
    //---
    vtkSlicerModuleCollapsibleFrame *fiducialFrame = vtkSlicerModuleCollapsibleFrame::New();
    fiducialFrame->SetParent ( fiducialListFrame->GetFrame() );
    fiducialFrame->Create();
    fiducialFrame->SetLabelText ("Add, Configure & Delete Fiducials" );
    fiducialFrame->ExpandFrame();
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                  fiducialFrame->GetWidgetName());

    //---
    //--- create some temporary frames for the buttons
    //---
    vtkKWFrame *f0 = vtkKWFrame::New();
    f0->SetParent ( fiducialFrame->GetFrame() );
    f0->Create();
    vtkKWFrame *f1 = vtkKWFrame::New();
    f1->SetParent ( f0 );
    f1->Create();
    vtkKWFrame *f2 = vtkKWFrame::New();
    f2->SetParent ( f0 );
    f2->Create();
    vtkKWFrame *f3 = vtkKWFrame::New();
    f3->SetParent ( f0 );
    f3->Create();
    vtkKWFrame *f4 = vtkKWFrame::New();
    f4->SetParent ( f0 );
    f4->Create();
    vtkKWFrame *f5 = vtkKWFrame::New();
    f5->SetParent ( f0 );
    f5->Create();

    //---
    // Buttons
    //---

     // add fiducial button
    this->AddFiducialButton = vtkKWPushButton::New ( );
    this->AddFiducialButton->SetParent ( f1 );
    this->AddFiducialButton->Create ( );
    this->AddFiducialButton->SetImageToIcon ( this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerFiducialsAddNewIcon() );
    this->AddFiducialButton->SetBorderWidth ( 0 );
    this->AddFiducialButton->SetReliefToFlat();
    this->AddFiducialButton->SetBalloonHelpString("Add a fiducial point to the current fiducial list");
    

    // add a select all fiducials on this list button
    this->SelectAllFiducialsInListButton = vtkKWPushButton::New ( );
    this->SelectAllFiducialsInListButton->SetParent ( f2 );
    this->SelectAllFiducialsInListButton->Create ( );
    this->SelectAllFiducialsInListButton->SetImageToIcon ( this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerFiducialsSelectAllInListIcon() );
    this->SelectAllFiducialsInListButton->SetReliefToFlat();
    this->SelectAllFiducialsInListButton->SetBorderWidth ( 0 );
    this->SelectAllFiducialsInListButton->SetBalloonHelpString("Select all fiducial points from this fiducial list.");

     // deselect all fiducials on this list button
    this->DeselectAllFiducialsInListButton = vtkKWPushButton::New ( );
    this->DeselectAllFiducialsInListButton->SetParent ( f2 );
    this->DeselectAllFiducialsInListButton->Create ( );
    this->DeselectAllFiducialsInListButton->SetImageToIcon ( this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerFiducialsSelectNoneInListIcon() );
    this->DeselectAllFiducialsInListButton->SetReliefToFlat();
    this->DeselectAllFiducialsInListButton->SetBorderWidth ( 0 );
    this->DeselectAllFiducialsInListButton->SetBalloonHelpString("Deselect all fiducial points from this fiducial list.");

    // remove fiducial button
    this->RemoveFiducialButton = vtkKWPushButton::New ( );
    this->RemoveFiducialButton->SetParent ( f3 );
    this->RemoveFiducialButton->Create ( );
    this->RemoveFiducialButton->SetImageToIcon ( this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerFiducialsDeleteLastClickedIcon() );
    this->RemoveFiducialButton->SetReliefToFlat();
    this->RemoveFiducialButton->SetBorderWidth ( 0 );
    this->RemoveFiducialButton->SetBalloonHelpString("Remove the last fiducial that was \"clicked on\" in the table from this fiducial list.");

    // add a remove all fiducials from this list button"
    this->RemoveFiducialsInListButton = vtkKWPushButton::New ( );
    this->RemoveFiducialsInListButton->SetParent ( f3 );
    this->RemoveFiducialsInListButton->Create ( );
    this->RemoveFiducialsInListButton->SetImageToIcon ( this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerFiducialsDeleteAllInListIcon() );
    this->RemoveFiducialsInListButton->SetReliefToFlat();
    this->RemoveFiducialsInListButton->SetBorderWidth ( 0 );
    this->RemoveFiducialsInListButton->SetBalloonHelpString("Remove all fiducial points from this fiducial list.");

    // add buttons to move the selected fiducial up or down
    this->MoveSelectedFiducialUpButton = vtkKWPushButton::New();
    this->MoveSelectedFiducialUpButton->SetParent ( f4 );
    this->MoveSelectedFiducialUpButton->Create ( );
    this->MoveSelectedFiducialUpButton->SetImageToIcon ( this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerFiducialsUpIcon() );
    this->MoveSelectedFiducialUpButton->SetReliefToFlat();
    this->MoveSelectedFiducialUpButton->SetBorderWidth ( 0 );
    this->MoveSelectedFiducialUpButton->SetBalloonHelpString("Move the last fiducial \"clicked on\" in the table one row up in the table");


    this->MoveSelectedFiducialDownButton = vtkKWPushButton::New();
    this->MoveSelectedFiducialDownButton->SetParent ( f4 );
    this->MoveSelectedFiducialDownButton->Create ( );
    this->MoveSelectedFiducialDownButton->SetImageToIcon ( this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerFiducialsDownIcon() );
    this->MoveSelectedFiducialDownButton->SetReliefToFlat();
    this->MoveSelectedFiducialDownButton->SetBorderWidth ( 0 );
    this->MoveSelectedFiducialDownButton->SetBalloonHelpString("Move the last fiducial \"clicked on\" in the table one row down in the table");

    //---
    //--- create all lock menu button and set up menu
    //---
    this->ListLockMenuButton = vtkKWMenuButton::New();
    this->ListLockMenuButton->SetParent ( f5 );
    this->ListLockMenuButton->Create();
    this->ListLockMenuButton->SetBorderWidth(0);
    this->ListLockMenuButton->SetReliefToFlat();
    this->ListLockMenuButton->IndicatorVisibilityOff();    
    this->ListLockMenuButton->SetImageToIcon ( this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerLockOrUnlockIcon() );
    this->ListLockMenuButton->SetBalloonHelpString ( "Lock or unlock all fiducials in this fiducial list. (Note: locking/unlocking individual fiducials is not yet implemented.)" );
    this->ListLockMenuButton->GetMenu()->AddRadioButton ( "Lock");
    index = this->ListLockMenuButton->GetMenu()->GetIndexOfItem ("Lock");
    this->ListLockMenuButton->GetMenu()->SetItemImageToIcon (index, this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerLockIcon()  );
    this->ListLockMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
    this->ListLockMenuButton->GetMenu()->SetItemIndicatorVisibility ( index, 0);
    this->ListLockMenuButton->GetMenu()->AddRadioButton ( "Unlock");
    index = this->ListLockMenuButton->GetMenu()->GetIndexOfItem ("Unlock");
    this->ListLockMenuButton->GetMenu()->SetItemImageToIcon (index, this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerUnlockIcon()  );
    this->ListLockMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
    this->ListLockMenuButton->GetMenu()->SetItemIndicatorVisibility ( index, 0);
    this->ListLockMenuButton->GetMenu()->AddSeparator();
    this->ListLockMenuButton->GetMenu()->AddRadioButton ( "close");
    index = this->ListLockMenuButton->GetMenu()->GetIndexOfItem ("close");
    this->ListLockMenuButton->GetMenu()->SetItemIndicatorVisibility ( index, 0);
    
    //---
    //--- create all visibility menu button and set up menu
    //---    
    this->ListVisibilityMenuButton = vtkKWMenuButton::New();
    this->ListVisibilityMenuButton->SetParent (f5 );
    this->ListVisibilityMenuButton->Create();
    this->ListVisibilityMenuButton->SetBorderWidth(0);
    this->ListVisibilityMenuButton->SetReliefToFlat();
    this->ListVisibilityMenuButton->IndicatorVisibilityOff();    
    this->ListVisibilityMenuButton->SetImageToIcon ( this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerVisibleOrInvisibleIcon() );
    this->ListVisibilityMenuButton->SetBalloonHelpString ( "Hide or expose the fiducial list and set visibility on all of its fiducials." );
    this->ListVisibilityMenuButton->GetMenu()->AddRadioButton ( "Visible");
    index = this->ListVisibilityMenuButton->GetMenu()->GetIndexOfItem ("Visible");
    this->ListVisibilityMenuButton->GetMenu()->SetItemImageToIcon (index, this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerVisibleIcon()  );
    this->ListVisibilityMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
    this->ListVisibilityMenuButton->GetMenu()->SetItemIndicatorVisibility ( index, 0);
    this->ListVisibilityMenuButton->GetMenu()->AddRadioButton ( "Invisible");
    index = this->ListVisibilityMenuButton->GetMenu()->GetIndexOfItem ("Invisible");
    this->ListVisibilityMenuButton->GetMenu()->SetItemImageToIcon (index, this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerInvisibleIcon()  );
    this->ListVisibilityMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
    this->ListVisibilityMenuButton->GetMenu()->SetItemIndicatorVisibility ( index, 0);
    this->ListVisibilityMenuButton->GetMenu()->AddSeparator();
    this->ListVisibilityMenuButton->GetMenu()->AddRadioButton ( "close");
    index = this->ListVisibilityMenuButton->GetMenu()->GetIndexOfItem ("close");
    this->ListVisibilityMenuButton->GetMenu()->SetItemIndicatorVisibility ( index, 0);
    
    //---
    //--- package these guys up to look nice.
    //---
    app->Script( "pack %s -side top -expand y -fill x -padx 0 -pady 2", f0->GetWidgetName() );
    app->Script( "pack %s -side left -anchor w -padx 6 -pady 2", f1->GetWidgetName() );
    app->Script( "pack %s -side left -anchor w -padx 6 -pady 2", f2->GetWidgetName() );
    app->Script( "pack %s -side left -anchor w -padx 6 -pady 2", f3->GetWidgetName() );
    app->Script( "pack %s -side left -anchor w -padx 6 -pady 2", f4->GetWidgetName() );    
    app->Script( "pack %s -side left -anchor e  -fill x -expand y -padx 2 -pady 2", f5->GetWidgetName() );

    app->Script( "pack %s -side left -anchor w -padx 2 -pady 2",
                 this->AddFiducialButton->GetWidgetName() );
    app->Script( "pack %s %s -side left -anchor nw -padx 2 -pady 2", 
                this->SelectAllFiducialsInListButton->GetWidgetName(),
                this->DeselectAllFiducialsInListButton->GetWidgetName() );
    app->Script( "pack %s %s -side left -anchor nw -padx 2 -pady 2", 
                this->RemoveFiducialButton->GetWidgetName(),
                this->RemoveFiducialsInListButton->GetWidgetName() );
    app->Script( "pack %s %s -side left -anchor nw -padx 2 -pady 2", 
                this->MoveSelectedFiducialUpButton->GetWidgetName(),
                this->MoveSelectedFiducialDownButton->GetWidgetName() );
    app->Script( "pack %s %s -side right -anchor ne -padx 2 -pady 2",
                 this->ListVisibilityMenuButton->GetWidgetName(),
                 this->ListLockMenuButton->GetWidgetName());

    
    //---
    // MultiColumn List
    //---
    vtkKWFrame *listFrame = vtkKWFrame::New();
    listFrame->SetParent( fiducialFrame->GetFrame() );
    listFrame->Create();
    app->Script ("pack %s -side top -anchor nw -fill x -pady 0",
                 listFrame->GetWidgetName());

    // add the multicolumn list to show the points
    this->MultiColumnList = vtkKWMultiColumnListWithScrollbars::New ( );
    this->MultiColumnList->SetParent ( listFrame );
    this->MultiColumnList->Create ( );
    this->MultiColumnList->SetHeight(4);
    this->MultiColumnList->GetWidget()->SetSelectionTypeToCell();
    this->MultiColumnList->GetWidget()->MovableRowsOff();
    this->MultiColumnList->GetWidget()->MovableColumnsOff();
    // set up the columns of data for each point
    // refer to the header file for order
    this->MultiColumnList->GetWidget()->AddColumn("Name");
    this->MultiColumnList->GetWidget()->AddColumn("Selected");
    // add the visibility column with no text, use an icon
    this->MultiColumnList->GetWidget()->AddColumn("Visible");
//    this->MultiColumnList->GetWidget()->SetColumnLabelImageToIcon(this->VisibilityColumn, this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerVisibleIcon() );
    this->MultiColumnList->GetWidget()->AddColumn("X");
    this->MultiColumnList->GetWidget()->AddColumn("Y");
    this->MultiColumnList->GetWidget()->AddColumn("Z");
    this->MultiColumnList->GetWidget()->AddColumn("OrW");
    this->MultiColumnList->GetWidget()->AddColumn("OrX");
    this->MultiColumnList->GetWidget()->AddColumn("OrY");
    this->MultiColumnList->GetWidget()->AddColumn("OrZ");
    this->MultiColumnList->GetWidget()->AddColumn("Locked");
    
    // make the selected, visible columns editable by checkbox
    this->MultiColumnList->GetWidget()->SetColumnEditWindowToCheckButton(this->SelectedColumn);
    this->MultiColumnList->GetWidget()->SetColumnEditWindowToCheckButton(this->VisibilityColumn);
    this->MultiColumnList->GetWidget()->SetColumnEditWindowToCheckButton(this->LockColumn);

    
    // now set the attributes that are equal across the columns
    int col;
    for (col = 0; col < this->NumberOfColumns; col++)
    {        
        this->MultiColumnList->GetWidget()->SetColumnWidth(col, 6);
        this->MultiColumnList->GetWidget()->SetColumnAlignmentToLeft(col);
        this->MultiColumnList->GetWidget()->ColumnEditableOn(col);
        if (col >= this->XColumn && col <= this->OrZColumn)
          {
          this->MultiColumnList->GetWidget()->SetColumnWidth(col, 11);
          this->MultiColumnList->GetWidget()->SetColumnEditWindowToSpinBox(col);
          }
    }

    // set some column widths to custom values
    this->MultiColumnList->GetWidget()->SetColumnWidth(this->NameColumn, 15);
    this->MultiColumnList->GetWidget()->SetColumnWidth(this->SelectedColumn, 8);
    this->MultiColumnList->GetWidget()->SetColumnWidth(this->VisibilityColumn, 7);
    this->MultiColumnList->GetWidget()->SetColumnWidth(this->LockColumn, 7);
    // turn user's ability to lock/unlock individual fiducials to OFF for now
    this->MultiColumnList->GetWidget()->ColumnEditableOff(this->LockColumn);
    
    // checkbox should be enough to convey state of these attributes; no need for 1|0 text.
    this->MultiColumnList->GetWidget()->SetColumnFormatCommandToEmptyOutput ( this->SelectedColumn );
    this->MultiColumnList->GetWidget()->SetColumnFormatCommandToEmptyOutput ( this->VisibilityColumn );
    this->MultiColumnList->GetWidget()->SetColumnFormatCommandToEmptyOutput ( this->LockColumn );
    
    app->Script ( "pack %s -fill both -expand true", this->MultiColumnList->GetWidgetName());
    this->MultiColumnList->GetWidget()->SetCellUpdatedCommand(this, "UpdateElement");
    // set up the right click jump slices to that fiducial point call back
    this->MultiColumnList->GetWidget()->SetRightClickCommand(this, "JumpSlicesCallback");
    //---
    // Point-to-Point measurement
    //---
    vtkKWFrame *measurementFrame = vtkKWFrame::New();
    measurementFrame->SetParent(fiducialFrame);
    measurementFrame->Create();
    app->Script ("pack %s -side top -anchor nw -fill x -pady 0",
                 measurementFrame->GetWidgetName());

    // add a label
    this->MeasurementLabel = vtkKWLabel::New();
    this->MeasurementLabel->SetParent( measurementFrame );
    this->MeasurementLabel->Create();
    this->MeasurementLabel->SetText("Distance: ");
    this->MeasurementLabel->SetBalloonHelpString("Distance between first two selected fiducials");
    app->Script("pack %s -side top -anchor nw -fill x -pady 0",
                this->MeasurementLabel->GetWidgetName());

    // ---
    // FIDUCIAL DISPLAY FRAME            
    vtkSlicerModuleCollapsibleFrame *fiducialDisplayFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    fiducialDisplayFrame->SetParent ( fiducialListFrame->GetFrame() );
    fiducialDisplayFrame->Create ( );
    fiducialDisplayFrame->SetLabelText ("Adjust Display Properties");
    fiducialDisplayFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                  fiducialDisplayFrame->GetWidgetName());
  
    // text scale
    this->ListTextScale = vtkKWScaleWithEntry::New();
    this->ListTextScale->SetParent( fiducialDisplayFrame->GetFrame() );
    this->ListTextScale->Create();
    this->ListTextScale->SetLabelText("Text Scale:");
    this->ListTextScale->SetLabelWidth ( 14 );
    this->ListTextScale->GetLabel()->SetAnchorToEast();
    this->ListTextScale->SetBalloonHelpString ( "Set the scale of the fiducial list points text.");
    this->ListTextScale->GetWidget()->SetRange(0.0, 20.0);
    this->ListTextScale->GetWidget()->SetOrientationToHorizontal ();
    this->ListTextScale->GetWidget()->SetResolution(0.5);
    this->ListTextScale->SetEntryWidth(5);

    // symbol scale
    this->ListSymbolScale = vtkKWScaleWithEntry::New();
    this->ListSymbolScale->SetParent( fiducialDisplayFrame->GetFrame() );
    this->ListSymbolScale->Create();
    this->ListSymbolScale->SetLabelText("Glyph Scale:");
    this->ListSymbolScale->GetLabel()->SetAnchorToEast();
    this->ListSymbolScale->SetLabelWidth ( 14 );
    this->ListSymbolScale->SetBalloonHelpString ( "Set the scale of the fiducial list symbols.");
    this->ListSymbolScale->GetWidget()->SetRange(0.0, 80.0);
    this->ListSymbolScale->GetWidget()->SetOrientationToHorizontal();
    this->ListSymbolScale->GetWidget()->SetResolution(0.5);
    this->ListSymbolScale->SetEntryWidth(5);

    // glyph type
    this->ListSymbolTypeMenu = vtkKWMenuButtonWithLabel::New();
    this->ListSymbolTypeMenu->SetParent( fiducialDisplayFrame->GetFrame() );
    this->ListSymbolTypeMenu->Create();
    this->ListSymbolTypeMenu->SetBorderWidth(0);
    this->ListSymbolTypeMenu->SetLabelWidth ( 14 );
    this->ListSymbolTypeMenu->GetLabel()->SetAnchorToEast();    
    this->ListSymbolTypeMenu->GetWidget()->SetWidth ( 13 );
    this->ListSymbolTypeMenu->SetBalloonHelpString("UNDER CONSTRUCTION: Change the type of glyph used to mark the fiducial list points");
    this->ListSymbolTypeMenu->SetLabelText("Glyph Type:");
    // add the valid glyph types
    vtkMRMLFiducialListNode * fidlist = vtkMRMLFiducialListNode::New();
    int glyphIndex = 0;
    for (int g = vtkMRMLFiducialListNode::GlyphMin;
         g <= vtkMRMLFiducialListNode::GlyphMax;
         g++)
      {
      this->ListSymbolTypeMenu->GetWidget()->GetMenu()->AddRadioButton(fidlist->GetGlyphTypeAsString(g));
//      this->ListSymbolTypeMenu->GetWidget()->GetMenu()->SetItemSelectedValueAsInt(glyphIndex, g);      
      glyphIndex++;
      }
    this->ListSymbolTypeMenu->GetWidget()->SetValue(fidlist->GetGlyphTypeAsString(vtkMRMLFiducialListNode::Diamond3D));
    fidlist->Delete();
    
    // visibility
    this->VisibilityToggle = vtkKWPushButtonWithLabel::New();
    this->VisibilityToggle->SetParent ( fiducialDisplayFrame->GetFrame() );
    this->VisibilityToggle->Create ( );
    this->VisibilityToggle->SetLabelWidth ( 14 );
    this->VisibilityToggle->SetLabelText ( "Hide or Expose:");
    this->VisibilityToggle->GetLabel()->SetAnchorToEast();
    this->VisibilityToggle->GetWidget()->SetReliefToFlat ( );
    this->VisibilityToggle->GetWidget()->SetOverReliefToNone ( );
    this->VisibilityToggle->GetWidget()->SetBorderWidth ( 0 );
    this->VisibilityToggle->GetWidget()->SetImageToIcon ( this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerInvisibleIcon() );
    this->VisibilityToggle->SetBalloonHelpString ( "Toggles visibility of all fiducial points in the selected list that have their visibility checked." );

    // opacity
    this->ListOpacity = vtkKWScaleWithEntry::New();
    this->ListOpacity->SetParent( fiducialDisplayFrame->GetFrame() );
    this->ListOpacity->Create();
    this->ListOpacity->SetLabelText("Opacity:");
    this->ListOpacity->GetLabel()->SetAnchorToEast();
    this->ListOpacity->SetLabelWidth ( 14 );
    this->ListOpacity->SetBalloonHelpString ( "Set the opacity of the fiducial list symbols.");
    this->ListOpacity->GetWidget()->SetRange(0.0, 1.0);
    this->ListOpacity->GetWidget()->SetOrientationToHorizontal();
    this->ListOpacity->GetWidget()->SetResolution(0.1);
    this->ListOpacity->SetEntryWidth(5);

    // ambient
    this->ListAmbient = vtkKWScaleWithEntry::New();
    this->ListAmbient->SetParent( fiducialDisplayFrame->GetFrame() );
    this->ListAmbient->Create();
    this->ListAmbient->SetLabelText("Ambient:");
    this->ListAmbient->GetLabel()->SetAnchorToEast();
    this->ListAmbient->SetLabelWidth ( 14 );
    this->ListAmbient->SetBalloonHelpString ( "Set the ambient light of the fiducial list.");
    this->ListAmbient->GetWidget()->SetRange(0.0, 1.0);
    this->ListAmbient->GetWidget()->SetOrientationToHorizontal();
    this->ListAmbient->GetWidget()->SetResolution(0.1);
    this->ListAmbient->SetEntryWidth(5);

    // diffuse
    this->ListDiffuse = vtkKWScaleWithEntry::New();
    this->ListDiffuse->SetParent( fiducialDisplayFrame->GetFrame() );
    this->ListDiffuse->Create();
    this->ListDiffuse->SetLabelText("Diffuse:");
    this->ListDiffuse->GetLabel()->SetAnchorToEast();
    this->ListDiffuse->SetLabelWidth ( 14 );
    this->ListDiffuse->SetBalloonHelpString ( "Set the diffuse light of the fiducial list.");
    this->ListDiffuse->GetWidget()->SetRange(0.0, 1.0);
    this->ListDiffuse->GetWidget()->SetOrientationToHorizontal();
    this->ListDiffuse->GetWidget()->SetResolution(0.1);
    this->ListDiffuse->SetEntryWidth(5);

    // specular
    this->ListSpecular = vtkKWScaleWithEntry::New();
    this->ListSpecular->SetParent( fiducialDisplayFrame->GetFrame() );
    this->ListSpecular->Create();
    this->ListSpecular->SetLabelText("Specular:");
    this->ListSpecular->GetLabel()->SetAnchorToEast();
    this->ListSpecular->SetLabelWidth ( 14 );
    this->ListSpecular->SetBalloonHelpString ( "Set the specular light of the fiducial list.");
    this->ListSpecular->GetWidget()->SetRange(0.0, 1.0);
    this->ListSpecular->GetWidget()->SetOrientationToHorizontal();
    this->ListSpecular->GetWidget()->SetResolution(0.1);
    this->ListSpecular->SetEntryWidth(5);

    // power
    this->ListPower = vtkKWScaleWithEntry::New();
    this->ListPower->SetParent( fiducialDisplayFrame->GetFrame() );
    this->ListPower->Create();
    this->ListPower->SetLabelText("Power:");
    this->ListPower->GetLabel()->SetAnchorToEast();
    this->ListPower->SetLabelWidth ( 14 );
    this->ListPower->SetBalloonHelpString ( "Set the power of the fiducial list.");
    this->ListPower->GetWidget()->SetRange(0.0, 1.0);
    this->ListPower->GetWidget()->SetOrientationToHorizontal();
    this->ListPower->GetWidget()->SetResolution(0.1);
    this->ListPower->SetEntryWidth(5);
    

    // color
    this->ListColorButton = vtkKWChangeColorButton::New();
    this->ListColorButton->SetParent( fiducialDisplayFrame->GetFrame() );
    this->ListColorButton->Create();
    this->ListColorButton->SetBorderWidth(0);
    this->ListColorButton->SetLabelOutsideButton(1);
    this->ListColorButton->GetLabel()->SetAnchorToEast();
    this->ListColorButton->SetLabelWidth ( 14 );
    this->ListColorButton->SetBalloonHelpString("Change the colour of the fiducial list symbols and text in the MainViewer");
    this->ListColorButton->SetDialogTitle("List symbol and text color");
    this->ListColorButton->SetLabelText("Unselected Color:");

    // selected colour
    this->ListSelectedColorButton = vtkKWChangeColorButton::New();
    this->ListSelectedColorButton->SetParent( fiducialDisplayFrame->GetFrame() );
    this->ListSelectedColorButton->Create();
    this->ListSelectedColorButton->SetLabelOutsideButton(1);
    this->ListSelectedColorButton->SetBorderWidth(0);
    this->ListSelectedColorButton->GetLabel()->SetAnchorToEast();
    this->ListSelectedColorButton->SetLabelWidth ( 14 );
    this->ListSelectedColorButton->SetBalloonHelpString("Change the colour of the selected fiducial list symbols and text in the MainViewer");
    this->ListSelectedColorButton->SetDialogTitle("List selected symbol and text color");
    this->ListSelectedColorButton->SetLabelText("Selected Color:");

    app->Script ( "pack %s %s %s %s %s %s %s %s %s %s %s -side top -anchor nw -padx 2 -pady 3",
                  this->VisibilityToggle->GetWidgetName(),
                  this->ListSymbolTypeMenu->GetWidgetName(),
                  this->ListSelectedColorButton->GetWidgetName(),
                  this->ListColorButton->GetWidgetName(),
                  this->ListSymbolScale->GetWidgetName(),
                  this->ListTextScale->GetWidgetName(),
                  this->ListOpacity->GetWidgetName(),
                  this->ListAmbient->GetWidgetName(),
                  this->ListDiffuse->GetWidgetName(),
                  this->ListSpecular->GetWidgetName(),
                  this->ListPower->GetWidgetName());
    
    //---
    //--- and clean up temporary stuff
    //---
    lA->Delete();
    fA->Delete();
    fB->Delete();
    f->Delete();
    f1->Delete();
    f2->Delete();
    f3->Delete();
    f4->Delete();
    f5->Delete();
    f0->Delete();
    measurementFrame->Delete();
    listFrame->Delete();
    fiducialDisplayFrame->Delete();
    fiducialFrame->Delete();
    fiducialListFrame->Delete();

}

//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::UpdateElement(int row, int col, char * str)
{
    vtkDebugMacro("UpdateElement: row = " << row << ", col = " << col << ", str = " << str << "\n");

    // make sure that the row and column exists in the table
    if ((row >= 0) && (row < this->MultiColumnList->GetWidget()->GetNumberOfRows()) &&
        (col >= 0) && (col < this->MultiColumnList->GetWidget()->GetNumberOfColumns()))
        {
            vtkMRMLFiducialListNode *activeFiducialListNode = (vtkMRMLFiducialListNode *)this->MRMLScene->GetNodeByID(this->GetFiducialListNodeID());
            // is there an active list?
            if (activeFiducialListNode == NULL)
            {
                // 
                vtkErrorMacro ("UpdateElement: ERROR: No Fiducial List, add one first!\n");
                return;
            }
            // save list for undo
            this->MRMLScene->SaveStateForUndo(activeFiducialListNode);
            // now update the requested value
            if (col == this->NameColumn)
            {
                activeFiducialListNode->SetNthFiducialLabelText(row, str);
            }
            else if (col == this->SelectedColumn)
            {
                // selected
                vtkDebugMacro("UpdateElement: setting node " <<  activeFiducialListNode->GetNthFiducialLabelText(row) << "'s selected flag to " << str << endl);
                activeFiducialListNode->SetNthFiducialSelected(row, (atoi(str) == 1));
                this->UpdateMeasurementLabel();
            }
            else if ( col == this->LockColumn)
              {
              // right now, cannot edit these individuals, so just return.
              return;
              }
            else if (col == this->VisibilityColumn)
              {
              // visible
              vtkDebugMacro("UpdateElement: setting node " <<  activeFiducialListNode->GetNthFiducialLabelText(row) << "'s visible flag to " << str << endl);
              activeFiducialListNode->SetNthFiducialVisibility(row, (atoi(str) == 1));
              // test: if any fiducials within a list are visible, then the list should be visible.
              for (  int n=0; n < activeFiducialListNode->GetNumberOfFiducials(); n++ )
                {
                if ( (activeFiducialListNode->GetNthFiducialVisibility(n) == 1) &&
                     (activeFiducialListNode->GetVisibility() != 1)  )
                  {
                  activeFiducialListNode->SetVisibility ( 1 );
                  break;
                  }
                }
              } 
            else if (col >= this->XColumn && col <= this->ZColumn)
            {
                // get the current xyz
                float * xyz = activeFiducialListNode->GetNthFiducialXYZ(row);
                // now set the new one
                float newCoordinate = atof(str);
                if ( xyz )
                  {
                  if (col == this->XColumn) { activeFiducialListNode->SetNthFiducialXYZ(row, newCoordinate, xyz[1], xyz[2]); }
                  if (col == this->YColumn) { activeFiducialListNode->SetNthFiducialXYZ(row, xyz[0], newCoordinate, xyz[2]); }
                  if (col == this->ZColumn) { activeFiducialListNode->SetNthFiducialXYZ(row, xyz[0], xyz[1], newCoordinate); }
                  this->UpdateMeasurementLabel();
                  }            
            }
            else if (col >= this->OrWColumn  && col <= this->OrZColumn)
            {
                float * wxyz = activeFiducialListNode->GetNthFiducialOrientation(row);
                float newCoordinate = atof(str);
                if (col == this->OrWColumn) { activeFiducialListNode->SetNthFiducialOrientation(row, newCoordinate, wxyz[1], wxyz[2], wxyz[3]); }
                if (col == this->OrXColumn) { activeFiducialListNode->SetNthFiducialOrientation(row, wxyz[0], newCoordinate, wxyz[2], wxyz[3]); }
                if (col == this->OrYColumn) { activeFiducialListNode->SetNthFiducialOrientation(row, wxyz[0], wxyz[1], newCoordinate, wxyz[3]); }
                if (col == this->OrZColumn) { activeFiducialListNode->SetNthFiducialOrientation(row, wxyz[0], wxyz[1], wxyz[2], newCoordinate); }
            }
            else
            {
                vtkErrorMacro ("UpdateElement: ERROR: invalid column number " << col << ", valid values are 0-" << this->NumberOfColumns << endl);
                return;
            }
        }
    else
    {
        vtkErrorMacro ("Invalid row " << row << " or column " << col <<  ", valid columns are 0-" << this->NumberOfColumns-1 << "\n");
    }
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::SetFiducialListNode (vtkMRMLFiducialListNode *fiducialListNode)
{
    if (fiducialListNode == NULL)
    {
        vtkErrorMacro ("ERROR: SetFiducialListNode - list node is null.\n");
        return;
    }
    // save the ID
    vtkDebugMacro("setting the fid list node id to " << fiducialListNode->GetID());
    this->SetFiducialListNodeID(fiducialListNode->GetID());
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::SetFiducialListNodeID (char * id)
{
  if (this->GetFiducialListNodeID() != NULL &&
      id != NULL &&
      strcmp(id,this->GetFiducialListNodeID()) == 0)
    {
    vtkDebugMacro("SetFiducialListNodeID: no change in id, not doing anything for now: " << id << endl);
    return;
    }

  // get the old node
  vtkMRMLFiducialListNode *fidlist = vtkMRMLFiducialListNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->GetFiducialListNodeID()));
       
    // set the id properly - see the vtkSetStringMacro
    this->FiducialListNodeID = id;

    if (id == NULL)
      {
      vtkDebugMacro("SetFiducialListNodeID: NULL input id, removed observers and returning.\n");
      return;
      }
    
    // get the new node
    fidlist = vtkMRMLFiducialListNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->GetFiducialListNodeID()));
    // set up observers on the new node
    if (fidlist != NULL)
      {
      if (this->GetDebug())
        {
        fidlist->DebugOn();
        }
      vtkIntArray *events = vtkIntArray::New();
      events->InsertNextValue(vtkCommand::ModifiedEvent);
      events->InsertNextValue(vtkMRMLFiducialListNode::DisplayModifiedEvent);
      events->InsertNextValue(vtkMRMLFiducialListNode::FiducialModifiedEvent);
      events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
      events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
      vtkSetAndObserveMRMLNodeEventsMacro(this->FiducialListNode, fidlist, events);
      events->Delete();

      // set up the GUI
      this->SetGUIFromList(this->FiducialListNode);
      }
    else
      {
      vtkDebugMacro ("ERROR: unable to get the mrml fiducial node to observe!\n");
      }

    // update the selected fid list id
    if (this->ApplicationLogic != NULL &&
        this->ApplicationLogic->GetSelectionNode() != NULL &&
        this->FiducialListNodeID != NULL)
      {
      vtkDebugMacro("Fid GUI: setting the active fid list id to " << this->FiducialListNodeID);
      this->ApplicationLogic->GetSelectionNode()->SetActiveFiducialListID( this->FiducialListNodeID );
      }
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::UpdateMeasurementLabel()
{
  if (!this->MeasurementLabel)
    {
    return;
    }
  // get the fiducial list
  vtkMRMLFiducialListNode * activeFiducialListNode = (vtkMRMLFiducialListNode *)this->MRMLScene->GetNodeByID(this->GetFiducialListNodeID());
  if (activeFiducialListNode == NULL)
    {
    return;
    }
  
  int numPoints = activeFiducialListNode->GetNumberOfFiducials();
  int numSelected = 0;
  int selectedIndices[2];
  std::string newLabel = "Distance: ";
  for (int n = 0; n < numPoints && numSelected < 2; n++)
    {
    if (activeFiducialListNode->GetNthFiducialSelected(n))
      {
      selectedIndices[numSelected] = n;
      numSelected++;
      
      if (numSelected == 2)
        {
        std::stringstream ss;
        ss << newLabel;
        ss << activeFiducialListNode->GetNthFiducialLabelText(selectedIndices[0]);
        ss << " to ";
        ss << activeFiducialListNode->GetNthFiducialLabelText(selectedIndices[1]);
        ss << " = ";
        float *xyz1 = activeFiducialListNode->GetNthFiducialXYZ(selectedIndices[0]);
        float *xyz2 = activeFiducialListNode->GetNthFiducialXYZ(selectedIndices[1]);
        double dist = 0.0;
        if (xyz1 != NULL && xyz2 != NULL)
          {
          dist = sqrt(pow(double(xyz2[0] - xyz1[0]),2) +
                      pow(double(xyz2[1] - xyz1[1]),2) +
                      pow(double(xyz2[2] - xyz1[2]),2));
          }
        ss << dist;
        ss << " mm";
        newLabel = ss.str();
        }
      }
    }
  this->MeasurementLabel->SetText(newLabel.c_str());
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::JumpSlicesCallback(int row, int col, int x, int y)

{
  double r = 0.0l, a = 0.0l, s = 0.0l;
  if (this->MultiColumnList == NULL)
    {
    return;
    }
  if ((row >= 0) && (row < this->MultiColumnList->GetWidget()->GetNumberOfRows()) && (this->MRMLScene != NULL))
    {
    // get the position of the fid in that row
    r = this->MultiColumnList->GetWidget()->GetCellTextAsDouble(row, this->XColumn);
    a = this->MultiColumnList->GetWidget()->GetCellTextAsDouble(row, this->YColumn);
    s = this->MultiColumnList->GetWidget()->GetCellTextAsDouble(row, this->ZColumn);
    vtkDebugMacro("JumpSlicesCallback: row = " << row << ", jumping to " << r << ", " << a << ", " << s);
    // JumpAllSlices only jumps the other slices, so call JumpSlice first to
    // get all of them
    //this->Script("[[$::slicer3::SlicesGUI GetFirstSliceGUI] GetSliceNode] JumpSlice %f %f %f", r, a, s);
    //this->Script("[[$::slicer3::SlicesGUI GetFirstSliceGUI] GetSliceNode] JumpAllSlices %f %f %f", r, a, s);

    vtkMRMLSliceNode *node= NULL;
    int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLSliceNode");
    for (int n=0; n<nnodes; n++)
      {
      node = vtkMRMLSliceNode::SafeDownCast (
          this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLSliceNode"));
      if ( node != NULL )
        {
        node->JumpSlice(r, a, s);
        }
      }
    }
  else
    {
    vtkWarningMacro("JumpSlicesCallback: row " << row << " out of range of 0 to " << this->MultiColumnList->GetWidget()->GetNumberOfRows() - 1);
    }
}
