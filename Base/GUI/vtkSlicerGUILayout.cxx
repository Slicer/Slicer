

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

  this->MainSlicerWin = NULL;
  this->SetCurrentViewArrangement ( this->SlicerLayoutInitialView);
}


//---------------------------------------------------------------------------
vtkSlicerGUILayout::~vtkSlicerGUILayout ( )
{
  this->SetMainSlicerWin ( NULL );
}



//---------------------------------------------------------------------------
void vtkSlicerGUILayout::InitializeLayoutDimensions ( )
{
  // Five layout choices: Default, 3D only, lightbox, tabbed-viewer, and 4-up.
  // Dimensions for these are computed here.
  int h, w;
    
  // Default 3D over axi-sag-cor layout:
  // specify dims of GUI Panel components here for now.
  this->SetDefaultLogoFrameHeight ( 40 );
  this->SetDefaultModuleChooseFrameHeight ( 60 );
  this->SetDefaultModuleControlPanelHeight ( 450 );
  this->SetDefaultSliceControlFrameHeight ( 60 );
  this->SetDefaultViewControlFrameHeight ( 240 );
  // entire GUI panel height and width
  h = this->GetDefaultLogoFrameHeight ( ) +
    this->GetDefaultModuleChooseFrameHeight ( ) +
    this->GetDefaultModuleControlPanelHeight ( ) +
    this->GetDefaultSliceControlFrameHeight ( ) +
    this->GetDefaultViewControlFrameHeight ( );
  this->SetDefaultGUIPanelHeight ( h );
  //    this->SetDefaultGUIPanelWidth ( 325 );
  this->SetDefaultGUIPanelWidth ( 400 );

  // constrain the slice windows to be a particular size
  this->SetDefaultSliceGUIFrameHeight ( 120 );
  this->SetDefaultSliceGUIFrameWidth ( 175 );
  this->SetDefaultSliceWindowHeight ( 120 );
  this->SetDefaultSliceWindowWidth ( 175 );
  w = 3 * this->GetDefaultSliceGUIFrameWidth ( );

  h = this->GetDefaultLogoFrameHeight ( ) +
    this->GetDefaultModuleChooseFrameHeight ( ) +
    this->GetDefaultModuleControlPanelHeight ( ) +
    this->GetDefaultSliceControlFrameHeight ( ) +
    this->GetDefaultViewControlFrameHeight ( );
  w = 3 * this->GetDefaultSliceGUIFrameWidth ( );
  // set up default Slicer Window size here for now
  this->SetDefaultMainViewerHeight ( h - this->GetDefaultSliceGUIFrameHeight () );
  this->SetDefaultMainViewerWidth ( w );
  this->SetDefaultQuadrantHeight ( h / 2 );
  this->SetDefaultQuadrantWidth ( w / 2 );

  int hbuf = 10;
  int vbuf = 60;
  // make room for window chrome or whatever; have to play with this buffer.
  h = this->GetDefaultLogoFrameHeight ( ) +
    this->GetDefaultModuleChooseFrameHeight ( ) +
    this->GetDefaultModuleControlPanelHeight ( ) +
    this->GetDefaultSliceControlFrameHeight ( ) +
    this->GetDefaultViewControlFrameHeight ( ) + hbuf;
  w = 3 * this->GetDefaultSliceGUIFrameWidth ( );
  w = w + this->GetDefaultGUIPanelWidth ( ) + hbuf;
  this->SetDefaultSlicerWindowWidth ( w + this->GetDefaultGUIPanelWidth ( ) );
  this->SetDefaultSlicerWindowHeight ( h );

  // make a minimum size for slice viewers (RenderWidget Size)
  this->SetSliceViewerMinDim ( 10 );

  // TODO: 3D only layout
    
  // TODO: Lightbox layout:

  // TODO: tabbed viewer notebook layout:

  // TODO: 4-up 3D-axi-sag-cor layout:
}



//---------------------------------------------------------------------------
void vtkSlicerGUILayout::ConfigureMainSlicerWindow ( )
{

  if ( this->MainSlicerWin != NULL )
    {
      this->MainSlicerWin->MainPanelVisibilityOn ();
      this->MainSlicerWin->SecondaryPanelVisibilityOn ();
      this->MainSlicerWin->SetSize ( this->GetDefaultSlicerWindowWidth ( ),
                                     this->GetDefaultSlicerWindowHeight () );
      // Configure the minimum width of Slicer's GUI panel.
      // Panel can be expanded and collapsed entirely, but
      // can't be resized by hand to a value smaller than what's set.
      this->MainSlicerWin->GetMainSplitFrame()->SetFrame1Size (this->GetDefaultGUIPanelWidth() );
      this->MainSlicerWin->GetMainSplitFrame()->SetFrame1MinimumSize (this->GetDefaultGUIPanelWidth ( ) );
    }
}


//---------------------------------------------------------------------------
void vtkSlicerGUILayout::ConfigureMainViewerPanel ( )
{

  if ( this->MainSlicerWin != NULL )
    {
      this->MainSlicerWin->GetViewFrame()->SetWidth ( this->GetDefaultMainViewerWidth() );
    }
}


//---------------------------------------------------------------------------
void vtkSlicerGUILayout::ConfigureSliceViewersPanel ( )
{

  if ( this->MainSlicerWin != NULL )
    {
    this->MainSlicerWin->GetSecondarySplitFrame()->SetFrame2Size (this->GetDefaultSliceGUIFrameWidth ( ) );
    this->MainSlicerWin->GetSecondarySplitFrame()->SetFrame2MinimumSize (this->GetDefaultSliceGUIFrameWidth ( ) );
        
    this->MainSlicerWin->GetSecondaryPanelFrame()->SetWidth ( 3 * this->GetDefaultSliceGUIFrameWidth () );
    this->MainSlicerWin->GetSecondaryPanelFrame()->SetHeight ( this->GetDefaultSliceGUIFrameHeight () );

    }

}



//---------------------------------------------------------------------------
void vtkSlicerGUILayout::PrintSelf ( ostream &os, vtkIndent indent )
{
  this->vtkObject::PrintSelf ( os, indent );
  os << indent << "SlicerGUILayout: " << this->GetClassName ( ) << "\n";
}
