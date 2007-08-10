/*=========================================================================

  Module:    $RCSfile: vtkKWMimxMainNotebook.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWMimxMainNotebook - a tabbed notebook of UI pages
// .SECTION Description
// The class is derived from vtkKWCompositeWidget. It contains 5 pages 1) Image
// 2) Surface 3) Bounding Box 4) F E Mesh 5) Mesh quality. Each page inturn
// contains a list of options specific to a page.

#ifndef __vtkKWMimxMainNotebook_h
#define __vtkKWMimxMainNotebook_h

#include "vtkBoundingBox.h"

#include "vtkKWCompositeWidget.h"
#include "vtkKWMimxFEMeshMenuGroup.h"
#include "vtkKWMimxViewWindow.h"
#include "vtkKWMimxSurfaceMenuGroup.h"

class vtkKWNotebook;
class vtkKWFrameWithScrollbar;
class vtkKWMenuButtonWithLabel;

class VTK_BOUNDINGBOX_EXPORT vtkKWMimxMainNotebook : public vtkKWCompositeWidget
{
public:
  static vtkKWMimxMainNotebook* New();
  vtkTypeRevisionMacro(vtkKWMimxMainNotebook,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
 // const char* GetActivePage();        // return the name of the chosen page
 // int GetActiveOption();        // return the chosen operation
  virtual void Update();
  virtual void UpdateEnableState();
  vtkSetObjectMacro(MimxViewWindow, vtkKWMimxViewWindow);
  vtkGetObjectMacro(MimxViewWindow, vtkKWMimxViewWindow);

protected:
        vtkKWMimxMainNotebook();
        ~vtkKWMimxMainNotebook();
  void SetLists();
        virtual void CreateWidget();
        vtkKWNotebook *Notebook;
        vtkKWMimxViewWindow *MimxViewWindow;
        vtkKWMimxSurfaceMenuGroup *SurfaceMenuGroup;
        vtkKWMimxFEMeshMenuGroup *FEMeshMenuGroup;
private:
  vtkKWMimxMainNotebook(const vtkKWMimxMainNotebook&); // Not implemented
  void operator=(const vtkKWMimxMainNotebook&); // Not implemented
 };

#endif

