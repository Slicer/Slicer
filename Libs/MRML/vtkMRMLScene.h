/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLScene.h,v $
Date:      $Date: 2006/03/17 15:10:09 $
Version:   $Revision: 1.18 $

=========================================================================auto=*/
//
//
// .NAME vtkMRMLScene - A set of MRML Nodes that supports serialization and undo/redo
// .SECTION Description
// vtkMRMLScene represents and provides methods to manipulate a list of
// MRML objects. The list is core and duplicate entries are not prevented.
//
// .SECTION see also
// vtkMRMLNode vtkCollection 

#ifndef __vtkMRMLScene_h
#define __vtkMRMLScene_h

#include <list>
#include <map>
#include <vector>
#include <string>
#include <vtksys/SystemTools.hxx> 
#include <vtksys/hash_map.hxx> 

#include "vtkCollection.h"
#include "vtkObjectFactory.h"

#include "vtkMRML.h"
#include "vtkMRMLNode.h"

class vtkGeneralTransform;

class VTK_MRML_EXPORT vtkMRMLScene : public vtkCollection
{
public:
  static vtkMRMLScene *New();
  vtkTypeMacro(vtkMRMLScene,vtkCollection);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set URL (file name) of the scene
  void SetURL(const char *url) {
    this->URL = vtksys_stl::string(url); 
  };
  
  // Description:
  // Get URL (file name) of the scene  
  const char *GetURL() {
    return this->URL.c_str();
  };
  
  // Description:
  // Set Root directory, where URL is pointing
  void SetRootDirectory(const char *dir) {
    this->RootDirectory = vtksys_stl::string(dir); 
  };
  
  // Description:
  // Get Root directory, where URL is pointing  
  const char *GetRootDirectory() {
    return this->RootDirectory.c_str();
  };
  
  // Description:
  // Create new scene from URL
  int Connect();
  
  // Description:
  // Add the scene from URL
  int Import();

  // Description:
  // Save scene into URL
  int Commit(const char* url=NULL);

  // Description:
  // Remove nodes and clear undo/redo stacks
  void Clear(int removeSingletons);

  // Description:
  // Reset all nodes to their constructor's state
  void ResetNodes();
  
  // Description:
  // Create node with a given class
  vtkMRMLNode* CreateNodeByClass(const char* className);

  // Description:
  // Register node class with the Scene so that it can create it from
  // a class name
  // -- this maintains a registered pointer to the node, so users should Delete()
  //    the node after calling this.  The node is Deleted when the scene is destroyed.
  void RegisterNodeClass(vtkMRMLNode* node);
  
  // Description:
  // Add a path to the list.
  const char* GetClassNameByTag(const char *tagName);

  // Description:
  // Add a path to the list.
  const char* GetTagByClassName(const char *className);

  // Description:
  // Called by another class to request that the node's id be set to the given
  // string
  // If the id is not in use, set it, otherwise, useit as a base for a unique
  // id and then set it
  void RequestNodeID(vtkMRMLNode *node, const char *ID);
  
  // Description:
  // Add a node to the scene and send NewNode and SceneModified events.
  vtkMRMLNode* AddNode(vtkMRMLNode *n);

  // Description:
  // Add a copy of a node to the scene.
  vtkMRMLNode* CopyNode(vtkMRMLNode *n);

  // Description:
  // Add a node to the scene.
  vtkMRMLNode* AddNodeNoNotify(vtkMRMLNode *n);
  
  // Description:
  // Remove a path from the list.
  void RemoveNode(vtkMRMLNode *n); 

  // Description:
  // Remove a path from the list without invoking NodeRemovedEvent
  // - use this when there are no references to the passed node (e.g. singletons 
  //   on scene load)
  void RemoveNodeNoNotify(vtkMRMLNode *n); 
  
  // Description:
  // Determine whether a particular node is present. Returns its position
  // in the list.
  int IsNodePresent(vtkMRMLNode *n) {
    return this->CurrentScene->vtkCollection::IsItemPresent((vtkObject *)n);};

  // Description:
  // Initialize a traversal (not reentrant!)
  void InitTraversal() { 
    if (this && this->CurrentScene) 
      {
      this->CurrentScene->InitTraversal(); 
      }
  };
  
  // Description:
  // Get next node in the scene.
  vtkMRMLNode *GetNextNode() {
    return (vtkMRMLNode *)(this->CurrentScene->GetNextItemAsObject());};
  
