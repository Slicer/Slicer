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

#ifndef __vtkMRMLMessageCollection_h
#define __vtkMRMLMessageCollection_h

// MRML includes
#include "vtkMRML.h"

// VTK includes
#include <vtkIdTypeArray.h>

// STD includes
#include <string>
#include <vector>

///
/// A class for recording warnings and errors associated with this
/// vtkMRMLStorableNode.  A substantially similar vtkMessageCollection
/// is planned for vtkAddon.  When that is released, this will be
/// replaced by that.
class VTK_MRML_EXPORT vtkMRMLMessageCollection : public vtkObject
{
public:
  vtkTypeMacro(vtkMRMLMessageCollection,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  static vtkMRMLMessageCollection* New();

  ///
  /// Return the total number of reported messages.
  virtual int GetNumberOfMessages() const;

  ///
  /// Return the number of reported messages of the specified event
  /// type
  virtual int GetNumberOfMessagesOfType(unsigned long messageType) const;

  ///
  /// Return the number of reported messages of the specified event
  /// type
  virtual int GetNumberOfMessagesOfType(const char *eventName) const;

  // Get the Nth message from the message vector
  virtual unsigned long GetNthMessageType(int index) const;
  virtual std::string GetNthMessageText(int index) const;

  // Append a message to the message vector
  virtual void AddMessage(unsigned long messageType, const std::string &messageText);

  // Clear the message vector
  virtual void ClearMessages();

protected:
  vtkMRMLMessageCollection();
  ~vtkMRMLMessageCollection() override;
  vtkMRMLMessageCollection(const vtkMRMLMessageCollection&);
  void operator=(const vtkMRMLMessageCollection&);

  ///
  /// A helper class that describes a single message
  struct VTK_MRML_EXPORT Message
    {
    Message(unsigned long messageType, const std::string &messageText);

    unsigned long MessageType;
    std::string MessageText;
    };

  std::vector<Message> Messages;
};


#endif
