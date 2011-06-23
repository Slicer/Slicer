/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerCropVolumeLogic.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

// .NAME vtkSlicerCropVolumeLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerCropVolumeLogic_h
#define __vtkSlicerCropVolumeLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes
#include <vtkMRML.h>

// STD includes
#include <cstdlib>

#include "vtkSlicerCropVolumeModuleLogicExport.h"

class vtkMRMLVolumeNode;
class vtkMRMLScalarVolumeNode;
class vtkMRMLScalarVolumeDisplayNode;
class vtkMRMLVolumeHeaderlessStorageNode;
class vtkStringArray;
class vtkMRMLCropVolumeParametersNode;

/// \ingroup Slicer_QtModules_CropVolume
class VTK_SLICER_CROPVOLUME_MODULE_LOGIC_EXPORT vtkSlicerCropVolumeLogic :
  public vtkSlicerModuleLogic
{
public:
  
  static vtkSlicerCropVolumeLogic *New();
  vtkTypeRevisionMacro(vtkSlicerCropVolumeLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  ///
  /// Update MRML events
  virtual void ProcessMRMLEvents(vtkObject * caller, unsigned long event, void * callData);
  
  ///
  /// Update logic events
  virtual void ProcessLogicEvents(vtkObject * caller, unsigned long event, void * callData);  
  
  //BTX
  using vtkMRMLAbstractLogic::ProcessLogicEvents; 
  //ETX
  
  int Apply(vtkMRMLCropVolumeParametersNode*);

  virtual void RegisterNodes();

protected:
  vtkSlicerCropVolumeLogic();
  virtual ~vtkSlicerCropVolumeLogic();

private:

  vtkSlicerCropVolumeLogic(const vtkSlicerCropVolumeLogic&); // Not implemented
  void operator=(const vtkSlicerCropVolumeLogic&);               // Not implementedo

};

#endif

