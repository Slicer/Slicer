/*=auto==============================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All
Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkEMSegmentSpatialPriorsStep.h,v$
Date:      $Date: 2006/01/06 17:56:51 $
Version:   $Revision: 1.6 $
Author:    $Nicolas Rannou (BWH), Sylvain Jaume (MIT)$

==============================================================auto=*/

#ifndef __vtkEMSegmentSpatialPriorsStep_h
#define __vtkEMSegmentSpatialPriorsStep_h

#include "vtkEMSegmentStep.h"

class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;

class VTK_EMSEGMENT_EXPORT vtkEMSegmentSpatialPriorsStep :
  public vtkEMSegmentStep
{
public:
  static vtkEMSegmentSpatialPriorsStep *New();
  vtkTypeRevisionMacro(vtkEMSegmentSpatialPriorsStep,vtkEMSegmentStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void ShowUserInterface();

  // Description:
  // Callbacks.
  virtual void SpatialPriorsVolumeCallback(vtkIdType, vtkIdType volumeId);
  virtual void DisplaySelectedNodeSpatialPriorsCallback();

protected:
  vtkEMSegmentSpatialPriorsStep();
  ~vtkEMSegmentSpatialPriorsStep();

  vtkKWFrameWithLabel      *SpatialPriorsVolumeFrame;
  vtkKWMenuButtonWithLabel *SpatialPriorsVolumeMenuButton;

private:
  vtkEMSegmentSpatialPriorsStep(const vtkEMSegmentSpatialPriorsStep&);
  void operator=(const vtkEMSegmentSpatialPriorsStep&);
};

#endif
