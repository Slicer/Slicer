/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFiberBundleGlyphDisplayNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkDiffusionTensorGlyph.h"


#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLFiberBundleGlyphDisplayNode.h"
#include "vtkMRMLDiffusionTensorDisplayPropertiesNode.h"

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLFiberBundleGlyphDisplayNode);


//----------------------------------------------------------------------------
vtkMRMLFiberBundleGlyphDisplayNode::vtkMRMLFiberBundleGlyphDisplayNode()
{
  this->DiffusionTensorGlyphFilter = vtkDiffusionTensorGlyph::New();

  this->TwoDimensionalVisibility = 0;
  this->ColorMode = vtkMRMLFiberBundleDisplayNode::colorModeScalar;
}


//----------------------------------------------------------------------------
vtkMRMLFiberBundleGlyphDisplayNode::~vtkMRMLFiberBundleGlyphDisplayNode()
{
  this->RemoveObservers ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
  this->DiffusionTensorGlyphFilter->Delete();
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleGlyphDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);
  of << indent << " twoDimensionalVisibility=\"" << (this->TwoDimensionalVisibility ? "true" : "false") << "\"";
}



//----------------------------------------------------------------------------
void vtkMRMLFiberBundleGlyphDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "twoDimensionalVisibility")) 
      {
      if (!strcmp(attValue,"true")) 
        {
        this->TwoDimensionalVisibility  = 1;
        }
      else
        {
        this->TwoDimensionalVisibility = 0;
        }
      }
    }  

  this->EndModify(disabledModify);
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLFiberBundleGlyphDisplayNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLFiberBundleGlyphDisplayNode *node = (vtkMRMLFiberBundleGlyphDisplayNode *) anode;

  this->SetTwoDimensionalVisibility(node->TwoDimensionalVisibility);

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleGlyphDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  //int idx;
  
  Superclass::PrintSelf(os,indent);
  os << indent << "TwoDimensionalVisibility:             " << this->TwoDimensionalVisibility << "\n";
}


//----------------------------------------------------------------------------
void vtkMRMLFiberBundleGlyphDisplayNode::SetPolyData(vtkPolyData *glyphPolyData)
{
  if ((this->DiffusionTensorGlyphFilter) && glyphPolyData != this->PolyData)
    {
    Superclass::SetPolyData(glyphPolyData);
    this->DiffusionTensorGlyphFilter->SetInput(glyphPolyData);
    }
}

//----------------------------------------------------------------------------
vtkPolyData* vtkMRMLFiberBundleGlyphDisplayNode::GetPolyData()
{
  if (this->DiffusionTensorGlyphFilter)
    {
    //this->UpdatePolyDataPipeline();
    //this->DiffusionTensorGlyphFilter->Update();
    return this->DiffusionTensorGlyphFilter->GetOutput();
    }
  else
    {
    return NULL;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleGlyphDisplayNode::UpdatePolyDataPipeline() 
{
  if (this->PolyData && this->Visibility)
    {
    if (this->DiffusionTensorGlyphFilter)
    {
        this->DiffusionTensorGlyphFilter->SetInput(this->PolyData);
    }

    // set display properties according to the tensor-specific display properties node for glyphs
    vtkMRMLDiffusionTensorDisplayPropertiesNode * DiffusionTensorDisplayNode = this->GetDiffusionTensorDisplayPropertiesNode( );
   
    if (DiffusionTensorDisplayNode != NULL) {
      // TO DO: need filter to calculate FA, average FA, etc. as requested
      
      // get tensors from the fiber bundle node and glyph them
      // TO DO: include superquadrics
      // if glyph type is other than superquadrics, get glyph source
      if (DiffusionTensorDisplayNode->GetGlyphGeometry( ) != vtkMRMLDiffusionTensorDisplayPropertiesNode::Superquadrics) 
        {
        this->DiffusionTensorGlyphFilter->ClampScalingOff();
        
        // TO DO: implement max # ellipsoids, random sampling features
        this->DiffusionTensorGlyphFilter->SetResolution(DiffusionTensorDisplayNode->GetLineGlyphResolution());
        
        this->DiffusionTensorGlyphFilter->SetScaleFactor( DiffusionTensorDisplayNode->GetGlyphScaleFactor( ) );
        
        this->DiffusionTensorGlyphFilter->SetSource( DiffusionTensorDisplayNode->GetGlyphSource( ) );
        
        vtkDebugMacro("setting glyph geometry" << DiffusionTensorDisplayNode->GetGlyphGeometry( ) );
        
        // set glyph coloring
        if (this->GetColorMode ( ) == vtkMRMLFiberBundleDisplayNode::colorModeSolid)
          {
          this->ScalarVisibilityOff( );
          }
        else if (this->GetColorMode ( ) == vtkMRMLFiberBundleDisplayNode::colorModeUseCellScalars)
      {
        this->ScalarVisibilityOn( );
       
      }
        else  
          {
          if (this->GetColorMode ( ) == vtkMRMLFiberBundleDisplayNode::colorModeScalar)
            {
            this->ScalarVisibilityOn( );

            switch ( DiffusionTensorDisplayNode->GetColorGlyphBy( ))
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
                  vtkDebugMacro("coloring with orientation ====================");
                  this->DiffusionTensorGlyphFilter->ColorGlyphsByOrientation( );
                  vtkMRMLNode* ColorNode = this->GetScene()->GetNodeByID("vtkMRMLColorTableNodeFullRainbow");
                  if (ColorNode)
                  {
                    this->SetAndObserveColorNodeID(ColorNode->GetID());
                  }
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
      //this->ScalarVisibilityOff( );
      }
     
    if (this->GetAutoScalarRange() && this->GetScalarVisibility() && this->DiffusionTensorGlyphFilter->GetInput() != NULL )
      {
        if (this->GetColorMode ( ) != vtkMRMLFiberBundleDisplayNode::colorModeUseCellScalars)
        {
          int ScalarInvariant = 0;
          if (DiffusionTensorDisplayPropertiesNode)
          {
           ScalarInvariant = DiffusionTensorDisplayPropertiesNode->GetColorGlyphBy( );
          }
          double range[2];
          if ( DiffusionTensorDisplayPropertiesNode && vtkMRMLDiffusionTensorDisplayPropertiesNode::ScalarInvariantHasKnownScalarRange(ScalarInvariant))
          {
            vtkMRMLDiffusionTensorDisplayPropertiesNode::ScalarInvariantKnownScalarRange(ScalarInvariant, range);
          } else {
            this->DiffusionTensorGlyphFilter->Update();
            this->DiffusionTensorGlyphFilter->GetOutput()->GetScalarRange(range);
          }
          this->ScalarRange[0] = range[0];
          this->ScalarRange[1] = range[1];
      }}
    }
}


