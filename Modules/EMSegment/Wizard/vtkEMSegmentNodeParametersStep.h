/*=auto=======================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights
  Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkEMSegmentNodeParametetrsStep.h,v$
  Date:      $Date: 2006/01/06 17:56:51 $
  Version:   $Revision: 1.6 $
  Author:    $Nicolas Rannou (BWH), Sylvain Jaume (MIT)$

=======================================================================auto=*/

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

class vtkSlicerNodeSelectorWidget;

// NEW
class vtkKWHistogram;

class vtkImageReslice;

class vtkExtractVOI;

class vtkKWMultiColumnList;

class vtkKWColorTransferFunctionEditor;

class vtkKWEntryWithLabel;

class vtkColorTransferFunction;

class vtkImageData;

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
  virtual void HideUserInterface();

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
  virtual void IntensityDistributionTargetSelectionChangedCallback(
    vtkIdType VolId);

  // Description:
  // Reimplement the superclass's method.
  virtual void Validate();

  // Description:
  // Observers
  virtual void AddPointMovingGUIEvents();
  virtual void RemovePointMovingGUIEvents();
  virtual void ProcessPointMovingGUIEvents(
  vtkObject *caller, unsigned long event, void *callData);

  // Description:
  // Observers
  virtual void AddComputeWeightsButtonGUIEvents();
  virtual void RemoveComputeWeightsButtonGUIEvents();
  virtual void ProcessComputeWeightsButtonGUIEvents(
  vtkObject *caller, unsigned long event, void *callData);

  // Description:
  // Observers
  virtual void AddPreviewGUIObservers();
  virtual void RemovePreviewGUIObservers();
  virtual void ProcessPreviewGUIEvents(
    vtkObject *caller, unsigned long event, void *callData);

  // Description:
  // Observers
  virtual void AddColumnListGUIObservers();
  virtual void RemoveColumnListGUIObservers();
  virtual void ProcessColumnListGUIEvents(
    vtkObject *caller, unsigned long event, void *callData);

  virtual void GetLeavesRange();
  virtual void VisualFeedback();
  virtual void GetNumberOfLeaf(const char*, vtkIdType);
  virtual void GetNodesWeights(int, vtkIdType);
  virtual void FillTheTreeStructure(int, vtkIdType);
  virtual double GetLeafWeight(int);

  int NumberOfLeaves;
  int DepthOfTheNodeInTheTreeStructure;

  double TreeNodesWeights[200]; // 200 nodes
  double LeavesRange[400]; // 2*200 leaves
  double OrderedLabels[200];

  const char* LeavesName[200];

  vtkIdType LeavesID[200];
  vtkIdType LeavesIDNewOrder[200];
  vtkIdType TreeStructure[200][200]; // 200 leaves, 200 max depth of tree
  vtkIdType TreeStructureTemporary[200][200];
  vtkIdType TreeNodes[200];

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
  vtkSlicerNodeSelectorWidget        *PreviewSelector;
  vtkImageData                       *ArrayToAccessSliceValues;
  vtkKWMenuButtonWithLabel           *HistogramVolumeSelector;
  vtkKWHistogram                     *HistogramData;
  vtkKWColorTransferFunctionEditor   *HistogramVisualization;
  vtkColorTransferFunction           *HistogramColorFunction;
  vtkKWEntryWithLabel                *NbOfClassesEntryLabel;
  vtkKWMultiColumnList               *ClassAndNodeList;
  vtkKWPushButton                    *ComputeWeightsButton;
  vtkExtractVOI                      *SliceExtracted;

  virtual void PopulateIntensityDistributionTargetVolumeSelector();
  virtual void PopulateClassAndNodeList();

private:
  vtkEMSegmentNodeParametersStep(const vtkEMSegmentNodeParametersStep&);
  void operator=(const vtkEMSegmentNodeParametersStep&);
};

#endif
