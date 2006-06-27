#ifndef __vtkSlicerSliceViewer_h
#define __vtkSlicerSliceViewer_h

#include "vtkSlicerBaseGUIWin32Header.h"

#include "vtkRenderWindow.h"

#include "vtkKWCompositeWidget.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWGenericRenderWindowInteractor.h"

class vtkImageMapper;
class vtkActor2D;
class vtkKWFrame;

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerSliceViewer : public vtkKWCompositeWidget
{
    
public:
  static vtkSlicerSliceViewer* New ( );
  vtkTypeRevisionMacro ( vtkSlicerSliceViewer, vtkKWCompositeWidget );
  void PrintSelf (ostream& os, vtkIndent indent);
    
  vtkGetObjectMacro ( ImageMapper, vtkImageMapper );
  vtkGetObjectMacro ( Actor2D, vtkActor2D );
  vtkGetObjectMacro ( RenderWidget, vtkKWRenderWidget );

  // Description:
  // Access the renderwindow interactor
  vtkRenderWindowInteractor *GetRenderWindowInteractor()
    {
    return this->RenderWidget->GetRenderWindowInteractor();
    }

protected:
  vtkSlicerSliceViewer ( );
  ~vtkSlicerSliceViewer ( );

  // Description:
  // Create the widget.
  virtual void CreateWidget( );

  // Slice viewer widgets
  vtkKWRenderWidget *RenderWidget;
  vtkImageMapper *ImageMapper;
  vtkActor2D *Actor2D;

private:
  vtkSlicerSliceViewer (const vtkSlicerSliceViewer &); //Not implemented
  void operator=(const vtkSlicerSliceViewer &);         //Not implemented

};

#endif

