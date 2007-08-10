/*=========================================================================

  Module:    $RCSfile: vtkKWMimxFEMeshMenuGroup.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWMimxFEMeshMenuGroup - It is the base class for all Object menu options.
// .SECTION Description
// The class is derived from vtkKWMimxMainMenuGroup. It is the base class
// for all Object menu options.

#ifndef __vtkKWMimxFEMeshMenuGroup_h
#define __vtkKWMimxFEMeshMenuGroup_h

#include "vtkBoundingBox.h"

#include "vtkKWMimxMainMenuGroup.h"

class vtkKWMimxCreateBBFromBoundsGroup;
class vtkKWMimxCreateBBMeshSeedGroup;
class vtkKWMimxEditBBGroup;
class vtkKWMimxCreateFEMeshFromBBGroup;
class vtkKWMimxViewProperties;
class vtkKWMimxSaveVTKBBGroup;
class vtkKWMimxEditBBMeshSeedGroup;
class vtkKWLoadSaveDialog;
class vtkKWMimxSaveVTKFEMeshGroup;
class vtkKWMimxDeleteObjectGroup;
class vtkKWMimxEditFEMeshLaplacianSmoothGroup;

class VTK_BOUNDINGBOX_EXPORT vtkKWMimxFEMeshMenuGroup : public vtkKWMimxMainMenuGroup
{
public:
  static vtkKWMimxFEMeshMenuGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxFEMeshMenuGroup,vtkKWMimxMainMenuGroup);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();
  void BBMenuCallback();
  void FEMeshMenuCallback();
  void LoadFEMeshCallback();
  void LoadBBCallback();
  void CreateBBCallback();
  void DeleteBBCallback();
  void CreateFEMeshCallback();
  void DeleteFEMeshCallback();
  void LoadVTKBBCallback();
  void LoadVTKFEMeshCallback();
  void CreateBBFromBoundsCallback();
  void BBMeshSeedMenuCallback();
  void CreateBBMeshSeedCallback();
  void EditBBMeshSeedCallback();
  void EditFEMeshCallback();
  void EditBBCallback();
  void CreateFEMeshFromBBCallback();
  void SaveBBCallback();
  void SaveFEMeshCallback();
  void SaveVTKBBCallback();
  void SaveVTKFEMeshCallback();
  
  vtkSetObjectMacro(SurfaceList, vtkLinkedListWrapper);
  vtkSetObjectMacro(FEMeshList, vtkLinkedListWrapper);
  void HideAllDialogBoxes();
  void SmoothLaplacianFEMeshCallback();
protected:
        vtkKWMimxFEMeshMenuGroup();
        ~vtkKWMimxFEMeshMenuGroup();

  vtkKWMimxCreateBBFromBoundsGroup *CreateBBFromBounds;
  vtkKWMimxCreateBBMeshSeedGroup *CreateBBMeshSeed;
  vtkKWMimxEditBBGroup *EditBB;
  vtkKWMimxCreateFEMeshFromBBGroup *FEMeshFromBB;
  vtkKWMimxViewProperties *BBViewProperties;
  vtkKWMimxSaveVTKBBGroup *SaveVTKBBGroup;
  vtkKWMimxEditBBMeshSeedGroup *EditBBMeshSeedGroup;
  vtkKWMimxSaveVTKFEMeshGroup *SaveVTKFEMeshGroup;
        virtual void CreateWidget();
vtkKWLoadSaveDialog *FileBrowserDialog;
vtkKWMimxDeleteObjectGroup *DeleteObjectGroup;
vtkKWMimxEditFEMeshLaplacianSmoothGroup *FEMeshLaplacianSmooth;
private:
  vtkKWMimxFEMeshMenuGroup(const vtkKWMimxFEMeshMenuGroup&); // Not implemented
  void operator=(const vtkKWMimxFEMeshMenuGroup&); // Not implemented
 };

#endif
