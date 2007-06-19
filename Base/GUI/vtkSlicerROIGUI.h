// .NAME vtkSlicerROIGUI 
// .SECTION Description
// Main ROI GUI and mediator methods for slicer3. 


#ifndef __vtkSlicerROIGUI_h
#define __vtkSlicerROIGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkSlicerROILogic.h"
#include "vtkMRMLROINode.h"
#include "vtkMRMLROIListNode.h"


#include "vtkKWFrame.h"
#include "vtkKWPushButton.h"
#include "vtkSlicerVisibilityIcons.h"
#include "vtkKWChangeColorButton.h"

#include "vtkKWScaleWithEntry.h"
#include "vtkKWScale.h"
#include "vtkKWLabel.h"
#include "vtkKWRange.h"
#include "vtkKWExtent.h"

class vtkKWMultiColumnListWithScrollbars;
class vtkKWPushButton;
class vtkKWMessage;
class vtkSlicerVisibilityIcons;
class vtkKWChangeColorButton;
class vtkKWScaleWithEntry;
class vtkKWMenuButtonWithLabel;

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerROIGUI : public vtkSlicerModuleGUI
{
public:
  // Description: 
  // Usual vtk class functions 
  static vtkSlicerROIGUI* New (  );
  vtkTypeRevisionMacro ( vtkSlicerROIGUI, vtkSlicerModuleGUI );
  void PrintSelf ( ostream& os, vtkIndent indent );

  // Description:
  // Get methods on class members ( no Set methods required. )
  vtkGetObjectMacro ( AddROIButton, vtkKWPushButton);
  vtkGetObjectMacro ( RemoveROIButton, vtkKWPushButton);
  vtkGetObjectMacro ( RemoveROIListButton, vtkKWPushButton);
  vtkGetObjectMacro ( VisibilityToggle, vtkKWPushButton);
  vtkGetObjectMacro ( VisibilityIcons, vtkSlicerVisibilityIcons);
  vtkGetObjectMacro ( ROIColorButton, vtkKWChangeColorButton);
  vtkGetObjectMacro ( ROISelectedColorButton, vtkKWChangeColorButton);
  vtkGetObjectMacro ( ROITextScale, vtkKWScaleWithEntry);
  vtkGetObjectMacro ( ROIOpacity, vtkKWScaleWithEntry);
  vtkGetObjectMacro ( Logic, vtkSlicerROILogic);

  void SetModuleLogic ( vtkSlicerROILogic *logic )
    { this->SetLogic ( vtkObjectPointer (&this->Logic), logic ); }
  void SetAndObserveModuleLogic ( vtkSlicerROILogic *logic )
    { this->SetAndObserveLogic ( vtkObjectPointer (&this->Logic), logic ); }

  // Description:
  // This method builds the ROI module GUI
  virtual void BuildGUI ( ) ;

  // Description:
  // This method releases references and key-bindings,
  // and optionally removes observers.
  virtual void TearDownGUI ( );

  // Description:
  // Methods for adding module-specific key bindings and
  // removing them.
  virtual void CreateModuleEventBindings ( );
  virtual void ReleaseModuleEventBindings ( );

  // Description:
  // Add/Remove observers on widgets in the GUI
  virtual void AddGUIObservers ( );
  virtual void RemoveGUIObservers ( );

  // Description:
  // Class's mediator methods for processing events invoked by
  // either the Logic, MRML or GUI.
  virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event, void *callData );
  virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event, void *callData );
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  // Description:
  // Once know that the GUI has to be cleared and updated to show elements
  // from a new list, use this call
  virtual void SetGUIFromList(vtkMRMLROIListNode * activeROIListNode);

  // Description:
  // Methods describe behavior at module enter and exit.
  virtual void Enter ( );
  virtual void Exit ( );

  virtual void UpdateElement(int row, int col, char * str);

  // Description:
  // Getting and setting the mrml ROI list node id
  vtkGetStringMacro(ROIListNodeID);
  void SetROIListNodeID(char *id);

  // Description:
  // Set the selected node, the ROI list id, and update the widgets
  void SetROIListNode(vtkMRMLROIListNode *ROIListNode);

  // Description:
  // Update the gui from the currently selected ROI node, called on Enter
  void UpdateGUI();

  // Description:
  // Which ROI list node are we displaying in this gui 
  vtkSlicerNodeSelectorWidget* ROIListSelectorWidget;

  // Description:
  // Which voliume node is associated with the ROI 
  vtkSlicerNodeSelectorWidget* VolumeNodeSelectorWidget;

protected:
  vtkSlicerROIGUI ( );
  virtual ~vtkSlicerROIGUI ( );

  // Description:
  // Module logic and mrml pointers
  vtkSlicerROILogic *Logic;

  // Description:
  // The ID of the ROI node that is currently displayed in the GUI
  char *ROIListNodeID;

  // Description:
  // The ROI list node that is currently displayed in the GUI
  vtkMRMLROIListNode *ROIListNode;

  // Widgets for the ROI module
  // add a point
  vtkKWPushButton *AddROIButton;
  // remove the last selected (multi column list definition of selected)
  // point
  vtkKWPushButton *RemoveROIButton;
  // remove all the ROI boxes on this list
  vtkKWPushButton *RemoveROIListButton;

  // ROI position and Raidus scale
  vtkKWScaleWithEntry * XPositionScale;
  vtkKWScaleWithEntry * YPositionScale;
  vtkKWScaleWithEntry * ZPositionScale;
  vtkKWScaleWithEntry * XRadiusScale;
  vtkKWScaleWithEntry * YRadiusScale;
  vtkKWScaleWithEntry * ZRadiusScale;

  vtkKWLabel *XPositionLabel;
  vtkKWLabel *YPositionLabel;
  vtkKWLabel *ZPositionLabel;
  vtkKWLabel *XRadiusLabel;
  vtkKWLabel *YRadiusLabel;
  vtkKWLabel *ZRadiusLabel;

  // ROI visibility
  vtkKWPushButton *VisibilityToggle;
  vtkSlicerVisibilityIcons *VisibilityIcons;

  // ROI colour
  vtkKWChangeColorButton *ROIColorButton;

  // ROI selected ROI colour
  vtkKWChangeColorButton *ROISelectedColorButton;

  // text scale
  vtkKWScaleWithEntry *ROITextScale;

  // opacity
  vtkKWScaleWithEntry *ROIOpacity;

  // display the points on the list
  vtkKWMultiColumnListWithScrollbars *MultiColumnList;

  // the columns that hold the
  // name, x, y, z, selected
  int NumberOfColumns;

  //BTX
  // Description:
  // The column orders in the list box
  enum
    {
    NameColumn = 0,
    SelectedColumn = 1,
    XColumn = 2,
    YColumn = 3,
    ZColumn = 4,
    RadiusXColumn = 5,
    RadiusYColumn = 6,
    RadiusZColumn = 7,
    };
  //ETX


private:
  vtkSlicerROIGUI ( const vtkSlicerROIGUI& ); // Not implemented.
  void operator = ( const vtkSlicerROIGUI& ); //Not implemented.
};
#endif
