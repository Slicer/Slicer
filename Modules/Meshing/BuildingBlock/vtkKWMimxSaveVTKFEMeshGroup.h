/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxSaveVTKFEMeshGroup.h,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:40 $
Version:   $Revision: 1.8.4.1 $

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
// .NAME vtkKWMimxSaveVTKFEMeshGroup - GUI for saving FE mesh in vtkUnstructuredGrid file format
// .SECTION Description
// Class contains the list of FE mesh objects to be saved in vtk file format.

#ifndef __vtkKWMimxSaveVTKFEMeshGroup_h
#define __vtkKWMimxSaveVTKFEMeshGroup_h

#include "vtkKWMimxGroupBase.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWComboBoxWithLabel;
class vtkKWFrameWithLabel;
class vtkKWLoadSaveDialog;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxSaveVTKFEMeshGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxSaveVTKFEMeshGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxSaveVTKFEMeshGroup,vtkKWMimxGroupBase);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();

  // Description:
  // Call backs for the push bottons
  void SaveVTKFEMeshCancelCallback();
  int SaveVTKFEMeshApplyCallback();
  void UpdateObjectLists();
  
protected:
        vtkKWMimxSaveVTKFEMeshGroup();
        ~vtkKWMimxSaveVTKFEMeshGroup();
        virtual void CreateWidget();
        
  vtkKWFrameWithLabel *ComponentFrame;
  vtkKWComboBoxWithLabel *ObjectListComboBox;
  vtkKWLoadSaveDialog *FileBrowserDialog;
private:
  vtkKWMimxSaveVTKFEMeshGroup(const vtkKWMimxSaveVTKFEMeshGroup&); // Not implemented
void operator=(const vtkKWMimxSaveVTKFEMeshGroup&); // Not implemented
 };

#endif

