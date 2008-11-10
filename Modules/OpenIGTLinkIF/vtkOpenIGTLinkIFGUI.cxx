/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkOpenIGTLinkIFGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkSlicerSliceControllerWidget.h"
#include "vtkSlicerSliceGUI.h"
#include "vtkSlicerSlicesGUI.h"

#include "vtkSlicerColor.h"
#include "vtkSlicerTheme.h"

#include "vtkSlicerFiducialsGUI.h"
#include "vtkSlicerFiducialsLogic.h"

#include "vtkKWRenderWidget.h"
#include "vtkKWWidget.h"
#include "vtkKWMenuButton.h"
#include "vtkKWCheckButton.h"
#include "vtkKWRadioButton.h"
#include "vtkKWRadioButtonSet.h"
#include "vtkKWPushButton.h"
#include "vtkKWPushButtonSet.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWMenu.h"
#include "vtkKWLabel.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWEvent.h"
#include "vtkKWOptions.h"
#include "vtkKWComboBox.h"

#include "vtkKWTkUtilities.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkCornerAnnotation.h"

#include "vtkStringArray.h"


// for Realtime Image
#include "vtkImageChangeInformation.h"
#include "vtkSlicerColorLogic.h"

#include "vtkMRMLLinearTransformNode.h"

#include "vtkIGTLConnector.h"

#include <vector>
#include <sstream>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkOpenIGTLinkIFGUI );
vtkCxxRevisionMacro ( vtkOpenIGTLinkIFGUI, "$Revision: 1.0 $");
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Labels
const char *vtkOpenIGTLinkIFGUI::ConnectorTypeStr[vtkIGTLConnector::NUM_TYPE] = 
{
  "?", // TYPE_NOT_DEFINED
  "S", // TYPE_SERVER
  "C", // TYPE_CLIENT
};

const char *vtkOpenIGTLinkIFGUI::ConnectorStatusStr[vtkIGTLConnector::NUM_STATE] = 
{
  "OFF",       // OFF
  "WAIT",      // WAIT_CONNECTION
  "ON",        // CONNECTED
};

//---------------------------------------------------------------------------
vtkOpenIGTLinkIFGUI::vtkOpenIGTLinkIFGUI ( )
{

  //----------------------------------------------------------------
  // Logic values
  
  this->Logic = NULL;
  this->DataManager = vtkIGTDataManager::New();
  this->Pat2ImgReg = vtkIGTPat2ImgRegistration::New();
  
  this->DataCallbackCommand = vtkCallbackCommand::New();
  this->DataCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
  this->DataCallbackCommand->SetCallback(vtkOpenIGTLinkIFGUI::DataCallback);
  
  this->SliceNode0 = NULL; 
  this->SliceNode1 = NULL; 
  this->SliceNode2 = NULL; 
  
  //----------------------------------------------------------------
  // Workphase Frame
  
  //----------------------------------------------------------------
  // Connector List Frame

  this->ConnectorList = NULL;
  this->AddConnectorButton = NULL;
  this->DeleteConnectorButton = NULL;
  this->ConnectorNameEntry = NULL;
  this->ConnectorTypeButtonSet = NULL;
  this->ConnectorStatusCheckButton = NULL;
  this->ConnectorAddressEntry = NULL;
  this->ConnectorPortEntry = NULL;

  this->EnableAdvancedSettingButton = NULL;
  this->AddDeviceNameButton = NULL;
  this->DeleteDeviceNameButton = NULL;

  this->CurrentMrmlNodeListID = -1;
  this->CurrentNodeListAvailable.clear();
  this->CurrentNodeListSelected.clear();


  //----------------------------------------------------------------
  // Visualization Control Frame
  
  this->FreezeImageCheckButton = NULL;
  this->ObliqueCheckButton     = NULL;
  this->SetLocatorModeButton   = NULL;
  this->SetUserModeButton      = NULL;
  this->RedSliceMenu           = NULL;
  this->YellowSliceMenu        = NULL;
  this->GreenSliceMenu         = NULL;

  this->StartScanButton        = NULL;
  this->StopScanButton         = NULL;

  this->FreezeImageCheckButton = NULL;
  this->LocatorCheckButton     = NULL;

  this->ImagingMenu            = NULL;


  //----------------------------------------------------------------
  // Locator  (MRML)
  this->CloseScene             = false;

  this->TimerFlag = 0;

}

//---------------------------------------------------------------------------
vtkOpenIGTLinkIFGUI::~vtkOpenIGTLinkIFGUI ( )
{

  if (this->DataManager)
    {
    // If we don't set the scence to NULL for DataManager,
    // Slicer will report a lot leak when it is closed.
    this->DataManager->SetMRMLScene(NULL);
    this->DataManager->Delete();
    }
  if (this->Pat2ImgReg)
    {
    this->Pat2ImgReg->Delete();
    }
  if (this->DataCallbackCommand)
    {
    this->DataCallbackCommand->Delete();
    }

  this->RemoveGUIObservers();


  //----------------------------------------------------------------
  // Workphase Frame

  this->SetModuleLogic ( NULL );


  //----------------------------------------------------------------
  // Visualization Control Frame

  if (this->FreezeImageCheckButton)
    {
    this->FreezeImageCheckButton->SetParent(NULL );
    this->FreezeImageCheckButton->Delete ( );
    }
  if (this->ObliqueCheckButton)
    {
    this->ObliqueCheckButton->SetParent(NULL );
    this->ObliqueCheckButton->Delete ( );
    }


  if (this->SetLocatorModeButton)
    {
    this->SetLocatorModeButton->SetParent(NULL);
    this->SetLocatorModeButton->Delete();
    }
  
  if (this->SetUserModeButton)
    {
    this->SetUserModeButton->SetParent(NULL);
    this->SetUserModeButton->Delete();
    }

  if (this->RedSliceMenu)
    {
    this->RedSliceMenu->SetParent(NULL );
    this->RedSliceMenu->Delete ( );
    }

  if (this->YellowSliceMenu)
    {
    this->YellowSliceMenu->SetParent(NULL );
    this->YellowSliceMenu->Delete ( );
    }

  if (this->GreenSliceMenu)
    {
    this->GreenSliceMenu->SetParent(NULL );
    this->GreenSliceMenu->Delete ( );
    }

  /*
  if (this->StartScanButton)
    {
    this->StartScanButton->SetParent(NULL);
    this->StartScanButton->Delete();
    }

  if (this->StopScanButton)
    {
    this->StopScanButton->SetParent(NULL);
    this->StopScanButton->Delete();
    }

  if ( this->ImagingMenu )
    {
    this->ImagingMenu->SetParent(NULL);
    this->ImagingMenu->Delete();
    }
  */

  this->IsSliceOrientationAdded = false;


  //----------------------------------------------------------------
  // Etc Frame


  if (this->LocatorCheckButton)
    {
    this->LocatorCheckButton->SetParent(NULL );
    this->LocatorCheckButton->Delete ( );
    }

}



