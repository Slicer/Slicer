/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxViewPropertiesGroup.h,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:40 $
Version:   $Revision: 1.11.4.1 $

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
// .NAME vtkKWMimxViewPropertiesGroup - GUI containing display properties
// of the actor selected.
// .SECTION Description
// The GUI is a pop-up window containing the display properties of the object
// selected. The properties displayed and can be changed are 1) Actor color,
// 2) Outline color, 3) Element set display , 4) Opacity 

#ifndef __vtkKWMimxViewPropertiesGroup_h
#define __vtkKWMimxViewPropertiesGroup_h

#include "vtkKWTopLevel.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkLocalLinkedListWrapper.h"
#include "vtkLinkedListWrapperTree.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWChangeColorButton;
class vtkKWCheckButtonWithLabel;
class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkKWFrameWithScrollbar;
class vtkKWLabel;
class vtkKWPushButton;
class vtkKWRange;
class vtkKWSpinBoxWithLabel;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxViewPropertiesGroup : public vtkKWTopLevel
{
public:
  static vtkKWMimxViewPropertiesGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxViewPropertiesGroup,vtkKWTopLevel);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();

  // Description:
  // Set all the accesses to the program.
  vtkSetObjectMacro(MimxMainWindow, vtkKWMimxMainWindow);
  vtkGetObjectMacro(MimxMainWindow, vtkKWMimxMainWindow);
  vtkSetObjectMacro(DoUndoTree, vtkLinkedListWrapperTree);
  vtkSetObjectMacro(MultiColumnList, vtkKWMultiColumnListWithScrollbars);
  vtkGetObjectMacro(MainFrame, vtkKWFrameWithLabel);

  // Description:
  // Set actor color
  virtual void SetActorColor(double color[3]);

  // Description:
  // Set the color of wireframe in wireframe+surface display mode
  virtual void SetOutlineColor(double color[3]);

  virtual void SetElementSetColor(double color[3]);

  // Description:
  // Callback to change the representation between surface, surface+wireframe
  // and wireframe.
  void RepresentationCallback(int type);

  // Description:
  // To shrink or expand the element size
  void SetElementSizeFromScaleCallback(double scale);

  // Description:
  // Change the line width for wireframe or surface+wireframe representation.
  void LineWidthCallback(double width);

  void OpacityCallback(double width);

  vtkSetObjectMacro(ObjectList, vtkLocalLinkedListWrapper);

  // Description:
  // Set the default view properties
  void SetViewProperties();
  vtkGetMacro(ElementsHighlighted,int);

  // Description:
  // Switch between Mesh and element set mode.
  void DisplayModeCallback(int mode);

  // Description:
  // Element set visibility when in element set display mode.
  void ElementSetVisibilityCallback(int index, int flag);

  // Description:
  // To display element set display property
  void ElementSetDisplayPropertyCallback( );

  void SetMeshDisplayPropertyCallback();

  // Description:
  // Populate the multicolumnlist with the element set labels
  void SetElementSetList();

  // Description:
  // Item/Object name selected
  void SetItemName(const char *item);

  // Description:
  // Item/Object id selected
  void SetItemId(const char *id);

  // Description:
  // Get the item row number. The row number will be from the multicolumnlist
  // from which the current window was launched.
  int GetItemRow( );
  vtkSetMacro(SelectionRow, int);
  
protected:
        vtkKWMimxViewPropertiesGroup();
        ~vtkKWMimxViewPropertiesGroup();
        virtual void CreateWidget();
  vtkKWMultiColumnListWithScrollbars *MultiColumnList;
  vtkKWMimxMainWindow *MimxMainWindow;
  vtkLocalLinkedListWrapper *ObjectList;
   vtkKWFrameWithLabel *MainFrame;
  vtkKWFrameWithLabel *PropertyFrame;
  vtkKWFrameWithLabel *ColorFrame;
  vtkKWFrameWithLabel *StyleFrame;
  vtkKWFrameWithLabel *NodeDataFrame;
  vtkKWFrameWithLabel *ElementDataFrame;
  vtkKWChangeColorButton *ActorColorButton;
  vtkKWChangeColorButton *ElementDataColorButton;
  vtkKWMenuButtonWithLabel *DisplayStyleMenuButton;
  vtkKWMenuButtonWithLabel *NodeDataMenuButton;
  vtkKWMenuButtonWithLabel *ElementDataMenuButton;
  vtkKWMenuButtonWithLabel *DisplayLabels;
  vtkKWSpinBoxWithLabel *LinewidthThumbwheel;
  vtkKWSpinBoxWithLabel *OpacityThumbwheel;
  vtkLinkedListWrapperTree *DoUndoTree;
  vtkKWFrameWithLabel *ElementSizeFrame;
  vtkKWSpinBoxWithLabel *ElementSizeScale;
  vtkKWCheckButtonWithLabel *OverrideColorButton;
  vtkKWRange *ElementColorRange;
  int ElementsHighlighted;
  vtkKWPushButton *CloseButton;
  vtkKWCheckButtonWithLabel *OutlineButton;
  vtkKWChangeColorButton *OutlineColorButton;
  vtkKWMenuButtonWithLabel *ColorMenuButton;
  vtkKWMenuButtonWithLabel *DisplayModeButton;
  vtkKWMultiColumnListWithScrollbars *ElementSetList;
  vtkKWLabel *ObjectNameLabel;
  
  int SelectionRow;
  bool inializedElementSetList;
  char ItemName[256];
  char ItemId[64];
  //int DataType;
private:
  vtkKWMimxViewPropertiesGroup(const vtkKWMimxViewPropertiesGroup&); // Not implemented
  void operator=(const vtkKWMimxViewPropertiesGroup&); // Not implemented
 };

#endif

