/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFiberBundleGlyphDisplayNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLDiffusionTensorDisplayPropertiesNode.h"
#include "vtkMRMLFiberBundleGlyphDisplayNode.h"
#include "vtkMRMLScene.h"

// Teem includes
#include "vtkDiffusionTensorGlyph.h"

// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkAssignAttribute.h>
#include <vtkCallbackCommand.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkVersion.h>

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

  this->Superclass::Copy(anode);
  vtkMRMLFiberBundleGlyphDisplayNode *node =
    vtkMRMLFiberBundleGlyphDisplayNode::SafeDownCast(anode);

  this->SetTwoDimensionalVisibility(node->TwoDimensionalVisibility);

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleGlyphDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  os << indent << "TwoDimensionalVisibility:             " << this->TwoDimensionalVisibility << "\n";
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLFiberBundleGlyphDisplayNode::GetOutputPolyDataConnection()
{
  return this->DiffusionTensorGlyphFilter->GetOutputPort();
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleGlyphDisplayNode::UpdatePolyDataPipeline()
{
  this->Superclass::UpdatePolyDataPipeline();

  this->DiffusionTensorGlyphFilter->SetInputConnection(
    this->Superclass::GetOutputPolyDataConnection());

  // set display properties according to the tensor-specific display properties node for glyphs
  vtkMRMLDiffusionTensorDisplayPropertiesNode * diffusionTensorDisplayNode =
    this->GetDiffusionTensorDisplayPropertiesNode( );

  if (diffusionTensorDisplayNode)
    {
    // Need to set the input connection to prevent unnecessary error messages
    // complaining about missing input port 1.
    this->DiffusionTensorGlyphFilter->SetSourceConnection(
      diffusionTensorDisplayNode->GetGlyphConnection() );
    }

  if (!this->Visibility)
    {
    return;
    }

  if (diffusionTensorDisplayNode != NULL)
    {
    // TO DO: need filter to calculate FA, average FA, etc. as requested

    // get tensors from the fiber bundle node and glyph them
    // TO DO: include superquadrics
    // if glyph type is other than superquadrics, get glyph source
    if (diffusionTensorDisplayNode->GetGlyphGeometry( ) !=
        vtkMRMLDiffusionTensorDisplayPropertiesNode::Superquadrics)
      {
      this->DiffusionTensorGlyphFilter->ClampScalingOff();
      // TO DO: implement max # ellipsoids, random sampling features
      this->DiffusionTensorGlyphFilter->SetResolution(
        diffusionTensorDisplayNode->GetLineGlyphResolution());
      this->DiffusionTensorGlyphFilter->SetScaleFactor(
        diffusionTensorDisplayNode->GetGlyphScaleFactor( ) );

      vtkDebugMacro("setting glyph geometry" << diffusionTensorDisplayNode->GetGlyphGeometry( ) );

      // set glyph coloring
      if (this->GetColorMode ( ) == vtkMRMLFiberBundleDisplayNode::colorModeScalar)
        {
        switch ( diffusionTensorDisplayNode->GetColorGlyphBy( ))
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
          case vtkMRMLDiffusionTensorDisplayPropertiesNode::SphericalMeasure:
            {
            vtkDebugMacro("coloring with spherical measure");
            this->DiffusionTensorGlyphFilter->ColorGlyphsBySphericalMeasure( );
            }
            break;
          case vtkMRMLDiffusionTensorDisplayPropertiesNode::ParallelDiffusivity:
            {
            vtkDebugMacro("coloring with parallel diff");
            this->DiffusionTensorGlyphFilter->ColorGlyphsByParallelDiffusivity( );
            }
            break;
          case vtkMRMLDiffusionTensorDisplayPropertiesNode::PerpendicularDiffusivity:
            {
            vtkDebugMacro("coloring with perpendicular diff");
            this->DiffusionTensorGlyphFilter->ColorGlyphsByPerpendicularDiffusivity( );
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
            vtkErrorMacro("unknown glyph coloring mode");
            this->ScalarVisibilityOff( );
            }
            break;
          }
        }
      }
    }
  else
    {
    //this->ScalarVisibilityOff( );
    }

  if (this->GetAutoScalarRange() &&
      this->GetScalarVisibility())
    {
    double range[2] = {0., 1.};
    if (this->GetColorMode ( ) != vtkMRMLFiberBundleDisplayNode::colorModeUseCellScalars)
      {
      int scalarInvariant = 0;
      if (DiffusionTensorDisplayPropertiesNode)
        {
        scalarInvariant = DiffusionTensorDisplayPropertiesNode->GetColorGlyphBy( );
        }
      if ( DiffusionTensorDisplayPropertiesNode &&
           vtkMRMLDiffusionTensorDisplayPropertiesNode::ScalarInvariantHasKnownScalarRange(
             scalarInvariant))
        {
        vtkMRMLDiffusionTensorDisplayPropertiesNode::ScalarInvariantKnownScalarRange(
          scalarInvariant, range);
        }
      else if (this->GetInputPolyData())
        {
        this->GetOutputPolyDataConnection()->GetProducer()->Update();
        vtkPointData *pointData = this->GetOutputPolyData()->GetPointData();
        if (pointData)
          {
          double *activeScalarRange = 0;
          if (pointData->GetArray(this->GetActiveScalarName()))
            {
            activeScalarRange = pointData->GetArray(
                                this->GetActiveScalarName())->GetRange();
            }
          else if (pointData->GetArray(0))
            {
            activeScalarRange = pointData->GetArray(0)->GetRange();
            }
         if (activeScalarRange)
            {
            range[0] = activeScalarRange[0];
            range[1] = activeScalarRange[1];
            }
          }
        }
      }

    //this->ScalarRange[0] = range[0];
    //this->ScalarRange[1] = range[1];
    this->SetScalarRange(range);
    }
}

