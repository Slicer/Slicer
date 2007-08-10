/*=========================================================================

  Module:    $RCSfile: vtkKWMimxObjectMenu.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWMimxObjectMenu - It is the base class for all operations menu options.
// .SECTION Description
// The class is derived from vtkKWCompositeWidget. It is the base class
// for all operations menu options.

#ifndef __vtkKWMimxObjectMenu_h
#define __vtkKWMimxObjectMenu_h

#include "vtkBoundingBox.h"

#include "vtkKWCompositeWidget.h"
#include "vtkKWMimxViewWindow.h"
#include "vtkKWMimxOperationsMenu.h"
#include "vtkKWMultiColumnList.h"

class vtkKWMenuButtonWithLabel;

class VTK_BOUNDINGBOX_EXPORT vtkKWMimxObjectMenu : public vtkKWCompositeWidget
{
public:
  static vtkKWMimxObjectMenu* New();
  vtkTypeRevisionMacro(vtkKWMimxObjectMenu,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void Update();
  virtual void UpdateEnableState();
  vtkSetObjectMacro(MimxOperationsMenu, vtkKWMimxOperationsMenu);
  vtkGetObjectMacro(MimxOperationsMenu, vtkKWMimxOperationsMenu);
protected:
        vtkKWMimxObjectMenu();
        ~vtkKWMimxObjectMenu();

        virtual void CreateWidget();
        vtkKWMimxOperationsMenu *MimxOperationsMenu;
private:
  vtkKWMimxObjectMenu(const vtkKWMimxObjectMenu&); // Not implemented
  void operator=(const vtkKWMimxObjectMenu&); // Not implemented
 };

#endif

