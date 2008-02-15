/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerNodeSelectorWidget.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

// .NAME vtkSlicerNodeSelectorWidget - menu to select volumes from current mrml scene
// .SECTION Description
// Inherits most behavior from kw widget, but is specialized to observe
// the current mrml scene and update the entries of the pop up menu to correspond
// to the currently available volumes.  This widget also has a notion of the current selection
// that can be observed or set externally
//


#ifndef __vtkSlicerModelDisplayWidget_h
#define __vtkSlicerModelDisplayWidget_h

#include "vtkSlicerWidget.h"

#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWScaleWithLabel.h"
#include "vtkKWSurfaceMaterialPropertyWidget.h"
#include "vtkKWChangeColorButton.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"

#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelDisplayNode.h"

#include "vtkSlicerModelHierarchyLogic.h"

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerModelDisplayWidget : public vtkSlicerWidget
{
  
public:
  static vtkSlicerModelDisplayWidget* New();
  vtkTypeRevisionMacro(vtkSlicerModelDisplayWidget,vtkSlicerWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set  MRML ModelDisplayNode.
  void SetModelDisplayNode ( vtkMRMLModelDisplayNode *node );
  
  // Description:
  // Set  MRML ModelNode for dscalar colors
  void SetModelNode ( vtkMRMLModelNode *node );
  
  // Description:
  // Set  MRML ModelHierarchyNode 
  void SetModelHierarchyNode ( vtkMRMLModelHierarchyNode *node );
  
  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  // Description:
  // removes observers on widgets in the class
  virtual void RemoveWidgetObservers ( );

  // Description:
  // add observers on display node
  virtual void AddMRMLObservers ( );

  // Description:
  // remove observers on display node
  virtual void RemoveMRMLObservers ( );
  
  // Description:
  // get/set vtkSlicerModelHierarchyLogic
  vtkGetObjectMacro( ModelHierarchyLogic, vtkSlicerModelHierarchyLogic );
  vtkSetObjectMacro( ModelHierarchyLogic, vtkSlicerModelHierarchyLogic );

 protected:
  vtkSlicerModelDisplayWidget();
  virtual ~vtkSlicerModelDisplayWidget();

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  void UpdateWidget();
  void UpdateMRML();
  
  vtkMRMLModelDisplayNode* ModelDisplayNode;
  vtkMRMLModelNode *ModelNode;
  vtkMRMLModelHierarchyNode *ModelHierarchyNode;
  
  vtkKWCheckButtonWithLabel *VisibilityButton;
  vtkKWCheckButtonWithLabel *ScalarVisibilityButton;
  vtkKWMenuButtonWithLabel *ScalarMenu;
  vtkSlicerNodeSelectorWidget* ColorSelectorWidget;
  vtkKWCheckButtonWithLabel *ClippingButton;
  vtkKWCheckButtonWithLabel *BackfaceCullingButton;
  vtkKWScaleWithLabel  *OpacityScale;
  vtkKWSurfaceMaterialPropertyWidget *SurfaceMaterialPropertyWidget;
  vtkKWChangeColorButton *ChangeColorButton;

  vtkSlicerModelHierarchyLogic * ModelHierarchyLogic;

  // Description:
  // Set this when processing an event, since repopulate the scalars menu
  int ProcessingMRMLEvent;
  int ProcessingWidgetEvent;

  int UpdatingMRML;
  int UpdatingWidget;
  
private:


  vtkSlicerModelDisplayWidget(const vtkSlicerModelDisplayWidget&); // Not implemented
  void operator=(const vtkSlicerModelDisplayWidget&); // Not Implemented
};

#endif

