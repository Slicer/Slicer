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
  this->GlyphScaleFactor = 1000;
  this->GlyphExtractEigenvalues = 1;
  this->GlyphEigenvector = this->Major;

  // Line Glyph parameters
  this->LineGlyphResolution = 5;  // was 10 in dtmri.tcl

  // Tube Glyph parameters
  this->TubeGlyphRadius = 0.1;
  this->TubeGlyphNumberOfSides = 4; // was 6 in dtmri.tcl

  // Ellipsoid Glyph parameters
  this->EllipsoidGlyphThetaResolution = 6; // was 12
  this->EllipsoidGlyphPhiResolution = 6; // was 12

  // Superquadric Glyph parameters
  this->SuperquadricGlyphGamma = 1;
  this->SuperquadricGlyphThetaResolution = 6; // was 12
  this->SuperquadricGlyphPhiResolution = 6; // was 12

  // VTK Objects
  this->GlyphSource = NULL;

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
  
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorDisplayPropertiesNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  int numColours;
  while (*atts != NULL) 
  {
      attName = *(atts++);
      attValue = *(atts++);
      if (!strcmp(attName, "name"))
      {
          this->SetName(attValue);
      }
      else if (!strcmp(attName, "id"))
      {
          // handled at the vtkMRMLNode level
      }
      else if (!strcmp(attName, "numcolors"))
        {
        std::stringstream ss;
        ss << attValue;
        ss >> numColours;
        vtkDebugMacro("Setting the look up table size to " << numColours << "\n");
        this->LookupTable->SetNumberOfTableValues(numColours);
        this->Names.clear();
        this->Names.resize(numColours);
        }
      else  if (!strcmp(attName, "colors")) 
      {
      std::stringstream ss;
      for (int i = 0; i < this->LookupTable->GetNumberOfTableValues(); i++)
        {
        vtkDebugMacro("Reading colour " << i << " of " << this->LookupTable->GetNumberOfTableValues() << endl);
        ss << attValue;
        // index name r g b a
        int index;
        std::string name;
        double r, g, b, a;
        ss >> index;
        ss >> name;          
        ss >> r;
        ss >> g;
        ss >> b;
        ss >> a;
        vtkDebugMacro("Adding colour at index " << index << ", r = " << r << ", g = " << g << ", b = " << b << ", a = " << a << " and then setting name to " << name.c_str() << endl);
        this->LookupTable->SetTableValue(index, r, g, b, a);
        this->SetColorNameWithSpaces(index, name.c_str(), "_");
        }
      }
      else if (!strcmp(attName, "type")) 
      {
      int type;
      std::stringstream ss;
      ss << attValue;
      ss >> type;
      this->SetType(type);
      }
      else if (!strcmp(attName, "filename"))
        {
        this->SetFileName(attValue);
        // read in the file with the colours
        std::cout << "Reading file " << this->FileName << endl;
        this->ReadFile();
        }
      else
      {
          std::cerr << "Unknown attribute name " << attName << endl;
      }
  }
  vtkDebugMacro("Finished reading in xml attributes, list id = " << this->GetID() << " and name = " << this->GetName() << endl);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLDiffusionTensorDisplayPropertiesNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLDiffusionTensorDisplayPropertiesNode *node = (vtkMRMLDiffusionTensorDisplayPropertiesNode *) anode;

}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorDisplayPropertiesNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);

}


//----------------------------------------------------------------------------
vtkPolyData * vtkMRMLDiffusionTensorDisplayPropertiesNode::GetGlyphSource ( )
{
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
      if (this->GlyphGeometry == this->Tubes)
        {
        vtkTubeFilter *tube = vtkTubeFilter::New();
        tube->SetInput( line->GetOutput( ) );
        tube->SetRadius( this->TubeGlyphRadius );
        tube->SetNumberOfSides( this->TubeGlyphNumberOfSides );
        tube->Update( );

        this->SetGlyphSource( tube->GetOutput( ) );
        tube->Delete( );

        }
      else
        {

        // here we are just displaying lines
        this->SetGlyphSource( line->GetOutput( ) );

        }

      line->Delete( );

      break;

    case Ellipsoids:

      vtkSphereSource *sphere = vtkSphereSource::New();
      sphere->SetThetaResolution( this->EllipsoidGlyphThetaResolution );
      sphere->SetPhiResolution( this->EllipsoidGlyphPhiResolution );

      sphere->Update( );
      this->SetGlyphSource( sphere->GetOutput( ) );
      sphere->Delete( );

      break;
    case Superquadrics:
      // Here do nothing, the superquadric must be created specifically for each tensor
      break;

    }

  return ( this->GlyphSource );

}
