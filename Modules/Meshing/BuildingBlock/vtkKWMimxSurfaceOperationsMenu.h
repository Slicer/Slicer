/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxSurfaceOperationsMenu.h,v $
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
// .NAME vtkKWMimxSurfaceOperationsMenu - It is the base class for all operations menu options.
// .SECTION Description
// The class is derived from vtkKWMimxOperationsMenu. It is the base class
// for all operations menu options.

#ifndef __vtkKWMimxSurfaceOperationsMenu_h
#define __vtkKWMimxSurfaceOperationsMenu_h

#include "vtkKWMimxOperationsMenu.h"
#include "vtkKWMimxViewWindow.h"
#include "vtkBuildingBlockWin32Header.h"


class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxSurfaceOperationsMenu : public vtkKWMimxOperationsMenu
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

