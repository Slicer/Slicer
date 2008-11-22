#ifndef __vtkChangeTrackerAnalysisStep_h
#define __vtkChangeTrackerAnalysisStep_h

#include "vtkChangeTrackerStep.h"

class vtkKWCheckButton;
class vtkKWLabel;
class vtkKWFrameWithLabel; 
class vtkKWPushButton;
class vtkKWFrame;
class vtkKWLoadSaveButtonWithLabel;

class VTK_CHANGETRACKER_EXPORT vtkChangeTrackerAnalysisStep : public vtkChangeTrackerStep
{
public:
  static vtkChangeTrackerAnalysisStep *New();
  vtkTypeRevisionMacro(vtkChangeTrackerAnalysisStep,vtkChangeTrackerStep);
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
  vtkChangeTrackerAnalysisStep();
  ~vtkChangeTrackerAnalysisStep();

  vtkKWFrame        *SensitivityRadio;
  vtkKWCheckButton  *SensitivityLow;
  vtkKWCheckButton  *SensitivityMedium;
  vtkKWCheckButton  *SensitivityHigh;

  vtkKWFrame                *FrameIntensity;
  vtkKWFrame                *FrameIntensityCol1;
  vtkKWFrame                *FrameIntensityCol2;
  vtkKWFrame                *FrameIntensityCol3;

  vtkKWLabel                *IntensityLabel;
  vtkKWLabel                *IntensityResultVolume;
  vtkKWLabel                *IntensityResultVoxel;

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
  vtkChangeTrackerAnalysisStep(const vtkChangeTrackerAnalysisStep&);
  void operator=(const vtkChangeTrackerAnalysisStep&);
};

#endif


