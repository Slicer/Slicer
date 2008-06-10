#ifndef __vtkTumorGrowthFirstScanStep_h
#define __vtkTumorGrowthFirstScanStep_h

#include "vtkTumorGrowthSelectScanStep.h"

class VTK_TUMORGROWTH_EXPORT vtkTumorGrowthFirstScanStep : public vtkTumorGrowthSelectScanStep
{
public:
  static vtkTumorGrowthFirstScanStep *New();
  vtkTypeRevisionMacro(vtkTumorGrowthFirstScanStep,vtkTumorGrowthStep);
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
  vtkTumorGrowthFirstScanStep();
  ~vtkTumorGrowthFirstScanStep();

  vtkSlicerNodeSelectorWidget *SecondVolumeMenuButton;

  static void WizardGUICallback(vtkObject *caller, unsigned long eid, void *clientData, void *callData );
  void ProcessGUIEvents(vtkObject *caller, void *callData);

private:
  vtkTumorGrowthFirstScanStep(const vtkTumorGrowthFirstScanStep&);
  void operator=(const vtkTumorGrowthFirstScanStep&);
};

#endif
