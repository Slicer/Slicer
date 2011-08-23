/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVolumeNode.cxx,v $
Date:      $Date: 2006/03/17 17:01:53 $
Version:   $Revision: 1.14 $

=========================================================================auto=*/
// MRML includes
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkImageData.h>
#include <vtkPointData.h>

// for calculating auto win/level
#include <vtkImageAccumulateDiscrete.h>
#include <vtkImageBimodalAnalysis.h>

// STD includes

//------------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* vtkMRMLScalarVolumeNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLScalarVolumeNode");
  if(ret)
    {
    return (vtkMRMLScalarVolumeNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLScalarVolumeNode;
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScalarVolumeNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLScalarVolumeNode");
  if(ret)
    {
    return (vtkMRMLScalarVolumeNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLScalarVolumeNode;
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode::vtkMRMLScalarVolumeNode()
{
  this->Bimodal = vtkImageBimodalAnalysis::New();
  this->Accumulate = vtkImageAccumulateDiscrete::New();
  this->CalculatingAutoLevels = 0;
  this->SetAttribute("LabelMap", "0"); // not label by default; avoid set method in constructor
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode::~vtkMRMLScalarVolumeNode()
{
  if (this->Bimodal)
    {
    this->Bimodal->Delete();
    this->Bimodal = NULL;
    }
  if (this->Accumulate)
    {
    this->Accumulate->Delete();
    this->Accumulate = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLScalarVolumeNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  
  vtkIndent indent(nIndent);
  std::stringstream ss;
  ss << this->GetLabelMap();
  of << indent << " labelMap=\"" << ss.str() << "\"";

}

//----------------------------------------------------------------------------
void vtkMRMLScalarVolumeNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "labelMap")) 
      {
      std::stringstream ss;
      int val;
      ss << attValue;
      ss >> val;
      this->SetLabelMap(val);
      }
    }  

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLScalarVolumeNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLScalarVolumeNode *node = (vtkMRMLScalarVolumeNode *) anode;

  this->SetLabelMap(node->GetLabelMap());

  this->EndModify(disabledModify);
}

//-----------------------------------------------------------
void vtkMRMLScalarVolumeNode::CreateNoneNode(vtkMRMLScene *scene)
{
  vtkMRMLScalarVolumeNode *n = vtkMRMLScalarVolumeNode::New();
  n->SetName("None");
  // the scene will set the id
//  n->SetID("None");

  // Create a None volume RGBA of 0, 0, 0 so the filters won't complain
  // about missing input
  vtkImageData *id;
  id = vtkImageData::New();
  id->SetDimensions(1, 1, 1);
  id->SetNumberOfScalarComponents(4);
  id->AllocateScalars();
  id->GetPointData()->GetScalars()->FillComponent(0, 0.0);
  id->GetPointData()->GetScalars()->FillComponent(1, 125.0);
  id->GetPointData()->GetScalars()->FillComponent(2, 0.0);
  id->GetPointData()->GetScalars()->FillComponent(3, 0.0);

  n->SetImageData(id);
  scene->AddNode(n);

  n->Delete();
  id->Delete();
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeDisplayNode* vtkMRMLScalarVolumeNode::GetScalarVolumeDisplayNode()
{
  return vtkMRMLScalarVolumeDisplayNode::SafeDownCast(this->GetDisplayNode());
}

//----------------------------------------------------------------------------
void vtkMRMLScalarVolumeNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);
}

int vtkMRMLScalarVolumeNode::GetLabelMap()
{
  if (!this->GetAttribute("LabelMap"))
    {
    return 0;
    }
  
  std::string value = this->GetAttribute("LabelMap");
  if (value == "0")
    {
    return 0;
    }
  else
    {
    return 1;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLScalarVolumeNode::LabelMapOn()
{
  this->SetLabelMap(1);
}

//----------------------------------------------------------------------------
void vtkMRMLScalarVolumeNode::LabelMapOff()
{
  this->SetLabelMap(0);
}


//----------------------------------------------------------------------------
void vtkMRMLScalarVolumeNode::SetLabelMap(int flag)
{
  std::string value;
  if (flag)
    {
    value = "1";
    }
  else
    {
    value = "0";
    }

  const char *attr = this->GetAttribute("LabelMap");
  if (attr && (value == attr))
    {
    return;
    }
  
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting LabelMap to " << flag);

  this->SetAttribute("LabelMap", value.c_str());

/*
    if (this->GetDisplayNode() != NULL)
      {
      if (this->LabelMap == 1)
        {
        // set the display node's color node to be Labels
        vtkDebugMacro("Label map is 1, need to update the display node to be labels\n");
        this->GetDisplayNode()->SetAndObserveColorNodeID("vtkMRMLColorNodeLabels");
        }
      else
        {
        vtkDebugMacro("Label map is not 1, updating color node in display to be grey (this is too restrictive)\n");
        this->GetDisplayNode()->SetAndObserveColorNodeID("vtkMRMLColorNodeGrey");
        }
      }
    else
      {
      vtkErrorMacro("ERROR: no display node associated with this scalar volume, not changing color node\n");
      }
*/
     // invoke a modified event
    this->Modified();
}

//---------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLScalarVolumeNode::CreateDefaultStorageNode()
{
  return vtkMRMLVolumeArchetypeStorageNode::New();
}

