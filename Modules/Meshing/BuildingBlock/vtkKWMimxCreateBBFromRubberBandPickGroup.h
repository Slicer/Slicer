/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxCreateBBFromRubberBandPickGroup.h,v $
Language:  C++
Date:      $Date: 2008/07/14 23:02:24 $
Version:   $Revision: 1.3 $

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
// .NAME vtkKWMimxCreateBBFromRubberBandPickGroup - a tabbed notebook of UI pages
// .SECTION Description
// The class is derived from vtkKWCompositeWidget. It contains 5 pages 1) Image
// 2) Surface 3) Building Block 4) F E Mesh 5) Mesh quality. Each page inturn
// contains a list of options specific to a page.

#ifndef __vtkKWMimxCreateBBFromRubberBandPickGroup_h
#define __vtkKWMimxCreateBBFromRubberBandPickGroup_h

#include "vtkKWMimxGroupBase.h"
#include "vtkKWMimxFEMeshMenuGroup.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWComboBoxWithLabel;
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
class vtkMimxCreateBuildingBlockFromPickWidget;
//class vtkRightButtonPressEventCallback;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxCreateBBFromRubberBandPickGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxCreateBBFromRubberBandPickGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxCreateBBFromRubberBandPickGroup,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();
  vtkGetObjectMacro(MimxMainWindow, vtkKWMimxMainWindow);
  vtkSetObjectMacro(MimxMainWindow, vtkKWMimxMainWindow);

  void CreateBBFromRubberBandPickDoneCallback();
  void CreateBBFromRubberBandPickCancelCallback();
  int CreateBBFromRubberBandPickApplyCallback();
  void CreateBBFromRubberBandPickCallback(int mode);
  void SelectionChangedCallback(const char*);
  void UpdateObjectLists();
  void ExtrusionLengthChangedCallback(const char *Entry);
  
protected:
        vtkKWMimxCreateBBFromRubberBandPickGroup();
        ~vtkKWMimxCreateBBFromRubberBandPickGroup();
        virtual void CreateWidget();
        
  vtkKWFrameWithLabel *ComponentFrame;
  vtkKWComboBoxWithLabel *ObjectListComboBox;
  vtkKWMimxMainWindow *MimxMainWindow;
  //vtkKWCheckButton *CreateBuildingBlockFromPickButton;
  vtkMimxCreateBuildingBlockFromPickWidget *CreateBuildingBlockFromPickWidget;
  vtkKWEntryWithLabel *ExtrusionLengthEntry;
  //vtkRightButtonPressEventCallback *RightButtonPressCallback;
private:
  vtkKWMimxCreateBBFromRubberBandPickGroup(const vtkKWMimxCreateBBFromRubberBandPickGroup&); // Not implemented
  void operator=(const vtkKWMimxCreateBBFromRubberBandPickGroup&); // Not implemented
  
  char PreviousSurface[64];
 };

#endif

