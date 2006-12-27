/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLFiberBundleDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/
// .NAME vtkMRMLFiberBundleDisplayNode - MRML node to represent display properties
// of trajectories from tractography in diffusion MRI data.
// .SECTION Description
// vtkMRMLFiberBundleDisplayNode nodes stores display properties for tractography
// including color (by bundle, by fiber, or by scalar invariants), display
// on/off for tensor glyphs and display of trajectory as a line or tube.

#ifndef __vtkMRMLFiberBundleDisplayNode_h
#define __vtkMRMLFiberBundleDisplayNode_h

#include <string>

#include "vtkMRML.h"
#include "vtkMRMLModelNode.h"

class VTK_MRML_EXPORT vtkMRMLFiberBundleDisplayNode : public vtkMRMLModelNode
{
public:
  static vtkMRMLFiberBundleDisplayNode *New();
  vtkTypeMacro(vtkMRMLFiberBundleDisplayNode,vtkMRMLModelNode);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);


  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);
  
  // Description:
  // Get node XML tag name (like Volume, FiberBundle)
  virtual const char* GetNodeTagName() {return "FiberBundleDisplay";};

  // Description:
  // alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );
  
protected:
  vtkMRMLFiberBundleDisplayNode();
  ~vtkMRMLFiberBundleDisplayNode();
  vtkMRMLFiberBundleDisplayNode(const vtkMRMLFiberBundleDisplayNode&);
  void operator=(const vtkMRMLFiberBundleDisplayNode&);

  // Numbers
  //double Opacity;
  
  // Enumerated
  // Here we need scalar display mode, glyph display mode, everything
  // from vtkMRMLDiffusionTensorVolumeDisplayNode...

  // color by: cell scalars (by fiber), by tensors (scalar display mode), 
  // or by bundle color (using superclass color info). Also would like
  // to color by any slicer volume, ex fMRI
  int ColorBy;

  // Booleans
  int LineVisibility;
  int TubeVisibility;
  int TwoDimensionalVisibility;

  // superclass has tensor visibility
  //int TensorVisibility;  
  

  // Arrays
  //double ScalarRange[2];

};

#endif
