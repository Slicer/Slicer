#ifndef __vtkIGTPlanningOptimizationStep_h
#define __vtkIGTPlanningOptimizationStep_h

#include "vtkIGTPlanningStep.h"

class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkKWEntryWithLabel;


class VTK_IGT_EXPORT vtkIGTPlanningOptimizationStep : public vtkIGTPlanningStep
{
public:
  static vtkIGTPlanningOptimizationStep *New();
  vtkTypeRevisionMacro(vtkIGTPlanningOptimizationStep,vtkIGTPlanningStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

protected:
  vtkIGTPlanningOptimizationStep();
  ~vtkIGTPlanningOptimizationStep();

  vtkKWFrameWithLabel *ProbeFrame;
  vtkKWFrameWithLabel *SpacingFrame;
  vtkKWFrameWithLabel *MoreFrame;
  vtkKWFrameWithLabel *RunFrame;

  // Probe info
  vtkKWEntryWithLabel    *ProbeAEntry;
  vtkKWEntryWithLabel    *ProbeBEntry;
  vtkKWEntryWithLabel    *ProbeCEntry;

  // Sample spacing 
  vtkKWEntryWithLabel    *SpacingXEntry;
  vtkKWEntryWithLabel    *SpacingYEntry;
  vtkKWEntryWithLabel    *SpacingZEntry;


private:
  vtkIGTPlanningOptimizationStep(const vtkIGTPlanningOptimizationStep&);
  void operator=(const vtkIGTPlanningOptimizationStep&);
};

#endif
