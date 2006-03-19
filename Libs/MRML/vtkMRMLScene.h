/*=auto=========================================================================

(c) Copyright 2003 Massachusetts Institute of Technology (MIT) All Rights Reserved.

This software ("3D Slicer") is provided by The Brigham and Women's 
Hospital, Inc. on behalf of the copyright holders and contributors.
Permission is hereby granted, without payment, to copy, modify, display 
and distribute this software and its documentation, if any, for  
research purposes only, provided that (1) the above copyright notice and 
the following four paragraphs appear on all copies of this software, and 
(2) that source code to any modifications to this software be made 
publicly available under terms no more restrictive than those in this 
License Agreement. Use of this software constitutes acceptance of these 
terms and conditions.

3D Slicer Software has not been reviewed or approved by the Food and 
Drug Administration, and is for non-clinical, IRB-approved Research Use 
Only.  In no event shall data or images generated through the use of 3D 
Slicer Software be used in the provision of patient care.

IN NO EVENT SHALL THE COPYRIGHT HOLDERS AND CONTRIBUTORS BE LIABLE TO 
ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL 
DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, 
EVEN IF THE COPYRIGHT HOLDERS AND CONTRIBUTORS HAVE BEEN ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGE.

THE COPYRIGHT HOLDERS AND CONTRIBUTORS SPECIFICALLY DISCLAIM ANY EXPRESS 
OR IMPLIED WARRANTIES INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND 
NON-INFRINGEMENT.

THE SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS 
IS." THE COPYRIGHT HOLDERS AND CONTRIBUTORS HAVE NO OBLIGATION TO 
PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


=========================================================================auto=*/
// .NAME vtkMRMLScene - a list of actors
// .SECTION Description
// vtkMRMLScene represents and provides methods to manipulate a list of
// MRML objects. The list is core and duplicate
// entries are not prevented.
//
// .SECTION see also
// vtkMRMLNode vtkCollection 

#ifndef __vtkMRMLScene_h
#define __vtkMRMLScene_h

#include <list>
#include <map>
#include <vector>
#include <string>

#include "vtkCollection.h"
#include "vtkObjectFactory.h"

#include "vtkMRML.h"
#include "vtkMRMLNode.h"

class vtkTransform;

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
  
  const char* GetClassNameByTag(const char *tagName);

  // Description:
  // Add a path to the list.
  void AddNode(vtkMRMLNode *n);
  
  // Description:
  // Remove a path from the list.
  void RemoveNode(vtkMRMLNode *n) {
    this->CurrentScene->vtkCollection::RemoveItem((vtkObject *)n);};
  
  // Description:
  // Determine whether a particular node is present. Returns its position
  // in the list.
  int IsNodePresent(vtkMRMLNode *n) {
    return this->CurrentScene->vtkCollection::IsItemPresent((vtkObject *)n);};
  
  // Description:
  // Get the next path in the list.
  vtkMRMLNode *GetNextNode() {
    return (vtkMRMLNode *)(this->CurrentScene->GetNextItemAsObject());};
  
  vtkMRMLNode *GetNextNodeByClass(const char* className);
  
  vtkCollection *GetNodesByName(const char* name);

  vtkCollection *GetNodesByID(const char* name);

  vtkCollection* GetNodesByClassByID(const char* className, const char* id);
  
  vtkCollection *GetNodesByClassByName(const char* className, const char* name);
  
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
                                vtkTransform *xform );
  
  int GetTransformBetweenSpaces( const char *space1, const char *space2, 
                                 vtkTransform *xform );

  // Undo API
  void SetUndoOn() {UndoFlag=true;};
  void SetUndoOff() {UndoFlag=false;};
  int  GetUndoFlag() {return UndoFlag;};
  void SetUndoFlag(int flag) {UndoFlag = flag;};

  void Undo();

  void SaveStateForUndo(vtkMRMLNode *node);
  void SaveStateForUndo(vtkCollection *nodes);
  //BTX
  void SaveStateForUndo(std::vector<vtkMRMLNode *> nodes);
  //ETX

protected:
  vtkMRMLScene();
  ~vtkMRMLScene();
  vtkMRMLScene(const vtkMRMLScene&);
  void operator=(const vtkMRMLScene&);
  

  void PushIntoUndoStack();

  void ReplaceNodeInUndoStack(vtkMRMLNode *node, vtkMRMLNode *withNode);
  
  vtkCollection* CurrentScene;
  int UndoStackSize;
  bool UndoFlag;
  //BTX
  std::list< vtkCollection* >  UndoStack;
  //ETX
  
  char *URL;
  
  //BTX
  std::map< std::string, int> UniqueIDByClass;
  std::vector< std::string > UniqueIDs;
  std::vector< vtkMRMLNode* > RegisteredNodeClasses;
  std::vector< std::string > RegisteredNodeTags;
  //ETX
  
  vtkSetMacro(ErrorCode,unsigned long);
  vtkGetMacro(ErrorCode,unsigned long);

  vtkSetStringMacro(ClassNameList);
  vtkGetStringMacro(ClassNameList);
  
private:
  // hide the standard AddItem from the user and the compiler.
  void AddItem(vtkObject *o) { this->CurrentScene->vtkCollection::AddItem(o); };
  void RemoveItem(vtkObject *o) { this->CurrentScene->vtkCollection::RemoveItem(o); };
  void RemoveItem(int i) { this->CurrentScene->vtkCollection::RemoveItem(i); };
  int  IsItemPresent(vtkObject *o) { return this->CurrentScene->vtkCollection::IsItemPresent(o);};
  
  unsigned long ErrorCode;

  char* ClassNameList;

};

#endif
