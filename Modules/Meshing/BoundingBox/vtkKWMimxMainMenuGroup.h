/*=========================================================================

  Module:    $RCSfile: vtkKWMimxMainMenuGroup.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWMimxMainMenuGroup - It is the base class for all operations menu options.
// .SECTION Description
// The class is derived from vtkKWCompositeWidget. It is the base class
// for all operations menu options.

#ifndef __vtkKWMimxMainMenuGroup_h
#define __vtkKWMimxMainMenuGroup_h

#include "vtkBoundingBox.h"

#include "vtkKWCompositeWidget.h"
#include "vtkKWMimxViewWindow.h"
//#include "vtkKWMenuButtonWithLabel.h"

#include "vtkMimxActorBase.h"

#include "vtkLinkedListWrapper.h"

class vtkKWMenuButtonWithLabel;
class vtkKWFrameWithLabel;
class vtkKWMimxViewProperties;

class vtkLinkedListWrapper;

class VTK_BOUNDINGBOX_EXPORT vtkKWMimxMainMenuGroup : public vtkKWCompositeWidget
{
public:
  static vtkKWMimxMainMenuGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxMainMenuGroup,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void Update();
  virtual void UpdateEnableState();
  vtkSetObjectMacro(MimxViewWindow, vtkKWMimxViewWindow);
  vtkGetObjectMacro(MimxViewWindow, vtkKWMimxViewWindow);
//  vtkGetObjectMacro(ObjectList, vtkLinkedListWrapper);
  vtkGetObjectMacro(SurfaceList, vtkLinkedListWrapper);
  vtkGetObjectMacro(BBoxList, vtkLinkedListWrapper);
  vtkGetObjectMacro(FEMeshList, vtkLinkedListWrapper);
  //
  virtual void SetMenuButtonsEnabled(int);
 protected:
        vtkKWMimxMainMenuGroup();
        virtual ~vtkKWMimxMainMenuGroup();

        virtual void CreateWidget();
        vtkKWMenuButtonWithLabel *ObjectMenuButton;
        vtkKWMenuButtonWithLabel *OperationMenuButton;
        vtkKWMenuButtonWithLabel *TypeMenuButton;
  vtkKWFrameWithLabel *MainFrame;
//        vtkLinkedListWrapper *ObjectList;
  vtkLinkedListWrapper *SurfaceList;
  vtkLinkedListWrapper *BBoxList;
  vtkLinkedListWrapper *FEMeshList;
  
        const char* ExtractFileName(const char*);
        vtkKWMimxViewWindow *MimxViewWindow;
        vtkKWMimxViewProperties *MimxViewProperties;
private:
  vtkKWMimxMainMenuGroup(const vtkKWMimxMainMenuGroup&); // Not implemented
  void operator=(const vtkKWMimxMainMenuGroup&); // Not implemented
 };

#endif

