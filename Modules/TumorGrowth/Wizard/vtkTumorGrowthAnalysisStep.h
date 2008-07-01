#ifndef __vtkTumorGrowthAnalysisStep_h
#define __vtkTumorGrowthAnalysisStep_h

#include "vtkTumorGrowthStep.h"

class vtkKWCheckButton;
class vtkKWLabel;
class vtkKWFrameWithLabel; 
class vtkKWPushButton;
class vtkKWFrame;
class vtkKWLoadSaveButtonWithLabel;

class VTK_TUMORGROWTH_EXPORT vtkTumorGrowthAnalysisStep : public vtkTumorGrowthStep
{
public:
  static vtkTumorGrowthAnalysisStep *New();
  vtkTypeRevisionMacro(vtkTumorGrowthAnalysisStep,vtkTumorGrowthStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

  // Description:
  // Callbacks.
  virtual void SensitivityChangedCallback(int value);

  void ResetPipelineCallback();
  // Kilian Work here tomorrow 
  virtual void TransitionCallback() { }; 
  // We call this function in order to remove nodes when going backwards 
  virtual void RemoveResults(); 

  void ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData);
  void RemoveGUIObservers();
  void AddGUIObservers(); 

  void SelectDirectoryCallback();
  void TakeScreenshot(); 

protected:
  vtkTumorGrowthAnalysisStep();
  ~vtkTumorGrowthAnalysisStep();

  vtkKWFrame        *SensitivityRadio;
  vtkKWCheckButton  *SensitivityLow;
  vtkKWCheckButton  *SensitivityMedium;
  vtkKWCheckButton  *SensitivityHigh;

  vtkKWLabel                 *GrowthLabel;

  vtkKWFrameWithLabel       *FrameDeformable;
  vtkKWFrame                *FrameDeformableCol1;
  vtkKWFrame                *FrameDeformableCol2;
  vtkKWLabel                *DeformableTextLabel;
  vtkKWLabel                *DeformableMeassureLabel;

  vtkKWFrameWithLabel       *FrameButtons;
  vtkKWFrame                *FrameButtonsFunctions;
  vtkKWLoadSaveButtonWithLabel *ButtonsWorkingDir;
  vtkKWPushButton           *ButtonsAnalysis;
  vtkKWPushButton           *ButtonsSnapshot;
  vtkKWPushButton           *ButtonsSave;
  vtkKWLabel                *ButtonsInfo;

  static void WizardGUICallback(vtkObject *caller, unsigned long event, void *clientData, void *callData );

  int SnapshotCount; 

private:
  vtkTumorGrowthAnalysisStep(const vtkTumorGrowthAnalysisStep&);
  void operator=(const vtkTumorGrowthAnalysisStep&);
};

#endif


