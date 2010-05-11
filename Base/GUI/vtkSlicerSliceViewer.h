#ifndef __vtkSlicerSliceViewer_h
#define __vtkSlicerSliceViewer_h

#include "vtkSlicerBaseGUIWin32Header.h"

#include "vtkRenderWindow.h"

#include "vtkSmartPointer.h"
#include "vtkKWCompositeWidget.h"
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
class vtkCallbackCommand;

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerSliceViewer : public vtkKWCompositeWidget
{
    
public:
  static vtkSlicerSliceViewer* New ( );
  vtkTypeRevisionMacro ( vtkSlicerSliceViewer, vtkKWCompositeWidget );
  void PrintSelf (ostream& os, vtkIndent indent);
    
  vtkGetObjectMacro ( ActorCollection, vtkActor2DCollection );
  vtkGetObjectMacro ( PolyDataCollection, vtkPolyDataCollection );
  vtkGetObjectMacro ( LookupTableCollection, vtkCollection );


  vtkGetObjectMacro ( RenderWidget, vtkKWRenderWidget );

  /// 
  /// Used to set the PolyData/LookupTable pairs to show arbitrary object in the 2D slices
  void SetCoordinatedPolyDataAndLookUpTableCollections( vtkPolyDataCollection* newPolyDataCollection, vtkCollection* newLookupTableCollection );

  ///  
  /// Used to track the fact that there is a idle task pending requesting a render
  vtkSetMacro (RenderPending, int);
  vtkGetMacro (RenderPending, int);

  vtkGetMacro (LayoutGridRows, int);
  vtkSetMacro (LayoutGridRows, int);

  vtkGetMacro (LayoutGridColumns, int);
  vtkSetMacro (LayoutGridColumns, int);

  /// 
  /// Control the highlight of a viewport in a lightbox. Hightlight is
  /// used to designate the active slice in a lightbox
  void HighlightSlice(int slice);
  void UnhighlightSlice(int slice);
  void UnhighlightAllSlices();
  vtkGetVector3Macro(HighlightColor, double);
  vtkSetVector3Macro(HighlightColor, double);

  /// 
  /// propagate events generated in GUI to logic / mrml
  /// Here, it calls RequestRender in response to Expose and Configure events
  /// BTX
  static void GUICallback( vtkObject *__caller,
                           unsigned long eid, void *__clientData, void *callData );    
  /// ETX
  
  /// 
  /// Request Render posts a request to the event queue for processing when
  /// all other user events have been handled.  Render does the actual render.
  void RequestRender();
  void Render();

  void SetImageData( vtkImageData* );

  void ChangeLayout( int, int );

  //int GetNumberOfTiles() { return ImageMapperVec; };

  /// void SetNthMapper( int n, vtkImageMapper* imageMapper } { ImageMapperVec[n] = imageMapper; };

protected:
  vtkSlicerSliceViewer ( );
  virtual ~vtkSlicerSliceViewer ( );

  /// 
  /// Create the widget.
  virtual void CreateWidget( );

  /// Slice viewer widgets
  vtkKWRenderWidget *RenderWidget;

  double HighlightColor[3];
  
  /// To accomadate changes in layout
  //BTX
  std::vector< vtkSmartPointer< vtkImageMapper > > ImageMapperVec;
  std::vector< vtkSmartPointer< vtkActor2D > > HighlightActorVec;
  //ETX

  int LayoutGridRows;
  int LayoutGridColumns;

  vtkActor2DCollection *ActorCollection;
  vtkPolyDataCollection* PolyDataCollection;
  vtkCollection* LookupTableCollection;

  vtkCallbackCommand *GUICallbackCommand;

  int RenderPending;

private:
  vtkSlicerSliceViewer (const vtkSlicerSliceViewer &); //Not implemented
  void operator=(const vtkSlicerSliceViewer &);         //Not implemented

};

#endif

