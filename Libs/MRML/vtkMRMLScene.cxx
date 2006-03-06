/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLScene.cxx,v $
Date:      $Date: 2006/03/03 22:26:40 $
Version:   $Revision: 1.11 $

=========================================================================auto=*/
#include <sstream>

#include <vtksys/SystemTools.hxx> 

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
void vtkMRMLScene::AddNode(vtkMRMLNode *n) 
{
  //TODO convert URL to Root directory
  vtksys_stl::string root = vtksys::SystemTools::GetParentDirectory(this->GetURL());   
  n->SetSceneRootDir(root.c_str());
  this->vtkCollection::AddItem((vtkObject *)n);
}

//------------------------------------------------------------------------------
int vtkMRMLScene::Connect()
{
  if (this->URL == NULL) {
    vtkErrorMacro("Need URL specified");
    return 0;
  }
  this->RemoveAllItems();
  vtkMRMLParser* parser = vtkMRMLParser::New();
  parser->SetMRMLScene(this);
  parser->SetFileName(URL);
  parser->Parse();
  parser->Delete();

  // create node references
  int nnodes = this->GetNumberOfItems();
  vtkMRMLNode *node = NULL;
  for (int n=0; n<nnodes; n++) {
    node = (vtkMRMLNode *)this->GetItemAsObject(n);
    node->UpdateScene(this);
  }
  return 1;
}

