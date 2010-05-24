/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxImageMenuGroup.h,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.9.4.2 $

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
// .NAME vtkKWMimxImageMenuGroup - It is the base class for all Image menu options.
//
// .SECTION Description
// The class is derived from vtkKWMimxMainMenuGroup. It is the base class
// for all Object menu options.

#ifndef __vtkKWMimxImageMenuGroup_h
#define __vtkKWMimxImageMenuGroup_h

#include "vtkKWMimxMainMenuGroup.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWFileBrowserDialog;
class vtkKWMimxDeleteObjectGroup;
class vtkKWMimxImageViewProperties;
class vtkFocalPlaneContourRepresentation;
class vtkContourWidget;
class vtkImageActor;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxImageMenuGroup : public vtkKWMimxMainMenuGroup
{
public:
  static vtkKWMimxImageMenuGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxImageMenuGroup,vtkKWMimxMainMenuGroup);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();
  
  void LoadImageCallback();
  void ImportImageCallback();
  
  // added for MRML import from Slicer
  void ImportMRMLVolumeCallback();

  void DeleteImageCallback();
  vtkSetObjectMacro(FEMeshList, vtkLinkedListWrapper);
  
protected:
        vtkKWMimxImageMenuGroup();
        ~vtkKWMimxImageMenuGroup();
        vtkKWFileBrowserDialog *FileBrowserDialog;
        virtual void CreateWidget();
        vtkKWMimxImageViewProperties *MimxImageViewProperties;
  vtkKWMimxDeleteObjectGroup *DeleteObjectGroup;
  
private:
  vtkKWMimxImageMenuGroup(const vtkKWMimxImageMenuGroup&); // Not implemented
  void operator=(const vtkKWMimxImageMenuGroup&); // Not implemented
 };

#endif
