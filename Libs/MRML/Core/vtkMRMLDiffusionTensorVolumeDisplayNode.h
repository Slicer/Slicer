/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLDiffusionTensorVolumeDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkMRMLDiffusionTensorVolumeDisplayNode_h
#define __vtkMRMLDiffusionTensorVolumeDisplayNode_h

#include "vtkMRMLGlyphableVolumeDisplayNode.h"
#include "vtkMRMLDiffusionTensorDisplayPropertiesNode.h"
class vtkMRMLGlyphableVolumeSliceDisplayNode;

class vtkAlgorithmOutput;
class vtkDiffusionTensorMathematics;
class vtkDiffusionTensorGlyph;
class vtkImageCast;
class vtkImageData;
class vtkImageExtractComponents;
class vtkImageShiftScale;
class vtkImageMathematics;

/// \brief MRML node for representing a volume (image stack).
///
/// Volume nodes describe data sets that can be thought of as stacks of 2D
/// images that form a 3D volume.  Volume nodes describe where the images
/// are stored on disk, how to render the data (window and level), and how
/// to read the files.  This information is extracted from the image
/// headers (if they exist) at the time the MRML file is generated.
/// Consequently, MRML files isolate MRML browsers from understanding how
/// to read the myriad of file formats for medical data.
class VTK_MRML_EXPORT vtkMRMLDiffusionTensorVolumeDisplayNode : public vtkMRMLGlyphableVolumeDisplayNode
{
  public:
  static vtkMRMLDiffusionTensorVolumeDisplayNode *New();
  vtkTypeMacro(vtkMRMLDiffusionTensorVolumeDisplayNode,vtkMRMLGlyphableVolumeDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Set node attributes
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  ///
  /// Copy the node's attributes to this object
  void Copy(vtkMRMLNode *node) override;

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "DiffusionTensorVolumeDisplay";}

  //virtual vtkPolyData* ExecuteGlyphPipeLineAndGetPolyData( vtkImageData* );

  ///
  /// Updates this node if it depends on other nodes
  /// when the node is deleted in the scene
  void UpdateReferences() override;

  ///
  /// Finds the storage node and read the data
  void UpdateScene(vtkMRMLScene *scene) override;

  ///
  /// Update the stored reference to another node in the scene
  void UpdateReferenceID(const char *oldID, const char *newID) override;

  ///
  /// alternative method to propagate events generated in Display nodes
  void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ ) override;

  //--------------------------------------------------------------------------
  /// Display Information
  //--------------------------------------------------------------------------

  //--------------------------------------------------------------------------
  /// MRML nodes that are observed
  //--------------------------------------------------------------------------

  ///
  /// Get type of scalar invariant (tensor-derived scalar, invariant to tensor
  /// rotation) selected for display.
  vtkGetMacro(ScalarInvariant, int);

  ///
  /// Get type of scalar invariant (tensor-derived scalar, invariant to tensor
  /// rotation) selected for display.
  vtkSetMacro(ScalarInvariant, int);

  ///
  /// Set scalar invariant to trace (sum of eigenvalues).
  void SetScalarInvariantToTrace() {
    this->SetScalarInvariant(vtkMRMLDiffusionTensorDisplayPropertiesNode::Trace);
  };

  //Description:
  /// Set scalar invariant to relative anisotropy
  void SetScalarInvariantToRelativeAnisotropy() {
    this->SetScalarInvariant(vtkMRMLDiffusionTensorDisplayPropertiesNode::RelativeAnisotropy);
  };

  ///
  /// Set scalar invariant to FA (normalized variance of eigenvalues)
  void SetScalarInvariantToFractionalAnisotropy() {
    this->SetScalarInvariant(vtkMRMLDiffusionTensorDisplayPropertiesNode::FractionalAnisotropy);
  };

  ///
  /// Set scalar invariant to C_L (Westin's linear measure)
  void SetScalarInvariantToLinearMeasure() {
    this->SetScalarInvariant(vtkMRMLDiffusionTensorDisplayPropertiesNode::LinearMeasure);
  };

  ///
  /// Set scalar invariant to C_P (Westin's planar measure)
  void SetScalarInvariantToPlanarMeasure() {
    this->SetScalarInvariant(vtkMRMLDiffusionTensorDisplayPropertiesNode::PlanarMeasure);
  };

  ///
  /// Set scalar invariant to C_S (Westin's spherical measure)
  void SetScalarInvariantToSphericalMeasure() {
    this->SetScalarInvariant(vtkMRMLDiffusionTensorDisplayPropertiesNode::SphericalMeasure);
  };

  ///
  /// Return a text string describing the ScalarInvariant variable
  virtual const char * GetScalarInvariantAsString()
    {
    return vtkMRMLDiffusionTensorDisplayPropertiesNode::GetScalarEnumAsString(this->ScalarInvariant);
    };

  /// Get the input of the pipeline
  vtkAlgorithmOutput* GetInputImageDataConnection() override;

  ///
  /// Get background mask stencil
  /// Reimplemented to return 0 when the background mask is not used.
  vtkAlgorithmOutput* GetBackgroundImageStencilDataConnection() override;

  void UpdateImageDataPipeline() override;

  vtkGetObjectMacro(DTIMathematics, vtkDiffusionTensorMathematics);
  vtkGetObjectMacro(DTIMathematicsAlpha, vtkDiffusionTensorMathematics);
  vtkGetObjectMacro (ShiftScale, vtkImageShiftScale);


  ///
  /// get associated slice glyph display node or nullptr if not set
  std::vector< vtkMRMLGlyphableVolumeSliceDisplayNode*> GetSliceGlyphDisplayNodes( vtkMRMLVolumeNode* node ) override;


  ///
  /// add slice glyph display nodes if not already present and return it
  void  AddSliceGlyphDisplayNodes( vtkMRMLVolumeNode* node ) override;

  ///
  /// Defines the expected range of the output data for given imageData after
  /// having been mapped through the current display options
  void GetDisplayScalarRange(double range[2]) override;

  static int GetNumberOfScalarInvariants();
  static int GetNthScalarInvariant(int i);

protected:
  vtkMRMLDiffusionTensorVolumeDisplayNode();
  ~vtkMRMLDiffusionTensorVolumeDisplayNode() override;
  vtkMRMLDiffusionTensorVolumeDisplayNode(const vtkMRMLDiffusionTensorVolumeDisplayNode&);
  void operator=(const vtkMRMLDiffusionTensorVolumeDisplayNode&);

  /// Set the input of the pipeline
  void SetInputToImageDataPipeline(vtkAlgorithmOutput *imageDataConnection) override;

  vtkAlgorithmOutput* GetScalarImageDataConnection() override;

  static std::vector<int> GetSupportedColorModes();

  vtkDiffusionTensorGlyph* DiffusionTensorGlyphFilter;

  /// used for main scalar invarant (can be 1 or 3 component)
  vtkDiffusionTensorMathematics *DTIMathematics;
  /// used for calculating single component magnitude for color images
  vtkDiffusionTensorMathematics *DTIMathematicsAlpha;

  vtkImageShiftScale *ShiftScale;

  vtkImageMathematics *ImageMath;

  vtkImageCast *ImageCast;

   /// Scalar display parameters
  int ScalarInvariant;


};

#endif

