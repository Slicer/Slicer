/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLScene.cxx,v $
Date:      $Date: 2006/03/17 15:10:09 $
Version:   $Revision: 1.18 $

=========================================================================auto=*/
#include <sstream>
#include <map>

//#include <hash_map>
#include "vtkMRMLScene.h"
#include "vtkMRMLParser.h"
#include "vtkObjectFactory.h"
#include "vtkErrorCode.h"

//------------------------------------------------------------------------------
vtkMRMLScene::vtkMRMLScene() 
{
  this->URL = NULL;
  this->ClassNameList = NULL;
  this->RegisteredNodeClasses.clear();
  this->UniqueIDByClass.clear();

  this->CurrentScene =  vtkCollection::New();
  this->UndoStackSize = 100;
  this->UndoFlag = true;
}

//------------------------------------------------------------------------------
vtkMRMLScene::~vtkMRMLScene() 
{
  if (this->URL) {
    delete this->URL;
  }
  if (this->ClassNameList) {
    delete this->ClassNameList;
  }
  this->CurrentScene->Delete();
}

//------------------------------------------------------------------------------
vtkMRMLScene* vtkMRMLScene::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLScene");
  if(ret) {
    return (vtkMRMLScene*)ret;
  }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLScene;
}

//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::CreateNodeByClass(const char* className) 
{
  vtkMRMLNode* node = NULL;
  for (unsigned int i=0; i<RegisteredNodeClasses.size(); i++) {
    if (!strcmp(RegisteredNodeClasses[i]->GetClassName(), className)) {
      node = RegisteredNodeClasses[i]->CreateNodeInstance();
      break;
    }
  }
  // non-registered nodes can have a registered factory
  if (node == NULL) {
    vtkObject* ret = vtkObjectFactory::CreateInstance(className); 
    if(ret) {
      node = static_cast<vtkMRMLNode *>(ret);
    }
  }
  return node;
}

//------------------------------------------------------------------------------
void vtkMRMLScene::RegisterNodeClass(vtkMRMLNode* node) 
{
  this->RegisteredNodeClasses.push_back(node);
  this->RegisteredNodeTags.push_back(std::string(node->GetNodeTagName()));
}

//------------------------------------------------------------------------------
const char* vtkMRMLScene::GetClassNameByTag(const char *tagName)
{
  for (unsigned int i=0; i<RegisteredNodeTags.size(); i++) {
    if (!strcmp(RegisteredNodeTags[i].c_str(), tagName)) {
      return (RegisteredNodeClasses[i])->GetClassName();
    }
  }
  return NULL;
}

//------------------------------------------------------------------------------
int vtkMRMLScene::Connect()
{
  if (this->URL == NULL) {
    vtkErrorMacro("Need URL specified");
    return 0;
  }
  this->RootDirectory = vtksys::SystemTools::GetParentDirectory(this->GetURL());   
  this->RootDirectory = this->RootDirectory + vtksys_stl::string("/");

  bool undoFlag = this->GetUndoFlag();
  this->SetUndoOff();

  this->CurrentScene->RemoveAllItems();
  vtkMRMLParser* parser = vtkMRMLParser::New();
  parser->SetMRMLScene(this);
  parser->SetFileName(URL);
  parser->Parse();
  parser->Delete();

  // create node references
  int nnodes = this->CurrentScene->GetNumberOfItems();
  vtkMRMLNode *node = NULL;
  for (int n=0; n<nnodes; n++) {
    node = (vtkMRMLNode *)this->CurrentScene->GetItemAsObject(n);
    node->UpdateScene(this);
  }

  this->SetUndoFlag(undoFlag);

  return 1;
}

//------------------------------------------------------------------------------
int vtkMRMLScene::Commit(const char* url)
{
  if (url == NULL) {
    url = URL;
  }

  vtkMRMLNode *node;
  ofstream file;
  int indent=0, deltaIndent;
  
  // Open file
  file.open(url);
  if (file.fail()) {
    vtkErrorMacro("Write: Could not open file " << url);
    cerr << "Write: Could not open file " << url;
#if (VTK_MAJOR_VERSION <= 5)      
    this->SetErrorCode(2);
#else
    this->SetErrorCode(vtkErrorCode::GetErrorCodeFromString("CannotOpenFileError"));
#endif
    return 1;
  }
  
  file << "<?xml version=\"1.0\" standalone='no'?>\n";
  file << "<!DOCTYPE MRML SYSTEM \"mrml20.dtd\">\n";
  file << "<MRML>\n";
   
  // Write each node
  for (int n=0; n < this->CurrentScene->GetNumberOfItems(); n++) {
    node = (vtkMRMLNode*)this->CurrentScene->GetItemAsObject(n);
    
    deltaIndent = node->GetIndent();
    if ( deltaIndent < 0 ) {
      indent -=2;
    }
    
    vtkIndent vindent(indent);
    file << vindent << "<" << node->GetNodeTagName() << "\n";

    node->WriteXML(file, indent);
    
    file << vindent << "></" << node->GetNodeTagName() << ">\n";
    
    if ( deltaIndent > 0 ) {
      indent += 2;
    }    
  }
  
  file << "</MRML>\n";
  
  // Close file
  file.close();
#if (VTK_MAJOR_VERSION <= 5)      
  this->SetErrorCode(0);
#else
  this->SetErrorCode(vtkErrorCode::GetErrorCodeFromString("NoError"));
#endif
  return 1;
}

