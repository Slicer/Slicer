/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLMultiVolumeNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// C++ includes
#include <algorithm>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

// VTK includes
#include <vtkCommand.h>
#include <vtkDoubleArray.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// MRML includes
#include "vtkMRMLDiffusionWeightedVolumeNode.h"
#include "vtkMRMLVolumeNode.h"

// CropModuleMRML includes
#include "vtkMRMLMultiVolumeNode.h"

// STD includes

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMultiVolumeNode);

//----------------------------------------------------------------------------
vtkMRMLMultiVolumeNode::vtkMRMLMultiVolumeNode()
{
  // TODO: use this->, use 0 instead of NULL
  this->VectorLabelArray = NULL;
  this->VectorLabelName = "";
  this->HideFromEditors = 0;
  this->DWVNodeID = "";
  std::cout << "Vector image container constructor called" << std::endl;
}

//----------------------------------------------------------------------------
vtkMRMLMultiVolumeNode::~vtkMRMLMultiVolumeNode()
{
  if(this->VectorLabelArray)
    {
    this->VectorLabelArray->Delete();
    this->VectorLabelArray = NULL;
    }
}

//----------------------------------------------------------------------------
const std::string vtkMRMLMultiVolumeNode::GetDWVNodeID()
{
  return this->DWVNodeID;
}

//----------------------------------------------------------------------------
void vtkMRMLMultiVolumeNode::SetDWVNodeID(std::string id)
{
  this->DWVNodeID = id;
}

//----------------------------------------------------------------------------
const vtkDoubleArray* vtkMRMLMultiVolumeNode::GetVectorLabelArray()
{
  return this->VectorLabelArray;
}

//----------------------------------------------------------------------------
void vtkMRMLMultiVolumeNode::SetVectorLabelArray(vtkDoubleArray* arr)
{
  if(!this->VectorLabelArray)
    {
    this->VectorLabelArray = vtkDoubleArray::New();
    this->VectorLabelArray->Allocate(arr->GetNumberOfTuples());
    this->VectorLabelArray->SetNumberOfTuples(arr->GetNumberOfTuples());
    this->VectorLabelArray->SetNumberOfComponents(1);
    }
  for(int i=0;i<arr->GetNumberOfTuples();i++)
    this->VectorLabelArray->SetComponent(i, 0, arr->GetComponent(i, 0));
}

//----------------------------------------------------------------------------
void vtkMRMLMultiVolumeNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);
  
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
    if (!strcmp(attName, "VectorLabelArray"))
    {
      std::vector<double> labels;
      char* str = (char*)attValue;
      char* pch = strtok(str, " ");
      while(pch)
        {
        labels.push_back(atof(pch));
        pch = strtok(NULL," ");
        }
      /*
      copy(std::istream_iterator<std::string>(iss),
        std::istream_iterator<std::string>(),
        std::back_inserter<std::vector<double> >(labels));
      */
      std::cout << "Number of elements found: " << labels.size() << std::endl;
      if(!this->VectorLabelArray)
        this->VectorLabelArray = vtkDoubleArray::New();
      this->VectorLabelArray->SetNumberOfTuples(labels.size());
      this->VectorLabelArray->SetNumberOfComponents(1);
      for(unsigned int i=0;i<labels.size();i++)
        {
        std::cout << "Setting " << i << " to " << labels[i] << std::endl;
        this->VectorLabelArray->SetComponent(i, 0, labels[i]);
        }
      continue;
    }
    if (!strcmp(attName, "VectorLabelName"))
    {
      this->VectorLabelName = attValue;
      continue;
    }
  }
  this->WriteXML(std::cout,1);
}

//----------------------------------------------------------------------------
void vtkMRMLMultiVolumeNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);
  of << indent << " DWVNodeID=\"" << this->DWVNodeID << "\"";
  if(this->VectorLabelArray)
    {
    int nItems = this->VectorLabelArray->GetNumberOfTuples();
    of << indent << " VectorLabelArray=\"";
    for(int i=0;i<nItems;i++)
      {
      of << indent << this->VectorLabelArray->GetComponent(i, 0) << indent;
      }
    of << indent << "\"";
    }
  of << indent << " VectorLabelName=\"" << this->VectorLabelName << "\"";
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, SliceID
void vtkMRMLMultiVolumeNode::Copy(vtkMRMLNode *anode)
{
  vtkMRMLMultiVolumeNode *n = vtkMRMLMultiVolumeNode::SafeDownCast(anode);
  if(!n)
    return;

  Superclass::Copy(anode);

  this->DWVNodeID = n->DWVNodeID;
  if(n->VectorLabelArray)
    {
    vtkDoubleArray *arr = n->VectorLabelArray;
    if(arr)
      {
      if(!this->VectorLabelArray)
        this->VectorLabelArray = vtkDoubleArray::New();
      this->VectorLabelArray->SetNumberOfTuples(arr->GetNumberOfTuples());
      this->VectorLabelArray->SetNumberOfComponents(1);
      for(int i=0;i<arr->GetNumberOfTuples();i++)
        this->VectorLabelArray->SetComponent(i, 0, arr->GetComponent(i, 0));
      }
    }
  this->VectorLabelName = n->VectorLabelName;
}

#if 0
//-----------------------------------------------------------
void vtkMRMLMultiVolumeNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);
  this->SetAndObserveInputVolumeNodeID(this->InputVolumeNodeID);
  this->SetAndObserveOutputVolumeNodeID(this->OutputVolumeNodeID);
  this->SetAndObserveROINodeID(this->ROINodeID);
}

//---------------------------------------------------------------------------
void vtkMRMLMultiVolumeNode::ProcessMRMLEvents ( vtkObject *caller,
                                                    unsigned long event,
                                                    void *callData )
{
    Superclass::ProcessMRMLEvents(caller, event, callData);
    this->InvokeEvent(vtkCommand::ModifiedEvent, NULL);
    return;
}

#endif // 0

//----------------------------------------------------------------------------
void vtkMRMLMultiVolumeNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << "DWVNodeID: " << this->DWVNodeID << "\n";
  if(this->VectorLabelArray)
    {
    os << "VectorLabelArray: ";
    for(int i=0;i<this->VectorLabelArray->GetNumberOfTuples();i++)
      os << this->VectorLabelArray->GetComponent(i, 0) << " ";
    os << std::endl;
    }
  os << "VectorLabelName: " << this->VectorLabelName << std::endl;
}
// End
