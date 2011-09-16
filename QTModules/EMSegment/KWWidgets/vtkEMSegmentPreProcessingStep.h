#ifndef __vtkEMSegmentPreProcessingStep_h
#define __vtkEMSegmentPreProcessingStep_h

#include "vtkEMSegmentStep.h"
class vtkEMSegmentKWDynamicFrame;

class VTK_EMSEGMENT_EXPORT vtkEMSegmentPreProcessingStep : public vtkEMSegmentStep
{
public:
  static vtkEMSegmentPreProcessingStep *New();
  vtkTypeRevisionMacro(vtkEMSegmentPreProcessingStep,vtkEMSegmentStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();
  void Validate();
  
  vtkSetMacro(askQuestionsBeforeRunningPreprocessingFlag,int);
  vtkBooleanMacro(askQuestionsBeforeRunningPreprocessingFlag,int);

  vtkGetObjectMacro(CheckListFrame,vtkEMSegmentKWDynamicFrame);

protected:
  vtkEMSegmentPreProcessingStep();
  ~vtkEMSegmentPreProcessingStep();

private:
  vtkEMSegmentPreProcessingStep(const vtkEMSegmentPreProcessingStep&);
  void operator=(const vtkEMSegmentPreProcessingStep&);
  int askQuestionsBeforeRunningPreprocessingFlag;
  vtkEMSegmentKWDynamicFrame* CheckListFrame;

};

#endif
