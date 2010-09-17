#include <string>

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerGUILayout.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerToolbarGUI.h"
#include "vtkSlicerWindow.h"
#include "vtkSlicerModuleChooseGUI.h"
#include "vtkSlicerModuleNavigator.h"
#include "vtkSlicerTheme.h"
#include "vtkSlicerColor.h"

#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLViewNode.h"
#include "vtkMRMLLayoutNode.h"

#include "vtkKWTkUtilities.h"
#include "vtkKWWidget.h"
#include "vtkKWToolbarSet.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWRadioButton.h"
#include "vtkKWSeparator.h"
#include "vtkKWMenu.h"
#include "vtkKWEntry.h"
#include "vtkKWLabel.h"
#include "vtkKWMessageDialog.h"

//#define LIGHTBOXGUI_DEBUG

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerToolbarGUI );
vtkCxxRevisionMacro ( vtkSlicerToolbarGUI, "$Revision$");


//---------------------------------------------------------------------------
vtkSlicerToolbarGUI::vtkSlicerToolbarGUI ( )
{
  vtkKWToolbar::SetGlobalToolbarAspectToUnChanged ( );
  vtkKWToolbar::SetGlobalWidgetsAspectToUnChanged ( );

  this->SlicerToolbarIcons = vtkSlicerToolbarIcons::New ( );

  this->ModulesToolbar = vtkKWToolbar::New ( );
  this->LoadSaveToolbar = vtkKWToolbar::New ( );
  this->ViewToolbar = vtkKWToolbar::New ( );
  this->InteractionModeToolbar = vtkKWToolbar::New ( );
  this->UndoRedoToolbar = vtkKWToolbar::New ( );
  this->UtilitiesToolbar = vtkKWToolbar::New();

  this->HomeIconButton = vtkKWPushButton::New ( );
  this->DataIconButton = vtkKWPushButton::New ( );
  this->VolumeIconButton = vtkKWPushButton::New ( );
  this->ModelIconButton = vtkKWPushButton::New ( );
  this->EditorIconButton = vtkKWPushButton::New ( );
  this->EditorToolboxIconButton = vtkKWPushButton::New ( );
  this->ColorIconButton = vtkKWPushButton::New ( );
  this->FiducialsIconButton = vtkKWPushButton::New ( );
  this->MeasurementsIconButton = vtkKWPushButton::New ( );
  this->TransformIconButton = vtkKWPushButton::New ( );
  this->SaveSceneIconButton = vtkKWPushButton::New ( );
  this->LoadSceneIconButton = vtkKWMenuButton::New ( );
  this->ChooseLayoutIconMenuButton = vtkKWMenuButton::New();
  this->MousePickOptionsButton = vtkKWMenuButton::New();
  this->MousePlaceOptionsButton = vtkKWMenuButton::New();
  this->MousePickButton = vtkKWRadioButton::New();
  this->MousePlaceButton = vtkKWRadioButton::New();
  this->MouseTransformViewButton = vtkKWRadioButton::New();

  this->UndoIconButton = vtkKWPushButton::New ( );
  this->RedoIconButton = vtkKWPushButton::New ( );
  this->ModuleChooseGUI = vtkSlicerModuleChooseGUI::New ( );
  
  this->ApplicationGUI = NULL;
  this->InteractionNodeID = NULL;
  this->InteractionNode = NULL;

  this->ProcessingMRMLEvent = 0;

  this->CompareViewBoxTopLevel = NULL; //vtkKWTopLevel::New( );
  this->CompareViewBoxApplyButton = NULL; //vtkKWPushButton::New( );
  this->CompareViewLightboxRowEntry = NULL;
  this->CompareViewLightboxColumnEntry = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerToolbarGUI::~vtkSlicerToolbarGUI ( )
{

  // Remove widgets from Toolbars
  if ( this->ModulesToolbar )
    {
    this->ModulesToolbar->RemoveAllWidgets( );
    }
  if ( this->LoadSaveToolbar )
    {
    this->LoadSaveToolbar->RemoveAllWidgets ( );
    }
  if ( this->ViewToolbar )
    {
    this->ViewToolbar->RemoveAllWidgets ( );
    }
  if ( this->UndoRedoToolbar )
    {
    this->UndoRedoToolbar->RemoveAllWidgets ( );
    }
  if ( this->InteractionModeToolbar )
    {
    this->InteractionModeToolbar->RemoveAllWidgets ( );
    }
  if ( this->UtilitiesToolbar )
    {
    this->UtilitiesToolbar->RemoveAllWidgets();
    }

  if ( this->CompareViewLightboxRowEntry)
    {
    this->CompareViewLightboxRowEntry->SetParent ( NULL );
    this->CompareViewLightboxRowEntry->Delete ( );
    this->CompareViewLightboxRowEntry = NULL;    
    }
  if (this->CompareViewLightboxColumnEntry)
    {
    this->CompareViewLightboxColumnEntry->SetParent ( NULL );
    this->CompareViewLightboxColumnEntry->Delete();
    this->CompareViewLightboxColumnEntry = NULL;    
    }

  if ( this->CompareViewBoxRowEntry )
    {
    this->CompareViewBoxRowEntry->SetParent ( NULL );
    this->CompareViewBoxRowEntry->Delete();
    this->CompareViewBoxRowEntry = NULL;
    }

//  if ( this->CompareViewBoxColumnEntry )
//    {
//    this->CompareViewBoxColumnEntry->SetParent ( NULL );
//    this->CompareViewBoxColumnEntry->Delete();
//    this->CompareViewBoxColumnEntry = NULL;
//    }

  // Delete module choose gui
  if ( this->ModuleChooseGUI )
    {
    this->ModuleChooseGUI->Delete ( );
    this->ModuleChooseGUI = NULL;
    }
    

  if ( this->HomeIconButton )
    {
    this->HomeIconButton->SetParent ( NULL );
    this->HomeIconButton->Delete ( );
    this->HomeIconButton = NULL;
    }
  if ( this->DataIconButton )
    {
    this->DataIconButton->SetParent ( NULL );
    this->DataIconButton->Delete ( );
    this->DataIconButton = NULL;
    }
  if ( this->VolumeIconButton )
    {
    this->VolumeIconButton->SetParent ( NULL );
    this->VolumeIconButton->Delete ( );
    this->VolumeIconButton = NULL;
    }
  if ( this->ModelIconButton )
    {
    this->ModelIconButton->SetParent ( NULL );
    this->ModelIconButton->Delete ( );
    this->ModelIconButton = NULL;
    }
  if ( this->EditorIconButton )
    {
    this->EditorIconButton->SetParent ( NULL );
    this->EditorIconButton->Delete ( );
    this->EditorIconButton = NULL;
    }

  if ( this->EditorToolboxIconButton )
    {
    this->EditorToolboxIconButton->SetParent ( NULL );
    this->EditorToolboxIconButton->Delete ( );
    this->EditorToolboxIconButton = NULL;
    }
  if ( this->TransformIconButton )
    {
    this->TransformIconButton->SetParent ( NULL );
    this->TransformIconButton->Delete ( );
    this->TransformIconButton = NULL;
    }
  if ( this->ColorIconButton )
    {
    this->ColorIconButton->SetParent ( NULL );
    this->ColorIconButton->Delete ( );
    this->ColorIconButton = NULL;
    }
  if ( this->FiducialsIconButton )
    {
    this->FiducialsIconButton->SetParent ( NULL );
    this->FiducialsIconButton->Delete ( );
    this->FiducialsIconButton = NULL;
    }
  if ( this->MeasurementsIconButton )
    {
    this->MeasurementsIconButton->SetParent ( NULL );
    this->MeasurementsIconButton->Delete ( );
    this->MeasurementsIconButton = NULL;
    }
  if ( this->SaveSceneIconButton )
    {
    this->SaveSceneIconButton->SetParent ( NULL );
    this->SaveSceneIconButton->Delete ( );
    this->SaveSceneIconButton = NULL;
    }
  if ( this->LoadSceneIconButton )
    {
    this->LoadSceneIconButton->SetParent ( NULL );
    this->LoadSceneIconButton->Delete ( );
    this->LoadSceneIconButton = NULL;
    }
  if ( this->ChooseLayoutIconMenuButton )
    {
    this->ChooseLayoutIconMenuButton->SetParent ( NULL );
    this->ChooseLayoutIconMenuButton->Delete();
    this->ChooseLayoutIconMenuButton = NULL;    
    }
  if ( this->UndoIconButton )
    {
    this->UndoIconButton->SetParent ( NULL );
    this->UndoIconButton->Delete ( );
    this->UndoIconButton = NULL;
    }
  if ( this->RedoIconButton )
    {
    this->RedoIconButton->SetParent ( NULL );
    this->RedoIconButton->Delete ( );
    this->RedoIconButton = NULL;
    }

  // Remove the toolbars from the window's toolbar set
    vtkSlicerApplicationGUI *p = this->GetApplicationGUI ( );
    if ( p ) 
      {
      vtkSlicerWindow *win = this->ApplicationGUI->GetMainSlicerWindow();
      if ( win)
        {
        vtkKWToolbarSet *tbs = win->GetMainToolbarSet ( );
        if (tbs) 
          {
          tbs->RemoveAllToolbars ( );
          }
        }
      }
   
    // Delete the toolbars
    if ( this->ModulesToolbar )
      {
      this->ModulesToolbar->SetParent ( NULL );
      this->ModulesToolbar->Delete ( );
      this->ModulesToolbar = NULL;
      }
    if ( this->LoadSaveToolbar )
      {
      this->LoadSaveToolbar->SetParent ( NULL );
      this->LoadSaveToolbar->Delete ( );
      this->LoadSaveToolbar = NULL;
      }
    if ( this->UndoRedoToolbar )
      {
      this->UndoRedoToolbar->SetParent ( NULL );
      this->UndoRedoToolbar->Delete ( );
      this->UndoRedoToolbar = NULL;
      }
    if ( this->UtilitiesToolbar )
      {
      this->UtilitiesToolbar->SetParent ( NULL );
      this->UtilitiesToolbar->Delete();
      this->UtilitiesToolbar = NULL;
      }
    if ( this->ViewToolbar )
      {
      this->ViewToolbar->SetParent ( NULL );
      this->ViewToolbar->Delete ( );
      this->ViewToolbar = NULL;
      }
    if ( this->InteractionModeToolbar )
      {
      this->InteractionModeToolbar->SetParent ( NULL );
      this->InteractionModeToolbar->Delete ( );
      this->InteractionModeToolbar = NULL;
      }
    if ( this->MousePickOptionsButton)
      {
      this->MousePickOptionsButton->SetParent ( NULL );
      this->MousePickOptionsButton->Delete();
      this->MousePickOptionsButton = NULL;
      }
    if ( this->MousePlaceOptionsButton )
      {
      this->MousePlaceOptionsButton->SetParent ( NULL );
      this->MousePlaceOptionsButton->Delete();
      this->MousePlaceOptionsButton = NULL;      
      }
    if ( this->MousePickButton )
      {
      this->MousePickButton->SetParent ( NULL );
      this->MousePickButton->Delete();
      this->MousePickButton = NULL;
      }
    if ( this->MousePlaceButton )
      {
      this->MousePlaceButton->SetParent ( NULL );
      this->MousePlaceButton->Delete();
      this->MousePlaceButton = NULL;
      }
    if ( this->MouseTransformViewButton )
      {
      this->MouseTransformViewButton->SetParent ( NULL );
      this->MouseTransformViewButton->Delete();
      this->MouseTransformViewButton = NULL;
      }
  // Delete Toolbar Icons
  if ( this->SlicerToolbarIcons )
    {
    this->SlicerToolbarIcons->Delete ( );
    this->SlicerToolbarIcons = NULL;
    }

  if ( this->CompareViewBoxApplyButton )
    {
    this->CompareViewBoxApplyButton->SetParent ( NULL );
    this->CompareViewBoxApplyButton->Delete ( );
    this->CompareViewBoxApplyButton = NULL;
    }
  if ( this->CompareViewBoxTopLevel )
    {
    this->CompareViewBoxTopLevel->SetParent(NULL);
    this->CompareViewBoxTopLevel->Delete  ( );
    this->CompareViewBoxTopLevel = NULL;
    }

  this->SetApplicationGUI ( NULL );

}


//---------------------------------------------------------------------------
void vtkSlicerToolbarGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerToolbarGUI: " << this->GetClassName ( ) << "\n";
    //os << indent << "Logic: " << this->GetLogic ( ) << "\n";
    // print widgets?
}


