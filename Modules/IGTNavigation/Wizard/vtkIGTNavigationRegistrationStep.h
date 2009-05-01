#ifndef __vtkIGTNavigationRegistrationStep_h
#define __vtkIGTNavigationRegistrationStep_h

#include "vtkIGTNavigationStep.h"

class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkKWPushButton;
class vtkKWEntry;

class VTK_IGT_EXPORT vtkIGTNavigationRegistrationStep : public vtkIGTNavigationStep
{
public:
  static vtkIGTNavigationRegistrationStep *New();
  vtkTypeRevisionMacro(vtkIGTNavigationRegistrationStep,vtkIGTNavigationStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Callback: Image fiducials
  virtual void FiducialPointSelectorCallback1(vtkIdType fiducialId);
  virtual void FiducialPointSelectorCallback2(vtkIdType fiducialId);
  virtual void FiducialPointSelectorCallback3(vtkIdType fiducialId);
  virtual void FiducialPointSelectorCallback4(vtkIdType fiducialId);

  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

  virtual void ComputeRegistration();

protected:
  vtkIGTNavigationRegistrationStep();
  ~vtkIGTNavigationRegistrationStep();

  virtual void PopulateFiducialPointsSelector( vtkKWMenuButtonWithLabel* menuLabelButton
                                               , const char* callback);

  bool GetFiducialXYZPosition ( float * fiducialPosition, vtkIdType fiducialId );

  vtkKWFrameWithLabel           *ImageFiducialFrame;
  vtkKWMenuButtonWithLabel      *ImageFiducialMenuButton1; 
  vtkKWMenuButtonWithLabel      *ImageFiducialMenuButton2; 
  vtkKWMenuButtonWithLabel      *ImageFiducialMenuButton3; 
  vtkKWMenuButtonWithLabel      *ImageFiducialMenuButton4; 


  vtkKWEntry                    *ImageFiducialTextBox1;
  vtkKWEntry                    *ImageFiducialTextBox2;
  vtkKWEntry                    *ImageFiducialTextBox3;
  vtkKWEntry                    *ImageFiducialTextBox4;

  vtkKWFrameWithLabel           *TrackerFiducialFrame;
  vtkKWPushButton               *TrackerFiducialPushButton1;
  vtkKWPushButton               *TrackerFiducialPushButton2;
  vtkKWPushButton               *TrackerFiducialPushButton3;
  vtkKWPushButton               *TrackerFiducialPushButton4;

  vtkKWEntry                    *TrackerFiducialTextBox1;
  vtkKWEntry                    *TrackerFiducialTextBox2;
  vtkKWEntry                    *TrackerFiducialTextBox3;
  vtkKWEntry                    *TrackerFiducialTextBox4;

  vtkKWPushButton               *RunRegistrationPushButton;

private:
  vtkIGTNavigationRegistrationStep(const vtkIGTNavigationRegistrationStep&);
  void operator=(const vtkIGTNavigationRegistrationStep&);
};

#endif
