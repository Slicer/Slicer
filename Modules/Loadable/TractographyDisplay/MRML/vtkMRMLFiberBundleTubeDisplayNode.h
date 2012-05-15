/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLFiberBundleTubeDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

  =========================================================================auto=*/
///  vtkMRMLFiberBundleTubeDisplayNode - MRML node to represent display properties for tractography.
/// 
/// vtkMRMLFiberBundleTubeDisplayNode nodes store display properties of trajectories 
/// from tractography in diffusion MRI data, including color type (by bundle, by fiber, 
/// or by scalar invariants), display on/off for tensor glyphs and display of 
/// trajectory as a line or tube.
//

#ifndef __vtkMRMLFiberBundleTubeDisplayNode_h
#define __vtkMRMLFiberBundleTubeDisplayNode_h

#include "vtkMRMLFiberBundleDisplayNode.h"

class vtkPolyData;
class vtkPolyDataTensorToColor;
class vtkTubeFilter;

class VTK_SLICER_TRACTOGRAPHY_DISPLAY_MODULE_MRML_EXPORT vtkMRMLFiberBundleTubeDisplayNode : public vtkMRMLFiberBundleDisplayNode
{
 public:
  static vtkMRMLFiberBundleTubeDisplayNode *New (  );
  vtkTypeMacro ( vtkMRMLFiberBundleTubeDisplayNode, vtkMRMLFiberBundleDisplayNode );
  void PrintSelf ( ostream& os, vtkIndent indent );
  
  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance (  );

  /// 
  /// Read node attributes from XML (MRML) file
  virtual void ReadXMLAttributes ( const char** atts );

  /// 
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML ( ostream& of, int indent );


  /// 
  /// Copy the node's attributes to this object
  virtual void Copy ( vtkMRMLNode *node );
  
  /// 
  /// Get node XML tag name (like Volume, UnstructuredGrid)
  virtual const char* GetNodeTagName ( ) {return "FiberBundleTubeDisplayNode";};

  /// 
  /// Sets polydata for glyph input (usually stored in FiberBundle node)
  void SetPolyData(vtkPolyData *glyphPolyData);

  /// 
  /// Gets resultin glyph PolyData 
  virtual vtkPolyData* GetPolyData();

  /// 
  /// Update the pipeline based on this node attributes
  virtual void UpdatePolyDataPipeline();
 
  //--------------------------------------------------------------------------
  /// Display Information: Geometry to display (not mutually exclusive)
  //--------------------------------------------------------------------------

  /// 
  /// Number of tube sides
  vtkSetMacro ( TubeRadius , double );
  vtkGetMacro ( TubeRadius , double );

  /// 
  /// Number of tube sides
  vtkSetMacro ( TubeNumberOfSides , int );
  vtkGetMacro ( TubeNumberOfSides , int );
  vtkBooleanMacro ( TubeNumberOfSides , int );


 protected:
  vtkMRMLFiberBundleTubeDisplayNode ( );
  ~vtkMRMLFiberBundleTubeDisplayNode ( );
  vtkMRMLFiberBundleTubeDisplayNode ( const vtkMRMLFiberBundleTubeDisplayNode& );
  void operator= ( const vtkMRMLFiberBundleTubeDisplayNode& );


  /// Enumerated

  int    TubeNumberOfSides;
  double TubeRadius;

  /// dispaly pipeline
  vtkTubeFilter *TubeFilter;
  vtkPolyDataTensorToColor *TensorToColor;

};

#endif
