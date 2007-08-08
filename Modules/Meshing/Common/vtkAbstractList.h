/*=========================================================================

  Module:    $RCSfile: vtkAbstractList.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkAbstractList - a dynamic list data structure
// .SECTION Description
// vtkAbstractList is a an abstract templated superclass of all
// containers that implement list data structure.
//
// List data structure is a one dimensional sequence of elements
// with strict ordering. Every element has an index and each 
// element except the first and the last one, have unique 
// predecessor and successor. Examples of list data structure
// are dynamic array (vector) and linked list.
//
// Each List container class has to implement the following methods:
// 
// int AppendItem(DType a);
// Append an Item to the end of the list. It returns VTK_OK if
// successfull.
//
// int PrependItem(DType a);
// Insert an Item to the front of the list. All items are moved one
// place to the right It returns VTK_OK if successfull.
//
// int InsertItem(vtkIdType loc, DType a);
// Insert an Item to the specific location in the list. All items from
// that location on are moved one place to the right.  It returns
// VTK_OK if successfull.
//
// int SetItem(vtkIdType loc, DType a);
// Sets the Item at the specific location in the list to a new value.
// The old value is lost. This method should also checks if the item
// can be set. It returns VTK_OK if successfull.
//
// void SetItemNoCheck(vtkIdType loc, DType a);
// Sets the Item at the specific location in the list to a new
// value. The old value is lost.  This method does not perform any
// error checking.
//
// int RemoveItem(vtkIdType loc);
// Remove an Item at a specified location from the list. This means
// that all items following this item will be moved one place to the
// left. It returns VTK_OK if successfull.
//
// int GetItem(vtkIdType loc, DType& ret);
// Return an item at the specified location of the list. It returns
// VTK_OK if successfull.
//      
// int FindItem(DType a, vtkIdType &res);
// Find an item in the list. Return VTK_OK if it was found, VTK_ERROR
// if it was not found. The location of the item is returned in res.
//
// int FindItem(DType a, CompareFunctionType compare, vtkIdType &res);
// Find an item in the list using a comparison routine.  Return VTK_OK
// if it was found, VTK_ERROR if it was not found. The location of the
// item is returned in res.
//
// int SetSize(vtkIdType size);
// Set the capacity of the list. It returns VTK_OK if successfull.
//
// vtkIdType GetNumberOfItems();
// Return the number of items currently held in this container. This
// different from GetSize which is provided for some
// containers. GetSize will return how many items the container can
// currently hold.
//
// vtkIdType GetSize();
// Returns the number of items the container can currently hold.  This
// is the capacity of the container.
//
// .SECTION See also
// vtkContainer vtkAbstractMap

#include "vtkCommon.h"

#include "vtkContainer.h"

#ifndef __vtkAbstractList_h
#define __vtkAbstractList_h

// Since some compilers have problems with keyword typename, we have 
// to do this with macros.
#define vtkAbstractListCompareFunction(KeyType, CompareFunction) \
    int (*CompareFunction)(const KeyType&  k1, const KeyType& k2)

template<class DType>
class vtkAbstractList : public vtkContainer
{
  virtual const char* GetClassNameInternal() const { return "vtkAbstractList"; }
public:
  typedef vtkContainer Superclass;
  static vtkAbstractList<DType>* New();

  // Just to avoid typing over and over, let us define some typedefs.
  // They will not work in subclasses, but this header file will 
  // be more readable.
  typedef vtkAbstractListCompareFunction(DType, CompareFunctionType);


protected:
  vtkAbstractList();

private:
  vtkAbstractList(const vtkAbstractList<DType>&); // Not implement
  void operator=(const vtkAbstractList<DType>&); // Not implement
};

#ifdef VTK_NO_EXPLICIT_TEMPLATE_INSTANTIATION
#include "vtkAbstractList.txx"
#endif 

#endif



