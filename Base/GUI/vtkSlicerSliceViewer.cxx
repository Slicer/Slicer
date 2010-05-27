#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkCornerAnnotation.h"
#include "vtkImageData.h"
#include "vtkImageViewer2.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkViewport.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSmartPointer.h"
#include "vtkMath.h"

#include "vtkSlicerSliceViewer.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerGUILayout.h"

#include "vtkKWWidget.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWFrame.h"

#include "vtkImageMapper.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkActor2D.h"
#include "vtkActor2DCollection.h"
#include "vtkMapper2D.h"
#include "vtkProperty2D.h"
#include "vtkCellArray.h"
#include "vtkCallbackCommand.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerSliceViewer );
vtkCxxRevisionMacro ( vtkSlicerSliceViewer, "$Revision$");


//---------------------------------------------------------------------------
vtkSlicerSliceViewer::vtkSlicerSliceViewer ( ) {

    //---  
    // widgets comprising the SliceViewer
    //
    
    //
    // Revert back to KW superclass renderwidget to address
    // window corruption on some linux boxes:
    //this->RenderWidget = vtkSlicerRenderWidget::New ( );
    this->RenderWidget = vtkKWRenderWidget::New ( );

    // tell the render widget not to respond to the Render() method
    // - this class turns on rendering explicitly when it's own
    //   Render() method is called.  This avoids redundant renders
    //   when, for example, the annotation is changed.
    this->RenderWidget->RenderStateOff();

    this->HighlightColor[0] = 1;
    this->HighlightColor[0] = 1;
    this->HighlightColor[0] = 0;
    
    this->ActorCollection = vtkActor2DCollection::New();
    this->PolyDataCollection = vtkPolyDataCollection::New();
    this->LookupTableCollection = vtkCollection::New();

    this->GUICallbackCommand = vtkCallbackCommand::New ( );
    this->GUICallbackCommand->SetClientData( reinterpret_cast<void *>(this) );
    this->GUICallbackCommand->SetCallback( vtkSlicerSliceViewer::GUICallback );

    this->RenderPending = 0;

    // Initialize the layout the zero so CreateWidget and set up
    // the rendering pipeline
    this->LayoutGridRows = 0;
    this->LayoutGridColumns = 0;
}


//---------------------------------------------------------------------------
vtkSlicerSliceViewer::~vtkSlicerSliceViewer ( ){

    // clear the smart pointer managed lists
    this->ImageMapperVec.clear();
    this->HighlightActorVec.clear();
    
    if ( this->PolyDataCollection )
      {
      this->PolyDataCollection->RemoveAllItems();
      this->PolyDataCollection->Delete ( );
      this->PolyDataCollection = NULL;
      }
    if ( this->LookupTableCollection )
      {
      this->LookupTableCollection->RemoveAllItems();
      this->LookupTableCollection->Delete ( );
      this->LookupTableCollection = NULL;
      }
    if ( this->ActorCollection )
      {
      this->ActorCollection->RemoveAllItems();
      this->ActorCollection->Delete ( );
      this->ActorCollection = NULL;
      }

    this->RenderWidget->RemoveAllRenderers();

    if ( this->RenderWidget ) {
      this->RenderWidget->SetParent ( NULL );
      this->RenderWidget->Delete ( );
      this->RenderWidget = NULL;
    }

    this->GUICallbackCommand->Delete();

}

