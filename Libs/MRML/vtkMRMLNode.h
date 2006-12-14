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

#ifndef vtkSetMRMLObjectMacro
#define vtkSetMRMLObjectMacro(node,value)  {this->MRMLObserverManager->SetObject ( vtkObjectPointer( &(node)), (value) );};
#endif

#ifndef vtkSetAndObserveMRMLObjectMacro
#define vtkSetAndObserveMRMLObjectMacro(node,value)  {this->MRMLObserverManager->SetAndObserveObject ( vtkObjectPointer( &(node)), (value) );};
#endif

#ifndef vtkSetAndObserveMRMLObjectEventsMacro
#define vtkSetAndObserveMRMLObjectEventsMacro(node,value,events)  {this->MRMLObserverManager->SetAndObserveObjectEvents ( vtkObjectPointer( &(node)), (value), (events));};
#endif


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
  // Copy everything from another node of the same type.
  // NOTE: Subclasses should implement this method
  // NOTE: Call this method in the subclass impementation
  virtual void Copy(vtkMRMLNode *node);

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
  // Describes if the data has been modified
  vtkGetMacro(ModifiedSinceRead, int);
  vtkSetMacro(ModifiedSinceRead, int);
  vtkBooleanMacro(ModifiedSinceRead, int);

  // Description:
  // Save node with MRML scene
  vtkGetMacro(SaveWithScene, int);
  vtkSetMacro(SaveWithScene, int);
  vtkBooleanMacro(SaveWithScene, int);
  
  vtkMRMLScene* GetScene() {return this->Scene;};
  void SetScene(vtkMRMLScene* scene) {this->Scene = scene;};

  // Description:
  // Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID) {};

  
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
  int Indent;
  int HideFromEditors;

  int  ModifiedSinceRead;
  int  SaveWithScene;

  vtkMRMLScene *Scene;

  //BTX
  std::map< std::string, std::string > Attributes;
  //ETX

  vtkObserverManager *MRMLObserverManager;

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

};

#endif



