/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxSaveVTKBBGroup.h,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:40 $
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
// .NAME vtkKWMimxSaveVTKBBGroup - GUI for saving building blocks as unstructured grids.
// .SECTION Description
// The class contains list of building blocks that can be savedin VTK unstructured grid
// file format

#ifndef __vtkKWMimxSaveVTKBBGroup_h
#define __vtkKWMimxSaveVTKBBGroup_h

#include "vtkKWMimxGroupBase.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWComboBoxWithLabel;
class vtkKWLoadSaveDialog;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxSaveVTKBBGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxSaveVTKBBGroup* New();
        vtkTypeRevisionMacro(vtkKWMimxSaveVTKBBGroup,vtkKWMimxGroupBase);
        void PrintSelf(ostream& os, vtkIndent indent);
        virtual void Update();
        virtual void UpdateEnableState();

        // Description:
        // Call backs for push buttons
        void SaveVTKBBDoneCallback();
        void SaveVTKBBCancelCallback();
        int SaveVTKBBApplyCallback();
        void UpdateObjectLists();
protected:
        vtkKWMimxSaveVTKBBGroup();
        ~vtkKWMimxSaveVTKBBGroup();
        virtual void CreateWidget();
        vtkKWComboBoxWithLabel *ObjectListComboBox;
        vtkKWLoadSaveDialog *FileBrowserDialog;
private:
  vtkKWMimxSaveVTKBBGroup(const vtkKWMimxSaveVTKBBGroup&); // Not implemented
void operator=(const vtkKWMimxSaveVTKBBGroup&); // Not implemented
 };

#endif

