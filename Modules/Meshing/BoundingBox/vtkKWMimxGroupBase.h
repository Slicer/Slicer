/*=========================================================================

  Module:    $RCSfile: vtkKWMimxGroupBase.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWMimxGroupBase - a tabbed notebook of UI pages
// .SECTION Description
// The class is derived from vtkKWCompositeWidget. It contains 5 pages 1) Image
// 2) Surface 3) Bounding Box 4) F E Mesh 5) Mesh quality. Each page inturn
// contains a list of options specific to a page.

#ifndef __vtkKWMimxGroupBase_h
#define __vtkKWMimxGroupBase_h

#include "vtkBoundingBox.h"

#include "vtkKWCompositeWidget.h"
#include "vtkKWMimxFEMeshMenuGroup.h"
#include "vtkKWMimxViewWindow.h"
#include "vtkKWMimxSurfaceMenuGroup.h"

#include "vtkKWMimxViewProperties.h"

class vtkKWMenuButtonWithLabel;
class vtkKWFrameWithLabel;
class vtkKWPushButton;

class vtkLinkedListWrapper;
class vtkKWComboBoxWithLabel;
class vtkKWMimxViewWindow;
class vtkKWMimxMainMenuGroup;

class VTK_BOUNDINGBOX_EXPORT vtkKWMimxGroupBase : public vtkKWCompositeWidget
{
public:
  static vtkKWMimxGroupBase* New();
  vtkTypeRevisionMacro(vtkKWMimxGroupBase,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();
  vtkSetObjectMacro(SurfaceList, vtkLinkedListWrapper);
  vtkSetObjectMacro(BBoxList, vtkLinkedListWrapper);
  vtkSetObjectMacro(FEMeshList, vtkLinkedListWrapper);
  vtkGetObjectMacro(MimxViewWindow, vtkKWMimxViewWindow);
  vtkSetObjectMacro(MimxViewWindow, vtkKWMimxViewWindow);
  vtkGetObjectMacro(MainFrame, vtkKWFrameWithLabel);
  vtkSetObjectMacro(ViewProperties, vtkKWMimxViewProperties);
  vtkSetObjectMacro(MenuGroup, vtkKWMimxMainMenuGroup);

  vtkIdType Count;  // to keep track of number of objects created during runtime
protected:
        vtkKWMimxGroupBase();
        virtual ~vtkKWMimxGroupBase();
        virtual void CreateWidget();
  vtkLinkedListWrapper *SurfaceList;
  vtkLinkedListWrapper *BBoxList;
  vtkLinkedListWrapper *FEMeshList;
  vtkKWPushButton *DoneButton;
  vtkKWPushButton *CancelButton;
  vtkKWFrameWithLabel *MainFrame;

  vtkKWMimxViewWindow *MimxViewWindow;
  vtkKWMimxViewProperties *ViewProperties;

  vtkKWMimxMainMenuGroup *MenuGroup;
private:
  vtkKWMimxGroupBase(const vtkKWMimxGroupBase&); // Not implemented
  void operator=(const vtkKWMimxGroupBase&); // Not implemented
 };

#endif

