/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: mimxLinkedList.h,v $
Language:  C++
Date:      $Date: 2007/05/17 16:30:26 $
Version:   $Revision: 1.4 $

 Musculoskeletal Imaging, Modelling and Experimentation (MIMX)
 Center for Computer Aided Design
 The University of Iowa
 Iowa City, IA 52242
 http://www.ccad.uiowa.edu/mimx/
 
Copyright (c) The University of Iowa. All rights reserved.
See MIMXCopyright.txt or http://www.ccad.uiowa.edu/mimx/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __mimxLinkedList_h
#define __mimxLinkedList_h

#include <list>


//***************************************

template <class LinkedListElementType>
class CLinkedList
{
public:
        typedef LinkedListElementType ElementType;
  typedef typename std::list<ElementType> ListType;
  typedef typename ListType::iterator IteratorType;

  void Add( ElementType );
        void Remove( ElementType );
        ElementType GetValue( int );
        ElementType GetLastItem();
        void Delete();
        int Size();
private:
        ListType List;

}; // end CLinkedList


//this function puts an item on the tail of the list
template <class LinkedListElementType>
void CLinkedList<LinkedListElementType>::Add(LinkedListElementType pData )
{
        this->List.push_back(pData);
} 

//this function an item in the list for a corresponding position
template <class LinkedListElementType>
LinkedListElementType CLinkedList<LinkedListElementType>::GetValue(int pos )
{
        int count = 0;
        IteratorType it = this->List.begin();
        do
          {
                count++;
                if(pos !=count) it++;
          }
  while (pos !=count);
        
  return (*it);
} 

//this function an item in the list for a corresponding position
template <class LinkedListElementType>
LinkedListElementType CLinkedList<LinkedListElementType>::GetLastItem()
{
        IteratorType it = this->List.begin();
        int size = this->List.size();
        int count = 0;
        do
          {
                count++;
                if(size !=count) it++;
          }
  while(size !=count);

        return (*it);
} 

template <class LinkedListElementType>
void CLinkedList<LinkedListElementType>::Delete()
{
        this->List.clear();
}

template <class LinkedListElementType>
void CLinkedList<LinkedListElementType>::Remove(LinkedListElementType pData )
{
        this->List.remove(pData);
} 

template <class LinkedListElementType>
int CLinkedList<LinkedListElementType>::Size()
{

        return this->List.size();
} 

#endif
