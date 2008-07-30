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

#include "vtkTransformPolyDataFilter.h"

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
  :vtkMRMLGlyphableVolumeSliceDisplayNode()
{

  // Enumerated
  this->DiffusionTensorDisplayPropertiesNode = NULL;
  this->DiffusionTensorDisplayPropertiesNodeID = NULL;


  this->DiffusionTensorGlyphFilter = vtkDiffusionTensorGlyph::New();
  this->DiffusionTensorGlyphFilter->SetResolution (1);

  this->ColorMode = this->colorModeScalar;
/*  
  this->SliceToXYTransformer = vtkTransformPolyDataFilter::New();

  this->SliceToXYTransform = vtkTransform::New();
  
  this->SliceToXYMatrix = vtkMatrix4x4::New();
  this->SliceToXYMatrix->Identity();
  this->SliceToXYTransform->PreMultiply();
  this->SliceToXYTransform->SetMatrix(this->SliceToXYMatrix);

  this->SliceToXYTransformer->SetTransform(this->SliceToXYTransform);
*/
  this->SliceToXYTransformer->SetInput(this->DiffusionTensorGlyphFilter->GetOutput());
}


//----------------------------------------------------------------------------
vtkMRMLDiffusionTensorVolumeSliceDisplayNode::~vtkMRMLDiffusionTensorVolumeSliceDisplayNode()
{
  this->RemoveObservers ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
  this->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(NULL);
  this->DiffusionTensorGlyphFilter->Delete();
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeSliceDisplayNode::WriteXML(ostream& of, int nIndent)
{

  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

//  of << indent << " colorMode =\"" << this->ColorMode << "\"";

  if (this->DiffusionTensorDisplayPropertiesNodeID != NULL) 
    {
    of << indent << " DiffusionTensorDisplayPropertiesNodeRef=\"" << this->DiffusionTensorDisplayPropertiesNodeID << "\"";
    }


}


//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeSliceDisplayNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
/*
    if (!strcmp(attName, "colorMode")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> ColorMode;
      }
    else 
*/
    if (!strcmp(attName, "DiffusionTensorDisplayPropertiesNodeRef")) 
      {
      this->SetDiffusionTensorDisplayPropertiesNodeID(attValue);
      //this->Scene->AddReferencedNodeID(this->FiberLineDiffusionTensorDisplayPropertiesNodeID, this);
      }
    }  


}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLDiffusionTensorVolumeSliceDisplayNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLDiffusionTensorVolumeSliceDisplayNode *node = (vtkMRMLDiffusionTensorVolumeSliceDisplayNode *) anode;

  this->SetDiffusionTensorDisplayPropertiesNodeID(node->DiffusionTensorDisplayPropertiesNodeID);
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeSliceDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
 //int idx;
  
  Superclass::PrintSelf(os,indent);
