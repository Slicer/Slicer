#ifndef __vtkProstateNavCalibrationStep_h
#define __vtkProstateNavCalibrationStep_h

#include "vtkProstateNavStep.h"

#include "vtkKWPushButton.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkKWFrame.h"


class VTK_PROSTATENAV_EXPORT vtkProstateNavCalibrationStep : public vtkProstateNavStep
{
public:
  static vtkProstateNavCalibrationStep *New();
  vtkTypeRevisionMacro(vtkProstateNavCalibrationStep,vtkProstateNavStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void ShowUserInterface();
  virtual void ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData);  

protected:
  vtkProstateNavCalibrationStep();
  ~vtkProstateNavCalibrationStep();

  vtkKWFrame *SelectImageFrame;
  vtkKWLoadSaveButtonWithLabel *SelectImageButton;
  vtkKWPushButton *CalibrateButton;

private:
  vtkProstateNavCalibrationStep(const vtkProstateNavCalibrationStep&);
  void operator=(const vtkProstateNavCalibrationStep&);
};

#endif
