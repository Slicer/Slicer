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
  this->ResetNumberOfEvents();
}

//---------------------------------------------------------------------------
vtkMRMLNodeCallback::~vtkMRMLNodeCallback()
{
}

//---------------------------------------------------------------------------
void vtkMRMLNodeCallback::ResetNumberOfEvents()
{
  this->ReceivedEvents.clear();
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
  return this->GetNumberOfEvents(vtkCommand::ModifiedEvent);
}

//---------------------------------------------------------------------------
int vtkMRMLNodeCallback::GetNumberOfEvents(unsigned long event)
{
  return this->ReceivedEvents[event];
}

//---------------------------------------------------------------------------
int vtkMRMLNodeCallback::GetTotalNumberOfEvents()
{
  int eventsCount = 0;
  for (std::map<unsigned long, unsigned int>::const_iterator it = this->ReceivedEvents.begin(),
       end = this->ReceivedEvents.end(); it != end; ++it)
    {
    eventsCount += it->second;
    }
  return eventsCount;
}

//---------------------------------------------------------------------------
std::vector<unsigned long> vtkMRMLNodeCallback::GetReceivedEvents()
{
  std::vector<unsigned long> receivedEvents;
  for(std::map<unsigned long,unsigned int>::iterator it = this->ReceivedEvents.begin();
      it != this->ReceivedEvents.end();
      ++it)
    {
    unsigned long event = it->first;
    if (this->GetNumberOfEvents(event) > 0)
      {
      receivedEvents.push_back(event);
      }
    }
  return receivedEvents;
}

//---------------------------------------------------------------------------
void vtkMRMLNodeCallback::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkCallbackCommand::PrintSelf(os,indent);

  os << indent << "ErrorString: " << this->GetErrorString() << "\n";
  os << indent << "TotalNumberOfEvents: " << this->GetTotalNumberOfEvents() << "\n";
  os << indent << "NumberOfModified: " << this->GetNumberOfModified() << "\n";
  std::vector<unsigned long> receivedEvent = this->GetReceivedEvents();
  os << indent << "ReceivedEvents: \n";
  for(std::vector<unsigned long>::iterator it = receivedEvent.begin();
      it != receivedEvent.end();
      ++it)
    {
    os << indent.GetNextIndent() << *it << " \n";
    }
}

//---------------------------------------------------------------------------
void vtkMRMLNodeCallback::Execute(vtkObject *vtkcaller,
  unsigned long eid, void *vtkNotUsed(calldata))
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

  ++this->ReceivedEvents[eid];
}
