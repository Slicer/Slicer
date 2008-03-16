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
class vtkMRMLDiffusionWeightedVolumeNode;
class vtkMRMLFiberBundleNode;
class vtkMRMLCommandLineModuleNode;
class vtkImageData;
//widgets
class vtkKWFrameWithLabel;
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
    // Method to propagate events generated in GUI to logic / mrml.
    void ProcessWidgetEvents(vtkObject *caller, unsigned long event, void *callData );

    // Description:
    // Method to update the widget when a new node is loaded.
    void UpdateWidget(vtkMRMLDiffusionWeightedVolumeNode *node);

    // Description:
    // Method to create tracts with calling CreateTracts from vtkSlicerTractographyFiducialSeedingLogic.
    void CreateTracts();

    vtkSetObjectMacro(Application, vtkSlicerApplication);

  protected:
    vtkSlicerGradientEditorWidget(void);
    virtual ~vtkSlicerGradientEditorWidget(void);

    // Description:
    // Method to create the widget.
    virtual void CreateWidget();

    vtkMRMLDiffusionWeightedVolumeNode *ActiveVolumeNode;
    vtkMRMLDiffusionWeightedVolumeNode *OriginalNode;
    vtkMRMLDiffusionTensorVolumeNode *TensorNode;
    vtkMRMLScalarVolumeNode *BaselineNode;
    vtkMRMLScalarVolumeNode *MaskNode;
    vtkMRMLFiberBundleNode *FiberNode;
    vtkSlicerApplication *Application;
    vtkMRMLCommandLineModuleNode *TensorCML;

    // Description:
    // Number of changes in gradients/measurement frame of ActiveVolumeNode.
    int NumberOfChanges;
    
    // Description:
    // Is 1, if tensor has to be newly estimated.
    // Is 0, if parameters have not changed and old tensor can be used for tractography.
    int ModifiedForNewTensor;
    
    //widgets (GUI)
    vtkSlicerMeasurementFrameWidget *MeasurementFrameWidget;
    vtkSlicerGradientsWidget *GradientsWidget;
    vtkKWFrameWithLabel *TestFrame;
    vtkSlicerNodeSelectorWidget *FiducialSelector;
    vtkSlicerNodeSelectorWidget *DTISelector;
    vtkKWPushButton *RunButton; 
    vtkKWFrame *RunFrame;
    vtkKWFrame *ButtonFrame;
    vtkKWPushButton *RestoreButton;
    vtkKWPushButton *UndoButton;
    vtkKWPushButton *RedoButton;
       
  private:
    vtkSlicerGradientEditorWidget (const vtkSlicerGradientEditorWidget&); // Not implemented.
    void operator = (const vtkSlicerGradientEditorWidget&); //Not implemented.
  };

#endif 
