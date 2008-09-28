/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLDiffusionTensorVolumeDisplayNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLDiffusionTensorVolumeDisplayNode.h"
#include "vtkMRMLScene.h"

#include "vtkMRMLDiffusionTensorVolumeSliceDisplayNode.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkDiffusionTensorGlyph.h"

#include "vtkSphereSource.h"

#include "vtkDiffusionTensorMathematicsSimple.h"

//------------------------------------------------------------------------------
vtkMRMLDiffusionTensorVolumeDisplayNode* vtkMRMLDiffusionTensorVolumeDisplayNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLDiffusionTensorVolumeDisplayNode");
  if(ret)
    {
    return (vtkMRMLDiffusionTensorVolumeDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLDiffusionTensorVolumeDisplayNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLDiffusionTensorVolumeDisplayNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLDiffusionTensorVolumeDisplayNode");
  if(ret)
    {
    return (vtkMRMLDiffusionTensorVolumeDisplayNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLDiffusionTensorVolumeDisplayNode;
}

//----------------------------------------------------------------------------
vtkMRMLDiffusionTensorVolumeDisplayNode::vtkMRMLDiffusionTensorVolumeDisplayNode()
{
 this->ScalarInvariant = vtkMRMLDiffusionTensorDisplayPropertiesNode::FractionalAnisotropy;
 this->DTIMathematics = vtkDiffusionTensorMathematicsSimple::New();
 this->DTIMathematicsAlpha = vtkDiffusionTensorMathematicsSimple::New();
 this->Threshold->SetInput( this->DTIMathematics->GetOutput());
 this->MapToWindowLevelColors->SetInput( this->DTIMathematics->GetOutput());

 this->DiffusionTensorGlyphFilter = vtkDiffusionTensorGlyph::New();
 vtkSphereSource *sphere = vtkSphereSource::New();
 sphere->Update();
 this->DiffusionTensorGlyphFilter->SetSource( sphere->GetOutput() );
 sphere->Delete();

 this->VisualizationMode = 0;
}

//----------------------------------------------------------------------------
vtkMRMLDiffusionTensorVolumeDisplayNode::~vtkMRMLDiffusionTensorVolumeDisplayNode()
{
  this->DTIMathematics->Delete();
  this->DTIMathematicsAlpha->Delete();

  this->DiffusionTensorGlyphFilter->Delete();
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
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLDiffusionTensorVolumeDisplayNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLDiffusionTensorVolumeDisplayNode *node = (vtkMRMLDiffusionTensorVolumeDisplayNode *) anode;
  this->SetScalarInvariant(node->ScalarInvariant);
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
void vtkMRMLDiffusionTensorVolumeDisplayNode::SetImageData(vtkImageData *imageData)
{
  this->DTIMathematics->SetInput(0, imageData);
  this->DTIMathematicsAlpha->SetInput(0, imageData);
}
//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeDisplayNode::UpdateImageDataPipeline()
{

  int operation = this->GetScalarInvariant();
  this->DTIMathematics->SetOperation(operation);
  switch (operation)
    {
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientation:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorMode:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientationMiddleEigenvector:
    case vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientationMinEigenvector:
      this->DTIMathematics->SetScaleFactor(1000.0);
      this->DTIMathematicsAlpha->SetOperation(
        vtkMRMLDiffusionTensorDisplayPropertiesNode::FractionalAnisotropy);
      this->Threshold->SetInput( this->DTIMathematicsAlpha->GetOutput());
      this->AppendComponents->RemoveAllInputs();
      this->AppendComponents->SetInputConnection(0, this->DTIMathematics->GetOutput()->GetProducerPort() );
      //this->AppendComponents->AddInputConnection(0, this->AlphaLogic->GetOutput()->GetProducerPort() );
      break;

    default:
      this->DTIMathematics->SetScaleFactor(1.0);
      this->Threshold->SetInput( this->DTIMathematics->GetOutput());
      this->MapToWindowLevelColors->SetInput( this->DTIMathematics->GetOutput());
      this->AppendComponents->RemoveAllInputs();
      this->AppendComponents->SetInputConnection(0, this->MapToColors->GetOutput()->GetProducerPort() );
      this->AppendComponents->AddInputConnection(0, this->AlphaLogic->GetOutput()->GetProducerPort() );
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
    glyphDTDPN->Delete();
    
    for (int i=0; i<3; i++)
      {
      vtkMRMLDiffusionTensorVolumeSliceDisplayNode *node = vtkMRMLDiffusionTensorVolumeSliceDisplayNode::New();
      node->SetVisibility(0);
      if (this->GetScene())
        {
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

        node->SetAndObserveColorNodeID("vtkMRMLColorTableNodeRainbow");

        volumeNode->AddAndObserveDisplayNodeID(node->GetID());
        
        }
      }
   }
}

//----------------------------------------------------------------------------
vtkImageData* vtkMRMLDiffusionTensorVolumeDisplayNode::GetImageData()
{
  this->UpdateImageDataPipeline();
  if (this->DTIMathematics->GetInput() == NULL)
    {
    return NULL;
    }
  this->AppendComponents->Update();
  return this->AppendComponents->GetOutput();
};
