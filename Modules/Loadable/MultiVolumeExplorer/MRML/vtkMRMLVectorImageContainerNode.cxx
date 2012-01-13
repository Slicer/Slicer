/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVectorImageContainerNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// VTK includes
#include <vtkCommand.h>
#include <vtkObjectFactory.h>
#include <vtkDoubleArray.h>

// MRML includes
#include "vtkMRMLVolumeNode.h"

// CropModuleMRML includes
#include "vtkMRMLVectorImageContainerNode.h"

// STD includes

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLVectorImageContainerNode);

//----------------------------------------------------------------------------
vtkMRMLVectorImageContainerNode::vtkMRMLVectorImageContainerNode()
{
  // TODO: use this->, use 0 instead of NULL
  DWVNode = NULL;
  VectorLabelArray = NULL;
  VectorLabelName = NULL;
  this->HideFromEditors = 0;
  std::cout << "Vector image container constructor called" << std::endl;
}

//----------------------------------------------------------------------------
vtkMRMLVectorImageContainerNode::~vtkMRMLVectorImageContainerNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLVectorImageContainerNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);
  
  /*
  const char* attName;
  const char* attValue;
  while (*atts != NULL)
  {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "inputVolumeNodeID"))
    {
      this->SetInputVolumeNodeID(attValue);
      continue;
    }
    if (!strcmp(attName, "outputVolumeNodeID"))
    {
      this->SetOutputVolumeNodeID(attValue);
      continue;
    }
    if (!strcmp(attName, "ROINodeID"))
    {
      this->SetROINodeID(attValue);
      continue;
    }
    if (!strcmp(attName,"ROIVisibility"))
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->ROIVisibility;
      continue;
    }
    if (!strcmp(attName,"interpolationMode"))
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->InterpolationMode;
      continue;
    }
  }
  */
  this->WriteXML(std::cout,1);
}

//----------------------------------------------------------------------------
void vtkMRMLVectorImageContainerNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  /*
  vtkIndent indent(nIndent);

  of << indent << " inputVolumeNodeID=\"" << (this->InputVolumeNodeID ? this->InputVolumeNodeID : "NULL") << "\"";
  of << indent << " outputVolumeNodeID=\"" << (this->OutputVolumeNodeID ? this->OutputVolumeNodeID : "NULL") << "\"";
  of << indent << " ROIVisibility=\""<< this->ROIVisibility << "\"";
  of << indent << " ROINodeID=\"" << (this->ROINodeID ? this->ROINodeID : "NULL") << "\"";
  of << indent << " interpolationMode=\"" << this->InterpolationMode << "\"";
  */
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, SliceID
void vtkMRMLVectorImageContainerNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  /*
  vtkMRMLVectorImageContainerNode *node = vtkMRMLVectorImageContainerNode::SafeDownCast(anode);
  this->DisableModifiedEventOn();

  this->SetInputVolumeNodeID(node->GetInputVolumeNodeID());
  this->SetOutputVolumeNodeID(node->GetOutputVolumeNodeID());
  this->SetROINodeID(node->GetROINodeID());
  this->SetInterpolationMode(node->GetInterpolationMode());
  this->SetROIVisibility(node->GetROIVisibility());
  
  this->DisableModifiedEventOff();
  this->InvokePendingModifiedEvent();
  */
}

#if 0
//----------------------------------------------------------------------------
void vtkMRMLVectorImageContainerNode::SetAndObserveInputVolumeNodeID(const char *volumeNodeID)
{
  vtkSetAndObserveMRMLObjectMacro(this->InputVolumeNode, NULL);

  if (volumeNodeID != NULL)
  {
    this->SetInputVolumeNodeID(volumeNodeID);
    vtkMRMLVolumeNode *node = this->GetInputVolumeNode();
    vtkSetAndObserveMRMLObjectMacro(this->InputVolumeNode, node);
  }
}

//----------------------------------------------------------------------------
void vtkMRMLVectorImageContainerNode::SetAndObserveOutputVolumeNodeID(const char *volumeNodeID)
{
  vtkSetAndObserveMRMLObjectMacro(this->OutputVolumeNode, NULL);

  if (volumeNodeID != NULL)
  {
    this->SetOutputVolumeNodeID(volumeNodeID);
    vtkMRMLVolumeNode *node = this->GetOutputVolumeNode();
    vtkSetAndObserveMRMLObjectMacro(this->OutputVolumeNode, node);
  }
}

