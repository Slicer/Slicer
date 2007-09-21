/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLFiberBundleLineDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

  =========================================================================auto=*/
// .NAME vtkMRMLFiberBundleLineDisplayNode - MRML node to represent display properties for tractography.
// .SECTION Description
// vtkMRMLFiberBundleLineDisplayNode nodes store display properties of trajectories 
// from tractography in diffusion MRI data, including color type (by bundle, by fiber, 
// or by scalar invariants), display on/off for tensor glyphs and display of 
// trajectory as a line or tube.
//

#ifndef __vtkMRMLFiberBundleLineDisplayNode_h
#define __vtkMRMLFiberBundleLineDisplayNode_h

#include <string>

#include "vtkPolyData.h"
#include "vtkShrinkPolyData.h"
#include "vtkGeometryFilter.h"

#include "vtkMRML.h"
#include "vtkMRMLFiberBundleDisplayNode.h"
#include "vtkMRMLDiffusionTensorDisplayPropertiesNode.h"

class VTK_MRML_EXPORT vtkMRMLFiberBundleLineDisplayNode : public vtkMRMLFiberBundleDisplayNode
{
 public:
  static vtkMRMLFiberBundleLineDisplayNode *New (  );
  vtkTypeMacro ( vtkMRMLFiberBundleLineDisplayNode,vtkMRMLModelDisplayNode );
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
  virtual const char* GetNodeTagName ( ) {return "FiberBundleLineDisplayNode";};

 
  //--------------------------------------------------------------------------
  // Display Information: Geometry to display (not mutually exclusive)
  //--------------------------------------------------------------------------

 protected:
  vtkMRMLFiberBundleLineDisplayNode ( );
  ~vtkMRMLFiberBundleLineDisplayNode ( );
  vtkMRMLFiberBundleLineDisplayNode ( const vtkMRMLFiberBundleLineDisplayNode& );
  void operator= ( const vtkMRMLFiberBundleLineDisplayNode& );

  int TwoDimensionalVisibility;


  // dispaly pipeline
};

#endif
