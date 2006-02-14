/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerSliceLogic.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

// .NAME vtkSlicerSliceLogic - slicer logic class for slice manipulation
// .SECTION Description
// This class manages the logic associated with display of slice windows
// (but not the GUI).  Features of the class include:
//  -- a back-to-front list of MrmlVolumes to be displayed
//  -- a compositing mode for each volume layer (opacity, outline, glyph, checkerboard, etc)
//  -- a current slice view specification 
//  ---- slab thickness 
//  ---- slice center
//  ---- pan/zoom settings
//  ---- slice space (pixel, RAS)
//  ---- view coordinate space (RAS, Camera Relative, other)
//
// This class manages internal vtk pipelines that create an output vtkImageData
// which can be used by the vtkSlicerSliceGUI class to display the resulting
// composite image or it can be used as a texture map in a vtkSlicerView.
// This class can also be used for resampling volumes for further computation.


#ifndef __vtkSlicerSliceLogic_h
#define __vtkSlicerSliceLogic_h

#include <stdlib.h>

#include "vtkSlicerLogic.h"
//#include "vtkMrml.h"
//#include "vtkMrmlVolume.h"

class VTK_SLICER_BASE_EXPORT vtkSlicerSliceLogic : public vtkSlicerLogic 
{
  public:
  
  // The Usual vtk class functions
  static vtkSlicerSliceLogic *New();
  vtkTypeRevisionMacro(vtkSlicerSliceLogic,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);
    
protected:
  vtkSlicerSliceLogic();
  ~vtkSlicerSliceLogic();
  vtkSlicerSliceLogic(const vtkSlicerSliceLogic&) {};
  void operator=(const vtkSlicerSliceLogic&) {};

  // Description:
  // 
};

#endif