//----------------------------------------------------------------------------
void vtkMRMLVectorImageContainerNode::SetAndObserveROINodeID(const char *ROINodeID)
{
  vtkSetAndObserveMRMLObjectMacro(this->ROINode, NULL);

  if (ROINodeID != NULL)
  {
    this->SetROINodeID(ROINodeID);
    vtkMRMLAnnotationROINode *node = this->GetROINode();
    vtkSetAndObserveMRMLObjectMacro(this->ROINode, node);
  }
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkMRMLVectorImageContainerNode::GetInputVolumeNode()
{
  if (this->InputVolumeNodeID == NULL)
    {
    vtkSetAndObserveMRMLObjectMacro(this->InputVolumeNode, NULL);
    }
  else if (this->GetScene() &&
           ((this->InputVolumeNode != NULL && strcmp(this->InputVolumeNode->GetID(), this->InputVolumeNodeID)) ||
            (this->InputVolumeNode == NULL)) )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->InputVolumeNodeID);
    vtkSetAndObserveMRMLObjectMacro(this->InputVolumeNode, vtkMRMLVolumeNode::SafeDownCast(snode));
    }
  return this->InputVolumeNode;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkMRMLVectorImageContainerNode::GetOutputVolumeNode()
{
  if (this->OutputVolumeNodeID == NULL)
    {
    vtkSetAndObserveMRMLObjectMacro(this->OutputVolumeNode, NULL);
    }
  else if (this->GetScene() &&
           ((this->OutputVolumeNode != NULL && strcmp(this->OutputVolumeNode->GetID(), this->OutputVolumeNodeID)) ||
            (this->OutputVolumeNode == NULL)) )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->OutputVolumeNodeID);
    vtkSetAndObserveMRMLObjectMacro(this->OutputVolumeNode, vtkMRMLVolumeNode::SafeDownCast(snode));
    }
  return this->OutputVolumeNode;
}

//----------------------------------------------------------------------------
vtkMRMLAnnotationROINode* vtkMRMLVectorImageContainerNode::GetROINode()
{
  if (this->ROINodeID == NULL)
    {
    vtkSetAndObserveMRMLObjectMacro(this->ROINode, NULL);
    }
  else if (this->GetScene() &&
           ((this->ROINode != NULL && strcmp(this->ROINode->GetID(), this->ROINodeID)) ||
            (this->ROINode == NULL)) )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->ROINodeID);
    vtkSetAndObserveMRMLObjectMacro(this->ROINode, vtkMRMLAnnotationROINode::SafeDownCast(snode));
    }
  return this->ROINode;
}

//-----------------------------------------------------------
void vtkMRMLVectorImageContainerNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);
  this->SetAndObserveInputVolumeNodeID(this->InputVolumeNodeID);
  this->SetAndObserveOutputVolumeNodeID(this->OutputVolumeNodeID);
  this->SetAndObserveROINodeID(this->ROINodeID);
}

//---------------------------------------------------------------------------
void vtkMRMLVectorImageContainerNode::ProcessMRMLEvents ( vtkObject *caller,
                                                    unsigned long event,
                                                    void *callData )
{
    Superclass::ProcessMRMLEvents(caller, event, callData);
    this->InvokeEvent(vtkCommand::ModifiedEvent, NULL);
    return;
}

#endif // 0

//----------------------------------------------------------------------------
void vtkMRMLVectorImageContainerNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  /*
  os << "InputVolumeNodeID: " << ( (this->InputVolumeNodeID) ? this->InputVolumeNodeID : "None" ) << "\n";
  os << "OutputVolumeNodeID: " << ( (this->OutputVolumeNodeID) ? this->OutputVolumeNodeID : "None" ) << "\n";
  os << "ROINodeID: " << ( (this->ROINodeID) ? this->ROINodeID : "None" ) << "\n";
  os << "ROIVisibility: " << this->ROIVisibility << "\n";
  os << "InterpolationMode: " << this->InterpolationMode << "\n";
  os << "IsotropicResampling: " << this->IsotropicResampling << "\n";
  */
}

// End
