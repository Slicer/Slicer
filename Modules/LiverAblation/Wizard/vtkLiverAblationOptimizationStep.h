#ifndef __vtkLiverAblationOptimizationStep_h
#define __vtkLiverAblationOptimizationStep_h

#include "vtkLiverAblationStep.h"

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

  virtual void OptimizeButtonCallback();
  virtual void LoadButtonCallback();

protected:
  vtkLiverAblationOptimizationStep();
  ~vtkLiverAblationOptimizationStep();

  vtkKWPushButton        *OptimizeButton;
  vtkKWPushButton        *LoadButton;

private:
  vtkLiverAblationOptimizationStep(const vtkLiverAblationOptimizationStep&);
  void operator=(const vtkLiverAblationOptimizationStep&);
};

#endif
