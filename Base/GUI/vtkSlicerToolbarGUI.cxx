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

#include "vtkKWWidget.h"
#include "vtkKWToolbarSet.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWMenuButton.h"
#include "vtkKWRadioButton.h"
#include "vtkKWSeparator.h"

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
  this->MouseModeToolbar = vtkKWToolbar::New ( );
  this->UndoRedoToolbar = vtkKWToolbar::New ( );

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
  this->LoadSceneIconButton = vtkKWPushButton::New ( );
  this->ConventionalViewIconButton = vtkKWPushButton::New ( );
  this->OneUp3DViewIconButton = vtkKWPushButton::New ( );
  this->OneUpSliceViewIconButton = vtkKWPushButton::New ( );
  this->FourUpViewIconButton = vtkKWPushButton::New ( );
  this->Tabbed3DViewIconButton = vtkKWPushButton::New ( );
  this->TabbedSliceViewIconButton = vtkKWPushButton::New ( );
  this->LightBoxViewIconButton = vtkKWPushButton::New ( );
  this->MouseModeRadioButtons = vtkKWRadioButtonSet::New ( );
  this->UndoIconButton = vtkKWPushButton::New ( );
  this->RedoIconButton = vtkKWPushButton::New ( );
  this->ModuleChooseGUI = vtkSlicerModuleChooseGUI::New ( );
  
  this->ApplicationGUI = NULL;

  
}


