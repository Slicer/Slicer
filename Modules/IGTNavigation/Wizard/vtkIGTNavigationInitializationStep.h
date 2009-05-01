#ifndef __vtkIGTNavigationInitializationStep_h
#define __vtkIGTNavigationInitializationStep_h

#include "vtkIGTNavigationStep.h"

class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;

class VTK_IGT_EXPORT vtkIGTNavigationInitializationStep : public vtkIGTNavigationStep
{
public:
  static vtkIGTNavigationInitializationStep *New();
  vtkTypeRevisionMacro(vtkIGTNavigationInitializationStep,vtkIGTNavigationStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

protected:
  vtkIGTNavigationInitializationStep();
  ~vtkIGTNavigationInitializationStep();

  vtkKWFrameWithLabel           *TrackingFrame;
  vtkKWFrameWithLabel           *EndoscopeCameraFrame;

private:
  vtkIGTNavigationInitializationStep(const vtkIGTNavigationInitializationStep&);
  void operator=(const vtkIGTNavigationInitializationStep&);
};

#endif
