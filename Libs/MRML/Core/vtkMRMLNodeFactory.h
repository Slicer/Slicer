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

/// \brief Registered MRML node classes of a scene.
///
/// Stores a prototype instance of each registered node class, used for creating nodes
/// by class name or XML tag.
///
/// It can be shared between scenes (see vtkMRMLScene::SetNodeFactory()), to allow creating
/// scenes without instantiating a prototype of all the node classes. This is useful when
/// a large number of scene nodes need to be created, for example for storing scene views.
///
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

  /// Register a node class, using \a node as prototype.
  /// If \a tagName is nullptr then the node's GetNodeTagName() is used.
  /// A class already registered with the same tag is replaced.
  void RegisterNodeClass(vtkMRMLNode* node, const char* tagName);

  /// Register a node class with the node's GetNodeTagName() as tag.
  void RegisterNodeClass(vtkMRMLNode* node);

  /// Register type display name of an abstract node class (e.g., "Volume"), for node selectors.
  void RegisterAbstractNodeClass(const std::string& className, const std::string& typeDisplayName);

  /// Register node classes of \a sourceFactory whose tag is not registered in this factory yet.
  /// Prototypes are shared, not copied.
  void CopyRegisteredNodeClasses(vtkMRMLNodeFactory* sourceFactory);

  /// Create a new node of the specified class (falls back to vtkObjectFactory for unregistered classes).
  /// The caller must delete the returned node. Returns nullptr on failure.
  vtkMRMLNode* CreateNodeByClass(const char* className);

  /// Get class name of the node class registered with the specified XML tag.
  /// Returns nullptr if no node class is registered with this tag.
  const char* GetClassNameByTag(const char* tagName);

  /// Get XML tag of the specified node class.
  /// Returns nullptr if the node class is not registered.
  const char* GetTagByClassName(const char* className);

  /// Get the type name shown in the GUI. Returns empty string if the class is not registered.
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
