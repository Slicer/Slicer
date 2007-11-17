/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLDiffusionTensorDisplayPropertiesNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.0 $

=========================================================================auto=*/
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLDiffusionTensorDisplayPropertiesNode.h"
#include "vtkMRMLScene.h"

#include "vtkLookupTable.h"
#include "vtkLineSource.h"
#include "vtkTubeFilter.h"
#include "vtkSphereSource.h"

//------------------------------------------------------------------------------
vtkMRMLDiffusionTensorDisplayPropertiesNode* vtkMRMLDiffusionTensorDisplayPropertiesNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLDiffusionTensorDisplayPropertiesNode");
  if(ret)
    {
    return (vtkMRMLDiffusionTensorDisplayPropertiesNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLDiffusionTensorDisplayPropertiesNode;
}

//-----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLDiffusionTensorDisplayPropertiesNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLDiffusionTensorDisplayPropertiesNode");
  if(ret)
    {
    return (vtkMRMLDiffusionTensorDisplayPropertiesNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLDiffusionTensorDisplayPropertiesNode;
}


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
  this->GlyphSource = NULL;
  this->UpdateGlyphSource();

}

//----------------------------------------------------------------------------
vtkMRMLDiffusionTensorDisplayPropertiesNode::~vtkMRMLDiffusionTensorDisplayPropertiesNode()
{
  if ( this->GlyphSource != NULL )
    {
    this->GlyphSource->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorDisplayPropertiesNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);
  
  vtkIndent indent(nIndent);
  
  of << indent << " glyphGeometry=\"" << this->GlyphGeometry << "\"";
  of << indent << " colorGlyphBy=\"" << this->ColorGlyphBy << "\"";
  of << indent << " glyphScaleFactor=\"" << this->GlyphScaleFactor << "\"";
  of << indent << " glyphEigenvector=\"" << this->GlyphEigenvector << "\"";
  of << indent << " glyphExtractEigenvalues=\"" << this->GlyphExtractEigenvalues << "\"";
  of << indent << " lineGlyphResolution=\"" << this->LineGlyphResolution << "\"";
  of << indent << " tubeGlyphRadius=\"" << this->TubeGlyphRadius << "\"";
  of << indent << " tubeGlyphNumberOfSides=\"" << this->TubeGlyphNumberOfSides << "\"";
  of << indent << " ellipsoidGlyphThetaResolution=\"" << this->EllipsoidGlyphThetaResolution << "\"";
  of << indent << " ellipsoidGlyphPhiResolution=\"" << this->EllipsoidGlyphPhiResolution << "\"";
  of << indent << " superquadricGlyphGamma=\"" << this->SuperquadricGlyphGamma << "\"";
  of << indent << " superquadricGlyphThetaResolution=\"" << this->SuperquadricGlyphThetaResolution << "\"";
  of << indent << " superquadricGlyphPhiResolution=\"" << this->SuperquadricGlyphPhiResolution << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorDisplayPropertiesNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
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

}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLDiffusionTensorDisplayPropertiesNode::Copy(vtkMRMLNode *anode)
{
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
void vtkMRMLDiffusionTensorDisplayPropertiesNode::UpdateGlyphSource ( )
{
  vtkDebugMacro("Get Glyph Source");

  // Get rid of any old glyph source
  if ( this->GlyphSource != NULL )
    {
    this->GlyphSource->Delete();
    this->GlyphSource = NULL;
    }
  
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

      line->Update( );

      // if we are doing tubes, put a tube on the line
      if (this->GlyphGeometry == Tubes)
        {
        vtkTubeFilter *tube = vtkTubeFilter::New();
        tube->SetInput( line->GetOutput( ) );
        tube->SetRadius( this->TubeGlyphRadius );
        tube->SetNumberOfSides( this->TubeGlyphNumberOfSides );
        tube->Update( );

        this->SetGlyphSource( tube->GetOutput( ) );
        tube->Delete( );

        vtkDebugMacro("Get Glyph Source: Tubes");
        }
      else
        {
        vtkDebugMacro("Get Glyph Source: Lines");
        // here we are just displaying lines
        this->SetGlyphSource( line->GetOutput( ) );

        }

      line->Delete( );
      }

      break;

    case Ellipsoids:

      {
      vtkSphereSource *sphere = vtkSphereSource::New();
      sphere->SetThetaResolution( this->EllipsoidGlyphThetaResolution );
      sphere->SetPhiResolution( this->EllipsoidGlyphPhiResolution );

      sphere->Update( );
      this->SetGlyphSource( sphere->GetOutput( ) );
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

const char* vtkMRMLDiffusionTensorDisplayPropertiesNode::GetScalarEnumAsString(int var)
{
  if (var ==  this->Trace)
    {
    return "Trace";
    }
  if (var == this->Determinant)
    {
    return "Determinant";
    }
  if (var == this->RelativeAnisotropy)
    {
    return "RelativeAnisotropy";
    }
  if (var == this->FractionalAnisotropy)
    {
    return "FractionalAnisotropy";
    }
  if (var == this->MaxEigenvalue)
    {
    return "MaxEigenvalue";
    }
  if (var == this->MidEigenvalue)
    {
    return "MidEigenvalue";
    }
  if (var == this->MinEigenvalue)
    {
    return "MinEigenvalue";
    }
  if (var == this->LinearMeasure)
    {
    return "LinearMeasure";
    }
  if (var == this->PlanarMeasure)
    {
    return "PlanarMeasure";
    }
  if (var == this->SphericalMeasure)
    {
    return "SphericalMeasure";
    }
  if (var == this->ColorOrientation)
    {
    return "ColorOrientation";
    }
  if (var == this->D11)
    {
    return "D11";
    }
  if (var == this->D22)
    {
    return "D22";
    }
  if (var == this->D33)
    {
    return "D33";
    }
  if (var == this->Mode)
    {
    return "Mode";
    }
  if (var == this->ColorMode)
    {
    return "ColorMode";
    }
  if (var == this->MaxEigenvalueProjX)
    {
    return "MaxEigenvalueProjX";
    }
  if (var == this->MaxEigenvalueProjY)
    {
    return "MaxEigenvalueProjY";
    }
  if (var == this->MaxEigenvalueProjZ)
    {
    return "MaxEigenvalueProjZ";
    }
  if (var == this->MaxEigenvec_ProjX)
    {
    return "MaxEigenvec_ProjX";
    }
  if (var == this->MaxEigenvec_ProjY)
    {
    return "MaxEigenvec_ProjY";
    }
  if (var == this->MaxEigenvec_ProjZ)
    {
    return "MaxEigenvec_ProjZ";
    }
  if (var == this->ParallelDiffusivity)
    {
    return "ParallelDiffusivity";
    }
  if (var == this->PerpendicularDiffusivity)
    {
    return "PerpendicularDiffusivity";
    }
  if (var == this->ColorOrientationMiddleEigenvector)
    {
    return "ColorOrientationMiddleEigenvector";
    }
  if (var == this->ColorOrientationMinEigenvector)
    {
    return "ColorOrientationMinEigenvector";
    }
  return "(unknown)";
}

const char* vtkMRMLDiffusionTensorDisplayPropertiesNode::GetScalarInvariantAsString()
{
  return this->GetScalarEnumAsString(this->ScalarInvariant);
}

const char* vtkMRMLDiffusionTensorDisplayPropertiesNode::GetGlyphGeometryAsString()
{
  if (this->GlyphGeometry == this->Lines)
    {
    return "Lines";
    }
  if (this->GlyphGeometry == this->Tubes)
    {
    return "Tubes";
    }
  if (this->GlyphGeometry == this->Ellipsoids)
    {
    return "Ellipsoids";
    }
  if (this->GlyphGeometry == this->Superquadrics)
    {
    return "Superquadrics";
    }
  return "(unknown)";
}

const char*
vtkMRMLDiffusionTensorDisplayPropertiesNode::GetGlyphEigenvectorAsString()
{

  if (this->GlyphEigenvector == this->Major)
    {
    return "Major";
    }
  if (this->GlyphEigenvector == this->Middle)
    {
    return "Middle";
    }
  if (this->GlyphEigenvector == this->Minor)
    {
    return "Minor";
    }
  return "(unknown)";
}

const char *
vtkMRMLDiffusionTensorDisplayPropertiesNode::GetColorGlyphByAsString()
{
  return this->GetScalarEnumAsString(this->ColorGlyphBy);
}
