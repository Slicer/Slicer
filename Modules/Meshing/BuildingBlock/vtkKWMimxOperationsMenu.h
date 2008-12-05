/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxOperationsMenu.h,v $
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
// .NAME vtkKWMimxOperationsMenu - It is the base class for all operations menu options.
// .SECTION Description
// The class is derived from vtkKWCompositeWidget. It is the base class
// for all operations menu options.

#ifndef __vtkKWMimxOperationsMenu_h
#define __vtkKWMimxOperationsMenu_h

#include "vtkKWCompositeWidget.h"
#include "vtkKWMimxViewWindow.h"
#include "vtkKWMultiColumnList.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWMenuButtonWithLabel;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxOperationsMenu : public vtkKWCompositeWidget
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

