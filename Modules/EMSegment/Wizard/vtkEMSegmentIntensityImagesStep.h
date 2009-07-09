/*=auto==============================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All
Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkEMSegmentIntensityImagesStep.h,v$
Date:      $Date: 2006/01/06 17:56:51 $
Version:   $Revision: 1.6 $
Author:    $Nicolas Rannou (BWH), Sylvain Jaume (MIT)$

==============================================================auto=*/

#ifndef __vtkEMSegmentIntensityImagesStep_h
#define __vtkEMSegmentIntensityImagesStep_h

#include "vtkEMSegmentStep.h"

class vtkKWFrameWithLabel;
class vtkKWListBoxToListBoxSelectionEditor;
class vtkKWCheckButtonWithLabel;

class VTK_EMSEGMENT_EXPORT vtkEMSegmentIntensityImagesStep : public vtkEMSegmentStep
{
public:
  static vtkEMSegmentIntensityImagesStep *New();
  vtkTypeRevisionMacro(vtkEMSegmentIntensityImagesStep,vtkEMSegmentStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

  // Description:
  // Callbacks.
  virtual void IntensityImagesTargetSelectionChangedCallback();
  virtual void AlignTargetImagesCallback(int state);

  // Description:
  // Reimplement the superclass's method.
  virtual void Validate();

protected:
  vtkEMSegmentIntensityImagesStep();
  ~vtkEMSegmentIntensityImagesStep();

  virtual void PopulateIntensityImagesTargetVolumeSelector();

  vtkKWFrameWithLabel                  *IntensityImagesTargetSelectorFrame;
  vtkKWListBoxToListBoxSelectionEditor *IntensityImagesTargetVolumeSelector;
  vtkKWFrameWithLabel        *TargetToTargetRegistrationFrame;
  vtkKWCheckButtonWithLabel  *IntensityImagesAlignTargetImagesCheckButton;

private:
  vtkEMSegmentIntensityImagesStep(const vtkEMSegmentIntensityImagesStep&);
  void operator=(const vtkEMSegmentIntensityImagesStep&);
};

#endif
