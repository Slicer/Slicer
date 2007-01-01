#ifndef __vtkEMSegmentParametersSetStep_h
#define __vtkEMSegmentParametersSetStep_h

#include "vtkEMSegmentStep.h"

class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;

class VTK_EMSEGMENT_EXPORT vtkEMSegmentParametersSetStep : public vtkEMSegmentStep
{
public:
  static vtkEMSegmentParametersSetStep *New();
  vtkTypeRevisionMacro(vtkEMSegmentParametersSetStep,vtkEMSegmentStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

  // Description:
  // Update the list of loaded parameter sets.
  virtual void UpdateLoadedParameterSets();

  // Description:
  // Callbacks.
  virtual void SelectedParameterSetChangedCallback(int index);

protected:
  vtkEMSegmentParametersSetStep();
  ~vtkEMSegmentParametersSetStep();

  vtkKWFrameWithLabel      *ParameterSetFrame;
  vtkKWMenuButtonWithLabel *ParameterSetMenuButton;

  // Description:
  // Populate the list of loaded parameter sets.
  virtual void PopulateLoadedParameterSets(vtkObject *obj, const char *method);

private:
  vtkEMSegmentParametersSetStep(const vtkEMSegmentParametersSetStep&);
  void operator=(const vtkEMSegmentParametersSetStep&);
};

#endif
