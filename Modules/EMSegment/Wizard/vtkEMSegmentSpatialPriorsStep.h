#ifndef __vtkEMSegmentSpatialPriorsStep_h
#define __vtkEMSegmentSpatialPriorsStep_h

#include "vtkEMSegmentStep.h"

class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;

class VTK_EMSEGMENT_EXPORT vtkEMSegmentSpatialPriorsStep : public vtkEMSegmentStep
{
public:
  static vtkEMSegmentSpatialPriorsStep *New();
  vtkTypeRevisionMacro(vtkEMSegmentSpatialPriorsStep,vtkEMSegmentStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

  // Description:
  // Callbacks.
  virtual void SpatialPriorsVolumeCallback(vtkIdType, vtkIdType volumeId);
  virtual void DisplaySelectedNodeSpatialPriorsCallback();

protected:
  vtkEMSegmentSpatialPriorsStep();
  ~vtkEMSegmentSpatialPriorsStep();

  vtkKWFrameWithLabel      *SpatialPriorsVolumeFrame;
  vtkKWMenuButtonWithLabel *SpatialPriorsVolumeMenuButton;

private:
  vtkEMSegmentSpatialPriorsStep(const vtkEMSegmentSpatialPriorsStep&);
  void operator=(const vtkEMSegmentSpatialPriorsStep&);
};

#endif
