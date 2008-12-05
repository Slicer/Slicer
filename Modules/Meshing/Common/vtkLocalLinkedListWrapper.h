/*=========================================================================

  Module:    $RCSfile: vtkLocalLinkedListWrapper.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkLocalLinkedListWrapper - a base class for templated containers
// .SECTION Description
// vtkLocalLinkedListWrapper is a superclass for all container classes.  Since it
// does not provide any actuall data access methods, it is not
// templated, but it provides a set of method that can be used on all
// containers. It also provide a simple reference counting scheme.

// .SECTION Caveates
// Since vtkLocalLinkedListWrapper and vtkAbstractList provide some pure virtual
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

// Updates:
//   The interface stayed the same, but the implementation changed
//   to enable these lists to store data in the MRML tree as well as
//   the local lists.  

#include "vtkObject.h"
#include "vtkMimxCommonWin32Header.h"
#include "vtkMimxSurfacePolyDataActor.h"
#include "vtkMimxUnstructuredGridActor.h"
#include "vtkMimxMeshActor.h"
#include "vtkMimxImageActor.h"

#include "vtkSetGet.h" // For vtkTypeMacro.

#include "vtkLinkedList.txx"


#ifndef __vtkLocalLinkedListWrapper_h
#define __vtkLocalLinkedListWrapper_h

class VTK_MIMXCOMMON_EXPORT vtkLocalLinkedListWrapper : public vtkObject
{
public:
  static vtkLocalLinkedListWrapper *New();
  vtkTypeMacro(vtkLocalLinkedListWrapper, vtkObject);
  vtkLinkedList<vtkMimxActorBase*> *List;
  
  // Store actor hierarchy members in a local (non-MRML backed) list
  int AppendItem(vtkMimxActorBase*);
  
  vtkMimxActorBase* GetItem(vtkIdType);
  int GetNumberOfItems();
  int RemoveItem(int );

   
protected:
  vtkLocalLinkedListWrapper();
  virtual ~vtkLocalLinkedListWrapper();
  
  
private:
  vtkLocalLinkedListWrapper(const vtkLocalLinkedListWrapper&); // Not implemented
  void operator=(const vtkLocalLinkedListWrapper&); // Not implemented
};
#endif 



