/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerTractographyFiducialSeedingLogic.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

// .NAME vtkSlicerTractographyFiducialSeedingLogic - slicer logic class for fiber bundle display
// .SECTION Description
// This class manages the logic associated with creating standard slicer
// models for display of requested fiber bundle attributes
// (tubes, centerlines, glyphs, etc.)   These standard slicer models
// are then added to the scene as "hidden models" for display.

#ifndef __vtkSlicerTractographyFiducialSeedingLogic_h
#define __vtkSlicerTractographyFiducialSeedingLogic_h

#include <stdlib.h>

#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerTractographyModuleLogicExport.h"

class vtkMRMLDiffusionTensorVolumeNode;
class vtkMRMLFiducialListNode;
class vtkMRMLFiberBundleNode;
class vtkMRMLTransformableNode;
class vtkMaskPoints;

class VTK_SLICER_TRACTOGRAPHY_MODULE_LOGIC_EXPORT vtkSlicerTractographyFiducialSeedingLogic :
  public vtkSlicerModuleLogic
{
  public:
  
  // The Usual vtk class functions
  static vtkSlicerTractographyFiducialSeedingLogic *New();
  vtkTypeRevisionMacro(vtkSlicerTractographyFiducialSeedingLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  int CreateTracts(vtkMRMLDiffusionTensorVolumeNode *volumeNode,
                          vtkMRMLTransformableNode *fiducialListNode,
                          vtkMRMLFiberBundleNode *fiberNode,
                          const char * stoppinMode,
                          double stoppingValue, double stoppingCurvature, 
                          double integrationStepLength, double mnimumPathLength,
                          double resgionSize, double samplingStep,
                          int maxNumberOfSeeds,
                          int seedSelectedFiducials,
                          int displayMode);
protected:
  vtkSlicerTractographyFiducialSeedingLogic();
  ~vtkSlicerTractographyFiducialSeedingLogic();
  vtkSlicerTractographyFiducialSeedingLogic(const vtkSlicerTractographyFiducialSeedingLogic&);
  void operator=(const vtkSlicerTractographyFiducialSeedingLogic&);

  vtkMaskPoints *MaskPoints;

};

#endif

