#ifndef __vtkSlicerSliceWidget_h
#define __vtkSlicerSliceWidget_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkKWCompositeWidget.h"
#include "vtkKWRenderWidget.h"
#include "vtkImageViewer2.h"
#include "vtkKWFrame.h"
#include "vtkKWScaleWithEntry.h"

// Note:
// eventually this class will break out into 
// vtkSlicerSliceViewWidget and 
// vtkSlicerSliceControlWidget

                                                            
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerSliceWidget : public vtkKWCompositeWidget
{
    
 public:
    static vtkSlicerSliceWidget* New ( );
    vtkTypeRevisionMacro ( vtkSlicerSliceWidget, vtkKWCompositeWidget );
    
    vtkGetObjectMacro ( Scale, vtkKWScaleWithEntry );
    vtkSetObjectMacro ( Scale, vtkKWScaleWithEntry );
    vtkGetObjectMacro ( ImageViewer, vtkImageViewer2 );
    vtkSetObjectMacro ( ImageViewer, vtkImageViewer2 );
    vtkGetObjectMacro ( RenderWidget, vtkKWRenderWidget );
    vtkSetObjectMacro ( RenderWidget, vtkKWRenderWidget );
    vtkGetObjectMacro ( SliceFrame, vtkKWFrame );
    vtkSetObjectMacro ( SliceFrame, vtkKWFrame );

    virtual void Create ( );

 protected:
    vtkSlicerSliceWidget ( );
    ~vtkSlicerSliceWidget ( );

    vtkKWFrame *SliceFrame;
    vtkKWRenderWidget *RenderWidget;
    vtkImageViewer2 *ImageViewer;
    vtkKWScaleWithEntry *Scale;

 private:
    vtkSlicerSliceWidget (const vtkSlicerSliceWidget &); //Not implemented
    void operator=(const vtkSlicerSliceWidget &);         //Not implemented

};

#endif

