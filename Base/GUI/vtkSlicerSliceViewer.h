#ifndef __vtkSlicerSliceViewer_h
#define __vtkSlicerSliceViewer_h

#include "vtkSlicerBaseGUIWin32Header.h"

#include "vtkKWRenderWidget.h"
#include "vtkImageViewer2.h"
#include "vtkKWCompositeWidget.h"

class vtkKWFrame;

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerSliceViewer : public vtkKWCompositeWidget
{
    
public:
  static vtkSlicerSliceViewer* New ( );
  vtkTypeRevisionMacro ( vtkSlicerSliceViewer, vtkKWCompositeWidget );
  void PrintSelf (ostream& os, vtkIndent indent);
    
  vtkGetObjectMacro ( ImageViewer, vtkImageViewer2 );
  vtkGetObjectMacro ( RenderWidget, vtkKWRenderWidget );

protected:
  vtkSlicerSliceViewer ( );
  ~vtkSlicerSliceViewer ( );

  // Description:
  // Create the widget.
  virtual void CreateWidget( );

  // Slice viewer widgets
  vtkKWRenderWidget *RenderWidget;
  vtkImageViewer2 *ImageViewer;

private:
  vtkSlicerSliceViewer (const vtkSlicerSliceViewer &); //Not implemented
  void operator=(const vtkSlicerSliceViewer &);         //Not implemented

};

#endif