//---------------------------------------------------------------------------
void vtkSlicerToolbarGUI::ReconfigureGUIFonts ( )
{
  vtkSlicerApplicationGUI *p = this->GetApplicationGUI ( );  
  // populate the application's 3DView control GUI panel
  if ( p != NULL )
    {
    if ( p->GetApplication() != NULL )
      {
      vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( p->GetApplication() );
      if ( this->GetModuleChooseGUI() != NULL )
        {
        this->GetModuleChooseGUI()->GetModulesMenuButton()->SetFont ( app->GetSlicerTheme()->GetApplicationFont1() );
        this->GetModuleChooseGUI()->GetModulesLabel()->SetFont ( app->GetSlicerTheme()->GetApplicationFont1() );
        this->GetModuleChooseGUI()->GetModulesSearchEntry()->SetFont ( app->GetSlicerTheme()->GetApplicationFont1() );
        }
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerToolbarGUI::RemoveMRMLObservers()
{
  this->SetInteractionNode ( NULL );
}

//---------------------------------------------------------------------------
void vtkSlicerToolbarGUI::AddMRMLObservers()
{
  vtkMRMLInteractionNode *interactionNode = NULL;
  if (this->ApplicationLogic != NULL)
    {
    interactionNode =  this->ApplicationLogic->GetInteractionNode();
    if (interactionNode != NULL)
      {
      this->SetInteractionNode ( interactionNode );
      }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerToolbarGUI::RemoveGUIObservers ( )
{
    // Fill in
  this->LoadSceneIconButton->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SaveSceneIconButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->HomeIconButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DataIconButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->EditorIconButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->EditorToolboxIconButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->VolumeIconButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ModelIconButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->FiducialsIconButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->MeasurementsIconButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ColorIconButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ChooseLayoutIconMenuButton->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->UndoIconButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->RedoIconButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->MousePickOptionsButton->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->MousePlaceOptionsButton->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->MousePickButton->RemoveObservers( vtkKWRadioButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->MousePlaceButton->RemoveObservers( vtkKWRadioButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->MouseTransformViewButton->RemoveObservers( vtkKWRadioButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->CompareViewBoxApplyButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->CompareViewBoxRowEntry->RemoveObservers ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->CompareViewLightboxRowEntry->RemoveObservers ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->CompareViewLightboxColumnEntry->RemoveObservers  ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  if ( this->ModuleChooseGUI != NULL )
    {
    this->ModuleChooseGUI->RemoveGUIObservers();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerToolbarGUI::AddGUIObservers ( )
{
  // Fill in
  // add observers onto the module icon buttons 
  this->LoadSceneIconButton->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SaveSceneIconButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->HomeIconButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DataIconButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->EditorIconButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->EditorToolboxIconButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->VolumeIconButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ModelIconButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->FiducialsIconButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->MeasurementsIconButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->TransformIconButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ColorIconButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    
  // view configuration icon button observers...
  this->ChooseLayoutIconMenuButton->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->UndoIconButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->RedoIconButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->MousePickOptionsButton->GetMenu()->AddObserver (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->MousePlaceOptionsButton->GetMenu()->AddObserver (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->MousePickButton->AddObserver( vtkKWRadioButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->MousePlaceButton->AddObserver( vtkKWRadioButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->MouseTransformViewButton->AddObserver( vtkKWRadioButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->CompareViewBoxApplyButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->CompareViewBoxRowEntry->AddObserver ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->CompareViewLightboxRowEntry->AddObserver ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->CompareViewLightboxColumnEntry->AddObserver  ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  if ( this->ModuleChooseGUI != NULL )
    {
    this->ModuleChooseGUI->AddGUIObservers();
    }
}


//---------------------------------------------------------------------------
void vtkSlicerToolbarGUI::SetInteractionNode ( vtkMRMLInteractionNode *node )
{

  vtkSetAndObserveMRMLObjectMacro ( this->InteractionNode, NULL);
  if ( node )
    {
    vtkIntArray *events = vtkIntArray::New();
    //TODO.
    // should we also observe InteractionModeChangedEvents??
    events->InsertNextValue ( vtkMRMLInteractionNode::TransientTimeoutEvent);
    events->InsertNextValue ( vtkMRMLInteractionNode::InteractionModeChangedEvent);
    vtkSetAndObserveMRMLNodeEventsMacro (this->InteractionNode, node, events );
    events->Delete();
    }
  return;
}


//---------------------------------------------------------------------------
void vtkSlicerToolbarGUI::ProcessGUIEvents ( vtkObject *caller,
                                          unsigned long event, void * vtkNotUsed(callData) )
{
  int val;
  if ( this->GetApplicationGUI() != NULL )
    {
    // Toolbar's parent is the main vtkSlicerApplicationGUI;
    // Toolbar events will trigger vtkSlicerAppliationGUI methods

    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( this->GetApplication() );
    if ( !app )
      {
      vtkErrorMacro ("ProcessGUIEvents: Got NULL Application" );
      return;
      }
    vtkSlicerApplicationGUI *appGUI = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));
    if (!appGUI )
      {
      vtkErrorMacro ("ProcessGUIEvents: Got NULL Application GUI" );
      return;
      }
    vtkSlicerWindow *win = appGUI->GetMainSlicerWindow();
    if ( !win )
      {
      vtkErrorMacro ("ProcessGUIEvents: Got NULL Slicer Window" );
      return;
      }
    
    vtkMRMLInteractionNode *interactionNode = NULL;
    vtkKWRadioButton *radiob = vtkKWRadioButton::SafeDownCast ( caller );
    vtkKWPushButton *pushb = vtkKWPushButton::SafeDownCast ( caller );
    vtkKWMenu *menu = vtkKWMenu::SafeDownCast ( caller );
    vtkMRMLLayoutNode *layout;    

    //--- set interactionNode
    if ( this->ApplicationLogic != NULL )
      {
      interactionNode = this->GetApplicationLogic()->GetInteractionNode();
      }
    if (interactionNode == NULL)
      {
      vtkErrorMacro("ProcessGUIEvents: no interaction node in the scene, not updating the interaction mode!");
      }
    else
      {
      this->SetInteractionNode ( interactionNode );
      }
                                   
    // Process events from top row of buttons
    if ( app == NULL )
      {
      vtkErrorMacro ( "ProcessGUIEvents: Got NULL Application" );
      return;
      }
    if ( appGUI == NULL )
      {
      vtkErrorMacro ( "ProcessGUIEvents: Got NULL ApplicationGUI" );
      return;
      }
    // Mouse mode buttons:
    if ( radiob != NULL && event == vtkKWRadioButton::SelectedStateChangedEvent )
      {
      if ( radiob == this->MousePickButton)
        {
        val = radiob->GetSelectedState();
        if ( val && interactionNode && 
             interactionNode->GetCurrentInteractionMode() != vtkMRMLInteractionNode::PickManipulate )
          {
          interactionNode->NormalizeAllMouseModes();
          interactionNode->SetLastInteractionMode ( interactionNode->GetCurrentInteractionMode() );
          interactionNode->SetCurrentInteractionMode ( vtkMRMLInteractionNode::PickManipulate );
          this->MousePickOptionsButton->SetImageToIcon ( this->SlicerToolbarIcons->GetSlicerOptionsOnIcon ( ) );
          this->MousePlaceOptionsButton->SetImageToIcon ( this->SlicerToolbarIcons->GetSlicerOptionsOffIcon ( ) );
          }
        }
      else if ( radiob == this->MouseTransformViewButton )
        {
        val = radiob->GetSelectedState();
        if ( val && interactionNode &&
             interactionNode->GetCurrentInteractionMode() != vtkMRMLInteractionNode::ViewTransform )
          {
          interactionNode->NormalizeAllMouseModes();
          interactionNode->SetLastInteractionMode ( interactionNode->GetCurrentInteractionMode() );
          interactionNode->SetCurrentInteractionMode ( vtkMRMLInteractionNode::ViewTransform );
          this->MousePickOptionsButton->SetImageToIcon ( this->SlicerToolbarIcons->GetSlicerOptionsOffIcon ( ) );
          this->MousePlaceOptionsButton->SetImageToIcon ( this->SlicerToolbarIcons->GetSlicerOptionsOffIcon ( ) );
          }
        }
      else if ( radiob == this->MousePlaceButton )
        {
        val = radiob->GetSelectedState();
        if ( val && interactionNode &&
             interactionNode->GetCurrentInteractionMode() != vtkMRMLInteractionNode::Place )
          {
          interactionNode->NormalizeAllMouseModes();
          interactionNode->SetLastInteractionMode ( interactionNode->GetCurrentInteractionMode() );
          interactionNode->SetCurrentInteractionMode ( vtkMRMLInteractionNode::Place );
          this->MousePickOptionsButton->SetImageToIcon ( this->SlicerToolbarIcons->GetSlicerOptionsOffIcon ( ) );
          this->MousePlaceOptionsButton->SetImageToIcon ( this->SlicerToolbarIcons->GetSlicerOptionsOnIcon ( ) );          
          }
        }
      }

    if ( pushb != NULL && event == vtkKWPushButton::InvokedEvent )
      {
      layout = appGUI->GetGUILayoutNode();
      if ( pushb == this->HomeIconButton )
        {
        const char *homename = app->GetHomeModule();
        vtkSlicerModuleGUI *m = app->GetModuleGUIByName(homename);
        if ( m != NULL && this->GetModuleChooseGUI() != NULL )
          {
          this->GetModuleChooseGUI()->SelectModule ( homename );
          }
        else
          {
          vtkErrorMacro ("ERROR:  no slicer module gui found for Home module '" << (homename ? homename : "null") << "'"); 
          }
        }
      else if (pushb == this->DataIconButton )
        {
        vtkSlicerModuleGUI *m = app->GetModuleGUIByName("Data");
        if ( m != NULL && this->GetModuleChooseGUI() != NULL )
          {
          this->GetModuleChooseGUI()->SelectModule ( "Data" );
          }
        else
          {
          vtkDebugMacro ("ERROR:  no slicer module gui found for Data\n");
          }
        }
      else if (pushb == this->VolumeIconButton )
        {
        vtkSlicerModuleGUI *m = app->GetModuleGUIByName("Volumes");
        if ( m != NULL && this->GetModuleChooseGUI() != NULL)
          {
          this->GetModuleChooseGUI()->SelectModule ( "Volumes" );
          }
        else
          {
          vtkDebugMacro ("ERROR:  no slicer module gui found for Volumes\n");
          vtkKWMessageDialog *message = vtkKWMessageDialog::New();
          message->SetParent ( this->VolumeIconButton->GetParent() );
          message->SetStyleToMessage();
          std::string msg = "The Volumes module is not loaded, please check View, Application Settings, Module Settings";
          message->SetText(msg.c_str());
          message->Create();
          message->Invoke();
          message->Delete();
          }
        }
      else if (pushb == this->ModelIconButton )
        {
        vtkSlicerModuleGUI *m = app->GetModuleGUIByName("Models");
        if ( m != NULL  && this->GetModuleChooseGUI() != NULL)
          {
          this->GetModuleChooseGUI()->SelectModule ( "Models" );
          }
        else
          {
          vtkDebugMacro ("ERROR:  no slicer module gui found for Models\n");
          }
        }
      else if (pushb == this->FiducialsIconButton )
        {
        vtkSlicerModuleGUI *m = app->GetModuleGUIByName("Fiducials");
        if ( m != NULL  && this->GetModuleChooseGUI() != NULL)
          {
          this->GetModuleChooseGUI()->SelectModule ( "Fiducials" );
          }
        else
          {
          vtkDebugMacro ("ERROR:  no slicer module gui found for Fiducials\n");
          }
        }
      else if (pushb == this->MeasurementsIconButton )
        {
        vtkSlicerModuleGUI *m = app->GetModuleGUIByName("Measurements");
        if ( m != NULL  && this->GetModuleChooseGUI() != NULL)
          {
          this->GetModuleChooseGUI()->SelectModule ( "Measurements" );
          }
        else
          {
          vtkDebugMacro ("ERROR:  no slicer module gui found for Measurements\n");
          }
        }
      else if (pushb == this->ColorIconButton )
        {
        vtkSlicerModuleGUI *m = app->GetModuleGUIByName("Color");
        if ( m != NULL  && this->GetModuleChooseGUI() != NULL)
          {
          this->GetModuleChooseGUI()->SelectModule ( "Color" );
          }
        else
          {
          vtkDebugMacro ("ERROR:  no slicer module gui found for Color\n");
          }
        }
      else if (pushb == this->TransformIconButton )
        {
        vtkSlicerModuleGUI *m = app->GetModuleGUIByName("Transforms");
        if ( m != NULL  && this->GetModuleChooseGUI() != NULL)
          {
          this->GetModuleChooseGUI()->SelectModule ( "Transforms" );
          }
        else
          {
          vtkDebugMacro ("ERROR:  no slicer module gui found for Transforms\n");
          }
        }
      else if (pushb == this->EditorIconButton )
        {
        vtkSlicerModuleGUI *m = app->GetModuleGUIByName("Editor");
        if ( m != NULL  && this->GetModuleChooseGUI() != NULL)
          {
          this->GetModuleChooseGUI()->SelectModule ( "Editor" );
          }
        else
          {
          vtkDebugMacro ("ERROR:  no slicer module gui found for Editor\n");
          }
        }
      else if (pushb == this->EditorToolboxIconButton )
        {
        //---
        //--- Add code to pop up Editor toolbox here
        //---
        app->Script ("::EditBox::ShowDialog");
        }

      else if (pushb == this->CompareViewBoxApplyButton) 
        {
        this->HideCompareViewCustomLayoutFrame();
        int disabledModify = layout->StartModify();
        //--- note:
        //--- if side-by-side compare view has been selected,
        //--- then number of compare view rows is used to
        //--- specify number of columns instead.
        layout->SetNumberOfCompareViewRows ( this->CompareViewBoxRowEntry->GetValueAsInt() );
        layout->SetNumberOfCompareViewLightboxRows ( this->CompareViewLightboxRowEntry->GetValueAsInt () );
        layout->SetNumberOfCompareViewLightboxColumns ( this->CompareViewLightboxColumnEntry->GetValueAsInt() );
        if ( this->ChooseLayoutIconMenuButton )
          {
          const char *whichLayout = this->ChooseLayoutIconMenuButton->GetValue();
          if (!strcmp ( whichLayout, "Side-by-side lightbox layout"))
            {
            layout->SetViewArrangement (vtkMRMLLayoutNode::SlicerLayoutSideBySideLightboxView );
            }
          else if (!strcmp ( whichLayout, "Compare layout"))
            {
            layout->SetViewArrangement (vtkMRMLLayoutNode::SlicerLayoutCompareView );
            }
          }
        else
          {
          layout->SetViewArrangement (vtkMRMLLayoutNode::SlicerLayoutCompareView );
          }
        layout->EndModify(disabledModify);
        }
      else if ( pushb == this->UndoIconButton )
        {
        appGUI->GetMRMLScene()->Undo();
        }
      else if ( pushb == this->RedoIconButton )
        {
        appGUI->GetMRMLScene()->Redo();
        }
      else if ( pushb == this->SaveSceneIconButton )
        {
        appGUI->ProcessSaveSceneAsCommand();
        }
      }


    // TODO: figure out why we can't resume view rock or spin.
    if ( menu != NULL && event == vtkKWMenu::MenuItemInvokedEvent )
      {
      if ( this->ChooseLayoutIconMenuButton != NULL && menu == this->ChooseLayoutIconMenuButton->GetMenu() )
        {
        if ( appGUI->GetGUILayoutNode() == NULL )
          {
          //--- if there's no layout node yet, create it,
          //--- add it to the scene, and make the
          //--- applicationGUI observe it.
          layout = vtkMRMLLayoutNode::New();
          if (layout == NULL)
            {
            vtkErrorMacro ( "ERROR: No layout node exists and cannot create a new one." );
            return;
            }
          this->MRMLScene->AddNode(layout);
          appGUI->SetAndObserveGUILayoutNode ( layout );
          //--- update MRML selection node.
          if ( this->ApplicationLogic != NULL )
            {
            if ( this->ApplicationLogic->GetSelectionNode() != NULL )
              {
              this->ApplicationLogic->GetSelectionNode()->SetActiveLayoutID( layout->GetID() );
              }
            }
          layout->Delete();
          }
        layout = appGUI->GetGUILayoutNode();

        const char *whichLayout = this->ChooseLayoutIconMenuButton->GetValue();
        if ( !strcmp ( whichLayout, "Conventional layout"))
          {
          if ( layout->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutConventionalView )
            {
            appGUI->GetMRMLScene()->SaveStateForUndo ( layout );
            layout->SetViewArrangement (vtkMRMLLayoutNode::SlicerLayoutConventionalView);
            }
          }
        else if (!strcmp( whichLayout, "Dual 3D layout" ))
          {
          if ( layout->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutDual3DView )
            {
            appGUI->GetMRMLScene()->SaveStateForUndo ( layout );
            layout->SetViewArrangement (vtkMRMLLayoutNode::SlicerLayoutDual3DView);
            }
          }
        else if (!strcmp( whichLayout, "3D only layout" ))
          {
          if ( layout->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutOneUp3DView )
            {
            appGUI->GetMRMLScene()->SaveStateForUndo ( layout );
            layout->SetViewArrangement (vtkMRMLLayoutNode::SlicerLayoutOneUp3DView);
            }
          }
        else if ( !strcmp ( whichLayout, "Four-up layout"))
          {
          if ( layout->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutFourUpView )
            {
            appGUI->GetMRMLScene()->SaveStateForUndo ( layout );
            layout->SetViewArrangement (vtkMRMLLayoutNode::SlicerLayoutFourUpView);
            }
          }
        else if ( !strcmp (whichLayout, "Tabbed 3D layout") )
          {
          if ( layout->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutTabbed3DView )
            {
            appGUI->GetMRMLScene()->SaveStateForUndo ( layout );
            layout->SetViewArrangement (vtkMRMLLayoutNode::SlicerLayoutTabbed3DView);
            }
          }
        else if (!strcmp ( whichLayout, "Tabbed slice layout"))
          {
          if ( layout->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutTabbedSliceView )
            {
            appGUI->GetMRMLScene()->SaveStateForUndo ( layout );
            layout->SetViewArrangement (vtkMRMLLayoutNode::SlicerLayoutTabbedSliceView );
            }
          }
        else if (!strcmp ( whichLayout, "Side-by-side lightbox layout"))
          {
          PopUpCompareViewCustomLayoutFrame();
          }
        else if (!strcmp ( whichLayout, "Compare layout"))
          {
          PopUpCompareViewCustomLayoutFrame();
          }
        else if ( !strcmp (whichLayout, "Red slice only layout") )
          {
          if ( layout->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView )
            {
            appGUI->GetMRMLScene()->SaveStateForUndo ( layout );
            layout->SetViewArrangement (vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView);
            }
          }
        else if ( !strcmp (whichLayout, "Yellow slice only layout") )
          {
          if ( layout->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutOneUpYellowSliceView )
            {
            appGUI->GetMRMLScene()->SaveStateForUndo ( layout );
            layout->SetViewArrangement (vtkMRMLLayoutNode::SlicerLayoutOneUpYellowSliceView);
            }
          } 
        else if ( !strcmp (whichLayout, "Green slice only layout") )
          {
          if ( layout->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView )
            {
            appGUI->GetMRMLScene()->SaveStateForUndo ( layout );
            layout->SetViewArrangement (vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView);
            }
          }
        else if ( !strcmp ( whichLayout, "Toggle GUI panel visibility"))
          {
          int v = appGUI->GetMainSlicerWindow()->GetMainPanelVisibility();
          appGUI->GetMainSlicerWindow()->SetMainPanelVisibility (!v );
          layout->SetGUIPanelVisibility ( appGUI->GetMainSlicerWindow()->GetMainPanelVisibility() );
          this->SetLayoutMenubuttonValueToCurrentLayout ();
          }
        else if ( !strcmp ( whichLayout, "Toggle GUI panel L/R"))
          {
          int v = appGUI->GetMainSlicerWindow()->GetViewPanelPosition();
          appGUI->GetMainSlicerWindow()->SetViewPanelPosition ( !v );
          layout->SetGUIPanelLR ( appGUI->GetMainSlicerWindow()->GetViewPanelPosition() );
          this->SetLayoutMenubuttonValueToCurrentLayout();
          }
        }
      else if ( this->MousePickOptionsButton != NULL &&
                menu == this->MousePickOptionsButton->GetMenu()  &&
                interactionNode != NULL )
        {
        if ( this->MousePickOptionsButton->GetMenu()->GetNumberOfItems() > 2 )
          {
          if ( this->MousePickOptionsButton->GetMenu()->GetItemSelectedState ( "Use mouse to Pick-and-Manipulate one time."))
            {
            this->MousePickButton->SelectedStateOn();
            //--- ...and modify persistence
            interactionNode->SetPickModePersistence (0);
            }
          else if ( this->MousePickOptionsButton->GetMenu()->GetItemSelectedState ( "Use mouse to Pick-and-Manipulate persistently." ))
            {
            this->MousePickButton->SelectedStateOn();
            //--- ...and modify persistence
            interactionNode->SetPickModePersistence (1);
            }
          }
        }
      else if ( this->MousePlaceOptionsButton != NULL &&
                menu == this->MousePlaceOptionsButton->GetMenu() &&
                interactionNode != NULL )
        {
        if ( this->MousePlaceOptionsButton->GetMenu()->GetNumberOfItems() > 2 )
          {
          if ( this->MousePlaceOptionsButton->GetMenu()->GetItemSelectedState ( "Use mouse to Create-and-Place one time."))
            {
            this->MousePlaceButton->SelectedStateOn();
            //--- ...and modify persistence
            interactionNode->SetPlaceModePersistence (0);
            }
          else if ( this->MousePlaceOptionsButton->GetMenu()->GetItemSelectedState ("Use mouse to Create-and-Place persistently." ))
            {
            this->MousePlaceButton->SelectedStateOn();
            //--- ...and modify persistence
            interactionNode->SetPlaceModePersistence (1);
            }
          }
        }
      else if (this->LoadSceneIconButton!= NULL &&  menu == this->LoadSceneIconButton->GetMenu() )
        {
        const char *thingToDo = this->LoadSceneIconButton->GetValue();
        if ( !strcmp ( thingToDo, "Load scene"))
          {
          appGUI->ProcessLoadSceneCommand();
          this->LoadSceneIconButton->SetValue ( "");
          }
        else if (!strcmp (thingToDo, "Import scene"))
          {
          appGUI->ProcessImportSceneCommand();
          this->LoadSceneIconButton->SetValue ("");
          }
        else if ( !strcmp ( thingToDo, "Add data" ))
          {
          appGUI->ProcessAddDataCommand();
          this->LoadSceneIconButton->SetValue ("");
          }
        }
      }
    }
}




//---------------------------------------------------------------------------
void vtkSlicerToolbarGUI::SetLayoutMenubuttonValueToLayout (int layout)
{
//--- sets the layout dropdown menu selection to match layout value
if ( this->ChooseLayoutIconMenuButton->GetMenu() != NULL )
{
 switch ( layout )
   {
   case vtkMRMLLayoutNode::SlicerLayoutConventionalView:
     this->ChooseLayoutIconMenuButton->SetValue ( "Conventional layout" );
     break;
   case vtkMRMLLayoutNode::SlicerLayoutFourUpView:
     this->ChooseLayoutIconMenuButton->SetValue ( "Four-up layout" );
     break;
   case vtkMRMLLayoutNode::SlicerLayoutOneUp3DView:
     this->ChooseLayoutIconMenuButton->SetValue ( "3D only layout" );
     break;
   case vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView:
     this->ChooseLayoutIconMenuButton->SetValue ( "Red slice only layout" );
     break;
   case vtkMRMLLayoutNode::SlicerLayoutOneUpYellowSliceView:
     this->ChooseLayoutIconMenuButton->SetValue ( "Yellow slice only layout" );
     break;
   case vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView:
     this->ChooseLayoutIconMenuButton->SetValue ( "Green slice only layout" );
     break;
   case vtkMRMLLayoutNode::SlicerLayoutOneUpSliceView:
     this->ChooseLayoutIconMenuButton->SetValue ( "Red slice only layout" );
     break;
   case vtkMRMLLayoutNode::SlicerLayoutTabbed3DView:
     this->ChooseLayoutIconMenuButton->SetValue ( "Tabbed 3D layout" );
     break;
   case vtkMRMLLayoutNode::SlicerLayoutTabbedSliceView:
     this->ChooseLayoutIconMenuButton->SetValue ( "Tabbed slice layout" );
     break;
   case vtkMRMLLayoutNode::SlicerLayoutCompareView:
     this->ChooseLayoutIconMenuButton->SetValue ( "Compare layout" );
     break;
   case vtkMRMLLayoutNode::SlicerLayoutSideBySideLightboxView:
     this->ChooseLayoutIconMenuButton->SetValue ( "Side-by-side lightbox layout");
   case vtkMRMLLayoutNode::SlicerLayoutLightboxView:              
     this->ChooseLayoutIconMenuButton->SetValue ( "Lightbox layout" );
     break;
   case vtkMRMLLayoutNode::SlicerLayoutDual3DView:              
     this->ChooseLayoutIconMenuButton->SetValue ( "Dual 3D layout" );
     break;
   default:
     break;
   }
}
}


//---------------------------------------------------------------------------
void vtkSlicerToolbarGUI::SetLayoutMenubuttonValueToCurrentLayout ()
{
  if ( this->GetApplication() != NULL && this->GetApplicationGUI() != NULL )
    {
    if ( this->GetApplicationGUI()->GetGUILayoutNode() != NULL )
      {
      int layout = this->GetApplicationGUI()->GetGUILayoutNode()->GetViewArrangement ();
      this->SetLayoutMenubuttonValueToLayout ( layout );
      }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerToolbarGUI::ResumeViewRockOrSpin ( int mode )
{
  if ( this->ApplicationGUI != NULL && this->GetApplicationGUI() != NULL )
    {
    vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));
    if ( p->GetViewControlGUI() != NULL )
      {
      if ( p->GetViewControlGUI()->GetActiveView() != NULL )
        {
        p->GetViewControlGUI()->GetActiveView()->SetAnimationMode( mode );
        }
      }
    }
}



//---------------------------------------------------------------------------
int vtkSlicerToolbarGUI::StopViewRockOrSpin ( )
{
  if ( this->ApplicationGUI != NULL )
    {
    vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));
    if ( p->GetViewControlGUI() != NULL )
      {
      if ( p->GetViewControlGUI()->GetActiveView() != NULL )
        {
        int mode = p->GetViewControlGUI()->GetActiveView()->GetAnimationMode();
        if ( mode == vtkMRMLViewNode::Rock || mode == vtkMRMLViewNode::Spin )
          {
          p->GetViewControlGUI()->GetActiveView()->SetAnimationMode ( vtkMRMLViewNode::Off );
          }
        return ( mode );
        }
      }
    }
  return ( 0 );
}



//---------------------------------------------------------------------------
void vtkSlicerToolbarGUI::ProcessLogicEvents ( vtkObject * vtkNotUsed(caller),
                                               unsigned long vtkNotUsed(event), void * vtkNotUsed(callData) )
{
  // Fill in}
}




//---------------------------------------------------------------------------
void vtkSlicerToolbarGUI::UpdateLayoutMenu()
{
  if ( this->ApplicationGUI == NULL )
    {
    return;
    }
  vtkSlicerApplicationGUI *appGUI = vtkSlicerApplicationGUI::SafeDownCast ( this->GetApplicationGUI() );
  
  // has the layout changed? Make the GUI track
  //--- gui's current layout
  const char *newLayout = appGUI->GetCurrentLayoutStringName ( );
  const char *guiLayout = this->ChooseLayoutIconMenuButton->GetValue();

  if ( newLayout != NULL )
    {
    if ( (strcmp ( guiLayout, newLayout ) ) != 0 )
      {
      //--- set menu to match node
      this->SetLayoutMenubuttonValueToCurrentLayout ();
      }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerToolbarGUI::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, void * vtkNotUsed(callData) )
{
  vtkDebugMacro("ProcessMRMLEvents: got event " << event << "(modified = " << vtkCommand::ModifiedEvent << ")");

  // check for a change on the selection node regarding the mouse interaction mode
  vtkMRMLInteractionNode *interactionNode = this->GetInteractionNode();

  // has the interaction mode changed?
  if (interactionNode == NULL)
    {
    vtkWarningMacro("ProcessMRMLEvents: interaction node is null");
    return;
    }

  if ( vtkMRMLInteractionNode::SafeDownCast(caller) == interactionNode &&
       event == vtkMRMLInteractionNode::InteractionModeChangedEvent )
    {
    vtkDebugMacro("ProcessMRMLEvents: The selection node changed");
    int mode = interactionNode->GetCurrentInteractionMode();
    int selected = 0;
    switch (mode)
      {
      case vtkMRMLInteractionNode::PickManipulate:
        // turn on processing events on the vtk 3d widgets
        this->ModifyAll3DWidgetsLock(0);
        selected = this->MousePickButton->GetSelectedState();
        if ( !selected )
          {
          this->MousePickButton->SelectedStateOn();
          this->MousePickOptionsButton->SetImageToIcon ( this->SlicerToolbarIcons->GetSlicerOptionsOnIcon ( ) );
          this->MousePlaceOptionsButton->SetImageToIcon ( this->SlicerToolbarIcons->GetSlicerOptionsOffIcon ( ) );
          }
        break;
      case vtkMRMLInteractionNode::Place:
        // turn off processing events on the vtk 3d widgets
        this->ModifyAll3DWidgetsLock(1);
        selected = this->MousePlaceButton->GetSelectedState();
        if ( !selected)
          {
          this->MousePlaceButton->SelectedStateOn();
          this->MousePickOptionsButton->SetImageToIcon ( this->SlicerToolbarIcons->GetSlicerOptionsOffIcon ( ) );
          this->MousePlaceOptionsButton->SetImageToIcon ( this->SlicerToolbarIcons->GetSlicerOptionsOnIcon ( ) );
          }
        break;
      case vtkMRMLInteractionNode::ViewTransform:
        // turn on processing events on the vtk 3d widgets
        // TODO: should they be locked when transforming the view? only if we
        // can't get the hover zone to be coincident with the manipulate zone
        this->ModifyAll3DWidgetsLock(0);
        selected = this->MouseTransformViewButton->GetSelectedState();
        if ( !selected)
          {
          this->MouseTransformViewButton->SelectedStateOn();
          this->MousePickOptionsButton->SetImageToIcon ( this->SlicerToolbarIcons->GetSlicerOptionsOffIcon ( ) );
          this->MousePlaceOptionsButton->SetImageToIcon ( this->SlicerToolbarIcons->GetSlicerOptionsOffIcon ( ) );                  
          }
        break;
      default:
        break;
      }
    }
  else if ( vtkMRMLInteractionNode::SafeDownCast(caller) == interactionNode && event == vtkMRMLInteractionNode::TransientTimeoutEvent )
    {
    this->MouseTransformViewButton->SelectedStateOn();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerToolbarGUI::Enter ( )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerToolbarGUI::Exit ( )
{
    // Fill in
}


//---------------------------------------------------------------------------
void vtkSlicerToolbarGUI::SetApplicationGUI ( vtkSlicerApplicationGUI *appGUI )
{
  this->ApplicationGUI = appGUI;
}







//---------------------------------------------------------------------------
void vtkSlicerToolbarGUI::BuildGUI ( )
{
  //
  //--- toolbars
  //
  //--- configure the window's main toolbarset.
  vtkSlicerApplicationGUI *p = this->GetApplicationGUI ( );
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( p->GetApplication() );
  vtkSlicerWindow *win = p->GetMainSlicerWindow();

  vtkKWToolbarSet *tbs = win->GetMainToolbarSet();
  tbs->SetToolbarsWidgetsAspect ( vtkKWToolbar::WidgetsAspectUnChanged );
  tbs->SetTopSeparatorVisibility ( 0 );
  tbs->SetBottomSeparatorVisibility ( 0 );

  //--- configure toolbars
  vtkKWToolbar *mtb = this->GetModulesToolbar ( );
  mtb->SetParent ( tbs->GetToolbarsFrame ( ) );
  mtb->Create();
  mtb->SetWidgetsFlatAdditionalPadX ( 0 );
  mtb->SetWidgetsFlatAdditionalPadY ( 0 );
  mtb->ResizableOff ( );
  mtb->SetReliefToGroove ( );
  mtb->SetWidgetsPadX ( 2 );
  mtb->SetWidgetsPadY ( 2 );

  vtkKWToolbar *ltb = this->GetLoadSaveToolbar ( );
  ltb->SetName("Load/Save");
  ltb->SetParent ( tbs->GetToolbarsFrame ( ) );
  ltb->Create();
  ltb->SetWidgetsFlatAdditionalPadX ( 0 );
  ltb->SetWidgetsFlatAdditionalPadY ( 0 );
  ltb->ResizableOff ( );
  ltb->SetReliefToGroove ( );
  ltb->SetWidgetsPadX ( 2 );
  ltb->SetWidgetsPadY ( 2 );

  vtkKWToolbar *utb = this->GetUtilitiesToolbar ( );
  utb->SetName ("Utilities" );
  utb->SetParent ( tbs->GetToolbarsFrame() );
  utb->Create();
  utb->SetWidgetsFlatAdditionalPadX(0);
  utb->SetWidgetsFlatAdditionalPadY ( 0 );
  utb->ResizableOff();
  utb->SetReliefToGroove();
  utb->SetWidgetsPadX ( 2 );
  utb->SetWidgetsPadY ( 2 );  

  vtkKWToolbar *vtb = this->GetViewToolbar ( );
  vtb->SetName("View");
  vtb->SetParent ( tbs->GetToolbarsFrame ( ) );
  vtb->Create();
  vtb->SetWidgetsFlatAdditionalPadX ( 0 );
  vtb->SetWidgetsFlatAdditionalPadY ( 0 );
  vtb->ResizableOff ( );
  vtb->SetReliefToGroove ( );
  vtb->SetWidgetsPadX ( 2 );
  vtb->SetWidgetsPadY ( 2 );

  vtkKWToolbar *urtb = this->GetUndoRedoToolbar ( );
  urtb->SetName("Undo/Redo");
  urtb->SetParent ( tbs->GetToolbarsFrame ( ) );
  urtb->Create();
  urtb->SetWidgetsFlatAdditionalPadX ( 0 );
  urtb->SetWidgetsFlatAdditionalPadY ( 0 );
  urtb->ResizableOff ( );
  urtb->SetReliefToGroove ( );
  urtb->SetWidgetsPadX ( 2 );
  urtb->SetWidgetsPadY ( 2 );
  
  vtkKWToolbar *mmtb = this->GetInteractionModeToolbar ( );
  mmtb->SetName("Mouse Mode");
  mmtb->SetParent ( tbs->GetToolbarsFrame ( ) );
  mmtb->Create();
  mmtb->SetWidgetsFlatAdditionalPadX ( 0 );
  mmtb->SetWidgetsFlatAdditionalPadY ( 0 );
  mmtb->ResizableOff ( );
  mmtb->SetReliefToGroove ( );
  mmtb->SetWidgetsPadX ( 0 );
  mmtb->SetWidgetsPadY ( 2 );
        
  //--- and add toolbars to the window's main toolbar set.        
  tbs->AddToolbar ( this->GetLoadSaveToolbar() );
  tbs->AddToolbar ( this->GetModulesToolbar() );
  tbs->AddToolbar ( this->GetUndoRedoToolbar () );

  //---re-locating screen capture in view control GUI.
  //  tbs->AddToolbar ( this->GetUtilitiesToolbar() );
  tbs->AddToolbar ( this->GetViewToolbar() );
  tbs->AddToolbar ( this->GetInteractionModeToolbar() );
        
  //
  //--- create icons and the labels that display them and add to toolbar
  //
  // load scene icon
  int index;
  this->LoadSceneIconButton->SetParent ( ltb->GetFrame ( ) );
  this->LoadSceneIconButton->Create();
  this->LoadSceneIconButton->SetReliefToFlat ( );
  this->LoadSceneIconButton->SetBorderWidth ( 0 );
  this->LoadSceneIconButton->IndicatorVisibilityOff();
  this->LoadSceneIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetLoadSceneIcon( ) );
  this->LoadSceneIconButton->SetBalloonHelpString ( "Load or import a MRML scene, or add data.");
  this->LoadSceneIconButton->GetMenu()->DeleteAllItems();
  this->LoadSceneIconButton->GetMenu()->AddRadioButton ( "Load scene" );
  index = this->LoadSceneIconButton->GetMenu()->GetIndexOfItem ( "Load scene");
  this->LoadSceneIconButton->GetMenu()->SetItemIndicatorVisibility ( index, 0 );
  this->LoadSceneIconButton->GetMenu()->AddRadioButton ( "Import scene" );
  index = this->LoadSceneIconButton->GetMenu()->GetIndexOfItem ( "Import scene");
  this->LoadSceneIconButton->GetMenu()->SetItemIndicatorVisibility ( index, 0 );
  this->LoadSceneIconButton->GetMenu()->AddRadioButton ( "Add data" );
  index = this->LoadSceneIconButton->GetMenu()->GetIndexOfItem ( "Add data");
  this->LoadSceneIconButton->GetMenu()->SetItemIndicatorVisibility ( index, 0 );
  this->LoadSceneIconButton->GetMenu()->AddSeparator ( );
  this->LoadSceneIconButton->GetMenu()->AddCommand ("close");  
  ltb->AddWidget ( this->LoadSceneIconButton );

  // save scene icon
  this->SaveSceneIconButton->SetParent ( ltb->GetFrame ( ));
  this->SaveSceneIconButton->Create ( );
  this->SaveSceneIconButton->SetReliefToFlat ( );
  this->SaveSceneIconButton->SetBorderWidth ( 0 );
  this->SaveSceneIconButton->SetOverReliefToNone ( );
  this->SaveSceneIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetSaveSceneIcon( ) );
  this->SaveSceneIconButton->SetBalloonHelpString ( "Save a MRML scene or data to a file.");
  ltb->AddWidget ( this->SaveSceneIconButton );

  // build module choose gui here.
    // Build the Module Choose GUI in the Modules toolbar.
  if (this->GetModuleChooseGUI() != NULL )
    {
    this->ModuleChooseGUI->SetApplicationGUI ( p );
    this->ModuleChooseGUI->SetApplication ( app );
    this->ModuleChooseGUI->BuildGUI( mtb );
    }

  // home icon
  this->HomeIconButton->SetParent ( mtb->GetFrame ( ));
  this->HomeIconButton->Create ( );
  this->HomeIconButton->SetReliefToFlat ( );
  this->HomeIconButton->SetBorderWidth ( 0 );
  this->HomeIconButton->SetOverReliefToNone ( );
  this->HomeIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetHomeIcon( ) );
  this->HomeIconButton->SetBalloonHelpString ( "Home module (set home module with Ctrl+h)" );
  mtb->AddWidget ( this->HomeIconButton );

  // data module icon
  this->DataIconButton->SetParent ( mtb->GetFrame ( ));
  this->DataIconButton->Create ( );
  this->DataIconButton->SetReliefToFlat ( );
  this->DataIconButton->SetBorderWidth ( 0 );
  this->DataIconButton->SetOverReliefToNone ( );
  this->DataIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetDataIcon ( ) );
  this->DataIconButton->SetBalloonHelpString ( "Data");
  mtb->AddWidget ( this->DataIconButton );

  // volume module icon
  this->VolumeIconButton->SetParent ( mtb->GetFrame ( ));
  this->VolumeIconButton->Create ( );
  this->VolumeIconButton->SetReliefToFlat ( );
  this->VolumeIconButton->SetBorderWidth ( 0 );
  this->VolumeIconButton->SetOverReliefToNone ( );
  this->VolumeIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetVolumeIcon ( ));
  this->VolumeIconButton->SetBalloonHelpString ( "Volumes");
  mtb->AddWidget ( this->VolumeIconButton );

  // models module icon
  this->ModelIconButton->SetParent (mtb->GetFrame ( ) );
  this->ModelIconButton->Create ( );
  this->ModelIconButton->SetReliefToFlat ( );
  this->ModelIconButton->SetBorderWidth ( 0 );
  this->ModelIconButton->SetOverReliefToNone ( );
  this->ModelIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetModelIcon ( ) );
  this->ModelIconButton->SetBalloonHelpString ( "Models");
  mtb->AddWidget ( this->ModelIconButton );

  // transforms module icon
  this->TransformIconButton->SetParent ( mtb->GetFrame ( ) );
  this->TransformIconButton->Create ( );
  this->TransformIconButton->SetReliefToFlat ( );
  this->TransformIconButton->SetBorderWidth ( 0 );
  this->TransformIconButton->SetOverReliefToNone ( );
  this->TransformIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetTransformIcon ( ) );
  this->TransformIconButton->SetBalloonHelpString ( "Transforms");
  mtb->AddWidget ( this->TransformIconButton );

  // fiducial utility icon
  this->FiducialsIconButton->SetParent ( mtb->GetFrame ( ) );
  this->FiducialsIconButton->Create ( );
  this->FiducialsIconButton->SetReliefToFlat ( );
  this->FiducialsIconButton->SetBorderWidth ( 0 );
  this->FiducialsIconButton->SetOverReliefToNone ( );
  this->FiducialsIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetFiducialsIcon ( ) );
  this->FiducialsIconButton->SetBalloonHelpString ( "Fiducials");
  mtb->AddWidget ( this->FiducialsIconButton );

  // editor module icon

  this->EditorToolboxIconButton->SetParent ( mtb->GetFrame ( ) );
  this->EditorToolboxIconButton->Create ( );
  this->EditorToolboxIconButton->SetReliefToFlat ( );
  this->EditorToolboxIconButton->SetBorderWidth ( 0 );
  this->EditorToolboxIconButton->SetOverReliefToNone ( );
  this->EditorToolboxIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetEditorToolboxIcon ( ) );
  this->EditorToolboxIconButton->SetBalloonHelpString ( "EditBox (or use keyboard 'space')");        
  mtb->AddWidget ( this->EditorToolboxIconButton );

  // editor module icon
  this->EditorIconButton->SetParent ( mtb->GetFrame ( ) );
  this->EditorIconButton->Create ( );
  this->EditorIconButton->SetReliefToFlat ( );
  this->EditorIconButton->SetBorderWidth ( 0 );
  this->EditorIconButton->SetOverReliefToNone ( );
  this->EditorIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetEditorIcon ( ) );
  this->EditorIconButton->SetBalloonHelpString ( "Editor (for Label Maps)");        
  mtb->AddWidget ( this->EditorIconButton );

#if ( (VTK_MAJOR_VERSION >= 6) || ( VTK_MAJOR_VERSION == 5 && VTK_MINOR_VERSION >= 4 ) )
  // measurements module icon
  this->MeasurementsIconButton->SetParent ( mtb->GetFrame ( ) );
  this->MeasurementsIconButton->Create ( );
  this->MeasurementsIconButton->SetReliefToFlat ( );
  this->MeasurementsIconButton->SetBorderWidth ( 0 );
  this->MeasurementsIconButton->SetOverReliefToNone ( );
  this->MeasurementsIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetMeasurementsIcon ( ) );
  this->MeasurementsIconButton->SetBalloonHelpString ( "Measurements");        
  mtb->AddWidget ( this->MeasurementsIconButton );
#endif
  
  // color utility icon
  this->ColorIconButton->SetParent ( mtb->GetFrame ( ) );
  this->ColorIconButton->Create ( );
  this->ColorIconButton->SetReliefToFlat ( );
  this->ColorIconButton->SetBorderWidth ( 0 );
  this->ColorIconButton->SetOverReliefToNone ( );
  this->ColorIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetColorIcon ( ) );
  this->ColorIconButton->SetBalloonHelpString ( "Colors");
  mtb->AddWidget ( this->ColorIconButton );

  // undo icon
  this->UndoIconButton->SetParent ( urtb->GetFrame ( ) );
  this->UndoIconButton->Create ( );
  this->UndoIconButton->SetReliefToFlat ( );
  this->UndoIconButton->SetBorderWidth ( 0 );
  this->UndoIconButton->SetOverReliefToNone ( );
  this->UndoIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetUndoIcon ( ) );
  this->UndoIconButton->SetBalloonHelpString ( "Undo (or use keyboard Ctrl+Z)");
  urtb->AddWidget ( this->UndoIconButton );

  // redo icon
  this->RedoIconButton->SetParent ( urtb->GetFrame ( ) );
  this->RedoIconButton->Create ( );
  this->RedoIconButton->SetReliefToFlat ( );
  this->RedoIconButton->SetBorderWidth ( 0 );
  this->RedoIconButton->SetOverReliefToNone ( );
  this->RedoIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetRedoIcon ( ) );
  this->RedoIconButton->SetBalloonHelpString ( "Redo (or use keyboard Ctrl+Y)");
  urtb->AddWidget ( this->RedoIconButton );

  // Layout choose menu
  std::string imageName;
  this->ChooseLayoutIconMenuButton->SetParent ( vtb->GetFrame ( ) );
  this->ChooseLayoutIconMenuButton->Create ( );
  this->ChooseLayoutIconMenuButton->SetReliefToFlat ( );
  this->ChooseLayoutIconMenuButton->SetBorderWidth ( 0 );
  this->ChooseLayoutIconMenuButton->IndicatorVisibilityOff ( );
  this->ChooseLayoutIconMenuButton->SetImageToIcon ( this->SlicerToolbarIcons->GetChooseLayoutIcon ( ) );
  this->ChooseLayoutIconMenuButton->SetBalloonHelpString ( "Choose among layouts for the 3D and Slice viewers" );
  this->ChooseLayoutIconMenuButton->GetMenu()->DeleteAllItems ( );

  this->ChooseLayoutIconMenuButton->GetMenu()->AddRadioButton ( "Conventional layout" );
  index = this->ChooseLayoutIconMenuButton->GetMenu()->GetIndexOfItem ( "Conventional layout");
  imageName.clear();
  imageName = "SlicerConventionalLayoutImage";
  vtkKWTkUtilities::UpdatePhotoFromIcon ( this->GetApplication(), imageName.c_str(), this->SlicerToolbarIcons->GetConventionalViewIcon(), 0 );
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemImage ( index, imageName.c_str());
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
//  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemVariableValueAsInt ("Conventional layout", vtkMRMLLayoutNode::SlicerLayoutConventionalView );

  this->ChooseLayoutIconMenuButton->GetMenu()->AddRadioButton ( "Four-up layout");
  index = this->ChooseLayoutIconMenuButton->GetMenu()->GetIndexOfItem ( "Four-up layout");
  imageName.clear();
  imageName = "SlicerFourUpLayoutImage";
  vtkKWTkUtilities::UpdatePhotoFromIcon ( this->GetApplication(), imageName.c_str(), this->SlicerToolbarIcons->GetFourUpViewIcon(), 0 );
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemImage ( index, imageName.c_str());
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
//  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemVariableValueAsInt ( "Four-up layout", vtkMRMLLayoutNode::SlicerLayoutFourUpView );

  this->ChooseLayoutIconMenuButton->GetMenu()->AddRadioButton ( "Dual 3D layout");
  index = this->ChooseLayoutIconMenuButton->GetMenu()->GetIndexOfItem ( "Dual 3D layout");
  imageName.clear();
  imageName = "SlicerTwin3DLayoutImage";
  vtkKWTkUtilities::UpdatePhotoFromIcon ( this->GetApplication(), imageName.c_str(), this->SlicerToolbarIcons->GetTwinViewIcon(), 0 );
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemImage ( index, imageName.c_str());
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
//  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemVariableValueAsInt
//  ( "Dual 3D layout", vtkMRMLLayoutNode::SlicerLayoutDual3DView );

  this->ChooseLayoutIconMenuButton->GetMenu()->AddRadioButton ( "3D only layout");
  index = this->ChooseLayoutIconMenuButton->GetMenu()->GetIndexOfItem ( "3D only layout");
  imageName.clear();
  imageName = "Slicer3DOnlyLayoutImage";
  vtkKWTkUtilities::UpdatePhotoFromIcon ( this->GetApplication(), imageName.c_str(), this->SlicerToolbarIcons->GetOneUp3DViewIcon(), 0 );
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemImage ( index, imageName.c_str());
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
//  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemVariableValueAsInt ( "3D only layout", vtkMRMLLayoutNode::SlicerLayoutOneUp3DView );

  this->ChooseLayoutIconMenuButton->GetMenu()->AddRadioButton ( "Red slice only layout");
  index = this->ChooseLayoutIconMenuButton->GetMenu()->GetIndexOfItem ( "Red slice only layout");
  imageName.clear();
  imageName = "SlicerRedSliceOnlyLayoutImage";
  vtkKWTkUtilities::UpdatePhotoFromIcon ( this->GetApplication(), imageName.c_str(), this->SlicerToolbarIcons->GetOneUpRedSliceViewIcon(), 0 );
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemImage ( index, imageName.c_str());
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
//  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemVariableValueAsInt ( "Red slice only layout", vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView);

  this->ChooseLayoutIconMenuButton->GetMenu()->AddRadioButton ( "Yellow slice only layout");
  index = this->ChooseLayoutIconMenuButton->GetMenu()->GetIndexOfItem ( "Yellow slice only layout");
  imageName.clear();
  imageName = "SlicerYellowSliceOnlyLayoutImage";
  vtkKWTkUtilities::UpdatePhotoFromIcon ( this->GetApplication(), imageName.c_str(), this->SlicerToolbarIcons->GetOneUpYellowSliceViewIcon(), 0 );
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemImage ( index, imageName.c_str());
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
//  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemVariableValueAsInt ("Yellow slice only layout", vtkMRMLLayoutNode::SlicerLayoutOneUpYellowSliceView);

  this->ChooseLayoutIconMenuButton->GetMenu()->AddRadioButton ( "Green slice only layout" );
  index = this->ChooseLayoutIconMenuButton->GetMenu()->GetIndexOfItem ( "Green slice only layout");
  imageName.clear();
  imageName = "SlicerGreenSliceOnlyLayoutImage";
  vtkKWTkUtilities::UpdatePhotoFromIcon ( this->GetApplication(), imageName.c_str(), this->SlicerToolbarIcons->GetOneUpGreenSliceViewIcon(), 0 );
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemImage ( index, imageName.c_str());
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
//  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemVariableValueAsInt ( "Green slice only layout", vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView);

  this->ChooseLayoutIconMenuButton->GetMenu()->AddRadioButton ( "Tabbed 3D layout" );
  index = this->ChooseLayoutIconMenuButton->GetMenu()->GetIndexOfItem ( "Tabbed 3D layout");
  imageName.clear(); 
  imageName = "SlicerTabbed3DLayoutImage";
  vtkKWTkUtilities::UpdatePhotoFromIcon ( this->GetApplication(), imageName.c_str(), this->SlicerToolbarIcons->GetTabbed3DViewIcon(), 0 );
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemImage ( index, imageName.c_str());
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
//  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemVariableValueAsInt ( "Tabbed 3D layout", vtkMRMLLayoutNode::SlicerLayoutTabbed3DView);

  this->ChooseLayoutIconMenuButton->GetMenu()->AddRadioButton ( "Tabbed slice layout" );
  index = this->ChooseLayoutIconMenuButton->GetMenu()->GetIndexOfItem ( "Tabbed slice layout");
  imageName.clear();
  imageName = "SlicerTabbedSliceLayoutImage";
  vtkKWTkUtilities::UpdatePhotoFromIcon ( this->GetApplication(), imageName.c_str(), this->SlicerToolbarIcons->GetTabbedSliceViewIcon(), 0 );
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemImage ( index, imageName.c_str());
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
//  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemVariableValueAsInt ( "Tabbed slice layout", vtkMRMLLayoutNode::SlicerLayoutTabbedSliceView);

  this->ChooseLayoutIconMenuButton->GetMenu()->AddRadioButton ("Compare layout");
  index = this->ChooseLayoutIconMenuButton->GetMenu()->GetIndexOfItem ("Compare layout");
  imageName.clear();
  imageName = "SlicerCompareViewLayoutImage";
  vtkKWTkUtilities::UpdatePhotoFromIcon ( this->GetApplication(), imageName.c_str(), this->SlicerToolbarIcons->GetCompareViewIcon(), 0);
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemImage ( index, imageName.c_str() );
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );

  this->ChooseLayoutIconMenuButton->GetMenu()->AddRadioButton ("Side-by-side lightbox layout");
  index = this->ChooseLayoutIconMenuButton->GetMenu()->GetIndexOfItem ("Side-by-side lightbox layout");
  imageName.clear();
  imageName = "SlicerSideBySideCompareViewLayoutImage";
  vtkKWTkUtilities::UpdatePhotoFromIcon ( this->GetApplication(), imageName.c_str(), this->SlicerToolbarIcons->GetSideBySideCompareViewIcon(), 0);
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemImage ( index, imageName.c_str() );
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
  
  this->ChooseLayoutIconMenuButton->GetMenu()->AddRadioButton ( "Toggle GUI panel visibility" );
  index = this->ChooseLayoutIconMenuButton->GetMenu()->GetIndexOfItem ( "Toggle GUI panel visibility");
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemIndicatorVisibility ( index, 0 );


  this->ChooseLayoutIconMenuButton->GetMenu()->AddRadioButton ( "Toggle GUI panel L/R" );
  index = this->ChooseLayoutIconMenuButton->GetMenu()->GetIndexOfItem ( "Toggle GUI panel L/R");
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemIndicatorVisibility ( index, 0 );
  
  this->ChooseLayoutIconMenuButton->GetMenu()->AddSeparator ( );
  this->ChooseLayoutIconMenuButton->GetMenu()->AddCommand ("close");  
  this->ChooseLayoutIconMenuButton->SetBinding ( "<Button-1>", this, "StopViewRockOrSpin" );

    ////--- Pop-up frame for custom NXM lightbox configuration
    this->CompareViewBoxTopLevel = vtkKWTopLevel::New ( );
    this->CompareViewBoxTopLevel->SetApplication ( app );
    this->CompareViewBoxTopLevel->SetMasterWindow ( this->ChooseLayoutIconMenuButton );
    this->CompareViewBoxTopLevel->Create ( );
    this->CompareViewBoxTopLevel->HideDecorationOn ( );
    this->CompareViewBoxTopLevel->Withdraw ( );
    this->CompareViewBoxTopLevel->SetBorderWidth ( 2 );
    this->CompareViewBoxTopLevel->SetReliefToGroove ( );

    //--- create frames to display when compare viewer is selected
    vtkKWFrameWithLabel *viewerConfigFrame = vtkKWFrameWithLabel::New ( );
    viewerConfigFrame->SetParent( this->CompareViewBoxTopLevel );
    viewerConfigFrame->Create ( );
    viewerConfigFrame->SetLabelText ( "Compare viewer options" );
    this->Script ( "pack %s -side top -anchor w -padx 4 -pady 2 -fill x -fill y -expand n", viewerConfigFrame->GetWidgetName ( ) );   
    this->Script ( "grid columnconfigure %s 0 -weight 0", viewerConfigFrame->GetWidgetName() );
    this->Script ( "grid columnconfigure %s 1 -weight 0", viewerConfigFrame->GetWidgetName() );

    vtkKWFrame *applyCancelFrame = vtkKWFrame::New();
    applyCancelFrame->SetParent ( this->CompareViewBoxTopLevel );
    applyCancelFrame->Create();
    this->Script ( "pack %s -side top -anchor w -padx 2 -pady 2 -fill x -fill y -expand n", applyCancelFrame->GetWidgetName ( ) );   
    this->Script ( "grid columnconfigure %s 0 -weight 0", applyCancelFrame->GetWidgetName() );
    this->Script ( "grid columnconfigure %s 1 -weight 0", applyCancelFrame->GetWidgetName() );

    //--- choose how many compare viewers are arrayed horizontally and vertically
    vtkKWLabel *rowsLabel = vtkKWLabel::New();
    rowsLabel->SetParent ( viewerConfigFrame->GetFrame() );
    rowsLabel->Create ( );
    rowsLabel->SetText ( "Number of compare viewers:" );
    this->CompareViewBoxRowEntry = vtkKWEntry::New ( );
    this->CompareViewBoxRowEntry->SetParent ( viewerConfigFrame->GetFrame() );
    this->CompareViewBoxRowEntry->Create ( );
    this->CompareViewBoxRowEntry->SetValueAsInt (2);
    this->CompareViewBoxRowEntry->SetWidth ( 3 );
    //--- grid compare viewer configuration widgets
    this->Script ( "grid %s -row 0 -column 0 -sticky e -padx 2 -pady 4", rowsLabel->GetWidgetName());
    this->Script ( "grid %s -row 0 -column 1 -sticky w -padx 2 -pady 4", this->CompareViewBoxRowEntry->GetWidgetName() );

    vtkKWLabel *l2 = vtkKWLabel::New();
    l2->SetParent ( viewerConfigFrame->GetFrame() );
    l2->Create ( );
    l2->SetText ( "Lightbox rows in each viewer:" );
    this->CompareViewLightboxRowEntry = vtkKWEntry::New();
    this->CompareViewLightboxRowEntry->SetParent ( viewerConfigFrame->GetFrame() );
    this->CompareViewLightboxRowEntry->Create();
    this->CompareViewLightboxRowEntry->SetWidth ( 3 );
    this->CompareViewLightboxRowEntry->SetValueAsInt ( 1 );
    vtkKWLabel *l3 = vtkKWLabel::New();
    l3->SetParent ( viewerConfigFrame->GetFrame() );
    l3->Create ( );
    l3->SetText ( "Lightbox columns in each viewer:" );
    this->CompareViewLightboxColumnEntry = vtkKWEntry::New();
    this->CompareViewLightboxColumnEntry->SetParent ( viewerConfigFrame->GetFrame() );
    this->CompareViewLightboxColumnEntry->Create();    
    this->CompareViewLightboxColumnEntry->SetWidth ( 3 );
    this->CompareViewLightboxColumnEntry->SetValueAsInt ( 6 );
    //--- grid up lightbox configuration
    this->Script ( "grid %s -row 1 -column 0 -padx 2 -pady 4 -sticky e", l2->GetWidgetName());
    this->Script ( "grid %s -row 1 -column 1 -padx 2 -pady 4 -sticky w", this->CompareViewLightboxRowEntry->GetWidgetName() );
    this->Script ( "grid %s -row 2 -column 0 -padx 2 -pady 4 -sticky e", l3->GetWidgetName());
    this->Script ( "grid %s -row 2 -column 1 -padx 2 -pady 4 -sticky w", this->CompareViewLightboxColumnEntry->GetWidgetName() );

    //--- apply or cancel the compare view configuration.
    this->CompareViewBoxApplyButton = vtkKWPushButton::New ( );
    this->CompareViewBoxApplyButton->SetParent ( applyCancelFrame);
    this->CompareViewBoxApplyButton->Create ( );
    this->CompareViewBoxApplyButton->SetText ("Apply");    
    this->CompareViewBoxApplyButton->SetWidth ( 10 );
    vtkKWPushButton *b = vtkKWPushButton::New();
    b->SetParent ( applyCancelFrame );
    b->Create();
    b->SetText ( "Cancel");
    b->SetWidth ( 10 );
    b->SetBinding ( "<Button-1>", this,  "HideCompareViewCustomLayoutFrame");
    this->Script ( "grid %s -row 0 -column 0 -padx 2 -pady 8 -sticky ew", b->GetWidgetName() );
    this->Script ( "grid %s -row 0 -column 1 -padx 2 -pady 8 -sticky ew", this->CompareViewBoxApplyButton->GetWidgetName() );

    // delete temporary stuff
    l2->Delete();
    l3->Delete();
    b->Delete();
    applyCancelFrame->Delete();
    rowsLabel->Delete();
    viewerConfigFrame->Delete();

    vtb->AddWidget (this->ChooseLayoutIconMenuButton );

  //---
  //--- Mouse mode toolbar
  //---
  int mouseMode = vtkMRMLInteractionNode::ViewTransform;

  // try to get the mouse interaction mode from the mrml scene
  vtkMRMLInteractionNode *interactionNode = NULL;
  if (this->ApplicationLogic != NULL)
    {
    interactionNode =  this->ApplicationLogic->GetInteractionNode();
    if (interactionNode != NULL)
      {
      this->SetInteractionNode ( interactionNode );
      mouseMode = interactionNode->GetCurrentInteractionMode();
      }
    }
  else 
    { 
    this->SetInteractionNode ( NULL );
    vtkDebugMacro ("MRML Scene not set yet, not getting mouse interaction mode, using default of transform\n");
    mouseMode = vtkMRMLInteractionNode::ViewTransform;
    }

  this->MousePickButton->SetParent (mmtb->GetFrame() );
  this->MousePickButton->Create();
  this->MousePickButton->SetBorderWidth (0);
  this->MousePickButton->SetReliefToFlat();
  this->MousePickButton->SetValueAsInt ( vtkMRMLInteractionNode::PickManipulate );
  this->MousePickButton->SetImageToIcon ( this->SlicerToolbarIcons->GetMousePickOffIcon ( ) );
  this->MousePickButton->SetSelectImageToIcon ( this->SlicerToolbarIcons->GetMousePickOnIcon ( ) );
  this->MousePickButton->SetBalloonHelpString ( "Set the mouse mode to 'Pick-and-Manipulate' (to select and move a pickable object)." );
  if ( mouseMode == vtkMRMLInteractionNode::PickManipulate)
    {
    this->MousePickButton->SetSelectedState ( 1 );
    }
  else
    {
    this->MousePickButton->SetSelectedState ( 0 );
    }
  mmtb->AddWidget ( this->MousePickButton );

  this->MousePickOptionsButton->SetParent (mmtb->GetFrame() );
  this->MousePickOptionsButton->Create();
  this->MousePickOptionsButton->IndicatorVisibilityOff();
  this->MousePickOptionsButton->SetReliefToFlat ( );
  this->MousePickOptionsButton->SetBorderWidth ( 0 );
  this->MousePickOptionsButton->SetImageToIcon ( this->SlicerToolbarIcons->GetSlicerOptionsOffIcon ( ) );
  this->MousePickOptionsButton->GetMenu()->DeleteAllItems();
  this->MousePickOptionsButton->GetMenu()->AddRadioButton ("Use mouse to Pick-and-Manipulate one time.");
  index = this->MousePickOptionsButton->GetMenu()->GetIndexOfItem("Use mouse to Pick-and-Manipulate one time.");
  this->MousePickOptionsButton->GetMenu()->SetItemIndicatorVisibility (index, 0);
  this->MousePickOptionsButton->GetMenu()->SetItemMarginVisibility (index, 0);
  this->MousePickOptionsButton->GetMenu()->AddRadioButton ("Use mouse to Pick-and-Manipulate persistently.");
  index = this->MousePickOptionsButton->GetMenu()->GetIndexOfItem ("Use mouse to Pick-and-Manipulate persistently.");
  this->MousePickOptionsButton->GetMenu()->SetItemIndicatorVisibility (index, 0);
  this->MousePickOptionsButton->GetMenu()->SetItemMarginVisibility (index, 0);
  this->MousePickOptionsButton->GetMenu()->SelectItem ( "Use mouse to Pick-and-Manipulate one time.");
  this->MousePickOptionsButton->GetMenu()->AddSeparator();
  this->MousePickOptionsButton->GetMenu()->AddCommand ( "close");
  mmtb->AddWidget ( this->MousePickOptionsButton );
  
  this->MousePlaceButton->SetParent (mmtb->GetFrame() );
  this->MousePlaceButton->Create();
  this->MousePlaceButton->SetBorderWidth ( 0 );
  this->MousePlaceButton->SetReliefToFlat();
  this->MousePlaceButton->SetValueAsInt ( vtkMRMLInteractionNode::Place );
  this->MousePlaceButton->SetImageToIcon ( this->SlicerToolbarIcons->GetMousePlaceOffIcon ( ) );
  this->MousePlaceButton->SetSelectImageToIcon ( this->SlicerToolbarIcons->GetMousePlaceOnIcon ( ) );
  this->MousePlaceButton->SetBalloonHelpString ( "Set the mouse mode to 'Create-and-Place' (to create and position a new object like a fiducial point)." );
  this->MousePlaceButton->SetVariableName ( this->MousePickButton->GetVariableName() );
  if ( mouseMode == vtkMRMLInteractionNode::Place)
    {
    this->MousePlaceButton->SetSelectedState ( 1 );
    }
  else
    {
    this->MousePlaceButton->SetSelectedState ( 0 );
    }
  mmtb->AddWidget ( this->MousePlaceButton );

  this->MousePlaceOptionsButton->SetParent (mmtb->GetFrame() );
  this->MousePlaceOptionsButton->Create();
  this->MousePlaceOptionsButton->IndicatorVisibilityOff();
  this->MousePlaceOptionsButton->SetReliefToFlat ( );
  this->MousePlaceOptionsButton->SetBorderWidth ( 0 );
  this->MousePlaceOptionsButton->SetImageToIcon ( this->SlicerToolbarIcons->GetSlicerOptionsOffIcon ( ) );
  this->MousePlaceOptionsButton->GetMenu()->DeleteAllItems();
  this->MousePlaceOptionsButton->GetMenu()->AddRadioButton ("Use mouse to Create-and-Place one time.");
  index = this->MousePlaceOptionsButton->GetMenu()->GetIndexOfItem ("Use mouse to Create-and-Place one time.");
  this->MousePlaceOptionsButton->GetMenu()->SetItemIndicatorVisibility (index, 0);
  this->MousePlaceOptionsButton->GetMenu()->SetItemMarginVisibility (index, 0);
  this->MousePlaceOptionsButton->GetMenu()->AddRadioButton ("Use mouse to Create-and-Place persistently.");
  index = this->MousePlaceOptionsButton->GetMenu()->GetIndexOfItem ("Use mouse to Create-and-Place persistently.");
  this->MousePlaceOptionsButton->GetMenu()->SetItemIndicatorVisibility (index, 0);
  this->MousePlaceOptionsButton->GetMenu()->SetItemMarginVisibility (index, 0);
  this->MousePlaceOptionsButton->GetMenu()->SelectItem ( "Use mouse to Create-and-Place one time.");
  this->MousePlaceOptionsButton->GetMenu()->AddSeparator();
  this->MousePlaceOptionsButton->GetMenu()->AddCommand ( "close");
  mmtb->AddWidget ( this->MousePlaceOptionsButton );

  this->MouseTransformViewButton->SetParent (mmtb->GetFrame() );
  this->MouseTransformViewButton->Create();
  this->MouseTransformViewButton->SetBorderWidth ( 0 );
  this->MouseTransformViewButton->SetReliefToFlat();
  this->MouseTransformViewButton->SetValueAsInt ( vtkMRMLInteractionNode::ViewTransform );
  this->MouseTransformViewButton->SetImageToIcon ( this->SlicerToolbarIcons->GetMouseTransformViewOffIcon ( ) );
  this->MouseTransformViewButton->SetSelectImageToIcon ( this->SlicerToolbarIcons->GetMouseTransformViewOnIcon ( ) );
  this->MouseTransformViewButton->SetSelectColor ( app->GetSlicerTheme()->GetSlicerColors()->White );
  this->MouseTransformViewButton->SetBalloonHelpString ( "Set the 3DViewer mouse mode to 'transform view'" );
  this->MouseTransformViewButton->SetVariableName ( this->MousePickButton->GetVariableName() );
  if ( mouseMode == vtkMRMLInteractionNode::ViewTransform)
    {
    this->MouseTransformViewButton->SetSelectedState ( 1 );
    }
  else
    {
    this->MouseTransformViewButton->SetSelectedState ( 0 );
    }
  mmtb->AddWidget ( this->MouseTransformViewButton );

  // configure fonts for all widgets that have text.
  this->ReconfigureGUIFonts();

}

