#ifndef __vtkChangeTrackerSegmentationStep_h
#define __vtkChangeTrackerSegmentationStep_h

#include "vtkChangeTrackerStep.h"

class vtkImageThreshold;
class vtkMRMLScalarVolumeNode;   
class vtkKWFrame;
class vtkKWLabel;
class vtkKWRange;
class vtkSlicerNodeSelectorWidget;

class VTK_CHANGETRACKER_EXPORT vtkChangeTrackerSegmentationStep : public vtkChangeTrackerStep
{
public:
  static vtkChangeTrackerSegmentationStep *New();
  vtkTypeRevisionMacro(vtkChangeTrackerSegmentationStep,vtkChangeTrackerStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

  // Description:
  // Callbacks.
  virtual void ThresholdRangeChangedCallback(double min, double max);
  virtual void TransitionCallback(); 
  // We call this function in order to remove nodes when going backwards 
  virtual void RemoveResults();

  vtkGetObjectMacro(PreSegment,vtkImageThreshold);

  virtual void AddGUIObservers();
  virtual void RemoveGUIObservers();
  void ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData);

protected:
  vtkChangeTrackerSegmentationStep();
  ~vtkChangeTrackerSegmentationStep();

  vtkKWFrame *ThresholdFrame;
  vtkKWRange *ThresholdRange;
  vtkKWLabel *ThresholdLabel;

  vtkSlicerNodeSelectorWidget *Scan1_SegmSelector;
//  vtkSlicerNodeSelectorWidget *Scan2_SegmSelector;

  void ShowSegmentedVolume(vtkMRMLScalarVolumeNode*);
  static void WizardGUICallback(vtkObject *caller, unsigned long event, void *clientData, void *callData );

private:
  vtkChangeTrackerSegmentationStep(const vtkChangeTrackerSegmentationStep&);
  void operator=(const vtkChangeTrackerSegmentationStep&);

  void PreSegmentScan1Remove();
  void PreSegmentScan1Define();

  void SegmentScan1Remove();
  int SegmentScan1Define();

  vtkImageThreshold *PreSegment;
  vtkMRMLScalarVolumeNode *PreSegmentNode; 
  vtkMRMLScalarVolumeNode *SegmentNode; 
};

#endif
