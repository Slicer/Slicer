#ifndef __vtkChangeTrackerFirstScanStep_h
#define __vtkChangeTrackerFirstScanStep_h

#include "vtkChangeTrackerSelectScanStep.h"

class VTK_CHANGETRACKER_EXPORT vtkChangeTrackerFirstScanStep : public vtkChangeTrackerSelectScanStep
{
public:
  static vtkChangeTrackerFirstScanStep *New();
  vtkTypeRevisionMacro(vtkChangeTrackerFirstScanStep,vtkChangeTrackerStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

  // Description:
  // Callbacks. - Flag - show error message
  virtual void TransitionCallback(int Flag);
  void TransitionCallback() {this->TransitionCallback(1);}

  virtual void UpdateMRML();
  virtual void UpdateGUI();

  virtual void AddGUIObservers();
  virtual void RemoveGUIObservers(); 

  void ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData); 

  vtkSlicerNodeSelectorWidget* GetSecondVolumeMenuButton() {return this->SecondVolumeMenuButton;} 
  vtkSlicerNodeSelectorWidget* GetFirstVolumeMenuButton() {return this->VolumeMenuButton;} 

protected:
  vtkChangeTrackerFirstScanStep();
  ~vtkChangeTrackerFirstScanStep();

  vtkSlicerNodeSelectorWidget *SecondVolumeMenuButton;

  static void WizardGUICallback(vtkObject *caller, unsigned long eid, void *clientData, void *callData );
  void ProcessGUIEvents(vtkObject *caller, void *callData);

private:
  vtkChangeTrackerFirstScanStep(const vtkChangeTrackerFirstScanStep&);
  void operator=(const vtkChangeTrackerFirstScanStep&);
};

#endif
