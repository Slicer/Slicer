/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Lee Newberg, Kitware Inc.

==============================================================================*/

#include "vtkMRMLMessageCollection.h"

// MRML includes
#include "vtkCommand.h"

namespace
{
// This message type is chosen arbitrary and used only inside this class
// (this event is chosen because it has a name that is remotely related
// to separation between groups of messages, and unlikely to be used otherwise).
const int SEPARATOR_MESSAGE_TYPE = vtkCommand::PropertyModifiedEvent;
} // namespace

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLMessageCollection);

//----------------------------------------------------------------------------
void vtkMRMLMessageCollection::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
  os << indent << "Messages: " << &this->Messages << "\n";
  for (int i = 0; i < this->GetNumberOfMessages(); i++)
  {
    const unsigned long messageType = this->GetNthMessageType(i);
    const std::string messageText = this->GetNthMessageText(i);
    os << indent << "MessagesMember: " << messageType << " " << messageText << "\n";
  }
}

//----------------------------------------------------------------------------
vtkMRMLMessageCollection::Message::Message(unsigned long messageType, const std::string& messageText)
  : MessageType(messageType)
  , MessageText(messageText)
{
}

//----------------------------------------------------------------------------
int vtkMRMLMessageCollection::GetNumberOfMessages() const
{
  return this->Messages.size();
}

//----------------------------------------------------------------------------
int vtkMRMLMessageCollection::GetNumberOfMessagesOfType(unsigned long messageType) const
{
  int response = 0;
  for (int i = 0; i < static_cast<int>(this->Messages.size()); ++i)
  {
    if (this->GetNthMessageType(i) == messageType)
    {
      ++response;
    }
  }
  return response;
}

//----------------------------------------------------------------------------
int vtkMRMLMessageCollection::GetNumberOfMessagesOfType(const char* eventName) const
{
  return GetNumberOfMessagesOfType(vtkCommand::GetEventIdFromString(eventName));
}

//----------------------------------------------------------------------------
unsigned long vtkMRMLMessageCollection::GetNthMessageType(int index) const
{
  return this->Messages.at(index).MessageType;
}

//----------------------------------------------------------------------------
std::string vtkMRMLMessageCollection::GetNthMessageText(int index) const
{
  return this->Messages.at(index).MessageText;
}

//----------------------------------------------------------------------------
void vtkMRMLMessageCollection::AddMessage(unsigned long messageType, const std::string& messageText)
{
  this->Messages.push_back({ messageType, messageText });
}

//----------------------------------------------------------------------------
void vtkMRMLMessageCollection::AddSeparator()
{
  this->AddMessage(SEPARATOR_MESSAGE_TYPE, "\n--------\n");
}

//----------------------------------------------------------------------------
void vtkMRMLMessageCollection::ClearMessages()
{
  this->Messages.clear();
}

//----------------------------------------------------------------------------
vtkMRMLMessageCollection::vtkMRMLMessageCollection()
{
  this->CallbackCommand = vtkSmartPointer<vtkCallbackCommand>::New();
  this->CallbackCommand->SetCallback(vtkMRMLMessageCollection::CallbackFunction);
  this->CallbackCommand->SetClientData(this);
}

//----------------------------------------------------------------------------
vtkMRMLMessageCollection::~vtkMRMLMessageCollection()
{
  this->SetObservedObject(nullptr);
}

//----------------------------------------------------------------------------
void vtkMRMLMessageCollection::DeepCopy(vtkMRMLMessageCollection* source)
{
  if (!source)
  {
    vtkErrorMacro("vtkMRMLMessageCollection::DeepCopy failed: invalid source");
    return;
  }
  this->Messages = source->Messages;
}

//----------------------------------------------------------------------------
void vtkMRMLMessageCollection::AddMessages(vtkMRMLMessageCollection* source, const std::string& prefix)
{
  if (!source)
  {
    vtkErrorMacro("vtkMRMLMessageCollection::AddMessages failed: invalid source");
    return;
  }
  for (int i = 0; i < source->GetNumberOfMessages(); i++)
  {
    this->AddMessage(source->GetNthMessageType(i), prefix + source->GetNthMessageText(i));
  }
}

