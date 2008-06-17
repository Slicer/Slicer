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
#include "vtkKWFrameWithLabel.h"
#include "vtkKWRadioButton.h"
#include "vtkKWSeparator.h"
#include "vtkKWMenu.h"
#include "vtkKWEntry.h"
#include "vtkKWLabel.h"

//#define LIGHTBOXGUI_DEBUG

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerToolbarGUI );
vtkCxxRevisionMacro ( vtkSlicerToolbarGUI, "$Revision: 1.0 $");


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

  this->HomeIconButton = vtkKWPushButton::New ( );
  this->DataIconButton = vtkKWPushButton::New ( );
  this->VolumeIconButton = vtkKWPushButton::New ( );
  this->ModelIconButton = vtkKWPushButton::New ( );
  this->EditorIconButton = vtkKWPushButton::New ( );
  this->EditorToolboxIconButton = vtkKWPushButton::New ( );
  this->ColorIconButton = vtkKWPushButton::New ( );
  this->FiducialsIconButton = vtkKWPushButton::New ( );
//  this->MeasurementsIconButton = vtkKWPushButton::New ( );
  this->TransformIconButton = vtkKWPushButton::New ( );
  this->SaveSceneIconButton = vtkKWPushButton::New ( );
  this->LoadSceneIconButton = vtkKWMenuButton::New ( );
  this->ChooseLayoutIconMenuButton = vtkKWMenuButton::New();
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

  // Delete module choose gui
  if ( this->ModuleChooseGUI )
    {
    this->ModuleChooseGUI->Delete ( );
    this->ModuleChooseGUI = NULL;
    }
    
  // Delete the widgets
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
/*
  if ( this->MeasurementsIconButton )
    {
    this->MeasurementsIconButton->SetParent ( NULL );
    this->MeasurementsIconButton->Delete ( );
    this->MeasurementsIconButton = NULL;
    }
*/
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

    this->SetApplicationGUI ( NULL );
    this->SetInteractionNodeID ( NULL );
    vtkSetMRMLNodeMacro( this->InteractionNode, NULL);
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
      this->GetModuleChooseGUI()->GetModulesMenuButton()->SetFont ( app->GetSlicerTheme()->GetApplicationFont1() );
      this->GetModuleChooseGUI()->GetModulesLabel()->SetFont ( app->GetSlicerTheme()->GetApplicationFont1() );
      this->GetModuleChooseGUI()->GetModulesSearchEntry()->SetFont ( app->GetSlicerTheme()->GetApplicationFont1() );
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerToolbarGUI::RemoveMRMLObservers()
{
}

