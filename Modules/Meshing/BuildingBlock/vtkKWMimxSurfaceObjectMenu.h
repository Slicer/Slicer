/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxSurfaceObjectMenu.h,v $
Language:  C++
Date:      $Date: 2008/02/17 00:29:48 $
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
// .NAME vtkKWMimxSurfaceObjectMenu - It is the base class for all Object menu options.
// .SECTION Description
// The class is derived from vtkKWMimxObjectMenu. It is the base class
// for all Object menu options.

#ifndef __vtkKWMimxSurfaceObjectMenu_h
#define __vtkKWMimxSurfaceObjectMenu_h

#include "vtkKWMimxObjectMenu.h"
#include "vtkKWMimxViewWindow.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWMenuButtonWithLabel;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxSurfaceObjectMenu : public vtkKWMimxObjectMenu
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

