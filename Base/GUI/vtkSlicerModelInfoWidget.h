/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerNodeSelectorWidget.h,v $
  Date:      $Date$
  Version:   $Revision$

=========================================================================auto=*/

///  vtkSlicerNodeSelectorWidget - menu to select volumes from current mrml scene
/// 
/// Inherits most behavior from kw widget, but is specialized to observe
/// the current mrml scene and update the entries of the pop up menu to correspond
/// to the currently available volumes.  This widget also has a notion of the current selection
/// that can be observed or set externally
//


#ifndef __vtkSlicerModelInfoWidget_h
#define __vtkSlicerModelInfoWidget_h

#include "vtkSlicerWidget.h"

#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWEntry.h"

#include "vtkMRMLModelNode.h"
#include "vtkMRMLStorageNode.h"

class vtkKWPushButtonWithLabel;
class vtkMassProperties;
class vtkTriangleFilter;

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerModelInfoWidget : public vtkSlicerWidget
{
  
public:
  static vtkSlicerModelInfoWidget* New();
  vtkTypeRevisionMacro(vtkSlicerModelInfoWidget,vtkSlicerWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  void SetModelNode ( vtkMRMLModelNode *node );
  
  /// 
  /// alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  /// 
  /// alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  /// 
  /// removes observers on widgets in the class
  virtual void RemoveWidgetObservers ( );

  /// 
  /// add observers on widgets in the class
  virtual void AddWidgetObservers ( );

  virtual void UpdateWidgetFromMRML();

  /// 
  /// get current volume node
  vtkMRMLModelNode* GetModelNode ();
  
  /// 
  /// get current volume storage node
  vtkMRMLStorageNode* GetModelStorageNode ();

  /// 
  /// Add a Node Selector Widget
  vtkSetMacro(AddNodeSelectorWidget,int);
  vtkGetMacro(AddNodeSelectorWidget,int);
  vtkBooleanMacro(AddNodeSelectorWidget,int);

protected:
  vtkSlicerModelInfoWidget();
  virtual ~vtkSlicerModelInfoWidget();

  /// 
  /// Create the widget.
  virtual void CreateWidget();

  int AddNodeSelectorWidget;

  vtkMRMLModelNode *ModelNode;

  vtkSlicerNodeSelectorWidget *ModelSelectorWidget;


  vtkKWEntryWithLabel *AreaEntry;

  vtkKWEntryWithLabel *VolumeEntry;

  vtkKWEntryWithLabel *NumPointsEntry;

  vtkKWEntryWithLabel *NumCellsEntry;

  vtkKWEntryWithLabel *NumPointScalarsEntry;

  vtkKWEntryWithLabel *NumCellScalarsEntry;

  vtkKWEntryWithLabel *FileNameEntry;

  vtkTriangleFilter *Triangles;
  vtkMassProperties *MassProps;
private:

  /// internal flag to prevent widget updates from triggering 
  /// a feedback loop
  int UpdatingFromMRML;

  vtkSlicerModelInfoWidget(const vtkSlicerModelInfoWidget&); /// Not implemented
  void operator=(const vtkSlicerModelInfoWidget&); /// Not Implemented
};

#endif

