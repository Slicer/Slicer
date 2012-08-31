/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLFiberBundleLineDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

  =========================================================================auto=*/
///  vtkMRMLFiberBundleLineDisplayNode - MRML node to represent display properties for tractography.
///
/// vtkMRMLFiberBundleLineDisplayNode nodes store display properties of trajectories
/// from tractography in diffusion MRI data, including color type (by bundle, by fiber,
/// or by scalar invariants), display on/off for tensor glyphs and display of
/// trajectory as a line or tube.
//

#ifndef __vtkMRMLFiberBundleLineDisplayNode_h
#define __vtkMRMLFiberBundleLineDisplayNode_h

#include "vtkMRMLFiberBundleDisplayNode.h"

class vtkPolyData;
class vtkPolyDataTensorToColor;
class vtkPolyDataColorLinesByOrientation;


class VTK_SLICER_TRACTOGRAPHYDISPLAY_MODULE_MRML_EXPORT vtkMRMLFiberBundleLineDisplayNode
  : public vtkMRMLFiberBundleDisplayNode
{
 public:
  static vtkMRMLFiberBundleLineDisplayNode *New (  );
  vtkTypeMacro ( vtkMRMLFiberBundleLineDisplayNode, vtkMRMLFiberBundleDisplayNode );
  void PrintSelf ( ostream& os, vtkIndent indent );

  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance (  );

  ///
  /// Get node XML tag name (like Volume, UnstructuredGrid)
  virtual const char* GetNodeTagName ( ) {return "FiberBundleLineDisplayNode";};

  ///
  /// Update the pipeline based on this node attributes
  virtual void UpdatePolyDataPipeline();


 protected:
  vtkMRMLFiberBundleLineDisplayNode ( );
  ~vtkMRMLFiberBundleLineDisplayNode ( );
  vtkMRMLFiberBundleLineDisplayNode ( const vtkMRMLFiberBundleLineDisplayNode& );
  void operator= ( const vtkMRMLFiberBundleLineDisplayNode& );

  /// Reimplemented to return the output of the display pipeline
  virtual vtkAlgorithmOutput* GetOutputPort();

  /// display pipeline
  vtkPolyDataTensorToColor *TensorToColor;
  vtkPolyDataColorLinesByOrientation *ColorLinesByOrientation;
};

#endif
