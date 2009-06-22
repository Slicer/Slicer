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

class vtkKWPushButton;

// NEW
class vtkKWHistogram;

class vtkKWMultiColumnList;

class vtkKWColorTransferFunctionEditor;

class vtkKWEntryWithLabel;

class vtkColorTransferFunction;

// END NEW

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
  
    // Description:
  // Observers 
  /*virtual void AddUpdatePriorGUIEvents();
  virtual void RemoveUpdatePriorGUIEvents();
  virtual void ProcessUpdatePriorGUIEvents(
    vtkObject *caller, unsigned long event, void *callData);
    */
    
  //NEW
  
  virtual void IntensityDistributionTargetSelectionChangedCallback(
    vtkIdType VolId);

    // Description:
  // Observers
  virtual void AddPointMovingGUIEvents();
  virtual void RemovePointMovingGUIEvents();
  virtual void ProcessPointMovingGUIEvents(
  vtkObject *caller, unsigned long event, void *callData);
  
      // Description:
  // Observers
  virtual void AddPointAddGUIEvents();
  virtual void RemovePointAddGUIEvents();
  virtual void ProcessPointAddGUIEvents(
  vtkObject *caller, unsigned long event, void *callData);

        // Description:
  // Observers
  virtual void AddTestButtonGUIEvents();
  virtual void RemoveTestButtonGUIEvents();
  virtual void ProcessTestButtonGUIEvents(
  vtkObject *caller, unsigned long event, void *callData);

  virtual void test();
    
  //double* position;
  double size;  
  int nbOfLeaf;
  int depth;
  //const char* nameOfLeaf[] = {"Nicolas","je","suis"};
  double classSize[400];
  vtkIdType leafID[200];
  
  vtkIdType classPercentOrder[200][200];
  vtkIdType classPercentOrderCP[200][200];
  double class_weight[200];
  double class_size[400];
  
  const char* node_name[200];
  
  vtkIdType correspondanceArray[2][200];
  
  virtual void GetNumberOfLeaf(const char*, vtkIdType);
  
  virtual void GetParentPercent(int, vtkIdType);
  virtual void GetPercent(int, vtkIdType);
  virtual double GetWeight(int);
  
  //END NEW  

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
  
  vtkKWFrameWithLabel                *UpdateFrame;
  vtkKWPushButton                    *UpdatePrior;
  
  //NEW
  
  virtual void PopulateIntensityDistributionTargetVolumeSelector();
  virtual void PopulateClassAndNodeList();
  
  vtkKWMenuButtonWithLabel  *IntensityDistributionHistogramButton;
  
  vtkKWHistogram            *IntensityDistributionHistogramHistogram;
  vtkKWColorTransferFunctionEditor *IntensityDistributionHistogramHistogramVisu;
  vtkColorTransferFunction  *IntensityDistributionHistogramHistogramFunc;
  
  vtkKWEntryWithLabel       *NbOfClassesEntryLabel;
  
  vtkKWMultiColumnList      *ClassAndNodeList;
  
  vtkKWPushButton           *TestButton;
  
  //END NEW

private:
  vtkEMSegmentNodeParametersStep(const vtkEMSegmentNodeParametersStep&);
  void operator=(const vtkEMSegmentNodeParametersStep&);
};

#endif