//----------------------------------------------------------------------------
std::string vtkMRMLMessageCollection::GetAllMessagesAsString(bool* errorFoundPtr /*=nullptr*/, bool* warningFoundPtr /*=nullptr*/)
{
  std::string messagesStr;
  bool warningFound = false;
  bool errorFound = false;

  // Check if we need to display bullet-point list
  // (yes, if there are at least two non-separator messages)
  bool showAsBulletPointList = false;
  int numberOfNonSeparatorMessages = 0;
  const int numberOfMessages = this->GetNumberOfMessages();
  for (int index = 0; index < numberOfMessages; ++index)
  {
    const unsigned long messageType = this->GetNthMessageType(index);
    if (messageType != SEPARATOR_MESSAGE_TYPE)
    {
      numberOfNonSeparatorMessages++;
    }
    if (numberOfNonSeparatorMessages >= 2)
    {
      showAsBulletPointList = true;
      break;
    }
  }

  // There is at least one message from the storage node.
  for (int index = 0; index < numberOfMessages; ++index)
  {
    const unsigned long messageType = this->GetNthMessageType(index);
    const std::string messageText = this->GetNthMessageText(index);
    if (messageType == SEPARATOR_MESSAGE_TYPE)
    {
      // do not print separator at the end of the message list
      if (index == numberOfMessages - 1)
      {
        continue;
      }
    }
    else if (!messageText.empty() && showAsBulletPointList)
    {
      messagesStr += "- ";
    }
    switch (messageType)
    {
      case vtkCommand::WarningEvent:
        warningFound = true;
        if (!messageText.empty())
        {
          messagesStr.append("Warning: ");
        }
        break;
      case vtkCommand::ErrorEvent:
        errorFound = true;
        if (!messageText.empty())
        {
          messagesStr.append("Error: ");
        }
        break;
    }
    if (!messageText.empty())
    {
      messagesStr.append(messageText.c_str());
      messagesStr.append("\n");
    }
  }

  if (errorFoundPtr)
  {
    *errorFoundPtr = errorFound;
  }
  if (warningFoundPtr)
  {
    *warningFoundPtr = warningFound;
  }
  return messagesStr;
}

//----------------------------------------------------------------------------
void vtkMRMLMessageCollection::CallbackFunction(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData)
{
  vtkMRMLMessageCollection* self = reinterpret_cast<vtkMRMLMessageCollection*>(clientData);
  if (!self || !callData)
  {
    return;
  }
  std::string msg = reinterpret_cast<char*>(callData);

  // VTK error and warning macros prepend the source code location and the object description to the message
  // (for example: "ERROR: In vtkSomeClass.cxx, line 123\nvtkSomeClass (0000020FC2A2CDC0): Some message").
  // These details are useful for developers, but users should see the message text first, therefore
  // the details are moved after the message text (for example: "Some message [vtkSomeClass.cxx, line 123, vtkSomeClass (0000020FC2A2CDC0)]").
  // Details that will be appended to the end of the message (example: "vtkSomeClass.cxx, line 123, vtkSomeClass (0000020FC2A2CDC0)")
  std::string details;

  // Move the source code location from the first line of the message into details.
  // Example: msg = "ERROR: In vtkSomeClass.cxx, line 123\nvtkSomeClass (0000020FC2A2CDC0): Some message"
  //   -> details = "vtkSomeClass.cxx, line 123"
  //   -> msg = "vtkSomeClass (0000020FC2A2CDC0): Some message"
  const std::string sourceLocationPrefixes[] = { "ERROR: In ", "Warning: In " };
  for (const std::string& sourceLocationPrefix : sourceLocationPrefixes)
  {
    // Check if msg starts with this prefix
    if (msg.compare(0, sourceLocationPrefix.size(), sourceLocationPrefix) == 0)
    {
      // The source code location is the rest of the first line
      size_t sourceLocationEnd = msg.find('\n');
      if (sourceLocationEnd != std::string::npos)
      {
        // Store the source code location (without the prefix), then remove the entire first line from msg
        details = msg.substr(sourceLocationPrefix.size(), sourceLocationEnd - sourceLocationPrefix.size());
        msg.erase(0, sourceLocationEnd + 1);
      }
      break;
    }
  }

  // Move the object description from the beginning of the message into details.
  // Example: msg = "vtkSomeClass (0000020FC2A2CDC0): Some message"
  //   -> details = "vtkSomeClass.cxx, line 123, vtkSomeClass (0000020FC2A2CDC0)"
  //   -> msg = "Some message"
  if (caller)
  {
    // objectDescription example: "vtkSomeClass (0000020FC2A2CDC0)"
    const std::string objectDescription = caller->GetObjectDescription();
    // objectDescriptionPrefix example: "vtkSomeClass (0000020FC2A2CDC0): "
    const std::string objectDescriptionPrefix = objectDescription + ": ";
    // Remove prefix: if msg starts with the object description then remove it and add it to details
    if (msg.compare(0, objectDescriptionPrefix.size(), objectDescriptionPrefix) == 0)
    {
      msg.erase(0, objectDescriptionPrefix.size());
      details += (details.empty() ? "" : ", ") + objectDescription;
    }
  }

  // Remove trailing whitespace (VTK macros end the message with newline characters)
  const std::string chars = "\t\n\v\f\r ";
  msg.erase(msg.find_last_not_of(chars) + 1);

  // Append details to the end of the message.
  // Example: msg = "Some message [vtkSomeClass.cxx, line 123, vtkSomeClass (0000020FC2A2CDC0)]"
  if (!details.empty())
  {
    msg += " [" + details + "]";
  }
  self->AddMessage(eventId, msg);
}

//----------------------------------------------------------------------------
void vtkMRMLMessageCollection::SetObservedObject(vtkObject* observedObject)
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
