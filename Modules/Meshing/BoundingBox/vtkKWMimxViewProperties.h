/*=========================================================================

  Module:    $RCSfile: vtkKWMimxViewProperties.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWMimxViewProperties - a tabbed notebook of UI pages
// .SECTION Description
// The class is derived from vtkKWCompositeWidget. It contains 5 pages 1) Image
// 2) Surface 3) Bounding Box 4) F E Mesh 5) Mesh quality. Each page inturn
// contains a list of options specific to a page.

#ifndef __vtkKWMimxViewProperties_h
#define __vtkKWMimxViewProperties_h

#include "vtkBoundingBox.h"

#include "vtkKWCompositeWidget.h"
#include "vtkKWMimxFEMeshMenuGroup.h"
#include "vtkKWMimxViewWindow.h"
#include "vtkKWMimxSurfaceMenuGroup.h"

class vtkKWChangeColorButton;
class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkKWMultiColumnList;
class vtkKWThumbWheel;
class vtkLinkedListWrapper;

class VTK_BOUNDINGBOX_EXPORT vtkKWMimxViewProperties : public vtkKWCompositeWidget
{
public:
  static vtkKWMimxViewProperties* New();
  vtkTypeRevisionMacro(vtkKWMimxViewProperties,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
 // const char* GetActivePage();        // return the name of the chosen page
 // int GetActiveOption();        // return the chosen operation
  virtual void Update();
  virtual void UpdateEnableState();
  vtkSetObjectMacro(MimxViewWindow, vtkKWMimxViewWindow);
  vtkGetObjectMacro(MimxViewWindow, vtkKWMimxViewWindow);
  vtkSetObjectMacro(ObjectList, vtkLinkedListWrapper);
  vtkGetObjectMacro(MainFrame, vtkKWFrameWithLabel);
  void VisibilityCallback(int );
  void AddObjectList();
  void DeleteObjectList(int );
  virtual void SetActorColor(double color[3]);
  void SetViewProperties();
  void SortCommand();
  void SurfaceRepresentationCallback();
  void WireframeRepresentationCallback();
  void LineWidthCallback(double width);
  void OpacityCallback(double width);
  void UpdateVisibility();
protected:
        vtkKWMimxViewProperties();
        ~vtkKWMimxViewProperties();
        virtual void CreateWidget();
  vtkKWMultiColumnList *MultiColumnList;
  vtkKWMimxViewWindow *MimxViewWindow;
  vtkLinkedListWrapper *ObjectList;
  vtkKWFrameWithLabel *MainFrame;
  vtkKWChangeColorButton *ActorColorButton;
  vtkKWMenuButtonWithLabel *DisplayStyleMenuButton;
  vtkKWThumbWheel *LinewidthThumbwheel;
  vtkKWThumbWheel *OpacityThumbwheel;
private:
  vtkKWMimxViewProperties(const vtkKWMimxViewProperties&); // Not implemented
  void operator=(const vtkKWMimxViewProperties&); // Not implemented
 };

#endif

