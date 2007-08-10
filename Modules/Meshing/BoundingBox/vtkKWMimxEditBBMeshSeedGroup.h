/*=========================================================================

  Module:    $RCSfile: vtkKWMimxEditBBMeshSeedGroup.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWMimxEditBBMeshSeedGroup - a tabbed notebook of UI pages
// .SECTION Description
// The class is derived from vtkKWCompositeWidget. It contains 5 pages 1) Image
// 2) Surface 3) Bounding Box 4) F E Mesh 5) Mesh quality. Each page inturn
// contains a list of options specific to a page.

#ifndef __vtkKWMimxEditBBMeshSeedGroup_h
#define __vtkKWMimxEditBBMeshSeedGroup_h

#include "vtkBoundingBox.h"

#include "vtkKWMimxGroupBase.h"
#include "vtkKWMimxFEMeshMenuGroup.h"
#include "vtkKWMimxViewWindow.h"

class vtkKWMenuButtonWithLabel;
class vtkKWEntryWithLabel;
class vtkKWFrameWithLabel;
class vtkKWPushButton;

class vtkLinkedListWrapper;
class vtkMimxPlaceLocalAxesWidget;
class vtkKWRadioButton;
class vtkKWComboBoxWithLabel;
class vtkKWMimxViewWindow;

class VTK_BOUNDINGBOX_EXPORT vtkKWMimxEditBBMeshSeedGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxEditBBMeshSeedGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxEditBBMeshSeedGroup,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();
  vtkGetObjectMacro(MimxViewWindow, vtkKWMimxViewWindow);
  vtkSetObjectMacro(MimxViewWindow, vtkKWMimxViewWindow);

  void EditBBMeshSeedDoneCallback();
  void EditBBMeshSeedCancelCallback();
  void EditBBMeshSeedVtkInteractionCallback();
  void EditBBMeshSeedPickCellCallback();
  void SelectionChangedCallback(const char*);
  vtkGetObjectMacro(XMeshSeed, vtkKWEntryWithLabel);
  vtkGetObjectMacro(YMeshSeed, vtkKWEntryWithLabel);
  vtkGetObjectMacro(ZMeshSeed, vtkKWEntryWithLabel);

protected:
        vtkKWMimxEditBBMeshSeedGroup();
        ~vtkKWMimxEditBBMeshSeedGroup();
        virtual void CreateWidget();
  vtkKWComboBoxWithLabel *ObjectListComboBox;
  vtkMimxPlaceLocalAxesWidget *LocalAxesWidget;
  vtkKWMimxViewWindow *MimxViewWindow;
  vtkKWRadioButton *EditButton;
  vtkKWRadioButton *VtkInteractionButton;
  vtkKWEntryWithLabel *XMeshSeed;
  vtkKWEntryWithLabel *YMeshSeed;
  vtkKWEntryWithLabel *ZMeshSeed;
private:
  vtkKWMimxEditBBMeshSeedGroup(const vtkKWMimxEditBBMeshSeedGroup&); // Not implemented
  void operator=(const vtkKWMimxEditBBMeshSeedGroup&); // Not implemented
 };

#endif

