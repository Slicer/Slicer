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

#include "vtkSlicerSliceViewer.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerGUILayout.h"

#include "vtkKWWidget.h"
#include "vtkSlicerRenderWidget.h"
#include "vtkKWFrame.h"

#include "vtkImageMapper.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkActor2D.h"
#include "vtkActor2DCollection.h"
#include "vtkMapper2D.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerSliceViewer );
vtkCxxRevisionMacro ( vtkSlicerSliceViewer, "$Revision: 1.0 $");


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

    this->ImageMapper = vtkImageMapper::New();
    this->ImageMapper->SetColorWindow(255);
    this->ImageMapper->SetColorLevel(127.5);

    this->ImageMapperVec.push_back( ImageMapper );

    this->Actor2D = vtkActor2D::New();
    this->Actor2D->SetMapper( this->ImageMapper );

    this->LayoutGridRows = 1;
    this->LayoutGridColumns = 1;

    this->ActorCollection = vtkActor2DCollection::New();
    this->PolyDataCollection = vtkPolyDataCollection::New();

    this->RenderPending = 0;
}


//---------------------------------------------------------------------------
vtkSlicerSliceViewer::~vtkSlicerSliceViewer ( ){

    if ( this->Actor2D )
      {
        if ( this->RenderWidget )
          {
          int numberOfRenderers = this->RenderWidget->GetNumberOfRenderers();
          for ( int i=0; i<numberOfRenderers; i++ )
            {
            this->RenderWidget->GetNthRenderer( i )->RemoveActor2D( this->Actor2D );
            }
          }
        this->Actor2D->SetMapper ( NULL );
        this->Actor2D->Delete ( );
        this->Actor2D = NULL;
      }

    int numberMappers = this->ImageMapperVec.size();
    for ( int i=1; i<numberMappers; i++ )  // start at 1 to skip this->ImageMapper
      {
      this->ImageMapperVec[i]->Delete();
      this->ImageMapperVec[i] = NULL;
      }

    this->ImageMapperVec.clear();

    if ( this->ImageMapper )
      {
      this->ImageMapper->Delete ( );
      this->ImageMapper = NULL;
      }
    if ( this->PolyDataCollection )
      {
      this->PolyDataCollection->RemoveAllItems();
      this->PolyDataCollection->Delete ( );
      this->PolyDataCollection = NULL;
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

      if ( !newPolyDataCollection->IsItemPresent(polyData) )
        {
        this->PolyDataCollection->RemoveItem( polyData );
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
          
        if ( !this->PolyDataCollection->IsItemPresent(polyData) )
          {
//        vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
          vtkPolyDataMapper2D* mapper = vtkPolyDataMapper2D::New();

          mapper->SetInput( polyData );
  
          vtkScalarsToColors* lookupTable = vtkScalarsToColors::SafeDownCast(newLookupTableCollection->GetItemAsObject(i));
          if ( lookupTable != NULL ) 
            {
            mapper->SetLookupTable( lookupTable );
            } 
          else 
            {
            vtkErrorMacro("There is an object which is not a lookupTable in the newLookupTable Collection")
            }

          this->PolyDataCollection->AddItem( polyData );
          vtkActor2D* actor = vtkActor2D::New();

//        vtkActor* actor = vtkActor::New();
          actor->SetMapper( mapper );
          this->ActorCollection->AddItem( actor );

          this->RenderWidget->GetRenderer()->AddActor( actor );
          mapper->Delete();
          actor->Delete();
        }
      }
  
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
    this->RenderWidget->SetWidth ( app->GetMainLayout()->GetSliceViewerMinDim() );
    this->RenderWidget->SetHeight ( app->GetMainLayout()->GetSliceViewerMinDim() );
    this->RenderWidget->CornerAnnotationVisibilityOn();
    this->RenderWidget->SetBorderWidth(2);
    this->RenderWidget->SetReliefToGroove ( );
    this->RenderWidget->GetRenderer()->AddActor2D( this->Actor2D );
}

//---------------------------------------------------------------------------
void vtkSlicerSliceViewer::RequestRender()
{
  if (this->GetRenderPending())
    {
    return;
    }

  this->SetRenderPending(1);
  this->Script("after idle \"%s Render\"", this->GetTclName());
}

//---------------------------------------------------------------------------
void vtkSlicerSliceViewer::Render()
{
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
    vtkCamera *cam;
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

        vtkSmartPointer< vtkActor2D > actor2D = vtkActor2D::New();
          actor2D->SetMapper( mapper );

        vtkSmartPointer< vtkRenderer > renderer = vtkRenderer::New();
          renderer->SetBackground( 0.0, 0.0, 0.0 );
          renderer->SetViewport( xMin, yMin, (xMin+viewportWidth), (yMin+viewportHeight) );
          renderer->AddActor2D( actor2D );

        this->RenderWidget->AddRenderer( renderer );

        //-------
        // First renderer, grab a handle to the camera to share
        // amongst the rest of the renderers
        //
        if (first)
          {
          first = false;
          cam = renderer->GetActiveCamera();
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


