/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFiberBundleTubeDisplayNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include "vtkCellData.h"
#include "vtkPointData.h"

#include "vtkPolyDataTensorToColor.h"
#include "vtkTubeFilter.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLDiffusionTensorDisplayPropertiesNode.h"
#include "vtkMRMLFiberBundleTubeDisplayNode.h"
#include "vtkPolyDataColorLinesByOrientation.h"

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLFiberBundleTubeDisplayNode);


//----------------------------------------------------------------------------
vtkMRMLFiberBundleTubeDisplayNode::vtkMRMLFiberBundleTubeDisplayNode()
{
  this->TensorToColor = vtkPolyDataTensorToColor::New();
  this->ColorMode = vtkMRMLFiberBundleDisplayNode::colorModeScalar;
  this->ColorLinesByOrientation = vtkPolyDataColorLinesByOrientation::New();
  this->TubeFilter = vtkTubeFilter::New();
  this->TubeNumberOfSides = 6;
  this->TubeRadius = 0.5;
  
  this->TensorToColor->SetInput(this->TubeFilter->GetOutput());

}


//----------------------------------------------------------------------------
vtkMRMLFiberBundleTubeDisplayNode::~vtkMRMLFiberBundleTubeDisplayNode()
{
  this->RemoveObservers ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
  this->TubeFilter->Delete();
  this->TensorToColor->Delete();
  this->ColorLinesByOrientation->Delete();
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleTubeDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);
  of << indent << " tubeRadius =\"" << this->TubeRadius << "\"";
  of << indent << " tubeNumberOfSides =\"" << this->TubeNumberOfSides << "\"";
}



//----------------------------------------------------------------------------
void vtkMRMLFiberBundleTubeDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "tubeRadius")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> TubeRadius;
      }

    if (!strcmp(attName, "tubeNumberOfSides")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> TubeNumberOfSides;
      }
    }  

  this->EndModify(disabledModify);

}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLFiberBundleTubeDisplayNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLFiberBundleTubeDisplayNode *node = (vtkMRMLFiberBundleTubeDisplayNode *) anode;

  this->SetTubeNumberOfSides(node->TubeNumberOfSides);
  this->SetTubeRadius(node->TubeRadius);

  this->EndModify(disabledModify);

}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleTubeDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  //int idx;
  
  Superclass::PrintSelf(os,indent);
  os << indent << "TubeNumberOfSides:             " << this->TubeNumberOfSides << "\n";
  os << indent << "TubeRadius:             " << this->TubeRadius << "\n";
}

 
//----------------------------------------------------------------------------
void vtkMRMLFiberBundleTubeDisplayNode::SetPolyData(vtkPolyData *glyphPolyData)
{
  if ((this->PolyData != glyphPolyData) && (this->TubeFilter))
    {
    Superclass::SetPolyData(glyphPolyData);
    this->ColorLinesByOrientation->SetInput(glyphPolyData);
    this->TubeFilter->SetInput(glyphPolyData);
    }
}

