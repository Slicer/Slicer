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
// -- Colors vtkImageData for the given slice
// -- image is mapped through current window/level and lookup table
//
// This class can also be used for resampling volumes for further computation.
//


#ifndef __vtkSlicerSliceLayerLogic_h
#define __vtkSlicerSliceLayerLogic_h

#include <stdlib.h>

#include "vtkSlicerBaseLogic.h"
#include "vtkSlicerLogic.h"

#include "vtkMRML.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLSliceNode.h"

#include "vtkImageResliceMask.h"
#include "vtkImageReslice.h"
#include "vtkImageSlice.h"
#include "vtkImageMapToColors.h"
#include "vtkAssignAttribute.h"

#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLVectorVolumeNode.h"
#include "vtkMRMLDiffusionWeightedVolumeNode.h"
#include "vtkMRMLDiffusionTensorVolumeNode.h"

#include "vtkImageMapToWindowLevelColors.h"
#include "vtkImageThreshold.h"
#include "vtkImageAppendComponents.h"
#include "vtkImageLogic.h"
#include "vtkImageExtractComponents.h"
#include "vtkImageCast.h"
#include "vtkLookupTable.h"


class vtkDiffusionTensorMathematics;

class VTK_SLICER_BASE_LOGIC_EXPORT vtkSlicerSliceLayerLogic : public vtkSlicerLogic 
{
  public:
  
  // The Usual vtk class functions
  static vtkSlicerSliceLayerLogic *New();
  vtkTypeRevisionMacro(vtkSlicerSliceLayerLogic,vtkSlicerLogic);
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
  vtkGetObjectMacro (Slice, vtkImageSlice);
  vtkGetObjectMacro (Reslice, vtkImageResliceMask);

  // Description:
  // Select the vtkImageResliceMask or slicer's own vtkImageSlice
  vtkGetMacro (UseReslice, int);
  vtkSetMacro (UseReslice, int);
  vtkBooleanMacro(UseReslice, int);

  // Description:
  // The image map that applies the window/level
  // this happens before the color map, so can't use the color map
  // that's part of WindowLevelColors
  vtkGetObjectMacro (MapToWindowLevelColors, vtkImageMapToWindowLevelColors);

  // Description:
  // The image map that applies the lookup table
  vtkGetObjectMacro (MapToColors, vtkImageMapToColors);
  
  // Description:
  // The filter that applies the threshold
  vtkGetObjectMacro (Threshold, vtkImageThreshold);

  // Description:
  // The filter that applies the threshold to the input of the Reslice
  // so there's a fully opaque alpha channel within the image
  // but fully transparent outside of the image
  vtkGetObjectMacro (ResliceThreshold, vtkImageThreshold);

  // Description:
  // The add the alpha channel onto the image
  vtkGetObjectMacro (AppendComponents, vtkImageAppendComponents);

  // Description:
  // The add the alpha channel onto the image before the reslice
  vtkGetObjectMacro (ResliceAppendComponents, vtkImageAppendComponents);

  // Description:
  // Extract the two channels after reslice for separate processing
  vtkGetObjectMacro (ResliceExtractLuminance, vtkImageExtractComponents);
  vtkGetObjectMacro (ResliceExtractAlpha, vtkImageExtractComponents);

  // Description:
  // Used to convert the alpha channel of the reslice output to be unsigned char
  // so it can be blended with the image based threshold
  vtkGetObjectMacro (ResliceAlphaCast, vtkImageCast);

  // Description:
  // combine the reslice with the threshold 
  vtkGetObjectMacro (AlphaLogic, vtkImageLogic);

  // Description:
  // Get the output of the pipeline for this layer
  vtkImageData *GetImageData () { 
    if ( this->GetVolumeNode() == NULL || this->GetVolumeDisplayNode() == NULL) 
      {
      return NULL;
      } 
    else
      {
      return this->GetVolumeDisplayNode()->GetImageData();
      }
    /**
    if ( this->GetVolumeNode() == NULL ) 
      {
      return NULL;
      } 
    else
      {
      return (this->GetAppendComponents()->GetOutput());
      **/

  };

  void UpdateImageDisplay();

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

  void UpdateGlyphs(vtkImageData *sliceImage); 


  // Description:
  // Check that we are observing the correct display node
  // (correct means the same one that the volume node is referencing)
  void UpdateNodeReferences(); 

  // Description:
  // The current reslice transform XYToIJK
  vtkGetObjectMacro (XYToIJKTransform, vtkTransform);
    
protected:
  vtkSlicerSliceLayerLogic();
  virtual ~vtkSlicerSliceLayerLogic();
  vtkSlicerSliceLayerLogic(const vtkSlicerSliceLayerLogic&);
  void operator=(const vtkSlicerSliceLayerLogic&);

  // Description:
  // the MRML Nodes that define this Logic's parameters
  vtkMRMLVolumeNode *VolumeNode;
  vtkMRMLVolumeDisplayNode *VolumeDisplayNode;
  vtkMRMLVolumeDisplayNode *VolumeDisplayNodeObserved;
  vtkMRMLSliceNode *SliceNode;

  // Description:
  // the VTK class instances that implement this Logic's operations
  vtkImageThreshold *ResliceThreshold;
  vtkImageAppendComponents *ResliceAppendComponents;
  vtkImageExtractComponents *ResliceExtractLuminance;
  vtkImageExtractComponents *ResliceExtractAlpha;
  vtkImageCast *ResliceAlphaCast;
  vtkImageLogic *AlphaLogic;
  vtkImageResliceMask *Reslice;
  vtkImageSlice *Slice;
  vtkImageMapToColors *MapToColors;
  vtkImageThreshold *Threshold;
  vtkImageAppendComponents *AppendComponents;
  vtkImageMapToWindowLevelColors *MapToWindowLevelColors;

  // Description:
  // VTK class instances that implement the DWI logic operations
  vtkImageExtractComponents *DWIExtractComponent;

  vtkImageReslice *DTIReslice;
  vtkDiffusionTensorMathematics *DTIMathematics;

  vtkAssignAttribute* AssignAttributeTensorsFromScalars;
  vtkAssignAttribute* AssignAttributeScalarsFromTensors;

  // TODO: make this a vtkAbstractTransform for non-linear
  vtkTransform *XYToIJKTransform;

  int UseReslice;

  // Description:
  // Generic pipeline for scalar slice logic
  void ScalarSlicePipeline(vtkImageData *imageData, int labelMap, double window, double level, int interpolate, vtkLookupTable *lookupTable, int applyThreshold, double lowerThreshold, double upperThreshold);

  void VectorSlicePipeline(vtkImageData *imageData, int interpolate);
};

#endif

