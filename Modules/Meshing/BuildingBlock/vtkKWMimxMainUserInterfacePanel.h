/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxMainUserInterfacePanel.h,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.2.4.2 $

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
// .NAME vtkKWMimxMainUserInterfacePanel - a tabbed UserInterfacePanel of UI pages
// .SECTION Description
// The class is derived from vtkKWCompositeWidget. It contains 5 pages 1) Image
// 2) Surface 3) Building Block 4) F E Mesh 5) Mesh quality. Each page inturn
// contains a list of options specific to a page.

#ifndef __vtkKWMimxMainUserInterfacePanel_h
#define __vtkKWMimxMainUserInterfacePanel_h

#include "vtkKWCompositeWidget.h"
#include "vtkKWMimxFEMeshMenuGroup.h"
#include "vtkKWMimxSurfaceMenuGroup.h"
#include "vtkKWMimxBBMenuGroup.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkLinkedListWrapperTree.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWUserInterfaceManager.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWUserInterfacePanel;
class vtkKWFrameWithScrollbar;
class vtkKWMenuButtonWithLabel;
class vtkMimxErrorCallback;
class vtkKWMimxImageMenuGroup;
class vtkKWMimxMainWindow;
class vtkKWFrameWithLabel;
class vtkKWNotebook;
class vtkKWMimxQualityMenuGroup;
class vtkKWMimxMainNotebook;
class vtkKWMultiColumnListWithScrollbars;
class vtkKWMimxViewPropertiesGroup;
class vtkKWMimxDisplayPropertiesGroup;


class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxMainUserInterfacePanel : public vtkKWCompositeWidget
{
public:
  static vtkKWMimxMainUserInterfacePanel* New();
  vtkTypeRevisionMacro(vtkKWMimxMainUserInterfacePanel,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
 // const char* GetActivePage();        // return the name of the chosen page
 // int GetActiveOption();      // return the chosen operation
  virtual void Update();
  virtual void UpdateEnableState();
  vtkSetObjectMacro(MimxMainWindow, vtkKWMimxMainWindow);
  vtkGetObjectMacro(MimxMainWindow, vtkKWMimxMainWindow);
  vtkSetObjectMacro(DoUndoTree, vtkLinkedListWrapperTree);
  vtkGetObjectMacro(DoUndoTree, vtkLinkedListWrapperTree);
  vtkGetObjectMacro(MimxMainNotebook, vtkKWMimxMainNotebook);
  vtkGetObjectMacro(MainFrame, vtkKWFrame);
  vtkSetObjectMacro(MultiColumnList, vtkKWMultiColumnListWithScrollbars);
  vtkGetObjectMacro(ViewPropertiesGroup, vtkKWMimxViewPropertiesGroup);
  vtkSetObjectMacro(UserInterfaceManger, vtkKWUserInterfaceManager);
  
  vtkKWUserInterfaceManager *UserInterfaceManger;
  
protected:
        vtkKWMimxMainUserInterfacePanel();
        ~vtkKWMimxMainUserInterfacePanel();
  void SetLists();
        virtual void CreateWidget();
        vtkKWNotebook *UserInterfacePanel;
        vtkKWFrame *MainFrame;
        vtkKWMimxMainWindow *MimxMainWindow;
        vtkLinkedListWrapperTree *DoUndoTree;
        vtkKWMimxMainNotebook *MimxMainNotebook;
        vtkKWMimxViewPropertiesGroup *ViewPropertiesGroup;
        vtkKWMultiColumnListWithScrollbars *MultiColumnList;
        vtkKWMimxDisplayPropertiesGroup *DisplayPropertiesGroup;
        
private:
  vtkKWMimxMainUserInterfacePanel(const vtkKWMimxMainUserInterfacePanel&); // Not implemented
  void operator=(const vtkKWMimxMainUserInterfacePanel&); // Not implemented
 };

#endif

