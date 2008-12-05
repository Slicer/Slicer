/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxViewWindow.h,v $
Language:  C++
Date:      $Date: 2008/02/17 00:29:48 $
Version:   $Revision: 1.8 $

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

#ifndef __vtkKWMimxViewWindow_h
#define __vtkKWMimxViewWindow_h

#include "vtkKWWindowBase.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkCallbackCommand;
class vtkRenderer;
class vtkPVAxesActor;
class vtkKWRenderWidget;
class vtkKWChangeColorButton;
class vtkMimxErrorCallback;

//class vtkMimxUnstructuredGridWidget;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxViewWindow : public vtkKWWindowBase
{
public:
  static vtkKWMimxViewWindow* New();
  vtkTypeRevisionMacro(vtkKWMimxViewWindow,vtkKWWindowBase);
  vtkGetObjectMacro(RenderWidget, vtkKWRenderWidget);
  vtkGetObjectMacro(ErrorCallback, vtkMimxErrorCallback);
  vtkKWRenderWidget *RenderWidget;
  vtkRenderer *AxesRenderer;
  vtkPVAxesActor *PVAxesActor;
  vtkCallbackCommand *CallbackCommand;
  void ViewWindowProperties();
//  vtkMimxUnstructuredGridWidget *UnstructuredGridWidget;
protected:
  vtkKWMimxViewWindow();
  ~vtkKWMimxViewWindow();
   virtual void CreateWidget();
   vtkKWChangeColorButton *ChangeColorButton;
   vtkMimxErrorCallback *ErrorCallback;
   //void Keypress();
private:
  vtkKWMimxViewWindow(const vtkKWMimxViewWindow&);   // Not implemented.
  void operator=(const vtkKWMimxViewWindow&);  // Not implemented.
};
void updateAxis(vtkObject* caller, unsigned long , void* arg, void* );

#endif
