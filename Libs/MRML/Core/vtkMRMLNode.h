/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.18 $

=========================================================================auto=*/

#ifndef __vtkMRMLNode_h
#define __vtkMRMLNode_h

// MRML includes
#include "vtkMRML.h"
#include "vtkObserverManager.h"
#include "vtkIdTypeArray.h"
#include "vtkIntArray.h"

class vtkMRMLScene;
class vtkStringArray;

// VTK includes
#include <vtkObject.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>
class vtkCallbackCommand;

// Slicer VTK add-on includes
#include <vtkLoggingMacros.h>

// Helper macros for simplifying reading, writing, copying, and printing node properties.
#include "vtkMRMLNodePropertyMacros.h"

// STD includes
#include <map>
#include <set>
#include <string>
#include <vector>

#ifndef vtkSetMRMLObjectMacro
#define vtkSetMRMLObjectMacro(node,value)  {this->MRMLObserverManager->SetObject ( vtkObjectPointer( &(node)), (value) );};
#endif

/// Can be used to set any vtkObject, not only vtkMRMLNodes.
#ifndef vtkSetAndObserveMRMLObjectMacro
#define vtkSetAndObserveMRMLObjectMacro(node,value)  {this->MRMLObserverManager->SetAndObserveObject ( vtkObjectPointer( &(node)), (value) );};
#endif

#ifndef vtkSetAndObserveMRMLObjectMacroNoWarning
#define vtkSetAndObserveMRMLObjectMacroNoWarning(node,value)  {this->MRMLObserverManager->SetAndObserveObject ( vtkObjectPointer( &(node)), (value), 0.0, false /* no warning */ );};
#endif

#ifndef vtkSetAndObserveMRMLObjectEventsMacro
#define vtkSetAndObserveMRMLObjectEventsMacro(node,value,events)  {this->MRMLObserverManager->SetAndObserveObjectEvents ( vtkObjectPointer( &(node)), (value), (events));};
#endif

#ifndef vtkSetAndObserveMRMLObjectEventsMacroNoWarning
#define vtkSetAndObserveMRMLObjectEventsMacroNoWarning(node,value,events)  {this->MRMLObserverManager->SetAndObserveObjectEvents ( vtkObjectPointer( &(node)), (value), (events), nullptr /* priorities */, false /* no warning */);};
#endif

#ifndef vtkObserveMRMLObjectMacro
#define vtkObserveMRMLObjectMacro(node)  {this->MRMLObserverManager->ObserveObject ( (node) );};
#endif

#ifndef vtkObserveMRMLObjectEventsMacro
#define vtkObserveMRMLObjectEventsMacro(node, events)  {this->MRMLObserverManager->AddObjectEvents ( (node), (events) );};
#endif

#ifndef vtkUnObserveMRMLObjectMacro
#define vtkUnObserveMRMLObjectMacro(node)  {this->MRMLObserverManager->RemoveObjectEvents ( (node) );};
#endif

#ifndef vtkSetReferenceStringBodyMacro
#define vtkSetReferenceStringBodyMacro(name) \
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting " << #name " to " << (_arg?_arg:"(null)") ); \
  if ( this->name == nullptr && _arg == nullptr) { return;} \
  if ( this->name && _arg && (!strcmp(this->name,_arg))) { return;} \
  std::string oldValue; \
  if (this->name) { oldValue = this->name; delete [] this->name;  } \
  if (_arg) \
    { \
    size_t n = strlen(_arg) + 1; \
    char *cp1 =  new char[n]; \
    const char *cp2 = (_arg); \
    this->name = cp1; \
    do { *cp1++ = *cp2++; } while ( --n ); \
    } \
   else \
    { \
    this->name = nullptr; \
    } \
  this->Modified(); \
  if (this->Scene && this->name) \
    { \
    if (oldValue.size() > 0) \
      { \
      this->Scene->RemoveReferencedNodeID(oldValue.c_str(), this); \
      } \
    this->Scene->AddReferencedNodeID(this->name, this); \
    }
#endif

#ifndef vtkSetReferenceStringMacro
#define vtkSetReferenceStringMacro(name) \
virtual void Set##name (const char* _arg) \
  { \
  vtkSetReferenceStringBodyMacro(name)\
  }
#endif

#ifndef vtkCxxSetReferenceStringMacro
#define vtkCxxSetReferenceStringMacro(class,name)   \
void class::Set##name (const char* _arg)            \
  {                                                 \
  vtkSetReferenceStringBodyMacro(name);             \
  }
#endif

#ifndef vtkMRMLNodeNewMacro
#define vtkMRMLNodeNewMacro(newClass) \
  vtkStandardNewMacro(newClass); \
  vtkMRMLNode* newClass::CreateNodeInstance() \
  { \
    return newClass::New(); \
  }
#endif

