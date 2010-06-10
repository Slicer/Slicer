#ifndef __vtkBrainlabModuleNavigationStep_h
#define __vtkBrainlabModuleNavigationStep_h

#include "vtkBrainlabModuleStep.h"

class vtkKWFrame;
class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkKWPushButton;
class vtkKWCheckButton;
class vtkSlicerNodeSelectorWidget;
class vtkMRMLSliceNode;
class vtkMatrix4x4;

class VTK_BRAINLABMODULE_EXPORT vtkBrainlabModuleNavigationStep : public vtkBrainlabModuleStep
{
public:
  static vtkBrainlabModuleNavigationStep *New();
  vtkTypeRevisionMacro(vtkBrainlabModuleNavigationStep,vtkBrainlabModuleStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

  // Description:
  // Callbacks. Internal, do not use.
  virtual void FiducialSeedingButtonCallback();
  virtual void MRMLTreeButtonCallback();

  void Timer(int yes);
  void ProcessTimerEvents();
  void ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData);

protected:
  vtkBrainlabModuleNavigationStep();
  ~vtkBrainlabModuleNavigationStep();

  void CheckSliceNode();
  void UpdateSliceNode(int sliceNodeNumber, vtkMatrix4x4* transform);

  vtkKWPushButton             *FiducialSeedingButton; 
  vtkKWFrameWithLabel         *FiducialSeedingFrame;

  vtkKWPushButton             *MRMLTreeButton; 
  vtkKWFrameWithLabel         *MRMLTreeFrame;

  vtkKWFrameWithLabel         *SliceControlFrame;
  vtkKWFrame                  *ButtonFrame;
  vtkKWCheckButton            *AxialCheckButton;
  vtkKWCheckButton            *SagittalCheckButton;
  vtkKWCheckButton            *CoronalCheckButton;

  vtkSlicerNodeSelectorWidget *TrackingSourceSelectorWidget;

  int TimerFlag;
  int TimerInterval;
  vtkMRMLSliceNode *SliceNode[3];
  bool EnableOblique;


private:
  vtkBrainlabModuleNavigationStep(const vtkBrainlabModuleNavigationStep&);
  void operator=(const vtkBrainlabModuleNavigationStep&);
};

#endif
