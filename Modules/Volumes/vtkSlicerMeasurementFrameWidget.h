// .NAME vtkSlicerMeasurementFrameWidget 
// .SECTION Description
// This class implements Slicer's DWI Measurement Frame widget, part of the GradientEditor GUI.
// Inherits most behavior from vtkSlicerWidget.
#ifndef __vtkSlicerMeasurementFrameWidget_h
#define __vtkSlicerMeasurementFrameWidget_h

#include "vtkVolumes.h"
#include "vtkSlicerWidget.h"
#include "vtkSlicerGradientEditorLogic.h"

class vtkMRMLDiffusionWeightedVolumeNode;
class vtkMatrix4x4;
//widgets
class vtkKWPushButton;
class vtkKWMatrixWidget;
class vtkKWLabel;
class vtkKWComboBox;
class vtkKWCheckButton;
class vtkKWFrameWithLabel;

class VTK_VOLUMES_EXPORT vtkSlicerMeasurementFrameWidget : public vtkSlicerWidget
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
    // Propagates events generated in GUI to logic / mrml.
    virtual void ProcessWidgetEvents(vtkObject *caller, unsigned long event, void *callData);

    // Description:
    // Updates the widget when a new node is loaded.
    void UpdateWidget(vtkMRMLVolumeNode *node);

    // Description:
    // Event is invoked when a matrix value changed.
    //BTX
    enum
      {
      ChangedEvent = 1234,
      };
    //ETX

    void SetLogic(vtkSlicerGradientEditorLogic *logic);

  protected:
    vtkSlicerMeasurementFrameWidget(void);
    virtual ~vtkSlicerMeasurementFrameWidget(void);

    // Description:
    // Creates the widget.
    virtual void CreateWidget();

    // Description:
    // Updates the matrixWidget (GUI).
    void UpdateMatrix();

    // Description:
    // Saves changes of the matrix to the ActiveVolumeNode.
    void SaveMatrix();

    // Description:
    // Return value is 1 if Determinat is +1 or -1; otherwise 0;
    int CheckDeterminant();

    vtkMRMLDiffusionWeightedVolumeNode *ActiveVolumeNode;
    vtkMatrix4x4 *Matrix;
    vtkSlicerGradientEditorLogic *Logic;
    //widgets (GUI)
    vtkKWFrameWithLabel *MeasurementFrame;
    vtkKWMatrixWidget *MatrixWidget;    
    vtkKWPushButton *NegativeButton;
    vtkKWPushButton *SwapButton;    
    vtkKWPushButton *RotateButton;
    vtkKWPushButton *IdentityButton;
    vtkKWLabel *AngleLabel;
    vtkKWComboBox *AngleCombobox;
    vtkKWCheckButton* Checkbuttons[3];

  private:
    vtkSlicerMeasurementFrameWidget (const vtkSlicerMeasurementFrameWidget&); // Not implemented.
    void operator = (const vtkSlicerMeasurementFrameWidget&); //Not implemented.
  };

#endif 
