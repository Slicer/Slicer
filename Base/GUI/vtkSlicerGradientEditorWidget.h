#ifndef __vtkSlicerGradientEditorWidget_h
#define __vtkSlicerGradientEditorWidget_h

#include "vtkSlicerWidget.h"
#include "vtkMRMLDiffusionWeightedVolumeNode.h"

class vtkKWFrameWithLabel;
class vtkKWCheckButtonSet;
class vtkKWPushButton;
class vtkKWLabel;
class vtkKWComboBox;
class vtkKWTextWithScrollbars;
class vtkSlicerNodeSelectorWidget;
class vtkKWCheckButton;
class vtkKWLoadSaveButtonWithLabel;
class vtkMatrix4x4;
class vtkDoubleArray;
class vtkKWMatrixWidget;

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
    virtual void ProcessWidgetEvents(vtkObject *caller, unsigned long event, void *callData );

    // Description:
    // Method to update the widget when a new node is loaded.
    void UpdateWidget(vtkMRMLDiffusionWeightedVolumeNode *node);

  protected:
    vtkSlicerGradientEditorWidget(void);
    virtual ~vtkSlicerGradientEditorWidget(void);

    // Description:
    // Method to create the widget.
    virtual void CreateWidget();

    // Description:
    // Method to update the GUI matrix.
    void UpdateMatrix();

    // Description:
    // Method to update the GUI GradientsTextfield.
    void UpdateGradients();

    vtkKWLoadSaveButtonWithLabel *SaveButton;
    vtkKWLoadSaveButtonWithLabel *LoadGradientsButton;
    vtkKWFrameWithLabel *MeasurementFrame;
    vtkKWFrameWithLabel *TestFrame;
    vtkKWFrameWithLabel *GradientsFrame;
    vtkKWFrameWithLabel *LoadSaveFrame;
    vtkKWFrame *ButtonsFrame;
    vtkKWMatrixWidget *MatrixGUI;
    vtkKWPushButton *NegativeButton;
    vtkKWPushButton *SwapButton;    
    vtkKWPushButton *RunButton;
    vtkKWPushButton *RotateButton;
    vtkKWLabel *AngleLabel;
    vtkKWComboBox *AngleCombobox;
    vtkKWTextWithScrollbars *GradientsTextfield;
    vtkSlicerNodeSelectorWidget *ROIMenu;
    vtkKWCheckButton *EnableMatrixButton;
    vtkKWCheckButton *EnableGradientsButton;
    vtkKWCheckButton* Checkbuttons[3];
    vtkMatrix4x4 *Matrix;
    vtkDoubleArray *Gradients;
    vtkDoubleArray *BValues;

  private:
    vtkSlicerGradientEditorWidget ( const vtkSlicerGradientEditorWidget& ); // Not implemented.
    void operator = ( const vtkSlicerGradientEditorWidget& ); //Not implemented.
  };

#endif 
