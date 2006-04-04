#ifndef __vtkSlicerSliceWidget_h
#define __vtkSlicerSliceWidget_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkKWRenderWidget.h"
#include "vtkImageViewer2.h"
#include "vtkKWCompositeWidget.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkSlicerSliceLogic.h"


// Note:
// eventually this class will break out into 
// vtkSlicerSliceViewWidget and 
// vtkSlicerSliceControlWidget

                                                            
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerSliceWidget : public vtkKWCompositeWidget
{
    
 public:
    static vtkSlicerSliceWidget* New ( );
    vtkTypeRevisionMacro ( vtkSlicerSliceWidget, vtkKWCompositeWidget );
    void PrintSelf (ostream& os, vtkIndent indent);
    
    vtkGetMacro  ( SliceLogicObserverTag, unsigned long );
    vtkSetMacro  ( SliceLogicObserverTag, unsigned long );
    vtkGetObjectMacro ( OffsetScale, vtkKWScaleWithEntry );
    vtkGetObjectMacro ( FieldOfViewEntry, vtkKWEntryWithLabel );
    vtkGetObjectMacro ( OrientationMenu, vtkKWMenuButtonWithLabel );
    vtkGetObjectMacro ( ImageViewer, vtkImageViewer2 );
    vtkGetObjectMacro ( RenderWidget, vtkKWRenderWidget );
    vtkGetObjectMacro ( SliceLogic, vtkSlicerSliceLogic );
    vtkSetObjectMacro ( SliceLogic, vtkSlicerSliceLogic );

    virtual void Create ( );

 protected:
    vtkSlicerSliceWidget ( );
    ~vtkSlicerSliceWidget ( );

    vtkKWRenderWidget *RenderWidget;
    vtkImageViewer2 *ImageViewer;
    vtkKWScaleWithEntry *OffsetScale;
    vtkKWEntryWithLabel *FieldOfViewEntry;
    vtkKWMenuButtonWithLabel *OrientationMenu;
    vtkSlicerSliceLogic *SliceLogic;
    unsigned long SliceLogicObserverTag;
    

 private:
    vtkSlicerSliceWidget (const vtkSlicerSliceWidget &); //Not implemented
    void operator=(const vtkSlicerSliceWidget &);         //Not implemented

};

#endif

