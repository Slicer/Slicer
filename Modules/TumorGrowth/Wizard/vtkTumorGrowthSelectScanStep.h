#ifndef __vtkTumorGrowthSelectScanStep_h
#define __vtkTumorGrowthSelectScanStep_h

#include "vtkTumorGrowthStep.h"
#include "vtkCallbackCommand.h"

class vtkSlicerNodeSelectorWidget;

class VTK_TUMORGROWTH_EXPORT vtkTumorGrowthSelectScanStep : public vtkTumorGrowthStep
{
public:
  static vtkTumorGrowthSelectScanStep *New();
  vtkTypeRevisionMacro(vtkTumorGrowthSelectScanStep,vtkTumorGrowthStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

  virtual void AddGUIObservers();
  virtual void RemoveGUIObservers(); 
  void ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData);

protected:
  vtkTumorGrowthSelectScanStep();
  ~vtkTumorGrowthSelectScanStep();

  vtkSlicerNodeSelectorWidget *VolumeMenuButton;

private:
  vtkTumorGrowthSelectScanStep(const vtkTumorGrowthSelectScanStep&);
  void operator=(const vtkTumorGrowthSelectScanStep&);
};

#endif
