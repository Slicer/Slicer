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


#ifndef __vtkSlicerNodeSelectorWidget_h
#define __vtkSlicerNodeSelectorWidget_h

#include "vtkCallbackCommand.h"

#include "vtkKWMenuButtonWithSpinButtonsWithLabel.h"
#include "vtkKWMenuButtonWithSpinButtons.h"
#include "vtkKWMenuButton.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"

#include "vtkSlicerBaseGUI.h"
#include "vtkSlicerContextMenuHelper.h"

class vtkSlicerColorLUTIcons;
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerNodeSelectorWidget : public vtkKWMenuButtonWithSpinButtonsWithLabel
{
public:
  static vtkSlicerNodeSelectorWidget* New();
  vtkTypeRevisionMacro(vtkSlicerNodeSelectorWidget,vtkKWMenuButtonWithSpinButtonsWithLabel);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// 
  /// this is the scene to observe
  vtkGetObjectMacro(MRMLScene, vtkMRMLScene);
  void SetMRMLScene(vtkMRMLScene *MRMLScene);

  /// 
  /// Set class name of this node to select
  /// also sets:
  /// attribute name-value pair to use as additional node filter (optional)
  /// nodeName to use as a base name for creating new nodes (optional)
  void SetNodeClass(const char *className, const char *attName, const char* attValue, const char *nodeName);

  /// 
  /// Add class name of this node to select
  /// also sets:
  /// attribute name-value pair to use as additional node filter (optional)
  /// nodeName to use as a base name for creating new nodes (optional)
  void AddNodeClass(const char *className, const char *attName, const char* attValue, const char *nodeName);

  /// 
  /// Get a n-th class name of this node to select
  const char* GetNodeClass(int ind) {
    return NodeClasses[ind].c_str();
  };

  /// 
  /// Get a n-th base name of this node to select
  const char* GetNodeName(int ind) {
    return NodeNames[ind].c_str();
  };

  /// 
  /// Get a n-th attribute name of this node to select
  const char* GetNodeAttributeName(int ind) {
    return AttributeNames[ind]==std::string("") ? NULL : AttributeNames[ind].c_str();
  };

  /// 
  /// Get a n-th attribute name of this node to select
  const char* GetNodeAttributeValue(int ind) {
    return AttributeValues[ind]==std::string("") ? NULL : AttributeValues[ind].c_str();
  };

  /// 
  /// Get a number of class names to select
  int GetNumberOfNodeClasses() {
    return (int)NodeClasses.size();
  };

  /// 
  /// Specifies whether new node creation is enabled
  vtkBooleanMacro(NewNodeEnabled, int);
  vtkGetMacro(NewNodeEnabled, int);
  vtkSetMacro(NewNodeEnabled, int);

  /// 
  /// Specifies whether or not None can be selected
  vtkBooleanMacro(NoneEnabled, int);
  vtkGetMacro(NoneEnabled, int);
  vtkSetMacro(NoneEnabled, int);

  /// 
  /// Specifies whether a node selector can autoselect a node as a
  /// default
  vtkBooleanMacro(DefaultEnabled, int);
  vtkGetMacro(DefaultEnabled, int);
  vtkSetMacro(DefaultEnabled, int);

  /// 
  /// Specifies whether or not show hidden nodes
  vtkBooleanMacro(ShowHidden, int);
  vtkGetMacro(ShowHidden, int);
  vtkSetMacro(ShowHidden, int);

  /// 
  /// Specifies whether or not display child classes of a specified class
  /// Default: true
  vtkBooleanMacro(ChildClassesEnabled, int);
  vtkGetMacro(ChildClassesEnabled, int);
  vtkSetMacro(ChildClassesEnabled, int);

  /// 
  /// Add child class to exclude
  void AddExcludedChildClass(const char *className)
    {
    this->ExcludedChildClasses.push_back(className);
    };

  /// 
  /// Get number of excluded child classes 
  int GetNumberOfExcludedChildClasses()
    {
      return (int)this->ExcludedChildClasses.size();
    };

  /// 
  /// Get child class to exclude
  const char* GetExcludedChildClass(int index)
    {
    return this->ExcludedChildClasses[index].c_str();
    };


  /// Description
  /// Get selected node
  vtkMRMLNode *GetSelected();

  /// Description
  /// Set selected node
  void SetSelected(vtkMRMLNode *node);

  /// Description
  /// Get selection to new node of n-th class name
  void SetSelectedNew(const char *className);

//BTX
  enum
    {
      NodeSelectedEvent = 11000,
      NewNodeEvent = 11001,
      NodeRenameEvent = 11002,
      NodeAddedEvent = 11003
    };
//ETX

  /// 
  /// Reflect the state of the mrml scene in the menu
  /// - UpdateMenu only updates if widget is mapped
  /// - UnconditionalUpdateMenu updates even if not mapped
  void UpdateMenu();
  void UnconditionalUpdateMenu();

  /// 
  /// A helper class to manage context menus
  vtkSetObjectMacro(ContextMenuHelper, vtkSlicerContextMenuHelper);
  vtkGetObjectMacro(ContextMenuHelper, vtkSlicerContextMenuHelper);

  /// 
  /// Remove all menu entries
  void ClearMenu();

  void ProcessNewNodeCommand(const char *className, const char *nodeName);
  void ProcessCommand(char *selectedID);

  bool CheckNodeClass(vtkMRMLNode *node);
  /// Returns true if the className is in the list of node classes
  bool HasNodeClass(const char  *className);

  /// If this selector widget has vtkMRMLColorNodes, add color icons
  void AddColorIcons();

  ///
  /// Flags to avoid event loops
  /// NOTE: don't use the SetMacro or it call modified itself and generate even more events!
  vtkGetMacro(InMRMLCallbackFlag, int);
  void SetInMRMLCallbackFlag (int flag) {
    this->InMRMLCallbackFlag = flag;
  }

  /// 
  /// Flags to avoid too many refreshes when the scene is loading
  /// NOTE: don't use the SetMacro or it call modified itself and generate even more events!
  vtkGetMacro(IgnoreNodeAddedEvents, int);
  void SetIgnoreNodeAddedEvents (int flag) {
    this->IgnoreNodeAddedEvents = flag;
  }

protected:
  vtkSlicerNodeSelectorWidget();
  virtual ~vtkSlicerNodeSelectorWidget();



  /// 
  /// Create the widget
  virtual void CreateWidget();

//BTX
  std::vector<std::string> NodeClasses;
  std::vector<std::string> AttributeNames;
  std::vector<std::string> AttributeValues;
  std::vector<std::string> NodeNames;
  std::vector<std::string> ExcludedChildClasses;

  std::map<std::string, std::string> NodeID_to_EntryName;
  std::string MakeEntryName(vtkMRMLNode *node);
  std::string FindEntryName(vtkMRMLNode *node);

  std::string SelectedID;
//ETX

  int NewNodeEnabled;

  int NoneEnabled;

  int DefaultEnabled;

  int ShowHidden;

  int ChildClassesEnabled;

  vtkMRMLScene       *MRMLScene;
  vtkSlicerContextMenuHelper      *ContextMenuHelper;

private:

  int NewNodeCount;

  vtkCallbackCommand *MRMLCallbackCommand;

  /// 
  /// Flag to avoid event loops
  int InMRMLCallbackFlag;

  /// 
  /// Flag to avoid refresh when loading the scene
  int IgnoreNodeAddedEvents;

  /// 
  /// Check for additional conditions to filter the NodeSelector. node is the current node to be checked.
  /// Returns true if the condition is fullfilled
  /// Note: This method has only effects if overwritten in a sub class.
  virtual bool CheckAdditionalConditions(vtkMRMLNode* vtkNotUsed(node) ){return true;}

  /// 
  /// Add aditional Nodes to the current NodeSelector. Useful for presets, other MRML-Scenes etc.
  /// Returns the number of added items or 0 if no item was added.
  /// Note: This method has only effects if overwritten in a sub class. Also overwrite GetSelectedInAdditional().
  virtual int AddAditionalNodes(){return 0;}

  /// Description
  /// Look for corresponding node for the SelectedID in additional nodes.
  /// Returns NULL if there is no corresponding node otherwise the corresponding node
  /// Note: This method has only effects if overwritten in a sub class. Also overwrite AddAditionalNodes().
  virtual vtkMRMLNode* GetSelectedInAdditional(){return NULL;}

  /// pointers to icons for when have color nodes
  vtkSlicerColorLUTIcons *ColorIcons;
   
  vtkSlicerNodeSelectorWidget(const vtkSlicerNodeSelectorWidget&); /// Not implemented
  void operator=(const vtkSlicerNodeSelectorWidget&); /// Not Implemented
};

#endif

