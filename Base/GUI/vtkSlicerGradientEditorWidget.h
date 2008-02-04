#ifndef __vtkSlicerGradientEditorWidget_h
#define __vtkSlicerGradientEditorWidget_h

#include "vtkSlicerWidget.h"

class vtkKWFrameWithLabel;
class vtkKWFrame;
class vtkKWPushButton;
class vtkKWLabel;
class vtkKWComboBox;
class vtkKWTextWithScrollbars;
class vtkSlicerNodeSelectorWidget;
class vtkKWCheckButton;
class vtkKWLoadSaveButtonWithLabel;
class vtkMatrix4x4;
class vtkKWMatrixWidget;
class vtkMRMLDiffusionWeightedVolumeNode;
class vtkKWLabel;
class vtkDoubleArray;

// Description:
// This class implements Slicer's GradientsEditor GUI.
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerGradientEditorWidget : public vtkSlicerWidget
  {
  public:

    // Description:
    // Usual vtk class functions
    static vtkSlicerGradientEditorWidget* New();
    vtkTypeRevisionMacro(vtkSlicerGradientEditorWidget,vtkSlicerWidget);
    void PrintSelf (ostream& os, vtkIndent indent );

    // Description:
    // Add/Remove observers on widgets in the GUI
    virtual void AddWidgetObservers ( );
    virtual void RemoveWidgetObservers ( );

    // Description:
    // Method to propagate events generated in GUI to logic / mrml
    void ProcessWidgetEvents(vtkObject *caller, unsigned long event, void *callData );
    void ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData );

    // Description:
    // Method to update the widget when a new node is loaded.
    void UpdateWidget(vtkMRMLDiffusionWeightedVolumeNode *node);

    // Description:
    // Method to propagate keypress-events generated in the textbox of gradients.
    void TextFieldModifiedCallback();

  protected:
    vtkSlicerGradientEditorWidget(void);
    virtual ~vtkSlicerGradientEditorWidget(void);

    // Description:
    // Method to create the widget.
    virtual void CreateWidget();

    // Description:
    // Method to update the matrix (GUI).
    void UpdateMatrix();

    // Description:
    // Method to update the gradientsTextbox containing bValue and gradients (GUI).
    void UpdateGradients();

    vtkKWLoadSaveButtonWithLabel *LoadGradientsButton;
    vtkKWFrameWithLabel *MeasurementFrame;
    vtkKWFrameWithLabel *TestFrame;
    vtkKWFrameWithLabel *GradientsFrame;
    vtkKWFrame *ButtonsFrame;
    vtkKWMatrixWidget *MatrixGUI;
    vtkKWPushButton *NegativeButton;
    vtkKWPushButton *SwapButton;    
    vtkKWPushButton *RunButton;
    vtkKWPushButton *RotateButton;
    vtkKWPushButton *CancelButton;
    vtkKWLabel *AngleLabel;
    vtkKWComboBox *AngleCombobox;
    vtkKWTextWithScrollbars *GradientsTextbox;
    vtkKWLabel *StatusLabel;
    vtkSlicerNodeSelectorWidget *Mask;
    vtkKWCheckButton *EnableGradientsButton;
    vtkKWCheckButton* Checkbuttons[3];
    vtkMatrix4x4 *Matrix;
    vtkMRMLDiffusionWeightedVolumeNode *ActiveVolumeNode;

  private:
    vtkSlicerGradientEditorWidget ( const vtkSlicerGradientEditorWidget& ); // Not implemented.
    void operator = ( const vtkSlicerGradientEditorWidget& ); //Not implemented.
  };

#endif 
