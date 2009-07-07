/*=auto==============================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All
  Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkEMSegmentNodeParametersStep.cxx,v$
  Date:      $Date: 2009/06/23 10:30:00$
  Version:   $Revision: 0.0$
  Author:    $Nicolas Rannou (BWH), Sylvain Jaume (MIT)$

==============================================================auto=*/

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

class vtkKWHistogram;
class vtkKWMultiColumnList;
class vtkKWColorTransferFunctionEditor;
class vtkKWEntryWithLabel;
class vtkColorTransferFunction;

class VTK_EMSEGMENT_EXPORT vtkEMSegmentNodeParametersStep :
  public vtkEMSegmentStep
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
  virtual void NodeParametersGlobalPriorChangedCallback(vtkIdType,
      double value);
  virtual void NodeParametersSpatialPriorWeightChangedCallback(vtkIdType,
      double value);
  virtual void RightClickOnInputChannelWeightsListCallback(int row, int col,
      int x, int y);
  virtual void NodeParametersInputChannelWeightChangedCallback(vtkIdType,
      int row, int col, const char *value);
  virtual void NodeParametersAlphaChangedCallback(vtkIdType, double value);
  virtual void StoppingConditionsEMCallback(vtkIdType, int type);
  virtual void StoppingConditionsEMIterationsCallback(vtkIdType,
      const char *value);
  virtual void StoppingConditionsEMValueCallback(vtkIdType,const char *value);
  virtual void StoppingConditionsMFACallback(vtkIdType, int type);
  virtual void StoppingConditionsMFAIterationsCallback(vtkIdType,
      const char *value);
  virtual void StoppingConditionsMFAValueCallback(vtkIdType,
      const char *value);
  virtual void StoppingConditionsBiasIterationsCallback(vtkIdType,
      const char *value);
  virtual void NodeParametersPrintWeightCallback(vtkIdType, int value);
  virtual void NodeParametersPrintQualityCallback(vtkIdType, int value);
  virtual void NodeParametersPrintFrequencyChangedCallback(vtkIdType,
      int value);
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

  virtual void Test();

  int NumberOfLeaves;
  int Depth;

  double Size;
  double ClassSize[400];
  double ClassWeight[200];

  vtkIdType LeafId[200];
  vtkIdType ClassPercentOrder[200][200];
  vtkIdType ClassPercentOrderCP[200][200];

  vtkIdType correspondanceArray[2][200];

  virtual void GetNumberOfLeaf(const char*, vtkIdType);

  virtual void GetParentPercent(int, vtkIdType);
  virtual void GetPercent(int, vtkIdType);
  virtual double GetWeight(int);

protected:
  vtkEMSegmentNodeParametersStep();
  ~vtkEMSegmentNodeParametersStep();

  virtual void PopulateIntensityDistributionTargetVolumeSelector();
  virtual void PopulateClassAndNodeList();

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

  vtkKWFrameWithLabel                *NodeParametersInteractionMatricesFrame;
  vtkKWFrameWithLabel                *NodeParametersPCAFrame;
  vtkKWFrameWithLabel                *NodeParametersRegistrationFrame;
  vtkKWFrameWithLabel                *NodeParametersMiscellaeneousFrame;
  vtkKWFrameWithLabel                *NodeParametersInhomogeneityFrame;

  vtkKWCheckButtonWithLabel
    *NodeParametersPrintEMLabelMapConvergenceCheckButton;
  vtkKWCheckButtonWithLabel
    *NodeParametersPrintEMWeightsConvergenceCheckButton;
  vtkKWCheckButtonWithLabel
    *NodeParametersPrintMFALabelMapConvergenceCheckButton;
  vtkKWCheckButtonWithLabel
    *NodeParametersPrintMFAWeightsConvergenceCheckButton;

  vtkKWCheckButtonWithLabel *NodeParametersExcludeIncompleteEStepCheckButton;
  vtkKWCheckButtonWithLabel
    *NodeParametersGenerateBackgroundProbabilityCheckButton;

  vtkKWFrameWithLabel       *UpdateFrame;
  vtkKWPushButton           *UpdatePrior;

  vtkKWMenuButtonWithLabel  *IntensityDistributionHistogramButton;

  vtkKWHistogram            *IntensityDistributionHistogramHistogram;
  vtkColorTransferFunction  *IntensityDistributionHistogramHistogramFunc;
  vtkKWEntryWithLabel       *NumberOfClassesEntryLabel;
  vtkKWMultiColumnList      *ClassAndNodeList;
  vtkKWPushButton           *TestButton;

  vtkKWColorTransferFunctionEditor
    *IntensityDistributionHistogramHistogramVisualization;

private:
  vtkEMSegmentNodeParametersStep(const vtkEMSegmentNodeParametersStep&);
  void operator=(const vtkEMSegmentNodeParametersStep&);
};

#endif

