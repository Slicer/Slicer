/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFiberBundleLineDisplayNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLDiffusionTensorDisplayPropertiesNode.h"
#include "vtkMRMLFiberBundleLineDisplayNode.h"
#include "vtkMRMLScene.h"

// Teem includes
#include "vtkPolyDataColorLinesByOrientation.h"
#include "vtkPolyDataTensorToColor.h"

// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkAssignAttribute.h>
#include <vtkCallbackCommand.h>
#include <vtkCellData.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLFiberBundleLineDisplayNode);

//----------------------------------------------------------------------------
vtkMRMLFiberBundleLineDisplayNode::vtkMRMLFiberBundleLineDisplayNode()
{
  this->TensorToColor = vtkPolyDataTensorToColor::New();
  this->TensorToColor->SetInputConnection(this->Superclass::GetOutputPolyDataConnection());
  this->ColorLinesByOrientation = vtkPolyDataColorLinesByOrientation::New();
  this->ColorLinesByOrientation->SetInputConnection(this->TensorToColor->GetOutputPort());
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
void vtkMRMLFiberBundleLineDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkMRMLFiberBundleLineDisplayNode::GetOutputPolyDataConnection()
{
  //if (this->ColorLinesByOrientation)
  //  {
  //  return this->OutputPolyData;
  //  }
  // return NULL;
  vtkMRMLDiffusionTensorDisplayPropertiesNode * diffusionTensorDisplayPropertiesNode =
    this->GetDiffusionTensorDisplayPropertiesNode();

  vtkAlgorithmOutput* outputPort = NULL;
  if (diffusionTensorDisplayPropertiesNode != NULL)
    {
    switch (this->GetColorMode())
      {
      case vtkMRMLFiberBundleDisplayNode::colorModeSolid:
      case vtkMRMLFiberBundleDisplayNode::colorModeScalar:
        outputPort = this->TensorToColor->GetOutputPort();
        break;
      case vtkMRMLFiberBundleDisplayNode::colorModeUseCellScalars:
        if (!this->GetInputPolyData()->GetCellData()->HasArray(this->GetActiveScalarName()) ||
            this->GetActiveScalarName() == this->ColorLinesByOrientation->GetScalarArrayName())
          {
          outputPort = this->ColorLinesByOrientation->GetOutputPort();
          }
        else
          {
          outputPort = this->Superclass::GetOutputPolyDataConnection();
          }
        break;
      case vtkMRMLFiberBundleDisplayNode::colorModeMeanFiberOrientation:
      case vtkMRMLFiberBundleDisplayNode::colorModePointFiberOrientation:
        outputPort = this->ColorLinesByOrientation->GetOutputPort();
        break;
      case vtkMRMLFiberBundleDisplayNode::colorModeScalarData:
        outputPort = this->Superclass::GetOutputPolyDataConnection();
        break;
      default:
        outputPort = this->Superclass::GetOutputPolyDataConnection();
      }
    }
  else
    {
    outputPort = this->TensorToColor->GetOutputPort();
    }

  return outputPort;
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleLineDisplayNode::UpdatePolyDataPipeline()
{
  this->Superclass::UpdatePolyDataPipeline();
  this->TensorToColor->SetInputConnection(
    this->Superclass::GetOutputPolyDataConnection());

  if (!this->Visibility)
    {
    return;
    }

  vtkDebugMacro("Updating the PolyData Pipeline *****************************");
  // set display properties according to the tensor-specific display properties node for glyphs
  vtkMRMLDiffusionTensorDisplayPropertiesNode * DiffusionTensorDisplayPropertiesNode =
    this->GetDiffusionTensorDisplayPropertiesNode( );

  if (DiffusionTensorDisplayPropertiesNode != NULL)
    {
    // TO DO: need filter to calculate FA, average FA, etc. as requested
    // set line coloring
    if (this->GetColorMode ( ) == vtkMRMLFiberBundleDisplayNode::colorModeSolid)
      {
      this->ScalarVisibilityOff( );
      this->TensorToColor->SetExtractScalar(0);
      }
    else if (this->GetColorMode ( ) == vtkMRMLFiberBundleDisplayNode::colorModeUseCellScalars)
      {
      this->ScalarVisibilityOn( );
      this->TensorToColor->SetExtractScalar(0);
      if (!this->GetInputPolyData()->GetCellData()->HasArray(this->GetActiveScalarName()) ||
          this->GetActiveScalarName() == this->ColorLinesByOrientation->GetScalarArrayName())
        {
        this->SetActiveScalarName(this->ColorLinesByOrientation->GetScalarArrayName());
        vtkMRMLNode* colorNode = this->GetScene()->GetNodeByID("vtkMRMLColorTableNodeFullRainbow");
        if (colorNode)
          {
          this->SetAndObserveColorNodeID(colorNode->GetID());
          }
        }
      }
    else if (this->GetColorMode ( ) == vtkMRMLFiberBundleDisplayNode::colorModeMeanFiberOrientation)
      {
      vtkDebugMacro("Color by mean fiber orientation");
      this->ScalarVisibilityOn( );
      this->ColorLinesByOrientation->SetColorMode(ColorLinesByOrientation->colorModeMeanFiberOrientation);
      vtkMRMLNode* colorNode = this->GetScene()->GetNodeByID("vtkMRMLColorTableNodeFullRainbow");
      if (colorNode)
        {
        this->SetAndObserveColorNodeID(colorNode->GetID());
        }
      }
    else if (this->GetColorMode ( ) == vtkMRMLFiberBundleDisplayNode::colorModePointFiberOrientation)
      {
      vtkDebugMacro("Color by mean fiber orientation");
      this->ScalarVisibilityOn( );
      this->ColorLinesByOrientation->SetColorMode(
        this->ColorLinesByOrientation->colorModePointFiberOrientation);
      vtkMRMLNode* colorNode = this->GetScene()->GetNodeByID("vtkMRMLColorTableNodeFullRainbow");
      if (colorNode)
        {
        this->SetAndObserveColorNodeID(colorNode->GetID());
        }
      }
    else if (this->GetColorMode ( ) == vtkMRMLFiberBundleDisplayNode::colorModeScalarData)
      {
      vtkDebugMacro("coloring with Scalar Data");
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
          vtkMRMLNode* colorNode = this->GetScene()->GetNodeByID("vtkMRMLColorTableNodeFullRainbow");
          if (colorNode)
            {
            this->SetAndObserveColorNodeID(colorNode->GetID());
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
    }
  else
    {
    this->ScalarVisibilityOff( );
    this->TensorToColor->SetExtractScalar(0);
    }

    if (this->GetAutoScalarRange() &&
      this->GetScalarVisibility())
      {
      double range[2] = {0., 1.};
      if ((this->GetColorMode ( ) == vtkMRMLFiberBundleDisplayNode::colorModeScalar) ||
          (this->GetColorMode ( ) == vtkMRMLFiberBundleDisplayNode::colorModeFunctionOfScalar))
        {
        int scalarInvariant = 0;
        if (DiffusionTensorDisplayPropertiesNode)
          {
          scalarInvariant = DiffusionTensorDisplayPropertiesNode->GetColorGlyphBy( );
          }
        if (DiffusionTensorDisplayPropertiesNode &&
            vtkMRMLDiffusionTensorDisplayPropertiesNode::ScalarInvariantHasKnownScalarRange(
              scalarInvariant))
          {
          vtkMRMLDiffusionTensorDisplayPropertiesNode::ScalarInvariantKnownScalarRange(
            scalarInvariant, range);
          }
        else if (this->GetInputPolyData())
          {
#if (VTK_MAJOR_VERSION <= 5)
          this->GetOutputPolyData()->Update();
#else
          this->GetOutputPolyDataConnection()->GetProducer()->Update();
#endif
          vtkPointData *pointData = this->GetOutputPolyData()->GetPointData();
          if (pointData &&
              pointData->GetArray(this->GetActiveScalarName()))
            {
            double *activeScalarRange = pointData->GetArray(
              this->GetActiveScalarName())->GetRange();
            if (activeScalarRange)
              {
              range[0] = activeScalarRange[0];
              range[1] = activeScalarRange[1];
              }
            }
          }
        }
      if ((this->GetColorMode ( ) == vtkMRMLFiberBundleDisplayNode::colorModeMeanFiberOrientation) ||
          (this->GetColorMode ( ) == vtkMRMLFiberBundleDisplayNode::colorModePointFiberOrientation))
        {
          vtkMRMLDiffusionTensorDisplayPropertiesNode::ScalarInvariantKnownScalarRange(
            vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientation, range);
        }
      else if (this->GetColorMode() == vtkMRMLFiberBundleDisplayNode::colorModeScalarData &&
               this->GetInputPolyData())
        {
#if (VTK_MAJOR_VERSION <= 5)
        this->GetInputPolyData()->Update();
#else
        this->GetInputPolyDataConnection()->GetProducer()->Update();
#endif
        vtkPointData *pointData = this->GetOutputPolyData()->GetPointData();
        if (pointData &&
            pointData->GetArray(this->GetActiveScalarName()))
          {
          double *activeScalarRange = pointData->GetArray(
            this->GetActiveScalarName())->GetRange();
          if (activeScalarRange)
            {
            range[0] = activeScalarRange[0];
            range[1] = activeScalarRange[1];
            }
          }
        }
      //this->ScalarRange[0] = range[0];
      //this->ScalarRange[1] = range[1];
      this->SetScalarRange(range);
      }
}