void vtkMRMLScene::AddNode(vtkMRMLNode *n)
{
  //TODO convert URL to Root directory
  //n->SetSceneRootDir("");

  n->SetSceneRootDir(this->RootDirectory.c_str());

  this->CurrentScene->vtkCollection::AddItem((vtkObject *)n);
}

//------------------------------------------------------------------------------
int vtkMRMLScene::GetNumberOfNodesByClass(const char *className)
{
  int num=0;
  vtkMRMLNode *node;
  for (int n=0; n < this->CurrentScene->GetNumberOfItems(); n++) {
    node = (vtkMRMLNode*)this->CurrentScene->GetItemAsObject(n);
    if (!strcmp(node->GetClassName(), className)) {
      num++;
    }
  }
  return num;
}

//------------------------------------------------------------------------------
std::list< std::string > vtkMRMLScene::GetNodeClassesList()
{
  std::list< std::string > classes;

  vtkMRMLNode *node;
  for (int n=0; n < this->CurrentScene->GetNumberOfItems(); n++) {
    node = (vtkMRMLNode*)this->CurrentScene->GetItemAsObject(n);
    classes.push_back(node->GetClassName());
  }
  classes.unique();
  return classes;
  
}

//------------------------------------------------------------------------------
const char* vtkMRMLScene::GetNodeClasses()
{
  std::list< std::string > classes = this->GetNodeClassesList();
  std::string classList;

  std::list< std::string >::const_iterator iter;
  // Iterate through list and output each element.
  for (iter = classes.begin(); iter != classes.end(); iter++) {
    if (!(iter == classes.begin())) {
      classList += std::string(" ");
    }
    classList.append(*iter);
  }
  SetClassNameList(classList.c_str());
  return this->ClassNameList;
}



//------------------------------------------------------------------------------
vtkMRMLNode *vtkMRMLScene::GetNextNodeByClass(const char *className)
{
  vtkMRMLNode *node = (vtkMRMLNode*)this->CurrentScene->GetNextItemAsObject();

  while (node != NULL && strcmp(node->GetClassName(), className)) {
    node = (vtkMRMLNode*)this->CurrentScene->GetNextItemAsObject();
  }
  return node;
}


//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::GetNthNode(int n)
{

  if(n < 0 || n >= this->CurrentScene->GetNumberOfItems()) {
    return NULL;
  }
  else {
    return (vtkMRMLNode*)this->CurrentScene->GetItemAsObject(n);
  }
}

//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::GetNthNodeByClass(int n, const char *className)
{
  int num=0;
  vtkMRMLNode *node;
  for (int nn=0; nn < this->CurrentScene->GetNumberOfItems(); nn++) {
    node = (vtkMRMLNode*)this->CurrentScene->GetItemAsObject(nn);
    if (!strcmp(node->GetClassName(), className)) {
      if (num == n) {
        return node;
      }
      num++;
    }
  }
  return NULL;
}

//------------------------------------------------------------------------------
vtkCollection* vtkMRMLScene::GetNodesByName(const char* name)
{

  vtkCollection* nodes = vtkCollection::New();

  vtkMRMLNode *node;
  for (int n=0; n < this->CurrentScene->GetNumberOfItems(); n++) {
    node = (vtkMRMLNode*)this->CurrentScene->GetItemAsObject(n);
    if (!strcmp(node->GetName(), name)) {
      nodes->AddItem(node);
    }
  }
  
  return nodes;
}


//------------------------------------------------------------------------------
vtkCollection* vtkMRMLScene::GetNodesByID(const char* id)
{

  vtkCollection* nodes = vtkCollection::New();

  vtkMRMLNode *node;
  for (int n=0; n < this->CurrentScene->GetNumberOfItems(); n++) {
    node = (vtkMRMLNode*)this->CurrentScene->GetItemAsObject(n);
    if (node->GetID() && !strcmp(node->GetID(), id)) {
      nodes->AddItem(node);
    }
  }
  
  return nodes;
}

