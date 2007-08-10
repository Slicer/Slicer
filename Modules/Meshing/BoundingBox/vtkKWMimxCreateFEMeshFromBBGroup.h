/*=========================================================================

  Module:    $RCSfile: vtkKWMimxCreateFEMeshFromBBGroup.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWMimxCreateFEMeshFromBBGroup - a tabbed notebook of UI pages
// .SECTION Description
// The class is derived from vtkKWMimxGroupBase. It contains 5 pages 1) Image
// 2) Surface 3) Bounding Box 4) F E Mesh 5) Mesh quality. Each page inturn
// contains a list of options specific to a page.

#ifndef __vtkKWMimxCreateFEMeshFromBBGroup_h
#define __vtkKWMimxCreateFEMeshFromBBGroup_h

#include "vtkBoundingBox.h"

#include "vtkKWMimxGroupBase.h"
#include "vtkKWMimxFEMeshMenuGroup.h"
#include "vtkKWMimxViewWindow.h"
#include "vtkKWMimxSurfaceMenuGroup.h"

class vtkKWComboBoxWithLabel;

class VTK_BOUNDINGBOX_EXPORT vtkKWMimxCreateFEMeshFromBBGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxCreateFEMeshFromBBGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxCreateFEMeshFromBBGroup,vtkKWMimxGroupBase);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();
  void CreateFEMeshFromBBCallback();
  void CreateFEMeshFromBBCancelCallback();
  void UpdateObjectLists();
protected:
        vtkKWMimxCreateFEMeshFromBBGroup();
        ~vtkKWMimxCreateFEMeshFromBBGroup();
        virtual void CreateWidget();
  vtkKWComboBoxWithLabel *SurfaceListComboBox;
  vtkKWComboBoxWithLabel *BBListComboBox;
private:
  vtkKWMimxCreateFEMeshFromBBGroup(const vtkKWMimxCreateFEMeshFromBBGroup&); // Not implemented
  void operator=(const vtkKWMimxCreateFEMeshFromBBGroup&); // Not implemented
 };

#endif

