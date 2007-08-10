/*=========================================================================

  Module:    $RCSfile: vtkKWMimxSurfaceOperationsMenu.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWMimxSurfaceOperationsMenu - It is the base class for all operations menu options.
// .SECTION Description
// The class is derived from vtkKWMimxOperationsMenu. It is the base class
// for all operations menu options.

#ifndef __vtkKWMimxSurfaceOperationsMenu_h
#define __vtkKWMimxSurfaceOperationsMenu_h

#include "vtkBoundingBox.h"

#include "vtkKWMimxOperationsMenu.h"
#include "vtkKWMimxViewWindow.h"

class VTK_BOUNDINGBOX_EXPORT vtkKWMimxSurfaceOperationsMenu : public vtkKWMimxOperationsMenu
{
public:
  static vtkKWMimxSurfaceOperationsMenu* New();
  vtkTypeRevisionMacro(vtkKWMimxSurfaceOperationsMenu,vtkKWMimxOperationsMenu);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void Update();
  virtual void UpdateEnableState();
  virtual void LoadFileCallback();
  virtual void SetEnabled(int );

protected:
        vtkKWMimxSurfaceOperationsMenu();
        ~vtkKWMimxSurfaceOperationsMenu();

        virtual void CreateWidget();
private:
  vtkKWMimxSurfaceOperationsMenu(const vtkKWMimxSurfaceOperationsMenu&); // Not implemented
  void operator=(const vtkKWMimxSurfaceOperationsMenu&); // Not implemented
 };

#endif

