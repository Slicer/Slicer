/*=========================================================================

  Module:    $RCSfile: vtkAbstractIterator.txx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __vtkAbstractIterator_txx
#define __vtkAbstractIterator_txx

#include "vtkAbstractIterator.h"

#include "vtkDebugLeaks.h"
#include "vtkContainer.h"

template<class KeyType, class DataType>
vtkAbstractIterator<KeyType,DataType>::vtkAbstractIterator() 
{ 
  this->ReferenceCount = 1;
  this->Container = 0;
}

template<class KeyType, class DataType>
vtkAbstractIterator<KeyType,DataType>::~vtkAbstractIterator() 
{
  this->SetContainer(0);
}

template<class KeyType, class DataType>
void vtkAbstractIterator<KeyType,DataType>::SetContainer(vtkContainer* container)
{
  if ( this->Container == container )
    {
    return;
    }
  if ( this->Container )
    {
    this->Container->UnRegister(0);
    this->Container = 0;
    }
  this->Container = container;
  if ( this->Container )
    {
    this->Container->Register(0);
    }
}


#if defined ( _MSC_VER )
template <class KeyType,class DataType>
vtkAbstractIterator<KeyType,DataType>::vtkAbstractIterator(const vtkAbstractIterator<KeyType,DataType>&){}
template <class KeyType,class DataType>
void vtkAbstractIterator<KeyType,DataType>::operator=(const vtkAbstractIterator<KeyType,DataType>&){}
#endif

#endif



