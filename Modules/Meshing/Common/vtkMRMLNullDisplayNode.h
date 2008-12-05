/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLNullDisplayNode.h,v $
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

#ifndef __vtkMRMLNullDisplayNode_h
#define __vtkMRMLNullDisplayNode_h

#include <string>

#include "vtkMimxCommonWin32Header.h"
#include "vtkUnstructuredGrid.h"
#include "vtkShrinkPolyData.h"
#include "vtkGeometryFilter.h"

#include "vtkMRML.h"
#include "vtkMRMLModelDisplayNode.h"


//class vtkMeshQualityExtended; 
class vtkPolyData;


class VTK_MIMXCOMMON_EXPORT vtkMRMLNullDisplayNode : public vtkMRMLDisplayNode
{
 public:
  static vtkMRMLNullDisplayNode *New (  );
  vtkTypeMacro ( vtkMRMLNullDisplayNode,vtkMRMLDisplayNode );
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
  virtual const char* GetNodeTagName ( ) {return "FiniteElementBoundingBoxDisplay";};

  // Description:
  // alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );
 
   // overload the virtual placeholder in the parent class.  This one will setup
   // the beginning of the actual pipeline for rendering an unstructured grid instead
   virtual void SetUnstructuredGrid(vtkUnstructuredGrid *grid);
 
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
     vtkMRMLNullDisplayNode ( );
  ~vtkMRMLNullDisplayNode ( );
  vtkMRMLNullDisplayNode ( const vtkMRMLNullDisplayNode& );
  void operator= ( const vtkMRMLNullDisplayNode& );

  vtkPolyData *NullDataset;
 

  // display pipeline components declared here

};

#endif