//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
  this->vtkObject::PrintSelf ( os, indent );

  os << indent << "OpenIGTLinkGUI: " << this->GetClassName ( ) << "\n";
  os << indent << "Logic: " << this->GetLogic ( ) << "\n";
   
  // print widgets?
}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFGUI::RemoveGUIObservers ( )
{
  vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
  
  appGUI->GetMainSliceGUI("Red")->GetSliceViewer()->GetRenderWidget()
    ->GetRenderWindowInteractor()->GetInteractorStyle()->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
  appGUI->GetMainSliceGUI("Yellow")->GetSliceViewer()->GetRenderWidget()
    ->GetRenderWindowInteractor()->GetInteractorStyle()->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
  appGUI->GetMainSliceGUI("Green")->GetSliceViewer()->GetRenderWidget()
    ->GetRenderWindowInteractor()->GetInteractorStyle()->RemoveObserver((vtkCommand *)this->GUICallbackCommand);

  //----------------------------------------------------------------
  // Connector Browser Frame

  if (this->ConnectorList->GetWidget())
    {
    this->ConnectorList->GetWidget()
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }

  if (this->AddConnectorButton)
    {
    this->AddConnectorButton
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }

  if (this->DeleteConnectorButton)
    {
    this->DeleteConnectorButton
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }


  if (this->ConnectorNameEntry)
    {
    this->ConnectorNameEntry
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
      
  if (this->ConnectorTypeButtonSet)
    {
    this->ConnectorTypeButtonSet->GetWidget(0)
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    this->ConnectorTypeButtonSet->GetWidget(1)
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  
  if (this->ConnectorStatusCheckButton)
    {
    this->ConnectorStatusCheckButton
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand );
    }

  if (this->ConnectorAddressEntry)
    {
    this->ConnectorAddressEntry
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }

  if (this->ConnectorPortEntry)
    {
    this->ConnectorPortEntry
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }

  if (this->EnableAdvancedSettingButton)
    {
    this->EnableAdvancedSettingButton
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }

  if (this->AddDeviceNameButton)
    {
    this->AddDeviceNameButton
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }

  if (this->DeleteDeviceNameButton)
    {
    this->DeleteDeviceNameButton
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }

  
  if (this->MrmlNodeList->GetWidget())
    {
    this->MrmlNodeList->GetWidget()
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }


  //----------------------------------------------------------------
  // Visualization Control Frame

  if (this->FreezeImageCheckButton)
    {
    this->FreezeImageCheckButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand );
    }
  if (this->ObliqueCheckButton)
    {
    this->ObliqueCheckButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand );
    }

  if (this->SetLocatorModeButton)
    {
    this->SetLocatorModeButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand );
    }

  if (this->SetUserModeButton)
    {
    this->SetUserModeButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand );
    }

  if (this->LocatorCheckButton)
    {
    this->LocatorCheckButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand );
    }

  if (this->RedSliceMenu)
    {
    this->RedSliceMenu->GetMenu()
      ->RemoveObserver((vtkCommand*)this->GUICallbackCommand);
    }
  if (this->YellowSliceMenu)
    {
    this->YellowSliceMenu->GetMenu()
      ->RemoveObserver((vtkCommand*)this->GUICallbackCommand);
    }
  if (this->GreenSliceMenu)
    {
    this->GreenSliceMenu->GetMenu()
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->ImagingMenu)
    {
    this->ImagingMenu->GetMenu()
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  
  this->RemoveLogicObservers();
}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFGUI::RemoveLogicObservers ( )
{
  vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
  if (this->GetLogic())
    {
    this->GetLogic()->RemoveObservers(vtkCommand::ModifiedEvent,
                                      (vtkCommand *)this->LogicCallbackCommand);
    }
}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFGUI::AddGUIObservers ( )
{
  this->RemoveGUIObservers();

  // make a user interactor style to process our events
  // look at the InteractorStyle to get our events

  vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();

  //----------------------------------------------------------------
  // MRML

  vtkIntArray* events = vtkIntArray::New();
  //events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  //events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::SceneCloseEvent);
  
  if (this->GetMRMLScene() != NULL)
    {
    this->SetAndObserveMRMLSceneEvents(this->GetMRMLScene(), events);
    }
  events->Delete();

  
  //----------------------------------------------------------------
  // Main Slice GUI

  appGUI->GetMainSliceGUI("Red")->GetSliceViewer()->GetRenderWidget()
    ->GetRenderWindowInteractor()->GetInteractorStyle()
    ->AddObserver(vtkCommand::LeftButtonPressEvent, (vtkCommand *)this->GUICallbackCommand);
  appGUI->GetMainSliceGUI("Yellow")->GetSliceViewer()->GetRenderWidget()
    ->GetRenderWindowInteractor()->GetInteractorStyle()
    ->AddObserver(vtkCommand::LeftButtonPressEvent, (vtkCommand *)this->GUICallbackCommand);
  appGUI->GetMainSliceGUI("Green")->GetSliceViewer()->GetRenderWidget()
    ->GetRenderWindowInteractor()->GetInteractorStyle()
    ->AddObserver(vtkCommand::LeftButtonPressEvent, (vtkCommand *)this->GUICallbackCommand);
  

  //----------------------------------------------------------------
  // Connector Browser Frame

  //vtkKWMultiColumnListWithScrollbars* ConnectorList;
  
  this->ConnectorList->GetWidget()
    ->AddObserver(vtkKWMultiColumnList::SelectionChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->AddConnectorButton
    ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->DeleteConnectorButton
    ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);

  this->ConnectorNameEntry
    ->AddObserver(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);

  this->ConnectorTypeButtonSet->GetWidget(0)
    ->AddObserver(vtkKWRadioButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ConnectorTypeButtonSet->GetWidget(1)
    ->AddObserver(vtkKWRadioButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);

  this->ConnectorStatusCheckButton
    ->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand*)this->GUICallbackCommand);

  this->ConnectorAddressEntry
    ->AddObserver(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ConnectorPortEntry
    ->AddObserver(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  
  this->EnableAdvancedSettingButton
    ->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand*)this->GUICallbackCommand);

  this->AddDeviceNameButton
    ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->DeleteDeviceNameButton
    ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);

  // Advanced Settings
  this->MrmlNodeList->GetWidget()->
    AddObserver(vtkKWMultiColumnList::SelectionChangedEvent, (vtkCommand *)this->GUICallbackCommand);


  //----------------------------------------------------------------
  // Visualization Control Frame


  this->LocatorCheckButton
    ->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);

  this->RedSliceMenu->GetMenu()
    ->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand*)this->GUICallbackCommand);
  this->YellowSliceMenu->GetMenu()
    ->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand*)this->GUICallbackCommand);
  this->GreenSliceMenu->GetMenu()
    ->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->FreezeImageCheckButton
    ->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand*)this->GUICallbackCommand);
  this->ObliqueCheckButton
    ->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand*)this->GUICallbackCommand);

  this->SetLocatorModeButton
    ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->SetUserModeButton
    ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);

  /*
  this->StartScanButton
    ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->StopScanButton
    ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ImagingMenu->GetMenu()
    ->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  */



  //----------------------------------------------------------------
  // Etc Frame

  // observer load volume button

  this->AddLogicObservers();
  
  
}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFGUI::AddLogicObservers ( )
{
  this->RemoveLogicObservers();  

  if (this->GetLogic())
    {
    /*
      this->GetLogic()->AddObserver(vtkOpenIGTLinkIFLogic::LocatorUpdateEvent,
      (vtkCommand *)this->LogicCallbackCommand);
    */
    /*
      this->GetLogic()->AddObserver(vtkOpenIGTLinkIFLogic::SliceUpdateEvent,
      (vtkCommand *)this->LogicCallbackCommand);
    */
    this->GetLogic()->AddObserver(vtkOpenIGTLinkIFLogic::StatusUpdateEvent,
                                  (vtkCommand *)this->LogicCallbackCommand);
    }
}

