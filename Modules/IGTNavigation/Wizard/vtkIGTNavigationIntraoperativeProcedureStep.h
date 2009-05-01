#ifndef __vtkIGTNavigationIntraoperativeProcedureStep_h
#define __vtkIGTNavigationIntraoperativeProcedureStep_h

#include "vtkIGTNavigationStep.h"

class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkKWRadioButton;
class vtkKWRadioButtonSet;

class VTK_IGT_EXPORT vtkIGTNavigationIntraoperativeProcedureStep : public vtkIGTNavigationStep
{
public:
  static vtkIGTNavigationIntraoperativeProcedureStep *New();
  vtkTypeRevisionMacro(vtkIGTNavigationIntraoperativeProcedureStep,vtkIGTNavigationStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

protected:
  vtkIGTNavigationIntraoperativeProcedureStep();
  ~vtkIGTNavigationIntraoperativeProcedureStep();

  vtkKWFrameWithLabel           *TrackingFrame;
  vtkKWFrameWithLabel           *EndoscopeCameraFrame;

  vtkKWRadioButtonSet           *TrackingRadioButtonSet;


private:
  vtkIGTNavigationIntraoperativeProcedureStep(const vtkIGTNavigationIntraoperativeProcedureStep&);
  void operator=(const vtkIGTNavigationIntraoperativeProcedureStep&);
};

#endif
