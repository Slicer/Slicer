#ifndef __vtkEMSegmentIntensityImagesStep_h
#define __vtkEMSegmentIntensityImagesStep_h

#include "vtkEMSegmentStep.h"

class vtkKWFrameWithLabel;
class vtkKWListBoxToListBoxSelectionEditor;
class vtkKWCheckButtonWithLabel;

class VTK_EMSEGMENT_EXPORT vtkEMSegmentIntensityImagesStep : public vtkEMSegmentStep
{
public:
  static vtkEMSegmentIntensityImagesStep *New();
  vtkTypeRevisionMacro(vtkEMSegmentIntensityImagesStep,vtkEMSegmentStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

  // Description:
  // Callbacks.
  virtual void IntensityImagesTargetSelectionChangedCallback();
  virtual void AlignTargetImagesCallback(int state);

  // Description:
  // Reimplement the superclass's method.
  virtual void Validate();

protected:
  vtkEMSegmentIntensityImagesStep();
  ~vtkEMSegmentIntensityImagesStep();

  virtual void PopulateIntensityImagesTargetVolumeSelector();

  vtkKWFrameWithLabel                  *IntensityImagesTargetSelectorFrame;
  vtkKWListBoxToListBoxSelectionEditor *IntensityImagesTargetVolumeSelector;
  vtkKWFrameWithLabel        *TargetToTargetRegistrationFrame;
  vtkKWCheckButtonWithLabel  *IntensityImagesAlignTargetImagesCheckButton;

private:
  vtkEMSegmentIntensityImagesStep(const vtkEMSegmentIntensityImagesStep&);
  void operator=(const vtkEMSegmentIntensityImagesStep&);
};

#endif
