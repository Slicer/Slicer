#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkKWWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerFoundationIcons.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkMRMLViewNode.h"

#include "vtkKWMessage.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"

#include "vtkKWFrameWithLabel.h"

#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"

#include "vtkKWSimpleEntryDialog.h"

#include "vtkSlicerFiducialsGUI.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerFiducialsGUI );
vtkCxxRevisionMacro ( vtkSlicerFiducialsGUI, "$Revision$");


//---------------------------------------------------------------------------
vtkSlicerFiducialsGUI::vtkSlicerFiducialsGUI ( )
{
    this->Logic = NULL;
    this->FiducialListSelectorWidget = NULL;
    this->FiducialListNodeID = NULL;

    this->MeasurementLabel = NULL;
    this->ListMeasurementLabel = NULL;
    this->RenumberButton = NULL;
    this->RenumberDialogue = NULL;
    this->RenameButton = NULL;
    this->RenameDialogue = NULL;
    this->ListNumberingSchemeMenu = NULL;
    
    this->AddFiducialButton = NULL;
    this->RemoveFiducialButton = NULL;
    this->RemoveFiducialsInListButton = NULL;
    this->RemoveSelectedListButton = NULL;
    this->RemoveAllFiducialsButton = NULL;
    this->LockAllFiducialsButton = NULL;
    this->UnlockAllFiducialsButton = NULL;
    
    this->SelectAllFiducialsButton = NULL;
    this->DeselectAllFiducialsButton = NULL;
    this->SelectAllFiducialsInListButton = NULL;
    this->DeselectAllFiducialsInListButton = NULL;
    
    this->HideListToggle = NULL;

    this->MoveSelectedFiducialUpButton = NULL;
    this->MoveSelectedFiducialDownButton = NULL;
    this->Center3DViewOnSelectedFiducialButton = NULL;

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
    this->FiducialsListLockToggle = NULL;
    this->AllVisibilityMenuButton = NULL;
    this->ListVisibilityMenuButton = NULL;
    this->HideOrExposeAllFiducialListsMenuButton = NULL;

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
  this->RemoveMRMLObservers();

  this->SetModuleLogic (static_cast<vtkSlicerFiducialsLogic*>(0));

  if ( this->AllLockMenuButton)
    {
    this->AllLockMenuButton->SetParent ( NULL );
    this->AllLockMenuButton->Delete();
    this->AllLockMenuButton = NULL;
    }
  if ( this->FiducialsListLockToggle )
    {
    this->FiducialsListLockToggle->SetParent ( NULL );
    this->FiducialsListLockToggle->Delete();
    this->FiducialsListLockToggle = NULL;
    }
  if ( this->AllVisibilityMenuButton )
    {
    this->AllVisibilityMenuButton->SetParent ( NULL );
    this->AllVisibilityMenuButton->Delete();
    this->AllVisibilityMenuButton = NULL;
    }
  if (  this->HideOrExposeAllFiducialListsMenuButton )
    {
    this->HideOrExposeAllFiducialListsMenuButton->SetParent ( NULL );
    this->HideOrExposeAllFiducialListsMenuButton->Delete();
    this->HideOrExposeAllFiducialListsMenuButton = NULL;    
    }
  if ( this->ListVisibilityMenuButton )
    {
    this->ListVisibilityMenuButton->SetParent ( NULL );
    this->ListVisibilityMenuButton->Delete();
    this->ListVisibilityMenuButton = NULL;
    }
  if (this->FiducialListSelectorWidget)
    {
    this->FiducialListSelectorWidget->SetMRMLScene(NULL);
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
  if (this->ListMeasurementLabel)
    {
    this->ListMeasurementLabel->SetParent(NULL);
    this->ListMeasurementLabel->Delete();
    this->ListMeasurementLabel = NULL;
    }
  if (this->RenumberButton)
    {
    this->RenumberButton->SetParent(NULL);
    this->RenumberButton->Delete();
    this->RenumberButton = NULL;
    }
  if (this->RenumberDialogue)
    {
    this->RenumberDialogue->SetParent(NULL);
    this->RenumberDialogue->Delete();
    this->RenumberDialogue = NULL;
    }
  if (this->RenameButton)
    {
    this->RenameButton->SetParent(NULL);
    this->RenameButton->Delete();
    this->RenameButton = NULL;
    }
  if (this->RenameDialogue)
    {
    this->RenameDialogue->SetParent(NULL);
    this->RenameDialogue->Delete();
    this->RenameDialogue = NULL;
    }
  if (this->ListNumberingSchemeMenu)
    {
    this->ListNumberingSchemeMenu->SetParent(NULL);
    this->ListNumberingSchemeMenu->Delete();
    this->ListNumberingSchemeMenu = NULL;
    }
  if (this->AddFiducialButton )
    {
    this->AddFiducialButton->SetParent (NULL );
    this->AddFiducialButton->Delete ( );
    this->AddFiducialButton = NULL;
    }
  if ( this->RemoveSelectedListButton )
    {
    this->RemoveSelectedListButton->SetParent ( NULL );
    this->RemoveSelectedListButton->Delete();
    this->RemoveSelectedListButton = NULL;
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
  if (this->HideListToggle)
    {
    this->HideListToggle->SetParent(NULL);
    this->HideListToggle->Delete();
    this->HideListToggle = NULL;
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
  if (this->Center3DViewOnSelectedFiducialButton)
    {
    this->Center3DViewOnSelectedFiducialButton->SetParent(NULL);
    this->Center3DViewOnSelectedFiducialButton->Delete();
    this->Center3DViewOnSelectedFiducialButton = NULL;
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
  
  this->SetFiducialListNodeID(NULL);
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
  // check to make sure that the widgets are still defined
    if (this->FiducialListSelectorWidget == NULL)
      {
      return;
      }
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
    this->RemoveSelectedListButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->FiducialsListLockToggle->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand ); 

    this->MoveSelectedFiducialUpButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->MoveSelectedFiducialDownButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->Center3DViewOnSelectedFiducialButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->HideListToggle->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
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
    this->AllVisibilityMenuButton->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ListVisibilityMenuButton->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );    

    this->RenumberButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->RenameButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ListNumberingSchemeMenu->GetWidget()->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand);

    this->RemoveObservers (vtkSlicerFiducialsGUI::FiducialListIDModifiedEvent, (vtkCommand *)this->GUICallbackCommand);    
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::RemoveMRMLObservers ( )
{
  if (this->MRMLScene == NULL)
    {
    return;
    }
  vtkDebugMacro("RemoveMRMLObservers...");
  // remove observers on the fiducial list nodes
  int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLFiducialListNode");
  for (int n=0; n<nnodes; n++)
    {
    vtkMRMLFiducialListNode *fiducialListNode = vtkMRMLFiducialListNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLFiducialListNode"));
    if (fiducialListNode->HasObserver(vtkMRMLTransformableNode::TransformModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand))
      {
      fiducialListNode->RemoveObservers(vtkMRMLTransformableNode::TransformModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
      }
    if (fiducialListNode->HasObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand))
      {
      fiducialListNode->RemoveObservers(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
      }
    //vtkSetAndObserveMRMLNodeEventsMacro(fiducialListNode, NULL, NULL);
    fiducialListNode = NULL;
    }

  this->MRMLScene->RemoveObservers(vtkMRMLScene::NodeRemovedEvent, (vtkCommand *)this->MRMLCallbackCommand);
  this->MRMLScene->RemoveObservers(vtkMRMLScene::NodeAddedEvent, (vtkCommand *)this->MRMLCallbackCommand);
  this->MRMLScene->RemoveObservers(vtkMRMLScene::SceneClosedEvent, (vtkCommand *)this->MRMLCallbackCommand);
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
    this->RemoveSelectedListButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->FiducialsListLockToggle->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

    this->MoveSelectedFiducialUpButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->MoveSelectedFiducialDownButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->Center3DViewOnSelectedFiducialButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->HideListToggle->AddObserver (vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
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
    this->AllVisibilityMenuButton->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ListVisibilityMenuButton->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );    
    this->HideOrExposeAllFiducialListsMenuButton->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );

    this->RenumberButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->RenameButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ListNumberingSchemeMenu->GetWidget()->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand);

    this->AddObserver(vtkSlicerFiducialsGUI::FiducialListIDModifiedEvent, (vtkCommand *)this->GUICallbackCommand);

    this->AddMRMLObservers();
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::AddMRMLObservers ( )
{
 
  if (this->MRMLScene == NULL)
    {
    return;
    }
  vtkDebugMacro("AddMRMLObservers...");

  // add observers on the fiducial list nodes
  int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLFiducialListNode");
  for (int n=0; n<nnodes; n++)
    {
    vtkMRMLFiducialListNode *fiducialListNode = vtkMRMLFiducialListNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLFiducialListNode"));
    if (!fiducialListNode->HasObserver(vtkMRMLTransformableNode::TransformModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand))
      {
      fiducialListNode->AddObserver(vtkMRMLTransformableNode::TransformModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
      }
    if (!fiducialListNode->HasObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand))
      {
      fiducialListNode->AddObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
      }
    fiducialListNode = NULL;
    }
  
  // observe the scene for node deleted events
  if (this->MRMLScene->HasObserver(vtkMRMLScene::NodeRemovedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
    {
    this->MRMLScene->AddObserver(vtkMRMLScene::NodeRemovedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    }
  if (this->MRMLScene->HasObserver(vtkMRMLScene::NodeAddedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
    {
    this->MRMLScene->AddObserver(vtkMRMLScene::NodeAddedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    }
  if (this->MRMLScene->HasObserver(vtkMRMLScene::SceneClosedEvent, (vtkCommand *)this->MRMLCallbackCommand) != 1)
    {
    this->MRMLScene->AddObserver(vtkMRMLScene::SceneClosedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    }
}

//--------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::ProcessGUIEvents ( vtkObject *caller,
                                            unsigned long event, void * vtkNotUsed(callData) )
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
        this->SetFiducialListNodeID(fidList->GetID());
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
        //this->AddSeedWidget(newList);
        // AddFiducialList already calls delete
        //newList->Delete();
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
  
  //---
  //--- Pushbuttons
  //---
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
      /*
        else
        {
        vtkSlicerSeedWidgetClass *seedWidget = this->GetSeedWidget(activeFiducialListNode->GetID());
        if (seedWidget)
          {
          float *p = activeFiducialListNode->GetNthFiducialXYZ(modelIndex);
          seedWidget->AddSeed(p);
          }
        }
      */
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
        //vtkSlicerSeedWidgetClass *seedWidget = this->GetSeedWidget(activeFiducialListNode->GetID());
        //if (seedWidget)
        //  {
        //  seedWidget->RemoveSeed(row[0]);
        //  }
        }
      else
        {
        vtkErrorMacro ("Selected rows (" << numRows << ") not 1, just pick one to delete for now\n");
        return;
        }
      }
    else if (  button == this->RemoveSelectedListButton )
      {
      //--- ask user to confirm.
      std::string message = "Are you sure you want to delete all Fiducial Points in the selected list and the selected list?";
      vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
      dialog->SetParent (  this->RemoveSelectedListButton);
      dialog->SetStyleToOkCancel();
      dialog->SetText(message.c_str());
      dialog->Create ( );
      dialog->SetMasterWindow( this->RemoveSelectedListButton );
      dialog->ModalOn();
      int doit = dialog->Invoke();
      dialog->Delete();
      if ( doit )
        {
        // save state for undo
        this->MRMLScene->SaveStateForUndo();
        activeFiducialListNode->RemoveAllFiducials();
        this->GetMRMLScene()->RemoveNode(activeFiducialListNode);
        this->SetFiducialListNodeID(NULL);
        // is there now another list selected?
        if (this->FiducialListSelectorWidget->GetSelected() != NULL)
          {
          this->SetFiducialListNodeID(vtkMRMLFiducialListNode::SafeDownCast(this->FiducialListSelectorWidget->GetSelected())->GetID());
          }
        }
      return;
      }
    else if (button == this->RemoveFiducialsInListButton)
      {
      vtkDebugMacro("vtkSlicerFiducialsGUI: ProcessGUIEvent: Remove Fiducials In List Button event: " << event << ".\n");
      //--- ask user to confirm.
      std::string message = "Are you sure you want to delete all fiducial points in this list?";        
      vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
      dialog->SetParent (  this->RemoveFiducialsInListButton);
      dialog->SetStyleToOkCancel();
      dialog->SetText(message.c_str());
      dialog->Create ( );
      dialog->SetMasterWindow( this->RemoveFiducialsInListButton );
      dialog->ModalOn();
      int doit = dialog->Invoke();
      dialog->Delete();
      if ( doit )
        {
        // save state for undo
        this->MRMLScene->SaveStateForUndo(activeFiducialListNode);
        activeFiducialListNode->RemoveAllFiducials();
        /*
        vtkSlicerSeedWidgetClass *seedWidget = this->GetSeedWidget(activeFiducialListNode->GetID());
        if (seedWidget)
          {
          seedWidget->RemoveAllSeeds();
          }
        */
        }
      return;
      }
    else if (button == this->RemoveAllFiducialsButton)
      {
      numnodes = this->MRMLScene->GetNumberOfNodesByClass ( "vtkMRMLFiducialListNode" );
      if ( numnodes > 0 )
        {
        std::string message;
        if ( numnodes > 1 )
          {
          message = "Are you sure you want to delete all Fiducial Points in all Fiducial Lists, and all Fiducial Lists?";
          }
        else
          {
          message = "Are you sure you want to delete the Fiducial List and any Fiducial Points it contains?";        
          }
        //--- ask user to confirm.
        vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
        dialog->SetParent (  this->RemoveAllFiducialsButton );
        dialog->SetStyleToOkCancel();
        dialog->SetText(message.c_str());
        dialog->Create ( );
        dialog->SetMasterWindow( this->RemoveAllFiducialsButton );
        dialog->ModalOn();
        int doit = dialog->Invoke();
        dialog->Delete();

        if ( doit )
          {
          vtkDebugMacro("vtkSlicerFiducialsGUI: ProcessGUIEvent: Remove Fiducials Button event: " << event << ".\n");
          // save state for undo
          this->MRMLScene->SaveStateForUndo();
          for ( nn=0; nn<numnodes; nn++ )
            {
            flNode = vtkMRMLFiducialListNode::SafeDownCast (this->MRMLScene->GetNthNodeByClass ( nn, "vtkMRMLFiducialListNode" ));
            if ( flNode != NULL )
              {
              flNode->RemoveAllFiducials();
              }
            }
          //--- now delete all nodes... 
          for ( nn=0; nn<numnodes; nn++ )
            {
            flNode = vtkMRMLFiducialListNode::SafeDownCast (this->MRMLScene->GetNthNodeByClass ( 0, "vtkMRMLFiducialListNode" ));
            if ( flNode )
              {
              this->GetMRMLScene()->RemoveNode(flNode);
              
              this->SetFiducialListNodeID(NULL);
              }
            }
          // -- and all widgets
          //std::map<std::string, vtkSlicerSeedWidgetClass *>::iterator iter;
          //for (iter = this->SeedWidgets.begin();
          //     iter != this->SeedWidgets.end();
          //     iter++)
          //  {
          //  iter->second->Delete();
          //  }
          //this->SeedWidgets.clear();
          }
        }
      return;
      }
    else if ( button == this->FiducialsListLockToggle )
      {
      // set lock on all fiducials in selected list
      if ( activeFiducialListNode->GetLocked() > 0 )
        {
        this->ModifyListLock(0);
        }
      else
        {
        this->ModifyListLock(1);
        }
      }
    else if ( button == this->HideListToggle )
      {
      if ( activeFiducialListNode->GetVisibility() > 0 )
        {
        this->ModifyListExposure ( 0 );
        }
      else
        {
        this->ModifyListExposure ( 1 );
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
          //this->Update3DWidgetSelected(flNode);
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
          //this->Update3DWidgetSelected(flNode);
          }
        }
      }
    else if (button == this->SelectAllFiducialsInListButton)
      {
      vtkDebugMacro("vtkSlicerFiducialsGUI: ProcessGUIEvent: Select Fiducials In List Button event: " << event << ".\n");
      // save state for undo
      this->MRMLScene->SaveStateForUndo(activeFiducialListNode);
      activeFiducialListNode->SetAllFiducialsSelected(1);
      //this->Update3DWidgetSelected(activeFiducialListNode);
      }
    else if (button == this->DeselectAllFiducialsInListButton)
      {
      vtkDebugMacro("vtkSlicerFiducialsGUI: ProcessGUIEvent: Deselect all Fiducials In List Button event: " << event << ".\n");
      // save state for undo
      this->MRMLScene->SaveStateForUndo(activeFiducialListNode);
      activeFiducialListNode->SetAllFiducialsSelected(0);
      //this->Update3DWidgetSelected(activeFiducialListNode);
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
    else if (button == this->Center3DViewOnSelectedFiducialButton)
      {
      vtkDebugMacro("vtkSlicerFiducialsGUI: ProcessGUIEvent: Center 3d view on Selected Fiducial Down Button event: " << event << ".\n");
      // get the index of the selected fiducial
      int selectedIndex =  this->MultiColumnList->GetWidget()->GetIndexOfFirstSelectedRow();
      if (selectedIndex == -1)
        {
        vtkErrorMacro("Select a fiducial first...");
        }
      else
        {
        // no undo
        // get the r,a,s
        float *ras = activeFiducialListNode->GetNthFiducialXYZ(selectedIndex);
        // recenter the 3d view
        if (ras != NULL &&
            this->GetApplicationGUI() &&
            this->GetApplicationGUI()->GetViewControlGUI())
          {
          this->GetApplicationGUI()->GetViewControlGUI()->MainViewSetFocalPoint(ras[0], ras[1], ras[2]);
          }
        }
      }
    else if (button == this->RenumberButton)
      {
      if (this->RenumberDialogue == NULL)
        {
        this->RenumberDialogue = vtkKWSimpleEntryDialog::New();
        this->RenumberDialogue->SetParent(this->UIPanel->GetPageWidget ( "Fiducials" ));
        this->RenumberDialogue->SetTitle("Renumber Fiducials");
        this->RenumberDialogue->SetStyleToOkCancel();
        this->RenumberDialogue->GetEntry()->SetLabelText("Please enter a number suffix to start renumbering fiducials in this list:");
        this->RenumberDialogue->GetEntry()->GetWidget()->SetValue("0");
        this->RenumberDialogue->Create();
        }
      if (this->RenumberDialogue)
        {
        int result = this->RenumberDialogue->Invoke();
        if (result && 
            ((this->RenumberDialogue->GetEntry()->GetWidget()->GetValue() != NULL) || (!strcmp(this->RenumberDialogue->GetEntry()->GetWidget()->GetValue(),""))))
          {
          int startFrom = atoi(this->RenumberDialogue->GetEntry()->GetWidget()->GetValue());
          activeFiducialListNode->RenumberFiducials(startFrom);
          // this->Update3DWidgetText(activeFiducialListNode);
          }
        }
      }
    else if (button == this->RenameButton)
      {
      if (this->RenameDialogue == NULL)
        {
        this->RenameDialogue = vtkKWSimpleEntryDialog::New();
        this->RenameDialogue->SetParent(this->UIPanel->GetPageWidget ( "Fiducials" ));
        this->RenameDialogue->SetTitle("Rename Fiducials");
        this->RenameDialogue->SetStyleToOkCancel();
        this->RenameDialogue->GetEntry()->SetLabelText("Please enter the new name prefix for fiducials in this list:");
        this->RenameDialogue->GetEntry()->GetWidget()->SetValue("");
        this->RenameDialogue->Create();
        }
      if (this->RenameDialogue)
        {
        int result = this->RenameDialogue->Invoke();
        if (result && 
            ((this->RenameDialogue->GetEntry()->GetWidget()->GetValue() != NULL) || (!strcmp(this->RenameDialogue->GetEntry()->GetWidget()->GetValue(),""))))
          {
          const char *newName = this->RenameDialogue->GetEntry()->GetWidget()->GetValue();
          activeFiducialListNode->RenameFiducials(newName);
          // this->Update3DWidgetText(activeFiducialListNode);
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
    // this->Update3DWidgetColour(activeFiducialListNode);
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
      // this->Update3DWidgetScale(activeFiducialListNode);
      }
    else if (scale == this->ListTextScale)
      {
      activeFiducialListNode->SetTextScale(this->ListTextScale->GetValue());
      // this->Update3DWidgetScale(activeFiducialListNode);
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

  //---
  //--- MenuButton menus
  //---
  vtkKWMenu *menu = vtkKWMenu::SafeDownCast ( caller );
  if ( menu != NULL && event == vtkKWMenu::MenuItemInvokedEvent )
    {
    if ( menu == this->AllLockMenuButton->GetMenu() )
      {
      // set lock on all fiducials in all lists.
      if ( menu->GetItemSelectedState ( "Lock All Lists" ) == 1 )
        {
        this->ModifyAllLock(1);
        }
      if ( menu->GetItemSelectedState ( "Unlock All Lists" ) == 1 )
        {
        this->ModifyAllLock(0);
        }
      }   
    
    // list numberering scheme type
    else if (menu == this->ListNumberingSchemeMenu->GetWidget()->GetMenu())
      {
      // save state for undo
      this->MRMLScene->SaveStateForUndo(activeFiducialListNode);
      activeFiducialListNode->SetNumberingSchemeFromString(this->ListNumberingSchemeMenu->GetWidget()->GetValue());
      }
    else if ( menu == this->AllVisibilityMenuButton->GetMenu() )
      {
      // set visibility on all fiducials in all lists.
      if ( menu->GetItemSelectedState ( "All Fiducials Visible" ) == 1 )
        {
        this->ModifyAllFiducialVisibility (1 );
        }
      if ( menu->GetItemSelectedState ( "All Fiducials Invisible" ) == 1 )
        {
        this->ModifyAllFiducialVisibility (0 );
        }
      }
    else if ( menu == this->HideOrExposeAllFiducialListsMenuButton->GetMenu() )
      {
      // set visibility on all fiducials in all lists.
      if ( menu->GetItemSelectedState ( "Expose All Lists" ) == 1 )
        {
        this->ModifyAllListExposure (1 );
        }
      if ( menu->GetItemSelectedState ( "Hide All Lists" ) == 1 )
        {
        this->ModifyAllListExposure (0 );
        }
      }
    else if ( menu == this->ListVisibilityMenuButton->GetMenu() )
      {
      // set visibility on all fiducials in selected list.
      if ( menu->GetItemSelectedState ( "List's Fiducials Visible" ) == 1 )
        {
        this->ModifyFiducialsInListVisibility( 1 );
        }
      if ( menu->GetItemSelectedState ( "List's Fiducials Invisible" ) == 1 )
        {
        this->ModifyFiducialsInListVisibility( 0 );
        }
      }    
    }

  // list symbol type
  if (this->ListSymbolTypeMenu->GetWidget()->GetMenu() ==  vtkKWMenu::SafeDownCast(caller) &&
      event == vtkKWMenu::MenuItemInvokedEvent)
    {
    // save state for undo
    this->MRMLScene->SaveStateForUndo(activeFiducialListNode);
    vtkDebugMacro("Changing list glyph type to " << this->ListSymbolTypeMenu->GetWidget()->GetValue());
    activeFiducialListNode->SetGlyphTypeFromString(this->ListSymbolTypeMenu->GetWidget()->GetValue());
    // this->Update3DWidget(activeFiducialListNode);
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
  // update GUI
//  this->ModifySelectedListLockGUI();
//  this->ModifyIndividualFiducialsLockGUI();

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

  // this->Update3DWidgetLock(activeFiducialListNode);
  
  // updateGUI
//  this->ModifySelectedListLockGUI();
//  this->ModifyIndividualFiducialsLockGUI();

}

//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::ModifySelectedListLockGUI (  )
{
  if ( this->MRMLScene == NULL )
    {
    vtkErrorMacro ( "ModifySelectedListLockGUI: got NULL MRMLScene." );
    return;
    }
  vtkMRMLFiducialListNode *activeFiducialListNode = (vtkMRMLFiducialListNode *)this->MRMLScene->GetNodeByID(this->GetFiducialListNodeID());
  if ( activeFiducialListNode == NULL )
    {
    vtkErrorMacro ( "ModifySelectedListLockGUI: got NULL activeFiducialListNode." );
    return;
    }

  if ( this->GetApplicationGUI() == NULL )
    {
    vtkErrorMacro ( "ModifySelectedListLockGUI: Got NULL ApplicationGUI" );
    return;
    }
  if ( this->GetApplicationGUI()->GetSlicerFoundationIcons() == NULL )
    {
    vtkErrorMacro ( "ModifySelectedListLockGUI: Got NULL SlicerFoundationIcons." );
    return;
    }
  if (activeFiducialListNode->GetLocked() > 0)
    {
    this->GetFiducialsListLockToggle()->SetImageToIcon(this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerFiducialsLockListIcon() );
    this->FiducialsListLockToggle->SetBalloonHelpString ( "Unlock the selected fiducial list." );
    }
  else
    {
    this->GetFiducialsListLockToggle()->SetImageToIcon(this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerFiducialsUnlockListIcon() );
    this->FiducialsListLockToggle->SetBalloonHelpString ( "Lock the selected fiducial list." );
    }

}


//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::ModifyIndividualFiducialsLockGUI (  )
{
  // and update the selected list setting too.
  if ( !this->MultiColumnList )
    {
    vtkErrorMacro ( "ModifyIndividualFiducialsLockGUI: got NULL MultiColumnList." );
    return;
    }
  // and update the entire column of the multicolumn list to match the active list node.
  if ( this->MultiColumnList->GetWidget() == NULL )
    {
    vtkErrorMacro ( "ModifyIndividualFiducialsLockGUI: got NULL MultiColumnList." );
    return;
    }
  if ( this->MRMLScene == NULL )
    {
    vtkErrorMacro ( "ModifyIndividialFiducialsLockGUI: got NULL MRMLScene." );
    return;
    }

  vtkMRMLFiducialListNode *activeFiducialListNode = (vtkMRMLFiducialListNode *)this->MRMLScene->GetNodeByID(this->GetFiducialListNodeID());
  if ( activeFiducialListNode == NULL )
    {
    vtkErrorMacro ( "ModifyIndividualFiducialsLockGUI: got NULL activeFiducialListNode." );
    return;
    }

  int numPoints = this->MultiColumnList->GetWidget()->GetNumberOfRows();
  int cellLock;
  int nodeLock = activeFiducialListNode->GetLocked();
  for ( int i = 0; i < numPoints; i++ )
    {
    cellLock = this->MultiColumnList->GetWidget()->GetCellTextAsInt(i, this->LockColumn );
    if ( cellLock != nodeLock )
      {
      this->MultiColumnList->GetWidget()->SetCellEditable( i, this->LockColumn, 1 );
      this->MultiColumnList->GetWidget()->SetCellEnabledAttribute (i,  this->LockColumn, 1 );
      this->MultiColumnList->GetWidget()->ColumnEditableOn( this->LockColumn );

      if ( cellLock > 0 )
        {
        this->MultiColumnList->GetWidget()->SetCellTextAsInt(i, this->LockColumn, 0 );
        this->MultiColumnList->GetWidget()->SetCellImageToIcon (i, this->LockColumn, this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerUnlockIcon() );
        }
      else
        {
        this->MultiColumnList->GetWidget()->SetCellTextAsInt(i, this->LockColumn, 1);
        this->MultiColumnList->GetWidget()->SetCellImageToIcon (i, this->LockColumn, this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerLockIcon() );
        }
      this->MultiColumnList->GetWidget()->SetCellEditable( i, this->LockColumn, 0 );
      this->MultiColumnList->GetWidget()->SetCellEnabledAttribute (i,  this->LockColumn, 0 );
      this->MultiColumnList->GetWidget()->ColumnEditableOff( this->LockColumn );
      }
    }

}


//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::ModifyAllFiducialVisibility( int visibilityState)
{
  if ( this->MRMLScene == NULL )
    {
    vtkErrorMacro ( "ModifyAllFiducialVisibility: got NULL MRMLScene." );
    return;
    }
  if ( visibilityState != 0 && visibilityState != 1 )
    {
    vtkErrorMacro ( "ModifyAllFiducialVisibility: got bad value for lock state; should be 0 or 1" );
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
      // this->Update3DWidgetVisibility(flNode);
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

}



//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::ModifyFiducialsInListVisibility ( int visibilityState)
{
  if ( this->MRMLScene == NULL )
    {
    vtkErrorMacro ( "ModifyFiducialsInListVisibility: got NULL MRMLScene." );
    return;
    }
  if ( visibilityState != 0 && visibilityState != 1 )
    {
    vtkErrorMacro ( "ModifyFiducialsInListVisibility: got bad value for lock state; should be 0 or 1" );
    return;
    }

  vtkMRMLFiducialListNode *activeFiducialListNode = (vtkMRMLFiducialListNode *)this->MRMLScene->GetNodeByID(this->GetFiducialListNodeID());
  if ( activeFiducialListNode == NULL )
    {
    vtkErrorMacro ( "ModifyFiducialsInListVisibility: got NULL activeFiducialListNode." );
    return;
    }
  
  // save state of node for undo
  this->MRMLScene->SaveStateForUndo(activeFiducialListNode);

  // change the visibility on the fiducials in list
  activeFiducialListNode->SetAllFiducialsVisibility ( visibilityState );

  // this->Update3DWidget(activeFiducialListNode);
//  activeFiducialListNode->SetVisibility ( visibilityState );


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
                                                           
  // update GUI
//  this->ModifyIndividualFiducialsVisibilityGUI ( );
  
}




//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::ModifyIndividualFiducialsVisibilityGUI ( )
{
  if ( this->MRMLScene == NULL )
    {
    vtkErrorMacro ( "ModifyIndividualFiducialsVisibility: got NULL MRMLScene." );
    return;
    }

  vtkMRMLFiducialListNode *activeFiducialListNode = (vtkMRMLFiducialListNode *)this->MRMLScene->GetNodeByID(this->GetFiducialListNodeID());
  if ( activeFiducialListNode == NULL )
    {
    vtkErrorMacro ( "ModifyIndividualFiducialsVisibilityGUI: got NULL activeFiducialListNode." );
    return;
    }

  //--- set GUI to match the node.
  int numPoints = this->MultiColumnList->GetWidget()->GetNumberOfRows();
  for ( int row = 0; row < numPoints; row++ )
    {
    int cellVisibility = this->MultiColumnList->GetWidget()->GetCellTextAsInt ( row, this->VisibilityColumn);
    int fidVisibility = activeFiducialListNode->GetNthFiducialVisibility(row);
    if ( cellVisibility != fidVisibility )
      {
      if ( fidVisibility > 0 )
        {
        this->MultiColumnList->GetWidget()->SetCellTextAsInt ( row, this->VisibilityColumn, 1 );
        this->MultiColumnList->GetWidget()->SetCellImageToIcon (row, this->VisibilityColumn, this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerVisibleIcon() );
        }
      else
        {
        this->MultiColumnList->GetWidget()->SetCellTextAsInt ( row, this->VisibilityColumn, 0 );
        this->MultiColumnList->GetWidget()->SetCellImageToIcon (row, this->VisibilityColumn, this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerInvisibleIcon() );
        }
      }
    }
}

  
  
//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::ModifyListExposure( int exposureState )
{
  if ( this->MRMLScene == NULL )
    {
    vtkErrorMacro ( "ModifyListExposure: got NULL MRMLScene." );
    return;
    }
  if ( exposureState != 0 && exposureState != 1 )
    {
    vtkErrorMacro ( "ModifyListExposure: got bad value for lock state; should be 0 or 1" );
    return;
    }

  vtkMRMLFiducialListNode *activeFiducialListNode = (vtkMRMLFiducialListNode *)this->MRMLScene->GetNodeByID(this->GetFiducialListNodeID());
  if ( activeFiducialListNode == NULL )
    {
    vtkErrorMacro ( "ModifyListExposure: got NULL activeFiducialListNode." );
    return;
    }

  // save state of node for undo
  this->MRMLScene->SaveStateForUndo(activeFiducialListNode);
  activeFiducialListNode->SetVisibility ( exposureState );
  // this->Update3DWidgetVisibility(activeFiducialListNode);
//  this->ModifySelectedListExposureGUI();
}




//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::ModifyAllListExposure( int exposureState)
{
  if ( this->MRMLScene == NULL )
    {
    vtkErrorMacro ( "ModifyAllListExposure: got NULL MRMLScene." );
    return;
    }
  if ( exposureState != 0 && exposureState != 1 )
    {
    vtkErrorMacro ( "ModifyAllListExposure: got bad value for lock state; should be 0 or 1" );
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
      flNode->SetVisibility ( exposureState );
      // this->Update3DWidgetVisibility(flNode);
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

  // update GUI
//  this->ModifySelectedListExposureGUI ( );
}



//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::ModifySelectedListExposureGUI (  )
{
  vtkMRMLFiducialListNode *activeFiducialListNode = (vtkMRMLFiducialListNode *)this->MRMLScene->GetNodeByID(this->GetFiducialListNodeID());
  if ( activeFiducialListNode == NULL )
    {
    vtkErrorMacro ( "ModifySelectedListExposureGUI: got NULL activeFiducialListNode." );
    return;
    }

  // update the selected list's GUI setting too.
  if ( this->GetApplicationGUI() == NULL )
    {
    vtkErrorMacro ( "ModifySelectedListExposureGUI: Got NULL ApplicationGUI" );
    return;
    }
  if ( this->GetApplicationGUI()->GetSlicerFoundationIcons() == NULL )
    {
    vtkErrorMacro ( "ModifySelectedListExposureGUI: Got NULL SlicerFoundationIcons." );
    return;
    }
  if (activeFiducialListNode->GetVisibility() > 0)
    {
    this->GetHideListToggle()->SetImageToIcon(this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerFiducialsExposeListIcon() );
    this->HideListToggle->SetBalloonHelpString ( "Hide the selected fiducial list (preserve fiducial visibility settings.)" );
    }
  else
    {
    this->GetHideListToggle()->SetImageToIcon(this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerFiducialsHideListIcon() );
    this->HideListToggle->SetBalloonHelpString ( "Expose the selected fiducial list (preserve fiducial visibility settings.)" );
    }

}



//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::ProcessLogicEvents ( vtkObject * vtkNotUsed(caller),
                                              unsigned long vtkNotUsed(event), void * vtkNotUsed(callData) )
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
    vtkDebugMacro("vtkSlicerFiducialsGUI::ProcessMRMLEvents: got a node deleted event on scene");
    // check to see if it was the current node that was deleted
    if (callData != NULL)
      {
      vtkMRMLNode *delNode = (vtkMRMLNode *)callData;
      if (delNode != NULL &&
          delNode->GetID() == this->GetFiducialListNodeID())
        {
        vtkDebugMacro("My node got deleted " << this->GetFiducialListNodeID());
        // remove observers that were added in SetFiducialListNodeID
        this->MRMLObserverManager->SetObject( vtkObjectPointer( &delNode ), NULL);
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
        if (selnode->GetActiveFiducialListID() != NULL)
          {
          if (this->GetFiducialListNodeID() == NULL)
            {
            // set the selection node's choice to be active here
            this->SetFiducialListNodeID(selnode->GetActiveFiducialListID());
            }
          else if (strcmp(selnode->GetActiveFiducialListID(), this->GetFiducialListNodeID()) != 0)
            {
            vtkDebugMacro("Updating the fid gui's fid list node id\n");
            this->SetFiducialListNodeID(selnode->GetActiveFiducialListID());
            }
          }
        }
      }

    
    vtkMRMLFiducialListNode *node = vtkMRMLFiducialListNode::SafeDownCast(caller);
    vtkMRMLFiducialListNode *activeFiducialListNode = (vtkMRMLFiducialListNode *)this->MRMLScene->GetNodeByID(this->GetFiducialListNodeID());

    // don't need to do anything in particular if there's a node added event
    // on the scene, until it's selected in the GUI we don't care about new
    // fid lists
    /*
    // check for a node added event
    if (//vtkMRMLScene::SafeDownCast(caller) != NULL &&
        //vtkMRMLScene::SafeDownCast(caller) == this->MRMLScene &&
        node != NULL &&
        event == vtkMRMLScene::NodeAddedEvent)
      {
      vtkDebugMacro("vtkSlicerFiducialsGUI::ProcessMRMLEvents: got a node added event on scene");
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
    */
    if (node == activeFiducialListNode)
      {
      if (event == vtkCommand::ModifiedEvent ||
          event == vtkMRMLScene::NodeAddedEvent || event == vtkMRMLScene::NodeRemovedEvent)
        {
        vtkDebugMacro("Modified or node added or removed event on the fiducial list node.");
        
        if (node == NULL)
          {
          vtkDebugMacro("\tBUT: the node is null");
          // check to see if the id used to get the node is not null, if it's
          // a valid string, means that the node was deleted
          if (this->GetFiducialListNodeID() != NULL)
            {
            this->SetFiducialListNodeID(NULL);
            }
          return;
          }
       
        if (event == vtkCommand::ModifiedEvent || event == vtkMRMLScene::NodeRemovedEvent)
          {
          vtkDebugMacro("ProcessMRMLEvents: modified or node removed on the list. Updating the GUI");
          // update the table
          SetGUIFromList(activeFiducialListNode);
          }
        else if (event == vtkMRMLScene::NodeAddedEvent)
          {
          // just add the one fiducial to the end of the table
          this->MultiColumnList->GetWidget()->AddRow();
          int row = this->MultiColumnList->GetWidget()->GetNumberOfRows() - 1;
          vtkDebugMacro("ProcessMRMLEvents: Got node added on fid list, calling  update row " << row << " from " << row << "th fid");
          this->UpdateRowFromNthFiducial(row, activeFiducialListNode, row, true, true);
          }
        return;
        }
      else if ( event == vtkMRMLFiducialListNode::FiducialModifiedEvent)
        {
        vtkDebugMacro("\tfiducial modified event on the active fiducial list.");
        if (node == NULL)
          {
          return;
          }
        // check to see if have the id of the fid that was modified
        vtkDebugMacro("ProcessMRMLEvents: got a fiducial modified event on the active fiducial list.");
        // just one was modified, see if have the id
        if (callData != NULL)
          {
          std::string *id =  reinterpret_cast<std::string *>(callData);
          if (id)
            {
            int n = activeFiducialListNode->GetFiducialIndex(*id);
            vtkDebugMacro("ProcessMRMLEvents: got modified point id " << *id << " and index " << n << ", updating row " << n);
            this->UpdateRowFromNthFiducial(n, activeFiducialListNode, n, false, true);
            }
          }
        else
          {
          // one of them changed, update the whole list
          vtkDebugMacro("ProcessMRMLEvents: didn't get the id of the modified fid, calling SetGUIFromList");
          SetGUIFromList(activeFiducialListNode);
          }
        return;
        }
      else if (event == vtkMRMLFiducialListNode::DisplayModifiedEvent)
        {
        vtkDebugMacro("vtkSlicerFiducialsGUI::ProcessMRMLEvents: DisplayModified event on the fiducial list node...\n");
        this->SetGUIDisplayFrameFromList(activeFiducialListNode);
        }
      else if (event == vtkMRMLFiducialListNode::FiducialIndexModifiedEvent)
        {
        vtkDebugMacro("ProcessMRMLEvents: got a fiducial index modified event");
        FiducialListSwappedIndices *swap  = reinterpret_cast<FiducialListSwappedIndices *>(callData);
        if (swap)
          {
          vtkDebugMacro("ProcessMRMLEvents: fid index modified, first = " << swap->first << ", second = " << swap->second);
          // update those two rows
          this->UpdateRowFromNthFiducial(swap->first, activeFiducialListNode, swap->first, false, false);
          this->UpdateRowFromNthFiducial(swap->second, activeFiducialListNode, swap->second, false, true);
          }
        }
      } // end of events on the active fiducial list node

    if (node != NULL &&
        this->FiducialListSelectorWidget->GetSelected() != NULL &&
        node == vtkMRMLFiducialListNode::SafeDownCast(this->FiducialListSelectorWidget->GetSelected()) &&
        event == vtkCommand::ModifiedEvent)
      {
      vtkDebugMacro("\tmodified event on the fiducial list selected node.\n");
      if (activeFiducialListNode !=  vtkMRMLFiducialListNode::SafeDownCast(this->FiducialListSelectorWidget->GetSelected()))
        {
        // select it first off
        this->SetFiducialListNodeID(vtkMRMLFiducialListNode::SafeDownCast(this->FiducialListSelectorWidget->GetSelected())->GetID());
        }
      vtkDebugMacro("ProcessMRMLEvents: Setting gui from list after modified event");
      SetGUIFromList(activeFiducialListNode);
      return;        
      }    
    

    // did the scene close?
    if (event == vtkMRMLScene::SceneClosedEvent)
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
    int numToAdd = 0;
    vtkDebugMacro("SetGUIFromList: have " << numPoints << " points in the list and " << this->MultiColumnList->GetWidget()->GetNumberOfRows() << " rows in the multicolumn list box");
    
    if (numPoints > this->MultiColumnList->GetWidget()->GetNumberOfRows())
      {
      // add some rows
      numToAdd = numPoints -  this->MultiColumnList->GetWidget()->GetNumberOfRows();
      this->MultiColumnList->GetWidget()->AddRows(numToAdd);
      }
    if (numPoints <  this->MultiColumnList->GetWidget()->GetNumberOfRows())
      {
      // delete some rows
      for (int r = this->MultiColumnList->GetWidget()->GetNumberOfRows(); r >= numPoints; r--)
        {
        this->MultiColumnList->GetWidget()->DeleteRow(r);
        }
      }
    // final check (in case not all fids are going to be shown, see vtkSlicerColorDisplayWidget)
    if (numPoints !=  this->MultiColumnList->GetWidget()->GetNumberOfRows())
      {
      vtkDebugMacro("Clearing out the fids MCLB, numPoints = " << numPoints);
      this->MultiColumnList->GetWidget()->DeleteAllRows();
      }
    else
      {
      deleteFlag = false;
      }

    //--- configure fiducial list gui        
    vtkDebugMacro("SetGUIFromList, about to loop over " << numPoints << " points");
    // a row in the individual fiducial gui for each point
    for (int row = 0; row < numPoints; row++)
      {
      // add a row for this point
      if (deleteFlag)
        {
        vtkDebugMacro("SetGUIFromList: Adding point " << row << " to the table" << endl);
        this->MultiColumnList->GetWidget()->AddRow();
        }
      // if are now in the added rows, let the update row know it's a new row
      bool newRowFlag = deleteFlag;
      if (deleteFlag == false &&
          numToAdd != 0 &&
          row >= numPoints - numToAdd)
        {
        newRowFlag = true;
        }
      // now populate it
      this->UpdateRowFromNthFiducial(row, activeFiducialListNode, row, newRowFlag, false);
      }
    
    this->UpdateMeasurementLabels();

    // update the numbering scheme
    if (this->ListNumberingSchemeMenu->GetWidget()->GetValue() != activeFiducialListNode->GetNumberingSchemeAsString())
      {
      this->ListNumberingSchemeMenu->GetWidget()->SetValue(activeFiducialListNode->GetNumberingSchemeAsString());
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
void vtkSlicerFiducialsGUI::UpdateRowFromNthFiducial(int row, vtkMRMLFiducialListNode *fidList, int n, bool newRowFlag, bool updateMeasurementsFlag)
{
  if ((row < 0) ||
      (row >= this->MultiColumnList->GetWidget()->GetNumberOfRows()))
    {
    vtkErrorMacro("UpdateRowFromNthFiducial: row " << row << " out of range 0-" <<  this->MultiColumnList->GetWidget()->GetNumberOfRows());
    return;
    }
  if (fidList == NULL)
    {
    vtkErrorMacro("UpdateRowFromNthFiducial: null fiducial list for row " << row);
    return;
    }
  if (n < 0 ||
      n > fidList->GetNumberOfFiducials())
    {
    vtkErrorMacro("UpdateRowFromNthFiducial: n " << n << " is out of bounds from 0-" << fidList->GetNumberOfFiducials());
    return;
    }

  vtkDebugMacro("UpdateRowFromNthFiducial: row = " << row << ", n = " << n << ", new row = " << newRowFlag << ", update measurements = " << updateMeasurementsFlag);
  
      
  if (fidList->GetNthFiducialLabelText(n) != NULL)
    {
    if (strcmp(this->MultiColumnList->GetWidget()->GetCellText(row,this->NameColumn), fidList->GetNthFiducialLabelText(n)) != 0)
      {
      this->MultiColumnList->GetWidget()->SetCellText(row,this->NameColumn,fidList->GetNthFiducialLabelText(n));
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
  if (newRowFlag || this->MultiColumnList->GetWidget()->GetCellTextAsInt(row,this->SelectedColumn) != (fidList->GetNthFiducialSelected(n) ? 1 : 0))
    {
    this->MultiColumnList->GetWidget()->SetCellTextAsInt(row,this->SelectedColumn,(fidList->GetNthFiducialSelected(n) ? 1 : 0));
    this->MultiColumnList->GetWidget()->SetCellWindowCommandToCheckButton(row,this->SelectedColumn);
    this->MultiColumnList->GetWidget()->SetCellBackgroundColor ( row, this->SelectedColumn, 1.0, 1.0, 1.0 );
    this->MultiColumnList->GetWidget()->SetCellSelectionBackgroundColor ( row, this->SelectedColumn, 1.0, 1.0, 1.0 );
    }
  // visibility
  int fidVisibility = fidList->GetNthFiducialVisibility(n);
  if (newRowFlag || this->MultiColumnList->GetWidget()->GetCellTextAsInt(row,this->VisibilityColumn) != ( fidVisibility ? 1 : 0))
    {
    if ( !fidVisibility)
      {
      this->MultiColumnList->GetWidget()->SetCellTextAsInt(row, this->VisibilityColumn, 0);
      this->MultiColumnList->GetWidget()->SetCellImageToIcon (row, this->VisibilityColumn, this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerInvisibleIcon() );
      }
    else
      {
      this->MultiColumnList->GetWidget()->SetCellTextAsInt(row, this->VisibilityColumn, 1 );
      this->MultiColumnList->GetWidget()->SetCellImageToIcon (row, this->VisibilityColumn, this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerVisibleIcon() );
      }
    }
  // lock (not implemented on a per fid basis yet)
  /*
    int listLock = fidList->GetNthFiducialLocked(n);
    if (newRowFlag || this->MultiColumnList->GetWidget()->GetCellTextAsInt(row,this->LockColumn) != (listLock ? 1 : 0))
    {
    if ( !listLock )
    {
    this->MultiColumnList->GetWidget()->SetCellTextAsInt(row, this->LockColumn, 0 );
    this->MultiColumnList->GetWidget()->SetCellImageToIcon (row, this->LockColumn, this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerUnlockIcon() );
    }
    else
    {
    this->MultiColumnList->GetWidget()->SetCellTextAsInt(row, this->LockColumn, 1 );
    this->MultiColumnList->GetWidget()->SetCellImageToIcon (row, this->LockColumn, this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerLockIcon() );
    }
    }
  */
  float *xyz = fidList->GetNthFiducialXYZ(n);
  if (xyz == NULL)
    {
    vtkErrorMacro ("UpdateRowFromNthFiducial: ERROR: got null xyz for point " << row);
    }
  else
    {
    // always set it if it's a new row, because the numerical default is 0
    if (newRowFlag ||
        this->MultiColumnList->GetWidget()->GetCellTextAsDouble(row,this->XColumn) != xyz[0])
      {
      this->MultiColumnList->GetWidget()->SetCellTextAsDouble(row,this->XColumn,xyz[0]);
      } 
    if (newRowFlag ||
        this->MultiColumnList->GetWidget()->GetCellTextAsDouble(row,this->YColumn) != xyz[1])
      {
      this->MultiColumnList->GetWidget()->SetCellTextAsDouble(row,this->YColumn,xyz[1]);
      }
    if (newRowFlag ||
        this->MultiColumnList->GetWidget()->GetCellTextAsDouble(row,this->ZColumn) != xyz[2])
      {
      this->MultiColumnList->GetWidget()->SetCellTextAsDouble(row,this->ZColumn,xyz[2]);
      }
    }

  float *wxyz = fidList->GetNthFiducialOrientation(n);
  if (wxyz != NULL)
    {
    if (newRowFlag ||
        this->MultiColumnList->GetWidget()->GetCellTextAsDouble(row,this->OrWColumn) != wxyz[0])
      {
      this->MultiColumnList->GetWidget()->SetCellTextAsDouble(row,this->OrWColumn,wxyz[0]);
      }
    if (newRowFlag ||
        this->MultiColumnList->GetWidget()->GetCellTextAsDouble(row,this->OrXColumn) != wxyz[1])
      {
      this->MultiColumnList->GetWidget()->SetCellTextAsDouble(row,this->OrXColumn,wxyz[1]);
      }
    if (newRowFlag ||
        this->MultiColumnList->GetWidget()->GetCellTextAsDouble(row,this->OrYColumn) != wxyz[2])
      {
      this->MultiColumnList->GetWidget()->SetCellTextAsDouble(row,this->OrYColumn,wxyz[2]);
      }
    if (newRowFlag || this->MultiColumnList->GetWidget()->GetCellTextAsDouble(row,this->OrZColumn) != wxyz[3])
      {
      this->MultiColumnList->GetWidget()->SetCellTextAsDouble(row,this->OrZColumn,wxyz[3]);
      }
    }
  else
    {
    vtkErrorMacro ("UpdateRowFromNthFiducial: ERROR: got null wxyz for point " << n);
    }

  if (updateMeasurementsFlag)
    {
    this->UpdateMeasurementLabels();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::SetGUIDisplayFrameFromList(vtkMRMLFiducialListNode * activeFiducialListNode)
{
  if (activeFiducialListNode == NULL)
    {
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
  
  this->ModifySelectedListExposureGUI ( );
  this->ModifySelectedListLockGUI();
  this->ModifyIndividualFiducialsLockGUI();

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
     const char *help = "The Fiducials Module creates and manages lists of Fiducial points. <a>http://www.slicer.org/slicerWiki/index.php/Modules:Fiducials-Documentation-3.6</a>.\nClick on the tool bar icon of an arrow pointing to a starburst fiducial to enter the 'place a new object mode', then click on 3D models or on 2D slices.\nYou can also place fiducials while in 'tranform view' mode by positioning the mouse over a 2D slice plane in the Slice view windows (it must be the active window) and pressing the 'p' key. You can then click and drag the fiducial using the mouse in 'transform view' mode.\nYou can reset the positions of the fiducials in the table below, and adjust selection (fiducials must be selected if they are to be passed into a command line module).\nTo align slices with fiducials, move the fiducial while holding down the Control key.\nYou can right click in a row to align slices to that fiducial, or in the 2d slice windows you can use the '`' key to jump to the next fiducial, Control-` to jump backwards through the list.\nUse the backspace or delete key to delete a fiducial over which you are hovering in 2D.\nThe distance between the first two selected fiducials in the list will be computed automatically and appear in a label below the list of fiducials.";
    const char *about = "This module was developed by Nicole Aucoin, SPL, BWH (Ron Kikinis). This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details. ";
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
    // GLOBAL CONTROLS FRAME
    //---
    vtkSlicerModuleCollapsibleFrame *controlAllFrame = vtkSlicerModuleCollapsibleFrame::New();
    controlAllFrame->SetParent ( page );
    controlAllFrame->Create();
    controlAllFrame->SetLabelText ("Modify All Fiducial Lists & Their Fiducial Points" );
    controlAllFrame->ExpandFrame();
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  controlAllFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("Fiducials")->GetWidgetName());

    //---
    //--- create some temporary frames and a label for grid packaging.
    //---
    vtkKWFrame *allControlFrame = vtkKWFrame::New();
    allControlFrame->SetParent ( controlAllFrame->GetFrame() );
    allControlFrame->Create();
    vtkKWFrame *fA = vtkKWFrame::New();
    fA->SetParent ( allControlFrame );
    fA->Create();
    //---explicitly set size to align with  similar icons in selectedListFrame below
    fA->SetWidth ( 120 );
    vtkKWFrame *fB = vtkKWFrame::New();
    fB->SetParent ( allControlFrame );
    fB->Create();
    vtkKWFrame *fC = vtkKWFrame::New();
    fC->SetParent ( allControlFrame );
    fC->Create();

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
    this->AllLockMenuButton->SetBalloonHelpString ( "Lock or unlock all fiducial lists (and their fiducial points)." );
    this->AllLockMenuButton->GetMenu()->AddRadioButton ( "Lock All Lists");
    int index = this->AllLockMenuButton->GetMenu()->GetIndexOfItem ("Lock All Lists");
    this->AllLockMenuButton->GetMenu()->SetItemImageToIcon (index, this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerLockIcon()  );
    this->AllLockMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
    this->AllLockMenuButton->GetMenu()->SetItemIndicatorVisibility ( index, 0);
    this->AllLockMenuButton->GetMenu()->AddRadioButton ( "Unlock All Lists");
    index = this->AllLockMenuButton->GetMenu()->GetIndexOfItem ("Unlock All Lists");
    this->AllLockMenuButton->GetMenu()->SetItemImageToIcon (index, this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerUnlockIcon()  );
    this->AllLockMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
    this->AllLockMenuButton->GetMenu()->SetItemIndicatorVisibility ( index, 0);
    this->AllLockMenuButton->GetMenu()->AddSeparator();
    this->AllLockMenuButton->GetMenu()->AddRadioButton ( "close");
    index = this->AllLockMenuButton->GetMenu()->GetIndexOfItem ("close");
    this->AllLockMenuButton->GetMenu()->SetItemIndicatorVisibility ( index, 0);

    this->HideOrExposeAllFiducialListsMenuButton = vtkKWMenuButton::New();
    this->HideOrExposeAllFiducialListsMenuButton->SetParent ( fB );
    this->HideOrExposeAllFiducialListsMenuButton->Create();
    this->HideOrExposeAllFiducialListsMenuButton->SetBorderWidth(0);
    this->HideOrExposeAllFiducialListsMenuButton->SetReliefToFlat();
    this->HideOrExposeAllFiducialListsMenuButton->IndicatorVisibilityOff();
    this->HideOrExposeAllFiducialListsMenuButton->SetImageToIcon ( this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerFiducialsHideExposeAllListsIcon() );
    this->HideOrExposeAllFiducialListsMenuButton->SetBalloonHelpString ( "Hide or expose all fiducial lists (preserve fiducial visibility settings).");
    this->HideOrExposeAllFiducialListsMenuButton->GetMenu()->AddRadioButton ( "Hide All Lists");
    index = this->HideOrExposeAllFiducialListsMenuButton->GetMenu()->GetIndexOfItem ("Hide All Lists");
//here
    this->HideOrExposeAllFiducialListsMenuButton->GetMenu()->SetItemCompoundModeToLeft(index);
    this->HideOrExposeAllFiducialListsMenuButton->GetMenu()->SetItemImageToIcon (index, this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerFiducialsHideAllListsIcon()  );
    this->HideOrExposeAllFiducialListsMenuButton->GetMenu()->SetItemIndicatorVisibility ( index, 0);
    this->HideOrExposeAllFiducialListsMenuButton->GetMenu()->AddRadioButton ( "Expose All Lists");
    index = this->HideOrExposeAllFiducialListsMenuButton->GetMenu()->GetIndexOfItem ("Expose All Lists");
    this->HideOrExposeAllFiducialListsMenuButton->GetMenu()->SetItemCompoundModeToLeft(index);
    this->HideOrExposeAllFiducialListsMenuButton->GetMenu()->SetItemImageToIcon (index, this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerFiducialsExposeAllListsIcon()  );
    this->HideOrExposeAllFiducialListsMenuButton->GetMenu()->SetItemIndicatorVisibility ( index, 0);
    this->HideOrExposeAllFiducialListsMenuButton->GetMenu()->AddSeparator();
    this->HideOrExposeAllFiducialListsMenuButton->GetMenu()->AddRadioButton ( "close");
    index = this->HideOrExposeAllFiducialListsMenuButton->GetMenu()->GetIndexOfItem ("close");
    this->HideOrExposeAllFiducialListsMenuButton->GetMenu()->SetItemIndicatorVisibility ( index, 0);
    
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
    this->AllVisibilityMenuButton->SetBalloonHelpString ( "Set visibility on all fiducials in all fiducial lists (preserve fiducial list expose/hide settings)." );
    this->AllVisibilityMenuButton->GetMenu()->AddRadioButton ( "All Fiducials Visible");
    index = this->AllVisibilityMenuButton->GetMenu()->GetIndexOfItem ("All Fiducials Visible");
    this->AllVisibilityMenuButton->GetMenu()->SetItemImageToIcon (index, this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerVisibleIcon()  );
    this->AllVisibilityMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
    this->AllVisibilityMenuButton->GetMenu()->SetItemIndicatorVisibility ( index, 0);
    this->AllVisibilityMenuButton->GetMenu()->AddRadioButton ( "All Fiducials Invisible");
    index = this->AllVisibilityMenuButton->GetMenu()->GetIndexOfItem ("All Fiducials Invisible");
    this->AllVisibilityMenuButton->GetMenu()->SetItemImageToIcon (index, this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerInvisibleIcon()  );
    this->AllVisibilityMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
    this->AllVisibilityMenuButton->GetMenu()->SetItemIndicatorVisibility ( index, 0);
    this->AllVisibilityMenuButton->GetMenu()->AddSeparator();
    this->AllVisibilityMenuButton->GetMenu()->AddRadioButton ( "close");
    index = this->AllVisibilityMenuButton->GetMenu()->GetIndexOfItem ("close");
    this->AllVisibilityMenuButton->GetMenu()->SetItemIndicatorVisibility ( index, 0);

    // remove all fiducials from all lists and remove fiducial lists  button
    this->RemoveAllFiducialsButton = vtkKWPushButton::New ( );
    this->RemoveAllFiducialsButton->SetParent ( fC );
    this->RemoveAllFiducialsButton->Create ( );
    this->RemoveAllFiducialsButton->SetImageToIcon ( this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerFiducialsDeleteAllIcon() );
    this->RemoveAllFiducialsButton->SetReliefToFlat();
    this->RemoveAllFiducialsButton->SetBorderWidth ( 0 );
    this->RemoveAllFiducialsButton->SetBalloonHelpString("Delete all fiducial points in all fiducial lists, and all fiducial lists.");

    //---
    //--- package these up to look nice.
    //---
    app->Script( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2", allControlFrame->GetWidgetName() );
    app->Script ( "grid %s -row 0 -column 0 -sticky w -padx 0 -pady 2", fA->GetWidgetName() );
    app->Script ( "grid %s -row 0 -column 1 -sticky we -padx 4 -pady 2", fB->GetWidgetName() );
    app->Script ( "grid %s -row 0 -column 2 -sticky e -padx 4 -pady 2", fC->GetWidgetName() );
    app->Script ( "grid columnconfigure %s 0 -weight 0 -minsize 120", allControlFrame->GetWidgetName() );
    app->Script ( "grid columnconfigure %s 1 -weight 1", allControlFrame->GetWidgetName() );    
    app->Script ( "grid columnconfigure %s 2 -weight 0", allControlFrame->GetWidgetName() );    
    app->Script("pack %s %s -side left -anchor w -padx 2 -pady 2", 
                this->SelectAllFiducialsButton->GetWidgetName(),
                this->DeselectAllFiducialsButton->GetWidgetName() );
    app->Script("pack %s %s %s -side left -anchor w -padx 2 -pady 2", 
                this->HideOrExposeAllFiducialListsMenuButton->GetWidgetName(),
                this->AllLockMenuButton->GetWidgetName(),
                this->AllVisibilityMenuButton->GetWidgetName() );
    app->Script("pack %s -side right -anchor e -padx 2 -pady 2", 
                this->RemoveAllFiducialsButton->GetWidgetName() );
    

    //---
    // SELECTED LIST FRAME
    //---
    vtkSlicerModuleCollapsibleFrame *selectedListFrame = vtkSlicerModuleCollapsibleFrame::New();
    selectedListFrame->SetParent ( page );
    selectedListFrame->Create();
    selectedListFrame->SetLabelText ("Modify a Selected Fiducial List & its Fiducials" );
    selectedListFrame->ExpandFrame();
    app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 2 -pady 2 -in %s",
                  selectedListFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("Fiducials")->GetWidgetName());
    //---
    //--- create node selector
    //---
    this->FiducialListSelectorWidget = vtkSlicerNodeSelectorWidget::New();
    this->FiducialListSelectorWidget->SetParent( selectedListFrame->GetFrame());
    this->FiducialListSelectorWidget->Create();
    this->FiducialListSelectorWidget->SetNodeClass("vtkMRMLFiducialListNode", NULL, NULL, NULL);
    this->FiducialListSelectorWidget->NewNodeEnabledOn();
    this->FiducialListSelectorWidget->SetMRMLScene(this->GetMRMLScene());
    this->FiducialListSelectorWidget->SetBorderWidth(2);
    this->FiducialListSelectorWidget->SetPadX(2);
    this->FiducialListSelectorWidget->SetPadY(2);
    this->FiducialListSelectorWidget->GetWidget()->SetWidth(24);
    this->FiducialListSelectorWidget->SetLabelText( "Fiducial List:");
    this->FiducialListSelectorWidget->SetBalloonHelpString("Select a fiducial list from the current MRML scene.");
    app->Script ( "pack %s -side top -fill x -expand y -padx 2 -pady 2",
                  this->FiducialListSelectorWidget->GetWidgetName() );

    vtkKWFrame *listControlFrame = vtkKWFrame::New();
    listControlFrame->SetParent ( selectedListFrame->GetFrame() );
    listControlFrame->Create();

    vtkKWFrame *fA2 = vtkKWFrame::New();
    fA2->SetParent ( listControlFrame );
    fA2->Create();
    //---explicitly set size to align with  similar icons in controlAllFrame above
    fA2->SetWidth ( 120 );
    vtkKWFrame *fB2 = vtkKWFrame::New();
    fB2->SetParent ( listControlFrame );
    fB2->Create();
    vtkKWFrame *fC2 = vtkKWFrame::New();
    fC2->SetParent ( listControlFrame );
    fC2->Create();
    app->Script( "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2", listControlFrame->GetWidgetName() );
    app->Script ( "grid %s -row 0 -column 0 -sticky w -padx 0 -pady 2", fA2->GetWidgetName() );
    app->Script ( "grid %s -row 0 -column 1 -sticky we -padx 4 -pady 2", fB2->GetWidgetName() );
    app->Script ( "grid %s -row 0 -column 2 -sticky e -padx 4 -pady 2", fC2->GetWidgetName() );
    app->Script ( "grid columnconfigure %s 0 -weight 0 -minsize 120", listControlFrame->GetWidgetName() );
    app->Script ( "grid columnconfigure %s 1 -weight 1", listControlFrame->GetWidgetName() );    
    app->Script ( "grid columnconfigure %s 2 -weight 0", listControlFrame->GetWidgetName() );    
    
    vtkKWLabel *ll = vtkKWLabel::New();
    ll->SetParent ( fA2);
    ll->Create();
    ll->SetText ( "  Modify List:");
    app->Script( "pack %s -side left -anchor w -padx 2 -pady 2", ll->GetWidgetName() );
    ll->Delete();
    
    // visibility
    this->HideListToggle = vtkKWPushButton::New();
    this->HideListToggle->SetParent ( fB2 );
    this->HideListToggle->Create ( );
    this->HideListToggle->SetReliefToFlat ( );
    this->HideListToggle->SetOverReliefToNone ( );
    this->HideListToggle->SetBorderWidth ( 0 );
    this->HideListToggle->SetImageToIcon ( this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerFiducialsExposeListIcon() );
    this->HideListToggle->SetBalloonHelpString ( "Hide the selected fiducial list (preserve fiducial visibility settings.)" );

    //---
    //--- create all lock menu button and set up menu
    //---
    this->FiducialsListLockToggle = vtkKWPushButton::New();
    this->FiducialsListLockToggle->SetParent ( fB2 );
    this->FiducialsListLockToggle->Create();
    this->FiducialsListLockToggle->SetOverReliefToNone();
    this->FiducialsListLockToggle->SetBorderWidth(0);
    this->FiducialsListLockToggle->SetReliefToFlat();
    this->FiducialsListLockToggle->SetImageToIcon ( this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerFiducialsUnlockListIcon() );
    this->FiducialsListLockToggle->SetBalloonHelpString ( "Lock or unlock the selected fiducial list." );

    //---
    //--- create all visibility menu button and set up menu
    //---    
    this->ListVisibilityMenuButton = vtkKWMenuButton::New();
    this->ListVisibilityMenuButton->SetParent (fB2 );
    this->ListVisibilityMenuButton->Create();
    this->ListVisibilityMenuButton->SetBorderWidth(0);
    this->ListVisibilityMenuButton->SetReliefToFlat();
    this->ListVisibilityMenuButton->IndicatorVisibilityOff();    
    this->ListVisibilityMenuButton->SetImageToIcon ( this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerVisibleOrInvisibleIcon() );
    this->ListVisibilityMenuButton->SetBalloonHelpString ( "Set visibility on all fiducials in the selected list (preserve fiducial list exposure setting)." );
    this->ListVisibilityMenuButton->GetMenu()->AddRadioButton ( "List's Fiducials Visible");
    index = this->ListVisibilityMenuButton->GetMenu()->GetIndexOfItem ("List's Fiducials Visible");
    this->ListVisibilityMenuButton->GetMenu()->SetItemImageToIcon (index, this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerVisibleIcon()  );
    this->ListVisibilityMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
    this->ListVisibilityMenuButton->GetMenu()->SetItemIndicatorVisibility ( index, 0);
    this->ListVisibilityMenuButton->GetMenu()->AddRadioButton ( "List's Fiducials Invisible");
    index = this->ListVisibilityMenuButton->GetMenu()->GetIndexOfItem ("List's Fiducials Invisible");
    this->ListVisibilityMenuButton->GetMenu()->SetItemImageToIcon (index, this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerInvisibleIcon()  );
    this->ListVisibilityMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
    this->ListVisibilityMenuButton->GetMenu()->SetItemIndicatorVisibility ( index, 0);
    this->ListVisibilityMenuButton->GetMenu()->AddSeparator();
    this->ListVisibilityMenuButton->GetMenu()->AddRadioButton ( "close");
    index = this->ListVisibilityMenuButton->GetMenu()->GetIndexOfItem ("close");
    this->ListVisibilityMenuButton->GetMenu()->SetItemIndicatorVisibility ( index, 0);

    // delete
    this->RemoveSelectedListButton = vtkKWPushButton::New();
    this->RemoveSelectedListButton->SetParent ( fC2 );
    this->RemoveSelectedListButton->Create ( );
    this->RemoveSelectedListButton->SetReliefToFlat ( );
    this->RemoveSelectedListButton->SetOverReliefToNone ( );
    this->RemoveSelectedListButton->SetBorderWidth ( 0 );
    this->RemoveSelectedListButton->SetImageToIcon ( this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerFiducialsDeleteListIcon() );
    this->RemoveSelectedListButton->SetBalloonHelpString ( "Delete all fiducial points in the selected list, and then delete the list." );
    
    app->Script("pack %s %s %s -side left -anchor w -padx 2 -pady 2", 
                  this->HideListToggle->GetWidgetName(),
                  this->FiducialsListLockToggle->GetWidgetName(),
                  this->ListVisibilityMenuButton->GetWidgetName() );
    app->Script("pack %s -side right -anchor e -padx 2 -pady 2", 
                this->RemoveSelectedListButton->GetWidgetName() );    

    vtkKWFrameWithLabel *otherFrame = vtkKWFrameWithLabel::New();
    otherFrame->SetParent ( selectedListFrame->GetFrame() );
    otherFrame->Create();
    otherFrame->SetLabelText ( " Other List Display Properties" );
    otherFrame->CollapseFrame();
    app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 2 -pady 2",
                  otherFrame->GetWidgetName());
    
    // glyph type
    this->ListSymbolTypeMenu = vtkKWMenuButtonWithLabel::New();
    this->ListSymbolTypeMenu->SetParent( otherFrame->GetFrame() );
    this->ListSymbolTypeMenu->Create();
    this->ListSymbolTypeMenu->SetLabelPositionToLeft();
    this->ListSymbolTypeMenu->SetLabelWidth ( 14 );
    this->ListSymbolTypeMenu->SetLabelText ( "Glyph type:" );
    this->ListSymbolTypeMenu->GetWidget()->IndicatorVisibilityOn();
//    this->ListSymbolTypeMenu->GetWidget()->SetImageToIcon ( this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerGlyphIcon() );
    this->ListSymbolTypeMenu->GetLabel()->SetAnchorToEast();    
    this->ListSymbolTypeMenu->GetWidget()->SetWidth ( 13 );
    this->ListSymbolTypeMenu->SetBalloonHelpString("Change the type of glyph used to mark the fiducial list points");
    // add the valid glyph types
    vtkSmartPointer<vtkMRMLFiducialListNode> fidlist = vtkSmartPointer<vtkMRMLFiducialListNode>::New();
    int glyphIndex = 0;
    for (int g = vtkMRMLFiducialListNode::GlyphMin;
         g <= vtkMRMLFiducialListNode::GlyphMax;
         g++)
      {
      this->ListSymbolTypeMenu->GetWidget()->GetMenu()->AddRadioButton(fidlist->GetGlyphTypeAsString(g));
//      this->ListSymbolTypeMenu->GetWidget()->GetMenu()->SetItemSelectedValueAsInt(glyphIndex, g);      
      glyphIndex++;
      }
    this->ListSymbolTypeMenu->GetWidget()->SetValue(fidlist->GetGlyphTypeAsString(vtkMRMLFiducialListNode::StarBurst2D));

    // text scale
    this->ListTextScale = vtkKWScaleWithEntry::New();
    this->ListTextScale->SetParent( otherFrame->GetFrame() );
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
    this->ListSymbolScale->SetParent( otherFrame->GetFrame() );
    this->ListSymbolScale->Create();
    this->ListSymbolScale->SetLabelText("Glyph Scale:");
    this->ListSymbolScale->GetLabel()->SetAnchorToEast();
    this->ListSymbolScale->SetLabelWidth ( 14 );
    this->ListSymbolScale->SetBalloonHelpString ( "Set the scale of the fiducial list symbols.");
    this->ListSymbolScale->GetWidget()->SetRange(0.0, 80.0);
    this->ListSymbolScale->GetWidget()->SetOrientationToHorizontal();
    this->ListSymbolScale->GetWidget()->SetResolution(0.5);
    this->ListSymbolScale->SetEntryWidth(5);

    
    // opacity
    this->ListOpacity = vtkKWScaleWithEntry::New();
    this->ListOpacity->SetParent( otherFrame->GetFrame() );
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
    this->ListAmbient->SetParent( otherFrame->GetFrame() );
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
    this->ListDiffuse->SetParent( otherFrame->GetFrame() );
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
    this->ListSpecular->SetParent( otherFrame->GetFrame() );
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
    this->ListPower->SetParent( otherFrame->GetFrame() );
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
    this->ListColorButton->SetParent( otherFrame->GetFrame() );
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
    this->ListSelectedColorButton->SetParent( otherFrame->GetFrame() );
    this->ListSelectedColorButton->Create();
    this->ListSelectedColorButton->SetLabelOutsideButton(1);
    this->ListSelectedColorButton->SetBorderWidth(0);
    this->ListSelectedColorButton->GetLabel()->SetAnchorToEast();
    this->ListSelectedColorButton->SetLabelWidth ( 14 );
    this->ListSelectedColorButton->SetBalloonHelpString("Change the colour of the selected fiducial list symbols and text in the MainViewer");
    this->ListSelectedColorButton->SetDialogTitle("List selected symbol and text color");
    this->ListSelectedColorButton->SetLabelText("Selected Color:");

    app->Script ( "pack %s %s %s %s %s %s %s %s %s %s -side top -anchor nw -padx 2 -pady 3",
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
    // FIDUCIAL FRAME
    //---
    vtkSlicerModuleCollapsibleFrame *fiducialFrame = vtkSlicerModuleCollapsibleFrame::New();
    fiducialFrame->SetParent ( selectedListFrame->GetFrame() );
    fiducialFrame->Create();
    fiducialFrame->SetLabelText ("Add & Modify Individual Fiducials" );
    fiducialFrame->ExpandFrame();
    app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 2 -pady 2",
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

    //---
    // Buttons
    //---

    // add a select all fiducials on this list button
    this->SelectAllFiducialsInListButton = vtkKWPushButton::New ( );
    this->SelectAllFiducialsInListButton->SetParent ( f1 );
    this->SelectAllFiducialsInListButton->Create ( );
    this->SelectAllFiducialsInListButton->SetImageToIcon ( this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerFiducialsSelectAllInListIcon() );
    this->SelectAllFiducialsInListButton->SetReliefToFlat();
    this->SelectAllFiducialsInListButton->SetBorderWidth ( 0 );
    this->SelectAllFiducialsInListButton->SetBalloonHelpString("Select all fiducial points from this fiducial list.");

     // deselect all fiducials on this list button
    this->DeselectAllFiducialsInListButton = vtkKWPushButton::New ( );
    this->DeselectAllFiducialsInListButton->SetParent ( f1 );
    this->DeselectAllFiducialsInListButton->Create ( );
    this->DeselectAllFiducialsInListButton->SetImageToIcon ( this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerFiducialsSelectNoneInListIcon() );
    this->DeselectAllFiducialsInListButton->SetReliefToFlat();
    this->DeselectAllFiducialsInListButton->SetBorderWidth ( 0 );
    this->DeselectAllFiducialsInListButton->SetBalloonHelpString("Deselect all fiducial points from this fiducial list.");
    
     // add fiducial button
    this->AddFiducialButton = vtkKWPushButton::New ( );
    this->AddFiducialButton->SetParent ( f2 );
    this->AddFiducialButton->Create ( );
    this->AddFiducialButton->SetImageToIcon ( this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerFiducialsAddNewIcon() );
    this->AddFiducialButton->SetBorderWidth ( 0 );
    this->AddFiducialButton->SetReliefToFlat();
    this->AddFiducialButton->SetBalloonHelpString("Add a new fiducial point to the selected fiducial list");
    
    // add buttons to move the selected fiducial up or down
    this->MoveSelectedFiducialUpButton = vtkKWPushButton::New();
    this->MoveSelectedFiducialUpButton->SetParent ( f3 );
    this->MoveSelectedFiducialUpButton->Create ( );
    this->MoveSelectedFiducialUpButton->SetImageToIcon ( this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerFiducialsUpIcon() );
    this->MoveSelectedFiducialUpButton->SetReliefToFlat();
    this->MoveSelectedFiducialUpButton->SetBorderWidth ( 0 );
    // using selection callback to capture the toggle on
    // visibility and lock; so to not confuse users with
    // unexpected 'move' behavior, encourage them to
    // click on a fiducial name to move it up/down in list.
    this->MoveSelectedFiducialUpButton->SetBalloonHelpString("Move the last fiducial whose name was \"clicked on\" in the table one row up in the table");


    this->MoveSelectedFiducialDownButton = vtkKWPushButton::New();
    this->MoveSelectedFiducialDownButton->SetParent ( f3 );
    this->MoveSelectedFiducialDownButton->Create ( );
    this->MoveSelectedFiducialDownButton->SetImageToIcon ( this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerFiducialsDownIcon() );
    this->MoveSelectedFiducialDownButton->SetReliefToFlat();
    this->MoveSelectedFiducialDownButton->SetBorderWidth ( 0 );
    // using selection callback to capture the toggle on
    // visibility and lock; so to not confuse users with
    // unexpected 'move' behavior, encourage them to
    // click on a fiducial name to move it up/down in list.
    this->MoveSelectedFiducialDownButton->SetBalloonHelpString("Move the last fiducial whose name was \"clicked on\" in the table one row down in the table");

    // center view on selected fiducial button
    this->Center3DViewOnSelectedFiducialButton = vtkKWPushButton::New();
    this->Center3DViewOnSelectedFiducialButton->SetParent ( f3 );
    this->Center3DViewOnSelectedFiducialButton->Create ( );
    this->Center3DViewOnSelectedFiducialButton->SetImageToIcon ( this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerCenterOnFiducialIcon());
    this->Center3DViewOnSelectedFiducialButton->SetReliefToFlat();
    this->Center3DViewOnSelectedFiducialButton->SetBorderWidth ( 0 );
    // encourage users to
    // click on a fiducial name to recenter.
    this->Center3DViewOnSelectedFiducialButton->SetBalloonHelpString("Center the 3D view on the last fiducial whose name was \"clicked on\" in the table.\nRight click on a fiducial name in the table to jump the 2d slices to that fiducial location.\nWARNING: no undo is currently implemented.");
    
    // remove fiducial button
    this->RemoveFiducialButton = vtkKWPushButton::New ( );
    this->RemoveFiducialButton->SetParent ( f4 );
    this->RemoveFiducialButton->Create ( );
    this->RemoveFiducialButton->SetImageToIcon ( this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerFiducialsDeleteLastClickedIcon() );
    this->RemoveFiducialButton->SetReliefToFlat();
    this->RemoveFiducialButton->SetBorderWidth ( 0 );
    // using selection callback to capture the toggle on
    // visibility and lock; so to not confuse users with
    // unexpected 'delete' behavior, encourage them to
    // click on a fiducial name to delete it from list.
    this->RemoveFiducialButton->SetBalloonHelpString("Remove the last fiducial whose name was \"clicked on\" in the table from this fiducial list.");

    // add a remove all fiducials from this list button"
    this->RemoveFiducialsInListButton = vtkKWPushButton::New ( );
    this->RemoveFiducialsInListButton->SetParent ( f4 );
    this->RemoveFiducialsInListButton->Create ( );
    this->RemoveFiducialsInListButton->SetImageToIcon ( this->GetApplicationGUI()->GetSlicerFoundationIcons()->GetSlicerFiducialsDeleteAllInListIcon() );
    this->RemoveFiducialsInListButton->SetReliefToFlat();
    this->RemoveFiducialsInListButton->SetBorderWidth ( 0 );
    this->RemoveFiducialsInListButton->SetBalloonHelpString("Remove all fiducial points from selected fiducial list.");

    //---
    //--- package these guys up to look nice.
    //---
    app->Script( "pack %s -side top -expand y -fill x -padx 0 -pady 2", f0->GetWidgetName() );
    app->Script( "pack %s -side left -anchor w -padx 6 -pady 2", f1->GetWidgetName() );
    app->Script( "pack %s -side left -anchor w -padx 6 -pady 2", f2->GetWidgetName() );
    app->Script( "pack %s -side left -anchor w -fill x -expand y -padx 6 -pady 2", f3->GetWidgetName() );
    app->Script( "pack %s -side right -anchor e -padx 2 -pady 2", f4->GetWidgetName() );

    app->Script( "pack %s %s -side left -anchor nw -padx 2 -pady 2", 
                this->SelectAllFiducialsInListButton->GetWidgetName(),
                this->DeselectAllFiducialsInListButton->GetWidgetName() );
    app->Script( "pack %s -side left -anchor w -padx 2 -pady 2",
                 this->AddFiducialButton->GetWidgetName() );
    app->Script( "pack %s %s %s -side left -anchor nw -padx 2 -pady 2", 
                 this->MoveSelectedFiducialUpButton->GetWidgetName(),
                 this->MoveSelectedFiducialDownButton->GetWidgetName(),
                 this->Center3DViewOnSelectedFiducialButton->GetWidgetName());
    app->Script( "pack %s %s -side left -anchor ne -padx 2 -pady 2", 
                this->RemoveFiducialButton->GetWidgetName(),
                this->RemoveFiducialsInListButton->GetWidgetName() );
    
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
    // add the selected column with no text; just use the checkbox.
    this->MultiColumnList->GetWidget()->AddColumn("");
    // add the visibility column with no text, use an icon
    this->MultiColumnList->GetWidget()->AddColumn("");
    this->MultiColumnList->GetWidget()->AddColumn("Name");
    this->MultiColumnList->GetWidget()->AddColumn("X");
    this->MultiColumnList->GetWidget()->AddColumn("Y");
    this->MultiColumnList->GetWidget()->AddColumn("Z");
    this->MultiColumnList->GetWidget()->AddColumn("OrW");
    this->MultiColumnList->GetWidget()->AddColumn("OrX");
    this->MultiColumnList->GetWidget()->AddColumn("OrY");
    this->MultiColumnList->GetWidget()->AddColumn("OrZ");
    // add the locked column with no text; just use icon.
    this->MultiColumnList->GetWidget()->AddColumn("");

    // make the selected, visible columns editable by checkbox
     // selection
    this->MultiColumnList->GetWidget()->SetColumnEditWindowToCheckButton(this->SelectedColumn);
    this->MultiColumnList->GetWidget()->SetColumnStretchable(this->SelectedColumn, 0);
    this->MultiColumnList->GetWidget()->SetColumnResizable(this->SelectedColumn, 0);
    // visibility
    this->MultiColumnList->GetWidget()->SetColumnEditWindowToCheckButton(this->VisibilityColumn);
    this->MultiColumnList->GetWidget()->SetColumnStretchable(this->VisibilityColumn, 0);
    this->MultiColumnList->GetWidget()->SetColumnResizable(this->VisibilityColumn, 0);
    // lock
    this->MultiColumnList->GetWidget()->SetColumnEditWindowToCheckButton(this->LockColumn);
    this->MultiColumnList->GetWidget()->SetColumnStretchable(this->LockColumn, 0);
    this->MultiColumnList->GetWidget()->SetColumnResizable(this->LockColumn, 0);

    
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
    this->MultiColumnList->GetWidget()->SetColumnWidth(this->SelectedColumn, 0);
    this->MultiColumnList->GetWidget()->SetColumnWidth(this->VisibilityColumn, 0);
    this->MultiColumnList->GetWidget()->SetColumnWidth(this->LockColumn, 0);
    // turn user's ability to lock/unlock individual fiducials to OFF for now
    this->MultiColumnList->GetWidget()->ColumnEditableOff(this->LockColumn);
    
    // checkbox should be enough to convey state of these attributes; no need for 1|0 text.
    this->MultiColumnList->GetWidget()->SetColumnFormatCommandToEmptyOutput ( this->SelectedColumn );
    this->MultiColumnList->GetWidget()->SetColumnFormatCommandToEmptyOutput ( this->VisibilityColumn );
    this->MultiColumnList->GetWidget()->SetColumnFormatCommandToEmptyOutput ( this->LockColumn );


    // for now, hide the orientation columns
    for (int hideCol = this->OrWColumn; hideCol <= this->OrZColumn; hideCol++)
      {
      this->MultiColumnList->GetWidget()->ColumnVisibilityOff(hideCol);
      }

    app->Script ( "pack %s -fill both -expand true", this->MultiColumnList->GetWidgetName());
    this->MultiColumnList->GetWidget()->SetCellUpdatedCommand(this, "UpdateElement");
    // set up the right click jump slices to that fiducial point call back
    this->MultiColumnList->GetWidget()->SetRightClickCommand(this, "JumpSlicesCallback");
    // will capture the lock/visibility cell toggles
    this->MultiColumnList->GetWidget()->SetSelectionCommand ( this, "VisibilityOrLockToggleCallback");
    
    //---
    // utilities frame
    //---
    vtkKWFrame *utilitiesFrame = vtkKWFrame::New();
    utilitiesFrame->SetParent(fiducialFrame->GetFrame());
    utilitiesFrame->Create();
    app->Script ("pack %s -side top -anchor nw -fill x -pady 0",
                 utilitiesFrame->GetWidgetName());

    // add a label to hold Point-to-Point measurement
    this->MeasurementLabel = vtkKWLabel::New();
    this->MeasurementLabel->SetParent( utilitiesFrame );
    this->MeasurementLabel->Create();
    this->MeasurementLabel->SetText("Distance: ");
    this->MeasurementLabel->SetBalloonHelpString("Distance between first two selected fiducials");
    app->Script("pack %s -side top -anchor nw -fill x -pady 0",
                this->MeasurementLabel->GetWidgetName());

    // add a label to hold Point-to-Point measurement on the whole list
    this->ListMeasurementLabel = vtkKWLabel::New();
    this->ListMeasurementLabel->SetParent( utilitiesFrame );
    this->ListMeasurementLabel->Create();
    this->ListMeasurementLabel->SetText("List Distance: ");
    this->ListMeasurementLabel->SetBalloonHelpString("Summed linear distance between sequential selected fiducials");
    app->Script("pack %s -side top -anchor nw -fill x -pady 0",
                this->ListMeasurementLabel->GetWidgetName());

    //---
    // utility buttons frame
    //---
    vtkKWFrame *utilitiesButtonFrame = vtkKWFrame::New();
    utilitiesButtonFrame->SetParent(fiducialFrame->GetFrame());
    utilitiesButtonFrame->Create();
    app->Script ("pack %s -side top -anchor nw -fill x -pady 0",
                 utilitiesButtonFrame->GetWidgetName());

    // add a button to renumber fids
    this->RenumberButton = vtkKWPushButton::New();
    this->RenumberButton->SetParent(utilitiesButtonFrame);
    this->RenumberButton->Create();
    this->RenumberButton->SetText("Renumber Fiducials");
    //this->RenumberButton->SetReliefToFlat();
    //this->RenumberButton->SetBorderWidth(0);
    this->RenumberButton->SetBalloonHelpString("Renumber the fiducials in this list. Removes any numbers from the ends of the labels and then appends a number denoting the current place in the list, starting from entered value, 0 if none. Note: does not affect the auto name generation for the next added fiducial.");

    // add a button to rename fids
    this->RenameButton = vtkKWPushButton::New();
    this->RenameButton->SetParent(utilitiesButtonFrame);
    this->RenameButton->Create();
    this->RenameButton->SetText("Rename Fiducials");
    //this->RenameButton->SetReliefToFlat();
    //this->RenameButton->SetBorderWidth(0);
    this->RenameButton->SetBalloonHelpString("Rename the fiducials in this list. Preserves any numbers at the end of the labels. Note: does not affect the auto name generation for the next added fiducial - change the list name from the node selector.");

    app->Script("pack %s %s -padx 4 -pady 2 -side left",
                 this->RenumberButton->GetWidgetName(), this->RenameButton->GetWidgetName());

    //---
    // numbering scheme menu frame
    //---
    vtkKWFrame *numberingSchemeFrame = vtkKWFrame::New();
    numberingSchemeFrame->SetParent(fiducialFrame->GetFrame());
    numberingSchemeFrame->Create();
    app->Script ("pack %s -side top -anchor nw -fill x -pady 0",
                 numberingSchemeFrame->GetWidgetName());
    // a menu for the list numbering schemes
    this->ListNumberingSchemeMenu = vtkKWMenuButtonWithLabel::New();
    this->ListNumberingSchemeMenu->SetParent( numberingSchemeFrame );
    this->ListNumberingSchemeMenu->Create();
    this->ListNumberingSchemeMenu->SetBorderWidth(0);
    this->ListNumberingSchemeMenu->SetLabelWidth ( 18 );
    this->ListNumberingSchemeMenu->GetLabel()->SetAnchorToEast();    
    this->ListNumberingSchemeMenu->GetWidget()->SetWidth ( 13 );
    this->ListNumberingSchemeMenu->SetBalloonHelpString("Set the numbering scheme used to assign integers to the next added fiducial in the list. UseID uses the point ID, UseIndex uses the list index of the point, UsePrevious increments the number at the end of the previous point. ");
    this->ListNumberingSchemeMenu->SetLabelText("Numbering Scheme: ");
    // add the valid schemes
    for (int s = vtkMRMLFiducialListNode::SchemeMin;
         s <= vtkMRMLFiducialListNode::SchemeMax;
         s++)
      {
      this->ListNumberingSchemeMenu->GetWidget()->GetMenu()->AddRadioButton(fidlist->GetNumberingSchemeAsString(s));
      }
    this->ListNumberingSchemeMenu->GetWidget()->SetValue(fidlist->GetNumberingSchemeAsString(vtkMRMLFiducialListNode::UseID));
    app->Script("pack %s -padx 2 -pady 2 -side left",
                this->ListNumberingSchemeMenu->GetWidgetName());
        
    //---
    //--- and clean up temporary stuff
    //---
    fA->Delete();
    fB->Delete();
    fC->Delete();
    allControlFrame->Delete();
    fA2->Delete();
    fB2->Delete();
    fC2->Delete();
    listControlFrame->Delete();

    f1->Delete();
    f2->Delete();
    f3->Delete();
    f4->Delete();
    f0->Delete();

    utilitiesFrame->Delete();
    utilitiesButtonFrame->Delete();
    numberingSchemeFrame->Delete();
    listFrame->Delete();
    otherFrame->Delete();
    fiducialFrame->Delete();
    selectedListFrame->Delete();
    controlAllFrame->Delete();
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::VisibilityOrLockToggleCallback ( )
{

  // get the fiducial list
  vtkMRMLFiducialListNode * activeFiducialListNode = (vtkMRMLFiducialListNode *)this->MRMLScene->GetNodeByID(this->GetFiducialListNodeID());
  if (activeFiducialListNode == NULL)
    {
    return;
    }

  // get the row that was last selected;
  // if nothing was selected, then return;
  int numRows = this->MultiColumnList->GetWidget()->GetNumberOfSelectedRows();
  int row[1];
  if (numRows == 1)
    {
    this->MultiColumnList->GetWidget()->GetSelectedRows(row);
    }
  else
    {
    return;
    }

  //--- get individual visibility and list lock status
  //int visibilityState = activeFiducialListNode->GetNthFiducialVisibility(row[0] );
  // what fiducial was selected?
  int s = this->MultiColumnList->GetWidget()->IsCellSelected ( row[0], this->VisibilityColumn );

  if ( s )
    {
    //--- if cell is visible, set to invisible
    if ( activeFiducialListNode->GetNthFiducialVisibility ( row[0] ) > 0 )
      {
      //--- toggle node state
      activeFiducialListNode->SetNthFiducialVisibility ( row[0], 0 );
      }
    else
      {
      activeFiducialListNode->SetNthFiducialVisibility ( row[0], 1 );
      }
    //-- now deselect the cell
    this->MultiColumnList->GetWidget()->DeselectCell (row[0],this->VisibilityColumn);
    return;
    }


  // individual fiducial lock toggle not yet available.
  /*
  //  int lockState = activeFiducialListNode->GetNthFiducialVisibility(row[0] );
  int lockState = activeFiducialListNode->GetLocked();
  s = this->MultiColumnList->GetWidget()->IsCellSelected ( row[0], this->LockColumn);
  if ( s )
  {
  //--- if cell is locked, set to unlocked
  if ( activeFiducialListNode->GetNthFiducialLock ( row[0] ) > 0 )
  {
  //--- toggle node state & update GUI

  activeFiducialListNode->SetNthFiducialLock ( 0 );
  }
  else
  {
  activeFiducialListNode->SetNthFiducialLock ( 1 );
  }
  }
  //-- now deselect the cell
    this->MultiColumnList->GetWidget()->DeselectCell (row[0],this->LockColumn);
  */
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
      this->UpdateMeasurementLabels();
      }
    else if ( col == this->LockColumn)
      {
      //handled in this->VisibilityOrLockToggleCallback
      return;
      }
    else if (col == this->VisibilityColumn)
      {
      //handled in this->VisibilityOrLockToggleCallback
/*
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
*/
      } 
    else if (col >= this->XColumn && col <= this->ZColumn)
      {
      // get the current xyz
      float * xyz = activeFiducialListNode->GetNthFiducialXYZ(row);
      // now set the new one
      float newCoordinate = atof(str);
      
      if ( xyz )
        {
        vtkDebugMacro("UpdateElement: got a position column update, col = " << col << ", row = " << row << "th xyz = " << xyz[0] << ", " << xyz[1] << ", " << xyz[2] << ", str = " << str << " newCoord = " << newCoordinate);
        if (col == this->XColumn) { activeFiducialListNode->SetNthFiducialXYZ(row, newCoordinate, xyz[1], xyz[2]); }
        if (col == this->YColumn) { activeFiducialListNode->SetNthFiducialXYZ(row, xyz[0], newCoordinate, xyz[2]); }
        if (col == this->ZColumn) { activeFiducialListNode->SetNthFiducialXYZ(row, xyz[0], xyz[1], newCoordinate); }
        this->UpdateMeasurementLabels();
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
  vtkMRMLFiducialListNode *oldFidList = vtkMRMLFiducialListNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->GetFiducialListNodeID()));

  // set the id properly - see the vtkSetStringMacro
  this->FiducialListNodeID = id;

  if (id == NULL)
    {
    vtkDebugMacro("SetFiducialListNodeID: NULL input id, clearing GUI and returning.\n");
    this->SetGUIFromList(NULL);
    if (oldFidList != NULL)
      {
      vtkDebugMacro("SetFiducialListNodeID: null input id, but oldFidList is not null, removing all observers");
      this->MRMLObserverManager->SetObject( vtkObjectPointer( &oldFidList), NULL);
      }
    return;
    }
  
  // get the new node
  vtkMRMLFiducialListNode *newFidList = vtkMRMLFiducialListNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->GetFiducialListNodeID()));
  // set up observers on the new node
  if (newFidList != NULL)
    {
    if (this->GetDebug())
      {
      newFidList->DebugOn();
      }
    vtkIntArray *events = vtkIntArray::New();
    events->InsertNextValue(vtkCommand::ModifiedEvent);
    events->InsertNextValue(vtkMRMLFiducialListNode::DisplayModifiedEvent);
    events->InsertNextValue(vtkMRMLFiducialListNode::FiducialModifiedEvent);
    events->InsertNextValue(vtkMRMLFiducialListNode::FiducialIndexModifiedEvent);
    events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
    events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
    vtkSetAndObserveMRMLNodeEventsMacro(oldFidList, newFidList, events);
    events->Delete();

    // set up the GUI
    this->SetGUIFromList(newFidList);
    }
  else
    {
    vtkDebugMacro ("ERROR: unable to get the mrml fiducial node to observe!\n");
    }
  newFidList = NULL;

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
void vtkSlicerFiducialsGUI::UpdateMeasurementLabels()
{
  if (!this->MeasurementLabel || !this->ListMeasurementLabel)
    {
    return;
    }
  std::string newLabel = "Distance: ";
  std::string newListLabel = "List Distance: ";

  // get the fiducial list
  vtkMRMLFiducialListNode * activeFiducialListNode = (vtkMRMLFiducialListNode *)this->MRMLScene->GetNodeByID(this->GetFiducialListNodeID());
  if (activeFiducialListNode == NULL)
    {
    this->MeasurementLabel->SetText(newLabel.c_str());
    this->ListMeasurementLabel->SetText(newListLabel.c_str());
    return;
    }
  
  int numPoints = activeFiducialListNode->GetNumberOfFiducials();
  int numSelected = 0;
  int lastSelectedIndex = -1;
  int thisSelectedIndex = -1;
  double dist = 0.0;
  double listDist = 0.0;
  for (int n = 0; n < numPoints; n++)
    {
    if (activeFiducialListNode->GetNthFiducialSelected(n))
      {
      lastSelectedIndex = thisSelectedIndex;
      thisSelectedIndex = n;
      numSelected++;
      
      if (numSelected >= 2)
        {
        float *xyz1 = activeFiducialListNode->GetNthFiducialXYZ(lastSelectedIndex);
        float *xyz2 = activeFiducialListNode->GetNthFiducialXYZ(thisSelectedIndex);
        if (xyz1 != NULL && xyz2 != NULL)
          {
          dist = sqrt(pow(double(xyz2[0] - xyz1[0]),2) +
                      pow(double(xyz2[1] - xyz1[1]),2) +
                      pow(double(xyz2[2] - xyz1[2]),2));
          }
        if (numSelected == 2)
          {
          std::stringstream ss;
          ss << newLabel;
          ss << activeFiducialListNode->GetNthFiducialLabelText(lastSelectedIndex);
          ss << " to ";
          ss << activeFiducialListNode->GetNthFiducialLabelText(thisSelectedIndex);
          ss << " = ";
          ss << dist;
          ss << " mm";
          newLabel = ss.str();
          listDist = dist;
          }
        else
          {
          listDist += dist;
          }
        }
      }
    }
  this->MeasurementLabel->SetText(newLabel.c_str());
  if (listDist > 0.0)
    {
    std::stringstream ss;
    ss << newListLabel;
    ss << listDist;
    ss << " mm";
    newListLabel = ss.str();
    }
  this->ListMeasurementLabel->SetText(newListLabel.c_str());
}

//---------------------------------------------------------------------------
void vtkSlicerFiducialsGUI::JumpSlicesCallback(int row, int vtkNotUsed(col), int vtkNotUsed(x), int vtkNotUsed(y) )

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

