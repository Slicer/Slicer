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
 this->DiffusionTensorDisplayPropertiesNode = NULL;
 this->DiffusionTensorDisplayPropertiesNodeID = NULL;
}

//----------------------------------------------------------------------------
vtkMRMLDiffusionTensorVolumeDisplayNode::~vtkMRMLDiffusionTensorVolumeDisplayNode()
{
  this->DTIMathematics->Delete();
  this->DTIMathematicsAlpha->Delete();

  this->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(NULL); 
  this->DiffusionTensorGlyphFilter->Delete();
}


//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeDisplayNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  std::stringstream ss;
 if (this->DiffusionTensorDisplayPropertiesNodeID != NULL)
    {
    ss << this->DiffusionTensorDisplayPropertiesNodeID;
    of << indent << " diffusionTensorDisplayPropertiesNodeID=\"" << ss.str() << "\"";
    }
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
    if (!strcmp(attName, "diffusionTensorDisplayPropertiesNodeID"))
      {
      this->SetDiffusionTensorDisplayPropertiesNodeID(attValue);
      //this->Scene->AddReferencedNodeID(this->DiffusionTensorDisplayPropertiesNodeID, this);
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

  this->SetDiffusionTensorDisplayPropertiesNodeID(node->DiffusionTensorDisplayPropertiesNodeID);
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);


  os << indent << "DiffusionTensorDisplayPropertiesNodeID:  ";
  if ( this->DiffusionTensorDisplayPropertiesNodeID )
    {
    os << indent << this->DiffusionTensorDisplayPropertiesNodeID << "\n";
    }
  else
    {
    os << indent << "NULL\n";
    }
}

//---------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  vtkMRMLDiffusionTensorDisplayPropertiesNode *pnode = this->GetDiffusionTensorDisplayPropertiesNode();
  if (pnode != NULL && pnode == vtkMRMLDiffusionTensorDisplayPropertiesNode::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
    this->InvokeEvent(vtkCommand::ModifiedEvent, NULL);
    }
  return;
}

//-----------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeDisplayNode::UpdateScene(vtkMRMLScene *scene)
{
   Superclass::UpdateScene(scene);

   this->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(this->GetDiffusionTensorDisplayPropertiesNodeID());

}

//-----------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeDisplayNode::UpdateReferences()
{
  Superclass::UpdateReferences();

  if (this->DiffusionTensorDisplayPropertiesNodeID != NULL && this->Scene->GetNodeByID(this->DiffusionTensorDisplayPropertiesNodeID) == NULL)
    {
    this->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(NULL);
    }
}
//-----------------------------------------------------------
vtkPolyData* vtkMRMLDiffusionTensorVolumeDisplayNode::ExecuteGlyphPipeLineAndGetPolyData( vtkImageData* imageData )
{
  if ( true || this->GetVisualizationMode()==this->visModeGlyph || this->GetVisualizationMode()==this->visModeBoth )
  {
    vtkErrorMacro("Showing Tensor Glyph from data");
    imageData->PrintSelf( std::cout, vtkIndent() );
    this->DiffusionTensorGlyphFilter->SetInput( imageData );
    this->DiffusionTensorGlyphFilter->ClampScalingOff();

            // TO DO: implement max # ellipsoids, random sampling features
   this->DiffusionTensorGlyphFilter->SetResolution(2);
          
   if (this->GetDiffusionTensorDisplayPropertiesNode())
     {
     this->DiffusionTensorGlyphFilter->SetSource( this->GetDiffusionTensorDisplayPropertiesNode()->GetGlyphSource()   );
     }

    //this->DiffusionTensorGlyphFilter->SetScaleFactor( this->GetDiffusionTensorDisplayPropertiesNode()->GetGlyphScaleFactor()  );
    this->DiffusionTensorGlyphFilter->SetScaleFactor(100);

    this->DiffusionTensorGlyphFilter->ColorGlyphsByFractionalAnisotropy( );
    this->DiffusionTensorGlyphFilter->Update ( );
   
    return this->DiffusionTensorGlyphFilter->GetOutput();
  } else {
    return NULL;
  }
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeDisplayNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  if (this->DiffusionTensorDisplayPropertiesNodeID && !strcmp(oldID, this->DiffusionTensorDisplayPropertiesNodeID))
    {
    this->SetDiffusionTensorDisplayPropertiesNodeID(newID);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeDisplayNode::SetAndObserveDiffusionTensorDisplayPropertiesNodeID ( const char *ID )
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

//----------------------------------------------------------------------------
vtkMRMLDiffusionTensorDisplayPropertiesNode* vtkMRMLDiffusionTensorVolumeDisplayNode::GetDiffusionTensorDisplayPropertiesNode ( )
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
void vtkMRMLDiffusionTensorVolumeDisplayNode::SetImageData(vtkImageData *imageData)
{
  this->DTIMathematics->SetInput(0, imageData);
  this->DTIMathematicsAlpha->SetInput(0, imageData);
}
//----------------------------------------------------------------------------
void vtkMRMLDiffusionTensorVolumeDisplayNode::UpdateImageDataPipeline()
{
  if (this->GetDiffusionTensorDisplayPropertiesNode())
    {
    int operation = this->GetDiffusionTensorDisplayPropertiesNode()->GetScalarInvariant();
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

    }
  Superclass::UpdateImageDataPipeline();
}
