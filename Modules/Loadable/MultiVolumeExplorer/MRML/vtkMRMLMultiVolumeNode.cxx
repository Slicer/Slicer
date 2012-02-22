/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLMultiVolumeNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// VTK includes
#include <vtkCommand.h>
#include <vtkDoubleArray.h>
#include <vtkObjectFactory.h>

// MRML includes
#include <vtkMRMLDiffusionWeightedVolumeNode.h>
#include <vtkMRMLVolumeNode.h>

// CropModuleMRML includes
#include <vtkMRMLMultiVolumeNode.h>

// STD includes
#include <algorithm>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMultiVolumeNode);

//----------------------------------------------------------------------------
vtkMRMLMultiVolumeNode::vtkMRMLMultiVolumeNode()
{
  this->HideFromEditors = 0;
}

//----------------------------------------------------------------------------
vtkMRMLMultiVolumeNode::~vtkMRMLMultiVolumeNode()
{
}

//----------------------------------------------------------------------------
std::string vtkMRMLMultiVolumeNode::GetDWVNodeID()
{
  return this->DWVNodeID;
}

//----------------------------------------------------------------------------
void vtkMRMLMultiVolumeNode::SetDWVNodeID(const std::string& id)
{
  this->DWVNodeID = id;
}

//----------------------------------------------------------------------------
vtkDoubleArray* vtkMRMLMultiVolumeNode::GetLabelArray()
{
  return this->LabelArray;
}

//----------------------------------------------------------------------------
std::string vtkMRMLMultiVolumeNode::GetLabelName()
{
  return this->LabelName;
}

//----------------------------------------------------------------------------
void vtkMRMLMultiVolumeNode::SetLabelName(const std::string& name)
{
  this->LabelName = name;
}

//----------------------------------------------------------------------------
void vtkMRMLMultiVolumeNode::SetLabelArray(vtkDoubleArray* arr)
{
  if (!this->LabelArray)
    {
    this->LabelArray = vtkSmartPointer<vtkDoubleArray>::New();
    this->LabelArray->Allocate(arr->GetNumberOfTuples());
    this->LabelArray->SetNumberOfTuples(arr->GetNumberOfTuples());
    this->LabelArray->SetNumberOfComponents(1);
    }
  for (int i=0; i<arr->GetNumberOfTuples(); ++i)
    {
    this->LabelArray->SetComponent(i, 0, arr->GetComponent(i, 0));
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMultiVolumeNode::ReadXMLAttributes(const char** atts)
{
  this->Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "DWVNodeID"))
      {
      std::cout << "DWVNodeID is " << attValue << std::endl;
      this->DWVNodeID = attValue;
      continue;
      }
    if (!strcmp(attName, "LabelArray"))
      {
      std::vector<double> labels;
      char* str = (char*)attValue;
      char* pch = strtok(str, " ");
      while(pch)
        {
        labels.push_back(atof(pch));
        pch = strtok(NULL," ");
        }
      std::cout << "Number of elements found: " << labels.size() << std::endl;
      if (!this->LabelArray)
        {
        this->LabelArray = vtkSmartPointer<vtkDoubleArray>::New();
        }
      this->LabelArray->SetNumberOfTuples(labels.size());
      this->LabelArray->SetNumberOfComponents(1);
      for (unsigned int i = 0; i < labels.size(); ++i)
        {
        std::cout << "Setting " << i << " to " << labels[i] << std::endl;
        this->LabelArray->SetComponent(i, 0, labels[i]);
        }
      continue;
      }
    if (!strcmp(attName, "LabelName"))
      {
      this->LabelName = attValue;
      continue;
      }
    }
  this->WriteXML(std::cout,1);
}

//----------------------------------------------------------------------------
void vtkMRMLMultiVolumeNode::WriteXML(ostream& of, int nIndent)
{
  this->Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);
  of << indent << " DWVNodeID=\"" << this->DWVNodeID << "\"";
  if (this->LabelArray)
    {
    int nItems = this->LabelArray->GetNumberOfTuples();
    of << indent << " LabelArray=\"";
    for (int i=0; i < nItems; ++i)
      {
      of << indent << this->LabelArray->GetComponent(i, 0) << indent;
      }
    of << indent << "\"";
    }
  of << indent << " LabelName=\"" << this->LabelName << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLMultiVolumeNode::Copy(vtkMRMLNode *anode)
{
  vtkMRMLMultiVolumeNode *multiVolumeNode = vtkMRMLMultiVolumeNode::SafeDownCast(anode);
  if (!multiVolumeNode)
    {
    return;
    }

  this->Superclass::Copy(anode);

  this->DWVNodeID = multiVolumeNode->DWVNodeID;
  if (multiVolumeNode->LabelArray)
    {
    vtkDoubleArray *arr = multiVolumeNode->LabelArray;
    if (arr)
      {
      if (!this->LabelArray)
        {
        this->LabelArray = vtkSmartPointer<vtkDoubleArray>::New();
        }
      this->LabelArray->SetNumberOfTuples(arr->GetNumberOfTuples());
      this->LabelArray->SetNumberOfComponents(1);
      for (int i = 0; i < arr->GetNumberOfTuples(); ++i)
        {
        this->LabelArray->SetComponent(i, 0, arr->GetComponent(i, 0));
        }
      }
    }
  this->LabelName = multiVolumeNode->LabelName;
}

#if 0
//-----------------------------------------------------------
void vtkMRMLMultiVolumeNode::UpdateScene(vtkMRMLScene *scene)
{
  this->Superclass::UpdateScene(scene);
  this->SetAndObserveInputVolumeNodeID(this->InputVolumeNodeID);
  this->SetAndObserveOutputVolumeNodeID(this->OutputVolumeNodeID);
  this->SetAndObserveROINodeID(this->ROINodeID);
}

//---------------------------------------------------------------------------
void vtkMRMLMultiVolumeNode::ProcessMRMLEvents ( vtkObject *caller,
                                                    unsigned long event,
                                                    void *callData )
{
  this->Superclass::ProcessMRMLEvents(caller, event, callData);
  this->InvokeEvent(vtkCommand::ModifiedEvent, NULL);
}

#endif // 0

//----------------------------------------------------------------------------
void vtkMRMLMultiVolumeNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << "DWVNodeID: " << this->DWVNodeID << "\n";
  if (this->LabelArray)
    {
    os << "LabelArray: ";
    for (int i = 0; i < this->LabelArray->GetNumberOfTuples(); ++i)
      {
      os << this->LabelArray->GetComponent(i, 0) << " ";
      }
    os << std::endl;
    }
  os << "LabelName: " << this->LabelName << std::endl;
}