//---------------------------------------------------------------------------
vtkSlicerToolbarGUI::~vtkSlicerToolbarGUI ( )
{

  // Delete Toolbar Icons
  if ( this->SlicerToolbarIcons )
    {
    this->SlicerToolbarIcons->Delete ( );
    this->SlicerToolbarIcons = NULL;
    }

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
  if ( this->MouseModeToolbar )
    {
    this->MouseModeToolbar->RemoveAllWidgets ( );
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
  if ( this->ConventionalViewIconButton )
    {
    this->ConventionalViewIconButton->SetParent ( NULL );
    this->ConventionalViewIconButton->Delete ( );
    this->ConventionalViewIconButton = NULL;
    }
  if ( this->OneUp3DViewIconButton )
    {
    this->OneUp3DViewIconButton->SetParent ( NULL );
    this->OneUp3DViewIconButton->Delete ( );
    this->OneUp3DViewIconButton = NULL;
    }
  if ( this->OneUpSliceViewIconButton )
    {
    this->OneUpSliceViewIconButton->SetParent ( NULL );
    this->OneUpSliceViewIconButton->Delete ( );
    this->OneUpSliceViewIconButton = NULL;
    }
  if ( this->FourUpViewIconButton )
    {
    this->FourUpViewIconButton->SetParent ( NULL );
    this->FourUpViewIconButton->Delete ( );
    this->FourUpViewIconButton = NULL;
    }
  if ( this->Tabbed3DViewIconButton )
    {
    this->Tabbed3DViewIconButton->SetParent ( NULL );
    this->Tabbed3DViewIconButton->Delete ( );
    this->Tabbed3DViewIconButton = NULL;
    }
  if ( this->TabbedSliceViewIconButton )
    {
    this->TabbedSliceViewIconButton->SetParent ( NULL );
    this->TabbedSliceViewIconButton->Delete ( );
    this->TabbedSliceViewIconButton = NULL;
    }
  if ( this->LightBoxViewIconButton )
    {
    this->LightBoxViewIconButton->SetParent ( NULL );
    this->LightBoxViewIconButton->Delete ( );
    this->LightBoxViewIconButton = NULL;
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
      vtkSlicerWindow *win = p->GetMainSlicerWindow();
      if ( win ) 
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
    if ( this->MouseModeToolbar )
      {
      this->MouseModeToolbar->SetParent ( NULL );
      this->MouseModeToolbar->Delete ( );
      this->MouseModeToolbar = NULL;
      }
    if ( this->MouseModeRadioButtons )
      {
      this->MouseModeRadioButtons->SetParent ( NULL );
      this->MouseModeRadioButtons->Delete ( );
      this->MouseModeRadioButtons = NULL;      
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
void vtkSlicerToolbarGUI::RemoveGUIObservers ( )
{
    // Fill in
    this->HomeIconButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->DataIconButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->VolumeIconButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ModelIconButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->FiducialsIconButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ColorIconButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ConventionalViewIconButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->OneUp3DViewIconButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->OneUpSliceViewIconButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->FourUpViewIconButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->Tabbed3DViewIconButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->TabbedSliceViewIconButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->LightBoxViewIconButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->UndoIconButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->RedoIconButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

    this->MouseModeRadioButtons->GetWidget( vtkSlicerApplicationGUI::MouseSelect )->RemoveObservers( vtkKWRadioButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->MouseModeRadioButtons->GetWidget( vtkSlicerApplicationGUI::MouseTransform )->RemoveObservers( vtkKWRadioButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->MouseModeRadioButtons->GetWidget( vtkSlicerApplicationGUI::MousePut )->RemoveObservers( vtkKWRadioButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);

    this->ModuleChooseGUI->RemoveGUIObservers();
}

//---------------------------------------------------------------------------
void vtkSlicerToolbarGUI::AddGUIObservers ( )
{
    // Fill in
    // add observers onto the module icon buttons 
    this->HomeIconButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->DataIconButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->VolumeIconButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ModelIconButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->FiducialsIconButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->TransformIconButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ColorIconButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    
    // view configuration icon button observers...
    this->ConventionalViewIconButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->OneUp3DViewIconButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->OneUpSliceViewIconButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->FourUpViewIconButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->Tabbed3DViewIconButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->TabbedSliceViewIconButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->LightBoxViewIconButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->UndoIconButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->RedoIconButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

    this->MouseModeRadioButtons->GetWidget( vtkSlicerApplicationGUI::MouseSelect )->AddObserver ( vtkKWRadioButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->MouseModeRadioButtons->GetWidget( vtkSlicerApplicationGUI::MouseTransform )->AddObserver ( vtkKWRadioButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    this->MouseModeRadioButtons->GetWidget( vtkSlicerApplicationGUI::MousePut )->AddObserver ( vtkKWRadioButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);

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
    vtkSlicerGUILayout *layout = app->GetMainLayout();
    if ( app != NULL )
      {
      vtkKWRadioButton *radiob = vtkKWRadioButton::SafeDownCast ( caller );
      vtkKWPushButton *pushb = vtkKWPushButton::SafeDownCast ( caller );
  
      if ( mcGUI != NULL )
        {            
        // Process events from top row of buttons
        // Mouse mode buttons:
        if ( radiob == this->MouseModeRadioButtons->GetWidget ( vtkSlicerApplicationGUI::MouseSelect )
             && event == vtkKWRadioButton::SelectedStateChangedEvent )
          {
          val = radiob->GetSelectedState();
          if ( val )
            {
            p->SetMouseInteractionMode ( vtkSlicerApplicationGUI::MouseSelect );
            }
          }
        else if ( radiob == this->MouseModeRadioButtons->GetWidget ( vtkSlicerApplicationGUI::MouseTransform )
                  && event == vtkKWRadioButton::SelectedStateChangedEvent)
          {
          val = radiob->GetSelectedState();
          if ( val )
            {
            p->SetMouseInteractionMode ( vtkSlicerApplicationGUI::MouseTransform );
            }
          }
        else if ( radiob == this->MouseModeRadioButtons->GetWidget ( vtkSlicerApplicationGUI::MousePut )
                  && event == vtkKWRadioButton::SelectedStateChangedEvent)
          {
          val = radiob->GetSelectedState();
          if ( val )
            {
            p->SetMouseInteractionMode ( vtkSlicerApplicationGUI::MousePut );
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
            std::cerr << "ERROR:  no slicer module gui found for Volumes\n"; 
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
            std::cerr << "ERROR:  no slicer module gui found for Data\n"; 
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
            std::cerr << "ERROR:  no slicer module gui found for Volumes\n"; 
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
            std::cerr << "ERROR:  no slicer module gui found for Models\n"; 
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
            std::cerr << "ERROR:  no slicer module gui found for Fiducials\n"; 
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
            std::cerr << "ERROR:  no slicer module gui found for Color\n"; 
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
            std::cerr << "ERROR:  no slicer module gui found for Transforms\n"; 
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
            std::cerr << "ERROR:  no slicer module gui found for Editor\n"; 
            }
          }
        }
          
      if ( pushb == this->ConventionalViewIconButton && event == vtkKWPushButton::InvokedEvent )
        {
        p->RepackMainViewer (vtkSlicerGUILayout::SlicerLayoutDefaultView );
        }
      else if ( pushb == this->OneUp3DViewIconButton && event == vtkKWPushButton::InvokedEvent )
        {
        p->RepackMainViewer ( vtkSlicerGUILayout::SlicerLayoutOneUp3DView);
        }
      else if ( pushb == this->OneUpSliceViewIconButton && event == vtkKWPushButton::InvokedEvent )
        {
        p->RepackMainViewer ( vtkSlicerGUILayout::SlicerLayoutOneUpSliceView );
        }
      else if ( pushb == this->FourUpViewIconButton && event == vtkKWPushButton::InvokedEvent )
        {
        p->RepackMainViewer ( vtkSlicerGUILayout::SlicerLayoutFourUpView );
        }
      else if ( pushb == this->Tabbed3DViewIconButton && event == vtkKWPushButton::InvokedEvent )
        {
        p->RepackMainViewer ( vtkSlicerGUILayout::SlicerLayoutTabbed3DView );
        }
      else if ( pushb == this->TabbedSliceViewIconButton && event == vtkKWPushButton::InvokedEvent )
        {
//        TODO: finish implementing this
//        p->RepackMainViewer ( vtkSlicerGUILayout::SlicerLayoutTabbedSliceView );
        }
      else if ( pushb == this->LightBoxViewIconButton && event == vtkKWPushButton::InvokedEvent )
        {
        // TODO: implement this
        }
      else if ( pushb == this->UndoIconButton && event == vtkKWPushButton::InvokedEvent )
        {
        // FILL IN
        }
      else if ( pushb == this->RedoIconButton && event == vtkKWPushButton::InvokedEvent )
        {
        // FILL IN
        }
      }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerToolbarGUI::ProcessLogicEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
{
    // Fill in}
}

//---------------------------------------------------------------------------
void vtkSlicerToolbarGUI::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, void *callData )
{
    // Fill in
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
  ltb->SetParent ( tbs->GetToolbarsFrame ( ) );
  ltb->Create();
  ltb->SetWidgetsFlatAdditionalPadX ( 0 );
  ltb->SetWidgetsFlatAdditionalPadY ( 0 );
  ltb->ResizableOff ( );
  ltb->SetReliefToGroove ( );
  ltb->SetWidgetsPadX ( 3 );
  ltb->SetWidgetsPadY ( 2 );

  vtkKWToolbar *vtb = this->GetViewToolbar ( );
  vtb->SetParent ( tbs->GetToolbarsFrame ( ) );
  vtb->Create();
  vtb->SetWidgetsFlatAdditionalPadX ( 0 );
  vtb->SetWidgetsFlatAdditionalPadY ( 0 );
  vtb->ResizableOff ( );
  vtb->SetReliefToGroove ( );
  vtb->SetWidgetsPadX ( 3 );
  vtb->SetWidgetsPadY ( 2 );

  vtkKWToolbar *urtb = this->GetUndoRedoToolbar ( );
  urtb->SetParent ( tbs->GetToolbarsFrame ( ) );
  urtb->Create();
  urtb->SetWidgetsFlatAdditionalPadX ( 0 );
  urtb->SetWidgetsFlatAdditionalPadY ( 0 );
  urtb->ResizableOff ( );
  urtb->SetReliefToGroove ( );
  urtb->SetWidgetsPadX ( 3 );
  urtb->SetWidgetsPadY ( 2 );
  
  vtkKWToolbar *mmtb = this->GetMouseModeToolbar ( );
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
  tbs->AddToolbar ( this->GetMouseModeToolbar() );
        
  //
  //--- create icons and the labels that display them and add to toolbar
  //
  // load scene icon
  this->LoadSceneIconButton->SetParent ( ltb->GetFrame ( ) );
  this->LoadSceneIconButton->Create();
  this->LoadSceneIconButton->SetReliefToFlat ( );
  this->LoadSceneIconButton->SetBorderWidth ( 0 );
  this->LoadSceneIconButton->SetOverReliefToNone ( );
  this->LoadSceneIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetLoadSceneIcon( ) );
  this->LoadSceneIconButton->SetBalloonHelpString ( "Load a MRML scene.");
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
  this->HomeIconButton->SetBalloonHelpString ( "Home module" );
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
  this->EditorToolboxIconButton->SetBalloonHelpString ( "Editor Toolbox (not yet available)");        
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
  this->MeasurementsIconButton->SetParent ( mtb->GetFrame ( ) );
  this->MeasurementsIconButton->Create ( );
  this->MeasurementsIconButton->SetReliefToFlat ( );
  this->MeasurementsIconButton->SetBorderWidth ( 0 );
  this->MeasurementsIconButton->SetOverReliefToNone ( );
  this->MeasurementsIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetMeasurementsIcon ( ) );
  this->MeasurementsIconButton->SetBalloonHelpString ( "Measurements (not yet available)");        
  mtb->AddWidget ( this->MeasurementsIconButton );

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


  // conventional view icon
  this->ConventionalViewIconButton->SetParent (vtb->GetFrame ( ) );
  this->ConventionalViewIconButton->Create ( );
  this->ConventionalViewIconButton->SetReliefToFlat ( );
  this->ConventionalViewIconButton->SetBorderWidth ( 0 );
  this->ConventionalViewIconButton->SetOverReliefToNone ( );
  this->ConventionalViewIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetConventionalViewIcon ( ) );        
  this->ConventionalViewIconButton->SetBalloonHelpString ("Display the 3D viewer over 3 slice windows");
  vtb->AddWidget ( this->ConventionalViewIconButton );
  // 3Dview-only icon
  this->OneUp3DViewIconButton->SetParent ( vtb->GetFrame ( ) );
  this->OneUp3DViewIconButton->Create ( );
  this->OneUp3DViewIconButton->SetReliefToFlat ( );
  this->OneUp3DViewIconButton->SetBorderWidth ( 0 );
  this->OneUp3DViewIconButton->SetOverReliefToNone ( );
  this->OneUp3DViewIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetOneUp3DViewIcon ( ) );
  this->OneUp3DViewIconButton->SetBalloonHelpString ( "Display the 3D viewer without any slice windows" );
  vtb->AddWidget (this->OneUp3DViewIconButton );

  // Slice view-only icon
  this->OneUpSliceViewIconButton->SetParent ( vtb->GetFrame ( ) );
  this->OneUpSliceViewIconButton->Create ( );
  this->OneUpSliceViewIconButton->SetReliefToFlat ( );
  this->OneUpSliceViewIconButton->SetBorderWidth ( 0 );
  this->OneUpSliceViewIconButton->SetOverReliefToNone ( );
  this->OneUpSliceViewIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetOneUpSliceViewIcon ( ) );
  this->OneUpSliceViewIconButton->SetBalloonHelpString ( "Display one slice window with no 3D viewer" );
  vtb->AddWidget (this->OneUpSliceViewIconButton );

  // 4 equal windows icon
  this->FourUpViewIconButton->SetParent ( vtb->GetFrame ( ) );
  this->FourUpViewIconButton->Create ( );
  this->FourUpViewIconButton->SetReliefToFlat ( );
  this->FourUpViewIconButton->SetBorderWidth ( 0 );
  this->FourUpViewIconButton->SetOverReliefToNone ( );
  this->FourUpViewIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetFourUpViewIcon ( ) );
  this->FourUpViewIconButton->SetBalloonHelpString ( "Display the 3D viewer and 3 slice windows in a matrix" );
  vtb->AddWidget ( this->FourUpViewIconButton );

  // tabbed view icon
  this->TabbedSliceViewIconButton->SetParent ( vtb->GetFrame ( ) );
  this->TabbedSliceViewIconButton->Create ( );
  this->TabbedSliceViewIconButton->SetReliefToFlat ( );
  this->TabbedSliceViewIconButton->SetBorderWidth ( 0 );
  this->TabbedSliceViewIconButton->SetOverReliefToNone ( );
  this->TabbedSliceViewIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetTabbedSliceViewIcon ( ) );
  this->TabbedSliceViewIconButton->SetBalloonHelpString ( "Display a collection of slices in a notebook" );
  vtb->AddWidget ( this->TabbedSliceViewIconButton );

  // tabbed view icon
  this->Tabbed3DViewIconButton->SetParent ( vtb->GetFrame ( ) );
  this->Tabbed3DViewIconButton->Create ( );
  this->Tabbed3DViewIconButton->SetReliefToFlat ( );
  this->Tabbed3DViewIconButton->SetBorderWidth ( 0 );
  this->Tabbed3DViewIconButton->SetOverReliefToNone ( );
  this->Tabbed3DViewIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetTabbed3DViewIcon ( ) );
  this->Tabbed3DViewIconButton->SetBalloonHelpString ( "Display a collection of 3D views in a notebook" );
  vtb->AddWidget ( this->Tabbed3DViewIconButton );

  // lightbox view icon
  this->LightBoxViewIconButton->SetParent ( vtb->GetFrame ( ));
  this->LightBoxViewIconButton->Create ( );
  this->LightBoxViewIconButton->SetReliefToFlat ( );
  this->LightBoxViewIconButton->SetBorderWidth ( 0 );
  this->LightBoxViewIconButton->SetOverReliefToNone ( );
  this->LightBoxViewIconButton->SetImageToIcon ( this->SlicerToolbarIcons->GetLightBoxViewIcon( ) );
  this->LightBoxViewIconButton->SetBalloonHelpString ( "Display a slice-matrix and no 3D view (not yet available)" );
  vtb->AddWidget ( this->LightBoxViewIconButton );

  this->MouseModeRadioButtons->SetParent (mmtb->GetFrame ( ) );
  this->MouseModeRadioButtons->Create ( );
  this->MouseModeRadioButtons->PackHorizontallyOn();

  vtkKWRadioButton *radiob = this->MouseModeRadioButtons->AddWidget ( vtkSlicerApplicationGUI::MouseSelect );
  radiob->SetReliefToFlat ( );
  radiob->SetOverReliefToNone ( );
  radiob->SetImageToIcon ( this->SlicerToolbarIcons->GetMousePickIcon ( ) );
  radiob->SetBalloonHelpString ( "Set the 3DViewer mouse mode to 'pick' (not yet available)" );
  radiob->SelectedStateOff ( );

  radiob = this->MouseModeRadioButtons->AddWidget ( vtkSlicerApplicationGUI::MousePut );
  radiob->SetReliefToFlat ( );
  radiob->SetOverReliefToNone ( );
  radiob->SetImageToIcon ( this->SlicerToolbarIcons->GetMousePlaceFiducialIcon ( ) );
  radiob->SetBalloonHelpString ( "Set the 3DViewer mouse mode to 'place fiducials' (not yet available)" );
  radiob->SelectedStateOn( );
  mmtb->AddWidget ( this->MouseModeRadioButtons );

  radiob = this->MouseModeRadioButtons->AddWidget ( vtkSlicerApplicationGUI::MouseTransform );
  radiob->SetReliefToFlat ( );
  radiob->SetOverReliefToNone ( );
  radiob->SetImageToIcon ( this->SlicerToolbarIcons->GetMouseTransformViewIcon ( ) );
  radiob->SetBalloonHelpString ( "Set the 3DViewer mouse mode to 'transform view' (not yet available)" );
  radiob->SelectedStateOff ( );
  

  tbs->ShowToolbar ( this->GetModulesToolbar ( ));
  tbs->ShowToolbar ( this->GetLoadSaveToolbar ( ));
  tbs->ShowToolbar ( this->GetViewToolbar ( ));
  tbs->ShowToolbar ( this->GetUndoRedoToolbar ( ));
  tbs->ShowToolbar ( this->GetMouseModeToolbar ( ));

}