  // Description:
  // Get next node of the class in the scene.
  vtkMRMLNode *GetNextNodeByClass(const char* className);

  // Description:
  // Get nodes having the specified name
  vtkCollection *GetNodesByName(const char* name);

  // Description:
  // Get node given a unique ID
  vtkMRMLNode *GetNodeByID(const char* name);
  //BTX
  vtkMRMLNode *GetNodeByID(std::string name);
  //ETX
  
  // Description:
  // Get nodes of a specified class having the specified name
  vtkCollection *GetNodesByClassByName(const char* className, const char* name);
  
  // Description:
  // Get number of nodes in the scene
  int GetNumberOfNodes () { return this->CurrentScene->GetNumberOfItems(); };

  // Description:
  // Get n-th node in the scene
  vtkMRMLNode* GetNthNode(int n);
  
  // Description:
  // Get n-th node of a specified class  in the scene 
  vtkMRMLNode* GetNthNodeByClass(int n, const char* className );
  
  // Description:
  // Get number of nodes of a specified class in the scene
  int GetNumberOfNodesByClass(const char* className);
  
  // Description:
  // Get vector of nodes of a specified class in the scene
  //BTX
  int GetNodesByClass(const char *className, std::vector<vtkMRMLNode *> &nodes);
  //ETX
  
  //BTX
  std::list<std::string> GetNodeClassesList();
  //ETX
  
  // Description:
  // returns list of names
  const char* GetNodeClasses();
  
  const char* GetUniqueNameByString(const char* className);
  // Description:
  // Explore the MRML tree to find the next unique index for use as an ID,
  // starting from 1
  int GetUniqueIDIndexByClass(const char* className);
  // Description:
  // Explore the MRML tree to find the next unique index for use as an ID,
  // starting from hint
  int GetUniqueIDIndexByClassFromIndex(const char* className, int hint);
  
  // Description:
  // insert a node in the scene after a specified node
  void InsertAfterNode( vtkMRMLNode *item, vtkMRMLNode *newItem);
  // Description:
  // insert a node in the scene before a specified node
  void InsertBeforeNode( vtkMRMLNode *item, vtkMRMLNode *newItem);
  
  // Description:
  // Ger transformation between two nodes
  int GetTransformBetweenNodes( vtkMRMLNode *node1, vtkMRMLNode *node2, 
                                vtkGeneralTransform *xform );

  // Description:
  // Set undo on/off
  void SetUndoOn() {UndoFlag=true;};
  void SetUndoOff() {UndoFlag=false;};
  bool GetUndoFlag() {return UndoFlag;};
  void SetUndoFlag(bool flag) {UndoFlag = flag;};

  // Description:
  // undo, set the scene to previous state
  void Undo();
  
  // Description:
  // redo, set the scene to previously undone
  void Redo();

  // Description:
  // clear Undo stack, delete undo history
  void ClearUndoStack();
  
  // Description:
  // clear Redo stack, delete redo history
  void ClearRedoStack();

  // Description: 
  // returns number of undo steps in the history buffer
  int GetNumberOfUndoLevels() { return this->UndoStack.size();};
  
  // Description: 
  // returns number of redo steps in the history buffer
  int GetNumberOfRedoLevels() { return this->RedoStack.size();};

  // Description: 
  // Save current state in the undo buffer
  void SaveStateForUndo();
  // Description: 
  // Save current state of the node in the undo buffer
  void SaveStateForUndo(vtkMRMLNode *node);
  // Description: 
  // Save current state of the nodes in the undo buffer
  void SaveStateForUndo(vtkCollection *nodes);
  //BTX
  void SaveStateForUndo(std::vector<vtkMRMLNode *> nodes);
  //ETX

  void AddReferencedNodeID(const char *id, vtkMRMLNode *refrencingNode)
  {
    if (id && refrencingNode && refrencingNode->GetScene() && refrencingNode->GetID()) 
      {
      this->ReferencedIDs.push_back(id);
      this->ReferencingNodes.push_back(refrencingNode);
      }
  };

  void ClearReferencedNodeID()
  {
    this->ReferencedIDs.clear();
    this->ReferencingNodes.clear();
    this->ReferencedIDChanges.clear();
  };

  void RemoveReferencedNodeID(const char *id, vtkMRMLNode *refrencingNode);

