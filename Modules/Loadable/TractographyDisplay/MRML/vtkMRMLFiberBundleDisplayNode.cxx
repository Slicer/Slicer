/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFiberBundleDisplayNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/

// MRMLTractography includes
#include "vtkMRMLFiberBundleDisplayNode.h"
#include "vtkMRMLFiberBundleNode.h"

// MRML includes
#include "vtkMRMLDiffusionTensorDisplayPropertiesNode.h"
#include "vtkMRMLDisplayableNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkAssignAttribute.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkCxxSetReferenceStringMacro(vtkMRMLFiberBundleDisplayNode, DiffusionTensorDisplayPropertiesNodeID)

//----------------------------------------------------------------------------
vtkMRMLFiberBundleDisplayNode::vtkMRMLFiberBundleDisplayNode()
{
  this->BackfaceCulling = 0;
  this->ActiveTensorName = 0;

  // Enumerated
  this->ColorMode = this->colorModeSolid;

  this->DiffusionTensorDisplayPropertiesNode = NULL;
  this->DiffusionTensorDisplayPropertiesNodeID = NULL;

  this->ScalarRange[0] = 0.;
  this->ScalarRange[1] = 1.;

  this->SetColor(1,0.157,0);
}

//----------------------------------------------------------------------------
vtkMRMLFiberBundleDisplayNode::~vtkMRMLFiberBundleDisplayNode()
{
  this->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(NULL);
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << " colorMode =\"" << this->ColorMode << "\"";

  if (this->ActiveTensorName != NULL)
    {
    of << indent << " ActiveTensorName=\"" << this->ActiveTensorName << "\"";
    }

  if (this->DiffusionTensorDisplayPropertiesNodeID != NULL)
    {
    of << indent << " DiffusionTensorDisplayPropertiesNodeRef=\"" << this->DiffusionTensorDisplayPropertiesNodeID << "\"";
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
    else if (!strcmp(attName, "ActiveTensorName"))
      {
      this->SetActiveTensorName(attValue);
      }
    else if (!strcmp(attName, "DiffusionTensorDisplayPropertiesNodeRef"))
      {
      this->SetDiffusionTensorDisplayPropertiesNodeID(attValue);
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
  this->SetActiveTensorName(node->ActiveTensorName);

  this->EndModify(disabledModify);
  }

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  //int idx;

  Superclass::PrintSelf(os,indent);
  os << indent << "ColorMode:             " << this->ColorMode << "\n";
  os << indent<< "ActiveTensorName: " <<
    (this->ActiveTensorName ? this->ActiveTensorName : "(none)") << "\n";
}

//-----------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::UpdateScene(vtkMRMLScene *scene)
{
   Superclass::UpdateScene(scene);

   this->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(this->GetDiffusionTensorDisplayPropertiesNodeID());
}

//-----------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::UpdateReferences()
{
  Superclass::UpdateReferences();

  if (this->DiffusionTensorDisplayPropertiesNodeID != NULL &&
      this->Scene->GetNodeByID(this->DiffusionTensorDisplayPropertiesNodeID) == NULL)
    {
    this->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(NULL);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  this->Superclass::UpdateReferenceID(oldID, newID);
  if (this->DiffusionTensorDisplayPropertiesNodeID &&
      !strcmp(oldID, this->DiffusionTensorDisplayPropertiesNodeID))
    {
    this->SetDiffusionTensorDisplayPropertiesNodeID(newID);
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
  if (
      (id != this->GetDiffusionTensorDisplayPropertiesNodeID())
      && id != NULL && this->GetDiffusionTensorDisplayPropertiesNodeID() != NULL
      && (strcmp(id, this->GetDiffusionTensorDisplayPropertiesNodeID()) == 0)
      )
    {
    return;
    }
  // Stop observing any old node
  vtkSetAndObserveMRMLObjectMacro ( this->DiffusionTensorDisplayPropertiesNode, NULL );

  // Set the ID. This is the "ground truth" reference to the node.
  this->SetDiffusionTensorDisplayPropertiesNodeID ( id );

  // Get the node corresponding to the ID. This pointer is only to observe the object.
  vtkMRMLNode *cnode = this->GetDiffusionTensorDisplayPropertiesNode ( );

  // Observe the node using the pointer.
  vtkSetAndObserveMRMLObjectMacro ( this->DiffusionTensorDisplayPropertiesNode , cnode );

  //The new DiffusionTensorDisplayPropertiesNode can have a different setting on the properties
  //so we emit the event that the polydata has been modified
  if (cnode)
    {
    this->InvokeEvent(vtkMRMLModelNode::PolyDataModifiedEvent, this);
    }
}

//----------------------------------------------------------------------------
std::vector<int> vtkMRMLFiberBundleDisplayNode::GetSupportedColorModes()
{
  std::vector<int> modes;

  modes.clear();
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
int vtkMRMLFiberBundleDisplayNode::GetNumberOfScalarInvariants()
{
  static std::vector<int> modes = vtkMRMLFiberBundleDisplayNode::GetSupportedColorModes();
  return modes.size();
}

//----------------------------------------------------------------------------
int vtkMRMLFiberBundleDisplayNode::GetNthScalarInvariant(int i)
{
  static std::vector<int> modes = vtkMRMLFiberBundleDisplayNode::GetSupportedColorModes();
  return modes[i];
}

//---------------------------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  if (vtkMRMLDiffusionTensorDisplayPropertiesNode::SafeDownCast(caller) &&
    event ==  vtkCommand::ModifiedEvent)
    {
    this->Modified();
    }
  return;
}

//---------------------------------------------------------------------------
void vtkMRMLFiberBundleDisplayNode::UpdatePolyDataPipeline()
{
  if (this->GetActiveTensorName() &&
    std::string(this->GetActiveTensorName()) != std::string("") &&
    this->GetInputPolyData() &&
    this->GetInputPolyData()->GetPointData() &&
    this->GetInputPolyData()->GetPointData()->GetTensors() )
  {
    this->AssignAttribute->Assign(
      this->GetActiveTensorName(),
      this->GetActiveTensorName() ? vtkDataSetAttributes::TENSORS : -1,
      vtkAssignAttribute::POINT_DATA);
    this->AssignAttribute->Update();
  }
}
