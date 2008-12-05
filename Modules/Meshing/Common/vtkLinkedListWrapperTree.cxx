/*=========================================================================

  Module:    $RCSfile: vtkLinkedListWrapperTree.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkLinkedListWrapperTree.h"

#include "vtkDebugLeaks.h"
#include "vtkObject.h"
#include "vtkObjectFactory.h"

//vtkCxxRevisionMacro(vtkLinkedListWrapperTree, "$Revision: 1.3 $");

vtkStandardNewMacro(vtkLinkedListWrapperTree);

vtkLinkedListWrapperTree::vtkLinkedListWrapperTree() 
{ 
        List = vtkLinkedList<Node*>::New();
}

vtkLinkedListWrapperTree::~vtkLinkedListWrapperTree() 
{
        List->Delete();
}

int vtkLinkedListWrapperTree::AppendItem(Node* actor)
{
        return this->List->AppendItem(actor);
}

Node* vtkLinkedListWrapperTree::GetItem(vtkIdType id)
{
        return this->List->GetItem(id);
}

Node* vtkLinkedListWrapperTree::GetItem(const char* name)
{
        for (int i=0; i<this->List->GetNumberOfItems(); i++)
        {
                //while (this->List->GetItem(i) != NULL)
                //{
                if(!strcmp(name, this->List->GetItem(i)->Data->GetFileName()))
                {
                        return this->List->GetItem(i);
                }
                //}
        }
        return NULL;
}

int vtkLinkedListWrapperTree::GetNumberOfItems()
{
        return this->List->GetNumberOfItems();
}

int vtkLinkedListWrapperTree::RemoveItem(int Num)
{
        return this->List->RemoveItem(Num);
}

int vtkLinkedListWrapperTree::GetItemNumber(const char *name)
{
        //std::cout << "GetItemNumber of " << name << std::endl;
        for (int i=0; i<this->List->GetNumberOfItems(); i++)
        {
                //while (this->List->GetItem(i) != NULL)
                //{
                //std::cout << "\tItem " << i << " Name: " << this->List->GetItem(i)->Data->GetFileName() << std::endl;
                //std::cout << "\tName Length " << strlen(name) << " File Name Length: " << strlen( this->List->GetItem(i)->Data->GetFileName()) << std::endl;
                if(!strcmp(name, this->List->GetItem(i)->Data->GetFileName()))
                {
                        //std::cout << "\tReturn " << i << std::endl;
                        return i;
                }
                //}
        }
        return -1;
}
