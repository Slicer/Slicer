#ifndef __vtkEMSegmentNodeParametersStep_h
#define __vtkEMSegmentNodeParametersStep_h

#include "vtkEMSegmentStep.h"

class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkKWScaleWithEntry;
class vtkKWNotebook;
class vtkKWFrame;
class vtkKWEntryWithLabel;
class vtkKWCheckButtonWithLabel;
class vtkKWCheckButton;
class vtkKWWidget;
class vtkKWLabel;
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

  virtual void NodeParametersInputChannelWeightChangedCallback(vtkIdType sel_vol_id, int row, double value);
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
  virtual void NodeParametersInteractionMatrices2DCallback(vtkIdType sel_vol_id, int value);

  virtual void ExcludeIncompleteEStepCallback(vtkIdType, int state);
  virtual void GenerateBackgroundProbabilityCallback(vtkIdType, int state);

  // Description:
  // Reimplement the superclass's method.
  virtual void Validate();
  virtual void HideUserInterface();

  int ClassOverviewWeightAutomaticRecalculateFlag;
  void ClassWeightChangedCallback(vtkIdType  sel_class_id, double value);
  void ClassWeightAutoChangedCallback(vtkIdType  sel_class_id, int state);

protected:
  vtkEMSegmentNodeParametersStep();
  ~vtkEMSegmentNodeParametersStep();

  vtkKWNotebook                      *NodeParametersNotebook;
  vtkKWScaleWithEntry                *NodeParametersGlobalPriorScale;
  vtkKWScaleWithEntry                *NodeParametersSpatialPriorWeightScale;
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

  vtkKWFrameWithLabel                 *NodeParametersInteractionMatricesFrame;
  vtkKWCheckButtonWithLabel      *NodeParametersInteractionMatrices2DCheckButton;

  vtkKWFrameWithLabel                *NodeParametersPCAFrame;
  vtkKWFrameWithLabel                *NodeParametersRegistrationFrame;
  vtkKWFrameWithLabel                *NodeParametersMiscellaeneousFrame;
  vtkKWCheckButtonWithLabel          *NodeParametersExcludeIncompleteEStepCheckButton;
  vtkKWCheckButtonWithLabel          *NodeParametersGenerateBackgroundProbabilityCheckButton;
  vtkKWFrameWithLabel                *NodeParametersInhomogeneityFrame;

  vtkKWFrameWithLabel                *NodeParametersInputChannelWeightFrame;
  vtkKWFrameWithLabel                *NodeParametersClassOverviewWeightGlobalFrame;

  vtkKWLabel                         *NodeParametersLabelTabBasic;
  vtkKWLabel                         *NodeParametersLabelTabStoppingConditions;
  vtkKWLabel                         *NodeParametersLabelTabPrint;
  vtkKWLabel                         *NodeParametersLabelTabAdvanced;

  //BTX
  vtkstd::vector<vtkKWScaleWithEntry*>  NodeParametersInputChannelWeight;
  vtkstd::vector<vtkKWScaleWithEntry*>  NodeParametersClassOverviewWeightEntry;
  vtkstd::vector<vtkKWFrame*>           NodeParametersClassOverviewWeightFrame;
  vtkstd::vector<vtkKWCheckButton*>     NodeParametersClassOverviewWeightAuto;

 //ETX

  void DefineInputChannelWeightOverviewWindow(int enabled);

  void DefineClassOverviewWeightWindow(vtkIdType sel_tree_class_id, int enabled); 

private:
  vtkEMSegmentNodeParametersStep(const vtkEMSegmentNodeParametersStep&);
  void operator=(const vtkEMSegmentNodeParametersStep&);
};

#endif
