/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: ,v $
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
#include "vtkPolyData.h"


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

  //Description:
  // Set scalar invariant to relative anisotropy
  void SetScalarInvariantToRelativeAnisotropy() {
    this->SetScalarInvariant(this->RelativeAnisotropy);
  };

  // Description:
  // Set scalar invariant to FA (normalized variance of eigenvalues)
  void SetScalarInvariantToFractionalAnisotropy() {
    this->SetScalarInvariant(this->FractionalAnisotropy);
  };

  // Description:
  // Set scalar invariant to C_L (Westin's linear measure)
  void SetScalarInvariantToLinearMeasure() {
    this->SetScalarInvariant(this->LinearMeasure);
  };

  // Description:
  // Set scalar invariant to C_P (Westin's planar measure)
  void SetScalarInvariantToPlanarMeasure() {
    this->SetScalarInvariant(this->PlanarMeasure);
  };

  // Description:
  // Set scalar invariant to C_S (Westin's spherical measure)
  void SetScalarInvariantToSphericalMeasure() {
    this->SetScalarInvariant(this->SphericalMeasure);
  }

  // TO DO: add the rest of the scalars

  // Description:
  // Return the lowest and highest integers, for use in looping
  int GetFirstScalarInvariant() {return this->Trace;};
  int GetLastScalarInvariant() {return this->ColorOrientationMinEigenvector;};

  // Description:
  // Return a text string describing the ScalarInvariant variable
  virtual const char * GetScalarInvariantAsString();


  //--------------------------------------------------------------------------
  // Display Information: Types of glyph geometry that can be displayed
  //--------------------------------------------------------------------------

  //BTX
  enum
  {
    Lines = 0,
    Tubes = 1,
    Ellipsoids = 2,
    Superquadrics = 3,
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
  //vtkSetMacro(GlyphGeometry, int);
  // Also update the glyph polydata source
  void SetGlyphGeometry( int geometry ) {

    if ( this->GlyphGeometry != geometry ) 
      {
      this->GlyphGeometry = geometry;
      this->UpdateGlyphSource();
      this->Modified();
      }
  }

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


  // Description:
  // Return the lowest and highest integers, for use in looping
  int GetFirstGlyphGeometry() {return this->Lines;};
  int GetLastGlyphGeometry() {return this->Superquadrics;};

  // Description:
  // Return a text string describing the GlyphGeometry variable
  virtual const char * GetGlyphGeometryAsString();

  //--------------------------------------------------------------------------
  // Display Information: Parameters of glyph geometry
  //--------------------------------------------------------------------------

  // Description:
  // Get the scale factor applied to the glyphs.
  vtkGetMacro(GlyphScaleFactor, double);

  // Description:
  // Set the scale factor applied to the glyphs.
  vtkSetMacro(GlyphScaleFactor, double);

  // Description:
  // Whether the input tensors need eigensystem computation
  vtkGetMacro(GlyphExtractEigenvalues, int);
  vtkSetMacro(GlyphExtractEigenvalues, int);
  vtkBooleanMacro(GlyphExtractEigenvalues, int);

  //--------------------------------------------------------------------------
  // Display Information: Eigenvector to display for lines/tubes glyphs
  //--------------------------------------------------------------------------

  //BTX
  enum
  {
    Major = 1,
    Middle = 2,
    Minor = 3,
  };
  //ETX

  // Description
  // Which eigenvector to display with lines or tubes glyphs
  vtkGetMacro(GlyphEigenvector, int);

  // Description
  // Which eigenvector to display with lines or tubes glyphs
  //vtkSetMacro(GlyphEigenvector, int);
    // Description:
  // Set the type of glyph geometry (line, ellipsoid, etc.) 
  //vtkSetMacro(GlyphGeometry, int);
  // Also update the glyph polydata source
  void SetGlyphEigenvector( int eigenvector ) {

    if ( this->GlyphEigenvector != eigenvector ) 
      {
      this->GlyphEigenvector = eigenvector;
      if ( this->GlyphGeometry == this->Lines || this->GlyphGeometry == this->Tubes)
        {
        // Update the source if the eigenvector has changed
        this->UpdateGlyphSource();
        }

      this->Modified();
      }
  }

  // Description
  // Display major eigenvector with lines or tubes glyphs
  void SetGlyphEigenvectorToMajor() {
    this->SetGlyphEigenvector(this->Major);
  };

  // Description
  // Display "middle" (second) eigenvector with lines or tubes glyphs
  void SetGlyphEigenvectorToMiddle() {
    this->SetGlyphEigenvector(this->Middle);
  };

  // Description
  // Display minor eigenvector with lines or tubes glyphs
  void SetGlyphEigenvectorToMinor() {
    this->SetGlyphEigenvector(this->Minor);
  };

  // Description:
  // Return the lowest and highest integers, for use in looping
  int GetFirstGlyphEigenvector() {return this->Major;};
  int GetLastGlyphEigenvector() {return this->Minor;};

  // Description:
  // Return a text string describing GlyphEigenvector variable
  virtual const char * GetGlyphEigenvectorAsString();

  
  //--------------------------------------------------------------------------
  // Display Information: Parameters of Lines glyph geometry
  //--------------------------------------------------------------------------

  // Description:
  // Resolution of lines displayed as tensor glyphs
  vtkGetMacro(LineGlyphResolution, int);
  //vtkSetMacro(LineGlyphResolution, int);
  void SetLineGlyphResolution( int resolution ) {

    if ( this->LineGlyphResolution != resolution ) 
      {
      this->LineGlyphResolution = resolution;
      if ( this->GlyphGeometry == this->Lines || this->GlyphGeometry == this->Tubes)
        {
        // Update the source if the resolution has changed
        this->UpdateGlyphSource();
        }

      this->Modified();
      }
  }

  //--------------------------------------------------------------------------
  // Display Information: Parameters of Tubes glyph geometry
  //--------------------------------------------------------------------------

  // Description:
  // Get the radius of the tube glyph
  vtkGetMacro(TubeGlyphRadius, double);

  // Description:
  // Set the radius of the tube glyph
  //vtkSetMacro(TubeGlyphRadius, double);
  void SetTubeGlyphRadius( double radius ) {

    if ( this->TubeGlyphRadius != radius ) 
      {
      this->TubeGlyphRadius = radius;
      if ( this->GlyphGeometry == this->Lines || this->GlyphGeometry == this->Tubes)
        {
        // Update the source if the radius has changed
        this->UpdateGlyphSource();
        }

      this->Modified();
      }
  }

  // Description:
  // Number of sides of tube glyph (3 gives a triangular tube, etc.)
  vtkGetMacro(TubeGlyphNumberOfSides, int);
  //vtkSetMacro(TubeGlyphNumberOfSides, int);
  void SetTubeGlyphNumberOfSides( int numberOfSides ) {

    if ( this->TubeGlyphNumberOfSides != numberOfSides ) 
      {
      this->TubeGlyphNumberOfSides = numberOfSides;
      if ( this->GlyphGeometry == this->Lines || this->GlyphGeometry == this->Tubes)
        {
        // Update the source if the numberOfSides has changed
        this->UpdateGlyphSource();
        }

      this->Modified();
      }
  }
  //--------------------------------------------------------------------------
  // Display Information: Parameters of Ellipsoids glyph geometry
  //--------------------------------------------------------------------------

  // Description:
  // Number of polygons used in longitude direction for sphere that will
  // be scaled by tensor to form ellipsoid.
  vtkGetMacro(EllipsoidGlyphThetaResolution, int);
  vtkSetMacro(EllipsoidGlyphThetaResolution, int);

  // Description:
  // Number of polygons used in latitude direction for sphere that will
  // be scaled by tensor to form ellipsoid.
  vtkGetMacro(EllipsoidGlyphPhiResolution, int);
  vtkSetMacro(EllipsoidGlyphPhiResolution, int);

  //--------------------------------------------------------------------------
  // Display Information: Parameters of Superquadrics glyph geometry
  //--------------------------------------------------------------------------

  // Description:
  vtkGetMacro(SuperquadricGlyphGamma, double);
  vtkSetMacro(SuperquadricGlyphGamma, double);

  // Description:
  vtkGetMacro(SuperquadricGlyphThetaResolution, int);
  vtkSetMacro(SuperquadricGlyphThetaResolution, int);

  // Description:
  vtkGetMacro(SuperquadricGlyphPhiResolution, int);
  vtkSetMacro(SuperquadricGlyphPhiResolution, int);

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
  // Return the lowest and highest integers, for use in looping
  int GetFirstColorGlyphBy() {return this->Trace;};
  int GetLastColorGlyphBy() {return this->ColorOrientationMinEigenvector;};
  
  // Description:
  // Return a text string describing the ColorGlyphBy
  virtual const char * GetColorGlyphByAsString();
 
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

  // Description:
  // Set scalar invariant to FA (normalized variance of eigenvalues)
  void ColorGlyphByLinearMeasure() {
    this->SetColorGlyphBy(this->LinearMeasure);
  };  

  // TO DO: add the rest of the scalars

  //--------------------------------------------------------------------------
  // Convenience functions to get an appropriate glyph source
  //--------------------------------------------------------------------------

  // Description:
  // Get a polydata object according to current glyph display settings
  // (so a line, sphere, or tube) to use as a source for a glyphing filter.
  vtkGetObjectMacro( GlyphSource, vtkPolyData );

 protected:
  vtkMRMLDiffusionTensorDisplayPropertiesNode();
  ~vtkMRMLDiffusionTensorDisplayPropertiesNode();
  vtkMRMLDiffusionTensorDisplayPropertiesNode(const vtkMRMLDiffusionTensorDisplayPropertiesNode&);
  void operator=(const vtkMRMLDiffusionTensorDisplayPropertiesNode&);

  void UpdateGlyphSource ( );

  // ---- Parameters that should be written to MRML --- //

  // Scalar display parameters
  int ScalarInvariant;

  // Glyph general parameters
  int GlyphGeometry;
  int ColorGlyphBy;
  double GlyphScaleFactor;
  int GlyphEigenvector;
  int GlyphExtractEigenvalues;

  // Line Glyph parameters
  int LineGlyphResolution;

  // Tube Glyph parameters
  double TubeGlyphRadius;
  int TubeGlyphNumberOfSides;

  // Ellipsoid Glyph parameters
  int EllipsoidGlyphThetaResolution;
  int EllipsoidGlyphPhiResolution;

  // Superquadric Glyph parameters
  double SuperquadricGlyphGamma;
  int SuperquadricGlyphThetaResolution;
  int SuperquadricGlyphPhiResolution;

  // ---- End of parameters that should be written to MRML --- //

 
  // ---- VTK objects for display --- //
  vtkPolyData * GlyphSource;

  // This is used internally to set a pointer to this polydata
  // and reference count it.  
  // TO DO: is this causing an extra modified event?
  vtkSetObjectMacro( GlyphSource, vtkPolyData );

  //Helper function to get the string of Scalar enums
  virtual const char *GetScalarEnumAsString(int val);

  // TO DO: add specific lookup tables ranging from 0..1 for or -1 1
  // for scalar invariants with those ranges

  // TO DO: read/write MRML for all parameters

};

#endif

