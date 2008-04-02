#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkCornerAnnotation.h"
#include "vtkImageViewer.h"
#include "vtkRenderWindow.h"
#include "vtkImageActor.h"

#include "vtkSlicerInteractorStyle.h"
#include "vtkSlicerSliceGUI.h"
#include "vtkSlicerSliceViewer.h"
#include "vtkSlicerSliceControllerWidget.h"
#include "vtkSlicerSliceLogic.h"
#include "vtkSlicerApplication.h"
#include "vtkMRMLSliceNode.h"

#include "vtkKWApplication.h"
#include "vtkKWEvent.h"
#include "vtkKWWidget.h"
#include "vtkKWFrame.h"
#include "vtkKWScale.h"
#include "vtkKWEntry.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWPushButton.h"
#include "vtkKWRenderWidget.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerSliceGUI);
vtkCxxRevisionMacro(vtkSlicerSliceGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerSliceGUI::vtkSlicerSliceGUI (  ) {

    // Create objects and set null pointers
    this->SliceViewer = vtkSlicerSliceViewer::New ( );
    this->SliceController = vtkSlicerSliceControllerWidget::New ( );
    this->SliceGUIFrame = vtkKWFrame::New ( );
    this->Logic = NULL;
    this->SliceNode = NULL;
    this->CurrentGUIEvent = NULL;
    this->GrabID = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerSliceGUI::~vtkSlicerSliceGUI ( ) {

    
    this->RemoveGUIObservers ();
    this->RemoveMRMLObservers();

    // Unpack and set parents to be NULL
    this->SliceController->SetParent ( NULL );
    this->SliceViewer->SetParent ( NULL );

    if ( this->SliceViewer )
        {
            this->SliceViewer->SetParent(NULL );
            this->SliceViewer->Delete ( );
            this->SliceViewer = NULL;
        }
    if ( this->SliceController )
        {
            this->SliceController->RemoveWidgetObservers ( );
            this->SliceController->SetParent(NULL );
            this->SliceController->Delete ( );
            this->SliceController = NULL;
        }

    if ( this->SliceGUIFrame )
      {
        this->SliceGUIFrame->SetParent(NULL );
        this->SliceGUIFrame->Delete ( );
        this->SliceGUIFrame = NULL;
      }

    // Remove observers and references 
    this->SetModuleLogic ( NULL );
    vtkSetMRMLNodeMacro ( this->SliceNode, NULL );

    // give the slice viewer code a chance to free any vtk objects
    // it allocated.  This can be called with no impact on other 
    // slice gui instances, since the tcl code automatically re-initializes
    // if the event handler is called again.
    this->Script("SliceViewerShutdown %s", this->GetTclName());
}



//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );
    os << indent << "SlicerSliceGUI: " << this->GetClassName ( ) << "\n";
    os << indent << "SliceGUIFrame: " << this->GetSliceGUIFrame ( ) << "\n";
    os << indent << "SliceViewer: " << this->GetSliceViewer ( ) << "\n";
    os << indent << "SliceController: " << this->GetSliceController ( ) << "\n";
    os << indent << "Logic: " << this->GetLogic ( ) << "\n";
    os << indent << "SliceNode: " << this->GetSliceNode ( ) << "\n";
}





//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::AddGUIObservers ( ) {

  this->RemoveGUIObservers();

#if 0
  // this version doesn't handle the startup case correctly (windows don't appear)
  // so we observe the interactor style instead
  vtkRenderWindowInteractor *rwi = this->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();
  rwi->SetInteractorStyle (NULL);
  rwi->AddObserver ( vtkCommand::AnyEvent, (vtkCommand *)this->GUICallbackCommand );
#endif

  // make a user interactor style to process our events
  // look at the InteractorStyle to get our events
  vtkKWRenderWidget *rw = this->GetSliceViewer()->GetRenderWidget();
  vtkRenderWindowInteractor *rwi = rw->GetRenderWindowInteractor();
  if (rwi)
    {
    vtkSlicerInteractorStyle *iStyle = vtkSlicerInteractorStyle::New();
    rwi->SetInteractorStyle (iStyle);
    iStyle->AddObserver ( vtkCommand::AnyEvent, (vtkCommand *)this->GUICallbackCommand );
    iStyle->Delete();
    
    // These events tell us where the key and mouse wheel events will be sent, 
    // so we need to track them.  They only come from the KW layer, not from VTK
    rw->AddObserver ( vtkKWEvent::FocusInEvent, (vtkCommand *)this->GUICallbackCommand );
    rw->AddObserver ( vtkKWEvent::FocusOutEvent, (vtkCommand *)this->GUICallbackCommand );
    }
}



