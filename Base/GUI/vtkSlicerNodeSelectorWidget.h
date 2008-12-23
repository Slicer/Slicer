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


#ifndef __vtkSlicerNodeSelectorWidget_h
#define __vtkSlicerNodeSelectorWidget_h

#include "vtkCallbackCommand.h"

#include "vtkKWMenuButtonWithSpinButtonsWithLabel.h"
#include "vtkKWMenuButtonWithSpinButtons.h"
#include "vtkKWMenuButton.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"

#include "vtkSlicerBaseGUI.h"

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerNodeSelectorWidget : public vtkKWMenuButtonWithSpinButtonsWithLabel
{
public:
  static vtkSlicerNodeSelectorWidget* New();
  vtkTypeRevisionMacro(vtkSlicerNodeSelectorWidget,vtkKWMenuButtonWithSpinButtonsWithLabel);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // this is the scene to observe
  vtkGetObjectMacro(MRMLScene, vtkMRMLScene);
  void SetMRMLScene(vtkMRMLScene *MRMLScene);

  // Description:
  // Set class name of this node to select
  // also sets:
  // attribute name-value pair to use as additional node filter (optional)
  // nodeName to use as a base name for creating new nodes (optional)
  void SetNodeClass(const char *className, const char *attName, const char* attValue, const char *nodeName);

  // Description:
  // Add class name of this node to select
  // also sets:
  // attribute name-value pair to use as additional node filter (optional)
  // nodeName to use as a base name for creating new nodes (optional)
  void AddNodeClass(const char *className, const char *attName, const char* attValue, const char *nodeName);

  // Description:
  // Get a n-th class name of this node to select
  const char* GetNodeClass(int ind) {
    return NodeClasses[ind].c_str();
  };

  // Description:
  // Get a n-th base name of this node to select
  const char* GetNodeName(int ind) {
    return NodeNames[ind].c_str();
  };

  // Description:
  // Get a n-th attribute name of this node to select
  const char* GetNodeAttributeName(int ind) {
    return AttributeNames[ind]==std::string("") ? NULL : AttributeNames[ind].c_str();
  };

  // Description:
  // Get a n-th attribute name of this node to select
  const char* GetNodeAttributeValue(int ind) {
    return AttributeValues[ind]==std::string("") ? NULL : AttributeValues[ind].c_str();
  };

  // Description:
  // Get a number of class names to select
  int GetNumberOfNodeClasses() {
    return NodeClasses.size();
  };

  // Description:
  // Specifies whether new node creation is enabled
  vtkBooleanMacro(NewNodeEnabled, int);
  vtkGetMacro(NewNodeEnabled, int);
  vtkSetMacro(NewNodeEnabled, int);
  
  // Description:
  // Specifies whether or not None can be selected
  vtkBooleanMacro(NoneEnabled, int);
  vtkGetMacro(NoneEnabled, int);
  vtkSetMacro(NoneEnabled, int);

  // Description:
  // Specifies whether or not show hidden nodes
  vtkBooleanMacro(ShowHidden, int);
  vtkGetMacro(ShowHidden, int);
  vtkSetMacro(ShowHidden, int);

  // Description:
  // Specifies whether or not display child classes of a specified class
  // Default: true
  vtkBooleanMacro(ChildClassesEnabled, int);
  vtkGetMacro(ChildClassesEnabled, int);
  vtkSetMacro(ChildClassesEnabled, int);


  // Description
  // Get selected node
  vtkMRMLNode *GetSelected();

  // Description
  // Set selected node
  void SetSelected(vtkMRMLNode *node);

  // Description
  // Get selection to new node of n-th class name
  void SetSelectedNew(const char *className);

//BTX
  enum
    {
      NodeSelectedEvent = 11000,
      NewNodeEvent = 11001,
    };
//ETX

  // Description:
  // Reflect the state of the mrml scene in the menu
  void UpdateMenu();

  // Description:
  // Remove all menu entries
  void ClearMenu();

  void ProcessNewNodeCommand(const char *className, const char *nodeName);
  void ProcessCommand(char *slectedId);

  bool CheckNodeClass(vtkMRMLNode *node);

  // Description:
  // Flags to avoid event loops
  // NOTE: don't use the SetMacro or it call modified itself and generate even more events!
  vtkGetMacro(InMRMLCallbackFlag, int);
  void SetInMRMLCallbackFlag (int flag) {
    this->InMRMLCallbackFlag = flag;
  }

protected:
  vtkSlicerNodeSelectorWidget();
  virtual ~vtkSlicerNodeSelectorWidget();

//BTX
  std::vector<std::string> NodeClasses;
  std::vector<std::string> AttributeNames;
  std::vector<std::string> AttributeValues;
  std::vector<std::string> NodeNames;

  std::map<std::string, std::string> NodeID_to_EntryName;
  std::string MakeEntryName(vtkMRMLNode *node);
  std::string FindEntryName(vtkMRMLNode *node);

  std::string SelectedID;
//ETX
  
  int NewNodeEnabled;

  int NoneEnabled;

  int ShowHidden;

  int ChildClassesEnabled;

  vtkMRMLScene       *MRMLScene;

private:

  int NewNodeCount;

  vtkCallbackCommand *MRMLCallbackCommand;

  // Description:
  // Flag to avoid event loops
  int InMRMLCallbackFlag;

  // Description:
  // Check for additional conditions to filter the NodeSelector. node is the current node to be checked.
  // Returns true if the condition is fullfilled
  // Note: This method has only effects if overwritten in a sub class.
  virtual bool CheckAdditionalConditions(vtkMRMLNode* node){return true;}

  // Description:
  // Add aditional Nodes to the current NodeSelector. Useful for presets, other MRML-Scenes etc.
  // Returns the number of added items or 0 if no item was added.
  // Note: This method has only effects if overwritten in a sub class. Also overwrite GetSelectedInAdditional().
  virtual int AddAditionalNodes(){return 0;}

  // Description
  // Look for corresponding node for the SelectedID in additional nodes.
  // Returns NULL if there is no corresponding node otherwise the corresponding node
  // Note: This method has only effects if overwritten in a sub class. Also overwrite AddAditionalNodes().
  virtual vtkMRMLNode* GetSelectedInAdditional(){return NULL;}

  vtkSlicerNodeSelectorWidget(const vtkSlicerNodeSelectorWidget&); // Not implemented
  void operator=(const vtkSlicerNodeSelectorWidget&); // Not Implemented
};

#endif

