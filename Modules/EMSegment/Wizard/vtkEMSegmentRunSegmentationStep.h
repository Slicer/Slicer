#ifndef __vtkEMSegmentRunSegmentationStep_h
#define __vtkEMSegmentRunSegmentationStep_h

#include "vtkEMSegmentStep.h"

class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkSlicerNodeSelectorWidget;
class vtkKWLoadSaveButton;
class vtkKWCheckButtonWithLabel;
class vtkKWMatrixWidgetWithLabel;
class vtkKWMatrixWidget;
class vtkKWFrame;

class VTK_EMSEGMENT_EXPORT vtkEMSegmentRunSegmentationStep : public vtkEMSegmentStep
{
public:
  static vtkEMSegmentRunSegmentationStep *New();
  vtkTypeRevisionMacro(vtkEMSegmentRunSegmentationStep,vtkEMSegmentStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();
  virtual void HideUserInterface();

  // Description:
  // Callbacks.
  virtual void SelectTemplateFileCallback();
  virtual void SelectDirectoryCallback();
  virtual void GenerateSurfaceModelsCallback(int state);
  virtual void SaveIntermediateCallback(int state);
  virtual void SaveAfterSegmentationCallback(int state);
  virtual void RunSegmentationROIMaxChangedCallback(
    int row, int col, const char *value);
  virtual void RunSegmentationROIMinChangedCallback(
    int row, int col, const char *value);
  virtual void MultiThreadingCallback(int state);
  virtual void StartSegmentationCallback();
  virtual void CancelSegmentationCallback();

  // Description:
  // Observers.
  virtual void AddRunRegistrationOutputGUIObservers();
  virtual void RemoveRunRegistrationOutputGUIObservers();
  virtual void ProcessRunRegistrationOutputGUIEvents(
    vtkObject *caller, unsigned long event, void *callData);

protected:
  vtkEMSegmentRunSegmentationStep();
  ~vtkEMSegmentRunSegmentationStep();

  vtkKWFrameWithLabel        *RunSegmentationSaveFrame;
  vtkKWFrameWithLabel        *RunSegmentationDirectoryFrame;
  vtkKWFrameWithLabel        *RunSegmentationOutputFrame;
  vtkSlicerNodeSelectorWidget* RunSegmentationOutVolumeSelector;
  vtkKWLoadSaveButton        *RunSegmentationSaveTemplateButton;
  vtkKWFrame                 *RunSegmentationDirectorySubFrame;
  vtkKWLoadSaveButton         *RunSegmentationDirectoryButton;
  vtkKWCheckButtonWithLabel  *RunSegmentationSaveAfterSegmentationCheckButton;
  vtkKWCheckButtonWithLabel  *RunSegmentationSaveIntermediateCheckButton;
  vtkKWCheckButtonWithLabel  *RunSegmentationGenerateSurfaceCheckButton;
  vtkKWFrameWithLabel        *RunSegmentationROIFrame;
  vtkKWMatrixWidgetWithLabel *RunSegmentationROIMinMatrix;
  vtkKWMatrixWidgetWithLabel *RunSegmentationROIMaxMatrix;
  vtkKWFrameWithLabel        *RunSegmentationMiscFrame;
  vtkKWCheckButtonWithLabel  *RunSegmentationMultiThreadCheckButton;

  // Description:
  // Populate the run segmentation boundary matrix 
  void PopulateSegmentationROIMatrix(
    vtkKWMatrixWidget* matrix, int ijk[3]);

private:
  vtkEMSegmentRunSegmentationStep(const vtkEMSegmentRunSegmentationStep&);
  void operator=(const vtkEMSegmentRunSegmentationStep&);
};

#endif
