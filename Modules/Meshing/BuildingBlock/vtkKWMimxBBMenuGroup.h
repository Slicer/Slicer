/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxBBMenuGroup.h,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:39 $
Version:   $Revision: 1.6.4.1 $

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
// .NAME vtkKWMimxBBMenuGroup - Class contains all GUI declarations for 
// building block operations.
// .SECTION Description
// The class GUI contains a menu button with all the operations entry associated
// with a building block. The entries are 1) Create, 2) Load, 3) Build/Edit,
// 4) Delete and 5) Save. The entries except 'Load' invoke a GUI associated with
// the entry.
//
// .SECTION See Also
// vtkKWMimxCreateBBFromBoundsGroup, vtkKWMimxEditBBGroup, vtkKWMimxDeleteObjectGroup,
// vtkKWMimxSaveVTKBBGroup

#ifndef __vtkKWMimxBBMenuGroup_h
#define __vtkKWMimxBBMenuGroup_h

#include "vtkKWMimxMainMenuGroup.h"

#include "vtkKWMimxMainWindow.h"
#include "vtkKWMimxSurfaceMenuGroup.h"
#include "vtkBuildingBlockWin32Header.h"

class vtkUnstructuredGrid;
class vtkKWLoadSaveDialog;
class vtkMimxUnstructuredGridActor;
class vtkKWMimxCreateBBFromBoundsGroup;
class vtkKWMimxDeleteObjectGroup;
class vtkKWMimxEditBBGroup;
class vtkKWMimxEditBBMeshSeedGroup;
class vtkKWMimxSaveVTKBBGroup;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxBBMenuGroup : public vtkKWMimxMainMenuGroup
{
public:
  
  static vtkKWMimxBBMenuGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxBBMenuGroup,vtkKWMimxMainMenuGroup);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Called when the GUI is created or redrawn
  virtual void Update();
  virtual void UpdateEnableState();

  // Description:
  // Call back for deleting building blocks
  void DeleteBBCallback();

  // Description:
  // Load VTK format building block definition file
  void LoadVTKBBCallback();

  // Description:
  // To create a building block either from bounds or from rubber band pick
  void CreateBBFromBoundsCallback();

  // Description:
  // Edit a given building block
  void EditBBCallback();

  // Description:
  // Save the chosen building block
  //void SaveBBCallback();

  // Description:
  // Procedures to store the chosen building block in VTK file format
  void SaveVTKBBCallback();
 
  // Description:
  // Set the building block linked list. This is used to populate the combobox
  // with building block object names.
  vtkSetObjectMacro(BBoxList, vtkLinkedListWrapper);

  // Description:
  // Set the surface linked list
  vtkSetObjectMacro(SurfaceList, vtkLinkedListWrapper);

  // Description:
  // Set the FE mesh linked list
  vtkSetObjectMacro(FEMeshList, vtkLinkedListWrapper);

  // Description:
  // Set the Image linked list
  vtkSetObjectMacro(ImageList, vtkLinkedListWrapper);
   
protected:
        vtkKWMimxBBMenuGroup();
        ~vtkKWMimxBBMenuGroup();

  vtkKWMimxCreateBBFromBoundsGroup *CreateBBFromBounds;
  vtkKWMimxEditBBGroup *EditBB;
  vtkKWMimxSaveVTKBBGroup *SaveVTKBBGroup;
  vtkKWMimxEditBBMeshSeedGroup *EditBBMeshSeedGroup;
  vtkKWLoadSaveDialog *FileBrowserDialog;
  vtkKWMimxDeleteObjectGroup *DeleteObjectGroup;
  virtual void CreateWidget();

  // Description:
  // Update the combobox entries
  void UpdateObjectLists();

  // Description:
  // Check the type of cells in the building block read in.
  // As of now only hexahedra are allowed.
  int CheckCellTypesInUgrid(vtkUnstructuredGrid *);

private:
  vtkKWMimxBBMenuGroup(const vtkKWMimxBBMenuGroup&); // Not implemented
  void operator=(const vtkKWMimxBBMenuGroup&); // Not implemented
 };

#endif
