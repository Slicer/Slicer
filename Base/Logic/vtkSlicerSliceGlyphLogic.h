/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerSliceGlyphLogic.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

// .NAME vtkSlicerSliceGlyphLogic - slicer logic class for slice manipulation
// .SECTION Description
// This class manages the logic associated with reslicing of volumes
// (but not the GUI).  Features of the class include:
//
// - Reslicing 
// -- uses the vtkImageData and IJKToRAS transform from a vtkMRMLVolumeNode 
// -- disp
// -- uses a current slice view specification (typically set by vtkSlicerSliceLogic)
// - Outputs
// -- Colors vtkImageData for the given slice
// -- image is mapped through current window/level and lookup table
//
// This class can also be used for resampling volumes for further computation.
//


#ifndef __vtkSlicerSliceGlyphLogic_h
#define __vtkSlicerSliceGlyphLogic_h

#include <stdlib.h>

#include "vtkSlicerBaseLogic.h"
#include "vtkSlicerLogic.h"
#include "vtkSlicerGlyphLogic.h"

#include "vtkMRML.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLSliceNode.h"

#include "vtkImageReslice.h"
#include "vtkImageSlice.h"
#include "vtkPolyData.h"
#include "vtkAssignAttribute.h"

#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLVectorVolumeNode.h"
#include "vtkMRMLDiffusionWeightedVolumeNode.h"
#include "vtkMRMLDiffusionTensorVolumeNode.h"

#include "vtkImageMapToWindowLevelColors.h"
#include "vtkImageThreshold.h"
#include "vtkImageLogic.h"
#include "vtkImageExtractComponents.h"
#include "vtkImageCast.h"
#include "vtkLookupTable.h"

class vtkDiffusionTensorMathematics;

class VTK_SLICER_BASE_LOGIC_EXPORT vtkSlicerSliceGlyphLogic : public vtkSlicerLogic 
{
  public:
  
  // The Usual vtk class functions
  static vtkSlicerSliceGlyphLogic *New();
  vtkTypeRevisionMacro(vtkSlicerSliceGlyphLogic,vtkSlicerLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // The volume node to operate on
  vtkGetObjectMacro (VolumeNode, vtkMRMLVolumeNode);
  void SetVolumeNode (vtkMRMLVolumeNode *VolumeNode);

  // Description:
  // The volume display node has the render properties of the volume
  // - this node is set implicitly when the volume is set
  //   and it is observed by this logic
  vtkGetObjectMacro (VolumeDisplayNode, vtkMRMLVolumeDisplayNode);

  // Description:
  // The slice node that defines the view 
  vtkGetObjectMacro (SliceNode, vtkMRMLSliceNode);
  void SetSliceNode (vtkMRMLSliceNode *SliceNode);

  // Description:
  // The image reslice or slice being used
  vtkGetObjectMacro (Reslice, vtkImageReslice);

  // Description:
  // The glyph logic 
  vtkGetObjectMacro (SlicerGlyphLogic, vtkSlicerGlyphLogic);

  // Description:
  // The image map that applies the lookup table
  vtkGetObjectMacro (LookupTable, vtkScalarsToColors);

  // Description:
  // Get the output of the pipeline for this layer
  virtual vtkPolyData* GetPolyData();
  
  // Description:
  // provide the virtual method that updates this Logic based
  // on mrml changes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                  unsigned long /*event*/, 
                                  void * /*callData*/ );
  // Description:
  // set the Reslice transforms to reflect the current state
  // of the VolumeNode and the SliceNode
  void UpdateTransforms(); 

  void ScalarVolumeNodeUpdateTransforms();

  void VectorVolumeNodeUpdateTransforms();

  void DiffusionWeightedVolumeNodeUpdateTransforms();

  void DiffusionTensorVolumeNodeUpdateTransforms();

  // Description:
  // Check that we are observing the correct display node
  // (correct means the same one that the volume node is referencing)
  void UpdateNodeReferences(); 

  // Description:
  // The current reslice transform XYToIJK
  vtkGetObjectMacro (XYToIJKTransform, vtkTransform);
    
protected:
  vtkSlicerSliceGlyphLogic();
  virtual ~vtkSlicerSliceGlyphLogic();
  vtkSlicerSliceGlyphLogic(const vtkSlicerSliceGlyphLogic&);
  void operator=(const vtkSlicerSliceGlyphLogic&);

  // Description:
  // the MRML Nodes that define this Logic's parameters
  vtkMRMLVolumeNode *VolumeNode;
  vtkMRMLVolumeDisplayNode *VolumeDisplayNode;
  vtkMRMLSliceNode *SliceNode;


  // Description:
  // the VTK class that will handle the glyhing of the sliced volume
  vtkSlicerGlyphLogic* SlicerGlyphLogic;


  // Description:
  // the VTK class instances that implement this Logic's operations
  vtkImageMapToWindowLevelColors *MapToWindowLevelColors;
  vtkImageReslice *Reslice;
  vtkScalarsToColors *LookupTable;

  // Description:
  // VTK class instances that implement the DWI logic operations
  vtkImageExtractComponents *DWIExtractComponent;

  vtkImageReslice *DTIReslice;
  vtkDiffusionTensorMathematics *DTIMathematics;

  // TODO: make this a vtkAbstractTransform for non-linear
  vtkTransform *XYToIJKTransform;

  vtkAssignAttribute* AssignAttributeTensorsFromScalars;
  vtkAssignAttribute* AssignAttributeScalarsFromTensors;


  void UpdatePipeline();
};

#endif

