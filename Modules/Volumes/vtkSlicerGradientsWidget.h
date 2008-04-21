// .NAME vtkSlicerGradientsWidget 
// .SECTION Description
// This class implements Slicer's DWI Gradients widget, part of the GradientEditor GUI.
// Inherits most behavior from vtkSlicerWidget.
#ifndef __vtkSlicerGradientsWidget_h
#define __vtkSlicerGradientsWidget_h

#include "vtkVolumes.h"
#include "vtkSlicerWidget.h"
#include "vtkSlicerGradientEditorLogic.h"

class vtkMRMLDiffusionWeightedVolumeNode;
class vtkDoubleArray;
//widgets
class vtkKWFrameWithLabel;
class vtkKWFrame;
class vtkKWLoadSaveButtonWithLabel;
class vtkKWCheckButton;
class vtkKWTextWithScrollbars ;
class vtkKWLabel;
class vtkKWMessageDialog;
class vtkKWPushButton;

class VTK_VOLUMES_EXPORT vtkSlicerGradientsWidget : public vtkSlicerWidget
  {
  public:

    // Description:
    // Usual vtk class functions.
    static vtkSlicerGradientsWidget* New();
    vtkTypeRevisionMacro(vtkSlicerGradientsWidget,vtkSlicerWidget);
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
    void UpdateWidget(vtkMRMLDiffusionWeightedVolumeNode *dwiNode);

    // Description:
    // Propagates keypress-events generated in the gradientsTextbox.
    void TextFieldModifiedCallback();

    // Description:
    // Event is invoked when a matrix value changed.
    //BTX
    enum
      {
      ChangedEvent = 1234,
      };
    //ETX

    void SetLogic(vtkSlicerGradientEditorLogic *logic);

    void SetStatus(int status);

  protected:
    vtkSlicerGradientsWidget(void);
    virtual ~vtkSlicerGradientsWidget(void);

    // Description:
    // Creates the widget.
    virtual void CreateWidget();

    // Description:
    // Updates the gradientsTextbox containing bValue and gradients (GUI).
    void UpdateGradients();

    // Description:
    // Updates the status label for the gradients (GUI).
    void UpdateStatusLabel(int status);

    // Description:
    // Saves changes of the gradients/bValues to the ActiveVolumeNode.
    void SaveGradients();

    // Description:
    // Displays a message dialog to the user (GUI).
    void DisplayMessageDialog(const char* message);

    vtkMRMLDiffusionWeightedVolumeNode *ActiveVolumeNode;
    vtkDoubleArray *Gradients;
    vtkDoubleArray *BValues;
    vtkSlicerGradientEditorLogic *Logic;
    //widgets (GUI)
    vtkKWFrameWithLabel *GradientsFrame;
    vtkKWFrame *ButtonsFrame;
    vtkKWLoadSaveButtonWithLabel *LoadGradientsButton;
    vtkKWCheckButton *EnableGradientsButton;
    vtkKWTextWithScrollbars *GradientsTextbox;
    vtkKWLabel *StatusLabel;
    vtkKWMessageDialog *MessageDialog;

  private:
    vtkSlicerGradientsWidget (const vtkSlicerGradientsWidget&); // Not implemented.
    void operator = (const vtkSlicerGradientsWidget&); //Not implemented.
  };

#endif 