//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::RemoveGUIObservers ( ) {

#if 0
  // this version doesn't get ConfigureEvents
  // so we observe the interactor style instead
  vtkRenderWindowInteractor *rwi = this->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();
  rwi->RemoveObserver ( (vtkCommand *)this->GUICallbackCommand );
#endif
   
  vtkKWRenderWidget *rw = this->GetSliceViewer()->GetRenderWidget();
  vtkRenderWindowInteractor *rwi = rw->GetRenderWindowInteractor();
  if (rwi)
    {
    vtkSlicerInteractorStyle *istyle = vtkSlicerInteractorStyle::SafeDownCast(rwi->GetInteractorStyle());
    if (istyle)
      {
      istyle->RemoveObservers ( vtkCommand::AnyEvent, (vtkCommand *)this->GUICallbackCommand );
      }

    rw->RemoveObservers ( vtkKWEvent::FocusInEvent, (vtkCommand *)this->GUICallbackCommand );
    rw->RemoveObservers ( vtkKWEvent::FocusOutEvent, (vtkCommand *)this->GUICallbackCommand );
    }
}

//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::SetGUICommandAbortFlag ( int flag )
{
  this->GetGUICallbackCommand()->SetAbortFlag(flag);
}


//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::ProcessGUIEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
  vtkKWRenderWidget *rw = 
        vtkKWRenderWidget::SafeDownCast (caller);
  vtkKWGenericRenderWindowInteractor *rwi = 
        vtkKWGenericRenderWindowInteractor::SafeDownCast (caller);
  vtkSlicerInteractorStyle *iStyle = 
        vtkSlicerInteractorStyle::SafeDownCast (caller);

  vtkMRMLScene *mrml = this->GetApplicationLogic()->GetMRMLScene();

  if (mrml == NULL ) 
    {
    return;
    }

  // handle events from the Interactor Style
  if (iStyle == this->GetSliceViewer()->GetRenderWidget()->
                        GetRenderWindowInteractor()->GetInteractorStyle() &&
      this->GetLogic() != NULL)
    {
    this->SetCurrentGUIEvent( vtkCommand::GetStringFromEventId(event) );
    this->InvokeEvent (event, NULL);
    this->SetCurrentGUIEvent( "" ); // avoid extra processing of same event

    if ( !this->GUICallbackCommand->GetAbortFlag() )
      {
      this->Script( "SliceViewerHandleEvent %s %s", 
        this->GetTclName(), vtkCommand::GetStringFromEventId(event) );
      }
    }

  // handle events from the Render Widget
  if (rw == this->GetSliceViewer()->GetRenderWidget() &&
      this->GetLogic() != NULL)
    {
    this->SetCurrentGUIEvent( vtkKWEvent::GetStringFromEventId(event) );
    // don't invoke the real event, since there's no way for the tcl
    // layer to set an observer on it.  It won't matter because the callback
    // will look at the CurrentGUIEvent string to decide what to do.
    this->InvokeEvent (vtkCommand::UserEvent, NULL);
    this->SetCurrentGUIEvent( "" ); // avoid extra processing of same event

    if ( !this->GUICallbackCommand->GetAbortFlag() )
      {
      this->Script( "SliceViewerHandleEvent %s %s", 
        this->GetTclName(), vtkKWEvent::GetStringFromEventId(event) );
      }
    }

}





