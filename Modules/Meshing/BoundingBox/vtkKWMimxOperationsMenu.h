/*=========================================================================

  Module:    $RCSfile: vtkKWMimxOperationsMenu.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWMimxOperationsMenu - It is the base class for all operations menu options.
// .SECTION Description
// The class is derived from vtkKWCompositeWidget. It is the base class
// for all operations menu options.

#ifndef __vtkKWMimxOperationsMenu_h
#define __vtkKWMimxOperationsMenu_h

#include "vtkBoundingBox.h"

#include "vtkKWCompositeWidget.h"
#include "vtkKWMimxViewWindow.h"
#include "vtkKWMultiColumnList.h"

class vtkKWMenuButtonWithLabel;

class VTK_BOUNDINGBOX_EXPORT vtkKWMimxOperationsMenu : public vtkKWCompositeWidget
{
public:
  static vtkKWMimxOperationsMenu* New();
  vtkTypeRevisionMacro(vtkKWMimxOperationsMenu,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void Update();
  virtual void UpdateEnableState();
  vtkSetObjectMacro(MimxViewWindow, vtkKWMimxViewWindow);
  vtkGetObjectMacro(MimxViewWindow, vtkKWMimxViewWindow);
  virtual void LoadFileCallback(){};
  virtual void ShowHideActor(int, int, int);
  virtual void SetEnabled(int x){};

protected:
        vtkKWMimxOperationsMenu();
        ~vtkKWMimxOperationsMenu();

        virtual void CreateWidget();
        vtkKWMimxViewWindow *MimxViewWindow;
        vtkKWMenuButtonWithLabel *MenuButton;
        vtkKWMultiColumnList *ObjectList;
private:
  vtkKWMimxOperationsMenu(const vtkKWMimxOperationsMenu&); // Not implemented
  void operator=(const vtkKWMimxOperationsMenu&); // Not implemented
 };

#endif

