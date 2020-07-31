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

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLMessageCollection);

//----------------------------------------------------------------------------
void
vtkMRMLMessageCollection
::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  os << indent << "Messages: " << &this->Messages << "\n";
  for (int i = 0; i < this->GetNumberOfMessages(); i++)
    {
    const unsigned long messageType = this->GetNthMessageType(i);
    const std::string messageText = this->GetNthMessageText(i);
    os << indent << "MessagesMember: " << messageType << " "
       << messageText << "\n";
    }
}

//----------------------------------------------------------------------------
vtkMRMLMessageCollection::Message
::Message(unsigned long messageType, const std::string &messageText)
  : MessageType(messageType), MessageText(messageText)
{
}

//----------------------------------------------------------------------------
int
vtkMRMLMessageCollection
::GetNumberOfMessages() const
{
  return this->Messages.size();
}

//----------------------------------------------------------------------------
int
vtkMRMLMessageCollection
::GetNumberOfMessagesOfType(unsigned long messageType) const
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
int
vtkMRMLMessageCollection
::GetNumberOfMessagesOfType(const char *eventName) const
{
  return GetNumberOfMessagesOfType(vtkCommand::GetEventIdFromString(eventName));
}

//----------------------------------------------------------------------------
unsigned long
vtkMRMLMessageCollection
::GetNthMessageType(int index) const
{
  return this->Messages.at(index).MessageType;
}

//----------------------------------------------------------------------------
std::string
vtkMRMLMessageCollection
::GetNthMessageText(int index) const
{
  return this->Messages.at(index).MessageText;
}

//----------------------------------------------------------------------------
void
vtkMRMLMessageCollection
::AddMessage(unsigned long messageType, const std::string &messageText)
{
  this->Messages.push_back({messageType, messageText});
}

//----------------------------------------------------------------------------
void
vtkMRMLMessageCollection
::ClearMessages()
{
  this->Messages.clear();
}

//----------------------------------------------------------------------------
vtkMRMLMessageCollection::vtkMRMLMessageCollection()
{
}

//----------------------------------------------------------------------------
vtkMRMLMessageCollection::~vtkMRMLMessageCollection()
{
}
