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
//  -- each layer is required to provide an RGBA image in the space defined by the vtkMRMLSliceNode
//
// This class manages internal vtk pipelines that create an output vtkImageData
// which can be used by the vtkSlicerSliceGUI class to display the resulting
// composite image or it can be used as a texture map in a vtkSlicerView.
// This class can also be used for resampling volumes for further computation.


#ifndef __vtkSlicerSliceLogic_h
#define __vtkSlicerSliceLogic_h

#include <stdlib.h>

#include "vtkSlicerBaseLogic.h"
#include "vtkSlicerLogic.h"

#include "vtkMRML.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLSliceCompositeNode.h"
#include "vtkSlicerSliceLayerLogic.h"

#include "vtkImageBlend.h"

class VTK_SLICER_BASE_LOGIC_EXPORT vtkSlicerSliceLogic : public vtkSlicerLogic 
{
  public:
  
  // The Usual vtk class functions
  static vtkSlicerSliceLogic *New();
  vtkTypeRevisionMacro(vtkSlicerSliceLogic,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // The mrml slice node for this slice logic
  vtkGetObjectMacro (SliceNode, vtkMRMLSliceNode);
  void SetSliceNode (vtkMRMLSliceNode *SliceNode);

  // Description:
  // The mrml slice node for this slice logic
  vtkGetObjectMacro (SliceCompositeNode, vtkMRMLSliceCompositeNode);
  void SetSliceCompositeNode (vtkMRMLSliceCompositeNode *SliceCompositeNode);

  // Description:
  // The background slice layer
  // TODO: this will eventually be generalized to a list of layers
  vtkGetObjectMacro (BackgroundLayer, vtkSlicerSliceLayerLogic);
  void SetBackgroundLayer (vtkSlicerSliceLayerLogic *BackgroundLayer);

  // Description:
  // The forground slice layer
  // TODO: this will eventually be generalized to a list of layers
  vtkGetObjectMacro (ForegroundLayer, vtkSlicerSliceLayerLogic);
  void SetForegroundLayer (vtkSlicerSliceLayerLogic *ForegroundLayer);

  // Description:
  // The opacity of the forground slice layer
  // TODO: this will eventually be generalized to a per-layer compositing function
  // -- could be checkerboard or other filter
  vtkGetMacro (ForegroundOpacity, double);
  void SetForegroundOpacity (double ForegroundOpacity);

  // Description:
  // The compositing filter
  // TODO: this will eventually be generalized to a per-layer compositing function
  vtkGetObjectMacro (Blend, vtkImageBlend);

  // Description:
  // the tail of the pipeline
  vtkImageData *GetImageData () { return (this->Blend->GetOutput()); };
    
protected:
  vtkSlicerSliceLogic();
  ~vtkSlicerSliceLogic();
  vtkSlicerSliceLogic(const vtkSlicerSliceLogic&);
  void operator=(const vtkSlicerSliceLogic&);

  // Description:
  //
  vtkMRMLSliceNode *SliceNode;
  vtkMRMLSliceCompositeNode *SliceCompositeNode;
  vtkSlicerSliceLayerLogic *BackgroundLayer;
  vtkSlicerSliceLayerLogic *ForegroundLayer;
  double ForegroundOpacity;
  vtkImageBlend *Blend;
};

#endif