//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFGUI::HandleMouseEvent(vtkSlicerInteractorStyle *style)
{

  vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
  vtkSlicerInteractorStyle *istyle0 
    = vtkSlicerInteractorStyle::SafeDownCast(appGUI->GetMainSliceGUI("Red")->GetSliceViewer()
                                             ->GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle());
  vtkSlicerInteractorStyle *istyle1 
    = vtkSlicerInteractorStyle::SafeDownCast(appGUI->GetMainSliceGUI("Yellow")->GetSliceViewer()
                                             ->GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle());
  vtkSlicerInteractorStyle *istyle2 
    = vtkSlicerInteractorStyle::SafeDownCast(appGUI->GetMainSliceGUI("Green")->GetSliceViewer()
                                             ->GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle());
  
  vtkCornerAnnotation *anno = NULL;
  if (style == istyle0)
    {
    anno = appGUI->GetMainSliceGUI("Red")->GetSliceViewer()->GetRenderWidget()->GetCornerAnnotation();
    }
  else if (style == istyle1)
    {
    anno = appGUI->GetMainSliceGUI("Yellow")->GetSliceViewer()->GetRenderWidget()->GetCornerAnnotation();
    }
  else if (style == istyle2)
    {
    anno = appGUI->GetMainSliceGUI("Green")->GetSliceViewer()->GetRenderWidget()->GetCornerAnnotation();
    }
  if (anno)
    {
    const char *rasText = anno->GetText(1);
    if ( rasText != NULL )
      {
      std::string ras = std::string(rasText);
        
      // remove "R:," "A:," and "S:" from the string
      int loc = ras.find("R:", 0);
      if ( loc != std::string::npos ) 
        {
        ras = ras.replace(loc, 2, "");
        }
      loc = ras.find("A:", 0);
      if ( loc != std::string::npos ) 
        {
        ras = ras.replace(loc, 2, "");
        }
      loc = ras.find("S:", 0);
      if ( loc != std::string::npos ) 
        {
        ras = ras.replace(loc, 2, "");
        }
      
      // remove "\n" from the string
      int found = ras.find("\n", 0);
      while ( found != std::string::npos )
        {
        ras = ras.replace(found, 1, " ");
        found = ras.find("\n", 0);
        }
      
      }
    }
}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFGUI::ProcessGUIEvents(vtkObject *caller,
                                         unsigned long event, void *callData)
{

  const char *eventName = vtkCommand::GetStringFromEventId(event);

  if (strcmp(eventName, "LeftButtonPressEvent") == 0)
    {
    vtkSlicerInteractorStyle *style = vtkSlicerInteractorStyle::SafeDownCast(caller);
    HandleMouseEvent(style);
    return;
    }

  //----------------------------------------------------------------
  // Connector Browser Frame

  else if (this->ConnectorList->GetWidget() == vtkKWMultiColumnList::SafeDownCast(caller)
           && event == vtkKWMultiColumnList::SelectionChangedEvent)
    {
    int selected = this->ConnectorList->GetWidget()->GetIndexOfFirstSelectedRow();
    UpdateConnectorPropertyFrame(selected);
    UpdateMrmlNodeListFrame(selected);
    }

  else if (this->AddConnectorButton == vtkKWPushButton::SafeDownCast(caller)
           && event == vtkKWPushButton::InvokedEvent)
    {
    this->GetLogic()->AddConnector();
    UpdateConnectorList(UPDATE_ALL);
    int select = this->ConnectorList->GetWidget()->GetNumberOfRows() - 1;
    this->ConnectorList->GetWidget()->SelectSingleRow(select);
    UpdateConnectorPropertyFrame(select);
    UpdateMrmlNodeListFrame(select);
    }

  else if (this->DeleteConnectorButton == vtkKWPushButton::SafeDownCast(caller)
           && event == vtkKWPushButton::InvokedEvent)
    {
    int selected = this->ConnectorList->GetWidget()->GetIndexOfFirstSelectedRow();
    this->GetLogic()->DeleteConnector(selected);
    UpdateConnectorList(UPDATE_ALL);
    int nrow = this->ConnectorList->GetWidget()->GetNumberOfRows();
    if (selected >= nrow)
      {
      selected = nrow - 1;
      }
    this->ConnectorList->GetWidget()->SelectSingleRow(selected);
    UpdateConnectorList(UPDATE_ALL);
    UpdateConnectorPropertyFrame(selected);
    UpdateMrmlNodeListFrame(selected);
    }
  
  else if (this->ConnectorNameEntry == vtkKWEntry::SafeDownCast(caller)
           && event == vtkKWEntry::EntryValueChangedEvent)
    {
    int selected = this->ConnectorList->GetWidget()->GetIndexOfFirstSelectedRow();
    vtkIGTLConnector* connector = this->GetLogic()->GetConnector(selected);
    if (connector)
      {
      connector->SetName(this->ConnectorNameEntry->GetValue());
      UpdateConnectorList(UPDATE_SELECTED_ONLY);
      }
    }

  else if (this->ConnectorTypeButtonSet->GetWidget(0) == vtkKWRadioButton::SafeDownCast(caller)
           && event == vtkKWRadioButton::SelectedStateChangedEvent
           && this->ConnectorTypeButtonSet->GetWidget(0)->GetSelectedState() == 1)
    {
    int selected = this->ConnectorList->GetWidget()->GetIndexOfFirstSelectedRow();
    vtkIGTLConnector* connector = this->GetLogic()->GetConnector(selected);
    if (connector)
      {
      connector->SetType(vtkIGTLConnector::TYPE_SERVER);
      UpdateConnectorList(UPDATE_SELECTED_ONLY);
      UpdateConnectorPropertyFrame(selected);
      UpdateMrmlNodeListFrame(selected);
      }
    }

  else if (this->ConnectorTypeButtonSet->GetWidget(1) == vtkKWRadioButton::SafeDownCast(caller)
           && event == vtkKWRadioButton::SelectedStateChangedEvent
           && this->ConnectorTypeButtonSet->GetWidget(1)->GetSelectedState() == 1)
    {
    int selected = this->ConnectorList->GetWidget()->GetIndexOfFirstSelectedRow();
    vtkIGTLConnector* connector = this->GetLogic()->GetConnector(selected);
    if (connector)
      {
      connector->SetType(vtkIGTLConnector::TYPE_CLIENT);
      UpdateConnectorList(UPDATE_SELECTED_ONLY);
      UpdateConnectorPropertyFrame(selected);
      UpdateMrmlNodeListFrame(selected);
      }
    }

  else if (this->ConnectorStatusCheckButton == vtkKWCheckButton::SafeDownCast(caller) 
           && event == vtkKWCheckButton::SelectedStateChangedEvent )
    {
    int selected = this->ConnectorList->GetWidget()->GetIndexOfFirstSelectedRow();
    vtkIGTLConnector* connector = this->GetLogic()->GetConnector(selected);
    if (connector)
      {
      if (this->ConnectorStatusCheckButton->GetSelectedState()) // Activated
        {
        //vtkErrorMacro("Starting Connector...........");
        connector->Start();
        UpdateConnectorList(UPDATE_SELECTED_ONLY);
        UpdateConnectorPropertyFrame(selected);
        UpdateMrmlNodeListFrame(selected);
        }
      else  // Deactivated
        {
        connector->Stop();
        //vtkErrorMacro("Connector Stopped...........");
        UpdateConnectorList(UPDATE_SELECTED_ONLY);
        UpdateConnectorPropertyFrame(selected);
        UpdateMrmlNodeListFrame(selected);
        }
      }
    }

  else if (this->ConnectorAddressEntry == vtkKWEntry::SafeDownCast(caller)
           && event == vtkKWEntry::EntryValueChangedEvent)
    {
    int selected = this->ConnectorList->GetWidget()->GetIndexOfFirstSelectedRow();
    vtkIGTLConnector* connector = this->GetLogic()->GetConnector(selected);
    if (connector)
      {
      connector->SetServerHostname(this->ConnectorAddressEntry->GetValue());
      }
    UpdateConnectorList(UPDATE_SELECTED_ONLY);
    }

  else if (this->ConnectorPortEntry == vtkKWEntry::SafeDownCast(caller)
           && event == vtkKWEntry::EntryValueChangedEvent)
    {
    int selected = this->ConnectorList->GetWidget()->GetIndexOfFirstSelectedRow();
    vtkIGTLConnector* connector = this->GetLogic()->GetConnector(selected);
    if (connector)
      {
      connector->SetServerPort(this->ConnectorPortEntry->GetValueAsInt());
      }
    UpdateConnectorList(UPDATE_SELECTED_ONLY);
    }

  else if (this->EnableAdvancedSettingButton == vtkKWCheckButton::SafeDownCast(caller)
           && event == vtkKWCheckButton::SelectedStateChangedEvent )
    {
    if (this->EnableAdvancedSettingButton->GetSelectedState())
      {
      this->GetLogic()->SetRestrictDeviceName(1);
      }
    else
      {
      this->GetLogic()->SetRestrictDeviceName(0);
      }
    }

  else if (this->AddDeviceNameButton == vtkKWPushButton::SafeDownCast(caller)
           && event == vtkKWPushButton::InvokedEvent)
    {
    int selected = this->ConnectorList->GetWidget()->GetIndexOfFirstSelectedRow();
    this->GetLogic()->AddDeviceToConnector(selected, "Unspecified", "Unspecified", 0);
    //UpdateConnectorPropertyFrame(selected);
    UpdateMrmlNodeListFrame(selected);
    }

  else if (this->DeleteDeviceNameButton == vtkKWPushButton::SafeDownCast(caller)
           && event == vtkKWPushButton::InvokedEvent)
    {
    //this->GetLogic()->AddConnector();
    //int select = this->ConnectorList->GetWidget()->GetNumberOfRows() - 1;
    //this->ConnectorList->GetWidget()->SelectSingleRow(select);
    //UpdateConnectorPropertyFrame(select);
    //UpdateMrmlNodeListFrame(select);
    }


  else if (this->MrmlNodeList->GetWidget() == vtkKWMultiColumnList::SafeDownCast(caller)
           && event == vtkKWMultiColumnList::SelectionChangedEvent)
    {
      
    int col;
    int row;
    this->MrmlNodeList->GetWidget()->GetSelectedCells(&row, &col);
    this->MrmlNodeList->GetWidget()->GetSelectedCells(&row, &col);
    /*

      this->MrmlNodeList->GetWidget()->SetCellEditWindowToEntry(0,0);
      this->MrmlNodeList->GetWidget()->SetCellEditable(0, 0, 1);
      this->MrmlNodeList->GetWidget()->EditCell(0, 0);
    */
    }


  //----------------------------------------------------------------
  // Visualization Control Frame

  // -- this should be moved to Logic?
  else if (this->LocatorCheckButton == vtkKWCheckButton::SafeDownCast(caller) 
           && event == vtkKWCheckButton::SelectedStateChangedEvent )
    {
    int checked = this->LocatorCheckButton->GetSelectedState(); 
    if (!this->CloseScene)
      {
      if (checked)
        {
        this->GetLogic()->SetVisibilityOfLocatorModel("IGTLocator", 1);
        }
      else
        {
        this->GetLogic()->SetVisibilityOfLocatorModel("IGTLocator", 0);
        }
      }
    else
      {
      this->CloseScene = false;
      }
    }

  else if (this->RedSliceMenu->GetMenu() == vtkKWMenu::SafeDownCast(caller)
           && event == vtkKWMenu::MenuItemInvokedEvent)
    {
    const char* selected = this->RedSliceMenu->GetValue();
    ChangeSlicePlaneDriver(vtkOpenIGTLinkIFGUI::SLICE_PLANE_RED, selected);
    }

  else if (this->YellowSliceMenu->GetMenu() == vtkKWMenu::SafeDownCast(caller)
           && event == vtkKWMenu::MenuItemInvokedEvent)
    {
    const char* selected = this->YellowSliceMenu->GetValue();
    ChangeSlicePlaneDriver(vtkOpenIGTLinkIFGUI::SLICE_PLANE_YELLOW, selected);
    }

  else if (this->GreenSliceMenu->GetMenu() == vtkKWMenu::SafeDownCast(caller)
           && event == vtkKWMenu::MenuItemInvokedEvent)
    {
    const char* selected = this->GreenSliceMenu->GetValue();
    ChangeSlicePlaneDriver(vtkOpenIGTLinkIFGUI::SLICE_PLANE_GREEN, selected);
    }

  // -- "Locator" button 
  else if (this->SetLocatorModeButton == vtkKWPushButton::SafeDownCast(caller) 
           && event == vtkKWPushButton::InvokedEvent)
    {
    ChangeSlicePlaneDriver(vtkOpenIGTLinkIFGUI::SLICE_PLANE_RED, "Locator");
    ChangeSlicePlaneDriver(vtkOpenIGTLinkIFGUI::SLICE_PLANE_YELLOW, "Locator");
    ChangeSlicePlaneDriver(vtkOpenIGTLinkIFGUI::SLICE_PLANE_GREEN, "Locator");
    }
  
  // -- "User" button 
  else if (this->SetUserModeButton == vtkKWPushButton::SafeDownCast(caller) 
           && event == vtkKWPushButton::InvokedEvent)
    {
    ChangeSlicePlaneDriver(vtkOpenIGTLinkIFGUI::SLICE_PLANE_RED, "User");
    ChangeSlicePlaneDriver(vtkOpenIGTLinkIFGUI::SLICE_PLANE_YELLOW, "User");
    ChangeSlicePlaneDriver(vtkOpenIGTLinkIFGUI::SLICE_PLANE_GREEN, "User");
    }
  
  // -- "Freeze Image Position" check button 
  else if (this->FreezeImageCheckButton == vtkKWCheckButton::SafeDownCast(caller) 
           && event == vtkKWCheckButton::SelectedStateChangedEvent )
    {
    if (this->FreezeImageCheckButton->GetSelectedState() == 1)
      {
      this->GetLogic()->SetFreezePlane(true);
      }
    else
      {
      this->GetLogic()->SetFreezePlane(false);
      }
    }
  else if (this->ObliqueCheckButton == vtkKWCheckButton::SafeDownCast(caller) 
           && event == vtkKWCheckButton::SelectedStateChangedEvent )
    {
    if (this->ObliqueCheckButton->GetSelectedState() == 1)
      {
      this->GetLogic()->SetEnableOblique(true);
      }
    else
      {
      this->GetLogic()->SetEnableOblique(false);
      }
    }

  /*
  else if (this->ImagingMenu->GetMenu() == vtkKWMenu::SafeDownCast(caller)
           && event == vtkKWMenu::MenuItemInvokedEvent )
    {
      
    const char* selected = this->ImagingMenu->GetValue();
    if (strcmp(selected, "None") == 0)
      {
      //this->RealtimeImageOrient = vtkOpenIGTLinkIFGUI::SLICE_RTIMAGE_NONE;
      //this->GetLogic()->SetRealtimeImageOrient(vtkOpenIGTLinkIFLogic::SLICE_RTIMAGE_NONE);
      }
    else if (strcmp(selected, "Perpendicular") == 0)
      {
      //this->RealtimeImageOrient = vtkOpenIGTLinkIFGUI::SLICE_RTIMAGE_PERP;
      //this->GetLogic()->SetRealtimeImageOrient(vtkOpenIGTLinkIFLogic::SLICE_RTIMAGE_PERP);
      }
    else if (strcmp(selected, "In-plane 90") == 0)
      {
      //this->RealtimeImageOrient = vtkOpenIGTLinkIFGUI::SLICE_RTIMAGE_INPLANE90;
      //this->GetLogic()->SetRealtimeImageOrient(vtkOpenIGTLinkIFLogic::SLICE_RTIMAGE_INPLANE90);
      }
    else //if ( strcmp(selected, "In-plane") == 0 )
      {
      //this->RealtimeImageOrient = vtkOpenIGTLinkIFGUI::SLICE_RTIMAGE_INPLANE;
      //this->GetLogic()->SetRealtimeImageOrient(vtkOpenIGTLinkIFLogic::SLICE_RTIMAGE_INPLANE);
      }
    
    }
  */

  /*
  else if (this->StartScanButton == vtkKWPushButton::SafeDownCast(caller) 
           && event == vtkKWPushButton::InvokedEvent)
    {
    //this->Logic->ScanStart();
    }
  else if (this->StopScanButton == vtkKWPushButton::SafeDownCast(caller) 
           && event == vtkKWPushButton::InvokedEvent)
    {
    //this->Logic->ScanStop();
    }
  */

  //----------------------------------------------------------------
  // Etc Frame

          

} 