//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::ProcessLogicEvents ( vtkObject *caller,
                                                unsigned long event, void *callData )
{
  if ( !caller )
    {
    return;
    }

  // process Logic changes
  vtkSlicerSliceLogic *sliceLogic = vtkSlicerSliceLogic::SafeDownCast(caller);
  vtkSlicerApplicationLogic *appLogic = vtkSlicerApplicationLogic::SafeDownCast ( caller );
  
  if ( appLogic == this->GetApplicationLogic ( ) )
    {
    // Nothing yet
    }
  if ( sliceLogic == this->GetLogic ( ) ) 
    {
    vtkMRMLSliceNode *snode = this->GetLogic()->GetSliceNode();
    this->SetAndObserveMRMLNode( snode );
    this->SetupViewerAndController();

    // sliceLogic contains the pipeline that create viewer's input, so
    // assume we need to set the image data and render
    vtkSlicerSliceViewer *sliceViewer = this->GetSliceViewer( );

    if (snode != NULL)
      {
      sliceViewer->ChangeLayout( snode->GetLayoutGridRows(), 
                                 snode->GetLayoutGridColumns() );
      }

    vtkKWRenderWidget *rw = sliceViewer->GetRenderWidget ();
    //sliceViewer->GetImageMapper()->SetInput ( sliceLogic->GetImageData( ) );

//     if ( sliceLogic->GetImageData() != NULL )
//       {
//       double spacing[3];
//       spacing[0] = 10.;
//       spacing[1] = 10.;
//       spacing[2] = 10.;

//       sliceLogic->GetImageData()->SetSpacing( spacing );
//       sliceLogic->UpdateImageData();
//       }

    sliceViewer->SetImageData( sliceLogic->GetImageData() );

    // add poly data (glyphs)
    vtkPolyDataCollection *PolyDataCollection = vtkPolyDataCollection::New();
    vtkCollection *LookupTableCollection = vtkCollection::New();
    sliceLogic->GetPolyDataAndLookUpTableCollections(PolyDataCollection,
                                                      LookupTableCollection);
    sliceViewer->SetCoordinatedPolyDataAndLookUpTableCollections( sliceLogic->GetPolyDataCollection(), sliceLogic->GetLookupTableCollection()  );
    PolyDataCollection->RemoveAllItems();
    PolyDataCollection->Delete();
    LookupTableCollection->RemoveAllItems();
    LookupTableCollection->Delete();
    
    // add mrml display node observers
    this->RemoveMRMLObservers();
    std::vector< vtkMRMLDisplayNode*> dnodes = sliceLogic->GetPolyDataDisplayNodes();
    for (unsigned int i=0; i< dnodes.size(); i++)
      {
      vtkMRMLDisplayNode* dnode = dnodes[i];
      if (!dnode->HasObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand))
        {
        dnode->Register(this);
        dnode->AddObserver ( vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand );
        this->DisplayNodes.push_back(dnode);
        }
      }
    //rw->ResetCamera ( );
    sliceViewer->RequestRender ( );
    }
}



//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::ProcessMRMLEvents ( vtkObject *caller,
                                               unsigned long event, void *callData )
{
  if ( !caller )
    {
    return;
    }

  // process mrml changes
  vtkMRMLDisplayNode* dnode= vtkMRMLDisplayNode::SafeDownCast(caller);

  
  if ( this->GetLogic() && dnode )
    {
    vtkSlicerSliceLogic *sliceLogic = this->GetLogic ( );

    vtkSlicerSliceViewer *sliceViewer = this->GetSliceViewer( );
    vtkKWRenderWidget *rw = sliceViewer->GetRenderWidget ();
    // add poly data (glyphs)
    vtkPolyDataCollection *PolyDataCollection = vtkPolyDataCollection::New();
    vtkCollection *LookupTableCollection = vtkCollection::New();
    sliceLogic->GetPolyDataAndLookUpTableCollections(PolyDataCollection,
                                                      LookupTableCollection);
    sliceViewer->SetCoordinatedPolyDataAndLookUpTableCollections( sliceLogic->GetPolyDataCollection(), sliceLogic->GetLookupTableCollection()  );
    PolyDataCollection->RemoveAllItems();
    PolyDataCollection->Delete();
    LookupTableCollection->RemoveAllItems();
    LookupTableCollection->Delete();

      /*
    vtkMRMLSliceNode *snode = this->GetLogic()->GetSliceNode();

    this->SetAndObserveMRMLNode( snode );
    this->SetupViewerAndController();
    vtkMRMLSliceCompositeNode *scnode = this->GetLogic()->GetSliceCompositeNode();
    this->GetSliceController()->SetSliceNode (snode);
    this->GetSliceController()->SetSliceCompositeNode (scnode);
    */
    sliceViewer->RequestRender ( );
    }
}


void vtkSlicerSliceGUI::RemoveMRMLObservers ( )
{
  for(unsigned int i=0; i<this->DisplayNodes.size(); i++)
    {
    vtkMRMLDisplayNode* dnode = this->DisplayNodes[i];
    if (dnode)
      {
      dnode->RemoveObservers ( vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand );
      dnode->UnRegister(this);
      }
    }
  this->DisplayNodes.clear();
}

//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::Enter ( )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::Exit ( )
{
    // Fill in
}