// Use this macro to declare that this node supports
// CopyContent.It also declares CopyContent and
// HasCopyContent methods.
//
// The use of HasCopyContent() may be needed to determine if
// a specific node class has implemented CopyContent (if only
// a parent class implemented it that may not be enough).
//
#ifndef vtkMRMLCopyContentMacro
#define vtkMRMLCopyContentMacro(thisClassName) \
    void CopyContent(vtkMRMLNode* node, bool deepCopy=true) override; \
    bool HasCopyContent() const override \
    { \
    return strcmp(#thisClassName, this->GetClassNameInternal()) == 0; \
    }
#endif

// Use this macro to declare that this node supports
// CopyContent by only declaring HasCopyContent without
// implementing CopyContent method
// (this is the case when parent classes copy all content).
#ifndef vtkMRMLCopyContentDefaultMacro
#define vtkMRMLCopyContentDefaultMacro(thisClassName) \
    bool HasCopyContent() const override \
    { \
    return strcmp(#thisClassName, this->GetClassNameInternal()) == 0; \
    }
#endif

/// \brief Abstract Superclass for all specific types of MRML nodes.
///
/// This node encapsulates the functionality common to all types of MRML nodes.
/// This includes member variables for ID, Description, and Options,
/// as well as member functions to Copy() and Write().
class VTK_MRML_EXPORT vtkMRMLNode : public vtkObject
{
  /// make the vtkMRMLScene a friend so that AddNodeNoNotify can call
  /// SetID, but that's the only class that is allowed to do so
    friend class vtkMRMLScene;
    friend class vtkMRMLSceneViewNode;

public:
  vtkTypeMacro(vtkMRMLNode,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// \brief Create instance of the default node. Like New only virtual.
  ///
  /// \note Subclasses should implement this method
  virtual vtkMRMLNode* CreateNodeInstance() = 0;

  /// \brief Returns true if the class supports deep and shallow copying node content.
  virtual bool HasCopyContent() const;

  /// Set node attributes
  ///
  /// \note
  /// Subclasses should implement this method.
  /// Call this method in the subclass implementation.
  virtual void ReadXMLAttributes(const char** atts);

  /// \brief The method should remove all pointers and observations to all nodes
  /// that are not in the scene anymore.
  ///
  /// This method is called when one or more referenced nodes are deleted from the scene.
  virtual void UpdateReferences();

  /// \brief Set dependencies between this node and a child node
  /// when parsing XML file.
  virtual void ProcessChildNode(vtkMRMLNode *){};

  /// Updates other nodes in the scene depending on this node
  /// or updates this node if it depends on other nodes when the scene is read in
  /// This method is called automatically by XML parser after all nodes are created
  virtual void UpdateScene(vtkMRMLScene *)
  {
    this->UpdateNodeReferences();
  };

  /// Updates this node if it depends on other nodes when the scene is read in
  /// This method is called by scene when a node added to a scene.
  virtual void OnNodeAddedToScene()
  {
    this->UpdateNodeReferences();
  };

  /// Write this node's information to a MRML file in XML format.
  ///
  /// \note
  /// Subclasses should implement this method.
  /// Call this method in the subclass implementation.
  /// \param indent Deprecated argument that is kept for API backward-compatibility
  virtual void WriteXML(ostream& of, int indent);

  /// Write this node's body to a MRML file in XML format.
  virtual void WriteNodeBodyXML(ostream& of, int indent);

  /// \brief Copy node contents from another node of the same type.
  /// Does not copy node ID and Scene.
  /// Performs deep copy - an independent copy is created from all data, including bulk data.
  /// \note
  /// Subclasses should implement this method.
  /// Call this method in the subclass implementation.
  virtual void Copy(vtkMRMLNode *node);

  /// \brief Copy node contents from another node of the same type.
  /// Does not copy node ID, Scene, Name, SingletonTag, HideFromEditors, AddToScene, UndoEnabled,
  /// and node references.
  /// If deepCopy is set to false then a shallow copy of bulk data (such as image or mesh data) could be made;
  /// copying may be faster but the node may share some data with the source node instead of creating
  /// an independent copy.
  /// \note
  /// If a class implements this then make sure CopyContent and HasCopyContent methods are implemented
  /// in all parent classes by adding vtkMRMLCopyContentMacro(ClassName) to the class headers.
  virtual void CopyContent(vtkMRMLNode* node, bool deepCopy=true);

  /// \brief Copy the references of the node into this.
  ///
  /// Existing references will be replaced if found in node, or removed if not
  /// in node.
  virtual void CopyReferences(vtkMRMLNode* node);

  /// \brief Copy everything (including Scene and ID) from another node of
  /// the same type.
  ///
  /// \note The node is **not** added into the scene of \a node. You must do it
  /// manually **after** calling CopyWithScene(vtkMRMLNode*) using
  /// vtkMRMLScene::AddNode(vtkMRMLNode*).
  /// Only one vtkCommand::ModifiedEvent is invoked, after the copy is fully completed.
  ///
  /// \bug Calling vtkMRMLScene::AddNode(vtkMRMLNode*) **before**
  /// CopyWithScene(vtkMRMLNode*) is **NOT** supported, it will unsynchronize
  /// the node internal caches.
  /// See [#4078](https://github.com/Slicer/Slicer/issues/4078)
  ///
  /// \sa vtkMRMLScene::AddNode(vtkMRMLNode*)
  void CopyWithScene(vtkMRMLNode *node);

  /// \brief Reset node attributes to the initial state as defined in the
  /// constructor or the passed default node.
  ///
  /// It preserves values of the following dynamic attributes that may be
  /// set by an application:
  /// * SaveWithScene
  /// * HideFromEditors
  /// * Selectable
  /// * SingletonTag.
  ///
  /// If a defaultNode pointer is passed then the values stored in that node
  /// will be used to set the node contents. If defaultNode is nullptr then the values
  /// set in the constructor of the class will be used to set the node contents.
  ///
  /// \note Other attributes that needs to be preserved should be handled in the subclass.
  ///
  /// \sa SetSaveWithScene()
  /// \sa SetHideFromEditors()
  /// \sa SetSelectable()
  /// \sa SetSingletonTag()
  virtual void Reset(vtkMRMLNode* defaultNode);

  /// \brief Start modifying the node. Disable Modify events.
  ///
  /// Returns the previous state of \a DisableModifiedEvent flag
  /// that should be passed to EndModify() method.
  ///
  /// \sa EndModify()
  virtual int StartModify()
    {
    int disabledModify = this->GetDisableModifiedEvent();
    this->DisableModifiedEventOn();
    return disabledModify;
    };

  /// \brief End modifying the node.
  ///
  /// Enable Modify events if the previous state of
  /// \a DisableModifiedEvent flag is 0.
  ///
  /// Return the number of pending events (even if
  /// InvokePendingModifiedEvent() is not called).
  virtual int EndModify(int previousDisableModifiedEventState)
    {
    this->SetDisableModifiedEvent(previousDisableModifiedEventState);
    if (!previousDisableModifiedEventState)
      {
      return this->InvokePendingModifiedEvent();
      }
    return this->ModifiedEventPending;
    };


  /// Get node XML tag name (like Volume, Model).
  ///
  /// \note Subclasses should implement this method.
  virtual const char* GetNodeTagName() = 0;

  /// \brief Set a name value pair attribute.
  ///
  /// Fires a vtkCommand::ModifiedEvent.
  ///
  /// Attributes are written in the XML.
  /// If value is 0, the attribute \a name is removed
  /// from the pair list.
  ///
  /// This function is a no-op if \a name is null or empty.
  ///
  /// \sa WriteXML()
  void SetAttribute(const char* name, const char* value);

  /// Remove attribute with the specified name.
  void RemoveAttribute(const char* name);

  /// \brief Get value of a name value pair attribute.
  ///
  /// Return nullptr if the name does not exists.
  const char* GetAttribute(const char* name);

  /// Get all attribute names.
  std::vector< std::string > GetAttributeNames();

  /// Get all attribute names. Python-wrappable version.
  void GetAttributeNames(vtkStringArray* attributeNames);

  /// Describes if the node is hidden.
  vtkGetMacro(HideFromEditors, int);
  vtkSetMacro(HideFromEditors, int);
  vtkBooleanMacro(HideFromEditors, int);

  /// Describes if the node is selectable.
  vtkGetMacro(Selectable, int);
  vtkSetMacro(Selectable, int);
  vtkBooleanMacro(Selectable, int);

  /// Specifies if the state of this node is stored in the scene's undo buffer.
  /// False by default to make sure that undo can be enabled selectively,
  /// only for nodes that are prepared to work correctly when saved/restored.
  /// Nodes with different UndoEnabled value must not reference to each other,
  /// because restoring states could lead to unresolved node references.
  /// Therefore, when undo is enabled for a certain node, it must be enabled
  /// for nodes that it references (for example, if undo is enabled for
  /// vtkMRMLModelNode then it must be enabled for vtkMRMLModelDisplayNode
  /// and vtkMRMLModelStorageNode as well).
  vtkGetMacro(UndoEnabled, bool);
  vtkSetMacro(UndoEnabled, bool);
  vtkBooleanMacro(UndoEnabled, bool);

  /// Propagate events generated in mrml.
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  /// \brief Flags to avoid event loops.
  ///
  /// \warning Do NOT use the SetMacro or it call modified on itself and
  /// generate even more events!
  vtkGetMacro(InMRMLCallbackFlag, int);
  void SetInMRMLCallbackFlag (int flag) {
    this->InMRMLCallbackFlag = flag;
  }

  /// Text description of this node, to be set by the user.
  vtkSetStringMacro(Description);
  vtkGetStringMacro(Description);

  /// Name of this node, to be set by the user
  vtkSetStringMacro(Name);
  vtkGetStringMacro(Name);

  /// ID use by other nodes to reference this node in XML.
  //vtkSetStringMacro(ID);
  vtkGetStringMacro(ID);

  /// \brief Tag that make this node a singleton in the scene.
  ///
  /// If set to nullptr, multiple instances of this node class are allowed.
  ///
  /// If set to a non-nullptr string, the node will be a singleton and
  /// the scene will replace this node instead of adding new instances.
  ///
  /// The SingletonTag is used by the scene to build a unique ID.
  ///
  /// If the there can only be one instance of a given node class in the scene,
  /// then the singleton tag should be Singleton. For example, the interaction and
  /// selection nodes are named Selection and Interaction, with Singleton tags set to
  /// Singleton, and with IDs set to vtkMRMLSelectionNodeSingleton and
  /// vtkMRMLInteractionNodeSingleton.
  /// If the singleton node is associated with a specific module it should use
  /// the module name, which already needs to be unique, as the tag. The Editor module
  /// uses this naming convention, with a parameter node that has a singleton tag
  /// of Editor and a node ID of vtkMRMLScriptedModuleNodeEditor.
  /// If the there is more than one instance of the node class then the
  /// singleton tag should be Singleton post-pended with a unique identifier
  /// for that specific node (e.g. the name). Any new color nodes should use this
  /// convention, with a name of NewName, a Singleton tag of SingletonNewName, leading
  /// to an ID of vtkMRMLColorTableNodeSingletonNewName.
  /// The existing MRML nodes don't always use these conventions but are kept unchanged
  /// for backward compatibility.
  /// \sa vtkMRMLScene::BuildID
  vtkSetStringMacro(SingletonTag);
  vtkGetStringMacro(SingletonTag);
  void SetSingletonOn()
    {
    this->SetSingletonTag("Singleton");
    }
  void SetSingletonOff()
    {
    this->SetSingletonTag(nullptr);
    }
  bool IsSingleton()
    {
    return (this->GetSingletonTag() != nullptr);
    }

  /// Save node with MRML scene.
  vtkGetMacro(SaveWithScene, int);
  vtkSetMacro(SaveWithScene, int);
  vtkBooleanMacro(SaveWithScene, int);

  /// node added to MRML scene.
  vtkGetMacro(AddToScene, int);
  vtkSetMacro(AddToScene, int);
  vtkBooleanMacro(AddToScene, int);
  void SetAddToSceneNoModify(int value);

  /// Turn on/off generating InvokeEvent for set macros
  vtkGetMacro(DisableModifiedEvent, int);
  void SetDisableModifiedEvent(int onOff)
    {
    this->DisableModifiedEvent = onOff;
    }
  void DisableModifiedEventOn()
    {
    this->SetDisableModifiedEvent(1);
    }
  void DisableModifiedEventOff()
    {
    this->SetDisableModifiedEvent(0);
    }

  /// Number of pending modified events.
  ///
  /// \sa InvokePendingModifiedEvent()
  /// \sa Modified()
  /// \sa GetDisableModifiedEvent()
  vtkGetMacro(ModifiedEventPending, int);

  /// \brief Customized version of Modified() allowing to compress
  /// vtkCommand::ModifiedEvent.
  ///
  /// It overrides the vtkObject method so that all changes to the node which
  /// would normally generate a vtkCommand::ModifiedEvent can be grouped into
  /// an `atomic` operation.
  ///
  /// Typical usage would be to disable modified events, call a series of `Set*`
  /// operations, and then re-enable modified events and call InvokePendingModifiedEvent()
  /// to invoke the event (if any of the `Set*` calls actually changed the values
  /// of the instance variables).
  ///
  /// \sa GetDisableModifiedEvent()
  void Modified() override
    {
    if (!this->GetDisableModifiedEvent())
      {
      Superclass::Modified();
      }
    else
      {
      ++this->ModifiedEventPending;
      }
    }

  /// \brief Invokes any modified events that are `pending`.
  ///
  /// Pending modified events were generated while the DisableModifiedEvent
  /// flag was nonzero.
  ///
  /// Returns the total number of pending modified events that have been
  /// replaced by the just invoked modified event(s).
  virtual int InvokePendingModifiedEvent ()
    {
    int oldModifiedEventPending = 0;
    // Invoke pending standard Modified event
    if ( this->ModifiedEventPending )
      {
      oldModifiedEventPending += this->ModifiedEventPending;
      this->ModifiedEventPending = 0;
      Superclass::Modified();
      }
    // Invoke pending custom modified events
    if (!this->CustomModifiedEventPending.empty())
      {
      // Need to make a copy of the event IDs stored in this->CustomModifiedEventPending,
      // because event invocation may add more events to this->CustomModifiedEventPending,
      // which would then make the iterator invalid.
      std::vector<int> customEventsToInvoke;
      for (std::map< int, int >::iterator it=this->CustomModifiedEventPending.begin(); it!=this->CustomModifiedEventPending.end(); ++it)
        {
        oldModifiedEventPending += it->second;
        customEventsToInvoke.push_back(it->first);
        }
      this->CustomModifiedEventPending.clear();
      for (std::vector<int>::iterator it=customEventsToInvoke.begin(); it!=customEventsToInvoke.end(); ++it)
        {
        this->InvokeEvent(*it);
        }
      }
    return oldModifiedEventPending;
    }

  /// \brief This method allows the node to compress events.
  ///
  /// Instead of invoking a certain event several times, the event is called
  /// only once, for all the invocations that are made between StartModify()
  /// and EndModify().
  ///
  /// Typical usage is to group several `...Added`, `...Removed`, `...Modified`
  /// events into one, to improve performance.
  ///
  /// \a callData is passed to InvokeEvent() if the event is invoked immediately.
  ///
  /// If the event is not invoked immediately then it will be sent with `callData=nullptr`.
  virtual void InvokeCustomModifiedEvent(int eventId, void *callData=nullptr)
    {
    if (!this->GetDisableModifiedEvent())
      {
      // DisableModify is inactive, we immediately invoke the event
      this->InvokeEvent(eventId, callData);
      }
    else
      {
      // just remember the custom modified event and invoke it once,
      // when DisableModify is deactivated
      ++this->CustomModifiedEventPending[eventId];
      }
    }

  /// Get the scene this node has been added to.
  virtual vtkMRMLScene* GetScene();

  /// \brief This method is for internal use only.
  /// Use AddNode method of vtkMRMLScene to add a node to the scene.
  ///
  /// Internally calls SetSceneReferences()
  /// \sa SetSceneReferences()
  virtual void SetScene(vtkMRMLScene* scene);

  /// \brief Update the references of the node to the scene.
  ///
  /// \note You must unsure that a valid scene is set before calling
  /// SetSceneReferences().
  virtual void SetSceneReferences();

  /// Update the stored reference to another node in the scene.
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  /// Return list of events that indicate that content of the node is modified.
  /// For example, it is not enough to observe vtkCommand::ModifiedEvent to get
  /// notified when a transform stored in a transform node is modified, but
  /// vtkMRMLTransformableNode::TransformModifiedEvent must be observed as well.
  vtkGetObjectMacro(ContentModifiedEvents, vtkIntArray);

  /// \brief Encode a URL string.
  ///
  /// Returns the string (null) if the input is null.
  ///
  /// \note Currently only works on %, space, ', ", <, >
  /// \sa URLDecodeString()
  const char *URLEncodeString(const char *inString);

  /// \brief Decode a URL string.
  ///
  /// Returns the string (null) if the input is null.
  ///
  /// \note Currently only works on %, space, ', ", <, >
  /// \sa URLEncodeString()
  const char *URLDecodeString(const char *inString);

  /// \brief Encode an XML attribute string (replaces special characters by code sequences)
  ///
  /// \sa XMLAttributeDecodeString()
  std::string XMLAttributeEncodeString(const std::string& inString);

  /// \brief Decode an XML attribute string.
  ///
  /// Important: attributes that vtkMRMLNode::ReadXMLAttributes method receives are
  /// already decoded, therefore no XML attribute decoding must not be applied to
  /// those strings.
  ///
  /// \sa XMLAttributeEncodeString()
  std::string XMLAttributeDecodeString(const std::string& inString);

  /// Get/Set for Selected
  vtkGetMacro(Selected, int);
  vtkSetMacro(Selected, int);
  vtkBooleanMacro(Selected, int);

  /// \brief Add a \a referenceRole.
  ///
  /// The referenceRole can be any unique string, for example "display", "transform" etc.
  /// Optionally a MRML attribute name for storing the reference in the mrml scene file can be specified.
  /// For example "displayNodeRef". If omitted the MRML attribute name will be the same as the role.
  /// If referenceRole ends with '/', it is considered as a "template" reference role
  /// that can be used to generate attribute names dynamically by concatenation:
  /// If referenceRole is "unit/" and mrmlAttributeName is "UnitRef", then the generated
  /// MRML attribute names for a node reference role of "unit/length" will be "lengthUnitRef".
  /// Use this method to add new reference types to a node.
  /// This method is typically called in the constructors of each subclass.
  /// The optional events argument specifies what events should be observed by default (e.g., when loading the scene from file).
  /// \sa GetReferenceNodeFromMRMLAttributeName()
  void AddNodeReferenceRole(const char *referenceRole, const char *mrmlAttributeName=nullptr, vtkIntArray *events=nullptr);

  /// \brief Set a reference to a node with specified nodeID from this node for a specific \a referenceRole.
  vtkMRMLNode* SetNodeReferenceID(const char* referenceRole, const char* referencedNodeID);

  /// Convenience method that adds a \a referencedNodeID at the end of the list.
  vtkMRMLNode* AddNodeReferenceID(const char* referenceRole , const char* referencedNodeID);

  /// \brief Set a N-th reference from this node with specified
  /// \a referencedNodeID for a specific \a referenceRole.
  vtkMRMLNode* SetNthNodeReferenceID(const char* referenceRole, int n, const char* referencedNodeID);

  /// \brief Set and observe a reference node from this node for a specific
  /// \a referenceRole.
  ///
  /// Observe Modified event by default, optionally takes array of events
  vtkMRMLNode* SetAndObserveNodeReferenceID(const char* referenceRole , const char* referencedNodeID, vtkIntArray *events=nullptr);

  /// \brief Add and observe a reference node from this node for a specific
  /// \a referenceRole.
  ///
  /// Observe Modified event by default, optionally takes array of events.
  vtkMRMLNode* AddAndObserveNodeReferenceID(const char* referenceRole , const char* referencedNodeID, vtkIntArray *events=nullptr);

  /// \brief Set and observe the Nth node ID for a specific reference role.
  ///
  /// If n is larger than the number of reference nodes, the node ID
  /// is added at the end of the list. If nodeReferenceID is 0, the node ID is
  /// removed from the list.
  /// When a node ID is set (added or changed), its corresponding node is
  /// searched (slow) into the scene and cached for fast future access.
  /// It is possible however that the node is not yet into the scene (due to
  /// some temporary state (at loading time for example). UpdateScene() can
  /// later be called to retrieve the nodes from the scene
  /// (automatically done when loading a scene). Get(Nth)NodeReference() also
  /// scan the scene if the node was not yet cached.
  /// \sa SetAndObserveNodeReferenceID(const char*)
  /// \sa AddAndObserveNodeReferenceID(const char*)
  /// \sa RemoveNthNodeReferenceID(int)
  vtkMRMLNode* SetAndObserveNthNodeReferenceID(const char* referenceRole, int n, const char *referencedNodeID, vtkIntArray *events=nullptr);

  /// Convenience method that removes the Nth node ID from the list.
  void RemoveNthNodeReferenceID(const char* referenceRole, int n);

  /// \brief Remove all node IDs and associated nodes for a specific \a referenceRole.
  ///
  /// If \a referenceRole is 0 remove references for all roles
  void RemoveNodeReferenceIDs(const char* referenceRole);

  /// \brief Return true if \a referencedNodeID is in the node ID list for a
  /// specific \a referenceRole.
  ///
  /// If nullptr is specified as role then all roles are checked.
  bool HasNodeReferenceID(const char* referenceRole, const char* referencedNodeID);

  /// \brief Return the number of node IDs for a specific reference role (and nodes as they always
  /// have the same size).
  int GetNumberOfNodeReferences(const char* referenceRole);

  /// \brief Return the string of the Nth node ID for a specific reference role.
  ///
  /// Return 0 if no such node exist.
  ///
  /// \warning A temporary char generated from a std::string::c_str()
  /// is returned.
  const char *GetNthNodeReferenceID(const char* referenceRole, int n);

  /// \brief Utility function that returns the first node id for a specific
  /// \a referenceRole.
  ///
  /// \sa GetNthNodeReferenceID(int), GetNodeReference()
  const char *GetNodeReferenceID(const char* referenceRole);

  /// \brief Get referenced MRML node for a specific \a referenceRole.
  ///
  /// Can be 0 in temporary states; e.g. if
  /// the referenced node has no scene, or if the referenced is not
  /// yet into the scene.
  /// If not cached, it tnternally scans (slow) the scene to search for the
  /// associated referenced node ID.
  /// If the referencing node is no longer in the scene (GetScene() == 0), it
  /// happens after the node is removed from the scene (scene->RemoveNode(dn),
  /// the returned referenced node is 0.
  vtkMRMLNode* GetNthNodeReference(const char* referenceRole, int n);

  /// Utility function that returns the first referenced node.
  /// \sa GetNthNodeReference(int), GetNodeReferenceID()
  vtkMRMLNode* GetNodeReference(const char* referenceRole);

  /// \brief Return a list of the referenced nodes.
  ///
  /// \warning Some nodes can be 0 when the scene is in a temporary state.
  ///
  /// \note The list of nodes is browsed (slow) to make sure the pointers are
  /// up-to-date.
  /// \sa GetNthNodeReference
  void GetNodeReferences(const char* referenceRole, std::vector<vtkMRMLNode*> &nodes);

  /// \brief Return a list of the referenced node IDs.
  ///
  /// Some IDs may be null and nodes for valid IDs may not yet be in the scene.
  ///
  /// \sa GetNodeReferences(), GetNodeReferenceID()
  void GetNodeReferenceIDs(const char* referenceRole,
                           std::vector<const char*> &referencedNodeIDs);

  /// Get reference roles of the present node references.
  /// \sa GetNodeReferenceRoles(), GetNodeReferenceRoles(), GetNthNodeReferenceRole()
  void GetNodeReferenceRoles(std::vector<std::string> &roles);

  /// Get number of node reference role names.
  /// \sa GetNodeReferenceRoles(), GetNodeReferenceRoles(), GetNthNodeReferenceRole()
  int GetNumberOfNodeReferenceRoles();

  /// Get a specific node reference role name.
  /// \sa GetNodeReferenceRoles(), GetNodeReferenceRoles(), GetNthNodeReferenceRole()
  const char* GetNthNodeReferenceRole(int n);

  /// HierarchyModifiedEvent is generated when the hierarchy node with which
  /// this node is associated changes
  enum
    {
      HierarchyModifiedEvent = 16000,
      IDChangedEvent = 16001,
      ReferenceAddedEvent,
      ReferenceModifiedEvent,
      ReferenceRemovedEvent,
      ReferencedNodeModifiedEvent
    };


protected:

  /// \brief Class to hold information about a node reference
  class VTK_MRML_EXPORT vtkMRMLNodeReference : public vtkObject
  {
  public:
    vtkTypeMacro(vtkMRMLNodeReference,vtkObject);
    static vtkMRMLNodeReference *New();
    void PrintSelf(ostream& vtkNotUsed(os), vtkIndent vtkNotUsed(indent)) override {};

  public:
    vtkSetStringMacro(ReferenceRole);
    vtkGetStringMacro(ReferenceRole);

    vtkSetStringMacro(ReferencedNodeID);
    vtkGetStringMacro(ReferencedNodeID);

    /// \brief Set the events that will be observed when the referenced node
    /// will be available.
    ///
    /// If set to nullptr then the default event list (specified for the role) will be observed.
    /// If set to an empty event list then no events will be observed.
    void SetEvents(vtkIntArray* events);
    vtkIntArray* GetEvents() const;

    void SetReferencingNode(vtkMRMLNode* node);
    vtkMRMLNode* GetReferencingNode() const;

    void SetReferencedNode(vtkMRMLNode* node);
    vtkMRMLNode* GetReferencedNode() const;

  protected:
    vtkMRMLNodeReference();
    ~vtkMRMLNodeReference() override;

    vtkMRMLNodeReference(const vtkMRMLNodeReference&);
    void operator=(const vtkMRMLNodeReference&);

    /// Name of the reference role
    char*     ReferenceRole;

    /// Points to this MRML node (that added the reference)
    vtkWeakPointer<vtkMRMLNode> ReferencingNode;
    /// The referenced node that is actually observed now
    vtkSmartPointer<vtkMRMLNode> ReferencedNode;

    /// Referenced node that should be observed (may not be the same
    /// as ReferencedNode if the ReferencedNodeID is recently changed)
    char*     ReferencedNodeID;
    /// Events that should be observed (may not be the same as ReferencedNode
    /// if the ReferencedNodeID is recently changed)
    vtkSmartPointer<vtkIntArray> Events;
  };

  vtkMRMLNode();
  /// critical to have a virtual destructor!
  ~vtkMRMLNode() override;
  vtkMRMLNode(const vtkMRMLNode&);
  void operator=(const vtkMRMLNode&);

  /// a shared set of functions that call the
  /// virtual ProcessMRMLEvents
  static void MRMLCallback( vtkObject *caller,
                            unsigned long eid, void *clientData, void *callData );

  /// \brief Get/Set the string used to manage encoding/decoding of strings/URLs
  /// with special characters.
  vtkSetStringMacro( TempURLString );
  vtkGetStringMacro( TempURLString );

  /// \brief Return the reference role (if found) associated with the attribute
  /// name found in a MRML scene file. Return 0 otherwise.
  ///
  /// \note AddNodeReference() must be called prior.
  ///
  /// \sa GetMRMLAttributeNameFromReferenceRole(), AddNodeReference(),
  /// ReadXMLAttributes(), WriteXML()
  virtual const char* GetReferenceRoleFromMRMLAttributeName(const char* attName);

  /// \brief Return the mrml attribute name (if found) associated with a reference
  /// role. Return 0 otherwise.
  ///
  /// \note AddNodeReference() must be called prior.
  ///
  /// \sa GetReferenceRoleFromMRMLAttributeName(), AddNodeReference(),
  /// ReadXMLAttributes(), WriteXML()
  virtual const char* GetMRMLAttributeNameFromReferenceRole(const char* refRole);

  /// \brief Return true if the reference role is generic (ends with '/') or false
  /// otherwise.
  /// \sa AddNodeReference()
  virtual bool IsReferenceRoleGeneric(const char* refRole);

  /// Updates references and event observations on the selected referenced nodes.
  /// If referenceRole is nullptr then all the roles will be updated.
  /// \sa UpdateNthNodeReference
  virtual void UpdateNodeReferences(const char* referenceRole = nullptr);

  /// Updates references and event observations on the selected referenced node.
  /// Update is necessary because at the time the reference was added the referenced node might not have been
  /// available in the scene yet.
  /// Prerequisites: scene is valid, n >= 0 and n < referenced node IDs list size.
  virtual void UpdateNthNodeReference(const char* referenceRole, int n);

  /// Helper function for SetAndObserveNthNodeReferenceID.
  /// Updates the event observers on the old and new referenced node.
  /// referenceToIgnore should contain the current reference that is being updated.
  vtkMRMLNode* UpdateNodeReferenceEventObserver(vtkMRMLNode *oldReferencedNode, vtkMRMLNode *newReferencedNode,
    vtkIntArray *newEvents, vtkMRMLNodeReference* referenceToIgnore);

  /// Helper function for SetAndObserveNthNodeReferenceID (through UpdateNodeReferenceEventObserver).
  /// Counts how many times the old and new node is used and what events are observed.
  /// referenceToIgnore specifies a reference (the reference that is currently being processed)
  /// that is ignored.
  /// oldReferencedNodeUseCount and oldConsolidatedEventList are only computed if oldReferencedNode!=newReferencedNode.
  void GetUpdatedReferencedNodeEventList(int& oldReferencedNodeUseCount, int& newReferencedNodeUseCount,
    vtkIntArray* oldConsolidatedEventList, vtkIntArray* newConsolidatedEventList,
    vtkMRMLNode* oldReferencedNode, vtkMRMLNode* newReferencedNode,
    vtkMRMLNodeReference* referenceToIgnore, vtkIntArray* newEvents);

  /// Remove all references and event observers to referenced nodes but leave ID's and events.
  /// References and event observers can be re-added by calling UpdateNodeReferences().
  virtual void InvalidateNodeReferences();

  /// Called when a valid node reference is added.
  /// The event is not invoked when the referenced node ID is specified,
  /// but only when a valid node pointer is obtained.
  virtual void OnNodeReferenceAdded(vtkMRMLNodeReference *reference)
  {
    this->InvokeEvent(vtkMRMLNode::ReferenceAddedEvent, reference);
  }

  /// Called when a referenced node pointer is modified.
  virtual void OnNodeReferenceModified(vtkMRMLNodeReference *reference)
  {
    this->InvokeEvent(vtkMRMLNode::ReferenceModifiedEvent, reference);
  }

  /// Called when a referenced node pointer is removed (set to nullptr).
  virtual void OnNodeReferenceRemoved(vtkMRMLNodeReference *reference)
  {
    this->InvokeEvent(vtkMRMLNode::ReferenceRemovedEvent, reference);
  }

  /// Parse references in the form "role1:id1 id2;role2:id3;"
  /// map contains existing role-id pairs, so we don't repeat them
  void ParseReferencesAttribute(const char *attValue, std::set<std::string> &references);

  /// Holders for MRML callbacks
  vtkCallbackCommand *MRMLCallbackCommand;

  char *ID{nullptr};
  char *Name{nullptr};
  char *Description{nullptr};
  int HideFromEditors{0};
  int Selectable{1};
  int Selected{0};
  int AddToScene{1};
  bool UndoEnabled{false};

  int  SaveWithScene{true};

  ///
  /// Flag to avoid event loops
  int InMRMLCallbackFlag{0};

  // We don't increase the reference count of Scene when store its pointer
  // therefore we must use a weak pointer to prevent pointer dangling when
  // the scene is deleted.
  vtkWeakPointer<vtkMRMLScene> Scene;

  typedef std::map< std::string, std::string > AttributesType;
  AttributesType Attributes;

  vtkIntArray* ContentModifiedEvents;

  vtkObserverManager *MRMLObserverManager;

  /// NodeReferences is a map that stores vector of references for each referenceRole,
  /// the referenceRole can be any unique string, for example "display", "transform" etc.
  typedef std::vector< vtkSmartPointer<vtkMRMLNodeReference> > NodeReferenceListType;
  typedef std::map< std::string, NodeReferenceListType > NodeReferencesType;
  NodeReferencesType NodeReferences;

  std::map< std::string, std::string> NodeReferenceMRMLAttributeNames;

  typedef std::map< std::string, vtkSmartPointer<vtkIntArray> > NodeReferenceEventsType;
  NodeReferenceEventsType NodeReferenceEvents; // for each role it specifies which referenced node emitted events this node should observe

private:

  /// ID use by other nodes to reference this node in XML.
  /// The ID must be unique in the scene. Only the scene can set the ID
  void SetID(const char* newID);

  /// Variable used to manage encoded/decoded URL strings
  char *TempURLString{nullptr};

  char *SingletonTag{nullptr};

  int DisableModifiedEvent{0};
  int ModifiedEventPending{0};
  std::map<int, int> CustomModifiedEventPending; // event id, pending value (number of events grouped together)
};

/// \brief Safe replacement of MRML node start/end modify.
///
/// MRMLNodeModifyBlocker can be used wherever you would otherwise use
/// a pair of calls to node->StartModify() and node->EndModify().
/// It temporarily blocks invoke of node modify and custom modify events
/// in its constructor and in the destructor it resets the state to what
/// it was before the constructor ran.
///
class VTK_MRML_EXPORT MRMLNodeModifyBlocker
{
public:
  vtkWeakPointer<vtkMRMLNode> Node;
  int WasModifying;
  MRMLNodeModifyBlocker(vtkMRMLNode* node)
  {
    this->Node = node;
    if (this->Node)
      {
      this->WasModifying = this->Node->StartModify();
      }
  };
  ~MRMLNodeModifyBlocker()
  {
    if (this->Node)
      {
      this->Node->EndModify(this->WasModifying);
      }
  }
};

#endif
