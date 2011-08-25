/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFiberBundleDisplayNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include <sstream>


#include "vtkMRMLDiffusionTensorDisplayPropertiesNode.h"
#include "vtkMRMLAnnotationNode.h"
#include "vtkMRMLAnnotationROINode.h"
#include "vtkMRMLFiberBundleDisplayNode.h"
#include "vtkMRMLScene.h"

#include "vtkExtractPolyDataGeometry.h"
#include "vtkPlanes.h"
#include "vtkCommand.h"

vtkCxxSetReferenceStringMacro(vtkMRMLFiberBundleDisplayNode, DiffusionTensorDisplayPropertiesNodeID)
vtkCxxSetReferenceStringMacro(vtkMRMLFiberBundleDisplayNode, AnnotationNodeID)

//------------------------------------------------------------------------------
vtkMRMLFiberBundleDisplayNode* vtkMRMLFiberBundleDisplayNode::New()
{
  return NULL;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLFiberBundleDisplayNode::CreateNodeInstance()
{
  return NULL;
}


//----------------------------------------------------------------------------
vtkMRMLFiberBundleDisplayNode::vtkMRMLFiberBundleDisplayNode()
{
  this->BackfaceCulling = 0;


  this->OutputPolyData = NULL;

  // Enumerated
  this->ColorMode = this->colorModeSolid;

  this->DiffusionTensorDisplayPropertiesNode = NULL;
  this->DiffusionTensorDisplayPropertiesNodeID = NULL;
  this->AnnotationNode = NULL;
  this->AnnotationNodeID = NULL;

  this->ExtractPolyDataGeometry = vtkExtractPolyDataGeometry::New();
  this->ExtractPolyDataGeometry->ExtractInsideOn();
  this->ExtractPolyDataGeometry->ExtractBoundaryCellsOn();
  this->Planes = vtkPlanes::New();

  this->FilterWithAnnotationNode = 0;

  this->ScalarRange[0] = 0;
  this->ScalarRange[1] = 255;
}



//----------------------------------------------------------------------------
vtkMRMLFiberBundleDisplayNode::~vtkMRMLFiberBundleDisplayNode()
{
  this->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(NULL);
  this->SetAndObserveAnnotationNodeID(NULL);
  this->ExtractPolyDataGeometry->Delete();
  this->Planes->Delete();
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << " colorMode =\"" << this->ColorMode << "\"";

  if (this->DiffusionTensorDisplayPropertiesNodeID != NULL) 
    {
    of << indent << " DiffusionTensorDisplayPropertiesNodeRef=\"" << this->DiffusionTensorDisplayPropertiesNodeID << "\"";
    }

  if (this->AnnotationNodeID != NULL) 
    {
    of << indent << " AnnotationNodeRef=\"" << this->AnnotationNodeID << "\"";
    }
}



//----------------------------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "colorMode")) 
      {
      std::stringstream ss;
      ss << attValue;
      int colorMode;
      ss >> colorMode;
      this->SetColorMode(colorMode);
      }

    else if (!strcmp(attName, "DiffusionTensorDisplayPropertiesNodeRef")) 
      {
      this->SetDiffusionTensorDisplayPropertiesNodeID(attValue);
      }
    else if (!strcmp(attName, "AnnotationNodeRef")) 
      {
      this->SetAnnotationNodeID(attValue);
      }
    }  

  this->EndModify(disabledModify);
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLFiberBundleDisplayNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

 vtkMRMLFiberBundleDisplayNode *node = (vtkMRMLFiberBundleDisplayNode *) anode;
 this->SetColorMode(node->ColorMode); // do this first, since it affects how events are processed in glyphs

  Superclass::Copy(anode);

  this->SetDiffusionTensorDisplayPropertiesNodeID(node->DiffusionTensorDisplayPropertiesNodeID);
  this->SetAnnotationNodeID(node->AnnotationNodeID);

  this->EndModify(disabledModify);
  }

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  //int idx;
  
  Superclass::PrintSelf(os,indent);
  os << indent << "ColorMode:             " << this->ColorMode << "\n";
}

//---------------------------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  if (vtkMRMLAnnotationROINode::SafeDownCast(caller) && (event == vtkCommand::ModifiedEvent))
  {
   vtkDebugMacro("Updating the ROI node");
   vtkMRMLAnnotationROINode *AnnotationROI = vtkMRMLAnnotationROINode::SafeDownCast(this->AnnotationNode);
   AnnotationROI->GetTransformedPlanes(this->Planes);
   this->ExtractPolyDataGeometry->SetImplicitFunction(this->Planes); 
   //Are events meant to be cascaded?
   if (this->GetFilterWithAnnotationNode())
   {
     this->InvokeEvent(vtkMRMLDisplayableNode::PolyDataModifiedEvent);
   }
  }

  Superclass::ProcessMRMLEvents(caller, event, callData);
  return;
}

//-----------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::UpdateScene(vtkMRMLScene *scene)
{
   Superclass::UpdateScene(scene);

   this->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(this->GetDiffusionTensorDisplayPropertiesNodeID());
   this->SetAndObserveAnnotationNodeID(this->GetAnnotationNodeID());
}

