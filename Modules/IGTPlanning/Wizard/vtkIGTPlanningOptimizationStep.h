#ifndef __vtkIGTPlanningOptimizationStep_h
#define __vtkIGTPlanningOptimizationStep_h

#include "vtkIGTPlanningStep.h"

class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkKWEntryWithLabel;
class vtkKWLoadSaveButtonWithLabel;
class vtkKWPushButton;

class VTK_IGT_EXPORT vtkIGTPlanningOptimizationStep : public vtkIGTPlanningStep
{
public:
  static vtkIGTPlanningOptimizationStep *New();
  vtkTypeRevisionMacro(vtkIGTPlanningOptimizationStep,vtkIGTPlanningStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

  virtual void RunButtonCallback();
  virtual void GoToNavButtonCallback();

protected:
  vtkIGTPlanningOptimizationStep();
  ~vtkIGTPlanningOptimizationStep();

  vtkKWFrameWithLabel *RunFrame;

  vtkKWPushButton              *RunButton;
  vtkKWPushButton              *GoToNavButton;


private:
  vtkIGTPlanningOptimizationStep(const vtkIGTPlanningOptimizationStep&);
  void operator=(const vtkIGTPlanningOptimizationStep&);
};

#endif
