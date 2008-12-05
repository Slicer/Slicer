/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxMergeMeshGroup.h,v $
Language:  C++
Date:      $Date: 2008/03/19 23:04:55 $
Version:   $Revision: 1.7 $

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
// .NAME vtkKWMimxMergeMeshGroup - a taFEMeshed notebook of UI pages
// .SECTION Description
// The class is derived from vtkKWMimxGroupBase. It contains 5 pages 1) Image
// 2) Surface 3) Building Block 4) F E Mesh 5) Mesh quality. Each page inturn
// contains a list of options specific to a page.

#ifndef __vtkKWMimxMergeMeshGroup_h
#define __vtkKWMimxMergeMeshGroup_h

#include "vtkKWMimxGroupBase.h"
#include "vtkKWMimxFEMeshMenuGroup.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkKWMimxSurfaceMenuGroup.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWMultiColumnListWithScrollbars;
class vtkKWFrameWithLabel;
class vtkKWCheckButtonWithLabel;
class vtkKWEntryWithLabel;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxMergeMeshGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxMergeMeshGroup* New();
vtkTypeRevisionMacro(vtkKWMimxMergeMeshGroup,vtkKWMimxGroupBase);
void PrintSelf(ostream& os, vtkIndent indent);
virtual void Update();
virtual void UpdateEnableState();
void MergeMeshDoneCallback();
void MergeMeshCancelCallback();
int MergeMeshApplyCallback();
void UpdateObjectLists();
void MergeNodesCallback(int State);
protected:
        vtkKWMimxMergeMeshGroup();
        ~vtkKWMimxMergeMeshGroup();
        virtual void CreateWidget();
        vtkKWMultiColumnListWithScrollbars *MultiColumnList;
        vtkKWFrameWithLabel *NodeMergingFrame;
        vtkKWCheckButtonWithLabel *NodeMergeCheckButton;
        vtkKWEntryWithLabel *ToleranceEntry;
private:
  vtkKWMimxMergeMeshGroup(const vtkKWMimxMergeMeshGroup&); // Not implemented
void operator=(const vtkKWMimxMergeMeshGroup&); // Not implemented
 };

#endif

