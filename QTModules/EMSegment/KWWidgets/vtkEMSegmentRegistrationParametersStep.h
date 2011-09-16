#ifndef __vtkEMSegmentRegistrationParametersStep_h
#define __vtkEMSegmentRegistrationParametersStep_h

#include "vtkEMSegmentStep.h"

class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkKWFrame;

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
  virtual void RegistrationPackageCallback(int type);
  virtual void RegistrationDeformableCallback(int type);
  virtual void RegistrationAffineCallback(int type);
  virtual void RegistrationAtlasImageCallback(vtkIdType input_id,  vtkIdType volume_id);

protected:
  vtkEMSegmentRegistrationParametersStep();
  ~vtkEMSegmentRegistrationParametersStep();

  vtkKWFrameWithLabel      *RegistrationParametersFrame;
  vtkKWFrame      *RegistrationAtlasInputFrame;

  void AssignAtlasScansToInputChannels(int enabled); 
//BTX
  vtkstd::vector<vtkKWMenuButtonWithLabel*> RegistrationParametersAtlasImageMenuButton;
//ETX

  vtkKWMenuButtonWithLabel *RegistrationParametersAffineMenuButton;
  vtkKWMenuButtonWithLabel *RegistrationParametersDeformableMenuButton;
  vtkKWMenuButtonWithLabel *RegistrationParametersInterpolationMenuButton;
  vtkKWMenuButtonWithLabel *RegistrationParametersPackageMenuButton;

private:
   vtkEMSegmentRegistrationParametersStep(const vtkEMSegmentRegistrationParametersStep&);
//BTX
   vtkstd::string RegistrationTypeValueToString(int v);
//ETX
   void operator=(const vtkEMSegmentRegistrationParametersStep&);
};

#endif
