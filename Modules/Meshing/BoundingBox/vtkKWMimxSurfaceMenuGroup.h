/*=========================================================================

  Module:    $RCSfile: vtkKWMimxSurfaceMenuGroup.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWMimxSurfaceMenuGroup - It is the base class for all Object menu options.
// .SECTION Description
// The class is derived from vtkKWMimxMainMenuGroup. It is the base class
// for all Object menu options.

#ifndef __vtkKWMimxSurfaceMenuGroup_h
#define __vtkKWMimxSurfaceMenuGroup_h

#include "vtkBoundingBox.h"

#include "vtkKWMimxMainMenuGroup.h"

class vtkKWFileBrowserDialog;
class vtkKWMimxSaveSTLSurfaceGroup;
class vtkKWMimxSaveVTKSurfaceGroup;
class vtkKWMimxDeleteObjectGroup;

class VTK_BOUNDINGBOX_EXPORT vtkKWMimxSurfaceMenuGroup : public vtkKWMimxMainMenuGroup
{
public:
  static vtkKWMimxSurfaceMenuGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxSurfaceMenuGroup,vtkKWMimxMainMenuGroup);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();
  void SurfaceMenuCallback();
  void LoadSurfaceCallback();
  void LoadSTLSurfaceCallback();
  void LoadVTKSurfaceCallback();
  void SaveSurfaceCallback();
  void SaveSTLSurfaceCallback();
  void SaveVTKSurfaceCallback();
  void DeleteSurfaceCallback();

  vtkSetObjectMacro(BBoxList, vtkLinkedListWrapper);
  vtkSetObjectMacro(FEMeshList, vtkLinkedListWrapper);

protected:
        vtkKWMimxSurfaceMenuGroup();
        ~vtkKWMimxSurfaceMenuGroup();
        vtkKWFileBrowserDialog *FileBrowserDialog;
        virtual void CreateWidget();
        vtkKWMimxSaveSTLSurfaceGroup *SaveSTLGroup;
        vtkKWMimxSaveVTKSurfaceGroup *SaveVTKGroup;
        vtkKWMimxDeleteObjectGroup *DeleteObjectGroup;
private:
  vtkKWMimxSurfaceMenuGroup(const vtkKWMimxSurfaceMenuGroup&); // Not implemented
  void operator=(const vtkKWMimxSurfaceMenuGroup&); // Not implemented
 };

#endif
