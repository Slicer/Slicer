/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxSaveVTKSurfaceGroup.h,v $
Language:  C++
Date:      $Date: 2008/02/01 15:24:57 $
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
// .NAME vtkKWMimxSaveVTKSurfaceGroup - a tabbed notebook of UI pages
// .SECTION Description
// The class is derived from vtkKWMimxGroupBase. It contains 5 pages 1) Image
// 2) Surface 3) Building Block 4) F E Mesh 5) Mesh quality. Each page inturn
// contains a list of options specific to a page.

#ifndef __vtkKWMimxSaveVTKSurfaceGroup_h
#define __vtkKWMimxSaveVTKSurfaceGroup_h

#include "vtkKWMimxGroupBase.h"
#include "vtkKWMimxFEMeshMenuGroup.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkKWMimxSurfaceMenuGroup.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWComboBoxWithLabel;
class vtkKWLoadSaveDialog;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxSaveVTKSurfaceGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxSaveVTKSurfaceGroup* New();
vtkTypeRevisionMacro(vtkKWMimxSaveVTKSurfaceGroup,vtkKWMimxGroupBase);
void PrintSelf(ostream& os, vtkIndent indent);
virtual void Update();
virtual void UpdateEnableState();
void SaveVTKSurfaceDoneCallback();
void SaveVTKSurfaceCancelCallback();
int SaveVTKSurfaceApplyCallback();
void UpdateObjectLists();
protected:
        vtkKWMimxSaveVTKSurfaceGroup();
        ~vtkKWMimxSaveVTKSurfaceGroup();
        virtual void CreateWidget();
vtkKWComboBoxWithLabel *ObjectListComboBox;
vtkKWLoadSaveDialog *FileBrowserDialog;
private:
  vtkKWMimxSaveVTKSurfaceGroup(const vtkKWMimxSaveVTKSurfaceGroup&); // Not implemented
void operator=(const vtkKWMimxSaveVTKSurfaceGroup&); // Not implemented
 };

#endif

