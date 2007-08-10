/*=========================================================================

  Module:    $RCSfile: vtkKWMimxCreateBBFromBoundsGroup.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWMimxCreateBBFromBoundsGroup - a tabbed notebook of UI pages
// .SECTION Description
// The class is derived from vtkKWMimxGroupBase. It contains 5 pages 1) Image
// 2) Surface 3) Bounding Box 4) F E Mesh 5) Mesh quality. Each page inturn
// contains a list of options specific to a page.

#ifndef __vtkKWMimxCreateBBFromBoundsGroup_h
#define __vtkKWMimxCreateBBFromBoundsGroup_h

#include "vtkBoundingBox.h"

#include "vtkKWMimxGroupBase.h"
#include "vtkKWMimxFEMeshMenuGroup.h"
#include "vtkKWMimxViewWindow.h"
#include "vtkKWMimxSurfaceMenuGroup.h"

class vtkKWComboBoxWithLabel;

class VTK_BOUNDINGBOX_EXPORT vtkKWMimxCreateBBFromBoundsGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxCreateBBFromBoundsGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxCreateBBFromBoundsGroup,vtkKWMimxGroupBase);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();
  void CreateBBFromBoundsCallback();
  void CreateBBFromBoundsCancelCallback();
  void UpdateObjectLists();
protected:
        vtkKWMimxCreateBBFromBoundsGroup();
        ~vtkKWMimxCreateBBFromBoundsGroup();
        virtual void CreateWidget();
  vtkKWComboBoxWithLabel *ObjectListComboBox;
private:
  vtkKWMimxCreateBBFromBoundsGroup(const vtkKWMimxCreateBBFromBoundsGroup&); // Not implemented
  void operator=(const vtkKWMimxCreateBBFromBoundsGroup&); // Not implemented
 };

#endif

