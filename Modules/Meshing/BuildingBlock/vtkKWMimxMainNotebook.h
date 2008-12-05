/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxMainNotebook.h,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.24.4.2 $

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
// .NAME vtkKWMimxMainNotebook - a tabbed notebook of UI pages containing 
// menu buttons of operations.
// .SECTION Description
// Class contains 7 pages 1) Image 2) Surface 3) Block(s) 4) Mesh 5) Quality
// 6) Materials 7) Load/BC. Each page contains operations menu button to
// launch subsequent GUI

#ifndef __vtkKWMimxMainNotebook_h
#define __vtkKWMimxMainNotebook_h

#include "vtkKWCompositeWidget.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkLinkedListWrapperTree.h"
#include "vtkBuildingBlockWin32Header.h"

class vtkKWUserInterfacePanel;
class vtkKWMimxBBMenuGroup;
class vtkKWMimxBoundaryConditionsMenuGroup;
class vtkKWMimxFEMeshMenuGroup;
class vtkKWMimxImageMenuGroup;
class vtkKWMimxMainWindow;
class vtkKWMimxMaterialPropertyMenuGroup;
class vtkKWMimxQualityMenuGroup;
class vtkKWMimxSurfaceMenuGroup;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxMainNotebook : public vtkKWCompositeWidget
{
public:
  static vtkKWMimxMainNotebook* New();
  vtkTypeRevisionMacro(vtkKWMimxMainNotebook,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
 
  virtual void Update();
  virtual void UpdateEnableState();
  vtkSetObjectMacro(MimxMainWindow, vtkKWMimxMainWindow);
  vtkGetObjectMacro(MimxMainWindow, vtkKWMimxMainWindow);
  vtkGetObjectMacro(SurfaceMenuGroup, vtkKWMimxSurfaceMenuGroup);
  vtkGetObjectMacro(ImageMenuGroup, vtkKWMimxImageMenuGroup);
  vtkGetObjectMacro(FEMeshMenuGroup, vtkKWMimxFEMeshMenuGroup);
  vtkGetObjectMacro(BBMenuGroup, vtkKWMimxBBMenuGroup);
  vtkSetObjectMacro(DoUndoTree, vtkLinkedListWrapperTree);
  
protected:
        vtkKWMimxMainNotebook();
        ~vtkKWMimxMainNotebook();
        
  void SetLists();
        virtual void CreateWidget();
        
        vtkKWNotebook *Notebook;
        vtkKWMimxMainWindow *MimxMainWindow;
        vtkKWMimxSurfaceMenuGroup *SurfaceMenuGroup;
        vtkKWMimxBBMenuGroup *BBMenuGroup;
        vtkKWMimxFEMeshMenuGroup *FEMeshMenuGroup;
        vtkKWMimxImageMenuGroup *ImageMenuGroup;
  vtkKWMimxQualityMenuGroup *QualityMenuGroup;
  vtkKWMimxMaterialPropertyMenuGroup *MaterialPropertyMenuGroup;
  vtkKWMimxBoundaryConditionsMenuGroup *BoundaryConditionsMenuGroup;
        vtkLinkedListWrapperTree *DoUndoTree;
    
private:
  vtkKWMimxMainNotebook(const vtkKWMimxMainNotebook&); // Not implemented
  void operator=(const vtkKWMimxMainNotebook&); // Not implemented
  
};

#endif