void vtkOpenIGTLinkIFGUI::Init()
{
  this->DataManager->SetMRMLScene(this->GetMRMLScene());
}



void vtkOpenIGTLinkIFGUI::DataCallback(vtkObject *caller, 
                                     unsigned long eid, void *clientData, void *callData)
{
  vtkOpenIGTLinkIFGUI *self = reinterpret_cast<vtkOpenIGTLinkIFGUI *>(clientData);
  vtkDebugWithObjectMacro(self, "In vtkOpenIGTLinkIFGUI DataCallback");

  self->UpdateAll();
}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFGUI::ProcessLogicEvents ( vtkObject *caller,
                                             unsigned long event, void *callData )
{

  if (this->GetLogic() == vtkOpenIGTLinkIFLogic::SafeDownCast(caller))
    {
    if (event == vtkOpenIGTLinkIFLogic::StatusUpdateEvent)
      {
      //this->UpdateDeviceStatus();
      }
    }
}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFGUI::ProcessMRMLEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
{
  // Fill in

  if (event == vtkMRMLScene::SceneCloseEvent)
    {
    if (this->LocatorCheckButton != NULL && this->LocatorCheckButton->GetSelectedState())
      {
      this->CloseScene = true;
      this->LocatorCheckButton->SelectedStateOff();
      }
    }
}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFGUI::ProcessTimerEvents()
{
  if (this->TimerFlag)
    {
    // -----------------------------------------
    // Check connector status

    if (this->GetLogic()->CheckConnectorsStatusUpdates())
      {
      int selected = this->ConnectorList->GetWidget()->GetIndexOfFirstSelectedRow();
      UpdateConnectorList(UPDATE_STATUS_ALL);
      UpdateConnectorPropertyFrame(selected);
      UpdateMrmlNodeListFrame(selected);
      }


    // -----------------------------------------
    // Check incomming new data

    this->GetLogic()->ImportFromCircularBuffers();

    vtkKWTkUtilities::CreateTimerHandler(vtkKWApplication::GetMainInterp(), 
                                         this->TimerInterval,
                                         this, "ProcessTimerEvents");        
    }
}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFGUI::Enter()
{
  // Fill in
  vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
  
  this->SliceNode0 = appGUI->GetMainSliceGUI("Red")->GetLogic()->GetSliceNode();
  this->SliceNode1 = appGUI->GetMainSliceGUI("Yellow")->GetLogic()->GetSliceNode();
  this->SliceNode2 = appGUI->GetMainSliceGUI("Green")->GetLogic()->GetSliceNode();
  
  //this->GetLogic()->AddRealtimeVolumeNode("Realtime");
  //this->Logic0->GetForegroundLayer()->SetUseReslice(0);
  
  if (this->TimerFlag == 0)
    {
    this->TimerFlag = 1;
    this->TimerInterval = 100;  // 100 ms
    ProcessTimerEvents();
    }

  this->GetLogic()->Initialize();
  this->UpdateConnectorList(UPDATE_ALL);

  /*
  //----------------------------------------------------------------
  // Hack SlicerSlicesControlGUI -- Add "InPlane", "InPlane90", "Perp"

  if (this->IsSliceOrientationAdded == false && this->GetApplication() )
    {
    vtkSlicerSliceGUI* sgui;
    vtkSlicerSlicesGUI* ssgui;
    vtkSlicerApplication* app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
    ssgui = vtkSlicerSlicesGUI::SafeDownCast(app->GetModuleGUIByName ("Slices") );

    if (ssgui != NULL)
      {
      ssgui->GetSliceGUICollection()->InitTraversal();
      sgui = vtkSlicerSliceGUI::SafeDownCast(ssgui->GetSliceGUICollection()->GetNextItemAsObject());
      while ( sgui != NULL )
        {
        vtkSlicerSliceControllerWidget* sscw = sgui->GetSliceController();
        vtkKWMenuButtonWithSpinButtonsWithLabel* oriSel = sscw->GetOrientationSelector();
        vtkKWMenuButton *mb = oriSel->GetWidget()->GetWidget();
        mb->GetMenu()->AddRadioButton("InPlane");
        mb->GetMenu()->AddRadioButton("InPlane90");
        mb->GetMenu()->AddRadioButton("Perp");
        sgui = vtkSlicerSliceGUI::SafeDownCast ( ssgui->GetSliceGUICollection()->GetNextItemAsObject() );
        }
      }
    this->IsSliceOrientationAdded = true;
    }
  */

}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFGUI::Exit ( )
{
  // Fill in
}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFGUI::BuildGUI ( )
{

  // ---
  // MODULE GUI FRAME 
  // create a page
  this->UIPanel->AddPage ( "OpenIGTLink", "OpenIGTLink", NULL );

  BuildGUIForHelpFrame();
  BuildGUIForConnectorBrowserFrame();
  BuildGUIForVisualizationControlFrame();

  UpdateConnectorPropertyFrame(-1);
  UpdateMrmlNodeListFrame(-1);
}


void vtkOpenIGTLinkIFGUI::BuildGUIForHelpFrame ()
{

  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "OpenIGTLink" );

  // Define your help text here.
  const char *help = 
    "The **OpenIGTLink Module** helps you to manage OpenIGTLink connections:"
    " OpenIGTLink is an open network protocol for communication among software / hardware "
    " for image-guided therapy, e.g. robot-navigation and imager-viewer connections."
    " The information of the OpenIGTLink protocol can be found at http://wiki.na-mic.org/Wiki/index.php/OpenIGTLink ."
    " The module is designed and implemented by Junichi Tokuda for Brigham and Women's Hospital."
    " This work is supported by NCIGT, NA-MIC and BRP Prostate robot project.";

  // ----------------------------------------------------------------
  // HELP FRAME         
  // ----------------------------------------------------------------
  vtkSlicerModuleCollapsibleFrame *OpenIGTLinkHelpFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  OpenIGTLinkHelpFrame->SetParent ( page );
  OpenIGTLinkHelpFrame->Create ( );
  OpenIGTLinkHelpFrame->CollapseFrame ( );
  OpenIGTLinkHelpFrame->SetLabelText ("Help");
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                OpenIGTLinkHelpFrame->GetWidgetName(), page->GetWidgetName());
    
  // configure the parent classes help text widget
  this->HelpText->SetParent ( OpenIGTLinkHelpFrame->GetFrame() );
  this->HelpText->Create ( );
  this->HelpText->SetHorizontalScrollbarVisibility ( 0 );
  this->HelpText->SetVerticalScrollbarVisibility ( 1 );
  this->HelpText->GetWidget()->SetText ( help );
  this->HelpText->GetWidget()->SetReliefToFlat ( );
  this->HelpText->GetWidget()->SetWrapToWord ( );
  this->HelpText->GetWidget()->ReadOnlyOn ( );
  this->HelpText->GetWidget()->QuickFormattingOn ( );
  this->HelpText->GetWidget()->SetBalloonHelpString ( "" );
  app->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 4",
                this->HelpText->GetWidgetName ( ) );

  OpenIGTLinkHelpFrame->Delete();

}

