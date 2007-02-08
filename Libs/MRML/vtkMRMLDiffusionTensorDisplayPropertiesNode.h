/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLDiffusionTensorVolumeColorNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

  =========================================================================auto=*/
// .NAME vtkMRMLDiffusionTensorDisplayPropertiesNode - MRML node for display of a diffusion tensor.
// .SECTION Description
// This node describes display properties at the (conceptual) single-tensor level.
// A tensor can be displayed using various scalar invariants and glyphs.
// This class is used by classes (vtkMRMLFiberBundleDisplayNode, 
// vtkMRMLDiffusionTensorVolumeDisplayNode) that handle higher-level display
// concepts for many diffusion tensors, such as choosing between scalars/glyphs/etc. 
// for specific display needs.
// This class inherits from the vtkMRMLColorNode->vtkMRMLColorTableNode superclasses,
// used for vtkMRMLModelNodes and vtkMRMLVolumeNodes, in order to
// provide specific lookup tables for the scalar invariant display.
//

#ifndef __vtkMRMLDiffusionTensorDisplayPropertiesNode_h
#define __vtkMRMLDiffusionTensorDisplayPropertiesNode_h

#include "vtkMRML.h"
#include "vtkMRMLColorTableNode.h"


class VTK_MRML_EXPORT vtkMRMLDiffusionTensorDisplayPropertiesNode : public vtkMRMLColorTableNode
{
 public:
  static vtkMRMLDiffusionTensorDisplayPropertiesNode *New();
  vtkTypeMacro(vtkMRMLDiffusionTensorDisplayPropertiesNode,vtkMRMLColorTableNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Read node attributes from a MRML file in XML format.
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object.
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "DiffusionTensorDisplayProperties";};

  //--------------------------------------------------------------------------
  // Display Information: Types of scalars that may be generated from tensors.
  //--------------------------------------------------------------------------

  //BTX
  enum
  {
    Trace = 0,
    Determinant = 1,
    RelativeAnisotropy = 2,
    FractionalAnisotropy = 3,
    MaxEigenvalue = 4,
    MidEigenvalue = 5,
    MinEigenvalue = 6,
    LinearMeasure = 7,
    PlanarMeasure = 8,
    SphericalMeasure = 9,
    ColorOrientation = 10,
    D11 = 11,
    D22 = 12,
    D33 = 13,
    Mode = 14,
    ColorMode = 15,
    MaxEigenvalueProjX = 16,
    MaxEigenvalueProjY = 17,
    MaxEigenvalueProjZ = 18,
    MaxEigenvec_ProjX =  19,
    MaxEigenvec_ProjY =  20,
    MaxEigenvec_ProjZ =  21,
    ParallelDiffusivity =  22,
    PerpendicularDiffusivity =  23,
    ColorOrientationMiddleEigenvector = 24,
    ColorOrientationMinEigenvector = 25,
  };
  //ETX

  //--------------------------------------------------------------------------
  // Display Information: Functions to choose scalar invariant
  //--------------------------------------------------------------------------

  // Description:
  // Get type of scalar invariant (tensor-derived scalar, invariant to tensor 
  // rotation) selected for display.
  vtkGetMacro(ScalarInvariant, int);

  // Description:
  // Get type of scalar invariant (tensor-derived scalar, invariant to tensor 
  // rotation) selected for display.
  vtkSetMacro(ScalarInvariant, int);
 
  // Description:
  // Set scalar invariant to trace (sum of eigenvalues).
  void SetScalarInvariantToTrace() {
    this->SetScalarInvariant(this->Trace);
  };

  // Description:
  // Set scalar invariant to FA (normalized variance of eigenvalues)
  void SetScalarInvariantToFractionalAnisotropy() {
    this->SetScalarInvariant(this->FractionalAnisotropy);
  };  

  // TO DO: add the rest of the scalars
  //and so on.

  //--------------------------------------------------------------------------
  // Display Information: Types of glyph geometry that can be displayed
  //--------------------------------------------------------------------------

