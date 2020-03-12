/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLDiffusionTensorDisplayPropertiesNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.0 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLDiffusionTensorDisplayPropertiesNode.h"

// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkLineSource.h>
#include <vtkObjectFactory.h>
#include <vtkSphereSource.h>
#include <vtkTubeFilter.h>

// STD includes
#include <sstream>



//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLDiffusionTensorDisplayPropertiesNode);

//----------------------------------------------------------------------------
vtkMRMLDiffusionTensorDisplayPropertiesNode::vtkMRMLDiffusionTensorDisplayPropertiesNode()
{

  // Default display is FA (often used) and line glyphs (quickest to render)
  this->ScalarInvariant = this->FractionalAnisotropy;
  this->GlyphGeometry = this->Lines;
  this->ColorGlyphBy = this->FractionalAnisotropy;

  // Glyph general parameters
  this->GlyphScaleFactor = 50;
  this->GlyphExtractEigenvalues = 1;
  this->GlyphEigenvector = this->Major;

  // Line Glyph parameters
  this->LineGlyphResolution = 20;  // was 10 in dtmri.tcl

  // Tube Glyph parameters
  this->TubeGlyphRadius = 0.1;
  this->TubeGlyphNumberOfSides = 4; // was 6 in dtmri.tcl

  // Ellipsoid Glyph parameters
  this->EllipsoidGlyphThetaResolution = 9; // was 12
  this->EllipsoidGlyphPhiResolution = 9; // was 12

  // Superquadric Glyph parameters
  this->SuperquadricGlyphGamma = 1;
  this->SuperquadricGlyphThetaResolution = 6; // was 12
  this->SuperquadricGlyphPhiResolution = 6; // was 12

  // VTK Objects
  this->GlyphConnection = nullptr;
  this->UpdateGlyphSource();

  // set the type to user
  this->SetTypeToUser();

  // This node does not have a valid LUT
  this->SetLookupTable(nullptr);

}

