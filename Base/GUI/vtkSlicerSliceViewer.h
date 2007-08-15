#ifndef __vtkSlicerSliceViewer_h
#define __vtkSlicerSliceViewer_h

#include "vtkSlicerBaseGUIWin32Header.h"

#include "vtkRenderWindow.h"

#include "vtkSmartPointer.h"
#include "vtkKWCompositeWidget.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWGenericRenderWindowInteractor.h"
#include <vector>
//#include "vtkImageMapper.h"

#include "vtkPolyDataCollection.h"
#include "vtkActorCollection.h"
#include "vtkActor2DCollection.h"

class vtkImageMapper;
class vtkActor2D;
class vtkKWFrame;
class vtkImageData;
class vtkRenderer;

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerSliceViewer : public vtkKWCompositeWidget
{
    
public:
  static vtkSlicerSliceViewer* New ( );
  vtkTypeRevisionMacro ( vtkSlicerSliceViewer, vtkKWCompositeWidget );
  void PrintSelf (ostream& os, vtkIndent indent);
    
  vtkGetObjectMacro ( ImageMapper, vtkImageMapper );
  vtkGetObjectMacro ( Actor2D, vtkActor2D );
  vtkGetObjectMacro ( ActorCollection, vtkActor2DCollection );
  vtkGetObjectMacro ( PolyDataCollection, vtkPolyDataCollection );


  vtkGetObjectMacro ( RenderWidget, vtkKWRenderWidget );

  // Description:
  // Used to set the PolyData/LookupTable pairs to show arbitrary object in the 2D slices
  void SetCoordinatedPolyDataAndLookUpTableCollections( vtkPolyDataCollection* newPolyDataCollection, vtkCollection* newLookupTableCollection );

  // Description: 
  // Used to track the fact that there is a idle task pending requesting a render
  vtkSetMacro (RenderPending, int);
  vtkGetMacro (RenderPending, int);

  vtkGetMacro (LayoutGridRows, int);
  vtkSetMacro (LayoutGridRows, int);

  vtkGetMacro (LayoutGridColumns, int);
  vtkSetMacro (LayoutGridColumns, int);

  // Description:
  // Request Render posts a request to the event queue for processing when
  // all other user events have been handled.  Render does the actual render.
  void RequestRender();
  void Render();

  void SetImageData( vtkImageData* );

  void ChangeLayout( int, int );

  //int GetNumberOfTiles() { return ImageMapperVec; };

  // void SetNthMapper( int n, vtkImageMapper* imageMapper } { ImageMapperVec[n] = imageMapper; };

protected:
  vtkSlicerSliceViewer ( );
  virtual ~vtkSlicerSliceViewer ( );

  // Description:
  // Create the widget.
  virtual void CreateWidget( );

  // Slice viewer widgets
  vtkKWRenderWidget *RenderWidget;
  vtkImageMapper *ImageMapper;
  vtkActor2D *Actor2D;

  // To accomadate changes in layout
  //BTX
  std::vector< vtkSmartPointer< vtkImageMapper > > ImageMapperVec;
  //ETX

  int LayoutGridRows;
  int LayoutGridColumns;

  vtkActor2DCollection *ActorCollection;
  vtkPolyDataCollection* PolyDataCollection;

  int RenderPending;

private:
  vtkSlicerSliceViewer (const vtkSlicerSliceViewer &); //Not implemented
  void operator=(const vtkSlicerSliceViewer &);         //Not implemented

};

#endif

