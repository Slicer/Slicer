/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLDiffusionTensorVolumeDisplayNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLDiffusionTensorVolumeDisplayNode.h"
#include "vtkMRMLDiffusionTensorVolumeSliceDisplayNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLVolumeNode.h"

// Teem includes
#include <vtkDiffusionTensorGlyph.h>
#include <vtkDiffusionTensorMathematics.h>

// VTK includes
#include <vtkImageAppendComponents.h>
#include <vtkImageCast.h>
#include <vtkImageData.h>
#include <vtkImageExtractComponents.h>
#include <vtkImageLogic.h>
#include <vtkImageMathematics.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkImageShiftScale.h>
#include <vtkImageThreshold.h>
#include <vtkObjectFactory.h>
#include <vtkSphereSource.h>

// STD includes

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLDiffusionTensorVolumeDisplayNode);

//----------------------------------------------------------------------------
vtkMRMLDiffusionTensorVolumeDisplayNode::vtkMRMLDiffusionTensorVolumeDisplayNode()
{
 this->ScalarInvariant = vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientation;
 this->DTIMathematics = vtkDiffusionTensorMathematics::New();
 this->DTIMathematicsAlpha = vtkDiffusionTensorMathematics::New();
 this->Threshold->SetInput( this->DTIMathematics->GetOutput());
 this->MapToWindowLevelColors->SetInput( this->DTIMathematics->GetOutput());

 this->ShiftScale = vtkImageShiftScale::New();
 this->ShiftScale->SetOutputScalarTypeToUnsignedChar();
 this->ShiftScale->SetClampOverflow(1);

 this->ExtractComponents = vtkImageExtractComponents::New();
 this->ExtractComponents->SetComponents(0,1,2);

 this->ImageCast = vtkImageCast::New();
 this->ImageCast->SetOutputScalarTypeToUnsignedChar();

 this->ImageMath  = vtkImageMathematics::New();
 this->ImageMath->SetOperationToMultiplyByK();
 this->ImageMath->SetConstantK(255);

 this->DiffusionTensorGlyphFilter = vtkDiffusionTensorGlyph::New();
 vtkSphereSource *sphere = vtkSphereSource::New();
 sphere->Update();
 this->DiffusionTensorGlyphFilter->SetSource( sphere->GetOutput() );
 sphere->Delete();

 this->VisualizationMode = 0;
 this->AutoScalarRange = 1;

}

//----------------------------------------------------------------------------
vtkMRMLDiffusionTensorVolumeDisplayNode::~vtkMRMLDiffusionTensorVolumeDisplayNode()
{
  this->DTIMathematics->Delete();
  this->DTIMathematicsAlpha->Delete();

  this->DiffusionTensorGlyphFilter->Delete();
  this->ShiftScale->Delete();
  this->ExtractComponents->Delete();
  this->ImageMath->Delete();
  this->ImageCast->Delete();
}


//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeDisplayNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << " scalarInvariant=\"" << this->ScalarInvariant << "\"";

}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "scalarInvariant")) 
      {
      int scalarInvariant;
      std::stringstream ss;
      ss << attValue;
      ss >> scalarInvariant;
      this->SetScalarInvariant(scalarInvariant);
      }

    }
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLDiffusionTensorVolumeDisplayNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLDiffusionTensorVolumeDisplayNode *node = (vtkMRMLDiffusionTensorVolumeDisplayNode *) anode;
  this->SetScalarInvariant(node->ScalarInvariant);

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);

  os << indent << "ScalarInvariant:             " << this->ScalarInvariant << "\n";

}

//---------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  return;
}

//-----------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeDisplayNode::UpdateScene(vtkMRMLScene *scene)
{
   Superclass::UpdateScene(scene);

}

//-----------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeDisplayNode::UpdateReferences()
{
  Superclass::UpdateReferences();
}


