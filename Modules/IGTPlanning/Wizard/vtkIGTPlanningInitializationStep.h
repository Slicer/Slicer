#ifndef __vtkIGTPlanningInitializationStep_h
#define __vtkIGTPlanningInitializationStep_h

#include "vtkIGTPlanningStep.h"

class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;

class VTK_IGT_EXPORT vtkIGTPlanningInitializationStep : public vtkIGTPlanningStep
{
public:
  static vtkIGTPlanningInitializationStep *New();
  vtkTypeRevisionMacro(vtkIGTPlanningInitializationStep,vtkIGTPlanningStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

protected:
  vtkIGTPlanningInitializationStep();
  ~vtkIGTPlanningInitializationStep();

  vtkKWFrameWithLabel           *TrackingFrame;
  vtkKWFrameWithLabel           *EndoscopeCameraFrame;

private:
  vtkIGTPlanningInitializationStep(const vtkIGTPlanningInitializationStep&);
  void operator=(const vtkIGTPlanningInitializationStep&);
};

#endif
