

#include <sstream>
#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkSlicerGUILayout.h"
#include "vtkKWSplitFrame.h"
#include "vtkKWFrame.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerGUILayout );
vtkCxxRevisionMacro ( vtkSlicerGUILayout, "$Revision: 1.0 $" );


//---------------------------------------------------------------------------
vtkSlicerGUILayout::vtkSlicerGUILayout ( )
{

  this->MainSlicerWindow = NULL;
  this->CurrentViewArrangement = this->SlicerLayoutInitialView;
  // DefaultGUIPanelHeight defaults to 860 pixels
  // and DefaultGUIPanelWidth is fixed at 400 pixels.
  // DefaultGUIPanelHeight should be a function of the
  // window size saved in the registry, if a user has
  // resized the slicer window. How to do this?
  // ModuleControlPanelHeight is derived from 
  // total panel height and the other fixed panel heights.
  this->DefaultModuleControlPanelHeight = 0;
  this->DefaultTopFrameHeight = 70;
  this->DefaultSlicesControlFrameHeight = 60;
  this->DefaultViewControlFrameHeight = 240;

  this->DefaultGUIPanelWidth = 400;
  this->DefaultGUIPanelHeight = 860;
  this->DefaultQuadrantHeight = 0;
  this->DefaultQuadrantWidth = 0;
    
  this->DefaultSlicerWindowWidth = 0;
  this->DefaultSlicerWindowHeight = 0;

  // set the slice windows to be a default size
  this->SetDefaultSliceGUIFrameHeight ( 350 );
  this->SetDefaultSliceGUIFrameWidth ( 300);

  this->DefaultTopFrameHeight = 0;
  this->DefaultModuleControlPanelHeight = 0;
  this->DefaultSlicesControlFrameHeight = 0;
  this->DefaultViewControlFrameHeight = 0;
}


//---------------------------------------------------------------------------
vtkSlicerGUILayout::~vtkSlicerGUILayout ( )
{
  this->SetMainSlicerWindow ( NULL );
}



//---------------------------------------------------------------------------
void vtkSlicerGUILayout::InitializeLayoutDimensions ( )
{
  // Five layout choices: Default, 3D only, lightbox, tabbed-viewer, and 4-up.
  // Dimensions for these are computed here.
  int h, w;
    
  // Look for registry values here.
  
  // Size of the shared Module GUI Control frame
  h = this->DefaultGUIPanelHeight -
    ( this->GetDefaultTopFrameHeight() +
      this->GetDefaultSlicesControlFrameHeight() +
      this->GetDefaultViewControlFrameHeight() );
  this->SetDefaultModuleControlPanelHeight ( h );

  // Size of the main viewer.
  h = this->GetDefaultGUIPanelHeight ( );
  this->SetDefault3DViewerHeight ( h - this->GetDefaultSliceGUIFrameHeight () );
  w = 3 * this->GetDefaultSliceGUIFrameWidth ( );
  this->SetDefaultMainViewerWidth ( w );

  this->SetDefaultQuadrantHeight ( h / 2 );
  this->SetDefaultQuadrantWidth ( w / 2 );

  // make room for window chrome, menubar, toolbar, statusbar, etc.;
  // have to play with these buffers.
  int hbuf = 10;
  int vbuf = 60;
  
  h = this->GetDefaultGUIPanelHeight ( ) + vbuf;
  w = (3 * this->GetDefaultSliceGUIFrameWidth() ) + this->GetDefaultGUIPanelWidth() + hbuf;
  this->SetDefaultSlicerWindowWidth ( w );
  this->SetDefaultSlicerWindowHeight ( h );

  // make a minimum size for slice viewers (RenderWidget Size)
  this->SetSliceViewerMinDim ( 10 );
}



//---------------------------------------------------------------------------
void vtkSlicerGUILayout::InitializeMainSlicerWindowSize ( )
{
  if ( this->MainSlicerWindow != NULL )
    {
      this->MainSlicerWindow->SetSize ( this->GetDefaultSlicerWindowWidth ( ),
                                     this->GetDefaultSlicerWindowHeight () );
    }
}

//---------------------------------------------------------------------------
void vtkSlicerGUILayout::ConfigureMainSlicerWindowPanels ( )
{

  if ( this->MainSlicerWindow != NULL )
    {
      // Configure the minimum width of Slicer's GUI panel
      // and Main Viewer panels. Panels can be expanded and
      // collapsed entirely or can  be resized by hand.
      //
      this->MainSlicerWindow->MainPanelVisibilityOn ();
      this->MainSlicerWindow->SecondaryPanelVisibilityOn ();
      //
      //--- this split frame divides Slicer's GUI Panel with the Viewer Panel
      //
      this->MainSlicerWindow->GetMainSplitFrame()->SetFrame1Size (this->GetDefaultGUIPanelWidth() );
      this->MainSlicerWindow->GetMainSplitFrame()->SetFrame1MinimumSize (this->GetDefaultGUIPanelWidth ( ) );
      //
      //--- this split frame divides Slicer's 3DViewer from the Slice Viewers
      //--- in the conventional layout. Frame1 is the bottom (slice viewer) frame.
      // 
      this->MainSlicerWindow->GetSecondarySplitFrame()->SetFrame1Size ( this->GetDefaultSliceGUIFrameHeight () );
    }
}




//---------------------------------------------------------------------------
void vtkSlicerGUILayout::PrintSelf ( ostream &os, vtkIndent indent )
{
  this->vtkObject::PrintSelf ( os, indent );
  os << indent << "SlicerGUILayout: " << this->GetClassName ( ) << "\n";
}