//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFGUI::BuildGUIForConnectorBrowserFrame ()
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkKWWidget *page = this->UIPanel->GetPageWidget ("OpenIGTLink");
  
  vtkSlicerModuleCollapsibleFrame *conBrowsFrame = vtkSlicerModuleCollapsibleFrame::New();

  conBrowsFrame->SetParent(page);
  conBrowsFrame->Create();
  conBrowsFrame->SetLabelText("Connector Browser");
  conBrowsFrame->CollapseFrame();
  app->Script ("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
               conBrowsFrame->GetWidgetName(), page->GetWidgetName());

  // -----------------------------------------
  // Connector List Frame
  
  vtkKWFrameWithLabel *listFrame = vtkKWFrameWithLabel::New();
  listFrame->SetParent(conBrowsFrame->GetFrame());
  listFrame->Create();
  listFrame->SetLabelText ("Connectors");
  app->Script ( "pack %s -fill both -expand true",  
                listFrame->GetWidgetName());

  this->ConnectorList = vtkKWMultiColumnListWithScrollbars::New();
  this->ConnectorList->SetParent(listFrame->GetFrame());
  this->ConnectorList->Create();
  this->ConnectorList->SetHeight(1);
  this->ConnectorList->GetWidget()->SetSelectionTypeToRow();
  this->ConnectorList->GetWidget()->SetSelectionModeToSingle();
  this->ConnectorList->GetWidget()->MovableRowsOff();
  this->ConnectorList->GetWidget()->MovableColumnsOff();

  const char* labels[] =
    { "Name", "Type", "Status", "Destination"};
  const int widths[] = 
    { 10, 5, 7, 20 };

  for (int col = 0; col < 4; col ++)
    {
    this->ConnectorList->GetWidget()->AddColumn(labels[col]);
    this->ConnectorList->GetWidget()->SetColumnWidth(col, widths[col]);
    this->ConnectorList->GetWidget()->SetColumnAlignmentToLeft(col);
    this->ConnectorList->GetWidget()->ColumnEditableOff(col);
    //this->ConnectorList->GetWidget()->ColumnEditableOn(col);
    this->ConnectorList->GetWidget()->SetColumnEditWindowToSpinBox(col);
    }
  this->ConnectorList->GetWidget()->SetColumnEditWindowToCheckButton(0);
  //this->ConnectorList->GetWidget()->SetCellUpdatedCommand(this, "OnConnectorListUpdate");
  //this->ConnectorList->GetWidget()->SetSelectionChangedCommand(this, "OnConnectorListSelectionChanged");
  
  vtkKWFrame *listButtonsFrame = vtkKWFrame::New();
  listButtonsFrame->SetParent(listFrame->GetFrame());
  listButtonsFrame->Create();

  app->Script ("pack %s %s -fill both -expand true",  
               //app->Script( "pack %s %s -side left -anchor nw -expand n -padx 2 -pady 2",
               this->ConnectorList->GetWidgetName(), listButtonsFrame->GetWidgetName());

  this->AddConnectorButton = vtkKWPushButton::New();
  this->AddConnectorButton->SetParent(listButtonsFrame);
  this->AddConnectorButton->Create();
  this->AddConnectorButton->SetText( "Add" );
  this->AddConnectorButton->SetWidth (6);

  this->DeleteConnectorButton = vtkKWPushButton::New();
  this->DeleteConnectorButton->SetParent(listButtonsFrame);
  this->DeleteConnectorButton->Create();
  this->DeleteConnectorButton->SetText( "Delete" );
  this->DeleteConnectorButton->SetWidth (6);

  app->Script( "pack %s %s -side left -anchor nw -expand n -padx 2 -pady 2",
               this->AddConnectorButton->GetWidgetName(), this->DeleteConnectorButton->GetWidgetName());

  // -----------------------------------------
  // Connector Property frame

  vtkKWFrameWithLabel *controlFrame = vtkKWFrameWithLabel::New();
  controlFrame->SetParent(conBrowsFrame->GetFrame());
  controlFrame->Create();
  controlFrame->SetLabelText ("Connector Property");
  app->Script ( "pack %s -fill both -expand true",  
                controlFrame->GetWidgetName());


  // Connector Property -- Connector name
  vtkKWFrame *nameFrame = vtkKWFrame::New();
  nameFrame->SetParent(controlFrame->GetFrame());
  nameFrame->Create();
  app->Script ( "pack %s -fill both -expand true",  
                nameFrame->GetWidgetName());

  vtkKWLabel *nameLabel = vtkKWLabel::New();
  nameLabel->SetParent(nameFrame);
  nameLabel->Create();
  nameLabel->SetWidth(8);
  nameLabel->SetText("Name: ");

  this->ConnectorNameEntry = vtkKWEntry::New();
  this->ConnectorNameEntry->SetParent(nameFrame);
  this->ConnectorNameEntry->Create();
  this->ConnectorNameEntry->SetWidth(18);

  app->Script("pack %s %s -side left -anchor w -fill x -padx 2 -pady 2", 
              nameLabel->GetWidgetName() , this->ConnectorNameEntry->GetWidgetName());
  

  // Connector Property -- Connector type (server or client)
  vtkKWFrame *typeFrame = vtkKWFrame::New();
  typeFrame->SetParent(controlFrame->GetFrame());
  typeFrame->Create();
  app->Script ( "pack %s -fill both -expand true",  
                typeFrame->GetWidgetName());

  vtkKWLabel *typeLabel = vtkKWLabel::New();
  typeLabel->SetParent(typeFrame);
  typeLabel->Create();
  typeLabel->SetWidth(8);
  typeLabel->SetText("Type: ");

  this->ConnectorTypeButtonSet = vtkKWRadioButtonSet::New();
  this->ConnectorTypeButtonSet->SetParent(typeFrame);
  this->ConnectorTypeButtonSet->Create();
  this->ConnectorTypeButtonSet->PackHorizontallyOn();
  this->ConnectorTypeButtonSet->SetMaximumNumberOfWidgetsInPackingDirection(2);
  this->ConnectorTypeButtonSet->UniformColumnsOn();
  this->ConnectorTypeButtonSet->UniformRowsOn();

  this->ConnectorTypeButtonSet->AddWidget(0);
  this->ConnectorTypeButtonSet->GetWidget(0)->SetText("Server");
  this->ConnectorTypeButtonSet->AddWidget(1);
  this->ConnectorTypeButtonSet->GetWidget(1)->SetText("Client");
  
  app->Script("pack %s %s -side left -anchor w -fill x -padx 2 -pady 2", 
              typeLabel->GetWidgetName() , this->ConnectorTypeButtonSet->GetWidgetName());

  // Connector Property -- Connector type (server or client)
  vtkKWFrame *statusFrame = vtkKWFrame::New();
  statusFrame->SetParent(controlFrame->GetFrame());
  statusFrame->Create();
  app->Script ( "pack %s -fill both -expand true",  
                statusFrame->GetWidgetName());
  
  vtkKWLabel *statusLabel = vtkKWLabel::New();
  statusLabel->SetParent(statusFrame);
  statusLabel->Create();
  statusLabel->SetWidth(8);
  statusLabel->SetText("Status: ");

  this->ConnectorStatusCheckButton = vtkKWCheckButton::New();
  this->ConnectorStatusCheckButton->SetParent(statusFrame);
  this->ConnectorStatusCheckButton->Create();
  this->ConnectorStatusCheckButton->SelectedStateOff();
  this->ConnectorStatusCheckButton->SetText("Active");
  
  app->Script("pack %s %s -side left -anchor w -fill x -padx 2 -pady 2", 
              statusLabel->GetWidgetName() , this->ConnectorStatusCheckButton->GetWidgetName());

  vtkKWFrame *addressFrame = vtkKWFrame::New();
  addressFrame->SetParent(controlFrame->GetFrame());
  addressFrame->Create();
  app->Script ( "pack %s -fill both -expand true",  
                addressFrame->GetWidgetName());

  vtkKWLabel *addressLabel = vtkKWLabel::New();
  addressLabel->SetParent(addressFrame);
  addressLabel->Create();
  addressLabel->SetWidth(8);
  addressLabel->SetText("Addr: ");

  this->ConnectorAddressEntry = vtkKWEntry::New();
  this->ConnectorAddressEntry->SetParent(addressFrame);
  this->ConnectorAddressEntry->Create();
  this->ConnectorAddressEntry->SetWidth(18);

  app->Script("pack %s %s -side left -anchor w -fill x -padx 2 -pady 2", 
              addressLabel->GetWidgetName() , this->ConnectorAddressEntry->GetWidgetName());
  

  vtkKWFrame *portFrame = vtkKWFrame::New();
  portFrame->SetParent(controlFrame->GetFrame());
  portFrame->Create();
  app->Script ( "pack %s -fill both -expand true",  
                portFrame->GetWidgetName());

  vtkKWLabel *portLabel = vtkKWLabel::New();
  portLabel->SetParent(portFrame);
  portLabel->Create();
  portLabel->SetWidth(8);
  portLabel->SetText("Port: ");

  this->ConnectorPortEntry = vtkKWEntry::New();
  this->ConnectorPortEntry->SetParent(portFrame);
  this->ConnectorPortEntry->SetRestrictValueToInteger();
  this->ConnectorPortEntry->Create();
  this->ConnectorPortEntry->SetWidth(8);

  app->Script("pack %s %s -side left -anchor w -fill x -padx 2 -pady 2", 
              portLabel->GetWidgetName() , this->ConnectorPortEntry->GetWidgetName());


  // -----------------------------------------
  // Advanced Setting Frame

  vtkKWFrameWithLabel *advancedSettingFrame = vtkKWFrameWithLabel::New();
  advancedSettingFrame->SetParent(conBrowsFrame->GetFrame());
  advancedSettingFrame->Create();
  advancedSettingFrame->SetLabelText ("Advanced Settings");
  app->Script ( "pack %s -fill both -expand true",  
                advancedSettingFrame->GetWidgetName());

  vtkKWFrame *enableASFrame = vtkKWFrame::New();
  enableASFrame->SetParent(advancedSettingFrame->GetFrame());
  enableASFrame->Create();
  app->Script ( "pack %s -fill both -expand true",  
                enableASFrame->GetWidgetName());
  
  vtkKWLabel *enableASLabel = vtkKWLabel::New();
  enableASLabel->SetParent(enableASFrame);
  enableASLabel->Create();
  enableASLabel->SetWidth(20);
  enableASLabel->SetText("Restrict Device Name: ");

  this->EnableAdvancedSettingButton = vtkKWCheckButton::New();
  this->EnableAdvancedSettingButton->SetParent(enableASFrame);
  this->EnableAdvancedSettingButton->Create();
  this->EnableAdvancedSettingButton->SelectedStateOff();
  this->EnableAdvancedSettingButton->SetText("Enabled");
  
  app->Script("pack %s %s -side left -anchor w -fill x -padx 2 -pady 2", 
              enableASLabel->GetWidgetName() , this->EnableAdvancedSettingButton->GetWidgetName());

  this->MrmlNodeList = vtkKWMultiColumnListWithScrollbars::New();
  this->MrmlNodeList->SetParent(advancedSettingFrame->GetFrame());
  this->MrmlNodeList->Create();
  this->MrmlNodeList->SetHeight(1);
  this->MrmlNodeList->GetWidget()->SetSelectionTypeToRow();
  //this->MrmlNodeList->GetWidget()->SetSelectionTypeToCell();
  this->MrmlNodeList->GetWidget()->SetSelectionModeToSingle();
  this->MrmlNodeList->GetWidget()->MovableRowsOff();
  this->MrmlNodeList->GetWidget()->MovableColumnsOff();
  //this->MrmlNodeList->GetWidget()->SetReliefToFlat();

  const char* mrmllistlabels[] =
    { "Name (Type)", "IO"};
  const int mrmllistwidths[] = 
    { 30, 10};

  for (int col = 0; col < 2; col ++)
    {
    this->MrmlNodeList->GetWidget()->AddColumn(mrmllistlabels[col]);
    this->MrmlNodeList->GetWidget()->SetColumnWidth(col, mrmllistwidths[col]);
    this->MrmlNodeList->GetWidget()->SetColumnAlignmentToLeft(col);

    this->MrmlNodeList->GetWidget()->SetColumnLabelAlignmentToLeft(col);
    this->MrmlNodeList->GetWidget()->SetColumnFormatCommandToEmptyOutput(col);
    this->MrmlNodeList->GetWidget()->SetColumnAlignmentToCenter(col);
    //this->MrmlNodeList->GetWidget()->ColumnEditableOff(col);
    //this->ConnectorList->GetWidget()->SetColumnEditWindowToEntry(col);
    //this->ConnectorList->GetWidget()->ColumnEditableOn(col);
    }

  // device type is not editable at this moment.
  //this->MrmlNodeList->GetWidget()->ColumnEditableOff(1);

  //this->MrmlNodeList->GetWidget()->SetColumnEditWindowToCheckButton(0);

  
  //  vtkKWFrame *listButtonsFrame = vtkKWFrame::New();
  //  listButtonsFrame->SetParent(listFrame->GetFrame());
  //  listButtonsFrame->Create();
  //
  //  app->Script ("pack %s %s -fill both -expand true",  
  //               //app->Script( "pack %s %s -side left -anchor nw -expand n -padx 2 -pady 2",
  //               this->ConnectorList->GetWidgetName(), listButtonsFrame->GetWidgetName());


  vtkKWFrame *deviceNameListButtonsFrame = vtkKWFrame::New();
  deviceNameListButtonsFrame->SetParent(advancedSettingFrame->GetFrame());
  deviceNameListButtonsFrame->Create();

  app->Script ("pack %s %s %s -fill both -expand true",  
               //app->Script( "pack %s %s -side left -anchor nw -expand n -padx 2 -pady 2",
               enableASFrame->GetWidgetName(),
               this->MrmlNodeList->GetWidgetName(), 
               deviceNameListButtonsFrame->GetWidgetName());

  this->AddDeviceNameButton = vtkKWPushButton::New();
  this->AddDeviceNameButton->SetParent(deviceNameListButtonsFrame);
  this->AddDeviceNameButton->Create();
  this->AddDeviceNameButton->SetText( "Add" );
  this->AddDeviceNameButton->SetWidth (6);

  this->DeleteDeviceNameButton = vtkKWPushButton::New();
  this->DeleteDeviceNameButton->SetParent(deviceNameListButtonsFrame);
  this->DeleteDeviceNameButton->Create();
  this->DeleteDeviceNameButton->SetText( "Delete" );
  this->DeleteDeviceNameButton->SetWidth (6);

  /*
  this->EditDeviceNameButton = vtkKWPushButton::New();
  this->EditDeviceNameButton->SetParent(deviceNameListButtonsFrame);
  this->EditDeviceNameButton->Create();
  this->EditDeviceNameButton->SetText( "Edit" );
  this->EditDeviceNameButton->SetWidth (6);
  */

  app->Script( "pack %s %s -side left -anchor nw -expand n -padx 2 -pady 2",
               this->AddDeviceNameButton->GetWidgetName(), this->DeleteDeviceNameButton->GetWidgetName());

}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFGUI::BuildGUIForVisualizationControlFrame ()
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkKWWidget *page = this->UIPanel->GetPageWidget ("OpenIGTLink");
  
  vtkSlicerModuleCollapsibleFrame *visCtrlFrame = vtkSlicerModuleCollapsibleFrame::New();
  visCtrlFrame->SetParent(page);
  visCtrlFrame->Create();
  visCtrlFrame->SetLabelText("Visualization / Slice Control");
  visCtrlFrame->CollapseFrame();
  app->Script ("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
               visCtrlFrame->GetWidgetName(), page->GetWidgetName());

  // -----------------------------------------
  // Locator Display frame: Options to locator display 

  vtkKWFrameWithLabel *displayFrame = vtkKWFrameWithLabel::New ( );
  displayFrame->SetParent(visCtrlFrame->GetFrame());
  displayFrame->Create();
  displayFrame->SetLabelText("Locator Display");
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
               displayFrame->GetWidgetName());
  
  this->LocatorCheckButton = vtkKWCheckButton::New();
  this->LocatorCheckButton->SetParent(displayFrame->GetFrame());
  this->LocatorCheckButton->Create();
  this->LocatorCheckButton->SelectedStateOff();
  this->LocatorCheckButton->SetText("Show Locator");
  
  this->Script("pack %s -side left -anchor w -padx 2 -pady 2", 
               this->LocatorCheckButton->GetWidgetName());
  

  // -----------------------------------------
  // Driver frame: Locator can drive slices 


  vtkKWFrameWithLabel *driverFrame = vtkKWFrameWithLabel::New();
  driverFrame->SetParent(visCtrlFrame->GetFrame());
  driverFrame->Create();
  driverFrame->SetLabelText ("Driver");
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
               driverFrame->GetWidgetName());
  
  // slice frame
  vtkKWFrame *sliceFrame = vtkKWFrame::New();
  sliceFrame->SetParent(driverFrame->GetFrame());
  sliceFrame->Create();
  app->Script("pack %s -side top -anchor nw -fill x -pady 1 -in %s",
              sliceFrame->GetWidgetName(),
              driverFrame->GetFrame()->GetWidgetName());
  

  // Contents in slice frame 
  vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );
  
  this->RedSliceMenu = vtkKWMenuButton::New();
  this->RedSliceMenu->SetParent(sliceFrame);
  this->RedSliceMenu->Create();
  this->RedSliceMenu->SetWidth(10);
  this->RedSliceMenu->SetBackgroundColor(color->SliceGUIRed);
  this->RedSliceMenu->SetActiveBackgroundColor(color->SliceGUIRed);
  this->RedSliceMenu->GetMenu()->AddRadioButton ("User");
  this->RedSliceMenu->GetMenu()->AddRadioButton ("Locator");
  this->RedSliceMenu->GetMenu()->AddRadioButton ("RT Image");
  this->RedSliceMenu->SetValue ("User");
  
  this->YellowSliceMenu = vtkKWMenuButton::New();
  this->YellowSliceMenu->SetParent(sliceFrame);
  this->YellowSliceMenu->Create();
  this->YellowSliceMenu->SetWidth(10);
  this->YellowSliceMenu->SetBackgroundColor(color->SliceGUIYellow);
  this->YellowSliceMenu->SetActiveBackgroundColor(color->SliceGUIYellow);
  this->YellowSliceMenu->GetMenu()->AddRadioButton ("User");
  this->YellowSliceMenu->GetMenu()->AddRadioButton ("Locator");
  this->YellowSliceMenu->GetMenu()->AddRadioButton ("RT Image");
  this->YellowSliceMenu->SetValue ("User");
  
  this->GreenSliceMenu = vtkKWMenuButton::New();
  this->GreenSliceMenu->SetParent(sliceFrame);
  this->GreenSliceMenu->Create();
  this->GreenSliceMenu->SetWidth(10);
  this->GreenSliceMenu->SetBackgroundColor(color->SliceGUIGreen);
  this->GreenSliceMenu->SetActiveBackgroundColor(color->SliceGUIGreen);
  this->GreenSliceMenu->GetMenu()->AddRadioButton ("User");
  this->GreenSliceMenu->GetMenu()->AddRadioButton ("Locator");
  this->GreenSliceMenu->GetMenu()->AddRadioButton ("RT Image");
  this->GreenSliceMenu->SetValue ("User");
  
  this->Script("pack %s %s %s -side left -anchor w -padx 2 -pady 2", 
               this->RedSliceMenu->GetWidgetName(),
               this->YellowSliceMenu->GetWidgetName(),
               this->GreenSliceMenu->GetWidgetName());
  
  // Mode frame
  vtkKWFrame *modeFrame = vtkKWFrame::New();
  modeFrame->SetParent ( driverFrame->GetFrame() );
  modeFrame->Create ( );
  app->Script ("pack %s -side top -anchor nw -fill x -pady 1 -in %s",
               modeFrame->GetWidgetName(),
               driverFrame->GetFrame()->GetWidgetName());
  
  // "Locator All" button
  this->SetLocatorModeButton = vtkKWPushButton::New ( );
  this->SetLocatorModeButton->SetParent ( modeFrame );
  this->SetLocatorModeButton->Create ( );
  this->SetLocatorModeButton->SetText ("Locator All");
  this->SetLocatorModeButton->SetWidth (12);
  
  // "User All" button
  this->SetUserModeButton = vtkKWPushButton::New ( );
  this->SetUserModeButton->SetParent ( modeFrame );
  this->SetUserModeButton->Create ( );
  this->SetUserModeButton->SetText ("User All");
  this->SetUserModeButton->SetWidth (12);
  
  
  // "Freeze" check button
  this->FreezeImageCheckButton = vtkKWCheckButton::New();
  this->FreezeImageCheckButton->SetParent(modeFrame);
  this->FreezeImageCheckButton->Create();
  this->FreezeImageCheckButton->SelectedStateOff();
  this->FreezeImageCheckButton->SetText("Freeze");
  
  this->ObliqueCheckButton = vtkKWCheckButton::New();
  this->ObliqueCheckButton->SetParent(modeFrame);
  this->ObliqueCheckButton->Create();
  this->ObliqueCheckButton->SelectedStateOff();
  this->ObliqueCheckButton->SetText("Orient");

  this->Script("pack %s %s %s %s -side left -anchor w -padx 2 -pady 2", 
               this->SetLocatorModeButton->GetWidgetName(),
               this->SetUserModeButton->GetWidgetName(),
               this->FreezeImageCheckButton->GetWidgetName(),
               this->ObliqueCheckButton->GetWidgetName());


  // -----------------------------------------
  // Real-time imaging: Scanner controled

  /*
  vtkKWFrameWithLabel *rtImageFrame = vtkKWFrameWithLabel::New ( );
  rtImageFrame->SetParent(visCtrlFrame->GetFrame());
  rtImageFrame->Create();
  rtImageFrame->SetLabelText("Real-time Imaging");
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
               rtImageFrame->GetWidgetName());

  // Scan start/stop frame
  vtkKWFrame *scanFrame = vtkKWFrame::New();
  scanFrame->SetParent (rtImageFrame->GetFrame());
  scanFrame->Create();
  app->Script("pack %s -side top -anchor nw -fill x -pady 1 -in %s",
              scanFrame->GetWidgetName(),
              rtImageFrame->GetFrame()->GetWidgetName());
  
  this->StartScanButton = vtkKWPushButton::New();
  this->StartScanButton->SetParent(scanFrame);
  this->StartScanButton->Create();
  this->StartScanButton->SetText("Start Scan");
  this->StartScanButton->SetWidth(12);
  
  this->StopScanButton = vtkKWPushButton::New();
  this->StopScanButton->SetParent(scanFrame);
  this->StopScanButton->Create();
  this->StopScanButton->SetText("Stop Scan");
  this->StopScanButton->SetWidth(12);

  this->ImagingMenu = vtkKWMenuButton::New();
  this->ImagingMenu->SetParent(scanFrame);
  this->ImagingMenu->Create();
  this->ImagingMenu->SetWidth(10);
  this->ImagingMenu->GetMenu()->AddRadioButton ("None");
  this->ImagingMenu->GetMenu()->AddRadioButton ("Perpendicular");
  this->ImagingMenu->GetMenu()->AddRadioButton ("In-plane 90");
  this->ImagingMenu->GetMenu()->AddRadioButton ("In-plane");
  this->ImagingMenu->SetValue("None");

  this->Script("pack %s %s %s -side left -anchor w -padx 2 -pady 2", 
               StartScanButton->GetWidgetName(),
               StopScanButton->GetWidgetName(),
               ImagingMenu->GetWidgetName());

  */
  displayFrame->Delete();
  driverFrame->Delete();
  modeFrame->Delete();
  sliceFrame->Delete();
  visCtrlFrame->Delete();

}


