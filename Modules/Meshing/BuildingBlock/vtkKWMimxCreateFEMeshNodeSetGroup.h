/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxCreateFEMeshNodeSetGroup.h,v $
Language:  C++
Date:      $Date: 2008/04/16 23:29:58 $
Version:   $Revision: 1.5 $

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
// .NAME vtkKWMimxCreateFEMeshNodeSetGroup - a tabbed notebook of UI pages
// .SECTION Description
// The class is derived from vtkKWMimxGroupBase. It contains 5 pages 1) Image
// 2) Surface 3) Building Block 4) F E Mesh 5) Mesh quality. Each page inturn
// contains a list of options specific to a page.

#ifndef __vtkKWMimxCreateFEMeshNodeSetGroup_h
#define __vtkKWMimxCreateFEMeshNodeSetGroup_h

#include "vtkKWMimxGroupBase.h"
#include "vtkKWMimxFEMeshMenuGroup.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkKWMimxSurfaceMenuGroup.h"
#include "vtkBuildingBlockWin32Header.h"

class vtkKWComboBoxWithLabel;
class vtkKWRadioButtonSet;
class vtkMimxSelectPointsWidget;
class vtkKWEntryWithLabel;
class vtkMimxMeshActor;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxCreateFEMeshNodeSetGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxCreateFEMeshNodeSetGroup* New();
vtkTypeRevisionMacro(vtkKWMimxCreateFEMeshNodeSetGroup,vtkKWMimxGroupBase);
void PrintSelf(ostream& os, vtkIndent indent);
virtual void Update();
virtual void UpdateEnableState();
int SelectNodesThroughCallback();
int SelectNodesSurfaceCallback();
int SelectVisibleNodesSurfaceCallback();
int SelectSingleNodeCallback();
void VTKInteractionCallback();
int CreateNodeSetApplyCallback();
void CreateNodeSetCancelCallback();
int SelectFaceNodeCallback();
void UpdateObjectLists();
protected:
        vtkKWMimxCreateFEMeshNodeSetGroup();
        ~vtkKWMimxCreateFEMeshNodeSetGroup();
        virtual void CreateWidget();
vtkKWComboBoxWithLabel *ObjectListComboBox;
vtkKWRadioButtonSet *SelectSubsetRadiobuttonSet;
vtkMimxSelectPointsWidget *SelectPointsWidget;
vtkKWEntryWithLabel *NodeSetNameEntry;
vtkMimxMeshActor* GetSelectedObject();
private:
  vtkKWMimxCreateFEMeshNodeSetGroup(const vtkKWMimxCreateFEMeshNodeSetGroup&); // Not implemented
void operator=(const vtkKWMimxCreateFEMeshNodeSetGroup&); // Not implemented
 };

#endif

