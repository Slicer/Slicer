/*=========================================================================

  Module:    $RCSfile: vtkLocalLinkedListWrapper.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkLocalLinkedListWrapper.h"

#include "vtkDebugLeaks.h"
#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkMimxMeshActor.h"


//vtkCxxRevisionMacro(vtkLocalLinkedListWrapper, "$Revision: 1.3 $");

vtkStandardNewMacro(vtkLocalLinkedListWrapper);

vtkLocalLinkedListWrapper::vtkLocalLinkedListWrapper() 
{ 
        List = vtkLinkedList<vtkMimxActorBase*>::New();
  
}

vtkLocalLinkedListWrapper::~vtkLocalLinkedListWrapper() 
{
        this->List->Delete();

}

int vtkLocalLinkedListWrapper::AppendItem(vtkMimxActorBase* actor)
{
    return this->List->AppendItem(actor);
}



vtkMimxActorBase* vtkLocalLinkedListWrapper::GetItem(vtkIdType id)
{
        return this->List->GetItem(id);

}

int vtkLocalLinkedListWrapper::GetNumberOfItems()
{
    return this->List->GetNumberOfItems();
}

int vtkLocalLinkedListWrapper::RemoveItem(int Num)
{
    return this->List->RemoveItem(Num);  
}

