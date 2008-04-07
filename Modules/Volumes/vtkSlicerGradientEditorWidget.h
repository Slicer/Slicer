// .NAME vtkSlicerGradientEditorWidget 
// .SECTION Description
// This class implements Slicer's main GradientsEditor GUI.
// Inherits most behavior from vtkSlicerWidget.
#ifndef __vtkSlicerGradientEditorWidget_h
#define __vtkSlicerGradientEditorWidget_h

#include "vtkVolumes.h"
#include "vtkSlicerWidget.h"
#include "vtkSlicerMeasurementFrameWidget.h"
#include "vtkSlicerGradientsWidget.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerApplication.h"

class vtkSlicerNodeSelectorWidget;
class vtkSlicerGradientEditorLogic;
class vtkMRMLDiffusionWeightedVolumeNode;
class vtkMRMLFiberBundleNode;
class vtkMRMLCommandLineModuleNode;
class vtkImageData;
//widgets
class vtkKWFrameWithLabel;
class vtkKWPushButton;
class vtkKWPushButtonWithLabel;
class vtkKWCheckButton;

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
    void UpdateWidget(vtkMRMLDiffusionWeightedVolumeNode *node);

    // Description:
    // Creates tracts by calling CreateTracts from vtkSlicerTractographyFiducialSeedingLogic.
    void CreateTracts();

    vtkSetObjectMacro(Application, vtkSlicerApplication);

  protected:
    vtkSlicerGradientEditorWidget(void);
    virtual ~vtkSlicerGradientEditorWidget(void);

    // Description:
    // Creates the widget.
    virtual void CreateWidget();

    // Description:
    // Is 1, if tensor has to be newly estimated.
    // Is 0, if parameters have not changed and old tensor can be used for tractography.
    int ModifiedForNewTensor;

    vtkSlicerApplication *Application;
    vtkSlicerGradientEditorLogic *Logic;

    //nodes
    vtkMRMLDiffusionWeightedVolumeNode *ActiveVolumeNode;
    vtkMRMLDiffusionTensorVolumeNode *TensorNode;
    vtkMRMLScalarVolumeNode *BaselineNode;
    vtkMRMLScalarVolumeNode *MaskNode;
    vtkMRMLFiberBundleNode *FiberNode;

    //widgets (GUI)
    vtkSlicerMeasurementFrameWidget *MeasurementFrameWidget;
    vtkSlicerGradientsWidget *GradientsWidget;
    vtkKWFrameWithLabel *TestFrame;
    vtkSlicerNodeSelectorWidget *FiducialSelector;
    vtkSlicerNodeSelectorWidget *DTISelector;
    vtkKWPushButtonWithLabel *RunButton; 
    vtkKWFrame *RunFrame;
    vtkKWFrame *ButtonFrame;
    vtkKWPushButton *RestoreButton;
    vtkKWPushButton *UndoButton;
    vtkKWPushButton *RedoButton;
    vtkKWCheckButton *ViewGlyphs;
    vtkKWCheckButton *ViewTracts;

  private:
    vtkSlicerGradientEditorWidget (const vtkSlicerGradientEditorWidget&); // Not implemented.
    void operator = (const vtkSlicerGradientEditorWidget&); //Not implemented.
  };

#endif 
