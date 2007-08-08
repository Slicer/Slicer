/*=========================================================================

  Module:    $RCSfile: vtkLinkedListWrapper.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkLinkedListWrapper.h"

#include "vtkDebugLeaks.h"
#include "vtkObject.h"
#include "vtkObjectFactory.h"

//vtkCxxRevisionMacro(vtkLinkedListWrapper, "$Revision: 1.3 $");

vtkStandardNewMacro(vtkLinkedListWrapper);

vtkLinkedListWrapper::vtkLinkedListWrapper() 
{ 
  List = vtkLinkedList<vtkMimxActorBase*>::New();
}

vtkLinkedListWrapper::~vtkLinkedListWrapper() 
{
  List->Delete();
}

int vtkLinkedListWrapper::AppendItem(vtkMimxActorBase* actor)
{
  return this->List->AppendItem(actor);
}

vtkMimxActorBase* vtkLinkedListWrapper::GetItem(vtkIdType id)
{
  return this->List->GetItem(id);
}

int vtkLinkedListWrapper::GetNumberOfItems()
{
  return this->List->GetNumberOfItems();
}

int vtkLinkedListWrapper::RemoveItem(int Num)
{
  return this->List->RemoveItem(Num);
}
