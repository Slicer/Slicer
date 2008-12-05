/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxEditElementSetNumbersGroup.h,v $
Language:  C++
Date:      $Date: 2008/05/31 16:39:39 $
Version:   $Revision: 1.2 $

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
// .NAME vtkKWMimxEditElementSetNumbersGroup - a tabbed notebook of UI pages
// .SECTION Description
// The class is derived from vtkKWMimxGroupBase. It contains 5 pages 1) Image
// 2) Surface 3) Building Block 4) F E Mesh 5) Mesh quality. Each page inturn
// contains a list of options specific to a page.

#ifndef __vtkKWMimxEditElementSetNumbersGroup_h
#define __vtkKWMimxEditElementSetNumbersGroup_h

#include "vtkKWMimxGroupBase.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWComboBoxWithLabel;
class vtkKWEntryWithLabel;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxEditElementSetNumbersGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxEditElementSetNumbersGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxEditElementSetNumbersGroup,vtkKWMimxGroupBase);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();
  //void EditElementSetNumbersDoneCallback();
  void EditElementSetNumbersCancelCallback();
  int EditElementSetNumbersApplyCallback();
  void UpdateObjectLists();
  void SelectionChangedCallback(const char *Selection);
  void ElementSetChangedCallback(const char *Selection);
  void NodeSetChangedCallback(const char *Selection);
  
protected:
        vtkKWMimxEditElementSetNumbersGroup();
        ~vtkKWMimxEditElementSetNumbersGroup();
        virtual void CreateWidget();
        
        vtkKWFrameWithLabel *ComponentFrame;
  vtkKWComboBoxWithLabel *ObjectListComboBox;
  vtkKWComboBoxWithLabel *ElementSetComboBox;
  vtkKWEntryWithLabel *StartingElementNumberEntry;
  vtkKWComboBoxWithLabel *NodeSetComboBox;
  vtkKWEntryWithLabel *StartingNodeNumberEntry;
private:
  vtkKWMimxEditElementSetNumbersGroup(const vtkKWMimxEditElementSetNumbersGroup&); // Not implemented
void operator=(const vtkKWMimxEditElementSetNumbersGroup&); // Not implemented
 };

#endif

