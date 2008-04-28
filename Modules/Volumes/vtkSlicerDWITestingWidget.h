// .NAME vtkSlicerDWITestingWidget 
// .SECTION Description
// This class implements Slicer's main DWI Testing Widget, part of the GradientEditor GUI.
// Inherits most behavior from vtkSlicerWidget.
#ifndef __vtkSlicerDWITestingWidget_h
#define __vtkSlicerDWITestingWidget_h

#include "vtkVolumes.h"
#include "vtkSlicerWidget.h"
#include "vtkSlicerMeasurementFrameWidget.h"
#include "vtkSlicerGradientsWidget.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerApplication.h"

class vtkSlicerNodeSelectorWidget;
class vtkSlicerDiffusionEditorLogic;
class vtkMRMLDiffusionWeightedVolumeNode;
class vtkMRMLFiberBundleNode;
class vtkMRMLCommandLineModuleNode;
class vtkImageData;
//widgets
class vtkKWFrameWithLabel;
class vtkKWPushButton;
class vtkKWPushButtonWithLabel;
class vtkKWCheckButton;
class vtkKWScaleWithLabel;
class vtkSlicerVolumesGUI;
class vtkSlicerTractographyDisplayGUI;

class VTK_VOLUMES_EXPORT vtkSlicerDWITestingWidget : public vtkSlicerWidget
  {
  public:

    // Description:
    // Usual vtk class functions.
    static vtkSlicerDWITestingWidget* New();
    vtkTypeRevisionMacro(vtkSlicerDWITestingWidget,vtkSlicerWidget);
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

    //BTX
    // Description:
    // Creates glyphs by invoking events in the vtkSlicerDiffusionTensorGlyphDisplayWidget.
    void CreateGlyphs(vtkKWCheckButton *calledGlyph);
    
    // Description:
    // Sets the internal value ModifiedForNewTensor when a new tensor has to be estimated.
    // That means parameters (Measurement Frame or Gradients) have changed.
    void SetModifiedForNewTensor(int modified);
    //ETX

    void SetWidgetToDefault();

    // Description:
    // Sets the Application to the current vtkSlicerApplication.
    vtkSetObjectMacro(Application, vtkSlicerApplication);

  protected:
    vtkSlicerDWITestingWidget(void);
    virtual ~vtkSlicerDWITestingWidget(void);

    // Description:
    // Creates the widget.
    virtual void CreateWidget();

    // Description:
    // Enables all buttons for visibility options of DTIs. 
    void SetAllVisibilityButtons(int status);

    // Description:
    // Is 1, if tensor has to be newly estimated.
    // Is 0, if parameters have not changed and old tensor can be used for tractography.
    int ModifiedForNewTensor;

    vtkSlicerApplication *Application;

    //nodes
    vtkMRMLDiffusionWeightedVolumeNode *ActiveVolumeNode;
    vtkMRMLDiffusionTensorVolumeNode *TensorNode;
    vtkMRMLScalarVolumeNode *BaselineNode;
    vtkMRMLScalarVolumeNode *MaskNode;
    vtkMRMLFiberBundleNode *FiberNode;

    //widgets (GUI)
    vtkKWFrameWithLabel *TestFrame;
    vtkSlicerNodeSelectorWidget *FiducialSelector;
    vtkSlicerNodeSelectorWidget *DTISelector;
    vtkKWPushButtonWithLabel *RunButton; 
    vtkKWCheckButton *ViewGlyphsRed;
    vtkKWCheckButton *ViewGlyphsGreen;
    vtkKWCheckButton *ViewGlyphsYellow;
    vtkKWCheckButton *ViewTracts;
    vtkKWFrame *GlyphFrame;
    vtkKWScaleWithLabel *GlyphSpacingScale;
    //GUI
    vtkSlicerVolumesGUI *VolumesGUI;
    vtkSlicerTractographyDisplayGUI *TractDisplayGUI;

  private:
    vtkSlicerDWITestingWidget (const vtkSlicerDWITestingWidget&); // Not implemented.
    void operator = (const vtkSlicerDWITestingWidget&); //Not implemented.
  };

#endif 
