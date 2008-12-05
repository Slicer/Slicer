/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxBoundaryConditionsMenuGroup.h,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:39 $
Version:   $Revision: 1.4.4.1 $

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
// .NAME vtkKWMimxBoundaryConditionsMenuGroup - Class for procedures associated with
// boundary condition assignment and storage
// .SECTION Description
// Class contains menu button with the following entries 1) Assign boundary conditions
// 2) Save 3) Export Abaqus. Assign Boundary conditions pops-up an instance of 
// vtkKWMimxAssignBoundaryConditionsGroup, Save pops-up an instance of 
// vtkKWMimxSaveVTKFEMeshGroup and Export Abaqus pop-up an instance of 
// vtkKWMimxExportAbaqusFEMeshGroup.
//
// .SECTION See Also
// vtkKWMimxAssignBoundaryConditionsGroup, vtkKWMimxSaveVTKFEMeshGroup, vtkKWMimxExportAbaqusFEMeshGroup

#ifndef __vtkKWMimxBoundaryConditionsMenuGroup_h
#define __vtkKWMimxBoundaryConditionsMenuGroup_h

#include "vtkKWMimxMainMenuGroup.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWMimxAssignBoundaryConditionsGroup;
class vtkKWMimxExportAbaqusFEMeshGroup;
class vtkKWMimxSaveVTKFEMeshGroup;


class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxBoundaryConditionsMenuGroup : public vtkKWMimxMainMenuGroup
{
public:
  static vtkKWMimxBoundaryConditionsMenuGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxBoundaryConditionsMenuGroup,vtkKWMimxMainMenuGroup);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();

  // Description:
  // Access to list of FE mesh
  vtkSetObjectMacro(FEMeshList, vtkLinkedListWrapper);

  // Description:
  // Update the FE mesh list entries
  void UpdateObjectLists();

  // Description:
  // Draw the GUI associated with boundary condition assignment
  void AssignBoundaryConditionsCallback();

  // Description:
  // Save the mesh chosen in VTK file format
  void SaveVTKFEMeshCallback();

  // Description:
  // Export the chosen mesh in abaqus file format
  void ExportAbaqusFEMeshCallback();
  
protected:

        vtkKWMimxBoundaryConditionsMenuGroup();
        ~vtkKWMimxBoundaryConditionsMenuGroup();
        virtual void CreateWidget();
        // Declarations for GUI instances needed
        vtkKWMimxAssignBoundaryConditionsGroup *AssignBoundaryConditionsGroup;
        vtkKWMimxSaveVTKFEMeshGroup *SaveVTKFEMeshGroup;
    vtkKWMimxExportAbaqusFEMeshGroup *ExportAbaqusFEMeshGroup;
  
private:
  vtkKWMimxBoundaryConditionsMenuGroup(const vtkKWMimxBoundaryConditionsMenuGroup&); // Not implemented
  void operator=(const vtkKWMimxBoundaryConditionsMenuGroup&); // Not implemented

};

#endif