//---------------------------------------------------------------------------
void vtkSlicerSliceViewer::SetCoordinatedPolyDataAndLookUpTableCollections( vtkPolyDataCollection* newPolyDataCollection, 
                                                                            vtkCollection* newLookupTableCollection ){
  if ( newPolyDataCollection->GetNumberOfItems() == newLookupTableCollection->GetNumberOfItems() )
    {
    //All the actors that contain PolyDatas NOT INCLUDED in the
    //newPolyDataCollection collection are removed from the
    //polyDataMapper, actor and renderer collections and deleted
    for(int i=this->PolyDataCollection->GetNumberOfItems()-1; i>=0 ; i-- )
      {
      vtkActor2D* actor = vtkActor2D::SafeDownCast( this->ActorCollection->GetItemAsObject(i) );
      vtkPolyDataMapper2D* mapper = vtkPolyDataMapper2D::SafeDownCast(actor->GetMapper());
      vtkPolyData* polyData = mapper->GetInput();
      vtkScalarsToColors* lut = vtkScalarsToColors::SafeDownCast(this->LookupTableCollection->GetItemAsObject(i));

      if ( !newPolyDataCollection->IsItemPresent(polyData) || !newLookupTableCollection->IsItemPresent(lut))
        {
        this->PolyDataCollection->RemoveItem( polyData );
        this->LookupTableCollection->RemoveItem( lut );
        this->ActorCollection->RemoveItem( actor );
        this->RenderWidget->GetRenderer()->RemoveActor( actor );
        }
      } // for

    //All the actors that contain PolyDatas NOT INCLUDED in the
    //PolyDataCollection collection are added to the polyDataMapper, actor and renderer collections
    for(int i=newPolyDataCollection->GetNumberOfItems()-1; i>=0 ; i-- )
      {
      vtkPolyData* polyData = vtkPolyData::SafeDownCast(newPolyDataCollection->GetItemAsObject(i));
      if (polyData==NULL)
        {
          vtkErrorMacro("There's an element in the PolyDataCollection which is not a PolyData");
        } 
        
      vtkScalarsToColors* newLUT = vtkScalarsToColors::SafeDownCast(newLookupTableCollection->GetItemAsObject(i));
      if ( !this->PolyDataCollection->IsItemPresent(polyData) )
        {
//        vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
        vtkPolyDataMapper2D* mapper = vtkPolyDataMapper2D::New();

        mapper->SetInput( polyData );

        if ( newLUT != NULL ) 
          {
          mapper->SetLookupTable( newLUT );
          } 
        else 
          {
          vtkErrorMacro("There is an object which is not a lookupTable in the newLookupTable Collection")
          }

        this->PolyDataCollection->AddItem( polyData );
        this->LookupTableCollection->AddItem( newLUT );

        vtkActor2D* actor = vtkActor2D::New();

//        vtkActor* actor = vtkActor::New();
        actor->SetMapper( mapper );
        this->ActorCollection->AddItem( actor );

        this->RenderWidget->GetRenderer()->AddActor( actor );
        mapper->Delete();
        actor->Delete();
        }
      } // for
  
      //Due to the double inclusion principle at the end the PolyDataCollection, ActorCollection and Renderer include only the PolyDatas and LookUp tables in the new collections and The actors and rendered actors collections are finec
  
      this->Modified();
    } 
  else 
    {
    vtkErrorMacro("The PolyDataCollection and LookupTableCollection must have the same number of items");
    } 
}

//---------------------------------------------------------------------------
void vtkSlicerSliceViewer::CreateWidget ( ) {

    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast ( this->GetParent()->GetApplication() );

    // the widget is a frame with an image viewer packed inside
    if (this->IsCreated ( ) ) {
        vtkErrorMacro ( << this->GetClassName() << "already created.");
        return;
    }
    this->Superclass::CreateWidget ( );
    
    //---
    // Create a render widget
    //
    this->RenderWidget->SetParent ( this->GetParent( ) );
    this->RenderWidget->Create();
    this->RenderWidget->SetWidth ( app->GetDefaultGeometry()->GetSliceViewerMinDim() );
    this->RenderWidget->SetHeight ( app->GetDefaultGeometry()->GetSliceViewerMinDim() );
    this->RenderWidget->CornerAnnotationVisibilityOn();
    this->RenderWidget->SetBorderWidth(2);
    this->RenderWidget->SetReliefToGroove ( );

    // Don't use vtkKWRenderWidget's built-in ExposeEvent handler.  
    // It will call ProcessPendingEvents (update) even though it may already be inside
    // a call to update.  It also calls Render directly, which will pull the vtk pipeline chain.
    // Instead, use the RequestRender method to render when idle.
    this->RenderWidget->GetVTKWidget()->RemoveBinding("<Expose>");
    this->RenderWidget->GetVTKWidget()->AddBinding("<Expose>", this, "RequestRender");

    this->ChangeLayout(1, 1);
}

