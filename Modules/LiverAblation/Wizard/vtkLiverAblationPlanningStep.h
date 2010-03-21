#ifndef __vtkLiverAblationPlanningStep_h
#define __vtkLiverAblationPlanningStep_h

#include "vtkLiverAblationStep.h"

class vtkKWMenuButtonWithLabel;
class vtkKWPushButton;

class VTK_LIVERABLATION_EXPORT vtkLiverAblationPlanningStep : public vtkLiverAblationStep
{
public:
  static vtkLiverAblationPlanningStep *New();
  vtkTypeRevisionMacro(vtkLiverAblationPlanningStep,vtkLiverAblationStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

  // Description:
  // Callbacks. Internal, do not use.
  virtual void FiducialButtonCallback();
  virtual void EditorButtonCallback();
  virtual void SegmentationButtonCallback();



protected:
  vtkLiverAblationPlanningStep();
  ~vtkLiverAblationPlanningStep();

  void ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData);

  vtkKWPushButton *FiducialButton;
  vtkKWPushButton *EditorButton;
  vtkKWPushButton *SegmentationButton;

  void RaiseModule(const char *moduleName);


private:
  vtkLiverAblationPlanningStep(const vtkLiverAblationPlanningStep&);
  void operator=(const vtkLiverAblationPlanningStep&);
};

#endif
