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

  virtual void SaveAndRunButtonCallback();


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

  vtkKWEntryWithLabel    *AngularResolutionEntry;
  vtkKWEntryWithLabel    *NumberOfAblationsEntry;
  vtkKWEntryWithLabel    *NumberOfTrajectoriesEntry;
  vtkKWEntryWithLabel    *NumberOfPuncturesEntry;
  vtkKWEntryWithLabel    *TumorMarginEntry;

  vtkKWLoadSaveButtonWithLabel *DirectoryButton; 
  vtkKWEntryWithLabel          *FileNameEntry;
  vtkKWPushButton              *SaveAndRunButton;

  bool CheckInputErrors();
 

private:
  vtkIGTPlanningOptimizationStep(const vtkIGTPlanningOptimizationStep&);
  void operator=(const vtkIGTPlanningOptimizationStep&);
};

#endif
