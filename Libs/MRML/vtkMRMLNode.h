/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.18 $

=========================================================================auto=*/
// .NAME vtkMRMLNode - Abstract Superclass for all specific types of MRML nodes.
// .SECTION Description
// This node encapsulates the functionality common to all types of MRML nodes.
// This includes member variables for ID, Description, and Options,
// as well as member functions to Copy() and Write().

#ifndef __vtkMRMLNode_h
#define __vtkMRMLNode_h

#include <string>
#include <map>

#include "vtkObject.h"

#include "vtkMRML.h"
#include "vtkObserverManager.h"
#include "vtkEventBroker.h"

#ifndef vtkSetMRMLObjectMacro
#define vtkSetMRMLObjectMacro(node,value)  {this->MRMLObserverManager->SetObject ( vtkObjectPointer( &(node)), (value) );};
#endif

#ifndef vtkSetAndObserveMRMLObjectMacro
#define vtkSetAndObserveMRMLObjectMacro(node,value)  {this->MRMLObserverManager->SetAndObserveObject ( vtkObjectPointer( &(node)), (value) );};
#endif

#ifndef vtkSetAndObserveMRMLObjectEventsMacro
#define vtkSetAndObserveMRMLObjectEventsMacro(node,value,events)  {this->MRMLObserverManager->SetAndObserveObjectEvents ( vtkObjectPointer( &(node)), (value), (events));};
#endif

//BTX
#ifndef vtkSetReferenceStringMacro
#define vtkSetReferenceStringMacro(name) \
virtual void Set##name (const char* _arg) \
  { \
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
    } \
  } 
#endif
//ETX

class vtkMRMLScene;
class vtkCallbackCommand;