//-----------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::UpdateReferences()
{
  Superclass::UpdateReferences();

  if (this->DiffusionTensorDisplayPropertiesNodeID != NULL && this->Scene->GetNodeByID(this->DiffusionTensorDisplayPropertiesNodeID) == NULL)
    {
    this->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(NULL);
    }
  if (this->AnnotationNodeID != NULL && this->Scene->GetNodeByID(this->AnnotationNodeID) == NULL)
    {
    this->SetAndObserveAnnotationNodeID(NULL);
    }
}


//----------------------------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  if (this->DiffusionTensorDisplayPropertiesNodeID && !strcmp(oldID, this->DiffusionTensorDisplayPropertiesNodeID))
    {
    this->SetDiffusionTensorDisplayPropertiesNodeID(newID);
    }
  if (this->AnnotationNodeID && !strcmp(oldID, this->AnnotationNodeID))
    {
    this->SetAnnotationNodeID(newID);
    }
}



//----------------------------------------------------------------------------
vtkMRMLDiffusionTensorDisplayPropertiesNode* vtkMRMLFiberBundleDisplayNode::GetDiffusionTensorDisplayPropertiesNode ( )
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
void vtkMRMLFiberBundleDisplayNode::SetAndObserveDiffusionTensorDisplayPropertiesNodeID ( const char *id )
{
  // Stop observing any old node
  vtkSetAndObserveMRMLObjectMacro ( this->DiffusionTensorDisplayPropertiesNode, NULL );

  // Set the ID. This is the "ground truth" reference to the node.
  this->SetDiffusionTensorDisplayPropertiesNodeID ( id );

  // Get the node corresponding to the ID. This pointer is only to observe the object.
  vtkMRMLNode *cnode = this->GetDiffusionTensorDisplayPropertiesNode ( );

  // Observe the node using the pointer.
  vtkSetAndObserveMRMLObjectMacro ( this->DiffusionTensorDisplayPropertiesNode , cnode );

}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::SetFilterWithAnnotationNode(int _arg)
  {
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting FilterWithAnnotationNode  to " << _arg); 
  if (this->FilterWithAnnotationNode != _arg)
    { 
    this->FilterWithAnnotationNode = _arg;

    if (this->FilterWithAnnotationNode)
    {
      this->ExtractPolyDataGeometry->SetInput(this->PolyData);
    }

    this->Modified();
    this->UpdateReferences();
    }
  } 

//----------------------------------------------------------------------------
vtkMRMLAnnotationNode* vtkMRMLFiberBundleDisplayNode::GetAnnotationNode ( )
{
  vtkMRMLAnnotationNode* node = NULL;

  // Find the node corresponding to the ID we have saved.
  if  ( this->GetScene ( ) && this->GetAnnotationNodeID ( ) )
    {
    vtkMRMLNode* cnode = this->GetScene ( ) -> GetNodeByID ( this->AnnotationNodeID );
    node = vtkMRMLAnnotationNode::SafeDownCast ( cnode );
    }

  return node;
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::SetAndObserveAnnotationNodeID ( const char *id )
{
  if (id)
    {
    vtkDebugMacro("Observing annotation Node: "<<id);
    }
  // Stop observing any old node
  vtkSetAndObserveMRMLObjectMacro (this->AnnotationNode, NULL);

  // Set the ID. This is the "ground truth" reference to the node.
  this->SetAnnotationNodeID ( id );

  // Get the node corresponding to the ID. This pointer is only to observe the object.
  vtkMRMLNode *cnode = this->GetAnnotationNode ( );

  // Observe the node using the pointer.
  vtkSetAndObserveMRMLObjectMacro ( this->AnnotationNode , cnode );

  vtkMRMLAnnotationROINode* AnnotationROI = vtkMRMLAnnotationROINode::SafeDownCast(this->AnnotationNode);
  if ((this->AnnotationNode) && (AnnotationROI))
  {
   AnnotationROI->GetTransformedPlanes(this->Planes);
   this->ExtractPolyDataGeometry->SetImplicitFunction(this->Planes); 
  }

}


void vtkMRMLFiberBundleDisplayNode::GetSupportedColorModes(std::vector<int> &modes)
{
  modes.clear();
  modes.push_back(vtkMRMLDiffusionTensorDisplayPropertiesNode::FractionalAnisotropy);
  modes.push_back(vtkMRMLDiffusionTensorDisplayPropertiesNode::LinearMeasure);
  modes.push_back(vtkMRMLDiffusionTensorDisplayPropertiesNode::Trace);
  modes.push_back(vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientation);
  modes.push_back(vtkMRMLDiffusionTensorDisplayPropertiesNode::PlanarMeasure);
  modes.push_back(vtkMRMLDiffusionTensorDisplayPropertiesNode::MaxEigenvalue);
  modes.push_back(vtkMRMLDiffusionTensorDisplayPropertiesNode::MidEigenvalue);
  modes.push_back(vtkMRMLDiffusionTensorDisplayPropertiesNode::MinEigenvalue);
  modes.push_back(vtkMRMLDiffusionTensorDisplayPropertiesNode::RelativeAnisotropy);
}
