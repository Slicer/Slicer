/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#include "vtkErrorSink.h"

#include "vtkObjectFactory.h"
vtkStandardNewMacro(vtkErrorSink);

//----------------------------------------------------------------------------
vtkErrorSink::vtkErrorSink()
{
  this->CallbackCommand = vtkSmartPointer<vtkCallbackCommand>::New();
  this->CallbackCommand->SetCallback(vtkErrorSink::CallbackFunction);
  this->CallbackCommand->SetClientData(this);
}

//----------------------------------------------------------------------------
vtkErrorSink::~vtkErrorSink()
{
  this->SetObservedObject(nullptr);
}

//----------------------------------------------------------------------------
void vtkErrorSink::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  std::vector<vtkErrorSink::Message>::iterator it = this->MessageList.begin();
  os << indent << "MessageList = \n";
  while (it != this->MessageList.end())
    {
    os << indent.GetNextIndent();
    switch (it->EventId)
      {
      case vtkCommand::ErrorEvent: os << "Error: "; break;
      case vtkCommand::WarningEvent: os << "Warning: "; break;
      }
    os << it->MessageText.c_str() << std::endl;
    }
}

//----------------------------------------------------------------------------
void vtkErrorSink::DisplayMessages()
{
  std::vector<vtkErrorSink::Message>::iterator it = this->MessageList.begin();
  while (it != this->MessageList.end())
    {
    switch (it->EventId)
      {
      case vtkCommand::ErrorEvent: vtkOutputWindowDisplayErrorText(it->MessageText.c_str()); break;
      case vtkCommand::WarningEvent: vtkOutputWindowDisplayWarningText(it->MessageText.c_str()); break;
      }
    ++it;
    }
}

//----------------------------------------------------------------------------
void vtkErrorSink::CallbackFunction(vtkObject* vtkNotUsed(caller),
  long unsigned int eventId,
  void* clientData, void* callData)
{
  vtkErrorSink* self = reinterpret_cast<vtkErrorSink*>(clientData);
  vtkErrorSink::Message message;
  if (callData)
    {
    message.EventId = eventId;
    message.MessageText = reinterpret_cast<char*>(callData);
    self->MessageList.push_back(message);
    }
}

//----------------------------------------------------------------------------
void vtkErrorSink::SetObservedObject(vtkObject* observedObject)
{
  if (observedObject == this->ObservedObject)
    {
    // no change
    return;
    }
  if (this->ObservedObject)
    {
    this->ObservedObject->RemoveObservers(vtkCommand::ErrorEvent, this->CallbackCommand);
    this->ObservedObject->RemoveObservers(vtkCommand::WarningEvent, this->CallbackCommand);
    }
  this->ObservedObject = observedObject;
  if (this->ObservedObject)
    {
    this->ObservedObject->AddObserver(vtkCommand::ErrorEvent, this->CallbackCommand);
    this->ObservedObject->AddObserver(vtkCommand::WarningEvent, this->CallbackCommand);
    }
}

//----------------------------------------------------------------------------
int vtkErrorSink::GetNumberOfMessages()
{
  return this->MessageList.size();
}

//----------------------------------------------------------------------------
bool vtkErrorSink::HasErrors()
{
  std::vector<vtkErrorSink::Message>::iterator it = this->MessageList.begin();
  while (it != this->MessageList.end())
    {
    if (it->EventId == vtkCommand::ErrorEvent)
      {
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
bool vtkErrorSink::HasWarnings()
{
  std::vector<vtkErrorSink::Message>::iterator it = this->MessageList.begin();
  while (it != this->MessageList.end())
    {
    if (it->EventId == vtkCommand::WarningEvent)
      {
      return true;
      }
    }
  return false;
}
