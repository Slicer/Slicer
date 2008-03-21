/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLDiffusionTensorVolumeSliceDisplayNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkDiffusionTensorGlyph.h"

#include "vtkDiffusionTensorGlyph.h"

#include "vtkMRMLDiffusionTensorVolumeSliceDisplayNode.h"
#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkMRMLDiffusionTensorVolumeSliceDisplayNode* vtkMRMLDiffusionTensorVolumeSliceDisplayNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLDiffusionTensorVolumeSliceDisplayNode");
  if(ret)
    {
    return (vtkMRMLDiffusionTensorVolumeSliceDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLDiffusionTensorVolumeSliceDisplayNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLDiffusionTensorVolumeSliceDisplayNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLDiffusionTensorVolumeSliceDisplayNode");
  if(ret)
    {
    return (vtkMRMLDiffusionTensorVolumeSliceDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLDiffusionTensorVolumeSliceDisplayNode;
}


//----------------------------------------------------------------------------
vtkMRMLDiffusionTensorVolumeSliceDisplayNode::vtkMRMLDiffusionTensorVolumeSliceDisplayNode()
{
  this->DiffusionTensorGlyphFilter = vtkDiffusionTensorGlyph::New();
  this->DiffusionTensorGlyphFilter->SetResolution (1);

  this->ColorMode = vtkMRMLFiberBundleDisplayNode::colorModeScalar;
}


//----------------------------------------------------------------------------
vtkMRMLDiffusionTensorVolumeSliceDisplayNode::~vtkMRMLDiffusionTensorVolumeSliceDisplayNode()
{
  this->RemoveObservers ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
  this->DiffusionTensorGlyphFilter->Delete();
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeSliceDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

}


//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeSliceDisplayNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLDiffusionTensorVolumeSliceDisplayNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLDiffusionTensorVolumeSliceDisplayNode *node = (vtkMRMLDiffusionTensorVolumeSliceDisplayNode *) anode;

}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeSliceDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  //int idx;
  
  Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeSliceDisplayNode::SetSlicePositionMatrix(vtkMatrix4x4 *matrix)
{
  if (this->DiffusionTensorGlyphFilter)
    {
    this->DiffusionTensorGlyphFilter->SetVolumePositionMatrix(matrix);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeSliceDisplayNode::SetSliceImage(vtkImageData *image)
{
  if (this->DiffusionTensorGlyphFilter)
    {
    this->DiffusionTensorGlyphFilter->SetInput(image);
    this->DiffusionTensorGlyphFilter->SetDimensions(image->GetDimensions());
    }
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeSliceDisplayNode::SetPolyData(vtkPolyData *glyphPolyData)
{
}

//----------------------------------------------------------------------------
vtkPolyData* vtkMRMLDiffusionTensorVolumeSliceDisplayNode::GetPolyData()
{
  if (this->DiffusionTensorGlyphFilter)
    {
    this->UpdatePolyDataPipeline();
    this->DiffusionTensorGlyphFilter->Update();
    return this->DiffusionTensorGlyphFilter->GetOutput();
    }
  else
    {
    return NULL;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeSliceDisplayNode::UpdatePolyDataPipeline() 
{
  if (this->Visibility == 0)
    {
    return;
    }

  // set display properties according to the tensor-specific display properties node for glyphs
  vtkMRMLDiffusionTensorDisplayPropertiesNode * DTDisplayNode = this->GetDTDisplayPropertiesNode( );
  
  if (DTDisplayNode != NULL) {
    // TO DO: need filter to calculate FA, average FA, etc. as requested
    
    // get tensors from the fiber bundle node and glyph them
    // TO DO: include superquadrics
    // if glyph type is other than superquadrics, get glyph source
    if (DTDisplayNode->GetGlyphGeometry( ) != vtkMRMLDiffusionTensorDisplayPropertiesNode::Superquadrics) 
      {
      this->DiffusionTensorGlyphFilter->ClampScalingOff();
      
      // TO DO: implement max # ellipsoids, random sampling features
      this->DiffusionTensorGlyphFilter->SetResolution(1);
      this->DiffusionTensorGlyphFilter->SetDimensionResolution( DTDisplayNode->GetLineGlyphResolution(), DTDisplayNode->GetLineGlyphResolution());
      this->DiffusionTensorGlyphFilter->SetScaleFactor( DTDisplayNode->GetGlyphScaleFactor( ) );
      
      this->DiffusionTensorGlyphFilter->SetSource( DTDisplayNode->GetGlyphSource( ) );
      
      vtkDebugMacro("setting glyph geometry" << DTDisplayNode->GetGlyphGeometry( ) );
      
      // set glyph coloring
      if (this->GetColorMode ( ) == vtkMRMLFiberBundleDisplayNode::colorModeSolid)
        {
        this->ScalarVisibilityOff( );
        }
      else  
        {
        if (this->GetColorMode ( ) == vtkMRMLFiberBundleDisplayNode::colorModeScalar)
          {
          this->ScalarVisibilityOn( );

          switch ( DTDisplayNode->GetColorGlyphBy( ))
            {
            case vtkMRMLDiffusionTensorDisplayPropertiesNode::FractionalAnisotropy:
              {
                vtkDebugMacro("coloring with FA==============================");
                this->DiffusionTensorGlyphFilter->ColorGlyphsByFractionalAnisotropy( );
              }
              break;
            case vtkMRMLDiffusionTensorDisplayPropertiesNode::LinearMeasure:
              {
                vtkDebugMacro("coloring with Cl=============================");
                this->DiffusionTensorGlyphFilter->ColorGlyphsByLinearMeasure( );
              }
              break;
            case vtkMRMLDiffusionTensorDisplayPropertiesNode::Trace:
              {
                vtkDebugMacro("coloring with trace =================");
                this->DiffusionTensorGlyphFilter->ColorGlyphsByTrace( );
              }
              break;
            case vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientation:
              {
                vtkDebugMacro("coloring with direction (re-implement)");
                this->DiffusionTensorGlyphFilter->ColorGlyphsByOrientation( );
              }
              break;
            case vtkMRMLDiffusionTensorDisplayPropertiesNode::PlanarMeasure:
              {
                vtkDebugMacro("coloring with planar");
                this->DiffusionTensorGlyphFilter->ColorGlyphsByPlanarMeasure( );
              }
              break;
            case vtkMRMLDiffusionTensorDisplayPropertiesNode::MaxEigenvalue:
              {
                vtkDebugMacro("coloring with max eigenval");
                this->DiffusionTensorGlyphFilter->ColorGlyphsByMaxEigenvalue( );
              }
              break;
            case vtkMRMLDiffusionTensorDisplayPropertiesNode::MidEigenvalue:
              {
                vtkDebugMacro("coloring with mid eigenval");
                this->DiffusionTensorGlyphFilter->ColorGlyphsByMidEigenvalue( );
              }
              break;
            case vtkMRMLDiffusionTensorDisplayPropertiesNode::MinEigenvalue:
              {
                vtkDebugMacro("coloring with min eigenval");
                this->DiffusionTensorGlyphFilter->ColorGlyphsByMinEigenvalue( );
              }
              break;
            case vtkMRMLDiffusionTensorDisplayPropertiesNode::RelativeAnisotropy:
              {
                vtkDebugMacro("coloring with relative anisotropy");
                this->DiffusionTensorGlyphFilter->ColorGlyphsByRelativeAnisotropy( );
              }
              break;
          default:
            {
            vtkDebugMacro("coloring with relative anisotropy");
            this->ScalarVisibilityOff( );
            }
            break;
            
          }
        }
      }   
    }
    else
      {
      this->ScalarVisibilityOff( );
      }
    }
  else
    {
    this->ScalarVisibilityOff( );
    }
   
 if (this->GetScalarVisibility())
  {
  this->DiffusionTensorGlyphFilter->Update();
  double *range = this->DiffusionTensorGlyphFilter->GetOutput()->GetScalarRange();
  this->ScalarRange[0] = range[0];
  this->ScalarRange[1] = range[1];
  // avoid Set not to cause event loops
  //this->SetScalarRange( this->DiffusionTensorGlyphFilter->GetOutput()->GetScalarRange() );
  }

}


