/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerVolumesLogic.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

// .NAME vtkSlicerVolumesLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerVolumesLogic_h
#define __vtkSlicerVolumesLogic_h

#include <stdlib.h>

#include "vtkSlicerBaseLogic.h"
#include "vtkSlicerLogic.h"

#include "vtkMRML.h"
#include "vtkMRMLVolumeNode.h"


class VTK_SLICER_BASE_LOGIC_EXPORT vtkSlicerVolumesLogic : public vtkSlicerLogic 
{
  public:
  
  // The Usual vtk class functions
  static vtkSlicerVolumesLogic *New();
  vtkTypeRevisionMacro(vtkSlicerVolumesLogic,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // The currently active mrml volume node 
  vtkGetObjectMacro (ActiveVolumeNode, vtkMRMLVolumeNode);
  void SetActiveVolumeNode (vtkMRMLVolumeNode *ActiveVolumeNode);

  // Description:
  // Create new mrml node and associated storage node.
  // Read image data from a specified file
  vtkMRMLVolumeNode* AddArchetypeVolume (char* filename, int centerImage, int labelMap, const char* volname);

  // Description:
  // Write volume's image data to a specified file
  int SaveArchetypeVolume (char* filename, vtkMRMLVolumeNode *volumeNode);

  // Description:
  // Update MRML events
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                  unsigned long /*event*/, 
                                  void * /*callData*/ );    
  // Description:
  // Update logic events
  virtual void ProcessLogicEvents ( vtkObject * /*caller*/, 
                                  unsigned long /*event*/, 
                                  void * /*callData*/ );  

protected:
  vtkSlicerVolumesLogic();
  virtual ~vtkSlicerVolumesLogic();
  vtkSlicerVolumesLogic(const vtkSlicerVolumesLogic&);
  void operator=(const vtkSlicerVolumesLogic&);

  // Description:
  //
  vtkMRMLVolumeNode *ActiveVolumeNode;
};

#endif