//----------------------------------------------------------------------------
vtkMRMLDiffusionTensorDisplayPropertiesNode::~vtkMRMLDiffusionTensorDisplayPropertiesNode()
{
  this->SetGlyphConnection(nullptr);
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorDisplayPropertiesNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);

  of << " glyphGeometry=\"" << this->GlyphGeometry << "\"";
  of << " colorGlyphBy=\"" << this->ColorGlyphBy << "\"";
  of << " glyphScaleFactor=\"" << this->GlyphScaleFactor << "\"";
  of << " glyphEigenvector=\"" << this->GlyphEigenvector << "\"";
  of << " glyphExtractEigenvalues=\"" << this->GlyphExtractEigenvalues << "\"";
  of << " lineGlyphResolution=\"" << this->LineGlyphResolution << "\"";
  of << " tubeGlyphRadius=\"" << this->TubeGlyphRadius << "\"";
  of << " tubeGlyphNumberOfSides=\"" << this->TubeGlyphNumberOfSides << "\"";
  of << " ellipsoidGlyphThetaResolution=\"" << this->EllipsoidGlyphThetaResolution << "\"";
  of << " ellipsoidGlyphPhiResolution=\"" << this->EllipsoidGlyphPhiResolution << "\"";
  of << " superquadricGlyphGamma=\"" << this->SuperquadricGlyphGamma << "\"";
  of << " superquadricGlyphThetaResolution=\"" << this->SuperquadricGlyphThetaResolution << "\"";
  of << " superquadricGlyphPhiResolution=\"" << this->SuperquadricGlyphPhiResolution << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorDisplayPropertiesNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
  {
      attName = *(atts++);
      attValue = *(atts++);
      if (!strcmp(attName, "glyphGeometry"))
      {
      int glyphGeometry;
      std::stringstream ss;
      ss << attValue;
      ss >> glyphGeometry;
      this->SetGlyphGeometry(glyphGeometry);
      }
      else if (!strcmp(attName, "colorGlyphBy"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> ColorGlyphBy;
      }
      else if (!strcmp(attName, "glyphScaleFactor"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> GlyphScaleFactor;
      }
      else if (!strcmp(attName, "glyphEigenvector"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> GlyphEigenvector;
      }
      else if (!strcmp(attName, "glyphExtractEigenvalues"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >>GlyphExtractEigenvalues ;
      }
      else if (!strcmp(attName, "lineGlyphResolution"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> LineGlyphResolution;
      }
      else if (!strcmp(attName, "tubeGlyphRadius"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> TubeGlyphRadius;
      }
      else if (!strcmp(attName, "tubeGlyphNumberOfSides"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> TubeGlyphNumberOfSides;
      }
      else if (!strcmp(attName, "ellipsoidGlyphThetaResolution"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> EllipsoidGlyphThetaResolution;
      }
      else if (!strcmp(attName, "ellipsoidGlyphPhiResolution"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> EllipsoidGlyphPhiResolution;
      }
      else if (!strcmp(attName, "superquadricGlyphGamma"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> SuperquadricGlyphGamma;
      }
      else if (!strcmp(attName, "superquadricGlyphThetaResolution"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> SuperquadricGlyphThetaResolution;
      }
      else if (!strcmp(attName, "superquadricGlyphPhiResolution"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> SuperquadricGlyphPhiResolution;
      }
  }
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLDiffusionTensorDisplayPropertiesNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLDiffusionTensorDisplayPropertiesNode *node = (vtkMRMLDiffusionTensorDisplayPropertiesNode *) anode;

  this->SetScalarInvariant(node->ScalarInvariant);
  this->SetGlyphGeometry(node->GlyphGeometry);
  this->SetColorGlyphBy(node->ColorGlyphBy);
  this->SetGlyphScaleFactor(node->GlyphScaleFactor);
  this->SetGlyphEigenvector(node->GlyphEigenvector);
  this->SetGlyphExtractEigenvalues(node->GlyphExtractEigenvalues);
  this->SetLineGlyphResolution(node->LineGlyphResolution);
  this->SetTubeGlyphRadius(node->TubeGlyphRadius);
  this->SetTubeGlyphNumberOfSides(node->TubeGlyphNumberOfSides);
  this->SetEllipsoidGlyphThetaResolution(node->EllipsoidGlyphThetaResolution);
  this->SetEllipsoidGlyphPhiResolution(node->EllipsoidGlyphPhiResolution);
  this->SetSuperquadricGlyphGamma(node->SuperquadricGlyphGamma);
  this->SetSuperquadricGlyphThetaResolution(node->SuperquadricGlyphThetaResolution);
  this->SetSuperquadricGlyphPhiResolution(node->SuperquadricGlyphPhiResolution);

  this->EndModify(disabledModify);
  }

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorDisplayPropertiesNode::PrintSelf(ostream& os, vtkIndent indent)
{

  Superclass::PrintSelf(os,indent);
  os << indent << "ScalarInvariant:             " << this->ScalarInvariant << "\n";
  os << indent << "GlyphGeometry:             " << this->GlyphGeometry << "\n";
  os << indent << "ColorGlyphBy:             " << this->ColorGlyphBy << "\n";
  os << indent << "GlyphScaleFactor:             " << this->GlyphScaleFactor << "\n";
  os << indent << "GlyphEigenvector:             " << this->GlyphEigenvector << "\n";
  os << indent << "GlyphExtractEigenvalues:             " << this->GlyphExtractEigenvalues << "\n";
  os << indent << "LineGlyphResolution:             " << this->LineGlyphResolution << "\n";
  os << indent << "TubeGlyphRadius:             " << this->TubeGlyphRadius << "\n";
  os << indent << "TubeGlyphNumberOfSides:             " << this->TubeGlyphNumberOfSides << "\n";
  os << indent << "EllipsoidGlyphThetaResolution:             " << this->EllipsoidGlyphThetaResolution << "\n";
  os << indent << "EllipsoidGlyphPhiResolution:             " << this->EllipsoidGlyphPhiResolution << "\n";
  os << indent << "SuperquadricGlyphGamma:             " << this->SuperquadricGlyphGamma << "\n";
  os << indent << "SuperquadricGlyphThetaResolution:             " << this->SuperquadricGlyphThetaResolution << "\n";
  os << indent << "SuperquadricGlyphPhiResolution:             " << this->SuperquadricGlyphPhiResolution << "\n";
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorDisplayPropertiesNode
::SetGlyphConnection(vtkAlgorithmOutput* newGlyphConnection)
{
  if (newGlyphConnection == this->GlyphConnection)
    {
    return;
    }

  vtkAlgorithm* oldGlyphAlgorithm = this->GlyphConnection ?
    this->GlyphConnection->GetProducer() : nullptr;

  this->GlyphConnection = newGlyphConnection;

  vtkAlgorithm* glyphAlgorithm = this->GlyphConnection ?
    this->GlyphConnection->GetProducer() : nullptr;
  if (glyphAlgorithm != nullptr)
    {
    glyphAlgorithm->Register(this);
    }
  if (oldGlyphAlgorithm != nullptr)
    {
    oldGlyphAlgorithm->UnRegister(this);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorDisplayPropertiesNode::UpdateGlyphSource ( )
{
  vtkDebugMacro("Get Glyph Source");

  // Get rid of any old glyph source
  this->SetGlyphConnection(nullptr);

  // Create a new glyph source according to current settings

  switch ( this->GlyphGeometry )
    {
    case Lines:
    case Tubes:
      {
      vtkLineSource *line = vtkLineSource::New();

      // Scaling along x-axis corresponds to major eigenvector, etc.
      // Create a line along the proper axis for scaling:
      switch ( this->GlyphEigenvector )
        {
        case Major:
          line->SetPoint1( -1, 0, 0 );
          line->SetPoint2( 1, 0, 0 );
          break;
        case Middle:
          line->SetPoint1( 0, -1, 0 );
          line->SetPoint2( 0, 1, 0 );
          break;
        case Minor:
          line->SetPoint1( 0, 0, -1 );
          line->SetPoint2( 0, 0, 1 );
          break;
        }

      line->SetResolution( this->LineGlyphResolution );


      // if we are doing tubes, put a tube on the line
      if (this->GlyphGeometry == Tubes)
        {
        vtkTubeFilter *tube = vtkTubeFilter::New();
        tube->SetInputConnection( line->GetOutputPort( ) );
        tube->SetRadius( this->TubeGlyphRadius );
        tube->SetNumberOfSides( this->TubeGlyphNumberOfSides );

        this->SetGlyphConnection( tube->GetOutputPort( ) );
        tube->Delete( );

        vtkDebugMacro("Get Glyph Source: Tubes");
        }
      else
        {
        vtkDebugMacro("Get Glyph Source: Lines");
        // here we are just displaying lines
        this->SetGlyphConnection( line->GetOutputPort( ) );

        }

      line->Delete( );
      }

      break;

    case Ellipsoids:

      {
      vtkSphereSource *sphere = vtkSphereSource::New();
      sphere->SetThetaResolution( this->EllipsoidGlyphThetaResolution );
      sphere->SetPhiResolution( this->EllipsoidGlyphPhiResolution );

      this->SetGlyphConnection( sphere->GetOutputPort( ) );
      sphere->Delete( );

      vtkDebugMacro("Get Glyph Source: Ellipsoids");
      }

      break;

    case Superquadrics:
      {
      // TODO: this should be fleshed out
      vtkDebugMacro("vtkMRMLDiffusionTensorDisplayPropertiesNode: Superquadric glyph source not handled yet.");
      // Here do nothing, the superquadric must be created specifically for each tensor
      }

      break;

    }
}

//----------------------------------------------------------------------------
int vtkMRMLDiffusionTensorDisplayPropertiesNode::GetFirstScalarInvariant()
{
  return vtkMRMLDiffusionTensorDisplayPropertiesNode::Trace;
}

//----------------------------------------------------------------------------
int vtkMRMLDiffusionTensorDisplayPropertiesNode::GetLastScalarInvariant()
{
  return vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientationMinEigenvector;
}

//----------------------------------------------------------------------------
const char* vtkMRMLDiffusionTensorDisplayPropertiesNode::GetScalarEnumAsString(int var)
{
  if (var ==  vtkMRMLDiffusionTensorDisplayPropertiesNode::Trace)
    {
    return "Trace";
    }
  if (var == vtkMRMLDiffusionTensorDisplayPropertiesNode::Determinant)
    {
    return "Determinant";
    }
  if (var == vtkMRMLDiffusionTensorDisplayPropertiesNode::RelativeAnisotropy)
    {
    return "RelativeAnisotropy";
    }
  if (var == vtkMRMLDiffusionTensorDisplayPropertiesNode::FractionalAnisotropy)
    {
    return "FractionalAnisotropy";
    }
  if (var == vtkMRMLDiffusionTensorDisplayPropertiesNode::MaxEigenvalue)
    {
    return "MaxEigenvalue";
    }
  if (var == vtkMRMLDiffusionTensorDisplayPropertiesNode::MidEigenvalue)
    {
    return "MidEigenvalue";
    }
  if (var == vtkMRMLDiffusionTensorDisplayPropertiesNode::MinEigenvalue)
    {
    return "MinEigenvalue";
    }
  if (var == vtkMRMLDiffusionTensorDisplayPropertiesNode::LinearMeasure)
    {
    return "LinearMeasure";
    }
  if (var == vtkMRMLDiffusionTensorDisplayPropertiesNode::PlanarMeasure)
    {
    return "PlanarMeasure";
    }
  if (var == vtkMRMLDiffusionTensorDisplayPropertiesNode::SphericalMeasure)
    {
    return "SphericalMeasure";
    }
  if (var == vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientation)
    {
    return "ColorOrientation";
    }
  if (var == vtkMRMLDiffusionTensorDisplayPropertiesNode::D11)
    {
    return "D11";
    }
  if (var == vtkMRMLDiffusionTensorDisplayPropertiesNode::D22)
    {
    return "D22";
    }
  if (var == vtkMRMLDiffusionTensorDisplayPropertiesNode::D33)
    {
    return "D33";
    }
  if (var == vtkMRMLDiffusionTensorDisplayPropertiesNode::Mode)
    {
    return "Mode";
    }
  if (var == vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorMode)
    {
    return "ColorMode";
    }
  if (var == vtkMRMLDiffusionTensorDisplayPropertiesNode::MaxEigenvalueProjX)
    {
    return "MaxEigenvalueProjX";
    }
  if (var == vtkMRMLDiffusionTensorDisplayPropertiesNode::MaxEigenvalueProjY)
    {
    return "MaxEigenvalueProjY";
    }
  if (var == vtkMRMLDiffusionTensorDisplayPropertiesNode::MaxEigenvalueProjZ)
    {
    return "MaxEigenvalueProjZ";
    }
  if (var == vtkMRMLDiffusionTensorDisplayPropertiesNode::MaxEigenvec_ProjX)
    {
    return "MaxEigenvec_ProjX";
    }
  if (var == vtkMRMLDiffusionTensorDisplayPropertiesNode::MaxEigenvec_ProjY)
    {
    return "MaxEigenvec_ProjY";
    }
  if (var == vtkMRMLDiffusionTensorDisplayPropertiesNode::MaxEigenvec_ProjZ)
    {
    return "MaxEigenvec_ProjZ";
    }
  if (var == vtkMRMLDiffusionTensorDisplayPropertiesNode::ParallelDiffusivity)
    {
    return "ParallelDiffusivity";
    }
  if (var == vtkMRMLDiffusionTensorDisplayPropertiesNode::PerpendicularDiffusivity)
    {
    return "PerpendicularDiffusivity";
    }
  if (var == vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientationMiddleEigenvector)
    {
    return "ColorOrientationMiddleEigenvector";
    }
  if (var == vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientationMinEigenvector)
    {
    return "ColorOrientationMinEigenvector";
    }
  return "(unknown)";
}

//----------------------------------------------------------------------------
const char* vtkMRMLDiffusionTensorDisplayPropertiesNode::GetScalarInvariantAsString()
{
  return this->GetScalarEnumAsString(this->ScalarInvariant);
}

//----------------------------------------------------------------------------
const char* vtkMRMLDiffusionTensorDisplayPropertiesNode::GetGlyphGeometryAsString()
{
  return this->GetGlyphGeometryAsString(this->GlyphGeometry);
}

//----------------------------------------------------------------------------
const char* vtkMRMLDiffusionTensorDisplayPropertiesNode::GetGlyphGeometryAsString(int geometry)
{
  if (geometry == this->Lines)
    {
    return "Lines";
    }
  if (geometry == this->Tubes)
    {
    return "Tubes";
    }
  if (geometry == this->Ellipsoids)
    {
    return "Ellipsoids";
    }
  if (this->GlyphGeometry == this->Superquadrics)
    {
    return "Superquadrics";
    }
  return "(unknown)";
}

//----------------------------------------------------------------------------
const char*
vtkMRMLDiffusionTensorDisplayPropertiesNode::GetGlyphEigenvectorAsString()
{
  return this->GetGlyphEigenvectorAsString(this->GlyphEigenvector);
}

//----------------------------------------------------------------------------
const char*
vtkMRMLDiffusionTensorDisplayPropertiesNode::
  GetGlyphEigenvectorAsString(int eigenvector)
{

  if (eigenvector == this->Major)
    {
    return "Major";
    }
  if (eigenvector == this->Middle)
    {
    return "Middle";
    }
  if (eigenvector == this->Minor)
    {
    return "Minor";
    }
  return "(unknown)";
}

//----------------------------------------------------------------------------
const char *
vtkMRMLDiffusionTensorDisplayPropertiesNode::GetColorGlyphByAsString()
{
  return this->GetScalarEnumAsString(this->ColorGlyphBy);
}

//----------------------------------------------------------------------------
int vtkMRMLDiffusionTensorDisplayPropertiesNode::GetFirstColorGlyphBy()
{
  return vtkMRMLDiffusionTensorDisplayPropertiesNode::Trace;
}

//----------------------------------------------------------------------------
int vtkMRMLDiffusionTensorDisplayPropertiesNode::GetLastColorGlyphBy()
{
  return vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientationMinEigenvector;
}

//----------------------------------------------------------------------------
bool vtkMRMLDiffusionTensorDisplayPropertiesNode::ScalarInvariantHasKnownScalarRange(int ScalarInvariant)
{
  switch (ScalarInvariant)
    {
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientation:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorMode:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientationMiddleEigenvector:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientationMinEigenvector:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::Mode:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::FractionalAnisotropy:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::LinearMeasure:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::PlanarMeasure:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::SphericalMeasure:
      {
      return true;
      break;
      }
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::RelativeAnisotropy:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::ParallelDiffusivity:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::PerpendicularDiffusivity:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::MaxEigenvalueProjX:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::MaxEigenvalueProjY:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::MaxEigenvalueProjZ:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::MaxEigenvec_ProjX:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::MaxEigenvec_ProjY:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::MaxEigenvec_ProjZ:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::MaxEigenvalue:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::MidEigenvalue:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::MinEigenvalue:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::Trace:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::D11:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::D22:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::D33:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::Determinant:
      {
        return false;
        break;
      }
    default:
      return false;
      break;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorDisplayPropertiesNode::ScalarInvariantKnownScalarRange(int ScalarInvariant, double range[2])
{
  switch (ScalarInvariant)
    {
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientation:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorMode:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientationMiddleEigenvector:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientationMinEigenvector:
      {
      range[0] = 0;
      range[1] = 255.;
      break;
      }
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::RelativeAnisotropy:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::FractionalAnisotropy:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::LinearMeasure:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::PlanarMeasure:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::SphericalMeasure:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::ParallelDiffusivity:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::PerpendicularDiffusivity:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::MaxEigenvalueProjX:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::MaxEigenvalueProjY:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::MaxEigenvalueProjZ:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::MaxEigenvec_ProjX:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::MaxEigenvec_ProjY:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::MaxEigenvec_ProjZ:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::MaxEigenvalue:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::MidEigenvalue:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::MinEigenvalue:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::Trace:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::D11:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::D22:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::D33:
      {
      range[0] = 0;
      range[1] = 1.;
      break;
      }
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::Determinant:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::Mode:
    default:
      range[0] = -1;
      range[1] = 1.;
      break;
    }

}
