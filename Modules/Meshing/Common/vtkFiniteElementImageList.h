/*=========================================================================

  Module:    $RCSfile: vtkLinkedListWrapper.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkFiniteElementImageList - a  class that maintains a list of FE Image objects
// .SECTION Description
// vtkFiniteElementImageList manages the storage of several Finite Element
// image objects.  Storage is provided in the MRML tree contained in Slicer3.
// This interface is identical to the API used by the Univ. of Iowa stand-alone
// Finite Element tools to ease integration between the standalone tools and Slicer.
//
// 

//#include "vtkCommon.h"
#include "vtkObject.h"
#include "vtkMimxImageActor.h"
#include "vtkLinkedList.h"
#include "vtkLinkedListWrapper.h"
#include "vtkSetGet.h" // For vtkTypeMacro.
#include "vtkMimxCommonWin32Header.h"
#include "vtkMRMLFiniteElementImageNode.h"

// pointer to the scene to use for storage
class vtkMRMLScene;

#ifndef __vtkFiniteElementImageList_h
#define __vtkFiniteElementImageList_h

class  VTK_MIMXCOMMON_EXPORT vtkFiniteElementImageList : public vtkObject
{
public:
  static vtkFiniteElementImageList *New();
  vtkTypeMacro(vtkFiniteElementImageList, vtkObject);


  // save reference to the scene to be used for storage 
  void SetMRMLSceneForStorage(vtkMRMLScene* scene);
  
  virtual int AppendItem(vtkMimxImageActor*);
  virtual vtkMimxImageActor* GetItem(vtkIdType);
  virtual int GetNumberOfItems();
  virtual int RemoveItem(int );
protected:
    vtkMRMLScene* savedMRMLScene; 
    vtkFiniteElementImageList();
  virtual ~vtkFiniteElementImageList();
private:
    vtkFiniteElementImageList(const vtkFiniteElementImageList&); // Not implemented
   void operator=(const vtkFiniteElementImageList&); // Not implemented
};
#endif 



