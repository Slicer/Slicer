/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxObjectMenu.h,v $
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
// .NAME vtkKWMimxObjectMenu - It is the base class for all operations menu options.
// .SECTION Description
// The class is derived from vtkKWCompositeWidget. It is the base class
// for all operations menu options.

#ifndef __vtkKWMimxObjectMenu_h
#define __vtkKWMimxObjectMenu_h

#include "vtkKWCompositeWidget.h"
#include "vtkKWMimxViewWindow.h"
#include "vtkKWMimxOperationsMenu.h"
#include "vtkKWMultiColumnList.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWMenuButtonWithLabel;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxObjectMenu : public vtkKWCompositeWidget
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

