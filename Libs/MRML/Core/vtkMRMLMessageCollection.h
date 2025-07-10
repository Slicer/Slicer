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
#include <vtkCallbackCommand.h>
#include <vtkIdTypeArray.h>

// STD includes
#include <string>
#include <vector>

//@{
/// Macros to log warning or error message (for developers) and also
/// add a message to the input message collection (to be displayed
/// to the user later).

#ifndef vtkWarningToMessageCollectionMacro
# define vtkWarningToMessageCollectionMacro(messageCollection, devMsgPrefix, userMsg) \
   vtkWarningToMessageCollectionWithObjectMacro(this, messageCollection, devMsgPrefix, userMsg)
#endif

#ifndef vtkErrorToMessageCollectionMacro
# define vtkErrorToMessageCollectionMacro(messageCollection, devMsgPrefix, userMsg) \
   vtkErrorToMessageCollectionWithObjectMacro(this, messageCollection, devMsgPrefix, userMsg)
#endif

#ifndef vtkWarningToMessageCollectionWithObjectMacro
# define vtkWarningToMessageCollectionWithObjectMacro(self, messageCollection, devMsgPrefix, userMsg) \
   {                                                                                                  \
     vtkOStreamWrapper::EndlType endl;                                                                \
     vtkOStreamWrapper::UseEndl(endl);                                                                \
     if (messageCollection)                                                                           \
     {                                                                                                \
       vtkOStrStreamWrapper userMsgStream;                                                            \
       userMsgStream << userMsg;                                                                      \
       messageCollection->AddMessage(vtkCommand::WarningEvent, userMsgStream.str());                  \
       userMsgStream.rdbuf()->freeze(0);                                                              \
     }                                                                                                \
     vtkWarningWithObjectMacro(self, << devMsgPrefix << ": " << userMsg);                             \
   }
#endif

#ifndef vtkErrorToMessageCollectionWithObjectMacro
# define vtkErrorToMessageCollectionWithObjectMacro(self, messageCollection, devMsgPrefix, userMsg) \
   {                                                                                                \
     vtkOStreamWrapper::EndlType endl;                                                              \
     vtkOStreamWrapper::UseEndl(endl);                                                              \
     if (messageCollection)                                                                         \
     {                                                                                              \
       vtkOStrStreamWrapper userMsgStream;                                                          \
       userMsgStream << userMsg;                                                                    \
       messageCollection->AddMessage(vtkCommand::ErrorEvent, userMsgStream.str());                  \
       userMsgStream.rdbuf()->freeze(0);                                                            \
     }                                                                                              \
     vtkErrorWithObjectMacro(self, << devMsgPrefix << ": " << userMsg);                             \
   }
#endif
//@}

///
/// A class for recording warnings and errors associated with this
/// vtkMRMLStorableNode.  A substantially similar vtkMessageCollection
/// is planned for vtkAddon.  When that is released, this will be
/// replaced by that.
class VTK_MRML_EXPORT vtkMRMLMessageCollection : public vtkObject
{
public:
  vtkTypeMacro(vtkMRMLMessageCollection, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  static vtkMRMLMessageCollection* New();

  /// Return the total number of reported messages.
  virtual int GetNumberOfMessages() const;

  /// Return the number of reported messages of the specified event type.
  virtual int GetNumberOfMessagesOfType(unsigned long messageType) const;

  /// Get the number of reported messages of the specified event type.
  virtual int GetNumberOfMessagesOfType(const char* eventName) const;

  /// Get text of a message from the message vector.
  virtual unsigned long GetNthMessageType(int index) const;

  /// Get event type of a message from the message vector.
  virtual std::string GetNthMessageText(int index) const;

  /// Append a message to the message vector
  /// To mark a message as error or warning, set messageType to
  /// vtkCommand::ErrorEvent or vtkCommand::WarningEvent.
  virtual void AddMessage(unsigned long messageType, const std::string& messageText);

  /// Add a separator, for example to create message groups.
  /// (internally vtkCommand::PropertyModifiedEvent is used as messageType)
  virtual void AddSeparator();

  /// Copy all messages from another collection.
  /// If prefix is specified then that string is prepended to each copied message.
  virtual void AddMessages(vtkMRMLMessageCollection* source, const std::string& prefix = std::string());

  /// Clear the message vector.
  virtual void ClearMessages();

  /// Copy all messages from another collection.
  virtual void DeepCopy(vtkMRMLMessageCollection* source);

  /// Return all messages in a single formatted string.
  /// If optional errorFound or warningFound pointers are set then the caller get information
  /// about presence of warnings or errors in the message list.
  virtual std::string GetAllMessagesAsString(bool* errorFound = nullptr, bool* warningFound = nullptr);

  /// Observe error and warnings reported by observedObject.
  /// For example, this can be used to capture errors from VTK classes
  /// and display them to the user.
  virtual void SetObservedObject(vtkObject* observedObject);

protected:
  vtkMRMLMessageCollection();
  ~vtkMRMLMessageCollection() override;
  vtkMRMLMessageCollection(const vtkMRMLMessageCollection&);
  void operator=(const vtkMRMLMessageCollection&);

  ///
  /// A helper class that describes a single message
  struct VTK_MRML_EXPORT Message
  {
    Message(unsigned long messageType, const std::string& messageText);

    unsigned long MessageType;
    std::string MessageText;
  };

  static void CallbackFunction(vtkObject*, long unsigned int, void* clientData, void* callData);

  vtkSmartPointer<vtkObject> ObservedObject;
  vtkSmartPointer<vtkCallbackCommand> CallbackCommand;
  std::vector<Message> Messages;
};

#endif
