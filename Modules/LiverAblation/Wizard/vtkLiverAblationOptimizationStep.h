#ifndef __vtkLiverAblationOptimizationStep_h
#define __vtkLiverAblationOptimizationStep_h

#include "vtkLiverAblationStep.h"

class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkKWEntryWithLabel;
class vtkKWLoadSaveButtonWithLabel;
class vtkKWPushButton;

class VTK_LIVERABLATION_EXPORT vtkLiverAblationOptimizationStep : public vtkLiverAblationStep
{
public:
  static vtkLiverAblationOptimizationStep *New();
  vtkTypeRevisionMacro(vtkLiverAblationOptimizationStep,vtkLiverAblationStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

  virtual void RunButtonCallback();
  virtual void GoToNavButtonCallback();

protected:
  vtkLiverAblationOptimizationStep();
  ~vtkLiverAblationOptimizationStep();

  vtkKWFrameWithLabel *RunFrame;

  vtkKWPushButton              *RunButton;
  vtkKWPushButton              *GoToNavButton;


private:
  vtkLiverAblationOptimizationStep(const vtkLiverAblationOptimizationStep&);
  void operator=(const vtkLiverAblationOptimizationStep&);
};

#endif
