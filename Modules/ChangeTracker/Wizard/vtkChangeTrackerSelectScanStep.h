#ifndef __vtkChangeTrackerSelectScanStep_h
#define __vtkChangeTrackerSelectScanStep_h

#include "vtkChangeTrackerStep.h"
#include "vtkCallbackCommand.h"

class vtkSlicerNodeSelectorWidget;

class VTK_CHANGETRACKER_EXPORT vtkChangeTrackerSelectScanStep : public vtkChangeTrackerStep
{
public:
  static vtkChangeTrackerSelectScanStep *New();
  vtkTypeRevisionMacro(vtkChangeTrackerSelectScanStep,vtkChangeTrackerStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

  virtual void AddGUIObservers();
  virtual void RemoveGUIObservers(); 
  void ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData);

protected:
  vtkChangeTrackerSelectScanStep();
  ~vtkChangeTrackerSelectScanStep();

  vtkSlicerNodeSelectorWidget *VolumeMenuButton;

private:
  vtkChangeTrackerSelectScanStep(const vtkChangeTrackerSelectScanStep&);
  void operator=(const vtkChangeTrackerSelectScanStep&);
};

#endif