//---------------------------------------------------------------------------
void vtkSlicerToolbarGUI::AddMRMLObservers()
{
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
  this->ColorIconButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ChooseLayoutIconMenuButton->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->UndoIconButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->RedoIconButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->MousePickButton->RemoveObservers( vtkKWRadioButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->MousePlaceButton->RemoveObservers( vtkKWRadioButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->MouseTransformViewButton->RemoveObservers( vtkKWRadioButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
      
  this->ModuleChooseGUI->RemoveGUIObservers();
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
  this->TransformIconButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ColorIconButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    
  // view configuration icon button observers...
  this->ChooseLayoutIconMenuButton->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->UndoIconButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->RedoIconButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->MousePickButton->AddObserver( vtkKWRadioButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->MousePlaceButton->AddObserver( vtkKWRadioButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->MouseTransformViewButton->AddObserver( vtkKWRadioButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  
  this->ModuleChooseGUI->AddGUIObservers();
}


//---------------------------------------------------------------------------
void vtkSlicerToolbarGUI::ProcessGUIEvents ( vtkObject *caller,
                                          unsigned long event, void *callData )
{
  int val;
  if ( this->GetApplicationGUI() != NULL )
    {
    // Toolbar's parent is the main vtkSlicerApplicationGUI;
    // Toolbar events will trigger vtkSlicerAppliationGUI methods
    vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));
    vtkSlicerModuleChooseGUI *mcGUI = this->ModuleChooseGUI;
    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( p->GetApplication() );
    vtkMRMLInteractionNode *interactionNode = NULL;
    
    if (p != NULL)
      {
      //interactionNode = vtkMRMLInteractionNode::SafeDownCast (p->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLInteractionNode"));
      }
    interactionNode = this->GetApplicationLogic()->GetInteractionNode();

    if ( app != NULL )
      {
      vtkKWRadioButton *radiob = vtkKWRadioButton::SafeDownCast ( caller );
      vtkKWPushButton *pushb = vtkKWPushButton::SafeDownCast ( caller );
      vtkKWMenu *menu = vtkKWMenu::SafeDownCast ( caller );
  
      if ( mcGUI != NULL )
        {            
        // Process events from top row of buttons
        // Mouse mode buttons:
        if ( radiob == this->MousePickButton
             && event == vtkKWRadioButton::SelectedStateChangedEvent )
          {
          val = radiob->GetSelectedState();
          if ( val && interactionNode )
            {
            interactionNode->SetLastInteractionMode ( interactionNode->GetCurrentInteractionMode() );
            interactionNode->SetCurrentInteractionMode ( vtkMRMLInteractionNode::PickManipulate );
            }
          }
        else if ( radiob == this->MouseTransformViewButton
                  && event == vtkKWRadioButton::SelectedStateChangedEvent)
          {
          val = radiob->GetSelectedState();
          if ( val && interactionNode )
            {
            interactionNode->SetLastInteractionMode ( interactionNode->GetCurrentInteractionMode() );
            interactionNode->SetCurrentInteractionMode ( vtkMRMLInteractionNode::ViewTransform );
            }
          }
        else if ( radiob == this->MousePlaceButton
                  && event == vtkKWRadioButton::SelectedStateChangedEvent)
          {
          val = radiob->GetSelectedState();
          if ( val && interactionNode )
            {
            interactionNode->SetLastInteractionMode ( interactionNode->GetCurrentInteractionMode() );
            interactionNode->SetCurrentInteractionMode ( vtkMRMLInteractionNode::Place );
            }
          }

        if ( pushb == this->HomeIconButton && event == vtkKWPushButton::InvokedEvent )
          {
          const char *homename = app->GetHomeModule();
          vtkSlicerModuleGUI *m = app->GetModuleGUIByName(homename);
          if ( m != NULL )
            {
            this->GetModuleChooseGUI()->SelectModule ( homename );
            }
          else
            {
            vtkDebugMacro ("ERROR:  no slicer module gui found for Volumes\n"); 
            }
          }
        else if (pushb == this->DataIconButton && event == vtkKWPushButton::InvokedEvent )
          {
          vtkSlicerModuleGUI *m = app->GetModuleGUIByName("Data");
          if ( m != NULL )
            {
            this->GetModuleChooseGUI()->SelectModule ( "Data" );
            }
          else
            {
            vtkDebugMacro ("ERROR:  no slicer module gui found for Data\n");
            }
          }
        else if (pushb == this->VolumeIconButton && event == vtkKWPushButton::InvokedEvent )
          {
          vtkSlicerModuleGUI *m = app->GetModuleGUIByName("Volumes");
          if ( m != NULL )
            {
            this->GetModuleChooseGUI()->SelectModule ( "Volumes" );
            }
          else
            {
            vtkDebugMacro ("ERROR:  no slicer module gui found for Volumes\n");
            }
          }
        else if (pushb == this->ModelIconButton && event == vtkKWPushButton::InvokedEvent )
          {
          vtkSlicerModuleGUI *m = app->GetModuleGUIByName("Models");
          if ( m != NULL )
            {
            this->GetModuleChooseGUI()->SelectModule ( "Models" );
            }
          else
            {
            vtkDebugMacro ("ERROR:  no slicer module gui found for Models\n");
            }
          }
        else if (pushb == this->FiducialsIconButton && event == vtkKWPushButton::InvokedEvent )
          {
          vtkSlicerModuleGUI *m = app->GetModuleGUIByName("Fiducials");
          if ( m != NULL )
            {
            this->GetModuleChooseGUI()->SelectModule ( "Fiducials" );
            }
          else
            {
            vtkDebugMacro ("ERROR:  no slicer module gui found for Fiducials\n");
            }
          }
        else if (pushb == this->ColorIconButton && event == vtkKWPushButton::InvokedEvent )
          {
          vtkSlicerModuleGUI *m = app->GetModuleGUIByName("Color");
          if ( m != NULL )
            {
            this->GetModuleChooseGUI()->SelectModule ( "Color" );
            }
          else
            {
            vtkDebugMacro ("ERROR:  no slicer module gui found for Color\n");
            }
          }
        else if (pushb == this->TransformIconButton && event == vtkKWPushButton::InvokedEvent )
          {
          vtkSlicerModuleGUI *m = app->GetModuleGUIByName("Transforms");
          if ( m != NULL )
            {
            this->GetModuleChooseGUI()->SelectModule ( "Transforms" );
            }
          else
            {
            vtkDebugMacro ("ERROR:  no slicer module gui found for Transforms\n");
            }
          }
        else if (pushb == this->EditorIconButton && event == vtkKWPushButton::InvokedEvent )
          {
          vtkSlicerModuleGUI *m = app->GetModuleGUIByName("Editor");
          if ( m != NULL )
            {
            this->GetModuleChooseGUI()->SelectModule ( "Editor" );
            }
          else
            {
            vtkDebugMacro ("ERROR:  no slicer module gui found for Editor\n");
            }
          }
        else if (pushb == this->EditorToolboxIconButton && event == vtkKWPushButton::InvokedEvent )
          {
          //---
          //--- Add code to pop up Editor toolbox here
          //---
          app->Script ("::EditBox::ShowDialog");
          }

        }


      // TODO: figure out why we can't resume view rock or spin.
      if ( menu == this->ChooseLayoutIconMenuButton->GetMenu() && event == vtkKWMenu::MenuItemInvokedEvent )
        {

        vtkMRMLLayoutNode *layout;
        if ( p->GetGUILayoutNode() == NULL )
          {
          //--- if there's no layout node yet, create it,
          //--- add it to the scene, and make the
          //--- applicationGUI observe it.
          layout = vtkMRMLLayoutNode::New();
          this->MRMLScene->AddNode(layout);
          p->SetAndObserveGUILayoutNode ( layout );
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

        layout = p->GetGUILayoutNode();
        const char *whichLayout = this->ChooseLayoutIconMenuButton->GetValue();
        if ( !strcmp ( whichLayout, "Conventional layout"))
          {
          if ( layout->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutConventionalView )
            {
            p->GetMRMLScene()->SaveStateForUndo ( layout );
            layout->SetViewArrangement (vtkMRMLLayoutNode::SlicerLayoutConventionalView);
            }
          }
        else if (!strcmp( whichLayout, "3D only layout" ))
          {
          if ( layout->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutOneUp3DView )
            {
            p->GetMRMLScene()->SaveStateForUndo ( layout );
            layout->SetViewArrangement (vtkMRMLLayoutNode::SlicerLayoutOneUp3DView);
            }
          }
        else if ( !strcmp ( whichLayout, "Four-up layout"))
          {
          if ( layout->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutFourUpView )
            {
            p->GetMRMLScene()->SaveStateForUndo ( layout );
            layout->SetViewArrangement (vtkMRMLLayoutNode::SlicerLayoutFourUpView);
            }
          }
        else if ( !strcmp (whichLayout, "Tabbed 3D layout") )
          {
          if ( layout->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutTabbed3DView )
            {
            p->GetMRMLScene()->SaveStateForUndo ( layout );
            layout->SetViewArrangement (vtkMRMLLayoutNode::SlicerLayoutTabbed3DView);
            }
          }
        else if (!strcmp ( whichLayout, "Tabbed slice layout"))
          {
          if ( layout->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutTabbedSliceView )
            {
            p->GetMRMLScene()->SaveStateForUndo ( layout );
            layout->SetViewArrangement (vtkMRMLLayoutNode::SlicerLayoutTabbedSliceView );
            }
          }
        else if ( !strcmp (whichLayout, "Red slice only layout") )
          {
          if ( layout->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView )
            {
            p->GetMRMLScene()->SaveStateForUndo ( layout );
            layout->SetViewArrangement (vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView);
            }
          }
        else if ( !strcmp (whichLayout, "Yellow slice only layout") )
          {
          if ( layout->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutOneUpYellowSliceView )
            {
            p->GetMRMLScene()->SaveStateForUndo ( layout );
            layout->SetViewArrangement (vtkMRMLLayoutNode::SlicerLayoutOneUpYellowSliceView);
            }
          } 
        else if ( !strcmp (whichLayout, "Green slice only layout") )
          {
          if ( layout->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView )
            {
            p->GetMRMLScene()->SaveStateForUndo ( layout );
            layout->SetViewArrangement (vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView);
            }
          }
        else if ( !strcmp ( whichLayout, "Toggle GUI panel visibility"))
          {
          int v = p->GetMainSlicerWindow()->GetMainPanelVisibility();
          p->GetMainSlicerWindow()->SetMainPanelVisibility (!v );
          layout->SetGUIPanelVisibility ( p->GetMainSlicerWindow()->GetMainPanelVisibility() );
          this->SetLayoutMenubuttonValueToCurrentLayout ();
          }
        else if ( !strcmp ( whichLayout, "Toggle bottom panel visibility"))
          {
          int v = p->GetMainSlicerWindow()->GetSecondaryPanelVisibility();
          p->GetMainSlicerWindow()->SetSecondaryPanelVisibility (!v );
          layout->SetBottomPanelVisibility ( p->GetMainSlicerWindow()->GetSecondaryPanelVisibility() );
          this->SetLayoutMenubuttonValueToCurrentLayout ();
          }
        else if ( !strcmp ( whichLayout, "Toggle GUI panel L/R"))
          {
          int v = p->GetMainSlicerWindow()->GetViewPanelPosition();
          p->GetMainSlicerWindow()->SetViewPanelPosition ( !v );
          layout->SetGUIPanelLR ( p->GetMainSlicerWindow()->GetViewPanelPosition() );
          this->SetLayoutMenubuttonValueToCurrentLayout();
          }
        }
      else if ( menu == this->LoadSceneIconButton->GetMenu() && event == vtkKWMenu::MenuItemInvokedEvent )
        {
        const char *thingToDo = this->LoadSceneIconButton->GetValue();
        if ( !strcmp ( thingToDo, "Load scene"))
          {
          p->ProcessLoadSceneCommand();
          this->LoadSceneIconButton->SetValue ( "");
          }
        else if (!strcmp (thingToDo, "Import scene"))
          {
          p->ProcessImportSceneCommand();
          this->LoadSceneIconButton->SetValue ("");
          }
        else if ( !strcmp ( thingToDo, "Add data" ))
          {
          p->ProcessAddDataCommand();
          this->LoadSceneIconButton->SetValue ("");
          }
        }
      if ( pushb == this->UndoIconButton && event == vtkKWPushButton::InvokedEvent )
        {
        p->GetMRMLScene()->Undo();
        }
      else if ( pushb == this->RedoIconButton && event == vtkKWPushButton::InvokedEvent )
        {
        p->GetMRMLScene()->Redo();
        }
      else if ( pushb == this->SaveSceneIconButton && event == vtkKWPushButton::InvokedEvent)
        {
        p->ProcessSaveSceneAsCommand();
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
      default:
        break;
      }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerToolbarGUI::SetLayoutMenubuttonValueToCurrentLayout ()
{
  if ( this->GetApplication() != NULL )
    {
    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( this->GetApplication ( ));
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
  if ( this->ApplicationGUI != NULL )
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
void vtkSlicerToolbarGUI::ProcessLogicEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
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
                                           unsigned long event, void *callData )
{
  std::cout << "vtkSlicerToolbarGUI::ProcessMRMLEvents: got event " << event << "(modified = " << vtkCommand::ModifiedEvent << ")" << endl;

  // check for a change on the selection node regarding the mouse interaction mode
  vtkMRMLInteractionNode *interactionNode = this->GetInteractionNode();

  // has the interaction mode changed?
  if (interactionNode == NULL)
    {
    std::cout << "vtkSlicerToolbarGUI::ProcessMRMLEvents: interaction node is null\n";
    return;
    }
  if ( vtkMRMLInteractionNode::SafeDownCast(caller) == interactionNode && event == vtkCommand::ModifiedEvent)
    {
    std::cout << "The selection node changed\n";
    int mode = interactionNode->GetCurrentInteractionMode();
    switch (mode)
      {
      case vtkMRMLInteractionNode::PickManipulate:
        this->MousePickButton->SelectedStateOn();
        break;
      case vtkMRMLInteractionNode::Place:
        this->MousePlaceButton->SelectedStateOn();
        break;
      case vtkMRMLInteractionNode::ViewTransform:
        this->MouseTransformViewButton->SelectedStateOn();
        break;
      default:
        break;
      }
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
  mtb->SetWidgetsPadX ( 3 );
  mtb->SetWidgetsPadY ( 2 );

  vtkKWToolbar *ltb = this->GetLoadSaveToolbar ( );
  ltb->SetName("Load/Save");
  ltb->SetParent ( tbs->GetToolbarsFrame ( ) );
  ltb->Create();
  ltb->SetWidgetsFlatAdditionalPadX ( 0 );
  ltb->SetWidgetsFlatAdditionalPadY ( 0 );
  ltb->ResizableOff ( );
  ltb->SetReliefToGroove ( );
  ltb->SetWidgetsPadX ( 3 );
  ltb->SetWidgetsPadY ( 2 );

  vtkKWToolbar *vtb = this->GetViewToolbar ( );
  vtb->SetName("View");
  vtb->SetParent ( tbs->GetToolbarsFrame ( ) );
  vtb->Create();
  vtb->SetWidgetsFlatAdditionalPadX ( 0 );
  vtb->SetWidgetsFlatAdditionalPadY ( 0 );
  vtb->ResizableOff ( );
  vtb->SetReliefToGroove ( );
  vtb->SetWidgetsPadX ( 3 );
  vtb->SetWidgetsPadY ( 2 );

  vtkKWToolbar *urtb = this->GetUndoRedoToolbar ( );
  urtb->SetName("Undo/Redo");
  urtb->SetParent ( tbs->GetToolbarsFrame ( ) );
  urtb->Create();
  urtb->SetWidgetsFlatAdditionalPadX ( 0 );
  urtb->SetWidgetsFlatAdditionalPadY ( 0 );
  urtb->ResizableOff ( );
  urtb->SetReliefToGroove ( );
  urtb->SetWidgetsPadX ( 3 );
  urtb->SetWidgetsPadY ( 2 );
  
  vtkKWToolbar *mmtb = this->GetInteractionModeToolbar ( );
  mmtb->SetName("Mouse Mode");
  mmtb->SetParent ( tbs->GetToolbarsFrame ( ) );
  mmtb->Create();
  mmtb->SetWidgetsFlatAdditionalPadX ( 0 );
  mmtb->SetWidgetsFlatAdditionalPadY ( 0 );
  mmtb->ResizableOff ( );
  mmtb->SetReliefToGroove ( );
  mmtb->SetWidgetsPadX ( 3 );
  mmtb->SetWidgetsPadY ( 2 );
        
  //--- and add toolbars to the window's main toolbar set.        
  tbs->AddToolbar ( this->GetLoadSaveToolbar() );
  tbs->AddToolbar ( this->GetModulesToolbar() );
  tbs->AddToolbar ( this->GetUndoRedoToolbar () );
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
  this->ModuleChooseGUI->SetApplicationGUI ( p );
  this->ModuleChooseGUI->SetApplication ( app );
  this->ModuleChooseGUI->BuildGUI( mtb );

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
  this->EditorIconButton->SetBalloonHelpString ( "Editor");        
  mtb->AddWidget ( this->EditorIconButton );

  // measurements module icon
/*
  this->MeasurementsIconButton->SetParent ( mtb->GetFrame ( ) );
  this->MeasurementsIconButton->Create ( );
  this->MeasurementsIconButton->SetReliefToFlat ( );
  this->MeasurementsIconButton->SetBorderWidth ( 0 );
  this->MeasurementsIconButton->SetOverReliefToNone ( );
  this->MeasurementsIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetMeasurementsIcon ( ) );
  this->MeasurementsIconButton->SetBalloonHelpString ( "Measurements (not yet available)");        
  mtb->AddWidget ( this->MeasurementsIconButton );
*/
  
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
  const char *imageName;
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
  imageName = "SlicerConventionalLayoutImage";
  vtkKWTkUtilities::UpdatePhotoFromIcon ( this->GetApplication(), imageName, this->SlicerToolbarIcons->GetConventionalViewIcon(), 0 );
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemImage ( index, imageName);
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
//  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemVariableValueAsInt ("Conventional layout", vtkMRMLLayoutNode::SlicerLayoutConventionalView );

  this->ChooseLayoutIconMenuButton->GetMenu()->AddRadioButton ( "Four-up layout");
  index = this->ChooseLayoutIconMenuButton->GetMenu()->GetIndexOfItem ( "Four-up layout");
  imageName = "SlicerFourUpLayoutImage";
  vtkKWTkUtilities::UpdatePhotoFromIcon ( this->GetApplication(), imageName, this->SlicerToolbarIcons->GetFourUpViewIcon(), 0 );
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemImage ( index, imageName);
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
//  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemVariableValueAsInt ( "Four-up layout", vtkMRMLLayoutNode::SlicerLayoutFourUpView );

  this->ChooseLayoutIconMenuButton->GetMenu()->AddRadioButton ( "3D only layout");
  index = this->ChooseLayoutIconMenuButton->GetMenu()->GetIndexOfItem ( "3D only layout");
  imageName = "Slicer3DOnlyLayoutImage";
  vtkKWTkUtilities::UpdatePhotoFromIcon ( this->GetApplication(), imageName, this->SlicerToolbarIcons->GetOneUp3DViewIcon(), 0 );
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemImage ( index, imageName);
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
//  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemVariableValueAsInt ( "3D only layout", vtkMRMLLayoutNode::SlicerLayoutOneUp3DView );

  this->ChooseLayoutIconMenuButton->GetMenu()->AddRadioButton ( "Red slice only layout");
  index = this->ChooseLayoutIconMenuButton->GetMenu()->GetIndexOfItem ( "Red slice only layout");
  imageName = "SlicerRedSliceOnlyLayoutImage";
  vtkKWTkUtilities::UpdatePhotoFromIcon ( this->GetApplication(), imageName, this->SlicerToolbarIcons->GetOneUpRedSliceViewIcon(), 0 );
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemImage ( index, imageName);
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
//  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemVariableValueAsInt ( "Red slice only layout", vtkMRMLLayoutNode::SlicerLayoutOneUpRedSliceView);

  this->ChooseLayoutIconMenuButton->GetMenu()->AddRadioButton ( "Yellow slice only layout");
  index = this->ChooseLayoutIconMenuButton->GetMenu()->GetIndexOfItem ( "Yellow slice only layout");
  imageName = "SlicerYellowSliceOnlyLayoutImage";
  vtkKWTkUtilities::UpdatePhotoFromIcon ( this->GetApplication(), imageName, this->SlicerToolbarIcons->GetOneUpYellowSliceViewIcon(), 0 );
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemImage ( index, imageName);
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
//  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemVariableValueAsInt ("Yellow slice only layout", vtkMRMLLayoutNode::SlicerLayoutOneUpYellowSliceView);

  this->ChooseLayoutIconMenuButton->GetMenu()->AddRadioButton ( "Green slice only layout" );
  index = this->ChooseLayoutIconMenuButton->GetMenu()->GetIndexOfItem ( "Green slice only layout");
  imageName = "SlicerGreenSliceOnlyLayoutImage";
  vtkKWTkUtilities::UpdatePhotoFromIcon ( this->GetApplication(), imageName, this->SlicerToolbarIcons->GetOneUpGreenSliceViewIcon(), 0 );
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemImage ( index, imageName);
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
//  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemVariableValueAsInt ( "Green slice only layout", vtkMRMLLayoutNode::SlicerLayoutOneUpGreenSliceView);

  this->ChooseLayoutIconMenuButton->GetMenu()->AddRadioButton ( "Tabbed 3D layout" );
  index = this->ChooseLayoutIconMenuButton->GetMenu()->GetIndexOfItem ( "Tabbed 3D layout");
  imageName = "SlicerTabbed3DLayoutImage";
  vtkKWTkUtilities::UpdatePhotoFromIcon ( this->GetApplication(), imageName, this->SlicerToolbarIcons->GetTabbed3DViewIcon(), 0 );
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemImage ( index, imageName);
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
//  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemVariableValueAsInt ( "Tabbed 3D layout", vtkMRMLLayoutNode::SlicerLayoutTabbed3DView);

  this->ChooseLayoutIconMenuButton->GetMenu()->AddRadioButton ( "Tabbed slice layout" );
  index = this->ChooseLayoutIconMenuButton->GetMenu()->GetIndexOfItem ( "Tabbed slice layout");
  imageName = "SlicerTabbedSliceLayoutImage";
  vtkKWTkUtilities::UpdatePhotoFromIcon ( this->GetApplication(), imageName, this->SlicerToolbarIcons->GetTabbedSliceViewIcon(), 0 );
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemImage ( index, imageName);
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );
//  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemVariableValueAsInt ( "Tabbed slice layout", vtkMRMLLayoutNode::SlicerLayoutTabbedSliceView);

  this->ChooseLayoutIconMenuButton->GetMenu()->AddRadioButton ("Compare view layout");
  index = this->ChooseLayoutIconMenuButton->GetMenu()->GetIndexOfItem ("Compare view layout");
  imageName = "SlicerCompareViewLayoutImage";
  vtkKWTkUtilities::UpdatePhotoFromIcon ( this->GetApplication(), imageName, this->SlicerToolbarIcons->GetCompareViewIcon(), 0);
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemImage ( index, imageName );
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemCompoundModeToLeft ( index );

  this->ChooseLayoutIconMenuButton->GetMenu()->AddRadioButton ( "Toggle GUI panel visibility" );
  index = this->ChooseLayoutIconMenuButton->GetMenu()->GetIndexOfItem ( "Toggle GUI panel visibility");
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemIndicatorVisibility ( index, 0 );
  this->ChooseLayoutIconMenuButton->GetMenu()->AddRadioButton ( "Toggle bottom panel visibility" );
  index = this->ChooseLayoutIconMenuButton->GetMenu()->GetIndexOfItem ( "Toggle bottom panel visibility");
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemIndicatorVisibility ( index, 0 );

  this->ChooseLayoutIconMenuButton->GetMenu()->AddRadioButton ( "Toggle GUI panel L/R" );
  index = this->ChooseLayoutIconMenuButton->GetMenu()->GetIndexOfItem ( "Toggle GUI panel L/R");
  this->ChooseLayoutIconMenuButton->GetMenu()->SetItemIndicatorVisibility ( index, 0 );
  
  this->ChooseLayoutIconMenuButton->GetMenu()->AddSeparator ( );
  this->ChooseLayoutIconMenuButton->GetMenu()->AddCommand ("close");  
  this->ChooseLayoutIconMenuButton->SetBinding ( "<Button-1>", this, "StopViewRockOrSpin" );
  
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
      mouseMode = interactionNode->GetCurrentInteractionMode();
      }
    }
  else 
    { 
    vtkDebugMacro ("MRML Scene not set yet, not getting mouse interaction mode, using default of transform\n");
    mouseMode = vtkMRMLInteractionNode::ViewTransform;
    }

  this->MousePickButton->SetParent (mmtb->GetFrame() );
  this->MousePickButton->Create();
  this->MousePickButton->SetValueAsInt ( vtkMRMLInteractionNode::PickManipulate );
  this->MousePickButton->SetImageToIcon ( this->SlicerToolbarIcons->GetMousePickOffIcon ( ) );
  this->MousePickButton->SetSelectImageToIcon ( this->SlicerToolbarIcons->GetMousePickOnIcon ( ) );
  this->MousePickButton->SetBalloonHelpString ( "Set the 3DViewer mouse mode to 'pick'" );
  if ( mouseMode == vtkMRMLInteractionNode::PickManipulate)
    {
    this->MousePickButton->SetSelectedState ( 1 );
    }
  else
    {
    this->MousePickButton->SetSelectedState ( 0 );
    }
  mmtb->AddWidget ( this->MousePickButton );
  
  this->MousePlaceButton->SetParent (mmtb->GetFrame() );
  this->MousePlaceButton->Create();
  this->MousePlaceButton->SetValueAsInt ( vtkMRMLInteractionNode::Place );
  this->MousePlaceButton->SetImageToIcon ( this->SlicerToolbarIcons->GetMousePlaceOffIcon ( ) );
  this->MousePlaceButton->SetSelectImageToIcon ( this->SlicerToolbarIcons->GetMousePlaceOnIcon ( ) );
  this->MousePlaceButton->SetBalloonHelpString ( "Set the 3DViewer mouse mode to 'place a new object (like a fiducial point)'" );
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

  this->MouseTransformViewButton->SetParent (mmtb->GetFrame() );
  this->MouseTransformViewButton->Create();
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



