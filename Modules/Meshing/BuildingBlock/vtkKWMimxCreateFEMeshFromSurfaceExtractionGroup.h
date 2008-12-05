/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxCreateFEMeshFromSurfaceExtractionGroup.h,v $
Language:  C++
Date:      $Date: 2008/05/31 19:19:37 $
Version:   $Revision: 1.6 $

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
// .NAME vtkKWMimxCreateFEMeshFromSurfaceExtractionGroup - a tabbed notebook of UI pages
// .SECTION Description
// The class is derived from vtkKWCompositeWidget. It contains 5 pages 1) Image
// 2) Surface 3) Building Block 4) F E Mesh 5) Mesh quality. Each page inturn
// contains a list of options specific to a page.

#ifndef __vtkKWMimxCreateFEMeshFromSurfaceExtractionGroup_h
#define __vtkKWMimxCreateFEMeshFromSurfaceExtractionGroup_h

#include "vtkKWMimxGroupBase.h"
#include "vtkKWMimxFEMeshMenuGroup.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWMenuButtonWithLabel;
class vtkKWCheckButtonWithLabel;
class vtkKWEntryWithLabel;
class vtkKWFrameWithLabel;
class vtkKWPushButton;
class vtkLinkedListWrapper;
class vtkIntArray;
class vtkKWComboBoxWithLabel;
class vtkKWRenderWidget;
class vtkActor;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxCreateFEMeshFromSurfaceExtractionGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxCreateFEMeshFromSurfaceExtractionGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxCreateFEMeshFromSurfaceExtractionGroup,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();
  vtkGetObjectMacro(MimxMainWindow, vtkKWMimxMainWindow);
  vtkSetObjectMacro(MimxMainWindow, vtkKWMimxMainWindow);

  void CreateFEMeshFromSurfaceExtractionDoneCallback();
  void CreateFEMeshFromSurfaceExtractionCancelCallback();
  int CreateFEMeshFromSurfaceExtractionApplyCallback();

  void UpdateObjectLists();
protected:
        vtkKWMimxCreateFEMeshFromSurfaceExtractionGroup();
        ~vtkKWMimxCreateFEMeshFromSurfaceExtractionGroup();
        virtual void CreateWidget();
        
        vtkKWFrameWithLabel *ComponentFrame;
  vtkKWComboBoxWithLabel *ObjectListComboBox;
 private:
  vtkKWMimxCreateFEMeshFromSurfaceExtractionGroup(const vtkKWMimxCreateFEMeshFromSurfaceExtractionGroup&); // Not implemented
  void operator=(const vtkKWMimxCreateFEMeshFromSurfaceExtractionGroup&); // Not implemented
 };

#endif

