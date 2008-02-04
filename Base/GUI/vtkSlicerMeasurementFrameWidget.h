#ifndef __vtkSlicerMeasurementFrameWidget_h
#define __vtkSlicerMeasurementFrameWidget_h

#include "vtkSlicerWidget.h"


// Description:
// This class implements Slicer's DWI Measurement Frame GUI.
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerMeasurementFrameWidget : public vtkSlicerWidget
  {
  public:

    // Description:
    // Usual vtk class functions
    static vtkSlicerMeasurementFrameWidget* New();
    vtkTypeRevisionMacro(vtkSlicerMeasurementFrameWidget,vtkSlicerWidget);
    void PrintSelf (ostream& os, vtkIndent indent );

    // Description:
    // Add/Remove observers on widgets in the GUI
    virtual void AddWidgetObservers ( );
    virtual void RemoveWidgetObservers ( );

    // Description:
    // Method to propagate events generated in GUI to logic / mrml
    void ProcessWidgetEvents(vtkObject *caller, unsigned long event, void *callData );
    void ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData );

  protected:
    vtkSlicerMeasurementFrameWidget(void);
    virtual ~vtkSlicerMeasurementFrameWidget(void);

    // Description:
    // Method to create the widget.
    virtual void CreateWidget();
 
  private:
    vtkSlicerMeasurementFrameWidget ( const vtkSlicerMeasurementFrameWidget& ); // Not implemented.
    void operator = ( const vtkSlicerMeasurementFrameWidget& ); //Not implemented.
  };

#endif 
