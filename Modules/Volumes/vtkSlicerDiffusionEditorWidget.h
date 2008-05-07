// .NAME vtkSlicerDiffusionEditorWidget 
// .SECTION Description
// This class implements Slicer's main DiffusionEditor GUI.
// Inherits most behavior from vtkSlicerWidget.
#ifndef __vtkSlicerDiffusionEditorWidget_h
#define __vtkSlicerDiffusionEditorWidget_h

#include "vtkVolumes.h"
#include "vtkSlicerWidget.h"
#include "vtkSlicerApplication.h"

class vtkSlicerDiffusionEditorLogic;
class vtkMRMLDiffusionWeightedVolumeNode;
class vtkSlicerMeasurementFrameWidget;
class vtkSlicerDiffusionTestingWidget;
class vtkSlicerGradientsWidget;
//widgets
class vtkKWFrame;
class vtkKWPushButton;

class VTK_VOLUMES_EXPORT vtkSlicerDiffusionEditorWidget : public vtkSlicerWidget
  {
  public:

    // Description:
    // Usual vtk class functions.
    static vtkSlicerDiffusionEditorWidget* New();
    vtkTypeRevisionMacro(vtkSlicerDiffusionEditorWidget,vtkSlicerWidget);
    void PrintSelf(ostream& os, vtkIndent indent);

    // Description:
    // Add/Remove observers on KWwidgets in the GUI.
    virtual void AddWidgetObservers();
    virtual void RemoveWidgetObservers();

    // Description:
    // Propagates events generated in GUI to logic / mrml.
    void ProcessWidgetEvents(vtkObject *caller, unsigned long event, void *callData );

    // Description:
    // Updates the widget if a new ActiveVolumeNode is loaded.
    void UpdateWidget(vtkMRMLVolumeNode *node);

    // Description:
    // Sets the Application to the current vtkSlicerApplication.
    vtkSetObjectMacro(Application, vtkSlicerApplication);

    // Description:
    // Returns the logic of the DiffusionEditor.
    vtkGetObjectMacro(Logic, vtkSlicerDiffusionEditorLogic);

    vtkGetObjectMacro(TestingWidget, vtkSlicerDiffusionTestingWidget);

  protected:
    vtkSlicerDiffusionEditorWidget(void);
    virtual ~vtkSlicerDiffusionEditorWidget(void);

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
    vtkSlicerDiffusionTestingWidget *TestingWidget;
    vtkKWFrame *ButtonFrame;
    vtkKWPushButton *RestoreButton;
    vtkKWPushButton *UndoButton;
    vtkKWPushButton *RedoButton;

  private:
    vtkSlicerDiffusionEditorWidget (const vtkSlicerDiffusionEditorWidget&); // Not implemented.
    void operator = (const vtkSlicerDiffusionEditorWidget&); //Not implemented.
  };

#endif 
