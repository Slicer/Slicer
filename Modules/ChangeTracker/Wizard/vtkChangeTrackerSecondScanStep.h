#ifndef __vtkChangeTrackerSecondScanStep_h
#define __vtkChangeTrackerSecondScanStep_h

#include "vtkChangeTrackerSelectScanStep.h"

class VTK_CHANGETRACKER_EXPORT vtkChangeTrackerSecondScanStep : public vtkChangeTrackerSelectScanStep
{
public:
  static vtkChangeTrackerSecondScanStep *New();
  vtkTypeRevisionMacro(vtkChangeTrackerSecondScanStep,vtkChangeTrackerStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

  // Description:
  // Callbacks.
  virtual void TransitionCallback(int Flag);
  void TransitionCallback() {this->TransitionCallback(1);}
  void UpdateGUI();
  void UpdateMRML();

  void ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData) {
    this->vtkChangeTrackerSelectScanStep::ProcessGUIEvents(caller, event, callData); }

protected:
  vtkChangeTrackerSecondScanStep();
  ~vtkChangeTrackerSecondScanStep();

  static void WizardGUICallback(vtkObject *caller, unsigned long eid, void *clientData, void *callData );
  void ProcessGUIEvents(vtkObject *caller, void *callData);

private:
  vtkChangeTrackerSecondScanStep(const vtkChangeTrackerSecondScanStep&);
  void RemoveAnalysisOutput();
  void operator=(const vtkChangeTrackerSecondScanStep&);
};

#endif
