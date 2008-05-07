// .NAME vtkSlicerDiffusionTestingWidget 
// .SECTION Description
// This class implements Slicer's main DWI Testing Widget, part of the DiffusionEditor GUI.
// Inherits most behavior from vtkSlicerWidget.
#ifndef __vtkSlicerDiffusionTestingWidget_h
#define __vtkSlicerDiffusionTestingWidget_h

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
class vtkKWPushButtonWithLabel;
class vtkKWScaleWithLabel;
class vtkKWLabel;
class vtkSlicerVisibilityIcons;

class VTK_VOLUMES_EXPORT vtkSlicerDiffusionTestingWidget : public vtkSlicerWidget
  {
  public:

    // Description:
    // Usual vtk class functions.
    static vtkSlicerDiffusionTestingWidget* New();
    vtkTypeRevisionMacro(vtkSlicerDiffusionTestingWidget,vtkSlicerWidget);
    void PrintSelf(ostream& os, vtkIndent indent);

    // Description:
    // Add/Remove observers on widgets in the GUI.
    virtual void AddWidgetObservers();
    virtual void RemoveWidgetObservers();

    // Description:
    // Propagates events generated in GUI to logic / mrml.
    void ProcessWidgetEvents(vtkObject *caller, unsigned long event, void *callData );

    // Description:
    // Updates the widget if a new ActiveVolumeNode is loaded.
    void UpdateWidget(vtkMRMLDiffusionWeightedVolumeNode *node);

    //BTX
    // Description:
    // Sets the internal value ModifiedForNewTensor if a new tensor has to be estimated.
    // That means parameters (measurement frame or gradients) have changed.
    void SetModifiedForNewTensor(int modified);
    //ETX

    // Description:
    // Sets the widget to its default status.
    void SetWidgetToDefault();

    // Description:
    // Sets TractVisibility and changes the icon of TractVisibilityButton accordingly.
    void SetTractVisibility(int status);

    // Description:
    // Sets GlyphVisibility and changes the icon of GlyphVisibilityButton accordingly.
    void SetGlyphVisibility(int plane, int status);

    // Description:
    // Computes a new tensor for a DWI by calling Tensor Estimation CLM.
    // (Method is public because of tcl testing.)
    void RunTensor();

    // Description:
    // Computes a new tensor by rotating a DTI. The measurement frame is applyed to the DTI.
    void RotateTensor();

    // Description:
    // Creates tracts by calling CreateTracts from vtkSlicerTractographyFiducialSeedingLogic.
    // (Methos is public because of tcl testing.)
    void CreateTracts();

    vtkSetObjectMacro(Application, vtkSlicerApplication);    
    vtkSetObjectMacro(TensorNode, vtkMRMLDiffusionTensorVolumeNode);
    vtkGetObjectMacro(FiducialSelector, vtkSlicerNodeSelectorWidget);
    vtkGetObjectMacro(RunButton, vtkKWPushButtonWithLabel);

  protected:
    vtkSlicerDiffusionTestingWidget(void);
    virtual ~vtkSlicerDiffusionTestingWidget(void);

    // Description:
    // Creates the widget.
    virtual void CreateWidget();   

    // Description:
    // Creates glyphs by setting the visibility of the vtkMRMLDiffusionTensorVolumeSliceDisplayNode.
    void CreateGlyphs();

    // Description:
    // Update glyph spacing by setting parameters of the vtkMRMLDiffusionTensorDisplayPropertiesNode.
    void UpdateGlyphSpacing();    

    // Description:
    // Enables/Disables all buttons for visibility of glyphs and tracts. 
    void SetAllVisibilityButtons(int status);

    // Description:
    // Is 1 if TensorNode has to be newly estimated.
    // Is 0 if parameters have not changed and existing TensorNode can be used for tractography/glyphs.
    int ModifiedForNewTensor;

    // Description:
    // Is 1 if tracts are visible; otherwise 0;
    int TractVisibility;

    // Description:
    // Is 1 if glyphs are visible; otherwise 0;
    int GlyphVisibility[3];

    // Description:
    // Counts the number of estimated tensors, needed for naming of nodes.
    int NumberOfTensorEstimations;

    vtkSlicerApplication *Application;

    //nodes
    vtkMRMLDiffusionWeightedVolumeNode *ActiveVolumeNode;
    vtkMRMLDiffusionTensorVolumeNode *TensorNode;
    vtkMRMLFiberBundleNode *FiberNode;

    //widgets (GUI)
    vtkKWFrameWithLabel *TestFrame;
    vtkSlicerNodeSelectorWidget *FiducialSelector;
    vtkSlicerNodeSelectorWidget *DTISelector;
    vtkKWPushButtonWithLabel *RunButton;
    vtkKWLabel *TractVisibilityLabel;
    vtkKWPushButtonWithLabel *TractVisibilityButton;
    vtkKWFrame *GlyphFrame;
    vtkKWLabel *GlyphVisibilityLabel;
    vtkKWPushButtonWithLabel *GlyphVisibilityButton[3];
    vtkKWScaleWithLabel *GlyphSpacingScale;
    vtkSlicerVisibilityIcons *VisibilityIcons;

  private:
    vtkSlicerDiffusionTestingWidget (const vtkSlicerDiffusionTestingWidget&); // Not implemented.
    void operator = (const vtkSlicerDiffusionTestingWidget&); //Not implemented.
  };

#endif 
