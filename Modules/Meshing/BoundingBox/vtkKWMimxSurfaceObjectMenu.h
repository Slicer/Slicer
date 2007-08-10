/*=========================================================================

  Module:    $RCSfile: vtkKWMimxSurfaceObjectMenu.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWMimxSurfaceObjectMenu - It is the base class for all Object menu options.
// .SECTION Description
// The class is derived from vtkKWMimxObjectMenu. It is the base class
// for all Object menu options.

#ifndef __vtkKWMimxSurfaceObjectMenu_h
#define __vtkKWMimxSurfaceObjectMenu_h

#include "vtkBoundingBox.h"

#include "vtkKWMimxObjectMenu.h"
#include "vtkKWMimxViewWindow.h"

class vtkKWMenuButtonWithLabel;

class VTK_BOUNDINGBOX_EXPORT vtkKWMimxSurfaceObjectMenu : public vtkKWMimxObjectMenu
{
public:
  static vtkKWMimxSurfaceObjectMenu* New();
  vtkTypeRevisionMacro(vtkKWMimxSurfaceObjectMenu,vtkKWMimxObjectMenu);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void Update();
  virtual void UpdateEnableState();
  void SurfaceMenuCallback();
protected:
        vtkKWMimxSurfaceObjectMenu();
        ~vtkKWMimxSurfaceObjectMenu();
        vtkKWMenuButtonWithLabel *MenuButton;

        virtual void CreateWidget();
private:
  vtkKWMimxSurfaceObjectMenu(const vtkKWMimxSurfaceObjectMenu&); // Not implemented
  void operator=(const vtkKWMimxSurfaceObjectMenu&); // Not implemented
 };

#endif

