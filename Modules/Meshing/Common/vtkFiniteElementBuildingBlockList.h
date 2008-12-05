/*=========================================================================

  Module:    $RCSfile: vtkLinkedListWrapper.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkFiniteElementBuildingBlockList - a  class that maintains a list of 
//  finite element bounding box objects
// .SECTION Description
// vtkFiniteElementBuildingBlockList manages the storage of several Finite Element
// bbox objects.  Storage is provided in the MRML tree contained in Slicer3.  
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

#include "vtkMRMLFiniteElementBuildingBlockNode.h"

// pointer to the scene to use for storage
class vtkMRMLScene;

#ifndef __vtkFiniteElementBuildingBlockList_h
#define __vtkFiniteElementBuildingBlockList_h

class  VTK_MIMXCOMMON_EXPORT vtkFiniteElementBuildingBlockList : public vtkObject
{
public:
  static vtkFiniteElementBuildingBlockList *New();
  vtkTypeMacro(vtkFiniteElementBuildingBlockList, vtkObject);
//BTX
  vtkLinkedList<vtkMimxUnstructuredGridActor*> *InternalMimxObjectList;
//ETX

  // save reference to the scene to be used for storage 
  void SetMRMLSceneForStorage(vtkMRMLScene* scene);
  
  virtual int AppendItem(vtkMimxUnstructuredGridActor*);
  //virtual int ModifyItem(vtkIdType, vtkMimxUnstructuredGridActor*);
  virtual vtkMimxUnstructuredGridActor* GetItem(vtkIdType);
  virtual int GetNumberOfItems();
  virtual int RemoveItem(int );
  
protected:
    vtkMRMLScene* savedMRMLScene; 
    vtkFiniteElementBuildingBlockList();
    virtual ~vtkFiniteElementBuildingBlockList();
    //bool ItemMatchesMRMLNode(vtkMimxUnstructuredGridActor* actor, vtkMRMLFiniteElementBuildingBlockNode* testMRMLNode);
private:
    vtkFiniteElementBuildingBlockList(const vtkFiniteElementBuildingBlockList&); // Not implemented
   void operator=(const vtkFiniteElementBuildingBlockList&); // Not implemented
};
#endif 