  void RemoveNodeReferences(vtkMRMLNode *node);

  void RemoveReferencesToNode(vtkMRMLNode *node);

  // Description: 
  // Return collection of all nodes referenced directly or indirectly by a node.
  vtkCollection* GetReferencedNodes(vtkMRMLNode *node);

//BTX
  // Description:
  // Get/Set the active Scene 
  static void SetActiveScene(vtkMRMLScene *);
  static vtkMRMLScene *GetActiveScene();
//ETX

  //BTX
  enum
    {
      NodeAddedEvent = 66000,
      NodeRemovedEvent = 66001,
      NewSceneEvent = 66002,
      SceneCloseEvent = 66003,
      SceneClosingEvent = 66004,
    };
//ETX

  int IsFilePathRelative(const char * filepath);

  vtkSetMacro(ErrorCode,unsigned long);
  vtkGetMacro(ErrorCode,unsigned long);


//BTX
  void SetErrorMessage(const std::string &error) {
    this->ErrorMessage = error;
  };

  std::string GetErrorMessage() {
    return this->ErrorMessage;
  };
//ETX

  void SetErrorMessage(const char * message)
    {
    this->SetErrorMessage(std::string(message));
    }

  const char *GetErrorMessagePointer()
    {
    return (this->GetErrorMessage().c_str());
    }

  unsigned long GetSceneModifiedTime()
    {
    if (this->CurrentScene && this->CurrentScene->GetMTime() > this->SceneModifiedTime)
      {
      this->SceneModifiedTime = this->CurrentScene->GetMTime();
      }
    return this->SceneModifiedTime;
    };
    
  void IncrementSceneModifiedTime()
    {
    this->SceneModifiedTime ++;
    };

protected:
  vtkMRMLScene();
  ~vtkMRMLScene();
  vtkMRMLScene(const vtkMRMLScene&);
  void operator=(const vtkMRMLScene&);
  
  void PushIntoUndoStack();
  void PushIntoRedoStack();

  void CopyNodeInUndoStack(vtkMRMLNode *node);
  void CopyNodeInRedoStack(vtkMRMLNode *node);

  void AddReferencedNodes(vtkMRMLNode *node, vtkCollection *refNodes);

  vtkCollection* CurrentScene;
  
  unsigned long SceneModifiedTime;
  
  int UndoStackSize;
  bool UndoFlag;
  
  bool InUndo;

  //BTX
  std::list< vtkCollection* >  UndoStack;
  std::list< vtkCollection* >  RedoStack;
  //ETX
  

  //BTX
  vtksys_stl::string         URL;
  std::map< std::string, int> UniqueIDByClass;
  std::vector< std::string >  UniqueIDs;
  std::vector< vtkMRMLNode* > RegisteredNodeClasses;
  std::vector< std::string >  RegisteredNodeTags;
  vtksys_stl::string          RootDirectory;

  std::vector< std::string > ReferencedIDs;
  std::vector< vtkMRMLNode* > ReferencingNodes;
  std::map< std::string, std::string> ReferencedIDChanges;
  
  //vtksys::hash_map<const char*, vtkMRMLNode*> NodeIDs;
  std::map<std::string, vtkMRMLNode*> NodeIDs;

  std::string ErrorMessage;
  //ETX
  
  void UpdateNodeReferences();
  void UpdateNodeReferences(vtkCollection* chekNodes);

  void UpdateNodeIDs();

  unsigned long NodeIDsMTime;

  void RemoveAllNodesExceptSingletons();

  vtkSetStringMacro(ClassNameList);
  vtkGetStringMacro(ClassNameList);
  
private:
  // hide the standard AddItem from the user and the compiler.
  void AddItem(vtkObject *o) { this->CurrentScene->vtkCollection::AddItem(o); this->Modified();};
  void RemoveItem(vtkObject *o) { this->CurrentScene->vtkCollection::RemoveItem(o); this->Modified();};
  void RemoveItem(int i) { this->CurrentScene->vtkCollection::RemoveItem(i); this->Modified();};
  int  IsItemPresent(vtkObject *o) { return this->CurrentScene->vtkCollection::IsItemPresent(o);};
  
  int LoadIntoScene(vtkCollection* scene);

  unsigned long ErrorCode;

  char* ClassNameList;

  static vtkMRMLScene *ActiveScene;
};

#endif
