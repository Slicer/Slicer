#ifndef __vtkEMSegmentNodeParametersStep_h
#define __vtkEMSegmentNodeParametersStep_h

#include "vtkEMSegmentStep.h"

class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkKWScaleWithEntry;
class vtkKWNotebook;
class vtkKWMultiColumnListWithScrollbarsWithLabel;
class vtkKWFrame;
class vtkKWEntryWithLabel;
class vtkKWCheckButtonWithLabel;

class VTK_EMSEGMENT_EXPORT vtkEMSegmentNodeParametersStep : public vtkEMSegmentStep
{
public:
  static vtkEMSegmentNodeParametersStep *New();
  vtkTypeRevisionMacro(vtkEMSegmentNodeParametersStep,vtkEMSegmentStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

  // Description:
  // Callbacks.
  virtual void DisplaySelectedNodeParametersCallback();
  virtual void NodeParametersGlobalPriorChangedCallback(
    vtkIdType, double value);
  virtual void NodeParametersSpatialPriorWeightChangedCallback(
    vtkIdType, double value);
  virtual void RightClickOnInputChannelWeightsListCallback(
    int row, int col, int x, int y);
  virtual void NodeParametersInputChannelWeightChangedCallback(
    vtkIdType, int row, int col, const char *value);
  virtual void NodeParametersAlphaChangedCallback(vtkIdType, double value);
  virtual void StoppingConditionsEMCallback(vtkIdType, int type);
  virtual void StoppingConditionsEMIterationsCallback(
    vtkIdType, const char *value);
  virtual void StoppingConditionsEMValueCallback(
    vtkIdType, const char *value);
  virtual void StoppingConditionsMFACallback(vtkIdType, int type);
  virtual void StoppingConditionsMFAIterationsCallback(
    vtkIdType, const char *value);
  virtual void StoppingConditionsMFAValueCallback(
    vtkIdType, const char *value);
  virtual void StoppingConditionsBiasIterationsCallback(
    vtkIdType, const char *value);
  virtual void NodeParametersPrintWeightCallback(vtkIdType, int value);
  virtual void NodeParametersPrintQualityCallback(vtkIdType, int value);
  virtual void NodeParametersPrintFrequencyChangedCallback(
    vtkIdType, int value);
  virtual void NodeParametersPrintBiasCallback(vtkIdType, int value);
  virtual void NodeParametersPrintLabelMapCallback(vtkIdType, int value);
  virtual void NodeParametersPrintEMLabelMapCallback(vtkIdType, int value);
  virtual void NodeParametersPrintEMWeightsCallback(vtkIdType, int value);
  virtual void NodeParametersPrintMFALabelMapCallback(vtkIdType, int value);
  virtual void NodeParametersPrintMFAWeightsCallback(vtkIdType, int value);
  virtual void ExcludeIncompleteEStepCallback(vtkIdType, int state);
  virtual void GenerateBackgroundProbabilityCallback(vtkIdType, int state);

  // Description:
  // Reimplement the superclass's method.
  virtual void Validate();

protected:
  vtkEMSegmentNodeParametersStep();
  ~vtkEMSegmentNodeParametersStep();

  vtkKWNotebook                      *NodeParametersNotebook;
  vtkKWScaleWithEntry                *NodeParametersGlobalPriorScale;
  vtkKWScaleWithEntry                *NodeParametersSpatialPriorWeightScale;
  vtkKWMultiColumnListWithScrollbarsWithLabel 
                                     *NodeParametersInputChannelWeightsList;
  vtkKWScaleWithEntry                *NodeParametersAlphaScale;
  vtkKWMenuButtonWithLabel           *StoppingConditionsEMMenuButton;
  vtkKWEntryWithLabel                *StoppingConditionsEMIterationsEntry;
  vtkKWEntryWithLabel                *StoppingConditionsEMValueEntry;
  vtkKWMenuButtonWithLabel           *StoppingConditionsMFAMenuButton;
  vtkKWEntryWithLabel                *StoppingConditionsMFAIterationsEntry;
  vtkKWEntryWithLabel                *StoppingConditionsMFAValueEntry;
  vtkKWEntryWithLabel                *StoppingConditionsBiasIterationsEntry;
  vtkKWFrame                         *PrintBasicFrame;
  vtkKWCheckButtonWithLabel          *NodeParametersPrintWeightCheckButton;
  vtkKWCheckButtonWithLabel          *NodeParametersPrintQualityCheckButton;
  vtkKWScaleWithEntry                *NodeParametersPrintFrequencyScale;
  vtkKWCheckButtonWithLabel          *NodeParametersPrintBiasCheckButton;
  vtkKWCheckButtonWithLabel          *NodeParametersPrintLabelMapCheckButton;
  vtkKWFrameWithLabel                *PrintConvergenceFrame;
  vtkKWCheckButtonWithLabel          *NodeParametersPrintEMLabelMapConvergenceCheckButton;
  vtkKWCheckButtonWithLabel          *NodeParametersPrintEMWeightsConvergenceCheckButton;
  vtkKWCheckButtonWithLabel          *NodeParametersPrintMFALabelMapConvergenceCheckButton;
  vtkKWCheckButtonWithLabel          *NodeParametersPrintMFAWeightsConvergenceCheckButton;

  vtkKWFrameWithLabel                *NodeParametersInteractionMatricesFrame;
  vtkKWFrameWithLabel                *NodeParametersPCAFrame;
  vtkKWFrameWithLabel                *NodeParametersRegistrationFrame;
  vtkKWFrameWithLabel                *NodeParametersMiscellaeneousFrame;
  vtkKWCheckButtonWithLabel          *NodeParametersExcludeIncompleteEStepCheckButton;
  vtkKWCheckButtonWithLabel          *NodeParametersGenerateBackgroundProbabilityCheckButton;
  vtkKWFrameWithLabel                *NodeParametersInhomogeneityFrame;

private:
  vtkEMSegmentNodeParametersStep(const vtkEMSegmentNodeParametersStep&);
  void operator=(const vtkEMSegmentNodeParametersStep&);
};

#endif
