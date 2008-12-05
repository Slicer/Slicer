/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxCreateBBMeshSeedGroup.h,v $
Language:  C++
Date:      $Date: 2008/02/01 15:24:57 $
Version:   $Revision: 1.7 $

 Musculoskeletal Imaging, Modelling and Experimentation (MIMX)
 Center for Computer Aided Design
 The University of Iowa
 Iowa City, IA 52242
 http://www.ccad.uiowa.edu/mimx/
 
Copyright (c) The University of Iowa. All rights reserved.
See MIMXCopyright.txt or http://www.ccad.uiowa.edu/mimx/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
// .NAME vtkKWMimxCreateBBMeshSeedGroup - a tabbed notebook of UI pages
// .SECTION Description
// The class is derived from vtkKWCompositeWidget. It contains 5 pages 1) Image
// 2) Surface 3) Building Block 4) F E Mesh 5) Mesh quality. Each page inturn
// contains a list of options specific to a page.

#ifndef __vtkKWMimxCreateBBMeshSeedGroup_h
#define __vtkKWMimxCreateBBMeshSeedGroup_h

#include "vtkKWMimxGroupBase.h"
#include "vtkKWMimxFEMeshMenuGroup.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkKWMimxSurfaceMenuGroup.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWMenuButtonWithLabel;
class vtkKWFrameWithLabel;
class vtkKWPushButton;
class vtkLinkedListWrapper;
class vtkKWComboBoxWithLabel;
class vtkKWEntryWithLabel;
class vtkKWRenderWidget;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxCreateBBMeshSeedGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxCreateBBMeshSeedGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxCreateBBMeshSeedGroup,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();
  vtkGetObjectMacro(MimxMainWindow, vtkKWMimxMainWindow);
  vtkSetObjectMacro(MimxMainWindow, vtkKWMimxMainWindow);

  void CreateBBMeshSeedDoneCallback();
  int CreateBBMeshSeedApplyCallback();
  void CreateBBMeshSeedCancelCallback();
  void UpdateObjectLists();

protected:
        vtkKWMimxCreateBBMeshSeedGroup();
        ~vtkKWMimxCreateBBMeshSeedGroup();
        virtual void CreateWidget();
  vtkKWComboBoxWithLabel *ObjectListComboBox;
  vtkKWMimxMainWindow *MimxMainWindow;
  vtkKWEntryWithLabel *AvElementLength;
private:
  vtkKWMimxCreateBBMeshSeedGroup(const vtkKWMimxCreateBBMeshSeedGroup&); // Not implemented
  void operator=(const vtkKWMimxCreateBBMeshSeedGroup&); // Not implemented
 };

#endif

