#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerSlicesControlGUI.h"
#include "vtkSlicerSlicesControlIcons.h"
#include "vtkSlicerSlicesGUI.h"
#include "vtkSlicerSliceGUI.h"
#include "vtkMRMLFiducialListNode.h"
#include "vtkSlicerTheme.h"

#include "vtkKWWidget.h"
#include "vtkKWScale.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWPushButton.h"
#include "vtkKWLabel.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenu.h"
#include "vtkKWTopLevel.h"
#include "vtkKWTkUtilities.h"

#include "vtkRenderer.h"

// uncomment in order to stub out the FOV Entries.
//#define FOV_ENTRIES_DEBUG


//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerSlicesControlGUI );
vtkCxxRevisionMacro ( vtkSlicerSlicesControlGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerSlicesControlGUI::vtkSlicerSlicesControlGUI ( )
{

  //--- ui for the SliceControlframe.
  this->SlicesControlIcons = vtkSlicerSlicesControlIcons::New ( );
  this->SliceFadeScale = vtkKWScale::New ( );
  this->ShowFgButton = vtkKWPushButton::New ( );
  this->ShowBgButton = vtkKWPushButton::New ( );
  this->ToggleFgBgButton = vtkKWPushButton::New ( );
  this->LabelOpacityButton = vtkKWPushButton::New ( );
  this->FeaturesVisibleButton = vtkKWMenuButton::New ( );
  this->FitToWindowButton = vtkKWPushButton::New ( );
  this->CrossHairButton = vtkKWMenuButton::New ( );
  this->SpatialUnitsButton = vtkKWMenuButton::New ( );
  this->AnnotationButton = vtkKWMenuButton::New ( );
  this->LabelOpacityTopLevel = vtkKWTopLevel::New ( );
  this->LabelOpacityScale = vtkKWScaleWithEntry::New ( );
  this->FieldOfViewButton = vtkKWPushButton::New();
#ifndef FOV_ENTRIES_DEBUG
  this->FieldOfViewTopLevel = vtkKWTopLevel::New ( );
  this->RedFOVEntry = vtkKWEntryWithLabel::New();
  this->YellowFOVEntry = vtkKWEntryWithLabel::New();
  this->GreenFOVEntry = vtkKWEntryWithLabel::New();
#endif
  this->RedSliceNode = NULL;
  this->YellowSliceNode = NULL;
  this->GreenSliceNode = NULL;

  this->RedSliceEvents = NULL;
  this->YellowSliceEvents = NULL;
  this->GreenSliceEvents = NULL;
  this->EntryUpdatePending = 0;
  this->SceneClosing = false;
  this->ProcessingMRMLEvent = 0;
  this->SliceInteracting = 0;

}


//---------------------------------------------------------------------------
vtkSlicerSlicesControlGUI::~vtkSlicerSlicesControlGUI ( )
{
  if ( this->SlicesControlIcons )
    {
    this->SlicesControlIcons->Delete ( );
    this->SlicesControlIcons = NULL;
    }
  if ( this->SliceFadeScale )
    {
    this->SliceFadeScale->SetParent ( NULL );
    this->SliceFadeScale->Delete ( );
    this->SliceFadeScale = NULL;    
    }
#ifndef FOV_ENTRIES_DEBUG
  if ( this->RedFOVEntry )
    {
    this->RedFOVEntry->SetParent ( NULL );
    this->RedFOVEntry->Delete();
    this->RedFOVEntry = NULL;
    }
  if ( this->GreenFOVEntry )
    {
    this->GreenFOVEntry->SetParent ( NULL );
    this->GreenFOVEntry->Delete();
    this->GreenFOVEntry = NULL;
    }
  if ( this->YellowFOVEntry )
    {
    this->YellowFOVEntry->SetParent ( NULL );
    this->YellowFOVEntry->Delete();
    this->YellowFOVEntry = NULL;
    }
  if ( this->FieldOfViewTopLevel )
    {
    this->FieldOfViewTopLevel->SetParent ( NULL );
    this->FieldOfViewTopLevel->Delete ( );    
    this->FieldOfViewTopLevel = NULL;
    }
#endif
  if ( this->FieldOfViewButton )
    {
    this->FieldOfViewButton->SetParent ( NULL );
    this->FieldOfViewButton->Delete();
    this->FieldOfViewButton = NULL;
    }
  if ( this->ShowFgButton )
    {
    this->ShowFgButton->SetParent ( NULL );
    this->ShowFgButton->Delete ( );
    this->ShowFgButton = NULL;    
    }
  if ( this->ShowBgButton )
    {
    this->ShowBgButton->SetParent ( NULL );
    this->ShowBgButton->Delete ( );
    this->ShowBgButton = NULL;    
    }
  if ( this->ToggleFgBgButton )
    {
    this->ToggleFgBgButton->SetParent ( NULL );
    this->ToggleFgBgButton->Delete ( );
    this->ToggleFgBgButton = NULL;
    }
  if ( this->LabelOpacityButton )
    {
    this->LabelOpacityButton->SetParent ( NULL );
    this->LabelOpacityButton->Delete ( );
    this->LabelOpacityButton = NULL;    
    }
  if ( this->FitToWindowButton )
    {
    this->FitToWindowButton->SetParent ( NULL);
    this->FitToWindowButton->Delete ( );
    this->FitToWindowButton = NULL;
    }
  if ( this->FeaturesVisibleButton )
    {
    this->FeaturesVisibleButton->SetParent ( NULL );
    this->FeaturesVisibleButton->Delete ( );
    this->FeaturesVisibleButton = NULL;
    }
  
  if ( this->CrossHairButton )
    {
    this->CrossHairButton->SetParent ( NULL );
    this->CrossHairButton->Delete ( );    
    this->CrossHairButton = NULL;
    }
  if ( this->SpatialUnitsButton )
    {
    this->SpatialUnitsButton->SetParent ( NULL );
    this->SpatialUnitsButton->Delete ( );
    this->SpatialUnitsButton = NULL;    
    }
  if ( this->AnnotationButton )
    {
    this->AnnotationButton->SetParent ( NULL );
    this->AnnotationButton->Delete ( );
    this->AnnotationButton = NULL;    
    }
  if ( this->LabelOpacityTopLevel )
    {
    this->LabelOpacityTopLevel->SetParent ( NULL );
    this->LabelOpacityTopLevel->Delete ( );    
    this->LabelOpacityTopLevel = NULL;
    }
  if ( this->LabelOpacityScale )
    {
    this->LabelOpacityScale->SetParent ( NULL );
    this->LabelOpacityScale->Delete ( );
    this->LabelOpacityScale = NULL;    
    }

  vtkSetAndObserveMRMLNodeMacro ( this->RedSliceNode, NULL );
  vtkSetAndObserveMRMLNodeMacro ( this->GreenSliceNode, NULL );
  vtkSetAndObserveMRMLNodeMacro ( this->YellowSliceNode, NULL );
  this->RemoveSliceEventObservers();
  this->SetRedSliceEvents(NULL);
  this->SetYellowSliceEvents(NULL);
  this->SetGreenSliceEvents(NULL);

    this->SetApplicationGUI ( NULL );
}


//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::TearDownGUI ( )
{
  this->SetAndObserveMRMLScene ( NULL );
  this->RemoveSliceEventObservers();
  this->SetApplicationGUI ( NULL );
  this->SetApplication ( NULL );
}



//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );
    os << indent << "SlicerSlicesControlGUI: " << this->GetClassName ( ) << "\n";
    os << indent << "SliceFadeScale: " << this->GetSliceFadeScale ( ) << "\n";
    os << indent << "ShowFgButton: " << this->GetShowFgButton ( ) << "\n";
    os << indent << "ShowBgButton: " << this->GetShowBgButton ( ) << "\n";
    os << indent << "ToggleFgBgButton: " << this->GetToggleFgBgButton ( ) << "\n";
    os << indent << "LabelOpacityButton: " << this->GetLabelOpacityButton ( ) << "\n";
    os << indent << "LabelOpacityScale: " << this->GetLabelOpacityScale ( ) << "\n";
    os << indent << "LabelOpacityTopLevel: " << this->GetLabelOpacityTopLevel ( ) << "\n";

//    os << indent << "GridButton: " << this->GetGridButton ( ) << "\n";
    os << indent << "AnnotationButton: " << this->GetAnnotationButton ( ) << "\n";
    os << indent << "SpatialUnitsButton: " << this->GetSpatialUnitsButton ( ) << "\n";
    os << indent << "CrossHairButton: " << this->GetCrossHairButton ( ) << "\n";
    os << indent << "FitToWindowButton: " << this->GetFitToWindowButton ( ) << "\n";
    os << indent << "FeaturesVisibleButton: " << this->GetFeaturesVisibleButton ( ) << "\n";
    os << indent << "SlicesControlIcons: " << this->GetSlicesControlIcons ( ) << "\n";
    os << indent << "FieldOfViewButton: " << this->GetFieldOfViewButton () << "\n";
#ifndef FOV_ENTRIES_DEBUG
    os << indent << "FieldOfViewTopLevel: " << this->GetFieldOfViewTopLevel ( ) << "\n";
    os << indent << "RedFOVEntry: " << this->GetRedFOVEntry ( ) << "\n";
    os << indent << "YellowFOVEntry: " << this->GetYellowFOVEntry ( ) << "\n";    
    os << indent << "GreenFOVEntry: " << this->GetGreenFOVEntry ( ) << "\n";
#endif
    os << indent << "ApplicationGUI: " << this->GetApplicationGUI ( ) << "\n";
  os << indent << "RedSliceNode: " << this->GetRedSliceNode(  ) << "\n";
  os << indent << "GreenSliceNode: " << this->GetGreenSliceNode(  ) << "\n";    
  os << indent << "YellowSliceNode: " << this->GetYellowSliceNode(  ) << "\n";    
  os << indent << "RedSliceEvents: " << this->GetRedSliceEvents(  ) << "\n";    
  os << indent << "GreenSliceEvents: " << this->GetGreenSliceEvents(  ) << "\n";    
  os << indent << "YellowSliceEvents: " << this->GetYellowSliceEvents(  ) << "\n";    
}


