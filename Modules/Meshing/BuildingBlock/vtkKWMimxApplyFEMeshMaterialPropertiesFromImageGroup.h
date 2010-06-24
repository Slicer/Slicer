/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxApplyFEMeshMaterialPropertiesFromImageGroup.h,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:39 $
Version:   $Revision: 1.13.4.1 $

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
// .NAME vtkKWMimxApplyFEMeshMaterialPropertiesFromImageGroup - 
// Group for applying material properties calculated from image.
// .SECTION Description

#ifndef __vtkKWMimxApplyFEMeshMaterialPropertiesFromImageGroup_h
#define __vtkKWMimxApplyFEMeshMaterialPropertiesFromImageGroup_h

#include "vtkKWMimxGroupBase.h"

#include "vtkKWMimxFEMeshMenuGroup.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkKWMimxSurfaceMenuGroup.h"
#include "vtkBuildingBlockWin32Header.h"

#include "vtkMatrix4x4.h"

class vtkIntArray;
class vtkKWCheckButton;
class vtkKWCheckButtonWithLabel;
class vtkKWComboBoxWithLabel;
class vtkKWEntryWithLabel;
class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkKWMimxDefineElSetGroup;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxApplyFEMeshMaterialPropertiesFromImageGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxApplyFEMeshMaterialPropertiesFromImageGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxApplyFEMeshMaterialPropertiesFromImageGroup,vtkKWMimxGroupBase);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();

  // Description:
  // Call backs for push buttons
  void ApplyFEMeshMaterialPropertiesFromImageDoneCallback();
  int ApplyFEMeshMaterialPropertiesFromImageApplyCallback();
  void ApplyFEMeshMaterialPropertiesFromImageCancelCallback();

  // Description:
  // Updating all the entries in the combobox entries
  void UpdateObjectLists();

  void FEMeshSelectionChangedCallback(const char *Selection);
  void ElementSetChangedCallback(const char *Selection);

  // Description:
  // Callback to define a new element set
  void DefineElementSetCallback();

  // Description:
  // To bin material properties or not
  void BinCheckButtonCallback(int mode);

  // Description:
  // To specify range - both min and max
  void SpecifyRangeButtonCallback(int mode);

  int DefineConversionCallback();
  int ConversionApplyCallback();
  int ConversionCancelCallback();

protected:
  vtkKWMimxApplyFEMeshMaterialPropertiesFromImageGroup();
  ~vtkKWMimxApplyFEMeshMaterialPropertiesFromImageGroup();
  virtual void CreateWidget();

  // combobox to store all the entries
  vtkKWComboBoxWithLabel *ImageListComboBox;
  vtkKWComboBoxWithLabel *FEMeshListComboBox;
  vtkKWComboBoxWithLabel *ElementSetComboBox;
  vtkKWEntryWithLabel *PoissonsRatioEntry;
  vtkKWFrameWithLabel *ComponentFrame;
  vtkKWPushButton *DefineElSetButton;
  vtkKWMimxDefineElSetGroup *DefineElementSetDialog;
  vtkKWCheckButton *BinCheckButton;
  vtkKWFrameWithLabel *BinFrame;
  vtkKWCheckButtonWithLabel *SpecifyRangeButton;
  vtkKWEntryWithLabel *MinimumBinEntry;
  vtkKWEntryWithLabel *MaximumBinEntry;
  vtkKWEntryWithLabel *NumOfBinsEntry;
  vtkKWFrameWithLabel *ImageConstantFrame;
  vtkKWEntryWithLabel *ImageConstantA;
  vtkKWEntryWithLabel *ImageConstantB;
  vtkKWEntryWithLabel *ImageConstantC;
  vtkKWTopLevel *ConversionDialog;
  vtkKWLabel *EquationLabel;
  vtkKWPushButton *DialogApplyButton;
  vtkKWPushButton *DialogCancelButton;
  vtkKWPushButton *DefineConversionButton;
  vtkKWMenuButtonWithLabel *ImageConversionType;
  char elementSetPrevious[256];
private:
  vtkKWMimxApplyFEMeshMaterialPropertiesFromImageGroup(const vtkKWMimxApplyFEMeshMaterialPropertiesFromImageGroup&); // Not implemented
  void operator=(const vtkKWMimxApplyFEMeshMaterialPropertiesFromImageGroup&); // Not implemented
  
  char meshName[64];
  char elementSetName[64];
  double ConversionA;
  double ConversionB;
  double ConversionC;
  char ConversionType[64];
  
 };

#endif