//----------------------------------------------------------------------------
void vtkOpenIGTLinkIFGUI::UpdateAll()
{

  /*
  if (this->LocatorCheckButton->GetSelectedState())
    {
    }
  */

}


void vtkOpenIGTLinkIFGUI::ChangeSlicePlaneDriver(int slice, const char* driver)
{

  if (slice == vtkOpenIGTLinkIFGUI::SLICE_PLANE_RED)
    {
    this->RedSliceMenu->SetValue(driver);
    if (strcmp(driver, "User") == 0)
      {
      this->SliceNode0->SetOrientationToAxial();
      //this->SliceDriver0 = vtkOpenIGTLinkIFGUI::SLICE_DRIVER_USER;
      this->GetLogic()->SetSliceDriver0(vtkOpenIGTLinkIFLogic::SLICE_DRIVER_USER);
      }
    else if (strcmp(driver, "Locator") == 0)
      {
      //this->SliceDriver0 = vtkOpenIGTLinkIFGUI::SLICE_DRIVER_LOCATOR;
      this->GetLogic()->SetSliceDriver0(vtkOpenIGTLinkIFLogic::SLICE_DRIVER_LOCATOR);
      }
    else if (strcmp(driver, "RT Image") == 0)
      {
      //this->SliceDriver0 = vtkOpenIGTLinkIFGUI::SLICE_DRIVER_RTIMAGE;
      this->GetLogic()->SetSliceDriver0(vtkOpenIGTLinkIFLogic::SLICE_DRIVER_RTIMAGE);
      }
    }
  else if (slice == vtkOpenIGTLinkIFGUI::SLICE_PLANE_YELLOW)
    {
    this->YellowSliceMenu->SetValue(driver);
    if (strcmp(driver, "User") == 0)
      {
      this->SliceNode1->SetOrientationToSagittal();
      //this->SliceDriver1 = vtkOpenIGTLinkIFGUI::SLICE_DRIVER_USER;
      this->GetLogic()->SetSliceDriver1(vtkOpenIGTLinkIFLogic::SLICE_DRIVER_USER);
      }
    else if (strcmp(driver, "Locator") == 0)
      {
      //this->SliceDriver1 = vtkOpenIGTLinkIFGUI::SLICE_DRIVER_LOCATOR;
      this->GetLogic()->SetSliceDriver1(vtkOpenIGTLinkIFLogic::SLICE_DRIVER_LOCATOR);
      }
    else if (strcmp(driver, "RT Image") == 0)
      {
      //this->SliceDriver1 = vtkOpenIGTLinkIFGUI::SLICE_DRIVER_RTIMAGE;
      this->GetLogic()->SetSliceDriver1(vtkOpenIGTLinkIFLogic::SLICE_DRIVER_RTIMAGE);
      }
    }
  else //if ( slice == vtkOpenIGTLinkIFGUI::SLICE_PLANE_GREEN )
    {
    this->GreenSliceMenu->SetValue(driver);
    if (strcmp(driver, "User") == 0)
      {
      this->SliceNode2->SetOrientationToCoronal();
      //this->SliceDriver2 = vtkOpenIGTLinkIFGUI::SLICE_DRIVER_USER;
      this->GetLogic()->SetSliceDriver2(vtkOpenIGTLinkIFLogic::SLICE_DRIVER_USER);
      }
    else if (strcmp(driver, "Locator") == 0)
      {
      //this->SliceDriver2 = vtkOpenIGTLinkIFGUI::SLICE_DRIVER_LOCATOR;
      this->GetLogic()->SetSliceDriver2(vtkOpenIGTLinkIFLogic::SLICE_DRIVER_LOCATOR);
      }
    else if (strcmp(driver, "RT Image") == 0)
      {
      //this->SliceDriver2 = vtkOpenIGTLinkIFGUI::SLICE_DRIVER_RTIMAGE;
      this->GetLogic()->SetSliceDriver2(vtkOpenIGTLinkIFLogic::SLICE_DRIVER_RTIMAGE);
      }
    }

}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFGUI::UpdateConnectorList(int updateLevel)
{
  if (this->ConnectorList == NULL)
    {
    return;
    }

  //----------------------------------------------------------------
  // Change number of rows (UPDATE_ALL only)
  if (updateLevel >= UPDATE_ALL)
    {
    // Adjust number of rows
    int numRows = this->ConnectorList->GetWidget()->GetNumberOfRows();
    int numConnectors = this->GetLogic()->GetNumberOfConnectors();
    if (numRows < numConnectors)
      {
      this->ConnectorList->GetWidget()->AddRows(numConnectors-numRows);
      }
    else
      {
      int ndel = numRows-numConnectors;
      for (int i = 0; i < ndel; i ++)
        {
        this->ConnectorList->GetWidget()->DeleteRow(numConnectors);
        }
      }
    }

  int numItems = this->GetLogic()->GetNumberOfConnectors();

  //----------------------------------------------------------------
  // Update rows (UPDATE_ALL, UPDATE_PROPERTIES_ALL and UPDATE_SELECTED_ONLY)

  // Generate lisft of rows to update
  std::vector<int> updateRows;
  updateRows.clear();
  if (updateLevel != UPDATE_STATUS_ALL)
    {
    if (updateLevel >= UPDATE_PROPERTY_ALL)
      {
      for (int i = 0; i < numItems; i ++)
        {
        updateRows.push_back(i);
        }
      }
    else if (updateLevel >= UPDATE_SELECTED_ONLY)
      {
      updateRows.push_back(this->ConnectorList->GetWidget()->GetIndexOfFirstSelectedRow());
      }
    }
      
  // update each row
  std::vector<int>::iterator iter;
  for (iter = updateRows.begin(); iter != updateRows.end(); iter ++)
    {
    int i = *iter;
    vtkIGTLConnector* connector = this->GetLogic()->GetConnector(i);
    if (connector)
      {
      // Connector Name
      this->ConnectorList->GetWidget()
        ->SetCellText(i,0, connector->GetName());
          
      // Connector Type
      this->ConnectorList->GetWidget()
        ->SetCellText(i,1, vtkOpenIGTLinkIFGUI::ConnectorTypeStr[connector->GetType()]);

      // Connector Status
      this->ConnectorList->GetWidget()
        ->SetCellText(i,2, vtkOpenIGTLinkIFGUI::ConnectorStatusStr[connector->GetState()]);
          
      // Server and port information
      std::ostringstream ss;
      if (connector->GetType() == vtkIGTLConnector::TYPE_SERVER)
        {
        ss << "--.--.--.--" << ":" << connector->GetServerPort();
        }
      else if (connector->GetType() == vtkIGTLConnector::TYPE_CLIENT)
        {
        ss << connector->GetServerHostname() << ":" << connector->GetServerPort();
        }
      else
        {
        ss << "--.--.--.--" << ":" << "--";
        }
      this->ConnectorList->GetWidget()->SetCellText(i, 3, ss.str().c_str());
      }
    }

  //----------------------------------------------------------------
  // UPDATE_ALL, UPDATE_PROPERTY_ALL and UPDATE_STATUS_ALL
  if (updateLevel == UPDATE_STATUS_ALL)
    {
    for (int i = 0; i < numItems; i ++)
      {
      vtkIGTLConnector* connector = this->GetLogic()->GetConnector(i);
      if (connector)
        {
        this->ConnectorList->GetWidget()
          ->SetCellText(i,2, ConnectorStatusStr[connector->GetState()]);
        }
      }
    }
}