class VTK_MRML_EXPORT vtkMRMLNode : public vtkObject
{
  //BTX
  // Description:
  // make the vtkMRMLScene a friend so that AddNodeNoNotify can call
  // SetID, but that's the only class that is allowed to do so
    friend class vtkMRMLScene;
  //ETX
public:
  static vtkMRMLNode *New();
  vtkTypeMacro(vtkMRMLNode,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Create instance of the default node. Like New only virtual
  // NOTE: Subclasses should implement this method
  virtual vtkMRMLNode* CreateNodeInstance() = 0;

  // Description:
  // Set node attributes
  // NOTE: Subclasses should implement this method
  // NOTE: Call this method in the subclass impementation
  virtual void ReadXMLAttributes(const char** atts);

  // Description:
  // Updates this node if it depends on other nodes 
  // when the node is deleted in the scene
  virtual void UpdateReferences() {};

  // Description:
  // Set dependencies between this node and the parent node
  // when parsing XML file
  virtual void ProcessParentNode(vtkMRMLNode *){};

  // Description:
  // Set dependencies between this node and a child node
  // when parsing XML file
  virtual void ProcessChildNode(vtkMRMLNode *){};
  
  // Description:
  // Updates other nodes in the scene depending on this node
  // or updates this node if it depends on other nodes when the scene is read in
  // This method is called automatically by XML parser after all nodes are created
  virtual void UpdateScene(vtkMRMLScene *) {};

  // Description:
  // Write this node's information to a MRML file in XML format.
  // NOTE: Subclasses should implement this method
  // NOTE: Call this method in the subclass impementation
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Write this node's body to a MRML file in XML format.
  virtual void WriteNodeBodyXML(ostream& of, int indent){};

  
  // Description:
  // Copy paramters (not including ID and Scene) from another node of the same type.
  // NOTE: Subclasses should implement this method
  // NOTE: Call this method in the subclass impementation
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Copy everything from another node of the same type.
  void CopyWithScene(vtkMRMLNode *node);
  
  // Description:
  // Reset node attributes to the initilal state as defined in the constructor.
  // NOTE:   it preserves values several dynamic attributes that may be set by an application:
  // SaveWithScene, HideFromEditors, Selectable, SingletonTag
  // NOTE: other attributes that needs to be preserved should be handled in the subclass
  virtual void Reset();


  // Description:
  // Get node XML tag name (like Volume, Model)
  // NOTE: Subclasses should implement this method
  virtual const char* GetNodeTagName() = 0;

  // Description:
  // Set a name value pair attribute
  void SetAttribute(const char* name, const char* value);

  // Description:
  // Get value of a name value pair attribute
  // or NULL if the name does not exists
  const char* GetAttribute(const char* name);

  // Description:
  // Describes if the node is hidden
  vtkGetMacro(HideFromEditors, int);
  vtkSetMacro(HideFromEditors, int);
  vtkBooleanMacro(HideFromEditors, int);

  // Description:
  // Describes if the node is selectable
  vtkGetMacro(Selectable, int);
  vtkSetMacro(Selectable, int);
  vtkBooleanMacro(Selectable, int);

  
  // Description:
  // method to propagate events generated in mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData ) { };

  // Description:
  // Flags to avoid event loops
  // NOTE: don't use the SetMacro or it call modified itself and generate even more events!
  vtkGetMacro(InMRMLCallbackFlag, int);
  void SetInMRMLCallbackFlag (int flag) {
    this->InMRMLCallbackFlag = flag;
  }

  // Description:
  // Text description of this node, to be set by the user
  vtkSetStringMacro(Description);
  vtkGetStringMacro(Description);

  // Description:
  // Root directory of MRML scene
  vtkSetStringMacro(SceneRootDir);
  vtkGetStringMacro(SceneRootDir);
  
  // Description:
  // Name of this node, to be set by the user
  vtkSetStringMacro(Name);
  vtkGetStringMacro(Name);
  
  
  // Description:
  // Node's effect on indentation when displaying the
  // contents of a MRML file. (0, +1, -1)
  vtkGetMacro(Indent, int);
  
  // Description:
  // ID use by other nodes to reference this node in XML
//  vtkSetStringMacro(ID);
  vtkGetStringMacro(ID);

  // Description:
  // Tag that make this node a singleton in the scene
  // if NULL multiple instances of this node class allowed,
  // otherwise scene can only replace this node not add new instances.
  vtkSetStringMacro(SingletonTag);
  vtkGetStringMacro(SingletonTag);

  // Description:
  // Describes if the data has been modified
  vtkGetMacro(ModifiedSinceRead, int);
  vtkSetMacro(ModifiedSinceRead, int);
  vtkBooleanMacro(ModifiedSinceRead, int);

  // Description:
  // Save node with MRML scene
  vtkGetMacro(SaveWithScene, int);
  vtkSetMacro(SaveWithScene, int);
  vtkBooleanMacro(SaveWithScene, int);

  // Description:
  // node added to MRML scene
  vtkGetMacro(AddToScene, int);
  vtkSetMacro(AddToScene, int);
  vtkBooleanMacro(AddToScene, int);

  // Description:
  // Turn off generating InvokeEvent for set macros
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

  virtual void Modified() 
    {
    if (!this->GetDisableModifiedEvent())
      {
      Superclass::Modified();
      }
    else
      {
      this->ModifiedEventPending = 1;
      }
    }

  void InvokePendingModifiedEvent ()
    {
    if ( this->ModifiedEventPending )
      {
      Superclass::Modified();
      }
    this->ModifiedEventPending = 0;
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
  
  vtkMRMLScene* GetScene() {return this->Scene;};
  void SetScene(vtkMRMLScene* scene) {this->Scene = scene;};

  // Description:
  // Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID) {};

  // Description:
  // Utility function that takes in a string and returns a URL encoded
  // string. Returns the string (null) if the input is null.
  // Currently only works on %, space, ', ", <, >
  const char *URLEncodeString(const char *inString);
  // Description:
  // Utility function that takes in a URL encoded string and returns a regular
  // one. Returns the string (null) if the input is null
  // Currently only works on %, space, ', ", <, >
  const char *URLDecodeString(const char *inString);
  
protected:
  
  vtkMRMLNode();
  // critical to have a virtual destructor!
  virtual ~vtkMRMLNode();
  vtkMRMLNode(const vtkMRMLNode&);
  void operator=(const vtkMRMLNode&);
  
  vtkSetMacro(Indent, int);

  //BTX
  // a shared set of functions that call the
  // virtual ProcessMRMLEvents
  static void MRMLCallback( vtkObject *caller,
                            unsigned long eid, void *clientData, void *callData );

  // Description::
  // Holders for MRML callbacks
  vtkCallbackCommand *MRMLCallbackCommand;

  // Description:
  // Flag to avoid event loops
  int InMRMLCallbackFlag;

  char *Description;
  char *SceneRootDir;
  char *Name;
  char *ID;
  char *SingletonTag;
  int Indent;
  int HideFromEditors;
  int Selectable;
  int AddToScene;

  int  ModifiedSinceRead;
  int  SaveWithScene;

  vtkMRMLScene *Scene;

  //BTX
  std::map< std::string, std::string > Attributes;
  //ETX

  vtkObserverManager *MRMLObserverManager;

  // Description:
  // Get/Set the string used to manage encoding/decoding of strings/URLs with special characters
  vtkSetStringMacro( TempURLString );
  vtkGetStringMacro( TempURLString );
  
private:
  // Description:
  // ID use by other nodes to reference this node in XML
  vtkSetStringMacro(ID);
 
  // Description:
  // Return the string that can be used for the id given a string and an
  // index
  const char* ConstructID(const char * str, int index);

  // Description:
  // Set the ID from a string and an index, calls ConstructID
  void ConstructAndSetID(const char * str, int index);

  //BTX
  // Description:
  // Variable used to manage constructed ids
  std::string TempID;
  //ETX

  // Description:
  // Variable used to manage encoded/decoded URL strings
  char * TempURLString;

  int DisableModifiedEvent;
  int ModifiedEventPending;
};

#endif



