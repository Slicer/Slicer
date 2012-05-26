/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFiberBundleLineDisplayNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include "vtkCellData.h"
#include "vtkPointData.h"

#include "vtkPolyDataTensorToColor.h"
#include "vtkPolyDataColorLinesByOrientation.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLFiberBundleLineDisplayNode.h"
#include "vtkMRMLDiffusionTensorDisplayPropertiesNode.h"

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLFiberBundleLineDisplayNode);


//----------------------------------------------------------------------------
vtkMRMLFiberBundleLineDisplayNode::vtkMRMLFiberBundleLineDisplayNode()
{
  this->TensorToColor = vtkPolyDataTensorToColor::New();
  this->ColorLinesByOrientation = vtkPolyDataColorLinesByOrientation::New();
  this->ColorMode = vtkMRMLFiberBundleDisplayNode::colorModeScalar;
}


//----------------------------------------------------------------------------
vtkMRMLFiberBundleLineDisplayNode::~vtkMRMLFiberBundleLineDisplayNode()
{
  this->RemoveObservers ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
  this->TensorToColor->Delete();
  this->ColorLinesByOrientation->Delete();
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleLineDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);
}



//----------------------------------------------------------------------------
void vtkMRMLFiberBundleLineDisplayNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLFiberBundleLineDisplayNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  //vtkMRMLFiberBundleLineDisplayNode *node = (vtkMRMLFiberBundleLineDisplayNode *) anode;

}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleLineDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  //int idx;
  
  Superclass::PrintSelf(os,indent);
}

 
//----------------------------------------------------------------------------
void vtkMRMLFiberBundleLineDisplayNode::SetPolyData(vtkPolyData *glyphPolyData)
{
  if ((this->PolyData != glyphPolyData) && (this->TensorToColor))
    {
    Superclass::SetPolyData(glyphPolyData);
    this->TensorToColor->SetInput(glyphPolyData);
    this->ColorLinesByOrientation->SetInput(this->TensorToColor->GetOutput());
    }
}

