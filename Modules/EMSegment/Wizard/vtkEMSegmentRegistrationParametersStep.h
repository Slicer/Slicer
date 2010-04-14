#ifndef __vtkEMSegmentRegistrationParametersStep_h
#define __vtkEMSegmentRegistrationParametersStep_h

#include "vtkEMSegmentStep.h"

class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;
#include "vector.h"

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
#if IBM_FLAG 
  // Have to do it that way bc TCL ignores IBM_Flag
  //BTX
  virtual void RegistrationAtlasImageCallback(vtkIdType input_id,  vtkIdType volume_id);
  //ETX
#endif 

protected:
  vtkEMSegmentRegistrationParametersStep();
  ~vtkEMSegmentRegistrationParametersStep();

  vtkKWFrameWithLabel      *RegistrationParametersFrame;
#if IBM_FLAG 
  void AssignAtlasScansToInputChannels(int enabled); 
//BTX
  vector<vtkKWMenuButtonWithLabel*> RegistrationParametersAtlasImageMenuButton;
//ETX
#else 
  vtkKWMenuButtonWithLabel *RegistrationParametersAtlasImageMenuButton;
#endif 
  vtkKWMenuButtonWithLabel *RegistrationParametersAffineMenuButton;
  vtkKWMenuButtonWithLabel *RegistrationParametersDeformableMenuButton;
  vtkKWMenuButtonWithLabel *RegistrationParametersInterpolationMenuButton;

private:
  vtkEMSegmentRegistrationParametersStep(const vtkEMSegmentRegistrationParametersStep&);
  void operator=(const vtkEMSegmentRegistrationParametersStep&);
};

#endif
