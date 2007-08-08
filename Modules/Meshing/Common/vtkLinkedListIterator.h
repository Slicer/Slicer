/*=========================================================================

  Module:    $RCSfile: vtkLinkedListIterator.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkLinkedListIterator - a templated linked list iterator

#ifndef __vtkLinkedListIterator_h
#define __vtkLinkedListIterator_h

#include "vtkCommon.h"


#include "vtkAbstractIterator.h"

template <class DType> class vtkLinkedList;
template <class DType> class vtkLinkedListNode;

template <class DType>
class VTK_MIMXCOMMON_EXPORT vtkLinkedListIterator : public vtkAbstractIterator<vtkIdType,DType>
{
  friend class vtkLinkedList<DType>;
  virtual const char* GetClassNameInternal() const { return "vtkLinkedListIterator"; }

public:
  // Description:
  // Retrieve the index of the element.
  // This method returns VTK_OK if key was retrieved correctly.
  int GetKey(vtkIdType&);

  // Description:
  // Retrieve the data from the iterator. 
  // This method returns VTK_OK if key was retrieved correctly.
  int GetData(DType&);

  // Description:
  // Initialize the traversal of the container. 
  // Set the iterator to the "beginning" of the container.
  void InitTraversal();

  // Description:
  // Check if the iterator is at the end of the container. Returns 1 for yes
  // and 0 for no.
  int IsDoneWithTraversal();

  // Description:
  // Increment the iterator to the next location.
  void GoToNextItem();

  // Description:
  // Decrement the iterator to the next location.
  // On singly Linked list this operations is extremely expensive, because
  // it has to traverse through potentially whole list.
  void GoToPreviousItem();

  // Description:
  // Go to the first item of the list.
  void GoToFirstItem();

  // Description:
  // Go to the last item of the list.
  void GoToLastItem();

protected:
  static vtkLinkedListIterator<DType> *New();

  vtkLinkedListIterator() {
    this->Pointer = 0; 
  }
  virtual ~vtkLinkedListIterator() {}

  vtkLinkedListNode<DType> *Pointer;

private:
  vtkLinkedListIterator(const vtkLinkedListIterator&); // Not implemented
  void operator=(const vtkLinkedListIterator&); // Not implemented
};

#ifdef VTK_NO_EXPLICIT_TEMPLATE_INSTANTIATION
#include "vtkLinkedListIterator.txx"
#endif 

#endif



