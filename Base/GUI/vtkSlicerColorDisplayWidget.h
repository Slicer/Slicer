/*=auto=========================================================================

  Portions (c) Copyright 2006 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerColorDisplayWidget.h,v $
  Date:      $Date$
  Version:   $Revision$

=========================================================================auto=*/

///  vtkSlicerColorDisplayWidget - displays the colour node colours
/// 
/// Displays the attributes of the vtkMRMLColorNode. Has a notion of the currently selected colour.

#ifndef __vtkSlicerColorDisplayWidget_h
#define __vtkSlicerColorDisplayWidget_h

#include "vtkSlicerWidget.h"

#include "vtkSlicerNodeSelectorWidget.h"

#include "vtkMRMLColorNode.h"

class vtkKWPushButton;
class vtkKWChangeColorButton;
class vtkKWMultiColumnListWithScrollbars;
class vtkKWCheckButton;
class vtkKWEntryWithLabel;
class vtkScalarBarWidget;
class vtkSlicerViewerWidget;
class vtkKWScalarBarAnnotation;
class vtkSlicerPopUpHelpWidget;

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerColorDisplayWidget : public vtkSlicerWidget
{
  
public:
  static vtkSlicerColorDisplayWidget* New();
  vtkTypeRevisionMacro(vtkSlicerColorDisplayWidget,vtkSlicerWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// 
  /// Get methods on class members ( no Set methods required. )
  vtkGetObjectMacro ( ColorNodeTypeLabel, vtkKWLabel);
  vtkGetObjectMacro ( NumberOfColorsLabel, vtkKWLabel);
  vtkGetObjectMacro ( MultiColumnList, vtkKWMultiColumnListWithScrollbars );
  vtkGetObjectMacro ( NodeHelpWidget, vtkSlicerPopUpHelpWidget );
  vtkGetObjectMacro ( ColorSelectorWidget, vtkSlicerNodeSelectorWidget);
  vtkGetObjectMacro ( ScalarBarAnnotation, vtkKWScalarBarAnnotation);

  //Description:
  //Enable/Disable MultiSelect possibilities
  //Call the set method before you call create. Otherwise there will be no effect
  vtkBooleanMacro(MultiSelectMode,int);
  vtkSetMacro(MultiSelectMode,int);

  /// 
  /// Set the selected node, the color id, and update the widgets
  void SetColorNode ( vtkMRMLColorNode *node );

  /// 
  /// Get the color node, needed for the Editor
  vtkGetObjectMacro ( ColorNode, vtkMRMLColorNode );
  
  /// 
  /// Getting and setting the mrml color node id
  vtkGetStringMacro(ColorNodeID);
  void SetColorNodeID(char *id);

  //BTX
  /// 
  /// ColorIDModifiedEvent is generated when the ColorNodeID is
  /// changed
  enum
  {
      ColorIDModifiedEvent = 30000,
      SelectedColorModifiedEvent = 30001,
  };
  //ETX

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
  /// add observers on color node
  virtual void AddMRMLObservers ( );

  /// 
  /// remove observers on color node
  virtual void RemoveMRMLObservers ( );

  /// 
  /// update a table entry
  void UpdateElement(int row, int col, char * str);  

  /// 
  /// Update the "enable" state of the object and its internal parts
  virtual void UpdateEnableState(void);

  /// 
  /// Return the index of the currently selected colour in the multi column 
  /// list box, for use by other classes when they wish to call GetColor 
  /// on the vtkMRMLColorNode. Returns -1 if no list box or no selection,
  /// or if more than one row is selected.
  int GetSelectedColorIndex();
  void SetSelectedColorIndex(int index);

  /// 
  /// API for setting ColorNode, and observing it
  /*
  void SetMRMLNode ( vtkMRMLColorNode *node )
  { this->SetMRML ( vtkObjectPointer( &this->ColorNode), node ); }
    void SetAndObserveMRMLNode ( vtkMRMLColorNode *node )
    { this->SetAndObserveMRML ( vtkObjectPointer( &this->ColorNode), node ); }
  */
  
  /// 
  /// Once know that the GUI has to be cleared and updated to show elements
  /// from a new list, use this call
  ///  virtual void SetGUIFromNode(vtkMRMLColorNode * activeColorNode);

  /// 
  /// Get/set the viewer widget so can add a the scalar bar widget to it
  vtkGetObjectMacro(ViewerWidget, vtkSlicerViewerWidget);
  virtual void SetViewerWidget(vtkSlicerViewerWidget *viewerWidget);

  /// 
  /// Get the scalar bar widget
  vtkGetObjectMacro(ScalarBarWidget, vtkScalarBarWidget);
 protected:
  vtkSlicerColorDisplayWidget();
  virtual ~vtkSlicerColorDisplayWidget();

  /// 
  /// Create the widget.
  virtual void CreateWidget();

  /// 
  /// Update the widget, used when the color node id changes
  void UpdateWidget();
  
  void UpdateMRML();

  /// 
  /// Called when the selected row changes, just update the label, called from UpdateWidget
  void UpdateSelectedColor();
  
  /// 
  /// id of the color node displayed in the widget
  char* ColorNodeID;
  
  /// 
  /// The the color node that is currently displayed in the widget
  vtkMRMLColorNode *ColorNode;
  
  /// 
  /// select a colour node to display
  vtkSlicerNodeSelectorWidget* ColorSelectorWidget;

  /// 
  /// pop up help describing the nodes
  vtkSlicerPopUpHelpWidget *NodeHelpWidget;

  /// 
  /// type of the colour node
  vtkKWLabel *ColorNodeTypeLabel;

  /// 
  /// displays the number of colours in the table
  vtkKWLabel *NumberOfColorsLabel;

  /// 
  /// displays the currently selected colour index
  vtkKWLabel *SelectedColorLabel;
  
  /// 
  /// display the colours in the table
  vtkKWMultiColumnListWithScrollbars *MultiColumnList;
  int NumberOfColumns;

  /// 
  /// Decides if MultipleColumns can be selected or not
  int MultiSelectMode;

///  vtkKWChangeColorButton *ChangeColorButton;
  //BTX
  /// 
  /// The column orders in the list box
  enum
    {
      EntryColumn = 0,
      NameColumn = 1,
      ColourColumn = 2,
    };
  //ETX

  vtkKWCheckButton *ShowOnlyNamedColorsCheckButton;
  /// 
  /// show and set the range on the colour lut
  vtkKWEntryWithLabel  *MinRangeEntry;
  vtkKWEntryWithLabel  *MaxRangeEntry;

  /// 
  /// Control the parameters for a scalar bar widget
  vtkKWScalarBarAnnotation* ScalarBarAnnotation;

  /// 
  /// a widget that manipulates a scalar bar actor (integrated). 
  vtkScalarBarWidget *ScalarBarWidget;

  /// 
  /// a pointer back to the viewer widget, useful for displaying the scalar bar actor
  vtkSlicerViewerWidget *ViewerWidget;

private:

  vtkSlicerColorDisplayWidget(const vtkSlicerColorDisplayWidget&); /// Not implemented
  void operator=(const vtkSlicerColorDisplayWidget&); /// Not Implemented
};

#endif

