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

==============================================================================*/

#ifndef __vtkMRMLNodeFactory_h
#define __vtkMRMLNodeFactory_h

// MRML includes
#include "vtkMRML.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkObject.h>

// STD includes
#include <map>
#include <string>
#include <vector>

class vtkMRMLNode;

/// \brief Registry of MRML node classes that can be instantiated by class name or XML tag.
///
/// The factory stores a prototype instance of each registered node class. New node instances
/// are created from the prototypes (using vtkMRMLNode::CreateNodeInstance()), node class names
/// are looked up from XML tags when a scene is read from file, and type display names are
/// provided for the GUI.
///
/// A factory can be shared between scenes (see vtkMRMLScene::SetNodeFactory()). For example,
/// each sequence node contains an internal scene. Sharing the factory of the main scene with
/// these internal scenes avoids instantiating a prototype of all the node classes for each
/// sequence node, which would be slow when there are many sequence nodes.
class VTK_MRML_EXPORT vtkMRMLNodeFactory : public vtkObject
{
public:
  static vtkMRMLNodeFactory* New();
  vtkTypeMacro(vtkMRMLNodeFactory, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  enum
  {
    /// Invoked when a node class or an abstract node class is registered
    NodeClassRegisteredEvent = vtkCommand::UserEvent + 1
  };

  /// Register the built-in node classes of the MRML core library.
  void RegisterCoreNodeClasses();

  /// \brief Register a node class so that nodes of this class can later be created
  /// from a tag or a class name.
  ///
  /// \a node is an instance of the class to instantiate when CreateNodeByClass()
  /// is called with a corresponding className retrieved using GetClassNameByTag().
  /// \a tagName can be nullptr or a custom XML tag. If \a tagName is nullptr then
  /// the node's GetNodeTagName() is used.
  /// If a class is already registered with the same tag then it is replaced.
  ///
  /// \sa CreateNodeByClass(), GetClassNameByTag()
  void RegisterNodeClass(vtkMRMLNode* node, const char* tagName);

  /// Utility function to RegisterNodeClass(), the node tag name is used when
  /// registering the node.
  void RegisterNodeClass(vtkMRMLNode* node);

  /// \brief Register abstract node type display name.
  ///
  /// This is used by GetTypeDisplayNameByClassName() for an abstract class (Volume, Markups, etc),
  /// for example in a node selector. Since abstract base classes cannot be instantiated,
  /// RegisterNodeClass() cannot be used for this purpose.
  void RegisterAbstractNodeClass(const std::string& className, const std::string& typeDisplayName);

  /// Register all node classes and abstract node classes of \a sourceFactory
  /// that are not registered in this factory yet.
  void CopyRegisteredNodeClasses(vtkMRMLNodeFactory* sourceFactory);

  /// \brief Create a new node of the specified class.
  ///
  /// The node is created from the registered prototype of the class. If the class is not registered
  /// then the node is created using vtkObjectFactory. Returns nullptr if the node cannot be created.
  /// The caller is responsible for deleting the returned node.
  vtkMRMLNode* CreateNodeByClass(const char* className);

  /// Get class name of the node class registered with the specified XML tag.
  /// Returns nullptr if no node class is registered with this tag.
  const char* GetClassNameByTag(const char* tagName);

  /// Get XML tag of the specified node class.
  /// Returns nullptr if the node class is not registered.
  const char* GetTagByClassName(const char* className);

  /// Get type display name which is shown in the GUI.
  /// Returns empty string if the class is not registered.
  std::string GetTypeDisplayNameByClassName(const std::string& className);

  /// Get the number of registered node classes.
  int GetNumberOfRegisteredNodeClasses();
  /// Get the prototype of the nth registered node class. Returns nullptr if n is out of range.
  vtkMRMLNode* GetNthRegisteredNodeClass(int n);
  /// Return true if \a className is a registered node class.
  bool IsNodeClassRegistered(const std::string& className);

  /// Get the number of registered abstract node classes.
  int GetNumberOfRegisteredAbstractNodeClasses();
  /// Get the class name of the nth registered abstract node class.
  std::string GetNthRegisteredAbstractNodeClassName(int n);
  /// Get the type display name of the nth registered abstract node class.
  std::string GetNthRegisteredAbstractNodeTypeDisplayName(int n);

protected:
  vtkMRMLNodeFactory();
  ~vtkMRMLNodeFactory() override;
  vtkMRMLNodeFactory(const vtkMRMLNodeFactory&) = delete;
  void operator=(const vtkMRMLNodeFactory&) = delete;

  std::vector<vtkMRMLNode*> RegisteredNodeClasses;
  std::vector<std::string> RegisteredNodeTags;
  std::map<std::string, std::string> RegisteredAbstractNodeClassTypeDisplayNames; // map class name to type display name
};

#endif