//----------------------------------------------------------------------------
vtkPolyData* vtkMRMLFiberBundleLineDisplayNode::GetPolyData()
{
  if (this->ColorLinesByOrientation)
    {
    return this->OutputPolyData;
    }
  else
    {
    return NULL;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleLineDisplayNode::UpdatePolyDataPipeline() 
{
  if (this->PolyData && this->Visibility)
    {
    vtkDebugMacro("Updating the PolyData Pipeline *****************************");
    // set display properties according to the tensor-specific display properties node for glyphs
    vtkMRMLDiffusionTensorDisplayPropertiesNode * DiffusionTensorDisplayPropertiesNode = this->GetDiffusionTensorDisplayPropertiesNode( );
    

    vtkPolyData *IntermediatePolyData = NULL;
    if (DiffusionTensorDisplayPropertiesNode != NULL) {
      // TO DO: need filter to calculate FA, average FA, etc. as requested
      
      // set line coloring
      if (this->GetColorMode ( ) == vtkMRMLFiberBundleDisplayNode::colorModeSolid)
        {
        this->ScalarVisibilityOff( );
        this->TensorToColor->SetExtractScalar(0);
        IntermediatePolyData = this->TensorToColor->GetOutput();

        vtkMRMLNode* ColorNode = this->GetScene()->GetNodeByID("vtkMRMLColorTableNodeFullRainbow");
        if (ColorNode)
          {
            this->SetAndObserveColorNodeID(ColorNode->GetID());
          }

        this->AutoScalarRangeOff();
        this->SetScalarRange(0, 255);
        }
      else if (this->GetColorMode ( ) == vtkMRMLFiberBundleDisplayNode::colorModeUseCellScalars)
      {
        this->ScalarVisibilityOn( );
        this->TensorToColor->SetExtractScalar(0);
        if (!this->PolyData->GetCellData()->HasArray(this->GetActiveScalarName()) || this->GetActiveScalarName() == this->ColorLinesByOrientation->GetScalarArrayName())
        {
          IntermediatePolyData = this->ColorLinesByOrientation->GetOutput();
          this->SetActiveScalarName(this->ColorLinesByOrientation->GetScalarArrayName());
          vtkMRMLNode* ColorNode = this->GetScene()->GetNodeByID("vtkMRMLColorTableNodeFullRainbow");
          if (ColorNode)
          {
            this->SetAndObserveColorNodeID(ColorNode->GetID());
          }
        } else {
          IntermediatePolyData = this->PolyData;
        }
      }
      else if (this->GetColorMode ( ) == vtkMRMLFiberBundleDisplayNode::colorModeMeanFiberOrientation)
      {
        vtkDebugMacro("Color by mean fiber orientation");
        this->ScalarVisibilityOn( );
        IntermediatePolyData = this->ColorLinesByOrientation->GetOutput();
        this->ColorLinesByOrientation->SetColorMode(ColorLinesByOrientation->colorModeMeanFiberOrientation);
        vtkMRMLDiffusionTensorDisplayPropertiesNode::ScalarInvariantKnownScalarRange(vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientation, this->ScalarRange);
        vtkMRMLNode* ColorNode = this->GetScene()->GetNodeByID("vtkMRMLColorTableNodeFullRainbow");
        if (ColorNode)
        {
          this->SetAndObserveColorNodeID(ColorNode->GetID());
        }

      }
      else if (this->GetColorMode ( ) == vtkMRMLFiberBundleDisplayNode::colorModePointFiberOrientation)
      {
        vtkDebugMacro("Color by mean fiber orientation");
        this->ScalarVisibilityOn( );
        IntermediatePolyData = this->ColorLinesByOrientation->GetOutput();
        this->ColorLinesByOrientation->SetColorMode(ColorLinesByOrientation->colorModePointFiberOrientation);
        vtkMRMLDiffusionTensorDisplayPropertiesNode::ScalarInvariantKnownScalarRange(vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientation, this->ScalarRange);
        vtkMRMLNode* ColorNode = this->GetScene()->GetNodeByID("vtkMRMLColorTableNodeFullRainbow");
        if (ColorNode)
        {
          this->SetAndObserveColorNodeID(ColorNode->GetID());
        }
      }
      else if (this->GetColorMode ( ) == vtkMRMLFiberBundleDisplayNode::colorModeScalarData)
      {
        vtkDebugMacro("coloring with Scalar Data");
        IntermediatePolyData = this->PolyData;
        IntermediatePolyData->GetPointData()->SetActiveScalars(this->GetActiveScalarName());
      }
      else if (this->GetColorMode ( ) == vtkMRMLFiberBundleDisplayNode::colorModeScalar)
      {
        this->ScalarVisibilityOn( );
        this->TensorToColor->SetExtractScalar(1);

        switch ( DiffusionTensorDisplayPropertiesNode->GetColorGlyphBy( ))
          {
          case vtkMRMLDiffusionTensorDisplayPropertiesNode::FractionalAnisotropy:
            {
              vtkDebugMacro("coloring with FA");
              this->TensorToColor->ColorGlyphsByFractionalAnisotropy( );
            }
            break;
          case vtkMRMLDiffusionTensorDisplayPropertiesNode::LinearMeasure:
            {
              vtkDebugMacro("coloring with Cl");
              this->TensorToColor->ColorGlyphsByLinearMeasure( );
            }
            break;
          case vtkMRMLDiffusionTensorDisplayPropertiesNode::Trace:
            {
              vtkDebugMacro("coloring with trace ");
              this->TensorToColor->ColorGlyphsByTrace( );
            }
            break;
          case vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientation:
            {
              vtkDebugMacro("coloring with orientation ");
              this->TensorToColor->ColorGlyphsByOrientation( );
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
              this->TensorToColor->ColorGlyphsByPlanarMeasure( );
            }
            break;
          case vtkMRMLDiffusionTensorDisplayPropertiesNode::MaxEigenvalue:
            {
              vtkDebugMacro("coloring with max eigenval");
              this->TensorToColor->ColorGlyphsByMaxEigenvalue( );
            }
            break;
          case vtkMRMLDiffusionTensorDisplayPropertiesNode::MidEigenvalue:
            {
              vtkDebugMacro("coloring with mid eigenval");
              this->TensorToColor->ColorGlyphsByMidEigenvalue( );
            }
            break;
          case vtkMRMLDiffusionTensorDisplayPropertiesNode::MinEigenvalue:
            {
              vtkDebugMacro("coloring with min eigenval");
              this->TensorToColor->ColorGlyphsByMinEigenvalue( );
            }
            break;
          case vtkMRMLDiffusionTensorDisplayPropertiesNode::RelativeAnisotropy:
            {
              vtkDebugMacro("coloring with relative anisotropy");
              this->TensorToColor->ColorGlyphsByRelativeAnisotropy( );
            }
            break;
          default:
            {
            vtkDebugMacro("coloring with relative anisotropy");
            this->ScalarVisibilityOff( );
            this->TensorToColor->SetExtractScalar(0);
            }
            break;
            
          }

        IntermediatePolyData = this->TensorToColor->GetOutput();
      }

      }   
    else
      {
      this->ScalarVisibilityOff( );
      this->TensorToColor->SetExtractScalar(0);
      IntermediatePolyData = this->TensorToColor->GetOutput();
      }

    this->OutputPolyData = IntermediatePolyData;

    if ( this->GetAutoScalarRange() && this->GetScalarVisibility())
      {
          if (this->GetColorMode ( ) == vtkMRMLFiberBundleDisplayNode::colorModeScalar && this->TensorToColor->GetInput() != NULL)
          {
            double range[2];
            int ScalarInvariant =  0;
            if ( DiffusionTensorDisplayPropertiesNode )
            {
              ScalarInvariant = DiffusionTensorDisplayPropertiesNode->GetColorGlyphBy( );
            }

            if (DiffusionTensorDisplayPropertiesNode && vtkMRMLDiffusionTensorDisplayPropertiesNode::ScalarInvariantHasKnownScalarRange(ScalarInvariant))
            {
              vtkDebugMacro("Data-based automatic range");
              vtkMRMLDiffusionTensorDisplayPropertiesNode::ScalarInvariantKnownScalarRange(ScalarInvariant, range);
            } else {
              vtkDebugMacro("Data-based scalar range");
              this->TensorToColor->Update();
              this->OutputPolyData->GetScalarRange(range);
            }
            this->ScalarRange[0] = range[0];
            this->ScalarRange[1] = range[1];
          }
        else if (this->GetColorMode ( ) == vtkMRMLFiberBundleDisplayNode::colorModeScalarData) 
          {
            double range[2];
            this->OutputPolyData->GetScalarRange(range);
            this->ScalarRange[0] = range[0];
            this->ScalarRange[1] = range[1];
          }
      }

    }
}