//------------------------------------------------------------------------------
vtkCollection* vtkMRMLScene::GetNodesByClassByID(const char* className, const char* id)
{
  vtkCollection* nodes = vtkCollection::New();
  
  vtkMRMLNode *node;
  for (int n=0; n < this->CurrentScene->GetNumberOfItems(); n++) {
    node = (vtkMRMLNode*)this->CurrentScene->GetItemAsObject(n);
    if (node->GetID() && !strcmp(node->GetID(), id) && strcmp(node->GetClassName(), className) == 0) {
      nodes->AddItem(node);
    }
  }

  return nodes;
}

//------------------------------------------------------------------------------
vtkCollection* vtkMRMLScene::GetNodesByClassByName(const char* className, const char* name)
{
  vtkCollection* nodes = vtkCollection::New();
  
  vtkMRMLNode *node;
  for (int n=0; n < this->CurrentScene->GetNumberOfItems(); n++) {
    node = (vtkMRMLNode*)this->CurrentScene->GetItemAsObject(n);
    if (!strcmp(node->GetName(), name) && strcmp(node->GetClassName(), className) == 0) {
      nodes->AddItem(node);
    }
  }

  return nodes;
}

//------------------------------------------------------------------------------
int  vtkMRMLScene::GetTransformBetweenNodes(vtkMRMLNode *node1,
                                            vtkMRMLNode *node2, 
                                            vtkTransform *xform)
{
  vtkErrorMacro("NOT IMPLEMENTEED YET");
  return 1;
}

//------------------------------------------------------------------------------
int vtkMRMLScene::GetTransformBetweenSpaces( const char *space1, 
                                             const char *space2, 
                                             vtkTransform *xform )
{
  vtkErrorMacro("NOT IMPLEMENTEED YET");
  return 1;
}


//------------------------------------------------------------------------------
void vtkMRMLScene::InsertAfterNode(vtkMRMLNode *item, vtkMRMLNode *n)
{
  vtkErrorMacro("NOT IMPLEMENTEED YET");
}

//------------------------------------------------------------------------------
void vtkMRMLScene::InsertBeforeNode(vtkMRMLNode *item, vtkMRMLNode *n)
{
  vtkErrorMacro("NOT IMPLEMENTEED YET");
}

//------------------------------------------------------------------------------
void vtkMRMLScene::PrintSelf(ostream& os, vtkIndent indent)
{
  this->CurrentScene->vtkCollection::PrintSelf(os,indent);
  std::list<std::string> classes = this->GetNodeClassesList();

  std::list< std::string >::const_iterator iter;
  // Iterate through list and output each element.
  for (iter = classes.begin(); iter != classes.end(); iter++) {
    std::string className = (*iter);
    os << indent << "Number Of Nodes for class " << className.c_str() << " : " << this->GetNumberOfNodesByClass(className.c_str()) << "\n";
  }
}

//------------------------------------------------------------------------------
const char* vtkMRMLScene::GetUniqueIDByClass(const char* className)
{
  std::string sname(className);
  if (UniqueIDByClass.find(sname) == UniqueIDByClass.end() ) {
    UniqueIDByClass[className] = 1;
  }
  int id = UniqueIDByClass[sname];

  std::string name;

  while (true) {
    std::stringstream ss;
    ss << className;
    ss << id;
    name = ss.str();
    bool nameExists = false;
    for (int i=0; i< UniqueIDs.size(); i++) {
      if (UniqueIDs[i] == name) {
        nameExists = true;
        break;
      }
    }
    if (nameExists) {
      id++;
      continue;
    }
    else {
      break;
    }
  }
  UniqueIDByClass[className] = id + 1;
  UniqueIDs.push_back(name);
  return UniqueIDs[UniqueIDs.size()-1].c_str();
}

//------------------------------------------------------------------------------
void vtkMRMLScene::SaveStateForUndo (vtkMRMLNode *node)
{
  this->SetUndoOn();
  this->PushIntoUndoStack();
  vtkMRMLNode *snode = node->CreateNodeInstance();
  if (snode != NULL) {
    snode->Copy(node);
    this->ReplaceNodeInUndoStack(node, snode);
  }
} 

//------------------------------------------------------------------------------
void vtkMRMLScene::SaveStateForUndo (std::vector<vtkMRMLNode *> nodes)
{
  this->SetUndoOn();
  this->PushIntoUndoStack();
  for (int n=0; n<nodes.size(); n++) {
    vtkMRMLNode *node = nodes[n];
    vtkMRMLNode *snode = node->CreateNodeInstance();
    if (snode != NULL) {
      snode->Copy(node);
      this->ReplaceNodeInUndoStack(node, snode);
    }
  }
} 

