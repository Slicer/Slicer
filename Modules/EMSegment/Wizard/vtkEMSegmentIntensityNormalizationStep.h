/*=auto=======================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights
  Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkEMSegmentIntensityNormalizationStep.h,v$
  Date:      $Date: 2006/01/06 17:56:51 $
  Version:   $Revision: 1.6 $
  Author:    $Nicolas Rannou (BWH), Sylvain Jaume (MIT)$

=======================================================================auto=*/

#ifndef __vtkEMSegmentIntensityNormalizationStep_h
#define __vtkEMSegmentIntensityNormalizationStep_h

#include "vtkEMSegmentStep.h"

class vtkKWMenuButton;
class vtkKWMenuButtonWithLabel;
class vtkKWScaleWithEntry;
class vtkKWFrameWithLabel;
class vtkKWEntryWithLabel;
class vtkKWCheckButtonWithLabel;
class vtkKWHistogram;
class vtkKWPiecewiseFunctionEditor;
class vtkKWRenderWidget;

class vtkActor;
class vtkPolyDataMapper;

class VTK_EMSEGMENT_EXPORT vtkEMSegmentIntensityNormalizationStep :
  public vtkEMSegmentStep
{
public:
  static vtkEMSegmentIntensityNormalizationStep *New();
  vtkTypeRevisionMacro(vtkEMSegmentIntensityNormalizationStep,
      vtkEMSegmentStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

  // Description:
  // Callbacks.
  virtual void NormalizationTargetSelectionChangedCallback(vtkIdType volId);
  virtual void NormalizationEnableCallback(vtkIdType volId, int state);
  virtual void NormalizationNormTypeCallback(vtkIdType volId, int enumType);
  virtual void NormalizationPrintInfoCallback(vtkIdType volId, int checked);
  virtual void NormalizationValueCallback(vtkIdType volId, double dValue);
  virtual void NormalizationSmoothingWidthCallback(vtkIdType volId, int
      iValue);
  virtual void NormalizationMaxSmoothingWidthCallback(vtkIdType volId, int
      iValue);
  virtual void NormalizationRelativeMaxVoxelNumCallback(vtkIdType volId,
      double dValue);

  virtual void NormalizationHistogramChangedCallback(vtkIdType volId);

  //Get histogram Value
  virtual void GetHistogramValue();

  // Description:
  // Observers
  virtual void AddCursorMovingGUIEvents();
  virtual void RemoveCursorMovingGUIEvents();
  virtual void ProcessCursorMovingGUIEvents(vtkObject *caller, unsigned long
      event, void *callData);

  //BTX
  enum
    {
    NormalizationDefaultT1SPGR = 0,
    NormalizationDefaultT2
    };
  //ETX

protected:
  vtkEMSegmentIntensityNormalizationStep();
  ~vtkEMSegmentIntensityNormalizationStep();

  virtual void PopulateNormalizationTargetVolumeSelector();
  virtual void PopulateNormalizationHistogramSelector();

  virtual void ResetDefaultParameters(vtkIdType targetvolId);
  virtual void HideUserInterface();

  vtkKWMenuButtonWithLabel     *NormalizationTargetVolumeMenuButton;
  vtkKWFrameWithLabel          *NormalizationParametersFrame;
  vtkKWCheckButtonWithLabel    *NormalizationEnableCheckButton;
  vtkKWMenuButton              *NormalizationDefaultsMenuButton;
  vtkKWCheckButtonWithLabel    *NormalizationPrintCheckButton;
  vtkKWEntryWithLabel          *NormalizationNormValueEntry;
  vtkKWEntryWithLabel          *NormalizationSmoothingWidthEntry;
  vtkKWEntryWithLabel          *NormalizationMaxSmoothingWidthEntry;
  vtkKWScaleWithEntry          *NormalizationRelativeMaxVoxelScale;

  vtkKWRenderWidget            *MaskRenderWidget;

  vtkKWMenuButtonWithLabel     *NormalizationHistogramMenuButton;
  vtkKWHistogram               *NormalizationHistogram;
  vtkKWFrameWithLabel          *NormalizationHistogramFrame;
  vtkKWPiecewiseFunctionEditor *NormalizationPiecewiseFunctionEditor;

  vtkKWEntryWithLabel          *NormalizationValueRecommendedEntry;
  vtkKWFrameWithLabel          *RecommendationFrame;

  vtkActor                     *Actor;
  vtkIdType                    IdEvent;
  vtkPolyDataMapper            *Mapper;

private:
  vtkEMSegmentIntensityNormalizationStep(const
      vtkEMSegmentIntensityNormalizationStep&);
  void operator=(const vtkEMSegmentIntensityNormalizationStep&);
};

#endif

