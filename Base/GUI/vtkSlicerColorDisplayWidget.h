/*=auto=========================================================================

  Portions (c) Copyright 2006 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerColorDisplayWidget.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

// .NAME vtkSlicerColorDisplayWidget - displays the colour node colours
// .SECTION Description
// Displays the attributes of the vtkMRMLColorNode. Has a notion of the currently selected colour.

#ifndef __vtkSlicerColorDisplayWidget_h
#define __vtkSlicerColorDisplayWidget_h

#include "vtkSlicerWidget.h"

#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWScaleWithLabel.h"

#include "vtkMRMLColorNode.h"

class vtkKWChangeColorButton;
class vtkKWScaleWithEntry;
class vtkKWEntryWithLabel;
class vtkKWMultiColumnListWithScrollbars;
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerColorDisplayWidget : public vtkSlicerWidget
{
  
public:
  static vtkSlicerColorDisplayWidget* New();
  vtkTypeRevisionMacro(vtkSlicerColorDisplayWidget,vtkSlicerWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Getting setting  MRML ColorNodeID.
  vtkGetStringMacro ( ColorNodeID );
  vtkSetStringMacro ( ColorNodeID );
  
  vtkGetObjectMacro ( ColorNodeTypeLabel, vtkKWLabel);
  vtkGetObjectMacro ( NumberOfColorsLabel, vtkKWLabel);

  void SetColorNode ( vtkMRMLColorNode *node );
  
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
  // add observers on color node
  virtual void AddMRMLObservers ( );

  // Description:
  // remove observers on color node
  virtual void RemoveMRMLObservers ( );

  // Description:
  // update a table entry
  void UpdateElement(int row, int col, char * str);  

  // Description:
  // Return the index of the currently selected colour in the multi column 
  // list box, for use by other classes when they wish to call GetColor 
  // on the vtkMRMLColorNode. Returns -1 if no list box or no selection,
  // or if more than one row is selected.
  int GetSelectedColorIndex();

 protected:
  vtkSlicerColorDisplayWidget();
  virtual ~vtkSlicerColorDisplayWidget();

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  void UpdateWidget();
  void UpdateMRML();
  
  char* ColorNodeID;
  
  vtkSlicerNodeSelectorWidget* ColorSelectorWidget;

  // type of the colour node
  vtkKWLabel *ColorNodeTypeLabel;

  // Description:
  // displays the number of colours in the table
  vtkKWLabel *NumberOfColorsLabel;

  // Description:
  // display the colours in the table
  vtkKWMultiColumnListWithScrollbars *MultiColumnList;
  int NumberOfColumns;

//  vtkKWChangeColorButton *ChangeColorButton;
  //BTX
  // Description:
  // The column orders in the list box
  enum
    {
      EntryColumn = 0,
      NameColumn = 1,
      ColourColumn = 2,
    };
  //ETX
  
private:

  vtkSlicerColorDisplayWidget(const vtkSlicerColorDisplayWidget&); // Not implemented
  void operator=(const vtkSlicerColorDisplayWidget&); // Not Implemented
};

#endif

