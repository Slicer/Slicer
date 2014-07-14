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


// VTK includes
#include <vtkObject.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>
class vtkCallbackCommand;

// STD includes
#include <string>
#include <map>
#include <vector>

#ifndef vtkSetMRMLObjectMacro
#define vtkSetMRMLObjectMacro(node,value)  {this->MRMLObserverManager->SetObject ( vtkObjectPointer( &(node)), (value) );};
#endif

/// Can be used to set any vtkObject, not only vtkMRMLNodes.
#ifndef vtkSetAndObserveMRMLObjectMacro
#define vtkSetAndObserveMRMLObjectMacro(node,value)  {this->MRMLObserverManager->SetAndObserveObject ( vtkObjectPointer( &(node)), (value) );};
#endif

#ifndef vtkSetAndObserveMRMLObjectEventsMacro
#define vtkSetAndObserveMRMLObjectEventsMacro(node,value,events)  {this->MRMLObserverManager->SetAndObserveObjectEvents ( vtkObjectPointer( &(node)), (value), (events));};
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
  if ( this->name == NULL && _arg == NULL) { return;} \
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
    this->name = NULL; \
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


/// \brief Abstract Superclass for all specific types of MRML nodes.
///
/// This node encapsulates the functionality common to all types of MRML nodes.
/// This includes member variables for ID, Description, and Options,
/// as well as member functions to Copy() and Write().
class VTK_MRML_EXPORT vtkMRMLNode : public vtkObject
{
  ///
  /// make the vtkMRMLScene a friend so that AddNodeNoNotify can call
  /// SetID, but that's the only class that is allowed to do so
    friend class vtkMRMLScene;
    friend class vtkMRMLSceneViewNode;

public:
  vtkTypeMacro(vtkMRMLNode,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  ///
  /// Create instance of the default node. Like New only virtual
  /// NOTE: Subclasses should implement this method
  virtual vtkMRMLNode* CreateNodeInstance() = 0;

  ///
  /// Set node attributes
  /// NOTE: Subclasses should implement this method
  /// NOTE: Call this method in the subclass impementation
  virtual void ReadXMLAttributes(const char** atts);

  ///
  /// Updates this node if it depends on other nodes
  /// when the node is deleted in the scene
  virtual void UpdateReferences();

  ///
  /// Set dependencies between this node and a child node
  /// when parsing XML file
  virtual void ProcessChildNode(vtkMRMLNode *){};

  ///
  /// Updates other nodes in the scene depending on this node
  /// or updates this node if it depends on other nodes when the scene is read in
  /// This method is called automatically by XML parser after all nodes are created
  virtual void UpdateScene(vtkMRMLScene *)
  {
    this->UpdateNodeReferences();
  };

  ///
  /// Updates this node if it depends on other nodes when the scene is read in
  /// This method is called by scene when a node added to a scene.
  virtual void OnNodeAddedToScene()
  {
    this->UpdateNodeReferences();
  };

  ///
  /// Write this node's information to a MRML file in XML format.
  /// NOTE: Subclasses should implement this method
  /// NOTE: Call this method in the subclass impementation
  virtual void WriteXML(ostream& of, int indent);

  ///
  /// Write this node's body to a MRML file in XML format.
  virtual void WriteNodeBodyXML(ostream& of, int indent);

  ///
  /// Copy parameters (not including ID and Scene) from another node of the same type.
  /// NOTE: Subclasses should implement this method
  /// NOTE: Call this method in the subclass impementation
  virtual void Copy(vtkMRMLNode *node);

  /// Copy the references of the node into this.
  /// Existing references will be replaced if found in node, or removed if not
  /// in node.
  virtual void CopyReferences(vtkMRMLNode* node);

  ///
  /// Copy everything (including Scene and ID) from another node of the same
  /// type.
  /// Note the the node is not added into the scene of \a node. You must do it
  /// manually before or after CopyWithScene.
  void CopyWithScene(vtkMRMLNode *node);

  ///
  /// Reset node attributes to the initilal state as defined in the constructor.
  /// NOTE:   it preserves values several dynamic attributes that may be set by an application:
  /// SaveWithScene, HideFromEditors, Selectable, SingletonTag
  /// NOTE: other attributes that needs to be preserved should be handled in the subclass
  virtual void Reset();

  ///
  /// Start modifying the node. Disable Modify events.
  /// Returns the previous state of DisableModifiedEvent flag
  /// that should be passed to EndModify() method
  virtual int StartModify()
    {
    int disabledModify = this->GetDisableModifiedEvent();
    this->DisableModifiedEventOn();
    return disabledModify;
    };

  ///
  /// End modifying the node. Enable Modify events if the
  /// previous state of DisableModifiedEvent flag is 0.
  /// Return the number of pending events (even if
  /// InvokePendingModifiedEvent is not called.
  virtual int EndModify(int previousDisableModifiedEventState)
    {
    this->SetDisableModifiedEvent(previousDisableModifiedEventState);
    if (!previousDisableModifiedEventState)
      {
      return this->InvokePendingModifiedEvent();
      }
    return this->ModifiedEventPending;
    };


  ///
  /// Get node XML tag name (like Volume, Model)
  /// NOTE: Subclasses should implement this method
  virtual const char* GetNodeTagName() = 0;

  ///
  /// Set a name value pair attribute. Fires a Modified event.
  /// Attributes are written in the XML.
  /// If value is 0, the attribute \a name is removed
  /// from the pair list.
  /// no-op if \a name is null or empty
  void SetAttribute(const char* name, const char* value);

  ///
  /// Remove attribute with the specified name
  void RemoveAttribute(const char* name);

  ///
  /// Get value of a name value pair attribute
  /// or NULL if the name does not exists
  const char* GetAttribute(const char* name);

  ///
  /// Get all attribute names
  std::vector< std::string > GetAttributeNames();

  ///
  /// Describes if the node is hidden
  vtkGetMacro(HideFromEditors, int);
  vtkSetMacro(HideFromEditors, int);
  vtkBooleanMacro(HideFromEditors, int);

  ///
  /// Describes if the node is selectable
  vtkGetMacro(Selectable, int);
  vtkSetMacro(Selectable, int);
  vtkBooleanMacro(Selectable, int);


  ///
  /// method to propagate events generated in mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  ///
  /// Flags to avoid event loops
  /// NOTE: don't use the SetMacro or it call modified itself and generate even more events!
  vtkGetMacro(InMRMLCallbackFlag, int);
  void SetInMRMLCallbackFlag (int flag) {
    this->InMRMLCallbackFlag = flag;
  }

  ///
  /// Text description of this node, to be set by the user
  vtkSetStringMacro(Description);
  vtkGetStringMacro(Description);

  ///
  /// Root directory of MRML scene
  vtkSetStringMacro(SceneRootDir);
  vtkGetStringMacro(SceneRootDir);

  ///
  /// Name of this node, to be set by the user
  vtkSetStringMacro(Name);
  vtkGetStringMacro(Name);


  ///
  /// Node's effect on indentation when displaying the
  /// contents of a MRML file. (0, +1, -1)
  vtkGetMacro(Indent, int);

  ///
  /// ID use by other nodes to reference this node in XML
  //vtkSetStringMacro(ID);
  vtkGetStringMacro(ID);

  ///
  /// Tag that make this node a singleton in the scene
  /// if NULL multiple instances of this node class allowed,
  /// otherwise scene can only replace this node not add new instances.
  /// The SingletonTag is used by the scene to build a unique ID.
  /// \sa vtkMRMLScene::BuildID
  vtkSetStringMacro(SingletonTag);
  vtkGetStringMacro(SingletonTag);

  ///
  /// Save node with MRML scene
  vtkGetMacro(SaveWithScene, int);
  vtkSetMacro(SaveWithScene, int);
  vtkBooleanMacro(SaveWithScene, int);

  ///
  /// node added to MRML scene
  vtkGetMacro(AddToScene, int);
  vtkSetMacro(AddToScene, int);
  vtkBooleanMacro(AddToScene, int);
  void SetAddToSceneNoModify(int value);

  ///
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

  /// Count of pending modified events
  vtkGetMacro(ModifiedEventPending, int);

  ///
  /// overrides the vtkObject method so that all changes to the node which would normally
  /// generate a ModifiedEvent can be grouped into an 'atomic' operation.  Typical usage
  /// would be to disable modified events, call a series of Set* operations, and then re-enable
  /// modified events and call InvokePendingModifiedEvent to invoke the event (if any of the Set*
  /// calls actually changed the values of the instance variables).
  virtual void Modified()
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

  ///
  /// Invokes any modified events that are 'pending', meaning they were generated
  /// while the DisableModifiedEvent flag was nonzero.
  /// Returns the old flag state.
  virtual int InvokePendingModifiedEvent ()
    {
    if ( this->ModifiedEventPending )
      {
      int oldModifiedEventPending = this->ModifiedEventPending;
      this->ModifiedEventPending = 0;
      Superclass::Modified();
      return oldModifiedEventPending;
      }
    return this->ModifiedEventPending;
    }

  void CopyWithSingleModifiedEvent (vtkMRMLNode *node)
    {
    int oldMode = this->GetDisableModifiedEvent();
    this->DisableModifiedEventOn();
    this->Copy(node);
    this->InvokePendingModifiedEvent();
    this->SetDisableModifiedEvent(oldMode);
    }

  void CopyWithoutModifiedEvent (vtkMRMLNode *node)
    {
    int oldMode = this->GetDisableModifiedEvent();
    this->DisableModifiedEventOn();
    this->Copy(node);
    this->SetDisableModifiedEvent(oldMode);
    }

  void CopyWithSceneWithSingleModifiedEvent (vtkMRMLNode *node)
    {
    int oldMode = this->StartModify();
    this->CopyWithScene(node);
    this->EndModify(oldMode);
    }

  ///
  /// Only the scene can set itself to the node
  /// Internally calls SetSceneReferences()
  /// \sa SetSceneReferences()
  vtkGetObjectMacro(Scene, vtkMRMLScene);
  virtual void SetScene(vtkMRMLScene* scene);

  /// Update the references of the node to the scene.
  /// You must unsure that a valid scene is set before calling
  /// SetSceneReferences().
  virtual void SetSceneReferences();

  ///
  /// Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  ///
  /// Utility function that takes in a string and returns a URL encoded
  /// string. Returns the string (null) if the input is null.
  /// Currently only works on %, space, ', ", <, >
  const char *URLEncodeString(const char *inString);
  ///
  /// Utility function that takes in a URL encoded string and returns a regular
  /// one. Returns the string (null) if the input is null
  /// Currently only works on %, space, ', ", <, >
  const char *URLDecodeString(const char *inString);

  /// Get/Set for Selected
  vtkGetMacro(Selected, int);
  vtkSetMacro(Selected, int);
  vtkBooleanMacro(Selected, int);

  ///
  /// the referenceRole can be any unique string, for example "display", "transform" etc.
  /// Optionally a MRML attribute name for storing the reference in the mrml scene file can be specified.
  /// For example "displayNodeRef". If ommitted the MRML attribute name will be the same as the role.
  /// If referenceRole ends with '/', it is considered as a "template" reference role
  /// that can be used to generate attribute names dynamically by concatenation:
  /// If referenceRole is "unit/" and mrmlAttributeName is "UnitRef", then the generated
  /// MRML attribute names for a node reference role of "unit/length" will be "lengthUnitRef".
  /// Use this method to add new reference types to a node.
  /// This method is typically called in the contructors of each subclass.
  /// \sa GetReferenceNodeFromMRMLAttributeName()
  void AddNodeReferenceRole(const char *referenceRole, const char *mrmlAttributeName=0, vtkIntArray *events=0);

  ///
  /// set a reference to a node with specified nodeID from this node for a specific referenceRole
  vtkMRMLNode* SetNodeReferenceID(const char* referenceRole, const char* referencedNodeID);

  /// Convenience method that adds a reference node ID at the end of the list.
  vtkMRMLNode* AddNodeReferenceID(const char* referenceRole , const char* referencedNodeID);

  ///
  /// set a N-th reference from this node with specified referencedNodeID for a specific referenceRole
  vtkMRMLNode* SetNthNodeReferenceID(const char* referenceRole, int n, const char* referencedNodeID);

  ///
  /// set and observe a reference node from this node for a specific reference role
  /// observe Modified event by default, optionally takes array of events
  vtkMRMLNode* SetAndObserveNodeReferenceID(const char* referenceRole , const char* referencedNodeID, vtkIntArray *events=0);

  ///
  /// add and observe a reference node from this node for a specific reference role
  /// observe Modified event by default, optionally takes array of events
  vtkMRMLNode* AddAndObserveNodeReferenceID(const char* referenceRole , const char* referencedNodeID, vtkIntArray *events=0);

  ///
  /// Set and observe the Nth node ID for a specific reference role.
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
  /// \sa SetAndObserveNodeReferenceID(const char*),
  /// AddAndObserveNodeReferenceID(const char *), RemoveNthNodeReferenceID(int)
  vtkMRMLNode* SetAndObserveNthNodeReferenceID(const char* referenceRole, int n, const char *referencedNodeID, vtkIntArray *events=0);

  ///
  /// Convenience method that removes the Nth node ID from the list
  ///
  void RemoveNthNodeReferenceID(const char* referenceRole, int n);

  ///
  /// Remove all node IDs and associated nodes for a specific reference role.
  /// If referenceRole is 0 remove references for all roles
  void RemoveAllNodeReferenceIDs(const char* referenceRole);

  ///
  /// Return true if NodeReferenceID is in the node ID list for a specific reference role.
  bool HasNodeReferenceID(const char* referenceRole, const char* referencedNodeID);

  ///
  /// Return the number of node IDs for a specific reference role(and nodes as they always
  /// have the same size).
  int GetNumberOfNodeReferences(const char* referenceRole);
  ///
  /// Return the string of the Nth node ID for a specific reference role. Or 0 if no such
  /// node exist.
  /// Warning, a temporary char generated from a std::string::c_str()
  /// is returned.
  const char *GetNthNodeReferenceID(const char* referenceRole, int n);

  ///
  /// Utility function that returns the first node id for a specific reference role.
  /// \sa GetNthNodeReferenceID(int), GetNodeReference()
  const char *GetNodeReferenceID(const char* referenceRole);

  ///
  /// Get referenced MRML node for a specific reference role. Can be 0 in temporary states; e.g. if
  /// the referenced node has no scene, or if the referenced is not
  /// yet into the scene.
  /// If not cached, it tnternally scans (slow) the scene to search for the
  /// associated referenced node ID.
  /// If the referencing node is no longer in the scene (GetScene() == 0), it
  /// happens after the node is removed from the scene (scene->RemoveNode(dn),
  /// the returned referenced node is 0.
  vtkMRMLNode* GetNthNodeReference(const char* referenceRole, int n);

  ///
  /// Utility function that returns the first referenced node.
  /// \sa GetNthNodeReference(int), GetNodeReferenceID()
  vtkMRMLNode* GetNodeReference(const char* referenceRole);

  ///
  /// Return a list of the referenced nodes. Some nodes can be 0
  /// when the scene is in a temporary state.
  /// The list of nodes is browsed (slow) to make sure the pointers are
  /// up-to-date.
  /// \sa GetNthNodeReference
  void GetNodeReferences(const char* referenceRole, std::vector<vtkMRMLNode*> &nodes);

  ///
  /// Return a list of the referenced node IDs. Some IDs may be null and nodes
  /// for valid IDs may not yet be in the scene.
  /// \sa GetNodeReferences(), GetNodeReferenceID()
  void GetNodeReferenceIDs(const char* referenceRole,
                           std::vector<const char*> &referencedNodeIDs);


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
  ///
  /// class to hold information about a referenced node used by refering node
  class VTK_MRML_EXPORT vtkMRMLNodeReference : public vtkObject
  {
  public:
    vtkTypeMacro(vtkMRMLNodeReference,vtkObject);
    static vtkMRMLNodeReference *New();
    void PrintSelf(ostream& vtkNotUsed(os), vtkIndent vtkNotUsed(indent)){};

  public:
    vtkSetStringMacro(ReferenceRole);
    vtkGetStringMacro(ReferenceRole);

    vtkSetStringMacro(ReferencedNodeID);
    vtkGetStringMacro(ReferencedNodeID);

    vtkWeakPointer<vtkMRMLNode> ReferencingNode;
    vtkSmartPointer<vtkMRMLNode> ReferencedNode;
    vtkSmartPointer<vtkIntArray> Events;

  protected:

    vtkMRMLNodeReference()
      {
      this->ReferencedNodeID = 0;
      this->ReferenceRole = 0;
      }

    ~vtkMRMLNodeReference()
      {
      if (this->ReferencedNodeID)
        {
        delete [] this->ReferencedNodeID;
        this->ReferencedNodeID = 0;
        }
      }

    vtkMRMLNodeReference(const vtkMRMLNodeReference&);
    void operator=(const vtkMRMLNodeReference&);

    char*     ReferenceRole;
    char*     ReferencedNodeID;

  };

  /// NodeReferences is a map that stores vector of refererences for each referenceRole,
  /// the referenceRole can be any unique string, for example "display", "transform" etc.
  typedef std::vector< vtkSmartPointer<vtkMRMLNodeReference> > NodeReferenceListType;
  typedef std::map< std::string, NodeReferenceListType > NodeReferencesType;
  NodeReferencesType NodeReferences;

  std::map< std::string, std::string> NodeReferenceMRMLAttributeNames;

  typedef std::map< std::string, vtkSmartPointer<vtkIntArray> > NodeReferenceEventsType;
  NodeReferenceEventsType NodeReferenceEvents;

protected:

  vtkMRMLNode();
  /// critical to have a virtual destructor!
  virtual ~vtkMRMLNode();
  vtkMRMLNode(const vtkMRMLNode&);
  void operator=(const vtkMRMLNode&);

  vtkSetMacro(Indent, int);

  /// a shared set of functions that call the
  /// virtual ProcessMRMLEvents
  static void MRMLCallback( vtkObject *caller,
                            unsigned long eid, void *clientData, void *callData );

  /// :
  /// Holders for MRML callbacks
  vtkCallbackCommand *MRMLCallbackCommand;

  ///
  /// Flag to avoid event loops
  int InMRMLCallbackFlag;

  char *Description;
  char *SceneRootDir;
  char *Name;
  char *ID;
  int Indent;
  int HideFromEditors;
  int Selectable;
  int Selected;
  int AddToScene;

  int  SaveWithScene;

  vtkMRMLScene *Scene;

  typedef std::map< std::string, std::string > AttributesType;
  AttributesType Attributes;

  vtkObserverManager *MRMLObserverManager;

  ///
  /// Get/Set the string used to manage encoding/decoding of strings/URLs with special characters
  vtkSetStringMacro( TempURLString );
  vtkGetStringMacro( TempURLString );

  /// Return the reference role (if found) associated with the attribute
  /// name found in a MRML scene file. Return 0 otherwise.
  /// Note that AddNodeReference() must be called prior.
  /// \sa GetMRMLAttributeNameFromReferenceRole(), AddNodeReference(),
  /// ReadXMLAttributes(), WriteXML()
  virtual const char* GetReferenceRoleFromMRMLAttributeName(const char* attName);

  /// Return the mrml attribute name (if found) associated with a reference
  /// role. Return 0 otherwise.
  /// Note that AddNodeReference() must be called prior.
  /// \sa GetReferenceRoleFromMRMLAttributeName(), AddNodeReference(),
  /// ReadXMLAttributes(), WriteXML()
  virtual const char* GetMRMLAttributeNameFromReferenceRole(const char* refRole);

  /// Return true if the reference role is generic (ends with '/') or false
  /// otherwise.
  /// \sa AddNodeReference()
  virtual bool IsReferenceRoleGeneric(const char* refRole);

  ///
  /// Call UpdateNthNodeReference(referenceRole, i) on all nodes.
  void UpdateNodeReferences(const char* referenceRole);

  ///
  /// Call UpdateNthNodeReference(referenceRole, i) on all nodes for all roles
  void UpdateNodeReferences();

  /// Search the referenced node in the scene that match the associated node ID.
  /// Prerequisites: scene is valid, n >= 0 and n < referenced node IDs list size
  void UpdateNthNodeReference(const char* referenceRole, int n);

  void UpdateNthNodeReference(vtkMRMLNodeReference *reference, int n);

  void SetAndObserveNthNodeReference(const char* referenceRole, int n, vtkMRMLNode *referencedNode, vtkIntArray *events=0);

  /// Delete all internal references
  void DeleteAllReferences(bool callOnNodeReferenceRemoved=true);

  /// Remove all referenced nodes but leave ID's
  void RemoveAllReferencedNodes();

  ///
  /// Called when a node reference ID is added (list size increased).
  virtual void OnNodeReferenceAdded(vtkMRMLNodeReference *reference)
  {
    this->InvokeEvent(vtkMRMLNode::ReferenceAddedEvent, reference);
  }

  ///
  /// Called when a node reference ID is modified.
  virtual void OnNodeReferenceModified(vtkMRMLNodeReference *reference)
  {
    this->InvokeEvent(vtkMRMLNode::ReferenceModifiedEvent, reference);
  }

  ///
  /// Called after a node reference ID is removed (list size decreased).
  virtual void OnNodeReferenceRemoved(vtkMRMLNodeReference *reference)
  {
    this->InvokeEvent(vtkMRMLNode::ReferenceRemovedEvent, reference);
  }

  /// parse references in the form "role1:id1 id2;role2:id3;"
  /// map contains existing role-id pairs, so we don't repeat them
  void ParseReferencesAttribute(const char *attValue, std::map<std::string, std::string> &references);

private:
  ///
  /// ID use by other nodes to reference this node in XML.
  /// The ID must be unique in the scene. Only the scene can set the ID
  void SetID(const char* newID);

  ///
  /// Variable used to manage encoded/decoded URL strings
  char * TempURLString;

  char *SingletonTag;

  int DisableModifiedEvent;
  int ModifiedEventPending;

};

#endif