//----------------------------------------------------------------------------
vtkPolyData* vtkMRMLFiberBundleTubeDisplayNode::GetPolyData()
{
  if ( this->TubeFilter && this->TubeFilter->GetInput() && this->TensorToColor)
  {
    return this->OutputPolyData;
  }
  else
  {
    return NULL;
  }

}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleTubeDisplayNode::UpdatePolyDataPipeline() 
{
  if (this->PolyData && this->Visibility)
    {
    vtkDebugMacro("Updating the PolyData Pipeline *****************************");
    // set display properties according to the tensor-specific display properties node for glyphs
    vtkMRMLDiffusionTensorDisplayPropertiesNode * DiffusionTensorDisplayPropertiesNode = this->GetDiffusionTensorDisplayPropertiesNode( );
    

    vtkPolyData *IntermediatePolyData;
    this->TubeFilter->SetInput(this->PolyData);
    IntermediatePolyData = this->TensorToColor->GetOutput();

    // set line coloring
    if (this->GetColorMode ( ) == vtkMRMLFiberBundleDisplayNode::colorModeSolid)
    {
      this->ScalarVisibilityOff( );
      this->TensorToColor->SetExtractScalar(0);
    }
    else if (this->GetColorMode ( ) == vtkMRMLFiberBundleDisplayNode::colorModeUseCellScalars)
    {
      this->ScalarVisibilityOn( );
      this->TensorToColor->SetExtractScalar(0); // force a copy of the data
      this->SetActiveScalarName("ClusterId");
      if (this->PolyData->GetCellData()->HasArray("ClusterId"))
      {
        this->PolyData->GetCellData()->GetArray("ClusterId")->GetRange(this->ScalarRange);
      }
    }
    else if (this->GetColorMode ( ) == vtkMRMLFiberBundleDisplayNode::colorModeMeanFiberOrientation)
    {
        vtkDebugMacro("Color by mean fiber orientation");
        this->ScalarVisibilityOn( );
        this->ColorLinesByOrientation->SetColorMode(ColorLinesByOrientation->colorModeMeanFiberOrientation);
        this->TubeFilter->SetInput(this->ColorLinesByOrientation->GetOutput());
        vtkMRMLDiffusionTensorDisplayPropertiesNode::ScalarInvariantKnownScalarRange(vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientation, this->ScalarRange);
        vtkMRMLNode* ColorNode = this->GetScene()->GetNodeByID("vtkMRMLColorTableNodeFullRainbow");
        if (ColorNode)
        {
          this->SetAndObserveColorNodeID(ColorNode->GetID());
        }

    }
    else if (this->GetColorMode ( ) == vtkMRMLFiberBundleDisplayNode::colorModePointFiberOrientation)
    {
        vtkDebugMacro("Color by segment orientation");
        this->ScalarVisibilityOn( );
        this->ColorLinesByOrientation->SetColorMode(ColorLinesByOrientation->colorModePointFiberOrientation);
        this->TubeFilter->SetInput(this->ColorLinesByOrientation->GetOutput());
        vtkMRMLDiffusionTensorDisplayPropertiesNode::ScalarInvariantKnownScalarRange(vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientation, this->ScalarRange);
        vtkMRMLNode* ColorNode = this->GetScene()->GetNodeByID("vtkMRMLColorTableNodeFullRainbow");
        if (ColorNode)
        {
          this->SetAndObserveColorNodeID(ColorNode->GetID());
        }
    }
    else if (this->GetColorMode ( ) == vtkMRMLFiberBundleDisplayNode::colorModeScalarData)
    {
      vtkDebugMacro("coloring with Scalar Data==============================");
      this->PolyData->GetPointData()->SetActiveScalars(this->GetActiveScalarName());
      this->TubeFilter->Update();
      this->TubeFilter->GetOutput()->GetPointData()->SetActiveScalars(this->GetActiveScalarName());
      IntermediatePolyData = this->TubeFilter->GetOutput();
    }
    else if ((this->GetColorMode ( ) == vtkMRMLFiberBundleDisplayNode::colorModeScalar) && (DiffusionTensorDisplayPropertiesNode != NULL))
    {
        this->ScalarVisibilityOn( );
        this->AutoScalarRangeOn( );
        this->TensorToColor->SetExtractScalar(1);

        switch ( DiffusionTensorDisplayPropertiesNode->GetColorGlyphBy( ))
          {
          case vtkMRMLDiffusionTensorDisplayPropertiesNode::FractionalAnisotropy:
            {
              vtkDebugMacro("coloring with FA==============================");
              this->TensorToColor->ColorGlyphsByFractionalAnisotropy( );
            }
            break;
          case vtkMRMLDiffusionTensorDisplayPropertiesNode::LinearMeasure:
            {
              vtkDebugMacro("coloring with Cl=============================");
              this->TensorToColor->ColorGlyphsByLinearMeasure( );
            }
            break;
          case vtkMRMLDiffusionTensorDisplayPropertiesNode::Trace:
            {
              vtkDebugMacro("coloring with trace =================");
              this->TensorToColor->ColorGlyphsByTrace( );
            }
            break;
          case vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientation:
            {
              vtkDebugMacro("coloring with orientation =================");
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
      }
    else
      {
      this->ScalarVisibilityOff( );
      this->TensorToColor->SetExtractScalar(0);
      }

    this->OutputPolyData = IntermediatePolyData;

    if (this->GetAutoScalarRange() && this->GetScalarVisibility() && this->TensorToColor->GetInput() != NULL )
      {

        double range[2];
        if (
            ((this->GetColorMode ( ) == vtkMRMLFiberBundleDisplayNode::colorModeScalar) ||
            (this->GetColorMode ( ) == vtkMRMLFiberBundleDisplayNode::colorModeFunctionOfScalar)) &&
            (this->TensorToColor->GetInput() != NULL)
           )
        {
          int ScalarInvariant = 0;
          if (DiffusionTensorDisplayPropertiesNode)
          {
           ScalarInvariant = DiffusionTensorDisplayPropertiesNode->GetColorGlyphBy( );
          }
          if (DiffusionTensorDisplayPropertiesNode && vtkMRMLDiffusionTensorDisplayPropertiesNode::ScalarInvariantHasKnownScalarRange(ScalarInvariant))
          {
            vtkMRMLDiffusionTensorDisplayPropertiesNode::ScalarInvariantKnownScalarRange(ScalarInvariant, range);
          } else {
            this->TensorToColor->Update();
            this->TensorToColor->GetOutput()->GetScalarRange(range);
          }
          this->ScalarRange[0] = range[0];
          this->ScalarRange[1] = range[1];
        }
        else if (this->GetColorMode() == vtkMRMLFiberBundleDisplayNode::colorModeScalarData)
        {
          this->PolyData->GetScalarRange(range);
          this->ScalarRange[0] = range[0];
          this->ScalarRange[1] = range[1];
        }
      }

    }
}

