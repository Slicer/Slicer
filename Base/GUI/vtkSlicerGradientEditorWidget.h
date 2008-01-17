#ifndef __vtkSlicerGradientEditorWidget_h
#define __vtkSlicerGradientEditorWidget_h

#define ARRAY_LENGTH 3
#include "vtkSlicerWidget.h"
#include "vtkMRMLDiffusionWeightedVolumeNode.h"
class vtkKWFrameWithLabel;
class vtkKWEntrySet;
class vtkKWCheckButtonSet;
class vtkKWPushButton;
class vtkKWLabel;
class vtkKWComboBox;
class vtkKWTextWithScrollbars;
class vtkSlicerNodeSelectorWidget;
class vtkKWCheckButton;
class vtkKWLoadSaveButtonWithLabel;
class vtkMatrix4x4;

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerGradientEditorWidget : public vtkSlicerWidget
{
public:
    static vtkSlicerGradientEditorWidget* New();
    vtkTypeRevisionMacro(vtkSlicerGradientEditorWidget,vtkSlicerWidget);

    // Description:
    // Add/Remove observers on widgets in the GUI
    virtual void AddWidgetObservers ( );
    virtual void RemoveWidgetObservers ( );

    // Description:
    // Method to propagate events generated in GUI to logic / mrml
    virtual void ProcessWidgetEvents(vtkObject *caller, unsigned long event, void *callData );

    virtual void UpdateWidget(vtkMRMLVolumeNode *node);

protected:
    vtkSlicerGradientEditorWidget(void);
    virtual ~vtkSlicerGradientEditorWidget(void);

    // Description:
    // Create the widget.
    virtual void CreateWidget();

    // Description:
    // Update the GUI Matrix.
    void UpdateMatrix();

    void PrintSelf (ostream& os, vtkIndent indent );

    vtkKWLoadSaveButtonWithLabel *SaveButton;
    vtkKWLoadSaveButtonWithLabel *LoadButton;
    vtkKWLoadSaveButtonWithLabel *LoadGradientsButton;
    vtkKWFrameWithLabel *MeasurementFrame;
    vtkKWFrameWithLabel *TestFrame;
    vtkKWFrameWithLabel *GradientsFrame;
    vtkKWFrameWithLabel *LoadSaveFrame;
    vtkKWFrame *ButtonsFrame;
    vtkKWEntrySet *MatrixGUI;
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
    vtkKWCheckButton* Checkbuttons[ARRAY_LENGTH];
    vtkMatrix4x4 *Matrix;

private:
    vtkSlicerGradientEditorWidget ( const vtkSlicerGradientEditorWidget& ); // Not implemented.
    void operator = ( const vtkSlicerGradientEditorWidget& ); //Not implemented.
};

#endif 