//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::UpdateFromMRML()
{

  // called:
  // 1. whenever any new node is created or deleted
  // Needs to remove old observers, put new
  // observers on the current camera and view,
  // repopulate the NavigationZoom widget's actors, etc.,
  // and rerender the NavigationZoom widget's view.
  this->UpdateSlicesFromMRML();
}



//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::UpdateSliceGUIInteractorStyles ( )
{
  // get all views from the scene
  // and observe active view.
  if (this->SceneClosing)
    {
    return;
    }

  // Find current SliceGUIs; if there are none, do nothing.
  if ( ( this->GetApplicationGUI()->GetMainSliceGUI("Red") == NULL ) ||
       ( this->GetApplicationGUI()->GetMainSliceGUI("Yellow") == NULL ) ||
       ( this->GetApplicationGUI()->GetMainSliceGUI("Green") == NULL ))
    {
    return;
    }

  // If the interactor and these references are out of sync...
  if ( ( this->GetApplicationGUI()->GetMainSliceGUI("Red")->GetSliceViewer()->
         GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle() != this->RedSliceEvents ) ||
       ( this->GetApplicationGUI()->GetMainSliceGUI("Yellow")->GetSliceViewer()->
         GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle() != this->YellowSliceEvents ) ||
       ( this->GetApplicationGUI()->GetMainSliceGUI("Green")->GetSliceViewer()->
         GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle() != this->GreenSliceEvents ) )
    {
    this->RemoveSliceEventObservers();
    this->SetRedSliceEvents(NULL );
    this->SetYellowSliceEvents(NULL );
    this->SetGreenSliceEvents(NULL );

    this->SetRedSliceEvents( vtkSlicerInteractorStyle::SafeDownCast(
                                                                 this->GetApplicationGUI()->
                                                                 GetMainSliceGUI("Red")->
                                                                 GetSliceViewer()->
                                                                 GetRenderWidget()->
                                                                 GetRenderWindowInteractor()->
                                                                 GetInteractorStyle() ));
    this->SetYellowSliceEvents( vtkSlicerInteractorStyle::SafeDownCast(
                                                                 this->GetApplicationGUI()->
                                                                 GetMainSliceGUI("Yellow")->
                                                                 GetSliceViewer()->
                                                                 GetRenderWidget()->
                                                                 GetRenderWindowInteractor()->
                                                                 GetInteractorStyle() ));
    this->SetGreenSliceEvents( vtkSlicerInteractorStyle::SafeDownCast(
                                                                 this->GetApplicationGUI()->
                                                                 GetMainSliceGUI("Green")->
                                                                 GetSliceViewer()->
                                                                 GetRenderWidget()->
                                                                 GetRenderWindowInteractor()->
                                                                 GetInteractorStyle() ));
    this->AddSliceEventObservers();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::UpdateSlicesFromMRML()
{
  if (this->SceneClosing)
    {
    return;
    }

  // update Slice nodes
  vtkMRMLSliceNode *node= NULL;
  vtkMRMLSliceNode *nodeRed= NULL;
  vtkMRMLSliceNode *nodeGreen= NULL;
  vtkMRMLSliceNode *nodeYellow= NULL;
  int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLSliceNode");
  for (int n=0; n<nnodes; n++)
    {
    node = vtkMRMLSliceNode::SafeDownCast (
          this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLSliceNode"));
    if (!strcmp(node->GetLayoutName(), "Red"))
      {
      nodeRed = node;
      }
    else if (!strcmp(node->GetLayoutName(), "Green"))
      {
      nodeGreen = node;
      }
    else if (!strcmp(node->GetLayoutName(), "Yellow"))
      {
      nodeYellow = node;
      }
    node = NULL;
    }

  // set and observe
  if (nodeRed != this->RedSliceNode)
    {
    vtkSetAndObserveMRMLNodeMacro(this->RedSliceNode, nodeRed);
    }
  if (nodeGreen != this->GreenSliceNode)
   {
   vtkSetAndObserveMRMLNodeMacro(this->GreenSliceNode, nodeGreen);
   }
  if (nodeYellow != this->YellowSliceNode)
   {
   vtkSetAndObserveMRMLNodeMacro(this->YellowSliceNode, nodeYellow);
   }

  // tidy up.
  nodeRed = NULL;
  nodeGreen = NULL;
  nodeYellow = NULL;
}


//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::RemoveGUIObservers ( )
{
  this->SliceFadeScale->RemoveObservers ( vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SliceFadeScale->RemoveObservers ( vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand );    
  this->LabelOpacityScale->GetScale()->RemoveObservers ( vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->LabelOpacityScale->GetScale()->RemoveObservers ( vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand );    
  this->ToggleFgBgButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ShowFgButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ShowBgButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->LabelOpacityButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->AnnotationButton->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SpatialUnitsButton->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->CrossHairButton->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->FeaturesVisibleButton->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->FieldOfViewButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );    
  this->FitToWindowButton->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );    
#ifndef FOV_ENTRIES_DEBUG
  this->RedFOVEntry->GetWidget()->RemoveObservers (vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->YellowFOVEntry->GetWidget()->RemoveObservers (vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GreenFOVEntry->GetWidget()->RemoveObservers (vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
#endif
  
}


//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::AddGUIObservers ( )
{

  this->SliceFadeScale->AddObserver ( vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SliceFadeScale->AddObserver ( vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ToggleFgBgButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->LabelOpacityScale->GetScale()->AddObserver ( vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->LabelOpacityScale->GetScale()->AddObserver ( vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ShowFgButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ShowBgButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->LabelOpacityButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->AnnotationButton->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SpatialUnitsButton->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->CrossHairButton->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->FeaturesVisibleButton->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->FieldOfViewButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );    
  this->FitToWindowButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );    
#ifndef FOV_ENTRIES_DEBUG
  this->RedFOVEntry->GetWidget()->AddObserver ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->YellowFOVEntry->GetWidget()->AddObserver ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GreenFOVEntry->GetWidget()->AddObserver ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
#endif
}



//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::ProcessGUIEvents ( vtkObject *caller,
                                          unsigned long event, void *callData )
{

  
  vtkKWPushButton *pushb = vtkKWPushButton::SafeDownCast (caller );
  vtkKWScale *scale = vtkKWScale::SafeDownCast (caller);
  vtkKWMenu *menu = vtkKWMenu::SafeDownCast (caller);
  vtkKWEntry *e = vtkKWEntry::SafeDownCast (caller);
  vtkSlicerInteractorStyle *istyle = vtkSlicerInteractorStyle::SafeDownCast (caller);

  if ( this->GetApplicationGUI() != NULL )
    {
      vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));
      vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( p->GetApplication() );
      if ( app != NULL )
        {
          // Process the Fade scale and button
          // -- set save state when manipulation starts
          // -- toggle the value if needed
          // -- adjust the Opacity of every composite node on every event
          if ( scale == this->SliceFadeScale && event == vtkKWScale::ScaleValueStartChangingEvent ||
               pushb == this->ToggleFgBgButton && event == vtkKWPushButton::InvokedEvent ||
               pushb == this->ShowFgButton && event == vtkKWPushButton::InvokedEvent ||
               pushb == this->ShowBgButton && event == vtkKWPushButton::InvokedEvent )
            {
              if (p->GetMRMLScene()) 
                {
                  p->GetMRMLScene()->SaveStateForUndo();
                }
            }

          if ( scale == this->SliceFadeScale && event == vtkKWScale::ScaleValueChangingEvent ||
               pushb == this->ToggleFgBgButton && event == vtkKWPushButton::InvokedEvent ||
               pushb == this->ShowFgButton && event == vtkKWPushButton::InvokedEvent ||
               pushb == this->ShowBgButton && event == vtkKWPushButton::InvokedEvent )
            {

            if ( pushb == this->ShowFgButton && event == vtkKWPushButton::InvokedEvent )
              {
                  this->SliceFadeScale->SetValue( 1.0 );
              }
            if ( pushb == this->ShowBgButton && event == vtkKWPushButton::InvokedEvent )
              {
                  this->SliceFadeScale->SetValue( 0.0 );
              }
            if ( pushb == this->ToggleFgBgButton && event == vtkKWPushButton::InvokedEvent ) 
                {
                  // it seems like this following statement should be handled
                  // in ProcessMRMLEvents, after the Composite node changes, no?
                  this->SliceFadeScale->SetValue( 1.0 - this->SliceFadeScale->GetValue() );
                }
              int i, nnodes = p->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLSliceCompositeNode");
              vtkMRMLSliceCompositeNode *cnode;
              for (i = 0; i < nnodes; i++)
                {
                  cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
                                                                   p->GetMRMLScene()->GetNthNodeByClass( i, "vtkMRMLSliceCompositeNode" ) );
                  cnode->SetForegroundOpacity( this->SliceFadeScale->GetValue() );
                }
            }

      // has interaction occured in the slice viewers?
      if ( istyle == this->RedSliceEvents || istyle == this->YellowSliceEvents || istyle == this->GreenSliceEvents)
        {
        // set interacting flag -- don't update gui until
        // interaction has stopped.
        if ( event == vtkCommand::RightButtonPressEvent ||
             event == vtkCommand::MiddleButtonPressEvent )
          {
          this->SliceInteracting = 1;
          }
        // interaction has stopped; update GUI's FOVentry widgets
        if ( event == vtkCommand::RightButtonReleaseEvent ||
             event == vtkCommand::MiddleButtonReleaseEvent )
          {
          this->SliceInteracting = 0;
          this->RequestFOVEntriesUpdate();
          }
        }

#ifndef FOV_ENTRIES_DEBUG
      double val;
      vtkMRMLSliceNode *snode;
      // RedFOVEntry
      if ( e == this->RedFOVEntry->GetWidget() && event == vtkKWEntry::EntryValueChangedEvent )
        {
        val = this->RedFOVEntry->GetWidget()->GetValueAsDouble();
        snode  = p->GetMainSliceGUI("Red")->GetSliceNode();
        p->GetMRMLScene()->SaveStateForUndo( snode );
        if ( val > 0 && snode && p )
          {
          this->FitFOVToBackground( val, 0 );
          }
        }
      // YellowFOVEntry
      if ( e == this->YellowFOVEntry->GetWidget() && event == vtkKWEntry::EntryValueChangedEvent )
        {
        val = this->YellowFOVEntry->GetWidget()->GetValueAsDouble();
        snode  = p->GetMainSliceGUI("Yellow")->GetSliceNode();
        p->GetMRMLScene()->SaveStateForUndo( snode );
        if ( val > 0 && snode && p )
          {   
          this->FitFOVToBackground( val, 1 );
          }
        }
      // GreenFOVEntry
      if ( e == this->GreenFOVEntry->GetWidget() && event == vtkKWEntry::EntryValueChangedEvent )
        {
        val = this->GreenFOVEntry->GetWidget()->GetValueAsDouble();
        snode  = p->GetMainSliceGUI("Green")->GetSliceNode();
        p->GetMRMLScene()->SaveStateForUndo( snode );
        if ( val > 0 && snode && p )
          {
          this->FitFOVToBackground( val, 2 );
          }
        }
#endif
      
      // Process the label Opacity scale 
      // -- set save state when manipulation starts
      // -- adjust the Opacity of every composite node on every event
      if ( scale == this->LabelOpacityScale->GetScale() && event == vtkKWScale::ScaleValueStartChangingEvent )
        {
        int i, nnodes = p->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLSliceCompositeNode");
        vtkMRMLSliceCompositeNode *cnode;
        if (p->GetMRMLScene()) 
          {
          for (i = 0; i < nnodes; i++)
            {
            cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
                                                             p->GetMRMLScene()->GetNthNodeByClass( i, "vtkMRMLSliceCompositeNode" ) );
            if ( cnode )
              {
              p->GetMRMLScene()->SaveStateForUndo( cnode );
              }
            }
          }
        }

      if ( scale == this->LabelOpacityScale->GetScale() && event == vtkKWScale::ScaleValueChangingEvent )
        {
        // adjust the Label opacity value for all slice composite nodes.
        int i, nnodes = p->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLSliceCompositeNode");
        vtkMRMLSliceCompositeNode *cnode;
        for (i = 0; i < nnodes; i++)
          {
          cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
                                                           p->GetMRMLScene()->GetNthNodeByClass( i, "vtkMRMLSliceCompositeNode" ) );
          cnode->SetLabelOpacity( this->LabelOpacityScale->GetValue() );
          }
        }

      if ( pushb == this->LabelOpacityButton && event == vtkKWPushButton::InvokedEvent )
        {
        this->PopUpLabelOpacityScaleAndEntry();
        }
      else if ( pushb == this->FieldOfViewButton && event == vtkKWPushButton::InvokedEvent )
        {
        this->PopUpFieldOfViewEntries ( );
        }

      //
      // PushButtons:
      //

      if ( pushb == this->FitToWindowButton && event == vtkKWPushButton::InvokedEvent )
        {
        this->FitSlicesToBackground();
        }

      //
      // MenuButtons:
      //
      if ( menu == this->AnnotationButton->GetMenu() && event == vtkKWMenu::MenuItemInvokedEvent )
        {
        this->ModifyAnnotationMode ( );
        }
      else if ( menu == this->SpatialUnitsButton->GetMenu() && event == vtkKWMenu::MenuItemInvokedEvent )
        {
        this->ModifySpatialUnitsMode ( );
        }
      else if ( menu == this->CrossHairButton->GetMenu() && event == vtkKWMenu::MenuItemInvokedEvent )
        {
        this->ModifyCrossHairMode ( );
        }
      else if ( menu == this->FeaturesVisibleButton->GetMenu() && event == vtkKWMenu::MenuItemInvokedEvent )
        {
        this->ModifyVisibility( );
        }
        }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::AddSliceEventObservers()
{
  
  if ( this->GetApplicationGUI() != NULL )
    {
    if ( this->RedSliceEvents != NULL )
      {
      this->RedSliceEvents->AddObserver ( vtkCommand::RightButtonPressEvent, this->GUICallbackCommand );
      this->RedSliceEvents->AddObserver ( vtkCommand::RightButtonReleaseEvent, this->GUICallbackCommand );
      this->RedSliceEvents->AddObserver ( vtkCommand::MiddleButtonPressEvent, this->GUICallbackCommand );
      this->RedSliceEvents->AddObserver ( vtkCommand::MiddleButtonReleaseEvent, this->GUICallbackCommand );
      }
    if ( this->YellowSliceEvents != NULL )
      {
      this->YellowSliceEvents->AddObserver ( vtkCommand::RightButtonPressEvent, this->GUICallbackCommand );
      this->YellowSliceEvents->AddObserver ( vtkCommand::RightButtonReleaseEvent, this->GUICallbackCommand );
      this->YellowSliceEvents->AddObserver ( vtkCommand::MiddleButtonPressEvent, this->GUICallbackCommand );
      this->YellowSliceEvents->AddObserver ( vtkCommand::MiddleButtonReleaseEvent, this->GUICallbackCommand );
      }
    if ( this->GreenSliceEvents != NULL )
      {
      this->GreenSliceEvents->AddObserver ( vtkCommand::RightButtonPressEvent, this->GUICallbackCommand );
      this->GreenSliceEvents->AddObserver ( vtkCommand::RightButtonReleaseEvent, this->GUICallbackCommand );
      this->GreenSliceEvents->AddObserver ( vtkCommand::MiddleButtonPressEvent, this->GUICallbackCommand );
      this->GreenSliceEvents->AddObserver ( vtkCommand::MiddleButtonReleaseEvent, this->GUICallbackCommand );
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::RemoveSliceEventObservers()
{
  if ( this->GetApplicationGUI() != NULL )
    {
    if ( this->RedSliceEvents != NULL )
      {
      this->RedSliceEvents->RemoveObservers ( vtkCommand::RightButtonPressEvent, this->GUICallbackCommand );
      this->RedSliceEvents->RemoveObservers ( vtkCommand::RightButtonReleaseEvent, this->GUICallbackCommand );
      this->RedSliceEvents->RemoveObservers ( vtkCommand::MiddleButtonPressEvent, this->GUICallbackCommand );
      this->RedSliceEvents->RemoveObservers ( vtkCommand::MiddleButtonReleaseEvent, this->GUICallbackCommand );
      }
    if ( this->YellowSliceEvents != NULL )
      {
      this->YellowSliceEvents->RemoveObservers ( vtkCommand::RightButtonPressEvent, this->GUICallbackCommand );
      this->YellowSliceEvents->RemoveObservers ( vtkCommand::RightButtonReleaseEvent, this->GUICallbackCommand );
      this->YellowSliceEvents->RemoveObservers ( vtkCommand::MiddleButtonPressEvent, this->GUICallbackCommand );
      this->YellowSliceEvents->RemoveObservers ( vtkCommand::MiddleButtonReleaseEvent, this->GUICallbackCommand );
      }
    if ( this->GreenSliceEvents != NULL )
      {
      this->GreenSliceEvents->RemoveObservers ( vtkCommand::RightButtonPressEvent, this->GUICallbackCommand );
      this->GreenSliceEvents->RemoveObservers ( vtkCommand::RightButtonReleaseEvent, this->GUICallbackCommand );
      this->GreenSliceEvents->RemoveObservers ( vtkCommand::MiddleButtonPressEvent, this->GUICallbackCommand );
      this->GreenSliceEvents->RemoveObservers ( vtkCommand::MiddleButtonReleaseEvent, this->GUICallbackCommand );
      }
    }
}


// adjust the node's field of view to match the extent of current background volume
//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::FitFOVToBackground( double fov, int viewer )
{
  if ( viewer != 0 && viewer != 1 && viewer != 2 )
    {
    return;
    }
  
  // reference the slice node and composite node and sliceGUI
  if ( this->GetApplicationGUI() != NULL )
    {
    vtkSlicerApplicationGUI *appGUI = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));
    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( appGUI->GetApplication() );
    if ( appGUI != NULL )
      {
      vtkMRMLSliceNode *sliceNode = NULL;
      vtkMRMLSliceCompositeNode *compositeNode = NULL;
      vtkMRMLScalarVolumeNode *backgroundNode = NULL;
      vtkSlicerSliceGUI *sgui = NULL;
      if ( viewer == 0 )
        {
        sliceNode = this->RedSliceNode;
        compositeNode = appGUI->GetApplicationLogic()->GetSliceLogic("Red")->GetSliceCompositeNode();
        sgui = appGUI->GetMainSliceGUI("Red");
        }
      else if ( viewer == 1 )
        {
        sliceNode = this->YellowSliceNode;
        compositeNode = appGUI->GetApplicationLogic()->GetSliceLogic("Yellow")->GetSliceCompositeNode();
        //compositeNode = appGUI->GetMainSliceLogic1()->GetSliceCompositeNode();
        sgui = appGUI->GetMainSliceGUI("Yellow");
        }
      else if ( viewer == 2 )
        {
        sliceNode = this->GreenSliceNode;
        compositeNode = appGUI->GetApplicationLogic()->GetSliceLogic("Green")->GetSliceCompositeNode();
        sgui = appGUI->GetMainSliceGUI("Green");
        }
      appGUI->GetMRMLScene()->SaveStateForUndo( sliceNode );
      
      if ( !sgui )
        {
        return;
        }
      
      if ( !sliceNode || !compositeNode )
        {
        return;
        }
      
      // get viewer's width and height. we may be using a LightBox
      // display, so base width and height on renderer0 in the SliceViewer.
      int width, height;
      
      vtkRenderer *ren=sgui->GetSliceViewer()->GetRenderWidget()->GetRenderer();
      width = ren->GetSize()[0];
      height = ren->GetSize()[1];
      
      
      // get backgroundNode  and imagedata
      backgroundNode =
        vtkMRMLScalarVolumeNode::SafeDownCast (
          appGUI->GetMRMLScene()->GetNodeByID( compositeNode->GetBackgroundVolumeID() ));
      vtkImageData *backgroundImage;
      if ( !backgroundNode || ! (backgroundImage = backgroundNode->GetImageData()) )
        {
        return;
        }
      
      int dimensions[3];
      double rasDimensions[4];
      double doubleDimensions[4];
      vtkMatrix4x4 *ijkToRAS = vtkMatrix4x4::New();
      
      // what are the actual dimensions of the imagedata?
      backgroundImage->GetDimensions(dimensions);
      doubleDimensions[0] = dimensions[0];
      doubleDimensions[1] = dimensions[1];
      doubleDimensions[2] = dimensions[2];
      doubleDimensions[3] = 0.0;
      backgroundNode->GetIJKToRASMatrix (ijkToRAS);
      ijkToRAS->MultiplyPoint (doubleDimensions, rasDimensions);
      ijkToRAS->Delete();
      ijkToRAS = NULL;

      // and what are their slice dimensions?
      vtkMatrix4x4 *rasToSlice = vtkMatrix4x4::New();
      double sliceDimensions[4];
      rasToSlice->DeepCopy(sliceNode->GetSliceToRAS());
      rasToSlice->SetElement(0, 3, 0.0);
      rasToSlice->SetElement(1, 3, 0.0);
      rasToSlice->SetElement(2, 3, 0.0);
      rasToSlice->Invert();
      rasToSlice->MultiplyPoint( rasDimensions, sliceDimensions );
      rasToSlice->Delete();
      rasToSlice = NULL;
      
      double fovh, fovv;
      // which is bigger, slice viewer width or height?
      // assign user-specified fov to smaller slice window
      // dimension
      if ( width < height )
        {
        fovh = fov;
        fovv = fov * height/width;
        }
      else
        {
        fovv = fov;
        fovh = fov * width/height;
        }
      
      // we want to compute the slice dimensions of the
      // user-specified fov (note that the slice node's z field of
      // view is NOT changed)
      sliceNode->SetFieldOfView(fovh, fovv, sliceNode->GetFieldOfView()[2] );

      vtkMatrix4x4 *sliceToRAS = vtkMatrix4x4::New();
      sliceToRAS->DeepCopy(sliceNode->GetSliceToRAS());
      sliceNode->GetSliceToRAS()->DeepCopy(sliceToRAS);
      sliceToRAS->Delete();
      sliceToRAS = NULL;
      sliceNode->UpdateMatrices( );
      }
    }
}

//----------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::ModifySpatialUnitsMode ( )
{
  vtkSlicerApplicationGUI *appGUI;
  vtkMRMLSliceCompositeNode *cnode;
  
  if ( this->GetApplicationGUI() )
    {
    appGUI = vtkSlicerApplicationGUI::SafeDownCast (this->GetApplicationGUI());
    
    // first save the state of all slice composite nodes for undo
    int nnodes = appGUI->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLSliceCompositeNode");
    vtkCollection *nodes = vtkCollection::New();
    for (int i = 0; i < nnodes; i++)
      {
      cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
             appGUI->GetMRMLScene()->GetNthNodeByClass( i, "vtkMRMLSliceCompositeNode" ) );
      if ( cnode )
        {
        nodes->AddItem (cnode );
        }
    }
    this->MRMLScene->SaveStateForUndo ( nodes );
    nodes->Delete ( );

    // then change the annotation mode for all slice composite nodes
    for (int i = 0; i < nnodes; i++)
      {
      cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
             appGUI->GetMRMLScene()->GetNthNodeByClass( i, "vtkMRMLSliceCompositeNode" ) );
      if ( this->GetSpatialUnitsButton()->GetMenu()->GetItemSelectedState("XYZ") == 1 )
        {
        if ( cnode->GetAnnotationSpace() != vtkMRMLSliceCompositeNode::XYZ)
          {
          cnode->SetAnnotationSpace ( vtkMRMLSliceCompositeNode::XYZ );
          }
        }
      else if (this->GetSpatialUnitsButton()->GetMenu()->GetItemSelectedState("IJK") == 1 )
        {
        if ( cnode->GetAnnotationSpace() != vtkMRMLSliceCompositeNode::IJK)
          {
          cnode->SetAnnotationSpace( vtkMRMLSliceCompositeNode::IJK );
          }
        }
      else if ( this->GetSpatialUnitsButton()->GetMenu()->GetItemSelectedState( "RAS" ) == 1)
        {
        if ( cnode->GetAnnotationSpace() != vtkMRMLSliceCompositeNode::RAS)
          {
          cnode->SetAnnotationSpace( vtkMRMLSliceCompositeNode::RAS );
          }
        }
      else if ( this->GetSpatialUnitsButton()->GetMenu()->GetItemSelectedState( "IJK and RAS" ) == 1)
        {
        if ( cnode->GetAnnotationSpace() != vtkMRMLSliceCompositeNode::IJKAndRAS)
          {
          cnode->SetAnnotationSpace( vtkMRMLSliceCompositeNode::IJKAndRAS );
          }
        }
      }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::ModifyVisibility ( )
{
  vtkSlicerApplicationGUI *appGUI;
  vtkMRMLSliceCompositeNode *cnode;
//  vtkMRMLViewNode *vnode;
//  vtkMRMLSelectionNode *snode;
//  vtkMRMLFiducialListNode *fnode;
  
  if ( this->GetApplicationGUI() )
    {
    appGUI = vtkSlicerApplicationGUI::SafeDownCast (this->GetApplicationGUI());
    int state;
    int nnodes;
    vtkCollection *nodes = vtkCollection::New();
    
/*
    // first save the state of whichever view node is current and make change to node.
    nnodes = appGUI->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLViewNode");
    for (int i = 0; i < nnodes; i++)
      {
      vnode = vtkMRMLViewNode::SafeDownCast (
                                                       appGUI->GetMRMLScene()->GetNthNodeByClass( i, "vtkMRMLViewNode" ) );
      if (vnode->GetActive())
        {
        break;
        }
      }

    // Make ViewNode and FiducialListNodes match.
    //TODO: remove this and use above when there are multiple views and one is marked as active.
    vnode = vtkMRMLViewNode::SafeDownCast (
                                           appGUI->GetMRMLScene()->GetNthNodeByClass( 0, "vtkMRMLViewNode" ) );
    if (vnode != NULL )
      {
      // fiducial points show or hide
      state =  this->GetFeaturesVisibleButton()->GetMenu()->GetItemSelectedState("Fiducial points");
      if ( state != vnode->GetFiducialsVisible () )
        {
        nodes->AddItem (vnode);
        vnode->SetFiducialsVisible (state);
        nnodes = appGUI->GetMRMLScene()->GetNumberOfNodesByClass ("vtkMRMLFiducialListNode");
        for ( i = 0; i<nnodes; i++)
          {
          fnode = vtkMRMLFiducialListNode::SafeDownCast (appGUI->GetMRMLScene()->GetNthNodeByClass( i, "vtkMRMLFiducialListNode"));
          nodes->AddItem (fnode);
          fnode->SetVisibility ( state );
          }
        }
      // fiducial labels show or hide
      state =  this->GetFeaturesVisibleButton()->GetMenu()->GetItemSelectedState("Fiducial labels");
      if ( state != vnode->GetFiducialLabelsVisible() )
        {
        nodes->AddItem (vnode);
        vnode->SetFiducialLabelsVisible (state);
        nnodes = appGUI->GetMRMLScene()->GetNumberOfNodesByClass ("vtkMRMLFiducialListNode");
        for ( i = 0; i<nnodes; i++)
          {
          fnode = vtkMRMLFiducialListNode::SafeDownCast (appGUI->GetMRMLScene()->GetNthNodeByClass( i, "vtkMRMLFiducialListNode"));
          nodes->AddItem (fnode);
          fnode->SetVisibility ( state );
          }
        }
      }
*/
      
    // SliceCompositeNodes
    // add the state of all slice composite nodes for undo
    nnodes = appGUI->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLSliceCompositeNode");
    for (int i = 0; i < nnodes; i++)
      {
      cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
             appGUI->GetMRMLScene()->GetNthNodeByClass( i, "vtkMRMLSliceCompositeNode" ) );
      if ( cnode )
        {
        nodes->AddItem (cnode );
        }
    }
    this->MRMLScene->SaveStateForUndo ( nodes );


    nodes->Delete ( );

    // then change the annotation mode for all slice composite nodes
    for (int i = 0; i < nnodes; i++)
      {
      cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
                                                       appGUI->GetMRMLScene()->GetNthNodeByClass( i, "vtkMRMLSliceCompositeNode" ) );
      
      state =  this->GetFeaturesVisibleButton()->GetMenu()->GetItemSelectedState("Fiducial points");
      if ( cnode->GetFiducialVisibility () != state )
        {
//        cnode->SetFiducialVisibility( state);
        }
      state =  this->GetFeaturesVisibleButton()->GetMenu()->GetItemSelectedState("Fiducial labels");
      if ( cnode->GetFiducialLabelVisibility() != state )
        {
//        cnode->SetFiducialLabelVisibility ( state );
        }
      state =  this->GetFeaturesVisibleButton()->GetMenu()->GetItemSelectedState("Foreground grid");
      if ( cnode->GetForegroundGrid ( ) != state )
        {
        cnode->SetForegroundGrid (  state );
        }
      state = this->GetFeaturesVisibleButton()->GetMenu()->GetItemSelectedState("Background grid");
      if ( cnode->GetBackgroundGrid () != state )
        {
        cnode->SetBackgroundGrid ( state);
        }
      state = this->GetFeaturesVisibleButton()->GetMenu()->GetItemSelectedState( "Label grid" );
      if ( cnode->GetLabelGrid ( ) != state )
        {
        cnode->SetLabelGrid ( state );
         }
      }
    }

}



//----------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::ModifyCrossHairMode ( )
{
  vtkSlicerApplicationGUI *appGUI;
  vtkMRMLSliceCompositeNode *cnode;
  
  if ( this->GetApplicationGUI() )
    {
    appGUI = vtkSlicerApplicationGUI::SafeDownCast (this->GetApplicationGUI());
    
    // first save the state of all slice composite nodes for undo
    int nnodes = appGUI->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLSliceCompositeNode");
    vtkCollection *nodes = vtkCollection::New();
    for (int i = 0; i < nnodes; i++)
      {
      cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
             appGUI->GetMRMLScene()->GetNthNodeByClass( i, "vtkMRMLSliceCompositeNode" ) );
      if ( cnode )
        {
        nodes->AddItem (cnode );
        }
    }
    this->MRMLScene->SaveStateForUndo ( nodes );
    nodes->Delete ( );

    // then change the annotation mode for all slice composite nodes
    for (int i = 0; i < nnodes; i++)
      {
      cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
                                                       appGUI->GetMRMLScene()->GetNthNodeByClass( i, "vtkMRMLSliceCompositeNode" ) );
      if ( this->GetCrossHairButton()->GetMenu()->GetItemSelectedState("No crosshair") == 1 )
        {
        if ( cnode->GetCrosshairMode() != vtkMRMLSliceCompositeNode::NoCrosshair )
          {
          cnode->SetCrosshairMode ( vtkMRMLSliceCompositeNode::NoCrosshair );
          }
        }
      else if (this->GetCrossHairButton()->GetMenu()->GetItemSelectedState ("Basic crosshair") == 1)
        {
        if ( cnode->GetCrosshairMode() != vtkMRMLSliceCompositeNode::ShowBasic )
          {
          cnode->SetCrosshairMode ( vtkMRMLSliceCompositeNode::ShowBasic );
          }
        }
      else if (this->GetCrossHairButton()->GetMenu()->GetItemSelectedState ("Basic + intersection") == 1)
        {
        if ( cnode->GetCrosshairMode() != vtkMRMLSliceCompositeNode::ShowIntersection )
          {
          cnode->SetCrosshairMode ( vtkMRMLSliceCompositeNode::ShowIntersection );
          }
        }
      else if (this->GetCrossHairButton()->GetMenu()->GetItemSelectedState("Basic + hashmarks") ==1 )
        {
        if ( cnode->GetCrosshairMode() != vtkMRMLSliceCompositeNode::ShowHashmarks )
          {
          cnode->SetCrosshairMode ( vtkMRMLSliceCompositeNode::ShowHashmarks );
          }
        }
      else if (this->GetCrossHairButton()->GetMenu()->GetItemSelectedState( "Basic + hashmarks + intersection") ==1 )
        {
        if ( cnode->GetCrosshairMode() != vtkMRMLSliceCompositeNode::ShowAll )
          {
          cnode->SetCrosshairMode ( vtkMRMLSliceCompositeNode::ShowAll );
          }
        }      
      else if ( this->GetCrossHairButton()->GetMenu()->GetItemSelectedState("Jump slice") == 1)
        {
        if ( cnode->GetCrosshairMode() != vtkMRMLSliceCompositeNode::JumpSlice )
          {
          cnode->SetCrosshairBehavior ( vtkMRMLSliceCompositeNode::JumpSlice );
          }
        }      
      else if ( this->GetCrossHairButton()->GetMenu()->GetItemSelectedState("Jump slice") == 0)
        {
        if ( cnode->GetCrosshairMode() != vtkMRMLSliceCompositeNode::Normal )
          {
          cnode->SetCrosshairBehavior ( vtkMRMLSliceCompositeNode::Normal );
          }
        }      
      }
    }
}



//----------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::ModifyAnnotationMode ( )
{
  vtkSlicerApplicationGUI *appGUI;
  vtkMRMLSliceCompositeNode *cnode;
  
  if ( this->GetApplication() )
    {
    appGUI = vtkSlicerApplicationGUI::SafeDownCast (this->GetApplicationGUI());
    
    // first save the state of all slice composite nodes for undo
    int nnodes = appGUI->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLSliceCompositeNode");
    vtkCollection *nodes = vtkCollection::New();
    for (int i = 0; i < nnodes; i++)
      {
      cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
             appGUI->GetMRMLScene()->GetNthNodeByClass( i, "vtkMRMLSliceCompositeNode" ) );
      if ( cnode )
        {
        nodes->AddItem (cnode );
        }
    }
    this->MRMLScene->SaveStateForUndo ( nodes );
    nodes->Delete ( );

    // then change the annotation mode for all slice composite nodes
    for (int i = 0; i < nnodes; i++)
      {
      cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
             appGUI->GetMRMLScene()->GetNthNodeByClass( i, "vtkMRMLSliceCompositeNode" ) );
      if ( this->GetAnnotationButton()->GetMenu()->GetItemSelectedState("None") == 1)
        {
        if ( cnode->GetAnnotationMode() != vtkMRMLSliceCompositeNode::NoAnnotation )
          {
        cnode->SetAnnotationMode ( vtkMRMLSliceCompositeNode::NoAnnotation );
          }
        }
      else if (this->GetAnnotationButton()->GetMenu()->GetItemSelectedState( "Show all") == 1)
        {
        if ( cnode->GetAnnotationMode() != vtkMRMLSliceCompositeNode::All )
          {
        cnode->SetAnnotationMode ( vtkMRMLSliceCompositeNode::All );
          }
        }
      else if (this->GetAnnotationButton()->GetMenu()->GetItemSelectedState( "Show label values only") == 1)
        {
        if ( cnode->GetAnnotationMode() != vtkMRMLSliceCompositeNode::LabelValuesOnly )
          {
          cnode->SetAnnotationMode ( vtkMRMLSliceCompositeNode::LabelValuesOnly );
          }
        }
      else if ( this->GetAnnotationButton()->GetMenu()->GetItemSelectedState ("Show voxel and label values only") == 1)
        {
        if ( cnode->GetAnnotationMode() != vtkMRMLSliceCompositeNode::LabelAndVoxelValuesOnly )
          {
          cnode->SetAnnotationMode ( vtkMRMLSliceCompositeNode::LabelAndVoxelValuesOnly );
          }
        }
      }
    }
}



