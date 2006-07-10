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

#include "vtkCollection.h"
#include "vtkObjectFactory.h"

#include "vtkMRML.h"
#include "vtkMRMLNode.h"

class vtkGeneralTransform;

class VTK_MRML_EXPORT vtkMRMLScene : public vtkObject
{
public:
  static vtkMRMLScene *New();
  vtkTypeMacro(vtkMRMLScene,vtkCollection);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Filename should be flie://path/file.xml
  vtkSetStringMacro(URL);
  vtkGetStringMacro(URL);
  
  int Connect();
  
  int Commit(const char* url=NULL);

  // Create node with a given class
  vtkMRMLNode* CreateNodeByClass(const char* className);

  // Register node class with the Scene so that it can create it from
  // a class name
  void RegisterNodeClass(vtkMRMLNode* node);
  
  // Description:
  // Add a path to the list.
  const char* GetClassNameByTag(const char *tagName);

  // Description:
  // Add a path to the list.
  const char* GetTagByClassName(const char *className);

  // Description:
  // Add a path to the list.
  void AddNode(vtkMRMLNode *n);
  
  // Description:
  // Remove a path from the list.
  void RemoveNode(vtkMRMLNode *n) {
    n->Register(this);
    this->CurrentScene->vtkCollection::RemoveItem((vtkObject *)n);
    this->InvokeEvent(this->NodeRemovedEvent, n);
    n->UnRegister(this);

    this->Modified();
  };
  
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
  // Get the next path in the list.
  vtkMRMLNode *GetNextNode() {
    return (vtkMRMLNode *)(this->CurrentScene->GetNextItemAsObject());};
  
  vtkMRMLNode *GetNextNodeByClass(const char* className);
  
  vtkCollection *GetNodesByName(const char* name);

  vtkMRMLNode *GetNodeByID(const char* name);

  vtkMRMLNode* GetNodeByClassByID(const char* className, const char* id);
  
  vtkCollection *GetNodesByClassByName(const char* className, const char* name);
  
  int GetNumberOfNodes () { return this->CurrentScene->GetNumberOfItems(); };

  vtkMRMLNode* GetNthNode(int n);
  
  vtkMRMLNode* GetNthNodeByClass(int n, const char* className );
  
  int GetNumberOfNodesByClass(const char* className);
  
  //BTX
  std::list<std::string> GetNodeClassesList();
  //ETX
  
  // returns list of names
  const char* GetNodeClasses();
  
  const char* GetUniqueIDByClass(const char* className);
  
  void InsertAfterNode( vtkMRMLNode *item, vtkMRMLNode *newItem);
  void InsertBeforeNode( vtkMRMLNode *item, vtkMRMLNode *newItem);
  
  int GetTransformBetweenNodes( vtkMRMLNode *node1, vtkMRMLNode *node2, 
                                vtkGeneralTransform *xform );

  // Undo API
  void SetUndoOn() {UndoFlag=true;};
  void SetUndoOff() {UndoFlag=false;};
  bool GetUndoFlag() {return UndoFlag;};
  void SetUndoFlag(bool flag) {UndoFlag = flag;};

  void Undo();
  void Redo();

  void ClearUndoStack();
  void ClearRedoStack();

  int GetNumberOfUndoLevels() { return this->UndoStack.size();};
  int GetNumberOfRedoLevels() { return this->RedoStack.size();};

  void SaveStateForUndo();
  void SaveStateForUndo(vtkMRMLNode *node);
  void SaveStateForUndo(vtkCollection *nodes);
  //BTX
  void SaveStateForUndo(std::vector<vtkMRMLNode *> nodes);
  //ETX

//BTX
  // Description:
  // Get/Set the active Scene - to be used by MRMLIDImageIO
  static void SetActiveScene(vtkMRMLScene *);
  static vtkMRMLScene *GetActiveScene();
//ETX

  //BTX
  enum
    {
      NodeAddedEvent = 66000,
      NodeRemovedEvent = 66001,
      NodeModifiedEvent = 66002,
    };
//ETX

protected:
  vtkMRMLScene();
  ~vtkMRMLScene();
  vtkMRMLScene(const vtkMRMLScene&);
  void operator=(const vtkMRMLScene&);
  

  void PushIntoUndoStack();
  void PushIntoRedoStack();

  void CopyNodeInUndoStack(vtkMRMLNode *node);
  void CopyNodeInRedoStack(vtkMRMLNode *node);
  
  vtkCollection* CurrentScene;
  int UndoStackSize;
  bool UndoFlag;
  //BTX
  std::list< vtkCollection* >  UndoStack;
  std::list< vtkCollection* >  RedoStack;
  //ETX
  
  char *URL;

  //BTX
  std::map< std::string, int> UniqueIDByClass;
  std::vector< std::string >  UniqueIDs;
  std::vector< vtkMRMLNode* > RegisteredNodeClasses;
  std::vector< std::string >  RegisteredNodeTags;
  vtksys_stl::string          RootDirectory;
  //ETX
  
  vtkSetMacro(ErrorCode,unsigned long);
  vtkGetMacro(ErrorCode,unsigned long);

  vtkSetStringMacro(ClassNameList);
  vtkGetStringMacro(ClassNameList);
  
private:
  // hide the standard AddItem from the user and the compiler.
  void AddItem(vtkObject *o) { this->CurrentScene->vtkCollection::AddItem(o); this->Modified();};
  void RemoveItem(vtkObject *o) { this->CurrentScene->vtkCollection::RemoveItem(o); this->Modified();};
  void RemoveItem(int i) { this->CurrentScene->vtkCollection::RemoveItem(i); this->Modified();};
  int  IsItemPresent(vtkObject *o) { return this->CurrentScene->vtkCollection::IsItemPresent(o);};
  
  unsigned long ErrorCode;

  char* ClassNameList;

  static vtkMRMLScene *ActiveScene;
};

#endif
