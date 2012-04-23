/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"

//---------------------------------------------------------------------------
vtkMRMLNodeCallback::vtkMRMLNodeCallback()
  : Node(0)
{
  this->ResetNumberOfEventsVariables();
}

//---------------------------------------------------------------------------
vtkMRMLNodeCallback::~vtkMRMLNodeCallback()
{
}

//---------------------------------------------------------------------------
void vtkMRMLNodeCallback::ResetNumberOfEventsVariables()
{
  this->NumberOfModified = 0;
}

//---------------------------------------------------------------------------
void vtkMRMLNodeCallback::SetMRMLNode(vtkMRMLNode* node)
{
  this->Node = node;
}

//---------------------------------------------------------------------------
std::string vtkMRMLNodeCallback::GetErrorString()
{
  return this->ErrorString;
}

//---------------------------------------------------------------------------
void vtkMRMLNodeCallback::SetErrorString(const char* error)
{
  this->ErrorString = std::string(error);
}

//---------------------------------------------------------------------------
int vtkMRMLNodeCallback::GetNumberOfModified()
{
  return this->NumberOfModified;
}

//---------------------------------------------------------------------------
void vtkMRMLNodeCallback::Execute(vtkObject *vtkcaller,
  unsigned long eid, void *calldata)
{
  // Let's return if an error already occured
  if (this->ErrorString.size() > 0)
    {
    return;
    }
  if (this->Node &&
      this->Node != vtkMRMLNode::SafeDownCast(vtkcaller))
    {
    this->SetErrorString("vtkMRMLNodeCallback::Execute - node != vtkcaller");
    return;
    }
    
  if (eid == vtkCommand::ModifiedEvent)
    {
    ++this->NumberOfModified;
    }
}
