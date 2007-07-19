/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerGlyphLogic.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

// .NAME vtkSlicerGlyphLogic - slicer logic class for slice manipulation
// .SECTION Description
// This class manages the logic associated with reslicing of volumes
// (but not the GUI).  Features of the class include:
//
// - Reslicing 
// -- uses the vtkImageData and IJKToRAS transform from a vtkMRMLVolumeNode 
// -- disp
// -- uses a current slice view specification (typically set by vtkSlicerLogic)
// - Outputs
// -- Colors vtkImageData for the given slice
// -- image is mapped through current window/level and lookup table
//
// This class can also be used for resampling volumes for further computation.
//


#ifndef __vtkSlicerGlyphLogic_h
#define __vtkSlicerGlyphLogic_h

#include <stdlib.h>

#include "vtkSlicerBaseLogic.h"
#include "vtkSlicerLogic.h"

#include "vtkMRML.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLVolumeGlyphDisplayNode.h"

#include "vtkImageData.h"
#include "vtkPolyData.h"

class VTK_SLICER_BASE_LOGIC_EXPORT vtkSlicerGlyphLogic : public vtkSlicerLogic 
{
  public:
  
  // The Usual vtk class functions
  static vtkSlicerGlyphLogic *New();
  vtkTypeRevisionMacro(vtkSlicerGlyphLogic,vtkSlicerLogic);
  void PrintSelf(ostream& os, vtkIndent indent);


  // Description:
  // The volume display node has the render properties of the volume
  // - this node is set implicitly when the volume is set
  //   and it is observed by this logic
  vtkGetObjectMacro (VolumeDisplayNode, vtkMRMLVolumeDisplayNode);
  void SetVolumeDisplayNode (vtkMRMLVolumeDisplayNode *VolumeDisplayNode);

  vtkGetObjectMacro (ImageData, vtkImageData);
  vtkSetObjectMacro (ImageData, vtkImageData);

  virtual vtkPolyData* GetPolyData();

  protected:
    vtkSlicerGlyphLogic();
    ~vtkSlicerGlyphLogic();
    vtkSlicerGlyphLogic(const vtkSlicerGlyphLogic&);
    void operator=(const vtkSlicerGlyphLogic&);

    vtkMRMLVolumeDisplayNode* VolumeDisplayNode;
    vtkImageData* ImageData;   
    vtkPolyData*  PolyData; 
};

#endif

