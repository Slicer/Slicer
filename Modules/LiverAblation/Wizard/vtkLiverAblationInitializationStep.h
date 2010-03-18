#ifndef __vtkLiverAblationInitializationStep_h
#define __vtkLiverAblationInitializationStep_h

#include "vtkLiverAblationStep.h"

class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;

class VTK_LIVERABLATION_EXPORT vtkLiverAblationInitializationStep : public vtkLiverAblationStep
{
public:
  static vtkLiverAblationInitializationStep *New();
  vtkTypeRevisionMacro(vtkLiverAblationInitializationStep,vtkLiverAblationStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

protected:
  vtkLiverAblationInitializationStep();
  ~vtkLiverAblationInitializationStep();

  vtkKWFrameWithLabel           *TrackingFrame;
  vtkKWFrameWithLabel           *EndoscopeCameraFrame;

private:
  vtkLiverAblationInitializationStep(const vtkLiverAblationInitializationStep&);
  void operator=(const vtkLiverAblationInitializationStep&);
};

#endif