//---------------------------------------------------------------------------
void vtkSlicerSliceViewer::RequestRender()
{
  if (this->GetRenderPending())
    {
    return;
    }

  if (!this->GetRenderWidget()->IsMapped())
    {
    return;
    }

  this->SetRenderPending(1);
  this->Script("after cancel { if {[info command %s] != {}} {%s Render} }", this->GetTclName(), this->GetTclName());
  this->Script("after idle { if {[info command %s] != {}} {%s Render} }", this->GetTclName(), this->GetTclName());
}

//---------------------------------------------------------------------------
void vtkSlicerSliceViewer::Render()
{
  // Jim's hokey way to track the number of renders that occur when
  // something is changed
//   static int sp = 0;
//   for (int i=0; i < sp; ++i) std::cout << " ";
//   ++sp;
//   sp = sp % 8;
//   std::cout << "Render " << this->GetTclName() << std::endl;

  this->GetRenderWidget()->RenderStateOn();
  this->GetRenderWidget()->Render();
  this->GetRenderWidget()->RenderStateOff();
  this->SetRenderPending(0);
}

//----------------------------------------------------------------------------
void vtkSlicerSliceViewer::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  // widgets?
}

//----------------------------------------------------------------------------
void vtkSlicerSliceViewer::ChangeLayout( int numberRows, int numberColumns )
{
  if (( numberRows != LayoutGridRows )||( numberColumns != LayoutGridColumns ))
    {
    vtkImageData *imageData = 0;

    if (ImageMapperVec.size() > 0)
      {
      imageData = ImageMapperVec[0]->GetInput();
      }

    this->LayoutGridRows    = numberRows;
    this->LayoutGridColumns = numberColumns;

    //-------
    // Remove all the mappers.
    // 
    this->ImageMapperVec.clear();

    //-------
    // Remove all highlights
    this->HighlightActorVec.clear();
    
    //-------
    // Remove all current renderers.
    //
    this->RenderWidget->RemoveAllRenderers();

    //-------
    // Add new renderers.  First, define the width and height of each
    // renderer within the window.
    //
    float viewportWidth  = 1.0/float(numberColumns);
    float viewportHeight = 1.0/float(numberRows);

    // arrange the renderers from top to bottom and left to right
    vtkCamera *cam = NULL;
    float xMin, yMin;
    bool first = true;
    for ( int r=0; r<numberRows; r++ )
      {
      yMin = (numberRows - 1 - r) * viewportHeight;
      xMin = 0.0;
      for ( int c=0; c<numberColumns; c++ )
        {
        vtkSmartPointer< vtkImageMapper > mapper = vtkImageMapper::New();
          mapper->SetColorWindow(255);
          mapper->SetColorLevel(127.5);

        this->ImageMapperVec.push_back( mapper );
        mapper->Delete();

        vtkSmartPointer< vtkActor2D > actor2D = vtkActor2D::New();
          actor2D->SetMapper( mapper );
          actor2D->GetProperty()->SetDisplayLocationToBackground();
        actor2D->Delete();

        vtkSmartPointer< vtkRenderer > renderer = vtkRenderer::New();
          renderer->SetBackground( 0.0, 0.0, 0.0 );
          renderer->SetViewport( xMin, yMin, (xMin+viewportWidth), (yMin+viewportHeight) );
          renderer->AddActor2D( actor2D );

        this->RenderWidget->AddRenderer( renderer );
        renderer->Delete();

        // create a highlight actor (2D box around viewport) for each
        // viewport in the lightbox and hide them initially
        vtkPolyData *poly = vtkPolyData::New();
        vtkPoints *points = vtkPoints::New();
        double eps = 0.0;
        points->InsertNextPoint( eps, eps, 0 );
        points->InsertNextPoint( 1, eps, 0 );
        points->InsertNextPoint( 1, 1, 0 );
        points->InsertNextPoint( eps, 1, 0 );
        vtkCellArray *cells = vtkCellArray::New();
        cells->InsertNextCell(5);
        cells->InsertCellPoint( 0 );
        cells->InsertCellPoint( 1 );
        cells->InsertCellPoint( 2 );
        cells->InsertCellPoint( 3 );
        cells->InsertCellPoint( 0 );
        poly->SetPoints(points);
        poly->SetLines(cells);
        points->Delete();
        cells->Delete();

        vtkCoordinate *coordinate = vtkCoordinate::New();
        coordinate->SetCoordinateSystemToNormalizedViewport();
        coordinate->SetViewport(renderer);
        
        vtkPolyDataMapper2D *pmapper = vtkPolyDataMapper2D::New();
        pmapper->SetInput( poly );
        pmapper->SetTransformCoordinate( coordinate );
        coordinate->Delete();
        poly->Delete();

        vtkActor2D *pactor = vtkActor2D::New();
        pactor->SetMapper(pmapper);
        pactor->GetProperty()->SetColor(1, 1, 0);
        pactor->GetProperty()->SetDisplayLocationToForeground();
        pactor->GetProperty()->SetLineWidth(3); // wide enough so not clipped
        pactor->VisibilityOff();
        pmapper->Delete();

        renderer->AddActor2D(pactor);
        pactor->Delete();

        this->HighlightActorVec.push_back(pactor);
        
        //-------
        // First renderer, grab a handle to the camera to share
        // amongst the rest of the renderers
        //
        if (first)
          {
          first = false;
          cam = renderer->IsActiveCameraCreated() ? renderer->GetActiveCamera() : NULL;
          if (cam)
            {
            cam->ParallelProjectionOn();
            }
          }
        else
          {
          renderer->SetActiveCamera(cam);
          }
        xMin += viewportWidth;
        }
      }
    
    this->SetImageData(imageData);
    }
}


