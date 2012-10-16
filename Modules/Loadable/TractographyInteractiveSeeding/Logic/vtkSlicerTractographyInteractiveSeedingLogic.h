/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerTractographyInteractiveSeedingLogic.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

// .NAME vtkSlicerTractographyInteractiveSeedingLogic - slicer logic class for fiber bundle display
// .SECTION Description
// This class manages the logic associated with creating standard slicer
// models for display of requested fiber bundle attributes
// (tubes, centerlines, glyphs, etc.)   These standard slicer models
// are then added to the scene as "hidden models" for display.

#ifndef __vtkSlicerTractographyInteractiveSeedingLogic_h
#define __vtkSlicerTractographyInteractiveSeedingLogic_h

#include <cstdlib>

#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerTractographyInteractiveSeedingModuleLogicExport.h"

class vtkMRMLTractographyInteractiveSeedingNode;
class vtkMRMLDiffusionTensorVolumeNode;
class vtkMRMLScalarVolumeNode;
class vtkMRMLAnnotationHierarchyNode;
class vtkMRMLFiberBundleNode;
class vtkMRMLTransformableNode;
class vtkMaskPoints;
class vtkSeedTracts;

class VTK_SLICER_TRACTOGRAPHYINTERACTIVESEEDING_MODULE_LOGIC_EXPORT vtkSlicerTractographyInteractiveSeedingLogic :
  public vtkSlicerModuleLogic
{
public:

  // The Usual vtk class functions
  static vtkSlicerTractographyInteractiveSeedingLogic *New();
  vtkTypeRevisionMacro(vtkSlicerTractographyInteractiveSeedingLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void ProcessMRMLNodesEvents(vtkObject* caller,
                                      unsigned long event,
                                      void * callData);

  // Get parameters node
  vtkGetObjectMacro (TractographyInteractiveSeedingNode, vtkMRMLTractographyInteractiveSeedingNode);

  void CreateTractsForOneSeed(vtkSeedTracts *seed,
                              vtkMRMLDiffusionTensorVolumeNode *volumeNode,
                              vtkMRMLTransformableNode *transformableNode,
                              int stoppingMode,
                              double stoppingValue,
                              double stoppingCurvature,
                              double integrationStepLength,
                              double mnimumPathLength,
                              double regionSize, double sampleStep,
                              int maxNumberOfSeeds,
                              int seedSelectedFiducials);

  int CreateTracts( vtkMRMLTractographyInteractiveSeedingNode *parametersNode,
                    vtkMRMLDiffusionTensorVolumeNode *volumeNode,
                    vtkMRMLNode *seedingNode,
                    vtkMRMLFiberBundleNode *fiberNode,
                    int stoppinMode,
                    double stoppingValue, double stoppingCurvature,
                    double integrationStepLength, double mnimumPathLength,
                    double resgionSize, double samplingStep,
                    int maxNumberOfSeeds,
                    int seedSelectedFiducials,
                    int displayMode);

  int CreateTractsForLabelMap(
                              vtkSeedTracts *seed,
                              vtkMRMLDiffusionTensorVolumeNode *volumeNode,
                              vtkMRMLScalarVolumeNode *seedingNode,
                              int ROIlabel,
                              int useIndexSpace,
                              double seedSpacing,
                              int randomGrid,
                              double linearMeasureStart,
                              int stoppingMode,
                              double stoppingValue,
                              double stoppingCurvature,
                              double integrationStepLength,
                              double minPathLength,
                              double maxPathLength);

  void SetAndObserveTractographyInteractiveSeedingNode(vtkMRMLTractographyInteractiveSeedingNode *node);

  int IsObservedNode(vtkMRMLNode *node);

protected:
  vtkSlicerTractographyInteractiveSeedingLogic();
  ~vtkSlicerTractographyInteractiveSeedingLogic();
  vtkSlicerTractographyInteractiveSeedingLogic(const vtkSlicerTractographyInteractiveSeedingLogic&);
  void operator=(const vtkSlicerTractographyInteractiveSeedingLogic&);

  /// Register node classes into the mrml scene. Called each time a new scene
  /// is set. Do nothing by default. Can be reimplemented in derivated classes.
  virtual void RegisterNodes();

  // Initialize listening to MRML events
  virtual void SetMRMLSceneInternal(vtkMRMLScene * newScene);

  virtual void OnMRMLSceneEndImport();
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);

  virtual void OnMRMLNodeModified(vtkMRMLNode* node);

  void AddMRMLNodesObservers();

  void RemoveMRMLNodesObservers();

  vtkMaskPoints *MaskPoints;

  vtkMRMLTractographyInteractiveSeedingNode *TractographyInteractiveSeedingNode;
  std::vector<vtkMRMLTransformableNode *> ObservedNodes;
  vtkMRMLDiffusionTensorVolumeNode       *DiffusionTensorVolumeNode;

};

#endif