//------------------------------------------------------------------------------
int vtkMRMLScene::Commit(const char* url)
{
  if (url == NULL) {
    url = URL;
  }

  vtkCollectionElement *elem = this->Top;
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
  while (elem != NULL) {
    node = (vtkMRMLNode*)elem->Item;
    
    deltaIndent = node->GetIndent();
    if ( deltaIndent < 0 ) {
      indent -=2;
    }
    
    node->WriteXML(file, indent);
    
    if ( deltaIndent > 0 ) {
      indent += 2;
    }
    
    elem = elem->Next;
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

//------------------------------------------------------------------------------
int vtkMRMLScene::GetNumberOfNodesByClass(const char *className)
{
  vtkCollectionElement *elem=this->Top;
  int num=0;
  
  while (elem != NULL) {
    if (!strcmp(elem->Item->GetClassName(), className)) {
      num++;
    }
    elem = elem->Next;
  }
  return num;
}

//------------------------------------------------------------------------------
std::list< std::string > vtkMRMLScene::GetNodeClassesList()
{
  std::list< std::string > classes;
  vtkCollectionElement *elem=this->Top;
  while (elem != NULL) {
    classes.push_back(elem->Item->GetClassName());
    elem = elem->Next;
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
  vtkCollectionElement *elem=this->Current;
  
  if (elem != NULL) {
    elem = elem->Next;
  }
  
  while (elem != NULL && strcmp(elem->Item->GetClassName(), className)) {
    elem = elem->Next;
  }
  
  if (elem != NULL) {
    this->Current = elem;
    return (vtkMRMLNode*)(elem->Item);
  }
  else {
    return NULL;
  }
}

//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::InitTraversalByClass(const char *className)
{
  vtkCollectionElement *elem=this->Top;
  
  while (elem != NULL && strcmp(elem->Item->GetClassName(), className)) {
    elem = elem->Next;
  }
  
  if (elem != NULL) {
    this->Current = elem;
    return (vtkMRMLNode*)(elem->Item);
  }
  else {
    return NULL;
  }
}

//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::GetNthNode(int n)
{
  vtkCollectionElement *elem;
  
  if(n < 0 || n >= this->NumberOfItems) {
    return NULL;
  }
  
  elem = this->Top;
  for (int j = 0; j < n; j++, elem = elem->Next) {}
  
  return (vtkMRMLNode*)(elem->Item);
}

//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::GetNthNodeByClass(int n, const char *className)
{
  vtkCollectionElement *elem;
  int j=0;
  
  elem = this->Top;
  while (elem != NULL) {
    if (strcmp(elem->Item->GetClassName(), className) == 0) {
      if (j == n) {
        return (vtkMRMLNode*)(elem->Item);
      }
      j++;
    }
    elem = elem->Next;
  }
  
  return NULL;
}

//------------------------------------------------------------------------------
vtkCollection* vtkMRMLScene::GetNodesByName(const char* name)
{
  vtkCollection* nodes = vtkCollection::New();
  
  vtkCollectionElement *elem=this->Top;
  vtkMRMLNode* node;
  while (elem != NULL) {
    node = (vtkMRMLNode*)elem->Item;
    if (!strcmp(node->GetName(), name)) {
      nodes->AddItem(node);
    }
    
    elem = elem->Next;
  }
  return nodes;
}


//------------------------------------------------------------------------------
vtkCollection* vtkMRMLScene::GetNodesByID(const char* id)
{
  vtkCollection* nodes = vtkCollection::New();
  
  vtkCollectionElement *elem=this->Top;
  vtkMRMLNode* node;
  while (elem != NULL) {
    node = (vtkMRMLNode*)elem->Item;
    if (node->GetID() && !strcmp(node->GetID(), id)) {
      nodes->AddItem(node);
    }
    
    elem = elem->Next;
  }
  return nodes;
}

//------------------------------------------------------------------------------
vtkCollection* vtkMRMLScene::GetNodesByClassByID(const char* className, const char* id)
{
  vtkCollection* nodes = vtkCollection::New();
  
  vtkCollectionElement *elem=this->Top;
  vtkMRMLNode* node;
  
  while (elem != NULL) {
    node = (vtkMRMLNode*)elem->Item;
    if (node->GetID() && !strcmp(node->GetID(), id) && strcmp(elem->Item->GetClassName(), className) == 0) {
      nodes->AddItem(node);
    }

    elem = elem->Next;
  }
  return nodes;
}

//------------------------------------------------------------------------------
vtkCollection* vtkMRMLScene::GetNodesByClassByName(const char* className, const char* name)
{
  vtkCollection* nodes = vtkCollection::New();
  
  vtkCollectionElement *elem=this->Top;
  vtkMRMLNode* node;
  
  while (elem != NULL) {
    node = (vtkMRMLNode*)elem->Item;
    if (!strcmp(node->GetName(), name) && strcmp(elem->Item->GetClassName(), className) == 0) {
      nodes->AddItem(node);
    }

    elem = elem->Next;
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
  int i;
  vtkCollectionElement *elem, *newElem;
  
  // Empty list
  if (!this->Top) {
    return;
  }
  
  newElem = new vtkCollectionElement;
  n->Register(this);
  newElem->Item = n;
  
  elem = this->Top;
  for (i = 0; i < this->NumberOfItems; i++) {
    if (elem->Item == item) {
      newElem->Next = elem->Next;
      elem->Next = newElem;
      
      if (this->Bottom == elem) {
        this->Bottom = newElem;
      }
      this->NumberOfItems++;
      return;
    }
    else {
      elem = elem->Next;
    }
  }
}

//------------------------------------------------------------------------------
void vtkMRMLScene::InsertBeforeNode(vtkMRMLNode *item, vtkMRMLNode *n)
{
  int i;
  vtkCollectionElement *elem, *newElem, *prev;
  
  // Empty list
  if (!this->Top) {
    return;
  }
  
  newElem = new vtkCollectionElement;
  n->Register(this);
  newElem->Item = n;
  
  
  elem = this->Top;
  
  // if insert before 1st elem
  if (elem->Item == item) {
    newElem->Next = this->Top;
    this->Top = newElem;
    this->NumberOfItems++;
    return;
  }

  for (i = 1; i < this->NumberOfItems; i++) {
    prev = elem;
    elem = elem->Next;

    if (elem->Item == item) {
      newElem->Next = prev->Next;
      prev->Next = newElem;
      this->NumberOfItems++;
      return;
    }
  }
}

//------------------------------------------------------------------------------
void vtkMRMLScene::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkCollection::PrintSelf(os,indent);
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
