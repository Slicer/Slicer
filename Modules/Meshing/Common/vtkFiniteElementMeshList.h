/*=========================================================================

  Module:    $RCSfile: vtkLinkedListWrapper.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkFiniteElementMeshList - a  class that maintains a list of 
//  finite element mesh  objects
// .SECTION Description
// vtkFiniteElementMeshList manages the storage of several Finite Element
// mesh objects.  Storage is provided in the MRML tree contained in Slicer3.  
// This interface is identifical to the API used by the Univ. of Iowa stand-alone
// Finite Element tools to ease integration between the standalone tools and Slicer.
//
// 

//#include "vtkCommon.h"
#include "vtkObject.h"
#include "vtkMimxUnstructuredGridActor.h"
#include "vtkLinkedList.h"
#include "vtkLinkedListWrapper.h"
#include "vtkSetGet.h" // For vtkTypeMacro.
#include "vtkMimxCommonWin32Header.h"

#include "vtkMRMLFiniteElementMeshNode.h"

// pointer to the scene to use for storage
class vtkMRMLScene;
class vtkMimxMeshActor;

#ifndef __vtkFiniteElementMeshList_h
#define __vtkFiniteElementMeshList_h

class  VTK_MIMXCOMMON_EXPORT vtkFiniteElementMeshList : public vtkObject
{
public:
  static vtkFiniteElementMeshList *New();
  vtkTypeMacro(vtkFiniteElementMeshList, vtkObject);

  // save reference to the scene to be used for storage 
  void SetMRMLSceneForStorage(vtkMRMLScene* scene);
  
  virtual int AppendItem(vtkMimxMeshActor*);
  virtual vtkMimxMeshActor* GetItem(vtkIdType);
  virtual int GetNumberOfItems();
  virtual int RemoveItem(int );
protected:
    vtkMRMLScene* savedMRMLScene; 
    vtkFiniteElementMeshList();
  virtual ~vtkFiniteElementMeshList();
private:
    vtkFiniteElementMeshList(const vtkFiniteElementMeshList&); // Not implemented
   void operator=(const vtkFiniteElementMeshList&); // Not implemented
};
#endif 



