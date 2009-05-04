#ifndef __vtkIGTPlanningCalibrationStep_h
#define __vtkIGTPlanningCalibrationStep_h

#include "vtkIGTPlanningStep.h"

class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;

class VTK_IGT_EXPORT vtkIGTPlanningCalibrationStep : public vtkIGTPlanningStep
{
public:
  static vtkIGTPlanningCalibrationStep *New();
  vtkTypeRevisionMacro(vtkIGTPlanningCalibrationStep,vtkIGTPlanningStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

protected:
  vtkIGTPlanningCalibrationStep();
  ~vtkIGTPlanningCalibrationStep();

  virtual void PopulateCalibrationDataSelector();

  vtkKWFrameWithLabel           *TrackingFrame;
  vtkKWFrameWithLabel           *EndoscopeCameraFrame;
  vtkKWFrameWithLabel           *LoadPrecomputedCalibrationDataFrame;
  vtkKWFrameWithLabel           *CalibrateFrame;

  vtkKWMenuButtonWithLabel      *CalibrationDataMenuButton; 

private:
  vtkIGTPlanningCalibrationStep(const vtkIGTPlanningCalibrationStep&);
  void operator=(const vtkIGTPlanningCalibrationStep&);
};

#endif
