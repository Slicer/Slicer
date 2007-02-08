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

#include <string>

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
  virtual const char* GetNodeTagName ( ) {return "FiberBundleDisplay";};

  // Description:
  // alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );

  //--------------------------------------------------------------------------
  // Display Information: Geometry to display (not mutually exclusive)
  //--------------------------------------------------------------------------

  // Description:
  // Turn on/off visibility of trajectory (fiber) geometry.
  vtkSetMacro ( FiberLineVisibility, int );
  vtkGetMacro ( FiberLineVisibility, int );
  vtkBooleanMacro ( FiberLineVisibility, int );
  
  // Description:
  // Turn on/off visibility of tubed trajectory ("thick fibers") geometry.
  vtkSetMacro ( FiberTubeVisibility , int );
  vtkGetMacro ( FiberTubeVisibility , int );
  vtkBooleanMacro ( FiberTubeVisibility , int );
  
  // Description:
  // Turn on/off visibility of glyphs (tensors) along fibers.
  vtkSetMacro ( TensorGlyphVisibility , int );
  vtkGetMacro ( TensorGlyphVisibility , int );
  vtkBooleanMacro ( TensorGlyphVisibility , int );

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
  // Display Information: ColorMode for lines
  //--------------------------------------------------------------------------

  // Description:
  // Color mode for lines. The color modes are mutually exclusive.
  vtkGetMacro ( ColorModeForFiberLines, int );
  vtkSetMacro ( ColorModeForFiberLines, int );
 
  // Description:
  // Color by solid color (for example the whole fiber bundle red. blue, etc.)
  // TO DO: The color is defined by a vtkMRMLColorNode.
  // TO DO: how to define specularity, etc. for fiber lines.
  void SetColorModeForFiberLinesToSolid ( ) {
    this->SetColorModeForFiberLines ( this->colorModeSolid );
  };

  // Description:
  // Color according to the tensors using various scalar invariants.
  void SetColorModeForFiberLinesToScalar ( ) {
    this->SetColorModeForFiberLines ( this->colorModeScalar );
  };

  // Description:
  // Color according to the tensors using a function of scalar invariants along the tract.
  // This enables coloring by average FA, for example.
  void SetColorModeForFiberLinesToFunctionOfScalar ( ) {
    this->SetColorModeForFiberLines ( this->colorModeFunctionOfScalar );
  };

  // Description:
  // Use to color by the active cell scalars.  This is intended to support
  // external processing of fibers, for example to label each with the distance
  // of that fiber from an fMRI activation.  Then by making that information
  // the active cell scalar field, this will allow coloring by that information.
  // TO DO: make sure this information can be saved with the tract, save name of
  // active scalar field if needed.
  void SetColorModeForFiberLinesToUseCellScalars ( ) {
    this->SetColorModeForFiberLines ( this->colorModeUseCellScalars );
  };

  //--------------------------------------------------------------------------
  // MRML nodes that are observed
  //--------------------------------------------------------------------------
  
  // Description:
  // Get diffusion tensor display MRML object for fiber line.
  vtkMRMLDiffusionTensorDisplayPropertiesNode* GetFiberLineDTDisplayPropertiesNode ( );

  // Description:
  // Set diffusion tensor display MRML object for fiber line.
  void SetAndObserveFiberLineDTDisplayPropertiesNodeID ( const char *ID );

  // Description:
  // Get ID of diffusion tensor display MRML object for fiber line.
  vtkGetStringMacro(FiberLineDTDisplayPropertiesNodeID);

  // TO DO: Add updating of reference IDs

 protected:
  vtkMRMLFiberBundleDisplayNode ( );
  ~vtkMRMLFiberBundleDisplayNode ( );
  vtkMRMLFiberBundleDisplayNode ( const vtkMRMLFiberBundleDisplayNode& );
  void operator= ( const vtkMRMLFiberBundleDisplayNode& );

  // MRML nodes 
  vtkMRMLDiffusionTensorDisplayPropertiesNode *FiberLineDTDisplayPropertiesNode;
  vtkMRMLDiffusionTensorDisplayPropertiesNode *FiberTubeDTDisplayPropertiesNode;
  vtkMRMLDiffusionTensorDisplayPropertiesNode *TensorGlyphDTDisplayPropertiesNode;

  char *FiberLineDTDisplayPropertiesNodeID;
  char *FiberTubeDTDisplayPropertiesNodeID;
  char *TensorGlyphDTDisplayPropertiesNodeID;

  vtkSetReferenceStringMacro(FiberLineDTDisplayPropertiesNodeID);
  vtkSetReferenceStringMacro(FiberTubeDTDisplayPropertiesNodeID);
  vtkSetReferenceStringMacro(TensorGlyphDTDisplayPropertiesNodeID);

  // Numbers
  //double Opacity;
  
  // Enumerated
  int ColorModeForFiberLines;
  int ColorModeForFiberTubes;
  int ColorModeForTensorGlyphs;

  // Booleans
  int FiberLineVisibility;
  int FiberTubeVisibility;
  int TensorGlyphVisibility;

  int TwoDimensionalVisibility;


  // Arrays
  //double ScalarRange[2];

};

#endif
