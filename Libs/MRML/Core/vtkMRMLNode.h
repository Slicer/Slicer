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
class vtkMRMLScene;


// VTK includes
#include <vtkObject.h>
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
  virtual void UpdateReferences() {};

  /// 
  /// Set dependencies between this node and a child node
  /// when parsing XML file
  virtual void ProcessChildNode(vtkMRMLNode *){};
  
  /// 
  /// Updates other nodes in the scene depending on this node
  /// or updates this node if it depends on other nodes when the scene is read in
  /// This method is called automatically by XML parser after all nodes are created
  virtual void UpdateScene(vtkMRMLScene *) {};

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
    int oldMode = this->GetDisableModifiedEvent();
    this->DisableModifiedEventOn();
    this->CopyWithScene(node);
    this->InvokePendingModifiedEvent();
    this->SetDisableModifiedEvent(oldMode);
    }

  void CopyWithSceneWithoutModifiedEvent (vtkMRMLNode *node)
    {
    int oldMode = this->GetDisableModifiedEvent();
    this->DisableModifiedEventOn();
    this->CopyWithScene(node);
    this->SetDisableModifiedEvent(oldMode);
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

  /// HierarchyModifiedEvent is generated when the hierarchy node with which
  /// this node is associated changes
  enum
    {
      HierarchyModifiedEvent = 16000,
      IDChangedEvent = 16001
    };

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
