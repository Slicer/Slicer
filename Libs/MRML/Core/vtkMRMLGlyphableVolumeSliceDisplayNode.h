/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLGlyphableVolumeSliceDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

  =========================================================================auto=*/

#ifndef __vtkMRMLGlyphableVolumeSliceDisplayNode_h
#define __vtkMRMLGlyphableVolumeSliceDisplayNode_h

#include <string>

#include "vtkMRML.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkPolyData.h"

class vtkTransform;
class vtkTransformPolyDataFilter;
class vtkMatrix4x4;
class vtkImageData;

/// \brief MRML node to represent display properties for tractography.
///
/// vtkMRMLGlyphableVolumeSliceDisplayNode nodes store display properties of trajectories
/// from tractography in diffusion MRI data, including color type (by bundle, by fiber,
/// or by scalar invariants), display on/off for tensor glyphs and display of
/// trajectory as a line or tube.
class VTK_MRML_EXPORT vtkMRMLGlyphableVolumeSliceDisplayNode : public vtkMRMLModelDisplayNode
{
 public:
  static vtkMRMLGlyphableVolumeSliceDisplayNode *New (  );
  vtkTypeMacro ( vtkMRMLGlyphableVolumeSliceDisplayNode,vtkMRMLModelDisplayNode );
  void PrintSelf ( ostream& os, vtkIndent indent ) override;

  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance () override;

  ///
  /// Read node attributes from XML (MRML) file
  void ReadXMLAttributes ( const char** atts ) override;

  ///
  /// Write this node's information to a MRML file in XML format.
  void WriteXML ( ostream& of, int indent ) override;


  ///
  /// Copy the node's attributes to this object
  void Copy ( vtkMRMLNode *node ) override;

  ///
  /// Get node XML tag name (like Volume, UnstructuredGrid)
  const char* GetNodeTagName () override {return "GlyphableVolumeSliceDisplayNode";}

  ///
  /// Updates this node if it depends on other nodes
  /// when the node is deleted in the scene
  void UpdateReferences() override;

  void UpdateReferenceID(const char *oldID, const char *newID) override
    { Superclass::UpdateReferenceID(oldID, newID); }

  ///
  /// Finds the storage node and read the data
  void UpdateScene(vtkMRMLScene *scene) override;


  ///
  /// alternative method to propagate events generated in Display nodes
  void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ ) override;


  /// Return the output of the glyph producer for the input image data.
  /// The output is connected as the input of the slice transform.
  /// It must be reimplemented in subclasses.
  /// \sa GetOutputPolyData(), GetSliceOutputPort()
  vtkAlgorithmOutput* GetOutputMeshConnection() override;

  /// Return the glyph polydata for the input slice image.
  /// This is the polydata to use in a 3D view.
  /// Reimplemented to by-pass the check on the input polydata.
  /// \sa GetSliceOutputPolyData(), GetOutputPolyDataConnection()
  vtkPolyData* GetOutputMesh() override;

  /// Return the glyph polyData transformed to slice XY.
  /// This is the polydata to use in a 2D slice.
  /// \sa GetOutputPolyData(), GetSliceOutputPort()
  virtual vtkPolyData* GetSliceOutputPolyData();

  ///
  /// Update the pipeline based on this node attributes
  void UpdateAssignedAttribute() override;

  ///
  /// Set imageData of a volume slice. This is used as the input of the display
  /// pipeline instead of SetInputPolyData().
  /// \sa GetOutputPolyData(), SetInputPolyData()
  virtual void SetSliceImagePort(vtkAlgorithmOutput *imagePort);
  vtkGetObjectMacro(SliceImagePort, vtkAlgorithmOutput);
  /// Return the glyph output transformed to slice XY.
  /// Return the output of the glyph producer for the entire volume.
  /// \sa GetSliceOutputPolyData(), GetOutputPolyDataConnection()
  virtual vtkAlgorithmOutput* GetSliceOutputPort();

  ///
  /// Set slice to RAS transformation
  virtual void SetSlicePositionMatrix(vtkMatrix4x4 *matrix);

  ///
  /// Set slice to IJK transformation
  virtual void SetSliceGlyphRotationMatrix(vtkMatrix4x4 *matrix);

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
    colorModeUseCellScalars = 3
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



  //--------------------------------------------------------------------------
  /// Display Information: ColorMode for glyphs
  //--------------------------------------------------------------------------


  //--------------------------------------------------------------------------
  /// MRML nodes that are observed
  //--------------------------------------------------------------------------
 protected:
  vtkMRMLGlyphableVolumeSliceDisplayNode ( );
  ~vtkMRMLGlyphableVolumeSliceDisplayNode ( ) override;
  vtkMRMLGlyphableVolumeSliceDisplayNode ( const vtkMRMLGlyphableVolumeSliceDisplayNode& );
  void operator= ( const vtkMRMLGlyphableVolumeSliceDisplayNode& );

  /// Ignore input polydata as it takes a volume slice as input.
  virtual void SetInputToPolyDataPipeline(vtkAlgorithmOutput* glyphPolyData);

    vtkAlgorithmOutput       *SliceImagePort;
    vtkTransform             *SliceToXYTransform;
    vtkTransformPolyDataFilter *SliceToXYTransformer;
    vtkMatrix4x4             *SliceToXYMatrix;


    /// Enumerated
    int ColorMode;


};

#endif
