/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxLoadMeshGroup.h,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:39 $
Version:   $Revision: 1.1.2.1 $

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
// .NAME vtkKWMimxLoadSurfaceGroup - Load a Surface from disk
// .SECTION Description
// The class is derived from vtkKWMimxGroupBase. It will load a 
// surface mesh from disk

#ifndef __vtkKWMimxLoadMeshGroup_h
#define __vtkKWMimxLoadMeshGroup_h

#include "vtkKWMimxGroupBase.h"
#include "vtkBuildingBlockWin32Header.h"

class vtkKWComboBoxWithLabel;
class vtkKWLoadSaveDialog;
class vtkEntryChangedEventCallback;
class vtkKWFrameWithLabel;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxLoadMeshGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxLoadMeshGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxLoadMeshGroup,vtkKWMimxGroupBase);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  virtual void Update();
  virtual void UpdateEnableState();
  void UpdateObjectLists();
  
protected:
        vtkKWMimxLoadMeshGroup();
        ~vtkKWMimxLoadMeshGroup();
        virtual void CreateWidget();
        
private:
  vtkKWMimxLoadMeshGroup(const vtkKWMimxLoadMeshGroup&); // Not implemented
  void operator=(const vtkKWMimxLoadMeshGroup&); // Not implemented
};

#endif

