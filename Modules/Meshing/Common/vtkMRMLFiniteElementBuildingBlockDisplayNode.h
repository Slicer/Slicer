/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLFiniteElementBuildingBlockDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

  =========================================================================auto=*/
// .NAME vtkMRMLUnstructuredGridDisplayNode - MRML node to represent display properties for tractography.
// .SECTION Description
// vtkMRMLUnstructuredGridDisplayNode nodes store display properties of trajectories 
// from tractography in diffusion MRI data, including color type (by bundle, by fiber, 
// or by scalar invariants), display on/off for tensor glyphs and display of 
// trajectory as a line or tube.
//

#ifndef __vtkMRMLFiniteElementBuildingBlockDisplayNode_h
#define __vtkMRMLFiniteElementBuildingBlockDisplayNode_h

#include <string>

#include "vtkUnstructuredGrid.h"
#include "vtkShrinkPolyData.h"
#include "vtkGeometryFilter.h"
#include "vtkMimxCommonWin32Header.h"

#include "vtkMRML.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLUnstructuredGridNode.h"
#include "vtkMRMLUnstructuredGridDisplayNode.h"

class vtkFeatureEdges;
class vtkTubeFilter;

class VTK_MIMXCOMMON_EXPORT vtkMRMLFiniteElementBuildingBlockDisplayNode : public vtkMRMLUnstructuredGridDisplayNode
{
 public:
  static vtkMRMLFiniteElementBuildingBlockDisplayNode *New (  );
  vtkTypeMacro ( vtkMRMLFiniteElementBuildingBlockDisplayNode,vtkMRMLUnstructuredGridDisplayNode );
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
  virtual const char* GetNodeTagName ( ) {return "FiniteElementBuildingBlockDisplay";};

  // Description:
  // alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );
 

  // declare a rendering pipeline for bblock data in this class
  virtual vtkPolyData* GetPolyData();
   
  // Description:
  // Update the pipeline based on this node attributes
  virtual void UpdatePolyDataPipeline();
  
 
  //--------------------------------------------------------------------------
  // Display Information: Geometry to display (not mutually exclusive)
  //--------------------------------------------------------------------------

  // Description:
 
    
 protected:
     vtkMRMLFiniteElementBuildingBlockDisplayNode ( );
  ~vtkMRMLFiniteElementBuildingBlockDisplayNode ( );
  vtkMRMLFiniteElementBuildingBlockDisplayNode ( const vtkMRMLFiniteElementBuildingBlockDisplayNode& );
  void operator= ( const vtkMRMLFiniteElementBuildingBlockDisplayNode& );

  vtkFeatureEdges* FeatureEdges;
  vtkTubeFilter* OutlineTube;
 

  // dispaly pipeline components declared here

};

#endif
