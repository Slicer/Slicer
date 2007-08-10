/*=========================================================================

  Module:    $RCSfile: vtkKWMimxEditFEMeshLaplacianSmoothGroup.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWMimxEditFEMeshLaplacianSmoothGroup - a tabbed notebook of UI pages
// .SECTION Description
// The class is derived from vtkKWMimxGroupBase. It contains 5 pages 1) Image
// 2) Surface 3) Bounding Box 4) F E Mesh 5) Mesh quality. Each page inturn
// contains a list of options specific to a page.

#ifndef __vtkKWMimxEditFEMeshLaplacianSmoothGroup_h
#define __vtkKWMimxEditFEMeshLaplacianSmoothGroup_h

#include "vtkBoundingBox.h"

#include "vtkKWMimxGroupBase.h"
#include "vtkKWMimxFEMeshMenuGroup.h"
#include "vtkKWMimxViewWindow.h"
#include "vtkKWMimxSurfaceMenuGroup.h"

class vtkKWComboBoxWithLabel;
class vtkKWEntryWithLabel;
class vtkKWCheckButton;

class VTK_BOUNDINGBOX_EXPORT vtkKWMimxEditFEMeshLaplacianSmoothGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxEditFEMeshLaplacianSmoothGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxEditFEMeshLaplacianSmoothGroup,vtkKWMimxGroupBase);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();
  void EditFEMeshLaplacianSmoothDoneCallback();
  void EditFEMeshLaplacianSmoothCancelCallback();
  void UpdateObjectLists();
  void RecalculateCheckButtonCallback(int State);
protected:
        vtkKWMimxEditFEMeshLaplacianSmoothGroup();
        ~vtkKWMimxEditFEMeshLaplacianSmoothGroup();
        virtual void CreateWidget();
  vtkKWComboBoxWithLabel *FEMeshListComboBox;
  vtkKWComboBoxWithLabel *BBListComboBox;
  vtkKWComboBoxWithLabel *SurfaceListComboBox;
  vtkKWEntryWithLabel *SmoothIterations;
  vtkKWEntryWithLabel *RecalculateIterations;
  vtkKWCheckButton *RecalculateCheckButton;
private:
  vtkKWMimxEditFEMeshLaplacianSmoothGroup(const vtkKWMimxEditFEMeshLaplacianSmoothGroup&); // Not implemented
  void operator=(const vtkKWMimxEditFEMeshLaplacianSmoothGroup&); // Not implemented
 };

#endif

