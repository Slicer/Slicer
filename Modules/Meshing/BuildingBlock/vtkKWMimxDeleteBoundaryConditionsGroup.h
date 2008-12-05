/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxDeleteBoundaryConditionsGroup.h,v $
Language:  C++
Date:      $Date: 2008/04/25 21:31:09 $
Version:   $Revision: 1.2 $

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
// .NAME vtkKWMimxDeleteBoundaryConditionsGroup - a taFEMeshed notebook of UI pages
// .SECTION Description
// The class is derived from vtkKWMimxGroupBase. It contains 5 pages 1) Image
// 2) Surface 3) Building Block 4) F E Mesh 5) Mesh quality. Each page inturn
// contains a list of options specific to a page.

#ifndef __vtkKWMimxDeleteBoundaryConditionsGroup_h
#define __vtkKWMimxDeleteBoundaryConditionsGroup_h

#include "vtkKWMimxGroupBase.h"
#include "vtkKWMimxFEMeshMenuGroup.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkKWMimxSurfaceMenuGroup.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWComboBoxWithLabel;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxDeleteBoundaryConditionsGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxDeleteBoundaryConditionsGroup* New();
vtkTypeRevisionMacro(vtkKWMimxDeleteBoundaryConditionsGroup,vtkKWMimxGroupBase);
void PrintSelf(ostream& os, vtkIndent indent);
virtual void Update();
virtual void UpdateEnableState();
void DeleteBoundaryConditionDoneCallback();
void DeleteBoundaryConditionCancelCallback();
int DeleteBoundaryConditionApplyCallback();
void UpdateObjectLists();
void SelectionChangedCallback(const char *Selection);
protected:
        vtkKWMimxDeleteBoundaryConditionsGroup();
        ~vtkKWMimxDeleteBoundaryConditionsGroup();
        virtual void CreateWidget();
vtkKWComboBoxWithLabel *ObjectListComboBox;
vtkKWComboBoxWithLabel *StepNumberComboBox;
int CancelStatus;
private:
  vtkKWMimxDeleteBoundaryConditionsGroup(const vtkKWMimxDeleteBoundaryConditionsGroup&); // Not implemented
void operator=(const vtkKWMimxDeleteBoundaryConditionsGroup&); // Not implemented
 };

#endif