//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeDisplayNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  Superclass::UpdateReferenceID(oldID, newID);
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeDisplayNode::UpdateImageDataPipeline()
{
  int scalarInvariant = this->GetScalarInvariant();
  this->DTIMathematics->SetOperation(scalarInvariant);
  switch (scalarInvariant)
    {
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientation:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorMode:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientationMiddleEigenvector:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientationMinEigenvector:
      {
      // alpha
      this->DTIMathematics->SetScaleFactor(1000.0);
      this->DTIMathematicsAlpha->SetOperation(
        vtkMRMLDiffusionTensorDisplayPropertiesNode::FractionalAnisotropy);
      this->ImageMath->SetInput( this->DTIMathematicsAlpha->GetOutput());
      this->ImageCast->SetInput( this->ImageMath->GetOutput());
      this->Threshold->SetInput( this->ImageCast->GetOutput());

      // window/level
      this->ShiftScale->SetInput(this->DTIMathematics->GetOutput());
      double halfWindow = (this->GetWindow() / 2.);
      double min = this->GetLevel() - halfWindow;
      this->ShiftScale->SetShift ( -min );
      this->ShiftScale->SetScale ( 255. / (this->GetWindow()) );

      this->ExtractComponents->SetInput(this->ShiftScale->GetOutput());
      if (this->AppendComponents->GetInputConnection(0, 0) != this->ExtractComponents->GetOutput()->GetProducerPort() ||
          this->AppendComponents->GetInputConnection(0, 1) != this->Threshold->GetOutput()->GetProducerPort())
        {
        this->AppendComponents->RemoveAllInputs();
        this->AppendComponents->SetInputConnection(0, this->ExtractComponents->GetOutput()->GetProducerPort());
        this->AppendComponents->AddInputConnection(0, this->Threshold->GetOutput()->GetProducerPort() );
        }
      break;
      }
    default:
      this->DTIMathematics->SetScaleFactor(1.0);
      this->Threshold->SetInput( this->DTIMathematics->GetOutput());
      this->MapToWindowLevelColors->SetInput( this->DTIMathematics->GetOutput());
      if (this->AppendComponents->GetInputConnection(0, 0) != this->MapToColors->GetOutput()->GetProducerPort() ||
          this->AppendComponents->GetInputConnection(0, 1) != this->AlphaLogic->GetOutput()->GetProducerPort())
        {
        this->AppendComponents->RemoveAllInputs();
        this->AppendComponents->SetInputConnection(0, this->MapToColors->GetOutput()->GetProducerPort() );
        this->AppendComponents->AddInputConnection(0, this->AlphaLogic->GetOutput()->GetProducerPort() );
        }
      break;
    }

  Superclass::UpdateImageDataPipeline();

}