//------------------------------------------------------------------------------
void vtkMRMLScene::SaveStateForUndo (vtkCollection* nodes)
{
  this->SetUndoOn();
  this->PushIntoUndoStack();

  int nnodes = nodes->GetNumberOfItems();
  
  for (int n=0; n<nnodes; n++) {
    vtkMRMLNode *node  = dynamic_cast < vtkMRMLNode *>(nodes->GetItemAsObject(n));
    if (node) {
      vtkMRMLNode *snode = node->CreateNodeInstance();
      if (snode != NULL) {
        snode->Copy(node);
        this->ReplaceNodeInUndoStack(node, snode);
      }
    }
  }
} 

//------------------------------------------------------------------------------
void vtkMRMLScene::PushIntoUndoStack()
{
  if (this->CurrentScene == NULL) {
    return;
  }

  vtkCollection* newScene = vtkCollection::New();

  vtkCollection* currentScene = this->CurrentScene;
  
  int nnodes = currentScene->GetNumberOfItems();

  for (int n=0; n<nnodes; n++) {
    vtkMRMLNode *node  = dynamic_cast < vtkMRMLNode *>(currentScene->GetItemAsObject(n));
    if (node) {
      newScene->AddItem(node);
    }
  }

  //TODO check max stack size
  this->UndoStack.push_back(newScene);
}

//------------------------------------------------------------------------------
void vtkMRMLScene::ReplaceNodeInUndoStack(vtkMRMLNode *replaceNode, vtkMRMLNode *withNode)
{
  vtkCollection* undoScene = dynamic_cast < vtkCollection *>( this->UndoStack.back() );;
  int nnodes = undoScene->GetNumberOfItems();
  for (int n=0; n<nnodes; n++) {
    vtkMRMLNode *node  = dynamic_cast < vtkMRMLNode *>(undoScene->GetItemAsObject(n));
    if (node == replaceNode) {
      undoScene->ReplaceItem (n, withNode);
    }
  }
}

//------------------------------------------------------------------------------
void vtkMRMLScene::Undo()
{
  if (this->UndoStack.size() == 0) {
    return;
  }

  int nnodes;
  int n;

  vtkCollection* currentScene = this->CurrentScene;
  //std::hash_map<std::string, vtkMRMLNode*> currentMap;
  std::map<std::string, vtkMRMLNode*> currentMap;
  nnodes = currentScene->GetNumberOfItems();
  for (n=0; n<nnodes; n++) {
    vtkMRMLNode *node  = dynamic_cast < vtkMRMLNode *>(currentScene->GetItemAsObject(n));
    if (node) {
      currentMap[node->GetID()] = node;
    }
  }

  vtkCollection* undoScene = dynamic_cast < vtkCollection *>( this->UndoStack.back() );;
  //std::hash_map<std::string, vtkMRMLNode*> undoMap;
  std::map<std::string, vtkMRMLNode*> undoMap;
  nnodes = undoScene->GetNumberOfItems();
  for (n=0; n<nnodes; n++) {
    vtkMRMLNode *node  = dynamic_cast < vtkMRMLNode *>(undoScene->GetItemAsObject(n));
    if (node) {
      undoMap[node->GetID()] = node;
    }
  }

  //std::hash_map<std::string, vtkMRMLNode*>::iterator iter;
  //std::hash_map<std::string, vtkMRMLNode*>::iterator curIter;
  std::map<std::string, vtkMRMLNode*>::iterator iter;
  std::map<std::string, vtkMRMLNode*>::iterator curIter;

  // copy back changes and add deleted nodes to the current scene
  std::vector<vtkMRMLNode*> addNodes;

  for(iter=undoMap.begin(); iter != undoMap.end(); iter++) {
    curIter = currentMap.find(iter->first);
    if ( curIter == currentMap.end() ) {
      // the node was deleted, add Node back to the curreent scene
      addNodes.push_back(iter->second);
    }
    else if (iter->second != curIter->second) {
      // nodes differ, copy from undo to current scene
      curIter->second->Copy(iter->second);
    }
  }
  
  // remove new nodes created before Undo
  std::vector<vtkMRMLNode*> removeNodes;
  for(curIter=currentMap.begin(); curIter != currentMap.end(); curIter++) {
    iter = undoMap.find(curIter->first);
    if ( iter == undoMap.end() ) {
      removeNodes.push_back(curIter->second);
    }
  }

  for (n=0; n<addNodes.size(); n++) {
    this->AddNode(addNodes[n]);
  }
  for (n=0; n<removeNodes.size(); n++) {
    this->RemoveNode(removeNodes[n]);
  }
  undoScene->RemoveAllItems();
  undoScene->Delete();
  UndoStack.pop_back();
}

