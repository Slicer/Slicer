/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: QtSlicerNodeSelectorWidget.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

// .NAME QtSlicerNodeSelectorWidget - menu to select volumes from current mrml scene
// .SECTION Description
// Inherits most behavior from kw widget, but is specialized to observe
// the current mrml scene and update the entries of the pop up menu to correspond
// to the currently available volumes.  This widget also has a notion of the current selection
// that can be observed or set externally
//


#ifndef __QtSlicerNodeSelectorWidget_h
#define __QtSlicerNodeSelectorWidget_h

#include <QComboBox>

#include "vtkCallbackCommand.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"

#include "vtkSlicerBaseGUIQtWin32Header.h"
#include "vtkSlicerContextMenuHelper.h"

class VTK_SLICER_BASE_GUI_QT_EXPORT QtSlicerNodeSelectorWidget : public QComboBox 
{
  Q_OBJECT
public:

  QtSlicerNodeSelectorWidget();
  virtual ~QtSlicerNodeSelectorWidget();
  
  // Description:
  // this is the scene to observe
  vtkMRMLScene* GetMRMLScene() 
    {
      return this->MRMLScene;
    };
  
  void SetScene(vtkMRMLScene *scene)
    {
      this->MRMLScene = scene;
    };

  void SetMRMLScene(vtkMRMLScene *scene);

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
  int GetNewNodeEnabled() 
    {
      return this->NewNodeEnabled;
    };
  void SetNewNodeEnabled(int value)
    {
      this->NewNodeEnabled = value;
    }

  // Description:
  // Specifies whether or not None can be selected
  int GetNoneEnabled() 
    {
      return this->NoneEnabled;
    };
  void SetNoneEnabled(int value)
    {
      this->NoneEnabled = value;
    }

  // Description:
  // Specifies whether or not show hidden nodes
  int GetShowHidden() 
    {
      return this->ShowHidden;
    };
  void SetShowHidden(int value)
    {
      this->ShowHidden = value;
    }

  // Description:
  // Specifies whether or not display child classes of a specified class
  // Default: true
  int GetChildClassesEnabled() 
    {
      return this->ChildClassesEnabled;
    };
  void SetChildClassesEnabled(int value)
    {
      this->ChildClassesEnabled = value;
    }


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
      NodeRenameEvent = 11002,
    };
//ETX

  // Description:
  // Reflect the state of the mrml scene in the menu
  // - UpdateMenu only updates if widget is mapped
  // - UnconditionalUpdateMenu updates even if not mapped
  void UnconditionalUpdateMenu();

  //vtkSetObjectMacro(ContextMenuHelper, vtkSlicerContextMenuHelper);
  //vtkGetObjectMacro(ContextMenuHelper, vtkSlicerContextMenuHelper);

  // Description:
  // Remove all menu entries
  void ClearMenu();

  void ProcessNewNodeCommand(const char *className, const char *nodeName);

  bool CheckNodeClass(vtkMRMLNode *node);

  // Description:
  // Flags to avoid event loops
  // NOTE: don't use the SetMacro or it call modified itself and generate even more events!
  int GetInMRMLCallbackFlag()
    {
    return this->InMRMLCallbackFlag;
    };

  void SetInMRMLCallbackFlag (int flag) {
    this->InMRMLCallbackFlag = flag;
  };

public slots:

  void Select(const QString &id);

  void UpdateMenu();

Q_SIGNALS:
  void NodeSelected(const QString &id);

protected:

  // Description:
  // Create the widget
  virtual void CreateWidget();

//BTX
  std::vector<std::string> NodeClasses;
  std::vector<std::string> AttributeNames;
  std::vector<std::string> AttributeValues;
  std::vector<std::string> NodeNames;

  std::map<std::string, std::string> NodeID_to_EntryName;
  std::map<QString, std::string> EntryName_to_NodeID;
  std::map<std::string, std::string> ClassTag_to_Class;
  std::map<std::string, std::string> ClassTag_to_ClassName;
  std::string MakeEntryName(vtkMRMLNode *node);
  std::string FindEntryName(vtkMRMLNode *node);

  std::string SelectedID;
//ETX

  int NewNodeEnabled;

  int NoneEnabled;

  int ShowHidden;

  int ChildClassesEnabled;

  vtkMRMLScene       *MRMLScene;
  vtkSlicerContextMenuHelper      *ContextMenuHelper;

private:

  int NewNodeCount;

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

  QtSlicerNodeSelectorWidget(const QtSlicerNodeSelectorWidget&); // Not implemented
  void operator=(const QtSlicerNodeSelectorWidget&); // Not Implemented
};

#endif

