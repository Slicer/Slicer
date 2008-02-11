// .NAME vtkSlicerMeasurementFrameWidget 
// .SECTION Description
// This class implements Slicer's DWI Measurement Frame widget, part of the GradientEditor GUI.
// Inherits most behavior from vtkSlicerWidget.
#ifndef __vtkSlicerMeasurementFrameWidget_h
#define __vtkSlicerMeasurementFrameWidget_h

#include "vtkSlicerWidget.h"

class vtkMRMLDiffusionWeightedVolumeNode;
class vtkMatrix4x4;
//widgets
class vtkKWPushButton;
class vtkKWMatrixWidget;
class vtkKWLabel;
class vtkKWComboBox;
class vtkKWCheckButton;
class vtkKWFrameWithLabel;

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerMeasurementFrameWidget : public vtkSlicerWidget
  {
  public:

    // Description:
    // Usual vtk class functions.
    static vtkSlicerMeasurementFrameWidget* New();
    vtkTypeRevisionMacro(vtkSlicerMeasurementFrameWidget,vtkSlicerWidget);
    void PrintSelf(ostream& os, vtkIndent indent);

    // Description:
    // Add/Remove observers on widgets in the GUI.
    virtual void AddWidgetObservers();
    virtual void RemoveWidgetObservers();

    // Description:
    // Method to propagate events generated in GUI to logic / mrml.
    void ProcessWidgetEvents(vtkObject *caller, unsigned long event, void *callData);

    // Description:
    // Method to update the widget when a new node is loaded.
    void UpdateWidget(vtkMRMLDiffusionWeightedVolumeNode *dwiNode);

  protected:
    vtkSlicerMeasurementFrameWidget(void);
    virtual ~vtkSlicerMeasurementFrameWidget(void);

    // Description:
    // Method to create the widget.
    virtual void CreateWidget();

    // Description:
    // Method to update the matrixWidget (GUI).
    void UpdateMatrix();

    vtkMRMLDiffusionWeightedVolumeNode *ActiveVolumeNode;
    vtkMatrix4x4 *Matrix;
    //widgets (GUI)
    vtkKWFrameWithLabel *MeasurementFrame;
    vtkKWMatrixWidget *MatrixWidget;    
    vtkKWPushButton *NegativeButton;
    vtkKWPushButton *SwapButton;    
    vtkKWPushButton *RotateButton;
    vtkKWLabel *AngleLabel;
    vtkKWComboBox *AngleCombobox;
    vtkKWCheckButton* Checkbuttons[3];

  private:
    vtkSlicerMeasurementFrameWidget (const vtkSlicerMeasurementFrameWidget&); // Not implemented.
    void operator = (const vtkSlicerMeasurementFrameWidget&); //Not implemented.
  };

#endif 