void vtkSlicerSliceViewer::SetImageData( vtkImageData* imageData )
{
  //-------
  // Arrange the renderers left to right, top to bottom (instead of
  // left to right, bottom to right, as is the convention with VTK.
  //
  int inc = 0;
  for ( int r=0; r<this->LayoutGridRows; r++)
    {
    for ( int c=0; c<this->LayoutGridColumns; c++ )
      {
      this->ImageMapperVec[inc]->SetInput( imageData );
      this->ImageMapperVec[inc]->SetZSlice( r*this->LayoutGridColumns+c );

      inc++;
      }
    }
}


void vtkSlicerSliceViewer::HighlightSlice( int slice )
{
  if (slice >= 0 && slice < (int)this->HighlightActorVec.size())
    {
    // good time to propagate the color as well
    this->HighlightActorVec[slice]->GetProperty()->SetColor(this->HighlightColor);
    this->HighlightActorVec[slice]->VisibilityOn();
    }
}

void vtkSlicerSliceViewer::UnhighlightSlice( int slice )
{
  if (slice >= 0 && slice < (int) this->HighlightActorVec.size())
    {
    // good time to propagate the color as well
    this->HighlightActorVec[slice]->GetProperty()->SetColor(this->HighlightColor);
    this->HighlightActorVec[slice]->VisibilityOff();
    }
}

void vtkSlicerSliceViewer::UnhighlightAllSlices( )
{
  for (unsigned int i = 0; i < this->HighlightActorVec.size(); ++i)
    {
    // good time to propagate the color as well
    this->HighlightActorVec[i]->GetProperty()->SetColor(this->HighlightColor);
    this->HighlightActorVec[i]->VisibilityOff();
    }
}


//----------------------------------------------------------------------------
// Description:
// the GUICallback is a static function to handle events from the renderwidget
// This is a simplified version of what is used in vtkSlicerComponentGUI.
// Here we only need to call RequestRender
// 
// Note: this is not currently used: RequestRender is invoked directly
// from the AddBinding call after the widget is created.
//
void 
vtkSlicerSliceViewer::GUICallback(vtkObject * vtkNotUsed(caller), 
            unsigned long vtkNotUsed(eid), void *clientData, void * vtkNotUsed(callData) )
{
  vtkSlicerSliceViewer *self = reinterpret_cast<vtkSlicerSliceViewer *>(clientData);

  self->RequestRender();
}
