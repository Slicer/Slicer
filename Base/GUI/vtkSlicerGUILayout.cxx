

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
  this->DefaultTopFrameHeight = 118;
  this->DefaultModuleControlPanelHeight = 442;
  this->DefaultSlicesControlFrameHeight = 60;
  this->DefaultViewControlFrameHeight = 240;

  this->DefaultGUIPanelHeight = 0;
  this->DefaultGUIPanelWidth = 0;
  this->DefaultQuadrantHeight = 0;
  this->DefaultQuadrantWidth = 0;
    
  this->DefaultSlicerWindowWidth = 0;
  this->DefaultSlicerWindowHeight = 0;

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
    
  // Layout for default 3D over axi-sag-cor layout:
  // specify dims of GUI Panel components here for now.

  // This frame contains the logo and module choose/navigation widgets
  this->SetDefaultTopFrameHeight ( 118 );

  // This frame contains the Module GUI panel
  this->SetDefaultModuleControlPanelHeight ( 442 );

  // This frme contains the widgets to control the slice viewers
  this->SetDefaultSlicesControlFrameHeight ( 60 );

  // This frame contains the widgets to control the 3D view
  this->SetDefaultViewControlFrameHeight ( 240 );

  //
  // entire GUI panel height and width
  //
  h = this->GetDefaultTopFrameHeight ( ) +
    this->GetDefaultModuleControlPanelHeight ( ) +
    this->GetDefaultSlicesControlFrameHeight ( ) +
    this->GetDefaultViewControlFrameHeight ( );
  this->SetDefaultGUIPanelHeight ( h );
  this->SetDefaultGUIPanelWidth ( 400 );

  // set the slice windows to be a default size
  this->SetDefaultSliceGUIFrameHeight ( 350 );
  this->SetDefaultSliceGUIFrameWidth ( 300);

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