  //BTX
  enum
  {
    Lines = 1,
    Tubes = 2,
    Ellipsoids = 3,
    Superquadrics = 4,
  };
  //ETX

  //--------------------------------------------------------------------------
  // Display Information: Functions to choose the type of glyph geometry
  //--------------------------------------------------------------------------

  // Description:
  // Get the type of glyph geometry (line, ellipsoid, etc.) 
  vtkGetMacro(GlyphGeometry, int);

  // Description:
  // Set the type of glyph geometry (line, ellipsoid, etc.) 
  vtkSetMacro(GlyphGeometry, int);

  void SetGlyphGeometryToLines() {
    this->SetGlyphGeometry(this->Lines);
  };
  void SetGlyphGeometryToTubes() {
    this->SetGlyphGeometry(this->Tubes);
  };
  void SetGlyphGeometryToEllipsoids() {
    this->SetGlyphGeometry(this->Ellipsoids);
  };
  void SetGlyphGeometryToSuperquadrics() {
    this->SetGlyphGeometry(this->Superquadrics);
  };

  //--------------------------------------------------------------------------
  // Display Information: Parameters of glyph geometry
  //--------------------------------------------------------------------------

  // Description:
  // Get the scale factor applied to the glyphs.
  vtkGetMacro(GlyphScaleFactor, double);

  // Description:
  // Set the scale factor applied to the glyphs.
  vtkSetMacro(GlyphScaleFactor, double);

  //--------------------------------------------------------------------------
  // Display Information: Parameters of Lines glyph geometry
  //--------------------------------------------------------------------------

  //--------------------------------------------------------------------------
  // Display Information: Parameters of Tubes glyph geometry
  //--------------------------------------------------------------------------

  // Description:
  // Get the radius of the tube glyph
  vtkGetMacro(TubeGlyphRadius, double);

  // Description:
  // Set the radius of the tube glyph
  vtkSetMacro(TubeGlyphRadius, double);


  //--------------------------------------------------------------------------
  // Display Information: Parameters of Ellipsoids glyph geometry
  //--------------------------------------------------------------------------

  //--------------------------------------------------------------------------
  // Display Information: Parameters of Superquadrics glyph geometry
  //--------------------------------------------------------------------------



  //--------------------------------------------------------------------------
  // Display Information: Functions to choose the type of glyph coloring
  //--------------------------------------------------------------------------

  // Description:
  // Get type of scalar invariant (tensor-derived scalar, invariant to tensor 
  // rotation) selected for display.
  vtkGetMacro(ColorGlyphBy, int);

  // Description:
  // Get type of scalar invariant (tensor-derived scalar, invariant to tensor 
  // rotation) selected for display.
  vtkSetMacro(ColorGlyphBy, int);
 
  // Description:
  // Set scalar invariant to trace (sum of eigenvalues).
  void ColorGlyphByTrace() {
    this->SetColorGlyphBy(this->Trace);
  };

  // Description:
  // Set scalar invariant to FA (normalized variance of eigenvalues)
  void ColorGlyphByFractionalAnisotropy() {
    this->SetColorGlyphBy(this->FractionalAnisotropy);
  };  

  // TO DO: add the rest of the scalars
  //and so on.


 protected:
  vtkMRMLDiffusionTensorDisplayPropertiesNode();
  ~vtkMRMLDiffusionTensorDisplayPropertiesNode();
  vtkMRMLDiffusionTensorDisplayPropertiesNode(const vtkMRMLDiffusionTensorDisplayPropertiesNode&);
  void operator=(const vtkMRMLDiffusionTensorDisplayPropertiesNode&);

  int ScalarInvariant;
  int GlyphGeometry;
  int ColorGlyphBy;

  // Glyph general parameters
  double GlyphScaleFactor;

  // Line Glyph parameters

  // Tube Glyph parameters
  double TubeGlyphRadius;

  // Ellipsoid Glyph parameters

  // Superquadric Glyph parameters


  // TO DO: add specific lookup tables ranging from 0..1 for or -1 1
  // for scalar invariants with those ranges

};

#endif