//  os << indent << "ColorMode:             " << this->ColorMode << "\n";
}
//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeSliceDisplayNode::SetSliceTensorRotationMatrix(vtkMatrix4x4 *matrix)
{
  if (this->DiffusionTensorGlyphFilter)
    {
    this->DiffusionTensorGlyphFilter->SetTensorRotationMatrix(matrix);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeSliceDisplayNode::SetSlicePositionMatrix(vtkMatrix4x4 *matrix)
{
  Superclass::SetSlicePositionMatrix(matrix);
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
 // if (this->DiffusionTensorGlyphFilter && this->Visibility != 0)
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
vtkPolyData* vtkMRMLDiffusionTensorVolumeSliceDisplayNode::GetPolyDataTransformedToSlice()
{

  if (this->DiffusionTensorGlyphFilter && this->Visibility != 0)
    {
    this->UpdatePolyDataPipeline();
    this->DiffusionTensorGlyphFilter->Update();
    this->SliceToXYTransformer->Update();
    return this->SliceToXYTransformer->GetOutput();
    }
  else
    {
    return NULL;
    }
}
//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeSliceDisplayNode::UpdatePolyDataPipeline() 
{
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
      this->DiffusionTensorGlyphFilter->SetResolution(1);
      this->DiffusionTensorGlyphFilter->SetDimensionResolution( DiffusionTensorDisplayNode->GetLineGlyphResolution(), DiffusionTensorDisplayNode->GetLineGlyphResolution());
      this->DiffusionTensorGlyphFilter->SetScaleFactor( DiffusionTensorDisplayNode->GetGlyphScaleFactor( ) );
      
      this->DiffusionTensorGlyphFilter->SetSource( DiffusionTensorDisplayNode->GetGlyphSource( ) );
      
      vtkDebugMacro("setting glyph geometry" << DiffusionTensorDisplayNode->GetGlyphGeometry( ) );
      
      // set glyph coloring
      if (this->GetColorMode ( ) == colorModeSolid)
        {
        this->ScalarVisibilityOff( );
        }
      else  
        {
        if (this->GetColorMode ( ) == colorModeScalar)
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
   
 if (this->GetScalarVisibility() && this->GetAutoScalarRange())
  {
  //this->DiffusionTensorGlyphFilter->Update();
  double *range = this->DiffusionTensorGlyphFilter->GetOutput()->GetScalarRange();
  this->ScalarRange[0] = range[0];
  this->ScalarRange[1] = range[1];
  // avoid Set not to cause event loops
  //this->SetScalarRange( this->DiffusionTensorGlyphFilter->GetOutput()->GetScalarRange() );
  }

}

//----------------------------------------------------------------------------
vtkMRMLDiffusionTensorDisplayPropertiesNode* vtkMRMLDiffusionTensorVolumeSliceDisplayNode::GetDiffusionTensorDisplayPropertiesNode ( )
{
  vtkMRMLDiffusionTensorDisplayPropertiesNode* node = NULL;

  // Find the node corresponding to the ID we have saved.
  if  ( this->GetScene ( ) && this->GetDiffusionTensorDisplayPropertiesNodeID ( ) )
    {
    vtkMRMLNode* cnode = this->GetScene ( ) -> GetNodeByID ( this->DiffusionTensorDisplayPropertiesNodeID );
    node = vtkMRMLDiffusionTensorDisplayPropertiesNode::SafeDownCast ( cnode );
    }

  return node;
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeSliceDisplayNode::SetAndObserveDiffusionTensorDisplayPropertiesNodeID ( const char *ID )
{
  // Stop observing any old node
  vtkSetAndObserveMRMLObjectMacro ( this->DiffusionTensorDisplayPropertiesNode, NULL );

  // Set the ID. This is the "ground truth" reference to the node.
  this->SetDiffusionTensorDisplayPropertiesNodeID ( ID );

  // Get the node corresponding to the ID. This pointer is only to observe the object.
  vtkMRMLNode *cnode = this->GetDiffusionTensorDisplayPropertiesNode ( );

  // Observe the node using the pointer.
  vtkSetAndObserveMRMLObjectMacro ( this->DiffusionTensorDisplayPropertiesNode , cnode );

}
//---------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeSliceDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  return;
}

//-----------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeSliceDisplayNode::UpdateScene(vtkMRMLScene *scene)
{
   Superclass::UpdateScene(scene);

   this->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(this->GetDiffusionTensorDisplayPropertiesNodeID());
}

//-----------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeSliceDisplayNode::UpdateReferences()
{
  Superclass::UpdateReferences();

  if (this->DiffusionTensorDisplayPropertiesNodeID != NULL && this->Scene->GetNodeByID(this->DiffusionTensorDisplayPropertiesNodeID) == NULL)
    {
    this->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(NULL);
    }
}


//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeSliceDisplayNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  if (this->DiffusionTensorDisplayPropertiesNodeID && !strcmp(oldID, this->DiffusionTensorDisplayPropertiesNodeID))
    {
    this->SetDiffusionTensorDisplayPropertiesNodeID(newID);
    }
}




