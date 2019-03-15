/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLChartNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLChartViewNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLChartViewNode);

//vtkCxxSetReferenceStringMacro(vtkMRMLChartViewNode, ChartNodeID);

//----------------------------------------------------------------------------
vtkMRMLChartViewNode::vtkMRMLChartViewNode()
{
  this->ChartNodeID = nullptr;
}

//----------------------------------------------------------------------------
vtkMRMLChartViewNode::~vtkMRMLChartViewNode()
{
  if (this->ChartNodeID)
    {
    this->SetChartNodeID(nullptr);
    }
}

//----------------------------------------------------------------------------
const char* vtkMRMLChartViewNode::GetNodeTagName()
{
  return "ChartView";
}

//----------------------------------------------------------------------------
void vtkMRMLChartViewNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  if (this->ChartNodeID)
    {
    of << " chart=\"" << this->ChartNodeID << "\"";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLChartViewNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "chart"))
      {
      this->SetChartNodeID(attValue);
      }
    }

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLChartViewNode::Copy(vtkMRMLNode *anode)
{
  vtkMRMLChartViewNode *achartviewnode = vtkMRMLChartViewNode::SafeDownCast(anode);

  int disabledModify = this->StartModify();

  this->Superclass::Copy(anode);

  this->SetChartNodeID(achartviewnode->GetChartNodeID());

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLChartViewNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "ChartNodeID: " <<
   (this->ChartNodeID ? this->ChartNodeID : "(none)") << "\n";
}

//----------------------------------------------------------------------------
void vtkMRMLChartViewNode::SetChartNodeID(const char* _arg)
{
  vtkSetReferenceStringBodyMacro(ChartNodeID);

  this->InvokeEvent(vtkMRMLChartViewNode::ChartNodeChangedEvent);
}

//----------------------------------------------------------------------------
void vtkMRMLChartViewNode::UpdateReferences()
{
   Superclass::UpdateReferences();

  if (this->ChartNodeID != nullptr && this->Scene->GetNodeByID(this->ChartNodeID) == nullptr)
    {
    this->SetChartNodeID(nullptr);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLChartViewNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  Superclass::UpdateReferenceID(oldID, newID);

  if (this->ChartNodeID && !strcmp(oldID, this->ChartNodeID))
    {
    this->SetChartNodeID(newID);
    }
}
