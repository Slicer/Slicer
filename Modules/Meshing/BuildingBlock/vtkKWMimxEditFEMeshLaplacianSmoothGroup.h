/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxEditFEMeshLaplacianSmoothGroup.h,v $
Language:  C++
Date:      $Date: 2008/07/30 02:19:22 $
Version:   $Revision: 1.10 $

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
// .NAME vtkKWMimxEditFEMeshLaplacianSmoothGroup - a tabbed notebook of UI pages
// .SECTION Description
// The class is derived from vtkKWMimxGroupBase. It contains 5 pages 1) Image
// 2) Surface 3) Building Block 4) F E Mesh 5) Mesh quality. Each page inturn
// contains a list of options specific to a page.

#ifndef __vtkKWMimxEditFEMeshLaplacianSmoothGroup_h
#define __vtkKWMimxEditFEMeshLaplacianSmoothGroup_h

#include "vtkKWMimxGroupBase.h"
#include "vtkKWMimxFEMeshMenuGroup.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkKWMimxSurfaceMenuGroup.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWComboBoxWithLabel;
class vtkKWEntryWithLabel;
class vtkKWCheckButton;
class vtkKWCheckButtonWithLabel;
class vtkIntArray;
class vtkKWMenuButtonWithLabel;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxEditFEMeshLaplacianSmoothGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxEditFEMeshLaplacianSmoothGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxEditFEMeshLaplacianSmoothGroup,vtkKWMimxGroupBase);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();
  void EditFEMeshLaplacianSmoothDoneCallback();
  void EditFEMeshLaplacianSmoothCancelCallback();
  int EditFEMeshLaplacianSmoothApplyCallback();
  void UpdateObjectLists();
  void RecalculateCheckButtonCallback(int State);
  void InterpolationModeCallback();
protected:
        vtkKWMimxEditFEMeshLaplacianSmoothGroup();
        ~vtkKWMimxEditFEMeshLaplacianSmoothGroup();
        virtual void CreateWidget();
  vtkKWComboBoxWithLabel *FEMeshListComboBox;
  vtkKWComboBoxWithLabel *BBListComboBox;
  vtkKWComboBoxWithLabel *SurfaceListComboBox;
  vtkKWEntryWithLabel *SmoothIterations;
  vtkKWEntryWithLabel *RecalculateIterations;
  vtkKWCheckButton *RecalculateCheckButton;
  vtkKWCheckButtonWithLabel *RecalculateCheckButtonBB;
  vtkIntArray *OriginalPosition;
  vtkKWFrameWithLabel *EntryFrame;
  vtkKWFrameWithLabel *ComponentFrame;
  vtkKWMenuButtonWithLabel *InterpolationMenuButton;
private:
  vtkKWMimxEditFEMeshLaplacianSmoothGroup(const vtkKWMimxEditFEMeshLaplacianSmoothGroup&); // Not implemented
  void operator=(const vtkKWMimxEditFEMeshLaplacianSmoothGroup&); // Not implemented
 };

#endif

