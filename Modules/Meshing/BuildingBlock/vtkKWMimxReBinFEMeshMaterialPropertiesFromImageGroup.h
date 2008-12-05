/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxReBinFEMeshMaterialPropertiesFromImageGroup.h,v $
Language:  C++
Date:      $Date: 2008/04/24 14:08:40 $
Version:   $Revision: 1.3 $

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
// .NAME vtkKWMimxReBinFEMeshMaterialPropertiesFromImageGroup - a tabbed notebook of UI pages
// .SECTION Description
// The class is derived from vtkKWMimxGroupBase. It contains 5 pages 1) Image
// 2) Surface 3) Building Block 4) F E Mesh 5) Mesh quality. Each page inturn
// contains a list of options specific to a page.

#ifndef __vtkKWMimxReBinFEMeshMaterialPropertiesFromImageGroup_h
#define __vtkKWMimxReBinFEMeshMaterialPropertiesFromImageGroup_h

#include "vtkKWMimxGroupBase.h"
#include "vtkKWMimxFEMeshMenuGroup.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkKWMimxSurfaceMenuGroup.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWComboBoxWithLabel;
class vtkKWEntryWithLabel;
class vtkIntArray;
class vtkKWFrame;
class vtkKWCheckButtonWithLabel;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxReBinFEMeshMaterialPropertiesFromImageGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxReBinFEMeshMaterialPropertiesFromImageGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxReBinFEMeshMaterialPropertiesFromImageGroup,vtkKWMimxGroupBase);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();
  void ReBinFEMeshMaterialPropertiesFromImageDoneCallback();
  int ReBinFEMeshMaterialPropertiesFromImageApplyCallback();
  void ReBinFEMeshMaterialPropertiesFromImageCancelCallback();
  void UpdateObjectLists();
  void FEMeshSelectionChangedCallback(const char *Selection);
  void MinimumValueCheckButtonCallback(int State);
  void MaximumValueCheckButtonCallback(int State);
  void ElementSetChangedCallback(const char *Selection);
  
  void ViewMaterialPropertyCallback( int mode );
  int ClippingPlaneCallback(int mode);
  void ViewPropertyLegendCallback( int mode );
  
protected:
        vtkKWMimxReBinFEMeshMaterialPropertiesFromImageGroup();
        ~vtkKWMimxReBinFEMeshMaterialPropertiesFromImageGroup();
        virtual void CreateWidget();
  vtkKWComboBoxWithLabel *FEMeshListComboBox;
  vtkKWComboBoxWithLabel *ElementSetComboBox;
  vtkKWEntryWithLabel *NumOfBinsEntry;
  vtkKWFrame *MinimumValueFrame;
  vtkKWFrame *MaximumValueFrame;
  vtkKWEntryWithLabel *MinimumRebinEntry;
  vtkKWEntryWithLabel *MaximumRebinEntry;
  vtkKWCheckButtonWithLabel *MinimumValueCheckButton;
  vtkKWCheckButtonWithLabel *MaximumValueCheckButton;
  vtkKWFrameWithLabel *ViewFrame;
  vtkKWCheckButtonWithLabel *ViewPropertyButton;
  vtkKWCheckButtonWithLabel *ViewLegendButton;
  vtkKWMenuButtonWithLabel *ClippingPlaneMenuButton;

private:
  vtkKWMimxReBinFEMeshMaterialPropertiesFromImageGroup(const vtkKWMimxReBinFEMeshMaterialPropertiesFromImageGroup&); // Not implemented
  void operator=(const vtkKWMimxReBinFEMeshMaterialPropertiesFromImageGroup&); // Not implemented
  
  char meshName[64];
  char elementSetName[64];
 };

#endif

