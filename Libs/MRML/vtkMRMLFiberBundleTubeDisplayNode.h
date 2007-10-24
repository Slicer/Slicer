/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLFiberBundleTubeDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

  =========================================================================auto=*/
// .NAME vtkMRMLFiberBundleTubeDisplayNode - MRML node to represent display properties for tractography.
// .SECTION Description
// vtkMRMLFiberBundleTubeDisplayNode nodes store display properties of trajectories 
// from tractography in diffusion MRI data, including color type (by bundle, by fiber, 
// or by scalar invariants), display on/off for tensor glyphs and display of 
// trajectory as a line or tube.
//

#ifndef __vtkMRMLFiberBundleTubeDisplayNode_h
#define __vtkMRMLFiberBundleTubeDisplayNode_h

#include <string>

#include "vtkPolyData.h"

#include "vtkMRML.h"
#include "vtkMRMLFiberBundleDisplayNode.h"
#include "vtkMRMLDiffusionTensorDisplayPropertiesNode.h"

class vtkPolyDataTensorToColor;
class vtkTubeFilter;

class VTK_MRML_EXPORT vtkMRMLFiberBundleTubeDisplayNode : public vtkMRMLFiberBundleDisplayNode
{
 public:
  static vtkMRMLFiberBundleTubeDisplayNode *New (  );
  vtkTypeMacro ( vtkMRMLFiberBundleTubeDisplayNode, vtkMRMLFiberBundleDisplayNode );
  void PrintSelf ( ostream& os, vtkIndent indent );
  
  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance (  );

  // Description:
  // Read node attributes from XML (MRML) file
  virtual void ReadXMLAttributes ( const char** atts );

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML ( ostream& of, int indent );


  // Description:
  // Copy the node's attributes to this object
  virtual void Copy ( vtkMRMLNode *node );
  
  // Description:
  // Get node XML tag name (like Volume, UnstructuredGrid)
  virtual const char* GetNodeTagName ( ) {return "FiberBundleTubeDisplayNode";};

  // Description:
  // Sets polydata for glyph input (usually stored in FiberBundle node)
  void SetPolyData(vtkPolyData *glyphPolyData);

  // Description:
  // Gets resultin glyph PolyData 
  virtual vtkPolyData* GetPolyData();

  // Description:
  // Update the pipeline based on this node attributes
  virtual void UpdatePolyDataPipeline();
 
  //--------------------------------------------------------------------------
  // Display Information: Geometry to display (not mutually exclusive)
  //--------------------------------------------------------------------------

  // Description:
  // Number of tube sides
  vtkSetMacro ( TubeRadius , double );
  vtkGetMacro ( TubeRadius , double );

  // Description:
  // Number of tube sides
  vtkSetMacro ( TubeNumberOfSides , int );
  vtkGetMacro ( TubeNumberOfSides , int );
  vtkBooleanMacro ( TubeNumberOfSides , int );


 protected:
  vtkMRMLFiberBundleTubeDisplayNode ( );
  ~vtkMRMLFiberBundleTubeDisplayNode ( );
  vtkMRMLFiberBundleTubeDisplayNode ( const vtkMRMLFiberBundleTubeDisplayNode& );
  void operator= ( const vtkMRMLFiberBundleTubeDisplayNode& );


  // Enumerated

  int    TubeNumberOfSides;
  double TubeRadius;

  // dispaly pipeline
  vtkTubeFilter *TubeFilter;
  vtkPolyDataTensorToColor *TensorToColor;

};

#endif
