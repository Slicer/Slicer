/*=========================================================================

  Module:    $RCSfile: vtkLinkedList.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkLinkedList - a templated linked list

#ifndef __vtkLinkedList_h
#define __vtkLinkedList_h

#include "vtkCommon.h"


#include "vtkAbstractList.h"

template <class DType> class vtkLinkedListNode;
template <class DType> class vtkLinkedListIterator;

template <class DType>
class vtkLinkedList : public vtkAbstractList<DType>
{
  friend class vtkLinkedListIterator<DType>;
  virtual const char* GetClassNameInternal() const { return "vtkLinkedList"; }

public:
  typedef vtkAbstractList<DType> Superclass;
  typedef vtkLinkedListIterator<DType> IteratorType;

  static vtkLinkedList<DType> *New();

  // Description:
  // Return an iterator to the list. This iterator is allocated using
  // New, so the developer is responsible for deleating it.
  vtkLinkedListIterator<DType> *NewIterator();
  
  // Description:
  // Append an Item to the end of the linked list.
  int AppendItem(DType a);
  
  // Description:
  // Insert an Item to the front of the linked list.
  int PrependItem(DType a);
  
  // Description:
  // Insert an Item to the specific location in the linked list.
  int InsertItem(vtkIdType loc, DType a);
  
  // Description:
  // Sets the Item at the specific location in the list to a new value.
  // It also checks if the item can be set.
  // It returns VTK_OK if successfull.
  int SetItem(vtkIdType loc, DType a);

  // Description:
  // Sets the Item at the specific location in the list to a new value.
  // This method does not perform any error checking.
  void SetItemNoCheck(vtkIdType loc, DType a);

   // Description:
  // Remove an Item from the linked list
  int RemoveItem(vtkIdType id);
  
  // Description:
  // Return an item that was previously added to this linked list. 
  DType GetItem(vtkIdType id);
      
  // Description:
  // Find an item in the linked list. Return VTK_OK if it was found
  // od VTK_ERROR if not found. The location of the item is returned in res.
  int FindItem(DType a, vtkIdType &res);

  // Description:
  // Find an item in the linked list using a comparison routine. 
  // Return VTK_OK if it was found
  // od VTK_ERROR if not found. The location of the item is returned in res.
  int FindItem(DType a, 
               vtkAbstractListCompareFunction(DType, compare), 
               vtkIdType &res);
  
  // Description:
  // Return the number of items currently held in this container. This
  // different from GetSize which is provided for some containers. GetSize
  // will return how many items the container can currently hold.
  vtkIdType GetNumberOfItems() const { return this->NumberOfItems; }
  
  // Description:
  // Returns the number of items the container can currently hold.
  // Since capacity is arbitrary for the linked list, this will 
  // always return the current number of elements.
  vtkIdType GetSize() const { return this->NumberOfItems; }

  // Description:
  // Removes all items from the container.
  void RemoveAllItems();

  // Description:
  // Since linked list does not have the notion of capacity,
  // this method always return VTK_ERROR.
  int SetSize(vtkIdType ) { return VTK_ERROR; }

  // Description:
  // This method dumps debug of the linked list.
  void DebugList();


protected:
  vtkLinkedList() {
    this->Head = 0; this->Tail = 0;
    this->NumberOfItems = 0; 
  }
  virtual ~vtkLinkedList();

  // Description:
  // Find a node with given index.

  vtkIdType NumberOfItems;
  vtkLinkedListNode<DType> *Head;
  vtkLinkedListNode<DType> *Tail;
  vtkLinkedListNode<DType>* FindNode(vtkIdType i);

private:
  vtkLinkedList(const vtkLinkedList<DType>&); // Not implemented
  void operator=(const vtkLinkedList<DType>&); // Not implemented
};

#ifdef VTK_NO_EXPLICIT_TEMPLATE_INSTANTIATION
#include "vtkLinkedList.txx"
#endif 

#endif