//----------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::FitSlicesToBackground ( )
{

  vtkSlicerApplication *app;
  vtkSlicerSliceGUI *sgui;
  vtkSlicerSlicesGUI *ssgui;
    
  // find the sliceGUI for this controller
  if ( this->GetApplication() )
  {
          app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
          ssgui = vtkSlicerSlicesGUI::SafeDownCast ( app->GetModuleGUIByName ("Slices") );
          if ( ssgui != NULL )
          {
                  // First save all SliceNodes for undo:
                  char *layoutname = NULL;
                  vtkCollection *nodes = vtkCollection::New();
                  int nSliceGUI = ssgui->GetNumberOfSliceGUI();
                  for (int i = 0; i < nSliceGUI; i++)
                  {
                          if (i == 0)
                          {
                                  sgui = ssgui->GetFirstSliceGUI();
                                  layoutname = ssgui->GetFirstSliceGUILayoutName();
                          }
                          else
                          {
                                  sgui = ssgui->GetNextSliceGUI(layoutname);
                                  layoutname = ssgui->GetNextSliceGUILayoutName(layoutname);
                          }
                          nodes->AddItem ( sgui->GetSliceNode ( ) );
                  }
                  this->MRMLScene->SaveStateForUndo ( nodes );
                  nodes->Delete ( );

                  // Now fit all Slices to background
                  int w, h;
                  for (int i = 0; i < nSliceGUI; i++)
                  {
                          if (i == 0)
                          {
                                  sgui = ssgui->GetFirstSliceGUI();
                                  layoutname = ssgui->GetFirstSliceGUILayoutName();
                          }
                          else
                          {
                                  sgui = ssgui->GetNextSliceGUI(layoutname);
                                  layoutname = ssgui->GetNextSliceGUILayoutName(layoutname);
                          }
                          //w = sgui->GetSliceViewer()->GetRenderWidget ( )->GetWidth();
                          //h = sgui->GetSliceViewer()->GetRenderWidget ( )->GetHeight();
                          sscanf(
                                  this->Script("winfo width %s", 
                                  sgui->GetSliceViewer()->GetRenderWidget ( )->GetWidgetName()), 
                                  "%d", &w);
                          sscanf(
                                  this->Script("winfo height %s", 
                                  sgui->GetSliceViewer()->GetRenderWidget ( )->GetWidgetName()), 
                                  "%d", &h);
                          sgui->GetLogic()->FitSliceToAll ( w, h );
                          sgui->GetSliceNode()->UpdateMatrices( );
                          this->RequestFOVEntriesUpdate();
                  }

                  //ssgui->GetSliceGUICollection()->InitTraversal();
      //sgui = vtkSlicerSliceGUI::SafeDownCast ( ssgui->GetSliceGUICollection()->GetNextItemAsObject() );
      //vtkCollection *nodes = vtkCollection::New();
      //while ( sgui != NULL )
      //  {
      //  nodes->AddItem ( sgui->GetSliceNode ( ) );
      //  sgui = vtkSlicerSliceGUI::SafeDownCast ( ssgui->GetSliceGUICollection()->GetNextItemAsObject() );
      //  }
      //this->MRMLScene->SaveStateForUndo ( nodes );
      //nodes->Delete ( );

      //// Now fit all Slices to background
      //ssgui->GetSliceGUICollection()->InitTraversal();
      //sgui = vtkSlicerSliceGUI::SafeDownCast ( ssgui->GetSliceGUICollection()->GetNextItemAsObject() );
      //int w, h;
      //while ( sgui != NULL )
      //  {
      //  //w = sgui->GetSliceViewer()->GetRenderWidget ( )->GetWidth();
      //  //h = sgui->GetSliceViewer()->GetRenderWidget ( )->GetHeight();
      //  sscanf(
      //    this->Script("winfo width %s", 
      //        sgui->GetSliceViewer()->GetRenderWidget ( )->GetWidgetName()), 
      //    "%d", &w);
      //  sscanf(
      //    this->Script("winfo height %s", 
      //        sgui->GetSliceViewer()->GetRenderWidget ( )->GetWidgetName()), 
      //    "%d", &h);
      //  sgui->GetLogic()->FitSliceToAll ( w, h );
      //  sgui->GetSliceNode()->UpdateMatrices( );
      //  this->RequestFOVEntriesUpdate();
      //  sgui = vtkSlicerSliceGUI::SafeDownCast ( ssgui->GetSliceGUICollection()->GetNextItemAsObject() );
      //  }
      }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::ProcessLogicEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
{

}




//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, void *callData )
{
  if (this->ProcessingMRMLEvent != 0 )
    {
    return;
    }
  this->ProcessingMRMLEvent = event;
  vtkDebugMacro("processing event " << event);
   
  // has a node been added or deleted?
  if ( vtkMRMLScene::SafeDownCast(caller) == this->MRMLScene 
       && (event == vtkMRMLScene::NodeAddedEvent || event == vtkMRMLScene::NodeRemovedEvent ) )
    {
    this->UpdateFromMRML();
    }

  // is the scene closing?
  if (event == vtkMRMLScene::SceneCloseEvent )
    {
    this->SceneClosing = true;
    }
  else 
    {
    this->SceneClosing = false;
    }

  vtkMRMLSliceNode *slnode = vtkMRMLSliceNode::SafeDownCast ( caller );

  
  // update FOV entry widgets to match node, if
  // we're not interactively zooming the slices.
  // (too slow to track MRML during interaction)
  if ( !this->SliceInteracting)
    {
    if ( vtkMRMLSliceNode::SafeDownCast ( caller) )
      {
      this->RequestFOVEntriesUpdate();
      }
    }
}




//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::RequestFOVEntriesUpdate ( )
{
  if ( this->GetEntryUpdatePending() )
    {
    return;
    }
  this->SetEntryUpdatePending(1);
  this->Script("after idle \"%s FOVEntriesUpdate\"", this->GetTclName() );
}



//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::FOVEntriesUpdate ( )
{
  // check fov entries... see if they match MRML.
  // if not, update them.
  double fov, fovX, fovY;
  if ( this->RedSliceNode != NULL )
    {
    fovX = this->RedSliceNode->GetFieldOfView()[0];
    fovY = this->RedSliceNode->GetFieldOfView()[1];
    if ( fovX < fovY )
      {
      fov = fovX;
      }
    else
      {
      fov = fovY;
      }
#ifndef FOV_ENTRIES_DEBUG
    if ( this->RedFOVEntry->GetWidget()->GetValueAsDouble() != fov )
      {
      this->RedFOVEntry->GetWidget()->SetValueAsDouble ( fov );
      }
    }
#endif
  
  if ( this->YellowSliceNode != NULL )
    {
    fovX = this->YellowSliceNode->GetFieldOfView()[0];
    fovY = this->YellowSliceNode->GetFieldOfView()[1];
    if ( fovX < fovY )
      {
      fov = fovX;
      }
    else
      {
      fov = fovY;
      }
#ifndef FOV_ENTRIES_DEBUG
    if ( this->YellowFOVEntry->GetWidget()->GetValueAsDouble() != fov )
      {
      this->YellowFOVEntry->GetWidget()->SetValueAsDouble ( fov );
      }
    }
#endif
  if ( this->GreenSliceNode != NULL )
    {
    fovX = this->GreenSliceNode->GetFieldOfView()[0];
    fovY = this->GreenSliceNode->GetFieldOfView()[1];
    if ( fovX < fovY )
      {
      fov = fovX;
      }
    else
      {
      fov = fovY;
      }
#ifndef FOV_ENTRIES_DEBUG
    if ( this->GreenFOVEntry->GetWidget()->GetValueAsDouble() != fov )
      {
      this->GreenFOVEntry->GetWidget()->SetValueAsDouble ( fov );
      }
    }
#endif
  this->SetEntryUpdatePending(0);
}







  

//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::Enter ( )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::Exit ( )
{
    // Fill in
}


//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::SetApplicationGUI ( vtkSlicerApplicationGUI *appGUI )
{
  this->ApplicationGUI = appGUI;
}


//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::BuildAnnotationMenu ( )
{
  this->AnnotationButton->GetMenu()->DeleteAllItems ( );
  this->AnnotationButton->GetMenu()->AddRadioButton ( "None");
  this->AnnotationButton->GetMenu()->AddRadioButton ( "Show all");
  this->AnnotationButton->GetMenu()->AddRadioButton ( "Show label values only" );
  this->AnnotationButton->GetMenu()->AddRadioButton ( "Show voxel and label values only" );
  this->AnnotationButton->GetMenu()->SelectItem ( "Show all" );
  this->AnnotationButton->GetMenu()->AddSeparator ( );
  this->AnnotationButton->GetMenu()->AddCommand ("close");
}

//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::BuildCrossHairMenu ( )
{
  this->CrossHairButton->GetMenu()->DeleteAllItems ( );
  this->CrossHairButton->GetMenu()->AddRadioButton ("No crosshair" );
  this->CrossHairButton->GetMenu()->AddRadioButton ("Basic crosshair" );  
  this->CrossHairButton->GetMenu()->AddRadioButton ("Basic + intersection" );
  this->CrossHairButton->GetMenu()->AddRadioButton ("Basic + hashmarks" );
  this->CrossHairButton->GetMenu()->AddRadioButton ("Basic + hashmarks + intersection" );
  this->CrossHairButton->GetMenu()->AddCheckButton ("Jump slice" );
  this->CrossHairButton->GetMenu()->DeselectItem ( "Jump slice" );
  this->CrossHairButton->GetMenu()->SelectItem ("No crosshair");
  this->CrossHairButton->GetMenu()->AddSeparator ( );
  this->CrossHairButton->GetMenu()->AddCommand ("close");
}



//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::BuildVisibilityMenu ( )
{
  this->FeaturesVisibleButton->GetMenu()->DeleteAllItems ( );
  this->FeaturesVisibleButton->GetMenu()->AddCheckButton ( "Fiducial points");
  this->FeaturesVisibleButton->GetMenu()->AddCheckButton ( "Fiducial labels");
  this->FeaturesVisibleButton->GetMenu()->AddCheckButton ( "Foreground grid");
  this->FeaturesVisibleButton->GetMenu()->AddCheckButton ( "Background grid");
  this->FeaturesVisibleButton->GetMenu()->AddCheckButton ( "Label grid");
  this->FeaturesVisibleButton->GetMenu()->AddSeparator ( );
  this->FeaturesVisibleButton->GetMenu()->AddCommand ("close");
  this->FeaturesVisibleButton->GetMenu()->SetItemStateToDisabled ("Fiducial points");
  this->FeaturesVisibleButton->GetMenu()->SetItemStateToDisabled ("Fiducial labels");
  this->FeaturesVisibleButton->GetMenu()->SetItemStateToDisabled ("Foreground grid");
  this->FeaturesVisibleButton->GetMenu()->SetItemStateToDisabled ("Background grid");
//  this->FeaturesVisibleButton->GetMenu()->SelectItem ("Fiducial points");
//  this->FeaturesVisibleButton->GetMenu()->SelectItem ("Fiducial labels");
  this->FeaturesVisibleButton->GetMenu()->DeselectItem ("Foreground grid");
  this->FeaturesVisibleButton->GetMenu()->DeselectItem ("Background grid");
  this->FeaturesVisibleButton->GetMenu()->SelectItem ("Label grid");

}


//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::BuildSpacesMenu ( )
{
  // what other spaces should we represent here?
  // TODO: this selection should get saved to the registry.
  this->SpatialUnitsButton->GetMenu()->DeleteAllItems ( );
  this->SpatialUnitsButton->GetMenu()->AddRadioButton ( "XYZ");
  this->SpatialUnitsButton->GetMenu()->AddRadioButton ( "IJK");  
  this->SpatialUnitsButton->GetMenu()->AddRadioButton ( "RAS");
  this->SpatialUnitsButton->GetMenu()->AddRadioButton ( "IJK and RAS");
  this->SpatialUnitsButton->GetMenu()->SelectItem ( "IJK and RAS" );
  this->SpatialUnitsButton->GetMenu()->AddSeparator();
  this->SpatialUnitsButton->GetMenu()->AddCommand ( "close");
}



//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::PopUpFieldOfViewEntries ( )
{
#ifndef FOV_ENTRIES_DEBUG
    if ( !this->FieldOfViewButton || !this->FieldOfViewButton->IsCreated())
    {
    return;
    }

  // Get the position of the mouse, the position and size of the push button,
  // the size of the scale.

  int x, y, py, ph, sx, sy;
  vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI ( );
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(appGUI->GetApplication() );
  
  vtkKWTkUtilities::GetMousePointerCoordinates(this->FieldOfViewButton, &x, &y);
  vtkKWTkUtilities::GetWidgetCoordinates(this->FieldOfViewButton, NULL, &py);
  vtkKWTkUtilities::GetWidgetSize(this->FieldOfViewButton, NULL, &ph);
  vtkKWTkUtilities::GetWidgetRelativeCoordinates(this->RedFOVEntry, &sx, &sy);
 
  // Place the scale so that the slider is coincident with the x mouse position
  // and just below the push button
  x -= sx;
  if (py <= y && y <= (py + ph -1))
    {
    y = py + ph - 3;
    }
  else
    {
    y -= sy;
    }

  this->FieldOfViewTopLevel->SetPosition(x, y);
  app->ProcessPendingEvents();
  this->FieldOfViewTopLevel->DeIconify();
  this->FieldOfViewTopLevel->Raise();
#endif
}


//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::HideFieldOfViewEntries ( )
{
#ifndef FOV_ENTRIES_DEBUG
  if ( !this->FieldOfViewTopLevel )
    {
    return;
    }
  this->FieldOfViewTopLevel->Withdraw();
#endif
}


//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::HideLabelOpacityScaleAndEntry ( )
{
  if ( !this->LabelOpacityTopLevel )
    {
    return;
    }
  this->LabelOpacityTopLevel->Withdraw();
}


//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::PopUpLabelOpacityScaleAndEntry ( )
{
  if ( !this->LabelOpacityButton || !this->LabelOpacityButton->IsCreated())
    {
    return;
    }

  // Get the position of the mouse, the position and size of the push button,
  // the size of the scale.

  int x, y, py, ph, scx, scy, sx, sy;
  vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI ( );
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(appGUI->GetApplication() );
  
  vtkKWTkUtilities::GetMousePointerCoordinates(this->LabelOpacityButton, &x, &y);
  vtkKWTkUtilities::GetWidgetCoordinates(this->LabelOpacityButton, NULL, &py);
  vtkKWTkUtilities::GetWidgetSize(this->LabelOpacityButton, NULL, &ph);
  vtkKWTkUtilities::GetWidgetRelativeCoordinates(this->LabelOpacityScale->GetScale(), &sx, &sy);
  sscanf(this->Script("%s coords %g", this->LabelOpacityScale->GetScale()->GetWidgetName(),
                      this->LabelOpacityScale->GetScale()->GetValue()), "%d %d", &scx, &scy);
 
  // Place the scale so that the slider is coincident with the x mouse position
  // and just below the push button
  x -= sx + scx;
  if (py <= y && y <= (py + ph -1))
    {
    y = py + ph - 3;
    }
  else
    {
    y -= sy + scy;
    }

  this->LabelOpacityTopLevel->SetPosition(x, y);
  app->ProcessPendingEvents();
  this->LabelOpacityTopLevel->DeIconify();
  this->LabelOpacityTopLevel->Raise();
}

//---------------------------------------------------------------------------
void vtkSlicerSlicesControlGUI::BuildGUI ( vtkKWFrame *appF )
{

  vtkSlicerApplicationGUI *p = this->GetApplicationGUI ( );
  //--- Populate the Slice Control Frame
  if ( p != NULL )
    {
    if ( p->GetApplication() != NULL )
      {
      vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( p->GetApplication() );
      appF->SetReliefToGroove();

      vtkKWFrame *FgBgFrame = vtkKWFrame::New ( );
      FgBgFrame->SetParent ( appF);
      FgBgFrame->Create ( );
        
      this->ShowFgButton->SetParent (FgBgFrame);
      this->ShowFgButton->Create ( );
      this->ShowFgButton->SetBorderWidth ( 0 );
      this->ShowFgButton->SetImageToIcon ( this->SlicesControlIcons->GetFgIcon() );
      this->ShowFgButton->SetBalloonHelpString ( "Show Foreground Layer in Slice Viewers." );

      this->ShowBgButton->SetParent (FgBgFrame);
      this->ShowBgButton->Create ( );
      this->ShowBgButton->SetBorderWidth ( 0 );
      this->ShowBgButton->SetImageToIcon ( this->SlicesControlIcons->GetBgIcon() );
      this->ShowBgButton->SetBalloonHelpString ( "Show Background Layer in Slice Viewers." );
    
      this->ToggleFgBgButton->SetParent (FgBgFrame);
      this->ToggleFgBgButton->Create ( );
      this->ToggleFgBgButton->SetBorderWidth ( 0 );
      this->ToggleFgBgButton->SetImageToIcon ( this->SlicesControlIcons->GetToggleFgBgIcon() );
      this->ToggleFgBgButton->SetBalloonHelpString ( "Toggle visibility of Foreground and Background Layers in Slice Viewers." );

      //--- make scale for sliding slice visibility in the SliceViewers
      this->SliceFadeScale->SetParent ( FgBgFrame );
      this->SliceFadeScale->Create ( );
      this->SliceFadeScale->SetRange (0.0, 1.0);
      this->SliceFadeScale->SetResolution ( 0.01 );
      this->SliceFadeScale->SetValue ( 0.0 );
      this->SliceFadeScale->SetLength ( 120 );
      this->SliceFadeScale->SetOrientationToHorizontal ( );
      this->SliceFadeScale->ValueVisibilityOff ( );
      this->SliceFadeScale->SetBalloonHelpString ( "Scale fades between Foreground and Background Layers." );

      this->FitToWindowButton->SetParent (appF);
      this->FitToWindowButton->Create ( );
      this->FitToWindowButton->SetBorderWidth ( 0 );
      this->FitToWindowButton->SetImageToIcon ( this->SlicesControlIcons->GetFitToWindowIcon() );
      this->FitToWindowButton->SetBalloonHelpString ( "Fits image data to the window in all Slice Viewers." );

      this->AnnotationButton->SetParent (appF);
      this->AnnotationButton->Create ( );
      this->AnnotationButton->SetBorderWidth ( 0 );
      this->AnnotationButton->SetImageToIcon ( this->SlicesControlIcons->GetAnnotationIcon() );
      this->AnnotationButton->IndicatorVisibilityOff ( );
      this->AnnotationButton->SetBalloonHelpString ( "Choose annotation options for all Slice Viewers." );

      this->SpatialUnitsButton->SetParent (appF);
      this->SpatialUnitsButton->Create ( );
      this->SpatialUnitsButton->SetBorderWidth ( 0 );
      this->SpatialUnitsButton->SetImageToIcon ( this->SlicesControlIcons->GetSpatialUnitsIcon() );
      this->SpatialUnitsButton->IndicatorVisibilityOff ( );
      this->SpatialUnitsButton->SetBalloonHelpString ( "Assign axis labels in all Slice Viewers." );

      this->CrossHairButton->SetParent (appF);
      this->CrossHairButton->Create ( );
      this->CrossHairButton->SetBorderWidth ( 0 );
      this->CrossHairButton->SetImageToIcon ( this->SlicesControlIcons->GetCrossHairIcon() );
      this->CrossHairButton->IndicatorVisibilityOff ( );
      this->CrossHairButton->SetBalloonHelpString ( "Choose among crosshair options for all Slice Viewers." );

      this->FeaturesVisibleButton->SetParent (appF);
      this->FeaturesVisibleButton->Create ( );
      this->FeaturesVisibleButton->SetBorderWidth ( 0 );
      this->FeaturesVisibleButton->SetImageToIcon ( this->SlicesControlIcons->GetFeaturesVisibleIcon() );
      this->FeaturesVisibleButton->IndicatorVisibilityOff ( );
      this->FeaturesVisibleButton->SetBalloonHelpString ( "Toggle visibility of various Slice Viewer features." );

      //--- Popup Scale with Entry (displayed when user clicks LabelOpacityButton
      //--- LabelOpacityButton, LabelOpacityScale and its entry will be observed
      //--- and their events handled in ProcessGUIEvents;
      //--- the pop-up and hide behavior of the latter two will be managed locally
      //--- in the GUI.
      //--- TODO: make a SlicerWidget that handles this behavior. Leave event?
      this->LabelOpacityTopLevel->SetApplication ( this->GetApplication ( ) );
      this->LabelOpacityTopLevel->SetMasterWindow ( this->LabelOpacityButton );
      this->LabelOpacityTopLevel->Create ( );
      this->LabelOpacityTopLevel->HideDecorationOn ( );
      this->LabelOpacityTopLevel->Withdraw ( );
      this->LabelOpacityTopLevel->SetBorderWidth ( 2 );
      this->LabelOpacityTopLevel->SetReliefToGroove ( );
      //--- create frame
      vtkKWFrame *PopUpFrame = vtkKWFrame::New ( );
      PopUpFrame->SetParent ( this->LabelOpacityTopLevel );
      PopUpFrame->Create ( );
      PopUpFrame->SetBinding ( "<Leave>", this, "HideLabelOpacityScaleAndEntry" );
      this->Script ( "pack %s -side left -anchor w -padx 2 -pady 2 -fill x -fill y -expand n", PopUpFrame->GetWidgetName ( ) );   
      // Scale and entry packed in the pop-up toplevel's frame
      this->LabelOpacityScale->SetParent ( PopUpFrame );
      this->LabelOpacityScale->Create ( );
      this->LabelOpacityScale->SetRange ( 0.0, 1.0 );
      this->LabelOpacityScale->SetResolution ( 0.01 );
      this->LabelOpacityScale->GetScale()->SetLabelText ( "" );
      this->LabelOpacityScale->GetScale()->ValueVisibilityOff ( );
      this->LabelOpacityScale->SetValue ( 1.0 );
      this->Script ( "pack %s -side left -anchor w -padx 1 -pady 3 -expand n", this->LabelOpacityScale->GetWidgetName ( ) );
      this->LabelOpacityButton->SetParent (appF);
      this->LabelOpacityButton->Create ( );
      this->LabelOpacityButton->SetBorderWidth ( 0 );
      this->LabelOpacityButton->SetImageToIcon ( this->SlicesControlIcons->GetAllLabelOpacityIcon() );
      this->LabelOpacityButton->SetBalloonHelpString ( "Popup scale to adjust opacity of Label Layer in all Slice Viewers." );

#ifndef FOV_ENTRIES_DEBUG
      this->FieldOfViewTopLevel->SetApplication ( this->GetApplication ( ) );
      this->FieldOfViewTopLevel->SetMasterWindow ( this->FieldOfViewButton );
      this->FieldOfViewTopLevel->Create ( );
      this->FieldOfViewTopLevel->HideDecorationOn ( );
      this->FieldOfViewTopLevel->Withdraw ( );
      this->FieldOfViewTopLevel->SetBorderWidth ( 2 );
      this->FieldOfViewTopLevel->SetReliefToGroove ( );
      vtkKWFrame *PopUpFrame2 = vtkKWFrame::New ( );
      PopUpFrame2->SetParent ( this->FieldOfViewTopLevel );
      PopUpFrame2->Create ( );
      PopUpFrame2->SetBinding ( "<Leave>", this, "HideFieldOfViewEntries" );
      this->Script ( "pack %s -side left -anchor w -padx 2 -pady 2 -fill x -fill y -expand n", PopUpFrame2->GetWidgetName ( ) );   
      // create red viewer's entry
      this->RedFOVEntry->SetParent (PopUpFrame2);
      this->RedFOVEntry->Create ( );
      this->RedFOVEntry->SetBalloonHelpString("Set the Red Slice Viewer's field of view in mm");
      this->RedFOVEntry->GetWidget()->SetWidth (5);
      this->RedFOVEntry->GetWidget()->SetBackgroundColor ( app->GetSlicerTheme()->GetSlicerColors()->SliceGUIRed );
      this->RedFOVEntry->GetWidget()->SetValueAsDouble (250);
      this->RedFOVEntry->GetLabel()->SetText ("Red Slice FOV");
      this->RedFOVEntry->GetWidget()->SetCommandTrigger (vtkKWEntry::TriggerOnReturnKey );
      this->RedFOVEntry->SetLabelPositionToRight();
      // create green viewer's entry
      this->GreenFOVEntry->SetParent (PopUpFrame2);
      this->GreenFOVEntry->Create ( );
      this->GreenFOVEntry->SetBalloonHelpString("Set the Green Slice Viewer's field of view in mm");
      this->GreenFOVEntry->GetWidget()->SetWidth (5);
      this->GreenFOVEntry->GetWidget()->SetBackgroundColor ( app->GetSlicerTheme()->GetSlicerColors()->SliceGUIGreen );
      this->GreenFOVEntry->GetWidget()->SetValueAsDouble (250);
      this->GreenFOVEntry->GetLabel()->SetText ("Green Slice FOV");
      this->GreenFOVEntry->GetWidget()->SetCommandTrigger (vtkKWEntry::TriggerOnReturnKey );
      this->GreenFOVEntry->SetLabelPositionToRight();
      // create Yellow viewer's entry
      this->YellowFOVEntry->SetParent (PopUpFrame2);
      this->YellowFOVEntry->Create ( );
      this->YellowFOVEntry->SetBalloonHelpString("Set the Yellow Slice Viewer's field of view in mm");
      this->YellowFOVEntry->GetWidget()->SetWidth (5);
      this->YellowFOVEntry->GetWidget()->SetBackgroundColor ( app->GetSlicerTheme()->GetSlicerColors()->SliceGUIYellow );
      this->YellowFOVEntry->GetWidget()->SetValueAsDouble (250);
      this->YellowFOVEntry->GetLabel()->SetText ("Yellow Slice FOV");
      this->YellowFOVEntry->GetWidget()->SetCommandTrigger (vtkKWEntry::TriggerOnReturnKey );
      this->YellowFOVEntry->SetLabelPositionToRight();

      // make presentation consistent by providing a 'close popup' option
      vtkKWLabel *closer = vtkKWLabel::New();
      closer->SetParent (PopUpFrame2);
      closer->Create();
      closer->SetText ("close");
      this->Script ( "pack %s %s %s -side top -anchor w -padx 4 -pady 3 -expand n",
                     this->GetRedFOVEntry()->GetWidgetName ( ),
                     this->GetYellowFOVEntry()->GetWidgetName(),
                     this->GetGreenFOVEntry()->GetWidgetName() );
      this->Script ( "pack %s -side top -anchor c -padx 4 -pady 3 -expand n", closer->GetWidgetName());
      closer->SetBinding ( "<Button-1>", this, "HideFieldOfViewEntries" );
      closer->Delete();
#endif
      this->FieldOfViewButton->SetParent (appF);
      this->FieldOfViewButton->Create ( );
      this->FieldOfViewButton->SetBorderWidth ( 0 );
      this->FieldOfViewButton->SetImageToIcon ( this->SlicesControlIcons->GetFieldOfViewIcon() );
      this->FieldOfViewButton->SetBalloonHelpString ( "Set the field of view (in mm) in a Slice Window." );
#ifdef FOV_ENTRIES_DEBUG
      this->FieldOfViewButton->SetStateToDisabled ( );
#endif

      //--- pack everything up from left to right.
      this->Script ( "pack %s -side left -anchor w -padx 2 -pady 3 -expand n", this->FeaturesVisibleButton->GetWidgetName ( ) );
      this->Script ( "pack %s -side left -anchor w -padx 2 -pady 3 -expand n", this->FitToWindowButton->GetWidgetName ( ) );
      this->Script ( "pack %s -side left -anchor w -padx 2 -pady 3 -expand n", this->LabelOpacityButton->GetWidgetName ( ) );
      this->Script ( "pack %s -side left -anchor w -padx 2 -pady 3 -expand n", this->AnnotationButton->GetWidgetName ( ) );
      this->Script ( "pack %s -side left -anchor w -padx 2 -pady 3 -expand n", this->CrossHairButton->GetWidgetName ( ) );
      this->Script ( "pack %s -side left -anchor w -padx 2 -pady 3 -expand n", this->SpatialUnitsButton->GetWidgetName ( ) );
      this->Script ( "pack %s -side left -anchor w -padx 2 -pady 3 -expand n", this->FieldOfViewButton->GetWidgetName ( ) );

      this->Script ( "pack %s -side left -ipadx 10 -pady 1 -fill x -expand n", FgBgFrame->GetWidgetName ( ) );
      this->Script ( "pack %s -side right -anchor e -padx 0 -pady 3 -expand n", this->ShowFgButton->GetWidgetName ( ) );
      this->Script ( "pack %s -side right -anchor e -fill x -padx 0 -pady 3 -expand n", this->SliceFadeScale->GetWidgetName ( ) );
      this->Script ( "pack %s -side right -anchor e -padx 0 -pady 3 -expand n", this->ShowBgButton->GetWidgetName ( ) );
      this->Script ( "pack %s -side right -anchor e -padx 2 -pady 3 -expand n", this->ToggleFgBgButton->GetWidgetName ( ) );

      //--- populate menus
      this->BuildAnnotationMenu ( );
      this->BuildCrossHairMenu ( );
      this->BuildSpacesMenu ( );
      this->BuildVisibilityMenu ();
      PopUpFrame->Delete ( );
      PopUpFrame2->Delete ( );
      FgBgFrame->Delete ( );
      }
    }

}





