// .NAME vtkSlicerGradientEditorWidget 
// .SECTION Description
// This class implements Slicer's main GradientsEditor GUI.
// Inherits most behavior from vtkSlicerWidget.
#ifndef __vtkSlicerGradientEditorWidget_h
#define __vtkSlicerGradientEditorWidget_h

#include "vtkVolumes.h"
#include "vtkSlicerWidget.h"
#include "vtkSlicerApplication.h"

class vtkSlicerDiffusionEditorLogic;
class vtkMRMLDiffusionWeightedVolumeNode;
class vtkSlicerMeasurementFrameWidget;
class vtkSlicerDWITestingWidget;
class vtkSlicerGradientsWidget;
//widgets
class vtkKWFrame;
class vtkKWPushButton;

class VTK_VOLUMES_EXPORT vtkSlicerGradientEditorWidget : public vtkSlicerWidget
  {
  public:

    // Description:
    // Usual vtk class functions.
    static vtkSlicerGradientEditorWidget* New();
    vtkTypeRevisionMacro(vtkSlicerGradientEditorWidget,vtkSlicerWidget);
    void PrintSelf(ostream& os, vtkIndent indent);

    // Description:
    // Add/Remove observers on widgets in the GUI.
    virtual void AddWidgetObservers();
    virtual void RemoveWidgetObservers();

    // Description:
    // Propagates events generated in GUI to logic / mrml.
    void ProcessWidgetEvents(vtkObject *caller, unsigned long event, void *callData );

    // Description:
    // Updates the widget when a new node is loaded.
    void UpdateWidget(vtkMRMLVolumeNode *node);

    // Description:
    // Sets the Application to the current vtkSlicerApplication.
    vtkSetObjectMacro(Application, vtkSlicerApplication);

    vtkGetObjectMacro(Logic, vtkSlicerDiffusionEditorLogic);

  protected:
    vtkSlicerGradientEditorWidget(void);
    virtual ~vtkSlicerGradientEditorWidget(void);

    // Description:
    // Creates the widget.
    virtual void CreateWidget();

    vtkSlicerApplication *Application;
    vtkSlicerDiffusionEditorLogic *Logic;

    //nodes
    vtkMRMLDiffusionWeightedVolumeNode *ActiveVolumeNode;

    //widgets (GUI)
    vtkSlicerMeasurementFrameWidget *MeasurementFrameWidget;
    vtkSlicerGradientsWidget *GradientsWidget;
    vtkSlicerDWITestingWidget *TestingWidget;
    vtkKWFrame *ButtonFrame;
    vtkKWPushButton *RestoreButton;
    vtkKWPushButton *UndoButton;
    vtkKWPushButton *RedoButton;

  private:
    vtkSlicerGradientEditorWidget (const vtkSlicerGradientEditorWidget&); // Not implemented.
    void operator = (const vtkSlicerGradientEditorWidget&); //Not implemented.
  };

#endif 
