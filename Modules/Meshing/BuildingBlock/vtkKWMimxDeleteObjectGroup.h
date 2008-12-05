/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxDeleteObjectGroup.h,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:39 $
Version:   $Revision: 1.7.4.1 $

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
// .NAME vtkKWMimxDeleteObjectGroup - Class containing GUI for deletion of objects
// .SECTION Description
// The class is used to delete all the objects created in the program. The objects
// deleted are 1) vtkMimxSurfacePolyDataActor, 2) vtkMimxUnstructuredGridActor,
// 3) vtkMimxImageActor and 4) vtkMimxMeshActor. 
//
// .SECTION See Also
// vtkMimxSurfacePolyDataActor, vtkMimxUnstructuredGridActor, vtkMimxImageActor, vtkMimxMeshActor 

#ifndef __vtkKWMimxDeleteObjectGroup_h
#define __vtkKWMimxDeleteObjectGroup_h

#include "vtkKWMimxGroupBase.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWComboBoxWithLabel;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxDeleteObjectGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxDeleteObjectGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxDeleteObjectGroup,vtkKWMimxGroupBase);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // To update the entries in the combobox containing the names of the objects
  void UpdateObjectLists();

  virtual void Update();
  virtual void UpdateEnableState();

  vtkGetObjectMacro(ObjectListComboBox, vtkKWComboBoxWithLabel);
 
  // Description:
  // Delete the object.
  int DeleteObjectApplyCallback();

  // Description:
  // Remove the GUI from the display.
  void DeleteObjectCancelCallback();

protected:
        vtkKWMimxDeleteObjectGroup();
        ~vtkKWMimxDeleteObjectGroup();
        virtual void CreateWidget();
  vtkKWComboBoxWithLabel *ObjectListComboBox;
private:
  vtkKWMimxDeleteObjectGroup(const vtkKWMimxDeleteObjectGroup&); // Not implemented
  void operator=(const vtkKWMimxDeleteObjectGroup&); // Not implemented
 };

#endif

