#ifndef __vtkTumorGrowthSegmentationStep_h
#define __vtkTumorGrowthSegmentationStep_h

#include "vtkTumorGrowthStep.h"

class vtkImageThreshold;
class vtkMRMLScalarVolumeNode;   
class vtkKWFrame;
class vtkKWLabel;
class vtkKWRange;

class VTK_TUMORGROWTH_EXPORT vtkTumorGrowthSegmentationStep : public vtkTumorGrowthStep
{
public:
  static vtkTumorGrowthSegmentationStep *New();
  vtkTypeRevisionMacro(vtkTumorGrowthSegmentationStep,vtkTumorGrowthStep);
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


protected:
  vtkTumorGrowthSegmentationStep();
  ~vtkTumorGrowthSegmentationStep();

  vtkKWFrame *ThresholdFrame;
  vtkKWRange *ThresholdRange;
  vtkKWLabel *ThresholdLabel;

private:
  vtkTumorGrowthSegmentationStep(const vtkTumorGrowthSegmentationStep&);
  void operator=(const vtkTumorGrowthSegmentationStep&);

  void PreSegmentScan1Remove();
  void PreSegmentScan1Define();

  void SegmentScan1Remove();
  int SegmentScan1Define();

  vtkImageThreshold *PreSegment;
  vtkMRMLScalarVolumeNode *PreSegmentNode; 
  vtkMRMLScalarVolumeNode *SegmentNode; 
};

#endif
