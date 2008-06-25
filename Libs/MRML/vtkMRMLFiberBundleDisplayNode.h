/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLFiberBundleDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

  =========================================================================auto=*/
// .NAME vtkMRMLFiberBundleDisplayNode - MRML node to represent display properties for tractography.
// .SECTION Description
// vtkMRMLFiberBundleDisplayNode nodes store display properties of trajectories 
// from tractography in diffusion MRI data, including color type (by bundle, by fiber, 
// or by scalar invariants), display on/off for tensor glyphs and display of 
// trajectory as a line or tube.
//

#ifndef __vtkMRMLFiberBundleDisplayNode_h
#define __vtkMRMLFiberBundleDisplayNode_h

#include "vtkMRML.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLDiffusionTensorDisplayPropertiesNode.h"

class VTK_MRML_EXPORT vtkMRMLFiberBundleDisplayNode : public vtkMRMLModelDisplayNode
{
 public:
  static vtkMRMLFiberBundleDisplayNode *New (  );
  vtkTypeMacro ( vtkMRMLFiberBundleDisplayNode,vtkMRMLModelDisplayNode );
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
  // Get node XML tag name (like Volume, FiberBundle)
  virtual const char* GetNodeTagName() = 0;

  // Description:
  // Updates this node if it depends on other nodes 
  // when the node is deleted in the scene
  virtual void UpdateReferences();

  // Description:
  // Finds the storage node and read the data
  virtual void UpdateScene(vtkMRMLScene *scene);

  // Description:
  // Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  // Description:
  // alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );

  //--------------------------------------------------------------------------
  // Display Information: Geometry to display (not mutually exclusive)
  //--------------------------------------------------------------------------


  //--------------------------------------------------------------------------
  // Display Information: Color Mode
  // 0) solid color by group 1) color by scalar invariant 
  // 2) color by avg scalar invariant 3) color by other
  //--------------------------------------------------------------------------

  //BTX
  enum
  {
    colorModeSolid = 0,
    colorModeScalar = 1,
    colorModeFunctionOfScalar = 2,
    colorModeUseCellScalars = 3
  };
  //ETX

  //--------------------------------------------------------------------------
  // Display Information: ColorMode for ALL nodes
  //--------------------------------------------------------------------------

 // Description:
  // Color mode for glyphs. The color modes are mutually exclusive.
  vtkGetMacro ( ColorMode, int );
  vtkSetMacro ( ColorMode, int );
 
  // Description:
  // Color by solid color (for example the whole fiber bundle red. blue, etc.)
  void SetColorModeToSolid ( ) {
    this->SetColorMode ( this->colorModeSolid );
  };

  // Description:
  // Color according to the tensors using various scalar invariants.
  void SetColorModeToScalar ( ) {
    this->SetColorMode ( this->colorModeScalar );
  };

  // Description:
  // Color according to the tensors using a function of scalar invariants along the tract.
  // This enables coloring by average FA, for example.
  void SetColorModeToFunctionOfScalar ( ) {
    this->SetColorMode ( this->colorModeFunctionOfScalar );
  };

  // Description:
  // Use to color by the active cell scalars.  This is intended to support
  // external processing of fibers, for example to label each with the distance
  // of that fiber from an fMRI activation.  Then by making that information
  // the active cell scalar field, this will allow coloring by that information.
  // TO DO: make sure this information can be saved with the tract, save name of
  // active scalar field if needed.
  void SetColorModeToUseCellScalars ( ) {
    this->SetColorMode ( this->colorModeUseCellScalars );
  };



  //--------------------------------------------------------------------------
  // Display Information: ColorMode for glyphs
  //--------------------------------------------------------------------------
  

  //--------------------------------------------------------------------------
  // MRML nodes that are observed
  //--------------------------------------------------------------------------
  
 
  // Node reference to ALL DT nodes

  // Description:
  // Get diffusion tensor display MRML object for fiber glyph.
  vtkMRMLDiffusionTensorDisplayPropertiesNode* GetDTDisplayPropertiesNode ( );

  // Description:
  // Set diffusion tensor display MRML object for fiber glyph.
  void SetAndObserveDTDisplayPropertiesNodeID ( const char *ID );

  // Description:
  // Get ID of diffusion tensor display MRML object for fiber glyph.
  vtkGetStringMacro(DTDisplayPropertiesNodeID);

 protected:
  vtkMRMLFiberBundleDisplayNode ( );
  ~vtkMRMLFiberBundleDisplayNode ( );
  vtkMRMLFiberBundleDisplayNode ( const vtkMRMLFiberBundleDisplayNode& );
  void operator= ( const vtkMRMLFiberBundleDisplayNode& );

  // ALL MRML nodes
  vtkMRMLDiffusionTensorDisplayPropertiesNode *DTDisplayPropertiesNode;
  char *DTDisplayPropertiesNodeID;

  vtkSetReferenceStringMacro(DTDisplayPropertiesNodeID);

  // Enumerated
  int ColorMode;

  // Arrays
  //double ScalarRange[2];

};

#endif
