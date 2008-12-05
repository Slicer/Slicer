/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxFEMeshMenuGroup.h,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.24.4.2 $

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
// .NAME vtkKWMimxFEMeshMenuGroup - It is the base class for all menu options
// for FE mesh.
// .SECTION Description
// Menu options that the class supports are Assign/Edit Mesh Seeds, Create, Load,
// Renumber Nodes/Elements, Export, Delete and Save. If a menu option is chosen,
// corresponding GUI is launched.
//
// .SECTION See Also
// vtkKWMimxCreateFEMeshFromBBGroup, vtkKWMimxDeleteObjectGroup, vtkKWMimxEditBBMeshSeedGroup,
// vtkKWMimxEditElementSetNumbersGroup, vtkKWMimxExportAbaqusFEMeshGroup, vtkKWMimxSaveVTKFEMeshGroup

#ifndef __vtkKWMimxFEMeshMenuGroup_h
#define __vtkKWMimxFEMeshMenuGroup_h

#include "vtkKWMimxMainMenuGroup.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkUnstructuredGrid;
class vtkKWMimxCreateFEMeshFromBBGroup;
class vtkKWMimxDeleteObjectGroup;
class vtkKWMimxEditBBMeshSeedGroup;
class vtkKWMimxEditElementSetNumbersGroup;
class vtkKWMimxExportAbaqusFEMeshGroup;
class vtkKWMimxSaveVTKFEMeshGroup;
class vtkLinkedListWrapper;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxFEMeshMenuGroup : public vtkKWMimxMainMenuGroup
{
public:
  static vtkKWMimxFEMeshMenuGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxFEMeshMenuGroup,vtkKWMimxMainMenuGroup);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();
  
  // Description:
  // launch GUI for creating FE mesh
  void CreateFEMeshFromBBCallback();

  // Description:
  // launch GUI for deleting FE mesh
  void DeleteFEMeshCallback();

  // Description:
  // launch GUI for loading FE mesh (VTK format)
  void LoadVTKFEMeshCallback();

  // Description:
  // Save FE mesh in VTK file format
  void SaveVTKFEMeshCallback();
 
  // Description:
  // Launch GUI for node and element renumber
  void EditElementNumbersCallback();

  // Description:
  // Launch GUI for exporting the selected FE mesh in abaqus
  // file format (.inp)
  void ExportAbaqusFEMeshCallback();

  // Description:
  // Launch GUI used in modifying the mesh seeds
  void EditBBMeshSeedCallback();
  
  // Description:
  // Setting lists of all the objects for accessing
  vtkSetObjectMacro(SurfaceList, vtkLinkedListWrapper);
  vtkSetObjectMacro(FEMeshList, vtkLinkedListWrapper);
  vtkSetObjectMacro(ImageList, vtkLinkedListWrapper);
  vtkSetObjectMacro(BBoxList, vtkLinkedListWrapper);

protected:
        vtkKWMimxFEMeshMenuGroup();
        ~vtkKWMimxFEMeshMenuGroup();
        virtual void CreateWidget();

  vtkKWMimxCreateFEMeshFromBBGroup *FEMeshFromBB;
  vtkKWMimxSaveVTKFEMeshGroup *SaveVTKFEMeshGroup;
  vtkKWMimxDeleteObjectGroup *DeleteObjectGroup;
  vtkKWMimxEditElementSetNumbersGroup *EditElementSetNumbersGroup;
  vtkKWMimxExportAbaqusFEMeshGroup *ExportAbaqusFEMeshGroup;
  vtkKWMimxEditBBMeshSeedGroup *EditBBMeshSeedGroup;
  
  void UpdateObjectLists();

private:
  vtkKWMimxFEMeshMenuGroup(const vtkKWMimxFEMeshMenuGroup&); // Not implemented
  void operator=(const vtkKWMimxFEMeshMenuGroup&); // Not implemented
};

#endif
