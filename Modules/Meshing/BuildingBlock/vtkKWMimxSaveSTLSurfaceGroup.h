/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxSaveSTLSurfaceGroup.h,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:40 $
Version:   $Revision: 1.10.4.1 $

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
// .NAME vtkKWMimxSaveSTLSurfaceGroup - GUI for saving surfaces.
// .SECTION Description
// The class contains a list of surfaces that could be saved in either 
// STL format or vtkPolyData format.

#ifndef __vtkKWMimxSaveSTLSurfaceGroup_h
#define __vtkKWMimxSaveSTLSurfaceGroup_h

#include "vtkKWMimxGroupBase.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWComboBoxWithLabel;
class vtkKWFrameWithLabel;
class vtkKWLoadSaveDialog;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxSaveSTLSurfaceGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxSaveSTLSurfaceGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxSaveSTLSurfaceGroup,vtkKWMimxGroupBase);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();

  // Description:
  // Callbacks for push buttons
  void SaveSTLSurfaceDoneCallback();
  int SaveSTLSurfaceApplyCallback();
  void SaveSTLSurfaceCancelCallback();
  void UpdateObjectLists();
protected:
        vtkKWMimxSaveSTLSurfaceGroup();
        ~vtkKWMimxSaveSTLSurfaceGroup();
        virtual void CreateWidget();
        
  vtkKWFrameWithLabel *ComponentFrame;
  vtkKWComboBoxWithLabel *ObjectListComboBox;
  vtkKWLoadSaveDialog *FileBrowserDialog;
private:
  vtkKWMimxSaveSTLSurfaceGroup(const vtkKWMimxSaveSTLSurfaceGroup&); // Not implemented
  void operator=(const vtkKWMimxSaveSTLSurfaceGroup&); // Not implemented
 };

#endif

