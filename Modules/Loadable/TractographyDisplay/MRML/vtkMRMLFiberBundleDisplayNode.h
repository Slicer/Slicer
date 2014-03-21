/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLFiberBundleDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

  =========================================================================auto=*/
///  vtkMRMLFiberBundleDisplayNode - MRML node to represent display properties for tractography.
///
/// vtkMRMLFiberBundleDisplayNode nodes store display properties of trajectories
/// from tractography in diffusion MRI data, including color type (by bundle, by fiber,
/// or by scalar invariants), display on/off for tensor glyphs and display of
/// trajectory as a line or tube.
//

#ifndef __vtkMRMLFiberBundleDisplayNode_h
#define __vtkMRMLFiberBundleDisplayNode_h

// MRML includes
#include <vtkMRMLModelDisplayNode.h>

// Tractography includes
#include "vtkSlicerTractographyDisplayModuleMRMLExport.h"

class vtkMRMLDiffusionTensorDisplayPropertiesNode;

class VTK_SLICER_TRACTOGRAPHYDISPLAY_MODULE_MRML_EXPORT vtkMRMLFiberBundleDisplayNode : public vtkMRMLModelDisplayNode
{
 public:
  vtkTypeMacro(vtkMRMLFiberBundleDisplayNode, vtkMRMLModelDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

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
  /// Get node XML tag name (like Volume, FiberBundle)
  virtual const char* GetNodeTagName() = 0;

  ///
  /// Updates this node if it depends on other nodes
  /// when the node is deleted in the scene
  virtual void UpdateReferences();

  ///
  /// Finds the storage node and read the data
  virtual void UpdateScene(vtkMRMLScene *scene);

  ///
  /// Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  //--------------------------------------------------------------------------
  /// Display Information: Geometry to display (not mutually exclusive)
  //--------------------------------------------------------------------------

  //--------------------------------------------------------------------------
  /// Display Information: Color Mode
  /// 0) solid color by group 1) color by scalar invariant
  /// 2) color by avg scalar invariant 3) color by other
  //--------------------------------------------------------------------------

  enum
  {
    colorModeSolid = 0,
    colorModeScalar = 1,
    colorModeFunctionOfScalar = 2,
    colorModeUseCellScalars = 3,
    colorModeScalarData = 4,
    colorModePointFiberOrientation = 5,
    colorModeMeanFiberOrientation = 6,
  };

  //--------------------------------------------------------------------------
  /// Display Information: ColorMode for ALL nodes
  //--------------------------------------------------------------------------

 /// Description:
  /// Color mode for glyphs. The color modes are mutually exclusive.
  vtkGetMacro ( ColorMode, int );
  vtkSetMacro ( ColorMode, int );

  ///
  /// Color by solid color (for example the whole fiber bundle red. blue, etc.)
  void SetColorModeToSolid ( ) {
    this->SetColorMode ( this->colorModeSolid );
  };

  ///
  /// Color according to the tensors using various scalar invariants.
  void SetColorModeToScalar ( ) {
    this->SetColorMode ( this->colorModeScalar );
  };

  ///
  /// Color according to the tensors using a function of scalar invariants along the tract.
  /// This enables coloring by average FA, for example.
  void SetColorModeToFunctionOfScalar ( ) {
    this->SetColorMode ( this->colorModeFunctionOfScalar );
  };

  ///
  /// Use to color by the active cell scalars.  This is intended to support
  /// external processing of fibers, for example to label each with the distance
  /// of that fiber from an fMRI activation.  Then by making that information
  /// the active cell scalar field, this will allow coloring by that information.
  /// TO DO: make sure this information can be saved with the tract, save name of
  /// active scalar field if needed.
  void SetColorModeToUseCellScalars ( ) {
    this->SetColorMode ( this->colorModeUseCellScalars );
  };

  ///
  /// Color according to the tensors using scalars (not calculated from tensors)
  /// from the original fiber bundle node.
  void SetColorModeToScalarData ( ) {
    this->SetColorMode ( this->colorModeScalarData );
  };

  ///
  /// Color according to the mean orientation of the fiber
  /// calculated from the orientation of the segment original fiber bundle node.
  void SetColorModeToMeanFiberOrientation ( ) {
    this->SetColorMode ( this->colorModeMeanFiberOrientation );
  };

  ///
  /// Color according to the segment orientation of the fiber
  /// calculated from the orientation of the segment original fiber bundle node.
  void SetColorModeToPointFiberOrientation ( ) {
    this->SetColorMode ( this->colorModePointFiberOrientation );
  };

  /// Set the active tensor name of the display node.
  //virtual void SetActiveTensorName(const char *tensorName);

  /// Return the name of the currently active scalar field for this model.
  vtkGetStringMacro(ActiveTensorName);
  vtkSetStringMacro(ActiveTensorName);

  //--------------------------------------------------------------------------
  /// Display Information: ColorMode for glyphs
  //--------------------------------------------------------------------------

  //--------------------------------------------------------------------------
  /// MRML nodes that are observed
  //--------------------------------------------------------------------------

  /// Node reference to ALL DT nodes

  ///
  /// Get diffusion tensor display MRML object for fiber glyph.
  vtkMRMLDiffusionTensorDisplayPropertiesNode* GetDiffusionTensorDisplayPropertiesNode ( );

  ///
  /// Set diffusion tensor display MRML object for fiber glyph.
  void SetAndObserveDiffusionTensorDisplayPropertiesNodeID ( const char *ID );

  ///
  /// Get ID of diffusion tensor display MRML object for fiber glyph.
  vtkGetStringMacro(DiffusionTensorDisplayPropertiesNodeID);

  static int GetNumberOfScalarInvariants();
  static int GetNthScalarInvariant(int i);

  /// Update the pipeline based on this node attributes
  virtual void UpdatePolyDataPipeline();

 protected:
  vtkMRMLFiberBundleDisplayNode ( );
  ~vtkMRMLFiberBundleDisplayNode ( );
  vtkMRMLFiberBundleDisplayNode ( const vtkMRMLFiberBundleDisplayNode& );
  void operator= ( const vtkMRMLFiberBundleDisplayNode& );

  /// ALL MRML nodes
  vtkMRMLDiffusionTensorDisplayPropertiesNode *DiffusionTensorDisplayPropertiesNode;
  char *DiffusionTensorDisplayPropertiesNodeID;

  virtual void SetDiffusionTensorDisplayPropertiesNodeID(const char* id);

  /// ALL MRML nodes
  static std::vector<int> GetSupportedColorModes();

  /// Enumerated
  int ColorMode;

  /// Active Tensor Name
  char *ActiveTensorName;

  /// Arrays
  //double ScalarRange[2];
  //
};

#endif
