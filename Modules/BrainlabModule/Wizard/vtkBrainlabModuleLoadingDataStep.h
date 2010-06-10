#ifndef __vtkBrainlabModuleLoadingDataStep_h
#define __vtkBrainlabModuleLoadingDataStep_h

#include "vtkBrainlabModuleStep.h"

class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkKWPushButton;
class vtkKWTopLevel;

class VTK_BRAINLABMODULE_EXPORT vtkBrainlabModuleLoadingDataStep : public vtkBrainlabModuleStep
{
public:
  static vtkBrainlabModuleLoadingDataStep *New();
  vtkTypeRevisionMacro(vtkBrainlabModuleLoadingDataStep,vtkBrainlabModuleStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

  // Description:
  // Callbacks. Internal, do not use.
//  virtual void DefaultSceneButtonCallback();
  virtual void UserSceneButtonCallback();


protected:
  vtkBrainlabModuleLoadingDataStep();
  ~vtkBrainlabModuleLoadingDataStep();

  void ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData);

//  vtkKWPushButton            *DefaultSceneButton; 
  vtkKWPushButton            *UserSceneButton; 


private:
  vtkBrainlabModuleLoadingDataStep(const vtkBrainlabModuleLoadingDataStep&);
  void operator=(const vtkBrainlabModuleLoadingDataStep&);
};

#endif
