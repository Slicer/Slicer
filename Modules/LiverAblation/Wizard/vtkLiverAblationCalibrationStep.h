#ifndef __vtkLiverAblationCalibrationStep_h
#define __vtkLiverAblationCalibrationStep_h

#include "vtkLiverAblationStep.h"

class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;

class VTK_LIVERABLATION_EXPORT vtkLiverAblationCalibrationStep : public vtkLiverAblationStep
{
public:
  static vtkLiverAblationCalibrationStep *New();
  vtkTypeRevisionMacro(vtkLiverAblationCalibrationStep,vtkLiverAblationStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

protected:
  vtkLiverAblationCalibrationStep();
  ~vtkLiverAblationCalibrationStep();

  virtual void PopulateCalibrationDataSelector();

  vtkKWFrameWithLabel           *TrackingFrame;
  vtkKWFrameWithLabel           *EndoscopeCameraFrame;
  vtkKWFrameWithLabel           *LoadPrecomputedCalibrationDataFrame;
  vtkKWFrameWithLabel           *CalibrateFrame;

  vtkKWMenuButtonWithLabel      *CalibrationDataMenuButton; 

private:
  vtkLiverAblationCalibrationStep(const vtkLiverAblationCalibrationStep&);
  void operator=(const vtkLiverAblationCalibrationStep&);
};

#endif
