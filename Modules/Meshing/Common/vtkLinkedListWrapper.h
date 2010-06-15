/*=========================================================================

  Module:    $RCSfile: vtkLinkedListWrapper.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkLinkedListWrapper - a base class for templated containers
// .SECTION Description
// vtkLinkedListWrapper is a superclass for all container classes.  Since it
// does not provide any actuall data access methods, it is not
// templated, but it provides a set of method that can be used on all
// containers. It also provide a simple reference counting scheme.

// .SECTION Caveates
// Since vtkLinkedListWrapper and vtkAbstractList provide some pure virtual
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

#include "vtkMimxSurfacePolyDataActor.h"
#include "vtkMimxUnstructuredGridActor.h"
#include "vtkMimxMeshActor.h"
#include "vtkMimxImageActor.h"
#include "vtkMimxCommonWin32Header.h"

#include "vtkSetGet.h" // For vtkTypeMacro.

#include "vtkLinkedList.txx"

// store using the MRML-backed lists, do declare them here

class vtkFiniteElementImageList;
class vtkFESurfaceList;
class vtkFiniteElementBuildingBlockList;
class vtkFiniteElementMeshList;
class vtkMRMLScene;

#ifndef __vtkLinkedListWrapper_h
#define __vtkLinkedListWrapper_h


class VTK_MIMXCOMMON_EXPORT vtkLinkedListWrapper : public vtkObject
{
public:
  static vtkLinkedListWrapper *New();
  vtkTypeMacro(vtkLinkedListWrapper, vtkObject);
  vtkLinkedList<vtkMimxActorBase*> *List;
  
  // this interface is abstract.  However, it will be used for
  // surface dataypes, buildingBlocks, and mesh datatypes.  Inside
  // the implementation, a test will be made on the datatype value
  // set in the actor to decide how it should be stored in MRML.  The
  // abstract interface serves as a dispatching method for the correct
  // concrete implementation
  
  int AppendItem(vtkMimxActorBase*);
  int AppendItem(vtkMimxImageActor*);
  int AppendItem(vtkMimxSurfacePolyDataActor* actor);
  int AppendItem(vtkMimxUnstructuredGridActor* actor);
  int AppendItem(vtkMimxMeshActor* actor);

  
  vtkMimxActorBase* GetItem(vtkIdType);
  int GetNumberOfItems();
  int RemoveItem(int );
  
  // save reference to the scene to be used for storage 
   void SetMRMLSceneForStorage(vtkMRMLScene* scene);
   
protected:
  vtkLinkedListWrapper();
  virtual ~vtkLinkedListWrapper();
 
  // store objects in MRML-backed lists, instance these lists here.
  vtkFiniteElementImageList *MRMLImageList;
  vtkFESurfaceList *MRMLSurfaceList;
  vtkFiniteElementBuildingBlockList *MRMLBBlockList;
  vtkFiniteElementMeshList *MRMLMeshList;

  // since separate instances of this list will be used to store objects of different types,
  // record the proper datatype for this instance to store.  This is set during the first entry and
  // then used as an error check to make sure all types added to the list are consistent.  The types stored
  // here are defined in mimxCommonDefine and correspond to the types of actors used for storage. 
  int ListDatatype;
  
private:
  vtkLinkedListWrapper(const vtkLinkedListWrapper&); // Not implemented
  void operator=(const vtkLinkedListWrapper&); // Not implemented
};
#endif 



