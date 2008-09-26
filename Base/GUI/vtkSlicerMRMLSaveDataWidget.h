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
class vtkStringArray;

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
  // Add/removes observers on widgets in the class
  virtual void AddWidgetObservers ( );
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
  
  // Description:
  // Save Scene
  // Return 1 on success; 0 on failure
  int SaveScene();

  // Description:
  // Save all the node data given their corresponding row indices
  // Return 1 on success; 0 on failure
  int SaveData(vtkIntArray* arrayRows);

  // Description:
  // Save all modified data
  // Return 1 on success; 0 on failure.
  int SaveModifiedData();

  // Description:
  // Save all marked data
  // Return 1 on success; 0 on failure. If no marked data in the table, return 0. 
  int SaveMarkedData();

  void UpdateDataDirectory();

  // Description:
  // Create all the columns for the multicolumn list
  void SetupSaveDataListWidget();

  // Description:
  // Set the filename related cells given the row index and the full filename
  // and supported file types.
  // (FileFormat_Column, FileName_Column, FileDirectory_Column)
  void SetFileNameRelatedCells(
    int row, const char* filename, vtkStringArray* supportedFileFormats);

  // Description:
  // Get a file format given the file extension and all the supported formats
  // and supported file types.
  const char* GetFileFormatWithExtension(
    const char* fileext, vtkStringArray* supportedFileFormats);

  // Description:
  // Get the current file extension of the row
  const char* GetRowCurrentFileExtension(int row);
  
  // Description:
  // Update the filename/format according to the format/filename of the row
  void UpdateRowFileNameWithExtension(int row);
  void UpdateRowFileFormatWithName(int row);
  
  // Description:
  // Check if the input extension is within the file format dropdown list.
  const char* GetRowFileFormatWithExtension(int row, const char* extension);

  // Description:
  // Set the data diretory related cells given the row index and the full dirname
  // (FileDirectory_Column)
  void SetDataDirectoryRelatedCells(
    int row, const char* dirname);

  // Description:
  // Mark/unmark the row for save.
  void SetRowMarkedForSave(int row, int marked, int doUpdate=1);

  // Description:
  // Set the Status_Column to be "modified" or "not modified"
  void SetRowModified(int row, int modified);

  // Description:
  // Invoked when the user successfully updated the data table
  // located at ('row', 'col') with the new contents, as a result
  // of editing the corresponding cell interactively.
  virtual void UpdateDataTableCell(int row, int col);

  // Description:
  // Add the Node/StorageNode Id to internal vector and set the
  // corresponding columns for them given the row index.
  virtual void AddNodeId(const char* strID, int row);
  virtual void AddStorageNodeId(const char* strID, int row);

  // Description:
  // Update the "enable" state of the object and its internal parts.
  // Depending on different Ivars (this->Enabled, the application's 
  // Limited Edition Mode, etc.), the "enable" state of the object is updated
  // and propagated to its internal parts/subwidgets. This will, for example,
  // enable/disable parts of the widget UI, enable/disable the visibility
  // of 3D widgets, etc.
  virtual void UpdateEnableState();

  // Description:
  // Update the node data directory given the row index.
  virtual void UpdateNodeDataDirectory(int row);

  //BTX
  // Description:
  // The column orders in the list box
  enum
    {
    Save_Column = 0,
    NodeName_Column, 
    Type_Column, 
    Status_Column,
    Format_Column,
    FileName_Column,
    FileDirectory_Column,
    Hidden_NodeID_Column,
    Hidden_StorageNodeID_Column,
    Hidden_FileName_Column 
    };
  //ETX

private:
  
  char *DataDirectoryName;
  
  //BTX
  std::vector<std::string> Nodes;
  std::vector<std::string> StorageNodes;
  //ETX
  
  vtkKWDialog *SaveDialog;

  vtkKWLoadSaveButtonWithLabel *SaveSceneButton;

  //vtkKWEntryWithLabel *SceneName;

  //vtkKWCheckButton *SaveSceneCheckBox;

 
  
  vtkKWLoadSaveButtonWithLabel *SaveDataButton;

  // Description:
  // change all the save flags on the data
  vtkKWPushButton *SaveAllDataButton;
  vtkKWPushButton *SaveNoDataButton;
  
  vtkKWPushButton *SaveDataOnlyButton;
  vtkKWPushButton *OkButton;

  vtkKWPushButton *CancelButton;
  
  vtkKWMultiColumnListWithScrollbars *MultiColumnList;


  bool IsProcessing;
  
  vtkSlicerMRMLSaveDataWidget(const vtkSlicerMRMLSaveDataWidget&); // Not implemented
  void operator=(const vtkSlicerMRMLSaveDataWidget&); // Not Implemented
};

#endif



