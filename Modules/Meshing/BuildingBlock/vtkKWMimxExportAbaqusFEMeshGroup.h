/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxExportAbaqusFEMeshGroup.h,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:39 $
Version:   $Revision: 1.11.4.1 $

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
// .NAME vtkKWMimxExportAbaqusFEMeshGroup - To export selected FEMesh as an .inp 
// (abaqus) file
//
// .SECTION Description
// Class contains GUI to export a selected mesh as an abaqus (.inp) file.
//
// .SECTION See Also
// vtkMimxAbaqusFileWriter

#ifndef __vtkKWMimxExportAbaqusFEMeshGroup_h
#define __vtkKWMimxExportAbaqusFEMeshGroup_h

#include "vtkKWMimxGroupBase.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWComboBoxWithLabel;
class vtkKWEntryWithLabel;
class vtkKWFrameWithLabel;
class vtkKWLoadSaveDialog;
class vtkKWTextWithScrollbarsWithLabel;


class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxExportAbaqusFEMeshGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxExportAbaqusFEMeshGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxExportAbaqusFEMeshGroup,vtkKWMimxGroupBase);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();
  void UpdateObjectLists();

  // Description:
  // Hide the GUI
  void ExportAbaqusFEMeshCancelCallback();

  // Description:
  // Export the chosen FE mesh as an abaqus (.inp) file
  int ExportAbaqusFEMeshApplyCallback();
protected:
        vtkKWMimxExportAbaqusFEMeshGroup();
        ~vtkKWMimxExportAbaqusFEMeshGroup();
        virtual void CreateWidget();
        
        vtkKWFrameWithLabel *ComponentFrame;
  vtkKWComboBoxWithLabel *ObjectListComboBox;
  vtkKWLoadSaveDialog *FileBrowserDialog;
  vtkKWTextWithScrollbarsWithLabel *HeaderInformationText;
  vtkKWEntryWithLabel *UserNameEntry;
  
private:
  vtkKWMimxExportAbaqusFEMeshGroup(const vtkKWMimxExportAbaqusFEMeshGroup&); // Not implemented
  void operator=(const vtkKWMimxExportAbaqusFEMeshGroup&); // Not implemented
};

#endif