void vtkSlicerToolbarGUI::PopUpCompareViewCustomLayoutFrame( )
{
  if ( !this->ChooseLayoutIconMenuButton || !this->ChooseLayoutIconMenuButton->IsCreated())
    {
    return;
    }
  /*
  if ( !this->UndoIconButton || !this->UndoIconButton->IsCreated())
    {
    return;
    }
  */
  if ( !this->ColorIconButton || !this->ColorIconButton->IsCreated())
    {
    return;
    }

  // Get the position of the mouse, the position and size of the push button,
  // the size of the scale.

  int x, y, px, py, ph;
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast ( this->GetApplication());
  
  vtkKWTkUtilities::GetMousePointerCoordinates(this->ChooseLayoutIconMenuButton, &x, &y);
  vtkKWTkUtilities::GetWidgetCoordinates(this->ColorIconButton, &px, &py);
//  vtkKWTkUtilities::GetWidgetCoordinates(this->UndoIconButton, &px, &py);
  vtkKWTkUtilities::GetWidgetSize(this->ChooseLayoutIconMenuButton, NULL, &ph);
 
  this->CompareViewBoxTopLevel->SetPosition(px-ph, py+ph);
  app->ProcessPendingEvents();

  this->CompareViewBoxTopLevel->DeIconify();
  this->CompareViewBoxTopLevel->Raise();
}

void vtkSlicerToolbarGUI::HideCompareViewCustomLayoutFrame()
{
    if ( !this->CompareViewBoxTopLevel )
      return;

    this->CompareViewBoxTopLevel->Withdraw();
}



//--------------------------------------------------------------------------
void vtkSlicerToolbarGUI::ModifyAll3DWidgetsLock(int lockFlag)
{
  //vtkWarningMacro("ModifyAll3DWidgetsLock: lockFlag = " << lockFlag);

  vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI ( );
  if ( appGUI )
    {
    appGUI->ModifyAllWidgetLock(lockFlag);
    }

  vtkDebugMacro("ModifyAll3DWidgetsLock: done changing locks on widgets (lock = " << lockFlag << ")");
}
