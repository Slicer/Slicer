/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxSurfaceMenuGroup.h,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.13.4.2 $

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
// .NAME vtkKWMimxSurfaceMenuGroup - Base class for all surface related operations.
// .SECTION Description
// The class forms the 'Surface' page in the vtkMimxMainNotebook. The operations it contains
// are Load, Save and Delete

#ifndef __vtkKWMimxSurfaceMenuGroup_h
#define __vtkKWMimxSurfaceMenuGroup_h

#include "vtkKWMimxMainMenuGroup.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWMimxDeleteObjectGroup;
class vtkKWMimxSaveSTLSurfaceGroup;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxSurfaceMenuGroup : public vtkKWMimxMainMenuGroup
{
public:
  static vtkKWMimxSurfaceMenuGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxSurfaceMenuGroup,vtkKWMimxMainMenuGroup);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();
  
  // Description:
  // Load the surface file
  void LoadSurfaceCallback();
  
  // Description:
  // Save the surface file
  void SaveSurfaceCallback();
  
  // Description:
  // Delete the surface
  void DeleteSurfaceCallback();
  
  // Description:
  // Access to the lists of the objects used
  vtkSetObjectMacro(BBoxList, vtkLinkedListWrapper);
  vtkSetObjectMacro(FEMeshList, vtkLinkedListWrapper);
  
  void UpdateObjectLists();
  
  // slicer integration - add callback for model import
  void ImportModelSurfaceCallback();

protected:
        vtkKWMimxSurfaceMenuGroup();
        ~vtkKWMimxSurfaceMenuGroup();
        
        virtual void CreateWidget();
        vtkKWMimxSaveSTLSurfaceGroup *SaveSTLGroup;
        vtkKWMimxDeleteObjectGroup *DeleteObjectGroup;



private:
  vtkKWMimxSurfaceMenuGroup(const vtkKWMimxSurfaceMenuGroup&); // Not implemented
  void operator=(const vtkKWMimxSurfaceMenuGroup&); // Not implemented
};

#endif