//----------------------------------------------------------------------------
std::vector< vtkMRMLGlyphableVolumeSliceDisplayNode*> vtkMRMLDiffusionTensorVolumeDisplayNode::GetSliceGlyphDisplayNodes( vtkMRMLVolumeNode* volumeNode )
{
  std::vector< vtkMRMLGlyphableVolumeSliceDisplayNode*> nodes;
  int nnodes = volumeNode->GetNumberOfDisplayNodes();
  vtkMRMLDiffusionTensorVolumeSliceDisplayNode *node = NULL;
  for (int n=0; n<nnodes; n++)
    {
    node = vtkMRMLDiffusionTensorVolumeSliceDisplayNode::SafeDownCast(volumeNode->GetNthDisplayNode(n));
    if (node) 
      {
      nodes.push_back(node);
      }
    }
  return nodes;
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeDisplayNode::AddSliceGlyphDisplayNodes( vtkMRMLVolumeNode* volumeNode )
{
  std::vector< vtkMRMLGlyphableVolumeSliceDisplayNode*> nodes = this->GetSliceGlyphDisplayNodes( volumeNode );
  if (nodes.size() == 0)
    {
    vtkMRMLDiffusionTensorDisplayPropertiesNode *glyphDTDPN = vtkMRMLDiffusionTensorDisplayPropertiesNode::New();
    this->GetScene()->AddNode(glyphDTDPN);
    int modifyState = glyphDTDPN->StartModify();
    glyphDTDPN->SetLineGlyphResolution(5);
    glyphDTDPN->EndModify(modifyState);
    glyphDTDPN->Delete();
    
    for (int i=0; i<3; i++)
      {
      if (this->GetScene())
        {
        vtkMRMLDiffusionTensorVolumeSliceDisplayNode *node = vtkMRMLDiffusionTensorVolumeSliceDisplayNode::New();
        if (i == 0) 
          {
          node->SetName("Red");
          }
        else if (i == 1) 
          {
          node->SetName("Yellow");
          }
        else if (i == 2) 
          {
          node->SetName("Green");
          }

        this->GetScene()->AddNode(node);
        node->Delete();

        int modifyState2 = node->StartModify();     
        node->SetVisibility(0);

        node->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(glyphDTDPN->GetID());

        node->SetAndObserveColorNodeID("vtkMRMLColorTableNodeRainbow");
        
        node->EndModify(modifyState2);

        volumeNode->AddAndObserveDisplayNodeID(node->GetID());
        
        }
      }
   }
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeDisplayNode::SetInputToImageDataPipeline(vtkImageData *imageData)
{
  this->DTIMathematics->SetInput(imageData);
  this->DTIMathematicsAlpha->SetInput(0, imageData);
  //this->ShiftScale->SetInput(0, imageData );
};

//----------------------------------------------------------------------------
vtkImageData* vtkMRMLDiffusionTensorVolumeDisplayNode::GetInputImageData()
{
  return vtkImageData::SafeDownCast(this->DTIMathematics->GetInput());
}

//----------------------------------------------------------------------------
vtkImageData* vtkMRMLDiffusionTensorVolumeDisplayNode::GetOutputImageData()
{
  return this->AppendComponents->GetOutput();
}

//----------------------------------------------------------------------------
vtkImageData* vtkMRMLDiffusionTensorVolumeDisplayNode::GetBackgroundImageData()
{
  switch (this->GetScalarInvariant())
    {
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientation:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorMode:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientationMiddleEigenvector:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientationMinEigenvector:
      {
      return 0;
      }
    default:
      return this->Superclass::GetBackgroundImageData();
    }
}

//---------------------------------------------------------------------------
vtkImageData* vtkMRMLDiffusionTensorVolumeDisplayNode::GetScalarImageData()
{
  return vtkImageData::SafeDownCast(this->DTIMathematics->GetOutput());
}

//---------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeDisplayNode
::GetDisplayScalarRange(double range[2])
{
  const int ScalarInvariant = this->GetScalarInvariant();
  if (vtkMRMLDiffusionTensorDisplayPropertiesNode::ScalarInvariantHasKnownScalarRange(ScalarInvariant))
  {
    vtkMRMLDiffusionTensorDisplayPropertiesNode::ScalarInvariantKnownScalarRange(ScalarInvariant, range);
  } else {
    this->DTIMathematics->Update();
    return this->GetScalarImageData()->GetScalarRange(range);
  }

}

//----------------------------------------------------------------------------
std::vector<int> vtkMRMLDiffusionTensorVolumeDisplayNode::GetSupportedColorModes()
{
  std::vector<int> modes;
  modes.push_back(vtkMRMLDiffusionTensorDisplayPropertiesNode::FractionalAnisotropy);
  modes.push_back(vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientation);
  modes.push_back(vtkMRMLDiffusionTensorDisplayPropertiesNode::Trace);
  modes.push_back(vtkMRMLDiffusionTensorDisplayPropertiesNode::LinearMeasure);
  modes.push_back(vtkMRMLDiffusionTensorDisplayPropertiesNode::PlanarMeasure);
  modes.push_back(vtkMRMLDiffusionTensorDisplayPropertiesNode::SphericalMeasure);
  modes.push_back(vtkMRMLDiffusionTensorDisplayPropertiesNode::RelativeAnisotropy);
  modes.push_back(vtkMRMLDiffusionTensorDisplayPropertiesNode::ParallelDiffusivity);
  modes.push_back(vtkMRMLDiffusionTensorDisplayPropertiesNode::PerpendicularDiffusivity);
  modes.push_back(vtkMRMLDiffusionTensorDisplayPropertiesNode::MaxEigenvalue);
  modes.push_back(vtkMRMLDiffusionTensorDisplayPropertiesNode::MidEigenvalue);
  modes.push_back(vtkMRMLDiffusionTensorDisplayPropertiesNode::MinEigenvalue);
  return modes;
}

//----------------------------------------------------------------------------
int vtkMRMLDiffusionTensorVolumeDisplayNode::GetNumberOfScalarInvariants()
{
  static std::vector<int> modes = vtkMRMLDiffusionTensorVolumeDisplayNode::GetSupportedColorModes();
  return modes.size();
}

//----------------------------------------------------------------------------
int vtkMRMLDiffusionTensorVolumeDisplayNode::GetNthScalarInvariant(int i)
{
  static std::vector<int> modes = vtkMRMLDiffusionTensorVolumeDisplayNode::GetSupportedColorModes();
  return modes[i];
}

