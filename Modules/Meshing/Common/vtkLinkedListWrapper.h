/*=========================================================================

  Module:    $RCSfile: vtkLinkedListWrapper.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkLinkedListWrapper - a base class for templated containers
// .SECTION Description
// vtkLinkedListWrapper is a superclass for all container classes.  Since it
// does not provide any actuall data access methods, it is not
// templated, but it provides a set of method that can be used on all
// containers. It also provide a simple reference counting scheme.

// .SECTION Caveates
// Since vtkLinkedListWrapper and vtkAbstractList provide some pure virtual
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
//
#include "vtkCommon.h"


#include "vtkObject.h"

#include "vtkMimxActorBase.h"
#include "vtkSetGet.h" // For vtkTypeMacro.

#include "vtkLinkedList.txx"

#ifndef __vtkLinkedListWrapper_h
#define __vtkLinkedListWrapper_h

class VTK_MIMXCOMMON_EXPORT vtkLinkedListWrapper : public vtkObject
{
public:
  static vtkLinkedListWrapper *New();
  vtkTypeMacro(vtkLinkedListWrapper, vtkObject);
  vtkLinkedList<vtkMimxActorBase*> *List;
  int AppendItem(vtkMimxActorBase*);
  vtkMimxActorBase* GetItem(vtkIdType);
  int GetNumberOfItems();
  int RemoveItem(int );
protected:
  vtkLinkedListWrapper();
  virtual ~vtkLinkedListWrapper();
private:
  vtkLinkedListWrapper(const vtkLinkedListWrapper&); // Not implemented
  void operator=(const vtkLinkedListWrapper&); // Not implemented
};
#endif 



