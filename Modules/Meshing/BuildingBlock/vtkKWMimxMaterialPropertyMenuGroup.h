/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxMaterialPropertyMenuGroup.h,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.3.4.2 $

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
// .NAME vtkKWMimxMaterialPropertyMenuGroup - Class containing options
// associated with material property assignment and display.
//
// .SECTION Description
// Class contains a menu item whose entries are 1) User-Defined - to assign
// user defined material properties, 2)Image-Based - to assign image based
// material properties, 3) Display material properties, 4) Export Abaqus -File
// 5) Save - Save the FEMesh as a VTK file.

#ifndef __vtkKWMimxMaterialPropertyMenuGroup_h
#define __vtkKWMimxMaterialPropertyMenuGroup_h

#include "vtkKWMimxMainMenuGroup.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWMimxApplyFEMeshMaterialPropertiesFromImageGroup;
class vtkKWMimxConstMatPropElSetGroup;
class vtkKWMimxDisplayMatPropGroup;
class vtkKWMimxExportAbaqusFEMeshGroup;
class vtkKWMimxSaveVTKFEMeshGroup;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxMaterialPropertyMenuGroup : public vtkKWMimxMainMenuGroup
{
public:
  static vtkKWMimxMaterialPropertyMenuGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxMaterialPropertyMenuGroup,vtkKWMimxMainMenuGroup);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  virtual void Update();
  virtual void UpdateEnableState();  
  void UpdateObjectLists();
  
  vtkSetObjectMacro(FEMeshList, vtkLinkedListWrapper);
  vtkSetObjectMacro(ImageList, vtkLinkedListWrapper);
  
  // Description:
  // Callback for user defined material property for a chosen element set belonging
  // to a given mesh
  void AssignMaterialPropertiesElementSetCallback();

  // Description:
  // Callback for image based material property assignment for a given element set
  void AssignImageBasedMaterialPropertiesCallback();

  // Description:
  // Launch the GUI for the display of the material property.
  void DisplayMaterialPropertyCallback();

  // Description:
  // Save the FE mesh as a VTK file
  void SaveVTKFEMeshCallback();

  // Description:
  // Export the FE mesh in ABAQUS format
  void ExportAbaqusFEMeshCallback();
  
protected:
        vtkKWMimxMaterialPropertyMenuGroup();
        ~vtkKWMimxMaterialPropertyMenuGroup();
        virtual void CreateWidget();
  vtkKWMimxConstMatPropElSetGroup *ConstMatPropElSetGroup;
  vtkKWMimxApplyFEMeshMaterialPropertiesFromImageGroup *ImageMatPropGroup;
  vtkKWMimxDisplayMatPropGroup *DisplayMaterialPropertyGroup;
  vtkKWMimxSaveVTKFEMeshGroup *SaveVTKFEMeshGroup;
  vtkKWMimxExportAbaqusFEMeshGroup *ExportAbaqusFEMeshGroup;
  
private:
  vtkKWMimxMaterialPropertyMenuGroup(const vtkKWMimxMaterialPropertyMenuGroup&); // Not implemented
  void operator=(const vtkKWMimxMaterialPropertyMenuGroup&); // Not implemented

};

#endif