//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFGUI::UpdateConnectorPropertyFrame(int i)
{

  // if i < 0, all fields are deactivated.
  int numRows = this->ConnectorList->GetWidget()->GetNumberOfRows();
  
  if (i >= 0 && i >= numRows)
    {
    return;
    }

  //----------------------------------------------------------------
  // No connector is registered
  if (i < 0 || numRows <= 0)
    {
    // Deactivate everything
    // Connector Name entry
    this->ConnectorNameEntry->SetValue("");
    this->ConnectorNameEntry->EnabledOff();
    this->ConnectorNameEntry->UpdateEnableState();

    // Connector Type
    this->ConnectorTypeButtonSet->GetWidget(0)->SelectedStateOff();
    this->ConnectorTypeButtonSet->GetWidget(1)->SelectedStateOff();
    this->ConnectorTypeButtonSet->EnabledOff();
    this->ConnectorTypeButtonSet->UpdateEnableState();

    // Connector Status
    this->ConnectorStatusCheckButton->SelectedStateOff();
    this->ConnectorStatusCheckButton->EnabledOff();

    // Server Address
    this->ConnectorAddressEntry->SetValue("");
    this->ConnectorAddressEntry->EnabledOff();
    this->ConnectorAddressEntry->UpdateEnableState();

    // Server Port
    this->ConnectorPortEntry->SetValueAsInt(0);
    this->ConnectorPortEntry->EnabledOff();
    this->ConnectorPortEntry->UpdateEnableState();

    return;
    }


  //----------------------------------------------------------------
  // A connector is selected on the list

  vtkIGTLConnector* connector = this->GetLogic()->GetConnector(i);

  // Check if the connector is active
  bool activeFlag = (connector->GetState() != vtkIGTLConnector::STATE_OFF);

  // Connection Name entry
  this->ConnectorNameEntry->SetValue(connector->GetName());
  if (activeFlag)
    {
    this->ConnectorNameEntry->EnabledOff();
    }
  else
    {
    this->ConnectorNameEntry->EnabledOn();
    }
  this->ConnectorNameEntry->UpdateEnableState();

  // Connection Type (server or client)
  if (connector->GetType() == vtkIGTLConnector::TYPE_SERVER)
    {
    this->ConnectorTypeButtonSet->GetWidget(0)->SelectedStateOn();
    this->ConnectorTypeButtonSet->GetWidget(1)->SelectedStateOff();
    }
  else if (connector->GetType() == vtkIGTLConnector::TYPE_CLIENT)
    {
    this->ConnectorTypeButtonSet->GetWidget(0)->SelectedStateOff();
    this->ConnectorTypeButtonSet->GetWidget(1)->SelectedStateOn();
    }
  else // if (connector->GetType == TYPE_NOT_DEFINED)
    {
    this->ConnectorTypeButtonSet->GetWidget(0)->SelectedStateOff();
    this->ConnectorTypeButtonSet->GetWidget(1)->SelectedStateOff();
    }
  if (activeFlag)
    {
    this->ConnectorTypeButtonSet->EnabledOff();
    }
  else
    {
    this->ConnectorTypeButtonSet->EnabledOn();
    }

  this->ConnectorTypeButtonSet->UpdateEnableState();


  // Connection Status
  if (connector->GetState() == vtkIGTLConnector::STATE_OFF)
    {
    this->ConnectorStatusCheckButton->SelectedStateOff();
    }
  else
    {
    this->ConnectorStatusCheckButton->SelectedStateOn();
    }

  if (connector->GetType() == vtkIGTLConnector::TYPE_NOT_DEFINED)  
    {
    this->ConnectorStatusCheckButton->EnabledOff();
    }
  else
    {
    this->ConnectorStatusCheckButton->EnabledOn();
    }
  this->ConnectorStatusCheckButton->UpdateEnableState();


  // Connection Server Address entry
  if (connector->GetType() == vtkIGTLConnector::TYPE_SERVER)
    {
    this->ConnectorAddressEntry->SetValue("--.--.--.--");
    this->ConnectorAddressEntry->EnabledOff();
    }
  else
    {
    this->ConnectorAddressEntry->SetValue(connector->GetServerHostname());
    if (activeFlag)
      {
      this->ConnectorAddressEntry->EnabledOff();
      }
    else
      {
      this->ConnectorAddressEntry->EnabledOn();
      }
    }
  this->ConnectorAddressEntry->UpdateEnableState();

  // Connection Port entry
  this->ConnectorPortEntry->SetValueAsInt(connector->GetServerPort());
  if (activeFlag)
    {
    this->ConnectorPortEntry->EnabledOff();
    }
  else
    {
    this->ConnectorPortEntry->EnabledOn();
    }
  this->ConnectorPortEntry->UpdateEnableState();

}

