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

class vtkKWPushButton;
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
  // Get methods on class members ( no Set methods required. )
  vtkGetObjectMacro ( ColorNodeTypeLabel, vtkKWLabel);
  vtkGetObjectMacro ( NumberOfColorsLabel, vtkKWLabel);
  vtkGetObjectMacro ( AddColorButton, vtkKWPushButton);
  vtkGetObjectMacro ( MultiColumnList, vtkKWMultiColumnListWithScrollbars );

  //Description:
  //Enable/Disable MultiSelect possibilities
  //Call the set method before you call create. Otherwise there will be no effect
  vtkBooleanMacro(MultiSelectMode,int);
  vtkSetMacro(MultiSelectMode,int);

  // Description:
  // Set the selected node, the color id, and update the widgets
  void SetColorNode ( vtkMRMLColorNode *node );

  // Description:
  // Get the color node, needed for the Editor
  vtkGetObjectMacro ( ColorNode, vtkMRMLColorNode );
  
  // Description:
  // Getting and setting the mrml color node id
  vtkGetStringMacro(ColorNodeID);
  //vtkSetStringMacro(ColorlListNodeID);
  void SetColorNodeID(char *id);

  //BTX
  // Description:
  // ColorIDModifiedEvent is generated when the ColorNodeID is
  // changed
  enum
  {
      ColorIDModifiedEvent = 30000,
      SelectedColorModifiedEvent = 30001,
  };
  //ETX

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
  // Update the "enable" state of the object and its internal parts
  virtual void UpdateEnableState(void);

  // Description:
  // Return the index of the currently selected colour in the multi column 
  // list box, for use by other classes when they wish to call GetColor 
  // on the vtkMRMLColorNode. Returns -1 if no list box or no selection,
  // or if more than one row is selected.
  int GetSelectedColorIndex();
  void SetSelectedColorIndex(int index);

  // Description:
  // API for setting ColorNode, and observing it
  /*
  void SetMRMLNode ( vtkMRMLColorNode *node )
  { this->SetMRML ( vtkObjectPointer( &this->ColorNode), node ); }
    void SetAndObserveMRMLNode ( vtkMRMLColorNode *node )
    { this->SetAndObserveMRML ( vtkObjectPointer( &this->ColorNode), node ); }
  */
  
  // Description:
  // Once know that the GUI has to be cleared and updated to show elements
  // from a new list, use this call
  //  virtual void SetGUIFromNode(vtkMRMLColorNode * activeColorNode);
  
 protected:
  vtkSlicerColorDisplayWidget();
  virtual ~vtkSlicerColorDisplayWidget();

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  // Description:
  // Update the widget, used when the color node id changes
  void UpdateWidget();
  
  void UpdateMRML();

  // Description:
  // Called when the selected row changes, just update the label, called from UpdateWidget
  void UpdateSelectedColor();
  
  // Description:
  // id of the color node displayed in the widget
  char* ColorNodeID;
  
  // Description:
  // The the color node that is currently displayed in the widget
  vtkMRMLColorNode *ColorNode;
  
  // Description:
  // button to add a colour to a user defined table
  vtkKWPushButton *AddColorButton;

  // Description:
  // select a colour node to display
  vtkSlicerNodeSelectorWidget* ColorSelectorWidget;

  // Description:
  // type of the colour node
  vtkKWLabel *ColorNodeTypeLabel;

  // Description:
  // displays the number of colours in the table
  vtkKWLabel *NumberOfColorsLabel;

  // Description:
  // displays the currently selected colour index
  vtkKWLabel *SelectedColorLabel;
  
  // Description:
  // display the colours in the table
  vtkKWMultiColumnListWithScrollbars *MultiColumnList;
  int NumberOfColumns;

  // Description:
  // Decides if MultipleColumns can be selected or not
  int MultiSelectMode;

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

  vtkKWCheckButton *ShowOnlyNamedColorsCheckButton;
private:

  vtkSlicerColorDisplayWidget(const vtkSlicerColorDisplayWidget&); // Not implemented
  void operator=(const vtkSlicerColorDisplayWidget&); // Not Implemented
};

#endif

