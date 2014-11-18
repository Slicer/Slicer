/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLUnstructuredGridDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

  =========================================================================auto=*/

#ifndef __vtkMRMLUnstructuredGridDisplayNode_h
#define __vtkMRMLUnstructuredGridDisplayNode_h

// MRML includes
#include "vtkMRMLDisplayNode.h"

// VTK includes
class vtkGeometryFilter;
class vtkPolyData;
class vtkShrinkPolyData;
class vtkUnstructuredGrid;

/// \brief MRML node to represent display properties for tractography.
///
/// vtkMRMLUnstructuredGridDisplayNode nodes store display properties of trajectories
/// from tractography in diffusion MRI data, including color type (by bundle, by fiber,
/// or by scalar invariants), display on/off for tensor glyphs and display of
/// trajectory as a line or tube.
class VTK_MRML_EXPORT vtkMRMLUnstructuredGridDisplayNode : public vtkMRMLDisplayNode
{
 public:
  static vtkMRMLUnstructuredGridDisplayNode *New (  );
  vtkTypeMacro ( vtkMRMLUnstructuredGridDisplayNode,vtkMRMLDisplayNode );
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
  virtual const char* GetNodeTagName ( ) {return "UnstructuredGridDisplay";};

  ///
  /// alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ );
  ///
  /// Sets UnstructuredGrid from UnstructuredGrid model node
  void SetUnstructuredGrid(vtkUnstructuredGrid *grid);

  ///
  /// Gets PlyData converted from UnstructuredGrid
  virtual vtkPolyData* GetPolyData();

  ///
  /// Update the pipeline based on this node attributes
  virtual void UpdatePolyDataPipeline();

  //--------------------------------------------------------------------------
  /// Display Information: Geometry to display (not mutually exclusive)
  //--------------------------------------------------------------------------

  ///
  /// cell shrink factor
  vtkSetMacro ( ShrinkFactor, double );
  vtkGetMacro ( ShrinkFactor, double );

 protected:
  vtkMRMLUnstructuredGridDisplayNode ( );
  ~vtkMRMLUnstructuredGridDisplayNode ( );
  vtkMRMLUnstructuredGridDisplayNode ( const vtkMRMLUnstructuredGridDisplayNode& );
  void operator= ( const vtkMRMLUnstructuredGridDisplayNode& );

  double ShrinkFactor;

  /// display pipeline
  vtkGeometryFilter *GeometryFilter;
  vtkShrinkPolyData *ShrinkPolyData;
};

#endif
