/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxImportModelSurfaceGroup.h,v $
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
// .NAME vtkKWMimxImportModelSurfaceGroup - Load a Surface from disk
// .SECTION Description
// The class is derived from vtkKWMimxGroupBase. It will load a 
// surface mesh from disk

#ifndef __vtkKWMimxImportModelSurfaceGroup_h
#define __vtkKWMimxImportModelSurfaceGroup_h

#include "vtkKWMimxGroupBase.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWComboBoxWithLabel;
class vtkKWLoadSaveDialog;
class vtkEntryChangedEventCallback;
class vtkKWFrameWithLabel;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxImportModelSurfaceGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxImportModelSurfaceGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxImportModelSurfaceGroup,vtkKWMimxGroupBase);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  virtual void Update();
  virtual void UpdateEnableState();
  void UpdateObjectLists();
  void ImportToSurfaceCancelCallback();
  int SelectModelToImportToSurfaceCallback();
  
protected:
        vtkKWMimxImportModelSurfaceGroup();
        ~vtkKWMimxImportModelSurfaceGroup();
        virtual void CreateWidget();

        virtual void FillModelComboBox();
        
private:
  vtkKWMimxImportModelSurfaceGroup(const vtkKWMimxImportModelSurfaceGroup&); // Not implemented
  void operator=(const vtkKWMimxImportModelSurfaceGroup&); // Not implemented

  vtkKWComboBoxWithLabel* ModelListComboBox;

 };

#endif

