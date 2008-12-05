/*=========================================================================

  Module:    $RCSfile: vtkLinkedListWrapperTree.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkLinkedListWrapperTree - a base class for templated containers
// .SECTION Description
// vtkLinkedListWrapperTree is a superclass for all container classes.  Since it
// does not provide any actuall data access methods, it is not
// templated, but it provides a set of method that can be used on all
// containers. It also provide a simple reference counting scheme.

// .SECTION Caveates
// Since vtkLinkedListWrapperTree and vtkAbstractList provide some pure virtual
// methods, each object of type container will have v-tabe.
//
// For container of strings, use <const char*> as a template
// argument. This way you will be able to use string literals as keys
// or values. Key and Value types must be default constructable.
//
// Each container subclass have to understand the following methods:
// 
// vtkIdType GetNumberOfItems();
//
// Return the number of items currently held in this container. This
// different from GetSize which is provided for some
// containers. GetSize will return how many items the container can
// currently hold.
//
// void RemoveAllItems();
//
// Removes all items from the container.
  
// .SECTION See Also
// vtkAbstractIterator, vtkAbstractList, vtkAbstractMap

#include "vtkObject.h"
#include "vtkMimxUnstructuredGridActor.h"
#include "vtkSetGet.h" // For vtkTypeMacro.
#include "vtkMimxCommonWin32Header.h"

#include "vtkLinkedList.txx"

#ifndef __vtkLinkedListWrapperTree_h
#define __vtkLinkedListWrapperTree_h

struct Node
{
        vtkMimxUnstructuredGridActor *Data;
        Node *Parent;
        Node *PeerPrev;
        Node *PeerNext;
        Node *Child;
};

class VTK_MIMXCOMMON_EXPORT vtkLinkedListWrapperTree : public vtkObject
{
public:
        static vtkLinkedListWrapperTree *New();
  vtkTypeMacro(vtkLinkedListWrapperTree, vtkObject);
  vtkLinkedList<Node*> *List;
  int AppendItem(Node*);
  Node* GetItem(vtkIdType);
  Node* GetItem(const char*);
  int GetNumberOfItems();
  int RemoveItem(int );
  int GetItemNumber(const char* );
protected:
  vtkLinkedListWrapperTree();
  virtual ~vtkLinkedListWrapperTree();
private:
  vtkLinkedListWrapperTree(const vtkLinkedListWrapperTree&); // Not implemented
  void operator=(const vtkLinkedListWrapperTree&); // Not implemented
};
#endif 



