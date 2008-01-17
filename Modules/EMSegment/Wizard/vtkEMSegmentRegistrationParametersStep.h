#ifndef __vtkEMSegmentRegistrationParametersStep_h
#define __vtkEMSegmentRegistrationParametersStep_h

#include "vtkEMSegmentStep.h"

class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;

class VTK_EMSEGMENT_EXPORT vtkEMSegmentRegistrationParametersStep : public vtkEMSegmentStep
{
public:
  static vtkEMSegmentRegistrationParametersStep *New();
  vtkTypeRevisionMacro(vtkEMSegmentRegistrationParametersStep,vtkEMSegmentStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

  // Description:
  // Callbacks.
  virtual void RegistrationAtlasImageCallback(vtkIdType volumeId);
  virtual void RegistrationInterpolationCallback(int type);
  virtual void RegistrationDeformableCallback(int type);
  virtual void RegistrationAffineCallback(int type);

protected:
  vtkEMSegmentRegistrationParametersStep();
  ~vtkEMSegmentRegistrationParametersStep();

  vtkKWFrameWithLabel      *RegistrationParametersFrame;
  vtkKWMenuButtonWithLabel *RegistrationParametersAtlasImageMenuButton;
  vtkKWMenuButtonWithLabel *RegistrationParametersAffineMenuButton;
  vtkKWMenuButtonWithLabel *RegistrationParametersDeformableMenuButton;
  vtkKWMenuButtonWithLabel *RegistrationParametersInterpolationMenuButton;

private:
  vtkEMSegmentRegistrationParametersStep(const vtkEMSegmentRegistrationParametersStep&);
  void operator=(const vtkEMSegmentRegistrationParametersStep&);
};

#endif
