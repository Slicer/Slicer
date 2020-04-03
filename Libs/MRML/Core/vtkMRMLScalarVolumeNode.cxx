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
#include "vtkCodedEntry.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkMRMLVolumeSequenceStorageNode.h"

// VTK includes
#include <vtkDataArray.h>
#include <vtkObjectFactory.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkPointData.h>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLScalarVolumeNode);
vtkCxxSetObjectMacro(vtkMRMLScalarVolumeNode, VoxelValueQuantity, vtkCodedEntry);
vtkCxxSetObjectMacro(vtkMRMLScalarVolumeNode, VoxelValueUnits, vtkCodedEntry);

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode::vtkMRMLScalarVolumeNode() = default;

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode::~vtkMRMLScalarVolumeNode()
{
  this->SetVoxelValueQuantity(nullptr);
  this->SetVoxelValueUnits(nullptr);
}

//----------------------------------------------------------------------------
void vtkMRMLScalarVolumeNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  if (this->GetVoxelValueQuantity())
    {
    of << " voxelValueQuantity=\"" << vtkMRMLNode::URLEncodeString(this->GetVoxelValueQuantity()->GetAsString().c_str()) << "\"";
    }
  if (this->GetVoxelValueUnits())
    {
    of << " voxelValueUnits=\"" << vtkMRMLNode::URLEncodeString(this->GetVoxelValueUnits()->GetAsString().c_str()) << "\"";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLScalarVolumeNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  // For backward compatibility, we read the labelMap attribute and save it as a custom attribute.
  // This allows scene reader to detect that this node has to be converted to a segmentation node.
  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "labelMap"))
      {
      std::stringstream ss;
      int val;
      ss << attValue;
      ss >> val;
      if (val)
        {
        this->SetAttribute("LabelMap", "1");
        }
      }
    else if (!strcmp(attName, "voxelValueQuantity"))
      {
      vtkNew<vtkCodedEntry> entry;
      entry->SetFromString(vtkMRMLNode::URLDecodeString(attValue));
      this->SetVoxelValueQuantity(entry.GetPointer());
      }
    else if (!strcmp(attName, "voxelValueUnits"))
      {
      vtkNew<vtkCodedEntry> entry;
      entry->SetFromString(vtkMRMLNode::URLDecodeString(attValue));
      this->SetVoxelValueUnits(entry.GetPointer());
      }
    }

  this->EndModify(disabledModify);
}

//-----------------------------------------------------------
void vtkMRMLScalarVolumeNode::CreateNoneNode(vtkMRMLScene *scene)
{
  // Create a None volume RGBA of 0, 0, 0 so the filters won't complain
  // about missing input
  vtkNew<vtkImageData> id;
  id->SetDimensions(1, 1, 1);
  id->AllocateScalars(VTK_DOUBLE, 4);
  id->GetPointData()->GetScalars()->FillComponent(0, 0.0);
  id->GetPointData()->GetScalars()->FillComponent(1, 125.0);
  id->GetPointData()->GetScalars()->FillComponent(2, 0.0);
  id->GetPointData()->GetScalars()->FillComponent(3, 0.0);

  vtkNew<vtkMRMLScalarVolumeNode> n;
  n->SetName("None");
  // the scene will set the id
  n->SetAndObserveImageData(id.GetPointer());
  scene->AddNode(n.GetPointer());
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
  if (this->GetVoxelValueQuantity())
    {
    os << indent << "VoxelValueQuantity: " << this->GetVoxelValueQuantity()->GetAsPrintableString() << "\n";
    }
  if (this->GetVoxelValueUnits())
    {
    os << indent << "VoxelValueUnits: " << this->GetVoxelValueUnits()->GetAsPrintableString() << "\n";
    }
}

//---------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLScalarVolumeNode::CreateDefaultStorageNode()
{
  vtkMRMLScene* scene = this->GetScene();
  if (scene == nullptr)
    {
    vtkErrorMacro("CreateDefaultStorageNode failed: scene is invalid");
    return nullptr;
    }
  return vtkMRMLStorageNode::SafeDownCast(
    scene->CreateNodeByClass("vtkMRMLVolumeArchetypeStorageNode"));
}

//----------------------------------------------------------------------------
void vtkMRMLScalarVolumeNode::CreateDefaultDisplayNodes()
{
  if (vtkMRMLScalarVolumeDisplayNode::SafeDownCast(this->GetDisplayNode())!=nullptr)
    {
    // display node already exists
    return;
    }
  if (this->GetScene()==nullptr)
    {
    vtkErrorMacro("vtkMRMLScalarVolumeNode::CreateDefaultDisplayNodes failed: scene is invalid");
    return;
    }
  vtkMRMLScalarVolumeDisplayNode* dispNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(
    this->GetScene()->AddNewNodeByClass("vtkMRMLScalarVolumeDisplayNode") );
  dispNode->SetDefaultColorMap();
  this->SetAndObserveDisplayNodeID(dispNode->GetID());
}

//----------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLScalarVolumeNode::CreateDefaultSequenceStorageNode()
{
  return vtkMRMLVolumeSequenceStorageNode::New();
}
