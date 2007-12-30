/*=auto=========================================================================

Portions (c) Copyright 2007 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: $
Date:      $Date: $
Version:   $Revision: $

=========================================================================auto=*/

#include <typeinfo>
#include <iostream>

#include "vtkObjectFactory.h"
#include "vtkIGTMessageGenericAttribute.h"

template <class T>
vtkCxxRevisionMacro(vtkIGTMessageGenericAttribute<T>, "$Revision: 1.0 $");
//vtkInstantiatorNewMacro(vtkIGTMessageGenericAttribute);
//vtkStandardNewMacro(vtkIGTMessageGenericAttribute<T>);


template <class T>
vtkIGTMessageGenericAttribute<T>* vtkIGTMessageGenericAttribute<T>::New()
{
  return new vtkIGTMessageGenericAttribute<T>;
}

//template <class T>
//vtkIGTMessageGenericAttribute<T>* vtkIGTMessageGenericAttribute::vtkIGTMessageGenericAttribute()
//{
//  return new vtkIGTMessageGenericAttribute<T>;
//}

template <class T>
vtkIGTMessageGenericAttribute<T>::vtkIGTMessageGenericAttribute()
{
  Superclass::vtkIGTMessageAttributeBase();
  this->type_name = typeid(T).name();
}

template <class T>
vtkIGTMessageGenericAttribute<T>::~vtkIGTMessageGenericAttribute()
{
}

template <class T>
void vtkIGTMessageGenericAttribute<T>::PrintSelf(ostream& os, vtkIndent indent)
{
}

/*
template <class T, typename T2> 
virtual void vtkIGTMessageGenericAttribute<T>::SetAttribute(T2* ptr)
{
  if (typeid(T2) == typeid(T))
    {
      data = *ptr;
      return 1;
    }
  return 0;
}


template <class T, typename T2>
virtual int vtkIGTMessageGenericAttribute<T>::GetAttribute(T2* ptr)
{
  if (typeid(T2) == typeid(T))
    {
      *ptr = data;
      return 1;
    }
  return 0;
}
*/

template <class T> 
int vtkIGTMessageGenericAttribute<T>::SetAttribute(void* ptr)
{
  if (ptr)
    {
    T* attr = static_cast<T*>(ptr);
    this->data = *attr;
    return 1;
    }
  else
    {
    return 0;
    }

 
}


template <class T>
int vtkIGTMessageGenericAttribute<T>::GetAttribute(void* ptr)
{
  if (ptr)
    {
    T* attr = static_cast<T*>(ptr);
    *attr = this->data;
    return 1;
    }
  else
    {
    return 0;
    }
}


template <class T>
int vtkIGTMessageGenericAttribute<T>::SetAttribute(T* ptr)
{
  if (ptr)
    {
    this->data = *ptr;
    return 1;
    }
  else
    {
    return 0;
    }
}


template <class T>
int vtkIGTMessageGenericAttribute<T>::GetAttribute(T* ptr)
{
  if (ptr)
    {
    *ptr = this->data;
    return 1;
    }
  else
    {
    return 0;
    }
}


template <class T>
void vtkIGTMessageGenericAttribute<T>::ClearAttribute()
{
  this->data = static_cast<T>(0);
}

template <class T>
int vtkIGTMessageGenericAttribute<T>::Alloc()
{
}

template <class T>
int vtkIGTMessageGenericAttribute<T>::Free()
{
}

