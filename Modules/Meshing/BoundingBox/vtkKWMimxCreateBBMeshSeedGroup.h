/*=========================================================================

  Module:    $RCSfile: vtkKWMimxCreateBBMeshSeedGroup.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWMimxCreateBBMeshSeedGroup - a tabbed notebook of UI pages
// .SECTION Description
// The class is derived from vtkKWCompositeWidget. It contains 5 pages 1) Image
// 2) Surface 3) Bounding Box 4) F E Mesh 5) Mesh quality. Each page inturn
// contains a list of options specific to a page.

#ifndef __vtkKWMimxCreateBBMeshSeedGroup_h
#define __vtkKWMimxCreateBBMeshSeedGroup_h

#include "vtkBoundingBox.h"

#include "vtkKWMimxGroupBase.h"
#include "vtkKWMimxFEMeshMenuGroup.h"
#include "vtkKWMimxViewWindow.h"
#include "vtkKWMimxSurfaceMenuGroup.h"

class vtkKWMenuButtonWithLabel;
class vtkKWFrameWithLabel;
class vtkKWPushButton;

class vtkLinkedListWrapper;
class vtkKWComboBoxWithLabel;
class vtkKWEntryWithLabel;
class vtkKWMimxViewWindow;

class VTK_BOUNDINGBOX_EXPORT vtkKWMimxCreateBBMeshSeedGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxCreateBBMeshSeedGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxCreateBBMeshSeedGroup,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();
  vtkGetObjectMacro(MimxViewWindow, vtkKWMimxViewWindow);
  vtkSetObjectMacro(MimxViewWindow, vtkKWMimxViewWindow);

  void CreateBBMeshSeedCallback();
  void CreateBBMeshSeedCancelCallback();

protected:
        vtkKWMimxCreateBBMeshSeedGroup();
        ~vtkKWMimxCreateBBMeshSeedGroup();
        virtual void CreateWidget();
  vtkKWComboBoxWithLabel *ObjectListComboBox;
  vtkKWMimxViewWindow *MimxViewWindow;
  vtkKWEntryWithLabel *AvElementLength;
private:
  vtkKWMimxCreateBBMeshSeedGroup(const vtkKWMimxCreateBBMeshSeedGroup&); // Not implemented
  void operator=(const vtkKWMimxCreateBBMeshSeedGroup&); // Not implemented
 };

#endif

