#ifndef __vtkIGTNavigationCalibrationStep_h
#define __vtkIGTNavigationCalibrationStep_h

#include "vtkIGTNavigationStep.h"

class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;

class VTK_IGT_EXPORT vtkIGTNavigationCalibrationStep : public vtkIGTNavigationStep
{
public:
  static vtkIGTNavigationCalibrationStep *New();
  vtkTypeRevisionMacro(vtkIGTNavigationCalibrationStep,vtkIGTNavigationStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

protected:
  vtkIGTNavigationCalibrationStep();
  ~vtkIGTNavigationCalibrationStep();

  virtual void PopulateCalibrationDataSelector();

  vtkKWFrameWithLabel           *TrackingFrame;
  vtkKWFrameWithLabel           *EndoscopeCameraFrame;
  vtkKWFrameWithLabel           *LoadPrecomputedCalibrationDataFrame;
  vtkKWFrameWithLabel           *CalibrateFrame;

  vtkKWMenuButtonWithLabel      *CalibrationDataMenuButton; 

private:
  vtkIGTNavigationCalibrationStep(const vtkIGTNavigationCalibrationStep&);
  void operator=(const vtkIGTNavigationCalibrationStep&);
};

#endif
