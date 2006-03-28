/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerSliceLayerLogic.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

// .NAME vtkSlicerSliceLayerLogic - slicer logic class for slice manipulation
// .SECTION Description
// This class manages the logic associated with reslicing of volumes
// (but not the GUI).  Features of the class include:
//
// - Reslicing 
// -- uses the vtkImageData and IJKToRAS transform from a vtkMRMLVolumeNode 
// -- disp
// -- uses a current slice view specification (typically set by vtkSlicerSliceLogic)
// - Outputs
// -- RGBA vtkImageData for the given slice
// -- image is mapped through current window/level and lookup table
//
// This class can also be used for resampling volumes for further computation.
//


#ifndef __vtkSlicerSliceLayerLogic_h
#define __vtkSlicerSliceLayerLogic_h

#include <stdlib.h>

#include "vtkSlicerBaseLogic.h"
#include "vtkSlicerLogic.h"

#include "vtkMrml.h"
#include "vtkMrmlVolumeNode.h"
#include "vtkMrmlScalarVolumeNode.h"
#include "vtkMrmlSliceNode.h"

#include "vtkImageReslice.h"
#include "vtkImageMapToRGBA.h"
#include "vtkImageMapToWindowLevelColors.h"

class VTK_SLICER_BASE_LOGIC_EXPORT vtkSlicerSliceLayerLogic : public vtkSlicerLogic 
{
  public:
  
  // The Usual vtk class functions
  static vtkSlicerSliceLayerLogic *New();
  vtkTypeRevisionMacro(vtkSlicerSliceLayerLogic,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // The volume node to operate on
  vtkGetObjectMacro (VolumeNode, vtkMRMLVolumeNode);
  void SetVolumeNode (vtkMRMLVolumeNode *VolumeNode);

  // Description:
  // The slice node that defines the view 
  vtkGetObjectMacro (SliceNode, vtkMRMLSliceNode);
  void SetSliceNode (vtkMRMLSliceNode *SliceNode);

  // Description:
  // The image reslice being used
  vtkGetObjectMacro (Reslice, vtkImageReslice);

  // Description:
  // The image map that applies the window/level
  // this happens before the color map, so can't use the color map
  // that's part of WindowLevelColors
  vtkGetObjectMacro (MapToWindowLevelColors, vtkImageMapToWindowLevelColors);

  // Description:
  // The image map that applies the lookup table
  vtkGetObjectMacro (MapToRGBA, vtkImageMapToRGBA);

    
protected:
  vtkSlicerSliceLayerLogic();
  ~vtkSlicerSliceLayerLogic();
  vtkSlicerSliceLayerLogic(const vtkSlicerSliceLayerLogic&);
  void operator=(const vtkSlicerSliceLayerLogic&);

  // Description:
  // 
  vtkMRMLVolumeNode *VolumeNode;
  vtkMRMLSliceNode *SliceNode;
  vtkImageReslice *Reslice;
  vtkImageMapToRGBA *MapToRGBA;
  vtkImageMapToWindowLevelColors *MapToWindowLevelColors;
};

#endif

