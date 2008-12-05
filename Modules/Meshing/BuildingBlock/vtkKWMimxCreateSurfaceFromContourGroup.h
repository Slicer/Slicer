/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxCreateSurfaceFromContourGroup.h,v $
Language:  C++
Date:      $Date: 2008/02/25 02:26:25 $
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
// .NAME vtkKWMimxCreateSurfaceFromContourGroup - a tabbed notebook of UI pages
// .SECTION Description
// The class is derived from vtkKWCompositeWidget. It contains 5 pages 1) Image
// 2) Surface 3) Building Block 4) F E Mesh 5) Mesh quality. Each page inturn
// contains a list of options specific to a page.

#ifndef __vtkKWMimxCreateSurfaceFromContourGroup_h
#define __vtkKWMimxCreateSurfaceFromContourGroup_h

#include "vtkKWMimxGroupBase.h"
#include "vtkKWMimxFEMeshMenuGroup.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkBuildingBlockWin32Header.h"

class vtkKWMenuButtonWithLabel;
class vtkKWEntryWithLabel;
class vtkKWFrameWithLabel;
class vtkKWPushButton;
class vtkKWFrameWithLabel;
class vtkLinkedListWrapper;
class vtkIntArray;
class vtkKWRadioButton;
class vtkKWComboBoxWithLabel;
class vtkKWRenderWidget;
class vtkMimxTraceContourWidget;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxCreateSurfaceFromContourGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxCreateSurfaceFromContourGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxCreateSurfaceFromContourGroup,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();
  vtkGetObjectMacro(MimxMainWindow, vtkKWMimxMainWindow);
  vtkSetObjectMacro(MimxMainWindow, vtkKWMimxMainWindow);

  void CreateSurfaceFromContourDoneCallback();
  void CreateSurfaceFromContourCancelCallback();
  int CreateSurfaceFromContourApplyCallback();
  void CreateSurfaceFromContourVtkInteractionCallback();
  void CreateSurfaceFromContourTraceContourCallback();
  void SelectionChangedCallback(const char*);
  void UpdateObjectLists();
protected:
        vtkKWMimxCreateSurfaceFromContourGroup();
        ~vtkKWMimxCreateSurfaceFromContourGroup();
        virtual void CreateWidget();
  vtkKWComboBoxWithLabel *ObjectListComboBox;
  vtkKWMimxMainWindow *MimxMainWindow;
  vtkKWRadioButton *TraceContourButton;
  vtkKWRadioButton *VtkInteractionButton;
  vtkKWFrameWithLabel *InteractionFrame;
  vtkMimxTraceContourWidget *TraceContourWidget;
private:
  vtkKWMimxCreateSurfaceFromContourGroup(const vtkKWMimxCreateSurfaceFromContourGroup&); // Not implemented
  void operator=(const vtkKWMimxCreateSurfaceFromContourGroup&); // Not implemented
 };

#endif

