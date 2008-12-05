/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxCreateExtrudeFEMeshGroup.h,v $
Language:  C++
Date:      $Date: 2008/06/19 17:53:41 $
Version:   $Revision: 1.11 $

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
// .NAME vtkKWMimxCreateExtrudeFEMeshGroup - a tabbed notebook of UI pages
// .SECTION Description
// The class is derived from vtkKWCompositeWidget. It contains 5 pages 1) Image
// 2) Surface 3) Building Block 4) F E Mesh 5) Mesh quality. Each page inturn
// contains a list of options specific to a page.

#ifndef __vtkKWMimxCreateExtrudeFEMeshGroup_h
#define __vtkKWMimxCreateExtrudeFEMeshGroup_h

#include "vtkKWMimxGroupBase.h"
#include "vtkKWMimxFEMeshMenuGroup.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWMenuButtonWithLabel;
class vtkKWCheckButtonWithLabel;
class vtkKWEntryWithLabel;
class vtkKWFrameWithLabel;
class vtkKWPushButton;
class vtkKWRadioButtonSet;

class vtkLinkedListWrapper;
class vtkIntArray;
class vtkKWComboBoxWithLabel;
class vtkKWRenderWidget;
class vtkActor;
class vtkMimxSelectCellsWidgetFEMesh;
class vtkMimxMeshActor;

class vtkKWMimxNodeElementNumbersGroup;
class vtkKWMimxDefineElSetGroup;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxCreateExtrudeFEMeshGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxCreateExtrudeFEMeshGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxCreateExtrudeFEMeshGroup,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();
  vtkGetObjectMacro(MimxMainWindow, vtkKWMimxMainWindow);
  vtkSetObjectMacro(MimxMainWindow, vtkKWMimxMainWindow);

  void CreateExtrudeFEMeshDoneCallback();
  void CreateExtrudeFEMeshCancelCallback();
  int CreateExtrudeFEMeshApplyCallback();

  void UpdateObjectLists();
  void ShowNormalsCallback(int State);
  void SelectionChangedCallback(const char*);
  int SelectElementsThroughCallback();
  int SelectElementsSurfaceCallback();
  void VTKInteractionCallback();
  vtkMimxMeshActor* GetSelectedObject();
  void DefineElementSetCallback();
protected:
        vtkKWMimxCreateExtrudeFEMeshGroup();
        ~vtkKWMimxCreateExtrudeFEMeshGroup();
        virtual void CreateWidget();
  vtkKWComboBoxWithLabel *ObjectListComboBox;
  vtkKWMimxMainWindow *MimxMainWindow;
  vtkKWEntryWithLabel *ExtrusionLength;
  vtkKWEntryWithLabel *NumberOfDivisions;
  vtkKWCheckButtonWithLabel *ShowNormalsCheckButton;
  vtkActor *NormalsActor;
  vtkKWMimxNodeElementNumbersGroup *NodeElementNumbersGroup;
  vtkKWRadioButtonSet *SelectSubsetRadiobuttonSet;
  vtkMimxSelectCellsWidgetFEMesh *SelectCellsWidget;
  vtkKWPushButton *DefineElSetButton;
  int MaxEleNum;
  int MaxNodeNum;
  vtkKWMimxDefineElSetGroup *DefineElementSetDialog;
private:
  vtkKWMimxCreateExtrudeFEMeshGroup(const vtkKWMimxCreateExtrudeFEMeshGroup&); // Not implemented
  void operator=(const vtkKWMimxCreateExtrudeFEMeshGroup&); // Not implemented
 };

#endif

