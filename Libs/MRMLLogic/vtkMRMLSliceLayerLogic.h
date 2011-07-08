/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLSliceLayerLogic.h,v $
  Date:      $Date$
  Version:   $Revision$

=========================================================================auto=*/

///  vtkMRMLSliceLayerLogic - slicer logic class for slice manipulation
/// 
/// This class manages the logic associated with reslicing of volumes
/// (but not the GUI).  Features of the class include:
//
/// - Reslicing 
/// -- uses the vtkImageData and IJKToRAS transform from a vtkMRMLVolumeNode 
/// -- disp
/// -- uses a current slice view specification (typically set by vtkMRMLSliceLogic)
/// - Outputs
/// -- Colors vtkImageData for the given slice
/// -- image is mapped through current window/level and lookup table
//
/// This class can also be used for resampling volumes for further computation.
//


#ifndef __vtkMRMLSliceLayerLogic_h
#define __vtkMRMLSliceLayerLogic_h

// MRMLLogic includes
#include "vtkMRMLAbstractLogic.h"

// MRML includes
#include "vtkMRML.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLVectorVolumeNode.h"
#include "vtkMRMLDiffusionWeightedVolumeNode.h"
#include "vtkMRMLDiffusionTensorVolumeNode.h"

// VTK includes
#include "vtkImageMapToWindowLevelColors.h"
#include "vtkImageThreshold.h"
#include "vtkImageAppendComponents.h"
#include "vtkImageLogic.h"
#include "vtkImageExtractComponents.h"
#include "vtkImageCast.h"
#include "vtkLookupTable.h"

//
#include "vtkImageResliceMask.h"
#include "vtkImageReslice.h"
#include "vtkImageLinearReslice.h"
#include "vtkAssignAttribute.h"

// STL includes
#include <cstdlib>

#include "vtkMRMLLogicWin32Header.h"

class vtkDiffusionTensorMathematics;
class vtkImageLabelOutline;
class VTK_MRML_LOGIC_EXPORT vtkMRMLSliceLayerLogic : public vtkMRMLAbstractLogic 
{
public:
  
  /// The Usual vtk class functions
  static vtkMRMLSliceLayerLogic *New();
  vtkTypeRevisionMacro(vtkMRMLSliceLayerLogic,vtkMRMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// 
  /// The volume node to operate on
  vtkGetObjectMacro (VolumeNode, vtkMRMLVolumeNode);
  void SetVolumeNode (vtkMRMLVolumeNode *VolumeNode);

  /// 
  /// The volume display node has the render properties of the volume
  /// - this node is set implicitly when the volume is set
  ///   and it is observed by this logic
  vtkGetObjectMacro (VolumeDisplayNode, vtkMRMLVolumeDisplayNode);

  /// 
  /// The slice node that defines the view 
  vtkGetObjectMacro (SliceNode, vtkMRMLSliceNode);
  void SetSliceNode (vtkMRMLSliceNode *SliceNode);

  /// 
  /// The image reslice or slice being used
  vtkGetObjectMacro (Slice, vtkImageLinearReslice);
  vtkGetObjectMacro (Reslice, vtkImageResliceMask);

  /// 
  /// Select if this is a label layer or not (it currently determines if we use
  /// the label outline filter)
  vtkGetMacro (IsLabelLayer, int);
  vtkSetMacro (IsLabelLayer, int);
  vtkBooleanMacro (IsLabelLayer, int);

  /// 
  /// The filter that applies the threshold
  vtkGetObjectMacro (Threshold, vtkImageThreshold);

  /// 
  /// The filter that turns the label map into an outline
  vtkGetObjectMacro (LabelOutline, vtkImageLabelOutline);
  
  /// 
  /// The filter that applies the threshold to the input of the Reslice
  /// so there's a fully opaque alpha channel within the image
  /// but fully transparent outside of the image
  vtkGetObjectMacro (ResliceThreshold, vtkImageThreshold);

  /// 
  /// The add the alpha channel onto the image
  vtkGetObjectMacro (AppendComponents, vtkImageAppendComponents);

  /// 
  /// The add the alpha channel onto the image before the reslice
  vtkGetObjectMacro (ResliceAppendComponents, vtkImageAppendComponents);

  /// 
  /// Extract the two channels after reslice for separate processing
  vtkGetObjectMacro (ResliceExtractLuminance, vtkImageExtractComponents);
  vtkGetObjectMacro (ResliceExtractAlpha, vtkImageExtractComponents);

  /// 
  /// Used to convert the alpha channel of the reslice output to be unsigned char
  /// so it can be blended with the image based threshold
  vtkGetObjectMacro (ResliceAlphaCast, vtkImageCast);

  /// 
  /// combine the reslice with the threshold 
  vtkGetObjectMacro (AlphaLogic, vtkImageLogic);

  /// 
  /// Get the output of the pipeline for this layer
  vtkImageData *GetImageData ();

  void UpdateImageDisplay();

  /// 
  /// provide the virtual method that updates this Logic based
  /// on mrml changes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                  unsigned long /*event*/, 
                                  void * /*callData*/ );
  /// 
  /// set the Reslice transforms to reflect the current state
  /// of the VolumeNode and the SliceNode
  void UpdateTransforms();

  void UpdateGlyphs(); 


  /// 
  /// Check that we are observing the correct display node
  /// (correct means the same one that the volume node is referencing)
  void UpdateNodeReferences(); 

  /// 
  /// The current reslice transform XYToIJK
  vtkGetObjectMacro (XYToIJKTransform, vtkTransform);
    
protected:
  vtkMRMLSliceLayerLogic();
  virtual ~vtkMRMLSliceLayerLogic();
  vtkMRMLSliceLayerLogic(const vtkMRMLSliceLayerLogic&);
  void operator=(const vtkMRMLSliceLayerLogic&);

  // Initialize listening to MRML events
  virtual void SetMRMLSceneInternal(vtkMRMLScene * newScene);

  vtkImageData* GetSliceImageData();

  /// 
  /// the MRML Nodes that define this Logic's parameters
  vtkMRMLVolumeNode *VolumeNode;
  vtkMRMLVolumeDisplayNode *VolumeDisplayNode;
  vtkMRMLVolumeDisplayNode *VolumeDisplayNodeObserved;
  vtkMRMLSliceNode *SliceNode;

  /// 
  /// the VTK class instances that implement this Logic's operations
  vtkImageThreshold *ResliceThreshold;
  vtkImageAppendComponents *ResliceAppendComponents;
  vtkImageExtractComponents *ResliceExtractLuminance;
  vtkImageExtractComponents *ResliceExtractAlpha;
  vtkImageCast *ResliceAlphaCast;
  vtkImageLogic *AlphaLogic;
  vtkImageResliceMask *Reslice;
  vtkImageLinearReslice *Slice;
  vtkImageThreshold *Threshold;
  vtkImageLabelOutline *LabelOutline;
  vtkImageAppendComponents *AppendComponents;

  /// 
  /// VTK class instances that implement the DWI logic operations
  vtkImageExtractComponents *DWIExtractComponent;

  vtkImageReslice *DTIReslice;
  vtkDiffusionTensorMathematics *DTIMathematics;

  vtkAssignAttribute* AssignAttributeTensorsFromScalars;
  vtkAssignAttribute* AssignAttributeScalarsFromTensors;

  /// TODO: make this a vtkAbstractTransform for non-linear
  vtkTransform *XYToIJKTransform;

  int IsLabelLayer;

  int UpdatingTransforms;
};

#endif