//----------------------------------------------------------------------------
void vtkSlicerSliceGUI::BuildGUI ( vtkKWFrame *f )
{

  if ( this->GetApplication() != NULL )
    {
      vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication ( ) );
      vtkMRMLScene *mrml = this->GetApplicationLogic()->GetMRMLScene();

      this->SliceGUIFrame->SetApplication ( app );
      this->SliceGUIFrame->SetParent ( f );
      this->SliceGUIFrame->Create ( );

      this->SliceController->SetApplication ( app );
      this->SliceController->SetAndObserveMRMLScene ( mrml );
      this->SliceController->SetParent ( this->SliceGUIFrame );
      this->SliceController->Create (  );

      this->SliceViewer->SetApplication ( app );
      this->SliceViewer->SetParent ( this->SliceGUIFrame );
      this->SliceViewer->Create (  );

       this->PackGUI( f );
    }
}

//-----------------------------------------------------------------------------
void vtkSlicerSliceGUI::BuildGUI ( vtkKWFrame *f, double *c )
{

  if ( this->GetApplication() != NULL )
    {
      vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication ( ) );
      vtkMRMLScene *mrml = this->GetApplicationLogic()->GetMRMLScene();

      this->SliceGUIFrame->SetApplication ( app );
      this->SliceGUIFrame->SetParent ( f );
      this->SliceGUIFrame->Create ( );

      this->SliceController->SetApplication ( app );
      this->SliceController->SetAndObserveMRMLScene ( mrml );
      this->SliceController->SetParent ( this->SliceGUIFrame );
      this->SliceController->Create (  );
      this->SliceController->ApplyColorCode ( c );

      this->SliceViewer->SetApplication ( app );
      this->SliceViewer->SetParent ( this->SliceGUIFrame );
      this->SliceViewer->Create (  );

      this->SliceGUIFrame
        ->SetConfigurationOptionAsInt("-highlightthickness", 2);
      this->SliceGUIFrame
        ->SetConfigurationOptionAsColor("-highlightcolor", c[0], c[1], c[2]);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::PackGUI ( vtkKWFrame *f )
{
  if ( f )
    {
    this->Script("pack %s -side left -expand y -fill both -padx 0 -pady 0 -in %s", SliceGUIFrame->GetWidgetName(), f->GetWidgetName() );
    this->Script("pack %s -pady 0 -side top -expand false -fill x", SliceController->GetWidgetName() );
    this->Script("pack %s -anchor c -side top -expand true -fill both", SliceViewer->GetRenderWidget()->GetWidgetName());
    }
}



//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::UnpackGUI ()
{
    this->Script("pack forget %s", SliceController->GetWidgetName() );
    this->Script("pack forget %s", SliceViewer->GetRenderWidget()->GetWidgetName());
    this->Script("pack forget %s", SliceGUIFrame->GetWidgetName() );
}

//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::GridGUI ( vtkKWFrame *f, int row, int col )
{

  if ( f )
    {
    this->Script("grid configure %s -in %s -sticky news -padx 0 -pady 0 -row %d -column %d", SliceGUIFrame->GetWidgetName(), f->GetWidgetName(), row, col);
    this->Script("pack %s -pady 0 -side top -expand false -fill x", SliceController->GetWidgetName() );
    this->Script("pack %s -anchor c -side top -expand true -fill both", SliceViewer->GetRenderWidget()->GetWidgetName());
    }
}

//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::UngridGUI ()
{
    this->Script("pack forget %s", SliceController->GetWidgetName() );
    this->Script("pack forget %s", SliceViewer->GetRenderWidget()->GetWidgetName());
    this->Script("grid forget %s", SliceGUIFrame->GetWidgetName() );

}

//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::SetupViewerAndController()
{
  vtkImageData *idata = NULL;
  if ( this->GetLogic() )
    {
    idata = this->GetLogic()->GetImageData();
    
    vtkMRMLSliceNode *snode = this->GetLogic()->GetSliceNode();
    this->GetSliceController()->SetSliceNode (snode);
    
    vtkMRMLSliceCompositeNode *scnode = this->GetLogic()->GetSliceCompositeNode();
    this->GetSliceController()->SetSliceCompositeNode (scnode);
    
    this->GetSliceController()->SetAndObserveSliceLogic( this->GetLogic() );
    }

  if ( this->GetSliceViewer() && this->GetSliceViewer()->GetImageMapper() )
    {
    this->GetSliceViewer()->GetImageMapper()->SetInput( idata );
    }
  if ( this->GetSliceController() )
    {
    this->GetSliceController()->UpdateLayerMenus();
    }
}

