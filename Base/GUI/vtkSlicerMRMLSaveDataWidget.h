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


#ifndef __vtkSlicerMRMLSaveDataWidget_h
#define __vtkSlicerMRMLSaveDataWidget_h

#include "vtkSlicerWidget.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"

class vtkKWDialog;
class vtkKWLoadSaveDialog;
class vtkKWEntryWithLabel;
class vtkKWLoadSaveButtonWithLabel;
class vtkKWCheckButton;
class vtkKWMultiColumnListWithScrollbars;
class vtkKWPushButton;
class vtkMRMLStorageNode;

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerMRMLSaveDataWidget : public vtkSlicerWidget
{
  
public:
  static vtkSlicerMRMLSaveDataWidget* New();
  vtkTypeRevisionMacro(vtkSlicerMRMLSaveDataWidget,vtkSlicerWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
  
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
  // Directory to save files into
  vtkGetStringMacro(DataDirectoryName);
  vtkSetStringMacro(DataDirectoryName);
  
  // Description:
  // update list from MRML, return number of nodes need to save
  // TODO: make update event driven so that we don't have to call this
  int UpdateFromMRML();

  // Description:
  // invoke the widget
  void Invoke();
  
    //BTX
  enum
    {
      DataSavedEvent = 69010,
    };
//ETX


protected:
  vtkSlicerMRMLSaveDataWidget();
  virtual ~vtkSlicerMRMLSaveDataWidget();

  // Description:
  // Create the widget.
  virtual void CreateWidget();
  
  void SaveScene();

  void UpdateDataDirectory();

private:
  
  char *DataDirectoryName;
  
  //BTX
  std::vector<std::string> Nodes;
  std::vector<std::string> StorageNodes;
  //ETX
  
  vtkKWDialog *SaveDialog;

  vtkKWLoadSaveButtonWithLabel *SaveSceneButton;

  vtkKWEntryWithLabel *SceneName;

  vtkKWCheckButton *SaveSceneCheckBox;

 
  
  vtkKWLoadSaveButtonWithLabel *SaveDataButton;

  // Description:
  // change all the save flags on the data
  vtkKWPushButton *SaveAllDataButton;
  vtkKWPushButton *SaveNoDataButton;
  
  vtkKWPushButton *OkButton;

  vtkKWPushButton *CancelButton;
  
  vtkKWMultiColumnListWithScrollbars *MultiColumnList;


  bool IsProcessing;
  
  vtkSlicerMRMLSaveDataWidget(const vtkSlicerMRMLSaveDataWidget&); // Not implemented
  void operator=(const vtkSlicerMRMLSaveDataWidget&); // Not Implemented
};

#endif

