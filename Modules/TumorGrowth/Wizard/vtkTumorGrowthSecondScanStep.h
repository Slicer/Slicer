#ifndef __vtkTumorGrowthSecondScanStep_h
#define __vtkTumorGrowthSecondScanStep_h

#include "vtkTumorGrowthSelectScanStep.h"

class VTK_TUMORGROWTH_EXPORT vtkTumorGrowthSecondScanStep : public vtkTumorGrowthSelectScanStep
{
public:
  static vtkTumorGrowthSecondScanStep *New();
  vtkTypeRevisionMacro(vtkTumorGrowthSecondScanStep,vtkTumorGrowthStep);
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
    this->vtkTumorGrowthSelectScanStep::ProcessGUIEvents(caller, event, callData); }

protected:
  vtkTumorGrowthSecondScanStep();
  ~vtkTumorGrowthSecondScanStep();

  static void WizardGUICallback(vtkObject *caller, unsigned long eid, void *clientData, void *callData );
  void ProcessGUIEvents(vtkObject *caller, void *callData);

private:
  vtkTumorGrowthSecondScanStep(const vtkTumorGrowthSecondScanStep&);
  void RemoveAnalysisOutput();
  void operator=(const vtkTumorGrowthSecondScanStep&);
};

#endif
