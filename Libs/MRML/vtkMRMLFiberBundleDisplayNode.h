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
  vtkSetMacro ( FiberGlyphVisibility , int );
  vtkGetMacro ( FiberGlyphVisibility , int );
  vtkBooleanMacro ( FiberGlyphVisibility , int );

    // Description:
  // Turn on/off visibility of glyphs (tensors) along fibers.
  vtkSetMacro ( TwoDimensionalVisibility , int );
  vtkGetMacro ( TwoDimensionalVisibility , int );
  vtkBooleanMacro ( TwoDimensionalVisibility , int );

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
  // Display Information: ColorMode for lines
  //--------------------------------------------------------------------------
  // Description:
  // Color mode for lines. The color modes are mutually exclusive.
  vtkGetMacro ( ColorModeForFiberLines, int );
  vtkSetMacro ( ColorModeForFiberLines, int );
 
  // Description:
  // Color by solid color (for example the whole fiber bundle red. blue, etc.)
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
  // Display Information: line properties
  //--------------------------------------------------------------------------

  // Description:
  // Opacity of line trajectory
  vtkSetMacro ( FiberLineOpacity , double );
  vtkGetMacro ( FiberLineOpacity , double );

  //--------------------------------------------------------------------------
  // Display Information: ColorMode for tubes
  //--------------------------------------------------------------------------

  // Description:
  // Color mode for tubes. The color modes are mutually exclusive.
  vtkGetMacro ( ColorModeForFiberTubes, int );
  vtkSetMacro ( ColorModeForFiberTubes, int );
 
  // Description:
  // Color by solid color (for example the whole fiber bundle red. blue, etc.)
  void SetColorModeForFiberTubesToSolid ( ) {
    this->SetColorModeForFiberTubes ( this->colorModeSolid );
  };

  // Description:
  // Color according to the tensors using various scalar invariants.
  void SetColorModeForFiberTubesToScalar ( ) {
    this->SetColorModeForFiberTubes ( this->colorModeScalar );
  };

  // Description:
  // Color according to the tensors using a function of scalar invariants along the tract.
  // This enables coloring by average FA, for example.
  void SetColorModeForFiberTubesToFunctionOfScalar ( ) {
    this->SetColorModeForFiberTubes ( this->colorModeFunctionOfScalar );
  };

  // Description:
  // Use to color by the active cell scalars.  This is intended to support
  // external processing of fibers, for example to label each with the distance
  // of that fiber from an fMRI activation.  Then by making that information
  // the active cell scalar field, this will allow coloring by that information.
  // TO DO: make sure this information can be saved with the tract, save name of
  // active scalar field if needed.
  void SetColorModeForFiberTubesToUseCellScalars ( ) {
    this->SetColorModeForFiberTubes ( this->colorModeUseCellScalars );
  };


  //--------------------------------------------------------------------------
  // Display Information: tube properties
  //--------------------------------------------------------------------------

  // Description:
  // Opacity of tubed trajectory
  vtkSetMacro ( FiberTubeOpacity , double );
  vtkGetMacro ( FiberTubeOpacity , double );

  // Description:
  // Thickness (radius) of tubed trajectory ("thick fibers") geometry.
  vtkSetMacro ( FiberTubeRadius , double );
  vtkGetMacro ( FiberTubeRadius , double );

  // Description:
  // Resolution (number of sides) of tubed trajectory ("thick fibers") geometry.
  vtkSetMacro ( FiberTubeNumberOfSides , int );
  vtkGetMacro ( FiberTubeNumberOfSides , int );

  //--------------------------------------------------------------------------
  // Display Information: ColorMode for glyphs
  //--------------------------------------------------------------------------
  
  // Description:
  // Color mode for glyphs. The color modes are mutually exclusive.
  vtkGetMacro ( ColorModeForFiberGlyphs, int );
  vtkSetMacro ( ColorModeForFiberGlyphs, int );
 
  // Description:
  // Color by solid color (for example the whole fiber bundle red. blue, etc.)
  void SetColorModeForFiberGlyphsToSolid ( ) {
    this->SetColorModeForFiberGlyphs ( this->colorModeSolid );
  };

  // Description:
  // Color according to the tensors using various scalar invariants.
  void SetColorModeForFiberGlyphsToScalar ( ) {
    this->SetColorModeForFiberGlyphs ( this->colorModeScalar );
  };

  // Description:
  // Color according to the tensors using a function of scalar invariants along the tract.
  // This enables coloring by average FA, for example.
  void SetColorModeForFiberGlyphsToFunctionOfScalar ( ) {
    this->SetColorModeForFiberGlyphs ( this->colorModeFunctionOfScalar );
  };

  // Description:
  // Use to color by the active cell scalars.  This is intended to support
  // external processing of fibers, for example to label each with the distance
  // of that fiber from an fMRI activation.  Then by making that information
  // the active cell scalar field, this will allow coloring by that information.
  // TO DO: make sure this information can be saved with the tract, save name of
  // active scalar field if needed.
  void SetColorModeForFiberGlyphsToUseCellScalars ( ) {
    this->SetColorModeForFiberGlyphs ( this->colorModeUseCellScalars );
  };


  //--------------------------------------------------------------------------
  // Display Information: glyph properties
  //--------------------------------------------------------------------------

  // Description:
  // Opacity of glyphs
  vtkSetMacro ( FiberGlyphOpacity , double );
  vtkGetMacro ( FiberGlyphOpacity , double );


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

  // Description:
  // Get diffusion tensor display MRML object for fiber tube.
  vtkMRMLDiffusionTensorDisplayPropertiesNode* GetFiberTubeDTDisplayPropertiesNode ( );

  // Description:
  // Set diffusion tensor display MRML object for fiber tube.
  void SetAndObserveFiberTubeDTDisplayPropertiesNodeID ( const char *ID );

  // Description:
  // Get ID of diffusion tensor display MRML object for fiber tube.
  vtkGetStringMacro(FiberTubeDTDisplayPropertiesNodeID);

  // Description:
  // Get diffusion tensor display MRML object for fiber glyph.
  vtkMRMLDiffusionTensorDisplayPropertiesNode* GetFiberGlyphDTDisplayPropertiesNode ( );

  // Description:
  // Set diffusion tensor display MRML object for fiber glyph.
  void SetAndObserveFiberGlyphDTDisplayPropertiesNodeID ( const char *ID );

  // Description:
  // Get ID of diffusion tensor display MRML object for fiber glyph.
  vtkGetStringMacro(FiberGlyphDTDisplayPropertiesNodeID);

  // TO DO: Add updating of reference IDs
 
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

  // MRML nodes 
  vtkMRMLDiffusionTensorDisplayPropertiesNode *FiberLineDTDisplayPropertiesNode;
  vtkMRMLDiffusionTensorDisplayPropertiesNode *FiberTubeDTDisplayPropertiesNode;
  vtkMRMLDiffusionTensorDisplayPropertiesNode *FiberGlyphDTDisplayPropertiesNode;

  char *FiberLineDTDisplayPropertiesNodeID;
  char *FiberTubeDTDisplayPropertiesNodeID;
  char *FiberGlyphDTDisplayPropertiesNodeID;

  vtkSetReferenceStringMacro(FiberLineDTDisplayPropertiesNodeID);
  vtkSetReferenceStringMacro(FiberTubeDTDisplayPropertiesNodeID);
  vtkSetReferenceStringMacro(FiberGlyphDTDisplayPropertiesNodeID);


  // ALL MRML nodes
  vtkMRMLDiffusionTensorDisplayPropertiesNode *DTDisplayPropertiesNode;
  char *DTDisplayPropertiesNodeID;

  vtkSetReferenceStringMacro(DTDisplayPropertiesNodeID);



  // Numbers
  double FiberLineOpacity;
  double FiberTubeOpacity;
  double FiberGlyphOpacity;

  double FiberTubeRadius;
  int FiberTubeNumberOfSides;
  
  // Enumerated
  int ColorModeForFiberLines;
  int ColorModeForFiberTubes;
  int ColorModeForFiberGlyphs;

  // Booleans
  int FiberLineVisibility;
  int FiberTubeVisibility;
  int FiberGlyphVisibility;

  int TwoDimensionalVisibility;

  // Enumerated
  int ColorMode;

  // Arrays
  //double ScalarRange[2];

};

#endif
