/*=auto==============================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All
Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkEMSegmentIntensityDistributionsStep.h,v$
Date:      $Date: 2006/01/06 17:56:51 $
Version:   $Revision: 1.6 $
Author:    $Nicolas Rannou (BWH), Sylvain Jaume (MIT)$

==============================================================auto=*/

#ifndef __vtkEMSegmentIntensityDistributionsStep_h
#define __vtkEMSegmentIntensityDistributionsStep_h

#include "vtkEMSegmentStep.h"
#include "vtkMRMLScene.h"

class vtkKWFrameWithLabel;
class vtkKWMatrixWidgetWithLabel;
class vtkKWNotebook;
class vtkKWMultiColumnListWithScrollbarsWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkKWHistogram;
class vtkKWPushButton;

class vtkKWMultiColumnList;
class vtkKWColorTransferFunctionEditor;
class vtkKWEntryWithLabel;
class vtkColorTransferFunction;
class vtkGaussian2DWidget;

class vtkSlicerNodeSelectorWidget;

class VTK_EMSEGMENT_EXPORT vtkEMSegmentIntensityDistributionsStep :
  public vtkEMSegmentStep
{
public:
  static vtkEMSegmentIntensityDistributionsStep *New();
  vtkTypeRevisionMacro(vtkEMSegmentIntensityDistributionsStep,
      vtkEMSegmentStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();
  virtual void HideUserInterface();

  // Description:
  // Callbacks.
  virtual void IntensityDistributionSpecificationCallback(vtkIdType,
      int type);
  virtual void DisplaySelectedNodeIntensityDistributionsCallback();
  virtual void IntensityDistributionMeanChangedCallback(
      vtkIdType, int row, int col, const char *text);
  virtual void IntensityDistributionCovarianceChangedCallback(
      vtkIdType, int row, int col, const char *text);
  virtual void PopupManualIntensitySampleContextMenuCallback(
      int row, int col, int x, int y);
  virtual void DeleteManualIntensitySampleCallback(vtkIdType,
      int sampleIndex);
  virtual void DeleteAllManualIntensitySampleCallback(vtkIdType);
  virtual void IntensityDistributionTargetSelectionChangedCallback(
      vtkIdType volId);
  virtual void Gaussian2DVolumeXSelectionChangedCallback(vtkIdType
      targetVolId);
  virtual void Gaussian2DRenderingCallback(vtkIdType targetVolId, int type);

  // Description:
  // Observers
  virtual void AddManualIntensitySamplingGUIObservers();
  virtual void RemoveManualIntensitySamplingGUIObservers();
  virtual void ProcessManualIntensitySamplingGUIEvents(
  vtkObject *caller, unsigned long event, void *callData);

  // Description:
  // Observers
  virtual void AddGaussian2DButtonGUIEvents();
  virtual void RemoveGaussian2DButtonGUIEvents();
  virtual void ProcessGaussian2DButtonGUIEvents(vtkObject *caller,
      unsigned long event, void *callData);
      
  // Description:
  // Observers
  virtual void AddLabelButtonGUIEventsObservers();
  virtual void RemoveLabelButtonGUIEventsObservers();
  virtual void ProcessLabelButtonGUIEvents(vtkObject *caller,
      unsigned long event, void *callData);
      
  // Description:
  // Observers
  virtual void AddLabelSelectorGUIObservers();
  virtual void RemoveLabelSelectorGUIObservers();
  virtual void ProcessLabelSelectorGUIEvents(vtkObject *caller,
      unsigned long event, void *callData);
      
      
  
  // Description:
  // Get the number of the volumes to be displayed
  // on X and Y axes
  virtual void GetGaussian2DTargetVolumes();

  int    NumberOfLeaves;
  int    LeafLabel[200];
  int    TargetVolumeX;
  int    TargetVolumeY;
  double *TargetVolumeXRange;
  double *TargetVolumeYRange;
  
  vtkIdType LeavesID[200];

  vtkIdType LeafId[200];
  vtkIdType ClassPercentOrder[200][200];
  vtkIdType ClassPercentOrderCP[200][200];
  vtkIdType CorrespondenceArray[2][200];

  virtual void   GetNumberOfLeaf(const char*, vtkIdType);

protected:
  vtkEMSegmentIntensityDistributionsStep();
  ~vtkEMSegmentIntensityDistributionsStep();

  virtual void PopulateIntensityDistributionTargetVolumeSelector();
  //virtual void PopulateXVolumeSelector();
  //virtual void PopulateYVolumeSelector();
  virtual void PopulateGaussian2DVolumeXSelector();
  virtual void PopulateGaussian2DVolumeYSelector();

  vtkKWNotebook              *IntensityDistributionNotebook;
  vtkKWMenuButtonWithLabel   *IntensityDistributionSpecificationMenuButton;
  vtkKWMatrixWidgetWithLabel *IntensityDistributionMeanMatrix;
  vtkKWMatrixWidgetWithLabel *IntensityDistributionCovarianceMatrix;
  vtkKWMenu                  *ContextMenu;

  vtkKWMultiColumnListWithScrollbarsWithLabel
    *IntensityDistributionManualSamplingList;

  vtkKWMenuButtonWithLabel  *IntensityDistributionHistogramButton;
  vtkKWFrameWithLabel       *IntensityDistributionHistogramFrame;


  vtkGaussian2DWidget       *Gaussian2DWidget;
  vtkKWEntryWithLabel       *NumClassesEntryLabel;
  vtkKWPushButton           *Gaussian2DButton;
  vtkKWMenuButtonWithLabel  *Gaussian2DVolumeXMenuButton;
  vtkKWMenuButtonWithLabel  *Gaussian2DVolumeYMenuButton;
  vtkKWMenuButtonWithLabel  *Gaussian2DRenderingMenuButton;
  
  vtkSlicerNodeSelectorWidget *LabelSelector;
  vtkKWPushButton             *LabelmapButton;

  virtual void AddIntensityDistributionSamplePoint(double ras[3]);

private:
  vtkEMSegmentIntensityDistributionsStep(const
      vtkEMSegmentIntensityDistributionsStep&);
  void operator=(const vtkEMSegmentIntensityDistributionsStep&);
};

#endif

