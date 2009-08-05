#ifndef __vtkChangeTrackerTypeStep_h
#define __vtkChangeTrackerTypeStep_h

#include "vtkChangeTrackerStep.h"

class vtkKWCheckButton;
class vtkKWFrame;
class vtkKWRadioButtonSetWithLabel;

class VTK_CHANGETRACKER_EXPORT vtkChangeTrackerTypeStep : public vtkChangeTrackerStep
{
public:
  static vtkChangeTrackerTypeStep *New();
  vtkTypeRevisionMacro(vtkChangeTrackerTypeStep,vtkChangeTrackerStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

 // Description:
  // Callback functions for buttons
  // void ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData);
  // void AddGUIObservers();
  // void RemoveGUIObservers();

  virtual void TransitionCallback(); 

  // We call this function in order to remove nodes when going backwards 
  virtual void RemoveResults();

  // AF: necessary to handle global registration completion
  virtual void UpdateGUI();

protected:
  vtkChangeTrackerTypeStep();
  ~vtkChangeTrackerTypeStep();

  static void WizardGUICallback(vtkObject *caller, unsigned long event, void *clientData, void *callData );

  vtkKWFrame*FrameTypeIntensity;
  vtkKWFrame*FrameTypeJacobian;

  vtkKWCheckButton *TypeIntensityCheckButton;
  vtkKWCheckButton *TypeJacobianCheckButton; 

  vtkKWRadioButtonSetWithLabel *RegistrationChoice;
  vtkKWCheckButton *ROIRegistrationChoice;

private:
  vtkChangeTrackerTypeStep(const vtkChangeTrackerTypeStep&);
  void operator=(const vtkChangeTrackerTypeStep&);
};

#endif
