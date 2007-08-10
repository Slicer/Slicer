/*=========================================================================

  Module:    $RCSfile: vtkKWMimxEditBBGroup.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWMimxEditBBGroup - a tabbed notebook of UI pages
// .SECTION Description
// The class is derived from vtkKWCompositeWidget. It contains 5 pages 1) Image
// 2) Surface 3) Bounding Box 4) F E Mesh 5) Mesh quality. Each page inturn
// contains a list of options specific to a page.

#ifndef __vtkKWMimxEditBBGroup_h
#define __vtkKWMimxEditBBGroup_h

#include "vtkBoundingBox.h"

#include "vtkKWMimxGroupBase.h"
#include "vtkKWMimxFEMeshMenuGroup.h"
#include "vtkKWMimxViewWindow.h"

class vtkKWMenuButtonWithLabel;
class vtkKWFrameWithLabel;
class vtkKWPushButton;

class vtkLinkedListWrapper;
class vtkKWRadioButton;
class vtkKWComboBoxWithLabel;
class vtkKWMimxViewWindow;

class vtkMimxUnstructuredGridWidget;
class vtkMimxExtractEdgeWidget;
class vtkMimxExtractFaceWidget;
class vtkMimxExtractCellWidget;

class VTK_BOUNDINGBOX_EXPORT vtkKWMimxEditBBGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxEditBBGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxEditBBGroup,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();
  vtkGetObjectMacro(MimxViewWindow, vtkKWMimxViewWindow);
  vtkSetObjectMacro(MimxViewWindow, vtkKWMimxViewWindow);

  void EditBBAddCellCallback();
  void EditBBSplitCellCallback();
  void EditBBDeleteCellCallback();
  void EditBBDoneCallback();
  void EditBBCancelCallback();
  void EditBBMoveCellCallback();
  void EditBBVtkInteractionCallback();
  void SelectionChangedCallback(const char*);
protected:
        vtkKWMimxEditBBGroup();
        ~vtkKWMimxEditBBGroup();
        virtual void CreateWidget();
  vtkKWComboBoxWithLabel *ObjectListComboBox;
  vtkKWMimxViewWindow *MimxViewWindow;
  vtkKWRadioButton *MoveButton;
  vtkKWRadioButton *AddButton;
  vtkKWRadioButton *DeleteButton;
  vtkKWRadioButton *SplitButton;
  vtkKWRadioButton *VtkInteractionButton;
  vtkMimxUnstructuredGridWidget *UnstructuredGridWidget;
  vtkMimxExtractEdgeWidget *ExtractEdgeWidget;
  vtkMimxExtractFaceWidget *ExtractFaceWidget;
  vtkMimxExtractCellWidget *ExtractCellWidget;
  vtkIdType AddButtonState;
  vtkIdType DeleteButtonState;
  vtkIdType MoveButtonState;
  vtkIdType RegularButtonState;
  vtkIdType SplitButtonState;
//  vtkIdType SelectionState;
  vtkIdType SplitCount;
  vtkIdType AddCount;
  vtkIdType DeleteCount;
private:
  vtkKWMimxEditBBGroup(const vtkKWMimxEditBBGroup&); // Not implemented
  void operator=(const vtkKWMimxEditBBGroup&); // Not implemented
 };

#endif