//---------------------------------------------------------------------------
void vtkOpenIGTLinkIFGUI::UpdateMrmlNodeListFrame(int con)
{
  // if the connector has already chosen
  if (con == this->CurrentMrmlNodeListID)
    {
    // nothing to do
    }
  else
    {
    this->MrmlNodeList->GetWidget()->DeleteAllRows();
    }

  vtkIGTLConnector* connector = this->GetLogic()->GetConnector(con);
  if (connector == NULL)
    {
    // failed to get connector class
    return;
    }

  this->CurrentNodeListSelected.clear();

  // Get Device List for incoming data
  vtkIGTLConnector::DeviceNameList* incoming = connector->GetIncomingDeviceList();
  vtkIGTLConnector::DeviceNameList* outgoing = connector->GetOutgoingDeviceList();
  vtkIGTLConnector::DeviceNameList* unspecified = connector->GetUnspecifiedDeviceList();
  vtkIGTLConnector::DeviceNameList::iterator iter;

  // Set number of rows in the table
  int totalrows = incoming->size() + outgoing->size() + unspecified->size();
  int numRows = this->ConnectorList->GetWidget()->GetNumberOfRows();

  this->MrmlNodeList->GetWidget()->AddRows(totalrows);


  // Get list of node names from MRML scene
  this->GetLogic()->GetDeviceNamesFromMrml(this->CurrentNodeListAvailable);
  vtkStringArray* deviceNames = vtkStringArray::New();

  for (int i = 0; i < this->CurrentNodeListAvailable.size(); i ++)
    {
    char str[256];
    sprintf(str, "%s (%s)", this->CurrentNodeListAvailable[i].name.c_str(), this->CurrentNodeListAvailable[i].type.c_str());
    deviceNames->InsertValue(i, str);
    }
  
  // Add rows to the node list table.
  int row = 0;
  for (row = 0; row < totalrows; row ++) // First create unspecified rows
    {
    const char* typeStrs[] = {"TRANSFORM", "IMAGE", "POSITION"};
    const char* ioStrs[]   = {"--", "IN", "OUT"};

    char command[256];

    this->MrmlNodeList->GetWidget()->SetCellWindowCommandToComboBoxWithValuesAsArray(row, 0, deviceNames);
    sprintf(command, "OnMrmlNodeListChanged %d 0", row);
    this->MrmlNodeList->GetWidget()->GetCellWindowAsComboBox(row, 0)->SetCommand(this, command);

    this->MrmlNodeList->GetWidget()->SetCellWindowCommandToComboBoxWithValues(row, 1, 3, ioStrs);
    sprintf(command, "OnMrmlNodeListChanged %d 1", row);
    this->MrmlNodeList->GetWidget()->GetCellWindowAsComboBox(row, 1)->SetCommand(this, command);

    //this->MrmlNodeList->GetWidget()->SetCellEditable(row, 0, 1);

    }
  
  deviceNames->Delete();

  this->CurrentNodeListSelected.resize(totalrows);

  row = 0;
  char item[256];
  for (iter = incoming->begin(); iter != incoming->end(); iter ++)
    {
    sprintf(item, "%s (%s)", iter->first.c_str(), iter->second.c_str());

    this->MrmlNodeList->GetWidget()->GetCellWindowAsComboBox(row, 0)->SetValue(item);
    //this->MrmlNodeList->GetWidget()->GetCellWindowAsComboBox(row, 1)->SetValue(iter->second.c_str());
    this->MrmlNodeList->GetWidget()->GetCellWindowAsComboBox(row, 1)->SetValue("IN");

    this->CurrentNodeListSelected[row].name = std::string(iter->first.c_str());
    this->CurrentNodeListSelected[row].type = std::string(iter->second.c_str());
    this->CurrentNodeListSelected[row].io   = vtkOpenIGTLinkIFLogic::DEVICE_IN;//std::string("IN");

    row ++;
    }

  for (iter = outgoing->begin(); iter != outgoing->end(); iter ++)
    {
    sprintf(item, "%s (%s)", iter->first.c_str(), iter->second.c_str());
    this->MrmlNodeList->GetWidget()->GetCellWindowAsComboBox(row, 0)->SetValue(item);
    //this->MrmlNodeList->GetWidget()->GetCellWindowAsComboBox(row, 1)->SetValue(iter->second.c_str());
    this->MrmlNodeList->GetWidget()->GetCellWindowAsComboBox(row, 1)->SetValue("OUT");

    this->CurrentNodeListSelected[row].name = std::string(iter->first.c_str());
    this->CurrentNodeListSelected[row].type = std::string(iter->second.c_str());
    this->CurrentNodeListSelected[row].io   = vtkOpenIGTLinkIFLogic::DEVICE_OUT; //std::string("OUT");

    row ++;
    }

  for (iter = unspecified->begin(); iter != unspecified->end(); iter ++)
    {
    sprintf(item, "%s (%s)", iter->first.c_str(), iter->second.c_str());
    this->MrmlNodeList->GetWidget()->GetCellWindowAsComboBox(row, 0)->SetValue(item);
    //this->MrmlNodeList->GetWidget()->GetCellWindowAsComboBox(row, 1)->SetValue(iter->second.c_str());
    this->MrmlNodeList->GetWidget()->GetCellWindowAsComboBox(row, 1)->SetValue("--");

    this->CurrentNodeListSelected[row].name = std::string(iter->first.c_str());
    this->CurrentNodeListSelected[row].type = std::string(iter->first.c_str());
    this->CurrentNodeListSelected[row].io   = vtkOpenIGTLinkIFLogic::DEVICE_UNSPEC; //std::string("--");

    row ++;
    }

  this->CurrentMrmlNodeListID = con;
  
}

int vtkOpenIGTLinkIFGUI::OnMrmlNodeListChanged(int row, int col, const char* item)
{

  vtkIGTLConnector* connector = this->GetLogic()->GetConnector(this->CurrentMrmlNodeListID);
  if (connector == NULL)
    {
    // failed to get connector class
    return 0;
    }

  // -----------------------------------------
  // Get original node info at (row, col)

  vtkIGTLConnector::DeviceNameList* list;
  std::string& origName = this->CurrentNodeListSelected[row].name;
  std::string& origType = this->CurrentNodeListSelected[row].type;
  int origIo            = this->CurrentNodeListSelected[row].io;

  // -----------------------------------------
  // Update the outgoing / incoming node list

  if (col == 0) // Name (Type) column has been interacted
    {
    // Get current node info at (row, col)
    int index = this->MrmlNodeList->GetWidget()->GetCellWindowAsComboBox(row, 0)->GetValueIndex(item);
    std::cerr << "INDEX == " << index << std::endl;
    if (index < 0 || index >= this->CurrentNodeListAvailable.size())
      {
      // invalid index
      return 0;
      }
    std::string& currName = this->CurrentNodeListAvailable[index].name;
    std::string& currType = this->CurrentNodeListAvailable[index].type;
    
    if (origName != currName || origType != currType)
      {
      std::cerr << "node name or type is changed" << std::endl;
      this->GetLogic()->DeleteDeviceFromConnector(this->CurrentMrmlNodeListID, origName.c_str(), origType.c_str(), origIo);
      this->GetLogic()->AddDeviceToConnector(this->CurrentMrmlNodeListID, currName.c_str(), currType.c_str(), origIo);
      this->CurrentNodeListSelected[row].name = currName;
      this->CurrentNodeListSelected[row].type = currType;
      }
    }
  else // IO column has been interacted
    {
    // Get curretn IO (in integer)
    const char* iostr[] = {"--", "IN", "OUT"}; // refer vtkOpenIGTLinkIFLogic::DEVICE_* 
    int currIo;
    for (currIo = 0; currIo < 3; currIo ++)
      {
      if (strcmp(item, iostr[currIo]) == 0)
        break;
      }

    if (currIo != origIo)
      {
      this->GetLogic()->DeleteDeviceFromConnector(this->CurrentMrmlNodeListID,
                                                  origName.c_str(), origType.c_str(), origIo);
      this->GetLogic()->AddDeviceToConnector(this->CurrentMrmlNodeListID,
                                             origName.c_str(), origType.c_str(), currIo);
      this->CurrentNodeListSelected[row].io = currIo;
      }
    }
    
  return 1;
}
