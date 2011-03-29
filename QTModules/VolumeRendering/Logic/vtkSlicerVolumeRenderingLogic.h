/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerVolumeRenderingLogic.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

// .NAME vtkSlicerVolumeRenderingLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerVolumeRenderingLogic_h
#define __vtkSlicerVolumeRenderingLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes
#include <vtkMRML.h>
#include <vtkMRMLVolumeNode.h>
#include "vtkMRMLVolumeRenderingParametersNode.h"
#include "vtkMRMLVolumeRenderingScenarioNode.h"

// STD includes
#include <cstdlib>

#include "VolumeRenderingLogicExport.h"

class vtkMRMLScalarVolumeNode;
class vtkMRMLScalarVolumeDisplayNode;
class vtkMRMLVolumeHeaderlessStorageNode;
class vtkStringArray;

class Q_SLICER_QTMODULES_VOLUMERENDERING_LOGIC_EXPORT vtkSlicerVolumeRenderingLogic :
  public vtkSlicerModuleLogic
{
public:
  
  static vtkSlicerVolumeRenderingLogic *New();
  vtkTypeRevisionMacro(vtkSlicerVolumeRenderingLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Register local MRML nodes
  virtual void RegisterNodes();

  ///
  /// Update MRML events
  virtual void ProcessMRMLEvents(vtkObject * caller, unsigned long event, void * callData);
  
  ///
  /// Update logic events
  virtual void ProcessLogicEvents(vtkObject * caller, unsigned long event, void * callData);  
  
  //BTX
  using vtkMRMLAbstractLogic::ProcessLogicEvents; 
  //ETX

  // Description:
  // Create VolumeRenderingParametersNode
  vtkMRMLVolumeRenderingParametersNode* CreateParametersNode();

  // Description:
  // Create VolumeRenderingScenarioNode
  vtkMRMLVolumeRenderingScenarioNode* CreateScenarioNode();

  void UpdateVolumePropertyScalarRange(vtkMRMLVolumeRenderingParametersNode* vspNode);

  void UpdateFgVolumePropertyScalarRange(vtkMRMLVolumeRenderingParametersNode* vspNode);

  void UpdateVolumePropertyByDisplayNode(vtkMRMLVolumeRenderingParametersNode* vspNode);

  void SetupVolumePropertyFromImageData(vtkMRMLVolumeRenderingParametersNode* vspNode);

  void SetupFgVolumePropertyFromImageData(vtkMRMLVolumeRenderingParametersNode* vspNode);

  void FitROIToVolume(vtkMRMLVolumeRenderingParametersNode* vspNode);


protected:
  vtkSlicerVolumeRenderingLogic();
  virtual ~vtkSlicerVolumeRenderingLogic();
  
private:

  vtkSlicerVolumeRenderingLogic(const vtkSlicerVolumeRenderingLogic&); // Not implemented
  void operator=(const vtkSlicerVolumeRenderingLogic&);               // Not implemented
};

#endif

