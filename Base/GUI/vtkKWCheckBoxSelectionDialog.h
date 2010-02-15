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


#ifndef __vtkKWCheckBoxSelectionDialog_h
#define __vtkKWCheckBoxSelectionDialog_h

#include "vtkStringArray.h"

#include "vtkSlicerWidget.h"

class vtkKWDialog;
class vtkKWEntryWithLabel;
class vtkKWCheckButton;
class vtkKWMultiColumnListWithScrollbars;
class vtkKWPushButton;

class VTK_SLICER_BASE_GUI_EXPORT vtkKWCheckBoxSelectionDialog : public vtkSlicerWidget
{
  
public:
  static vtkKWCheckBoxSelectionDialog* New();
  vtkTypeRevisionMacro(vtkKWCheckBoxSelectionDialog,vtkSlicerWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  /// 
  /// alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  /// 
  /// removes observers on widgets in the class
  virtual void RemoveWidgetObservers ( );

  /// 
  /// Add entry name
  void AddEntry(const char *Label, int selected);

  /// 
  /// Clear all entries
  void RemoveAllEntries();

  /// 
  /// Sets all rows in the list box to be selected or unselected depending on
  /// the flag
  void SetAllEntriesSelected(int sel);

  /// 
  /// Returns selected entries
  vtkStringArray* GetSelectedEntries();
 
  /// 
  /// Returns unselected entries
  vtkStringArray* GetUnselectedEntries();

  /// 
  /// invoke the widget
  void Invoke();

  /// 
  /// Title
  vtkGetStringMacro(Title);
  vtkSetStringMacro(Title);

  /// 
  /// First colomn name
  vtkGetStringMacro(EntryColumnName);
  vtkSetStringMacro(EntryColumnName);

  /// 
  /// First colomn name
  vtkGetStringMacro(BoxColumnName);
  vtkSetStringMacro(BoxColumnName);

  /// 
  /// Cancel action
  vtkSetMacro(Cancel, int);
  vtkGetMacro(Cancel, int);


  /// Update widget
  //void UpdateWidget();
  
protected:
  vtkKWCheckBoxSelectionDialog();
  virtual ~vtkKWCheckBoxSelectionDialog();

  /// 
  /// Create the widget.
  virtual void CreateWidget();


private:
    
  //BTX
  //std::vector<std::string> Labels;
  //std::vector<std::string> Descriptions;
  //std::vector<std::int> Selected;
  //ETX
  
  vtkStringArray *SelectedLabels;

  char *Title;
  char *EntryColumnName;
  char *BoxColumnName;
  int Cancel;

  vtkKWDialog *Dialog;

  vtkKWPushButton *OkButton;

  vtkKWPushButton *CancelButton;

  /// 
  /// set all list box entries to selected/unselected in Box column 
  vtkKWPushButton *SelectAllButton;
  vtkKWPushButton *SelectNoneButton;
  
  vtkKWMultiColumnListWithScrollbars *MultiColumnList;

  vtkKWCheckBoxSelectionDialog(const vtkKWCheckBoxSelectionDialog&); /// Not implemented
  void operator=(const vtkKWCheckBoxSelectionDialog&); /// Not Implemented
};

#endif

