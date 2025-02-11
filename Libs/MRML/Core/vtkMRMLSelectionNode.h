/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLSelectionNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkMRMLSelectionNode_h
#define __vtkMRMLSelectionNode_h

// MRML includes
#include "vtkMRMLNode.h"

class vtkMRMLUnitNode;

// STD includes
#include <vector>

/// \brief  MRML node for storing information about the active nodes in the scene.
///
/// This node stores the information about the currently selected volume,
/// label volume, fiducial list, place node class name, place node id, ROI
/// list, camera, table, view, layout, units
/// Note: the SetReferenceActive* routines are added because
/// the vtkSetReferenceStringMacro is not wrapped (vtkSetStringMacro
/// on which it is based is a special case in vtk's parser).
class VTK_MRML_EXPORT vtkMRMLSelectionNode : public vtkMRMLNode
{
  public:
  static vtkMRMLSelectionNode *New();
  vtkTypeMacro(vtkMRMLSelectionNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  /// Set node attributes
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy the node's attributes to this object
  void Copy(vtkMRMLNode *node) override;

  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "Selection";}

  /// the ID of a MRMLVolumeNode (typically background)
  const char* GetActiveVolumeID();
  void SetActiveVolumeID(const char* id);
  /// \deprecated Use SetActiveVolumeID instead
  void SetReferenceActiveVolumeID (const char *id) { this->SetActiveVolumeID(id); };

  /// the ID of a MRMLVolumeNode (typically foreground)
  const char* GetSecondaryVolumeID();
  void SetSecondaryVolumeID(const char* id);
  /// \deprecated Use SetSecondaryVolumeID instead
  void SetReferenceSecondaryVolumeID (char *id) { this->SetSecondaryVolumeID(id); };

  /// the ID of a MRMLVolumeNode
  const char* GetActiveLabelVolumeID();
  void SetActiveLabelVolumeID(const char* id);
  /// \deprecated Use SetActiveLabelVolumeID instead
  void SetReferenceActiveLabelVolumeID (const char *id) { this->SetActiveLabelVolumeID(id); };

  /// Get the classname of the active placeNode type.
  /// The active placeNode is used to control what placeNode is being
  /// dropped by the user.
  /// \sa SetActivePlaceNodeClassName, SetReferenceActivePlaceNodeClassName
  vtkGetStringMacro (ActivePlaceNodeClassName);
  /// Set the classname of the active placeNode type.
  /// Use SetReferenceActivePlaceNodeClassName if you need the mouse mode tool
  /// bar to update.
  /// \sa GetActivePlaceNodeClassName, SetReferenceActivePlaceNodeClassName
  vtkSetStringMacro(ActivePlaceNodeClassName);
  /// Set the active placeNode class name and fire the event
  /// ActivePlaceNodeClassNameChangedEvent so that the Mouse mode tool bar
  /// will update.
  /// \sa GetActivePlaceNodeClassName, SetActivePlaceNodeClassName
  void SetReferenceActivePlaceNodeClassName (const char *className);

  /// Get the ID of the currently active placeNode.
  /// \sa SetActivePlaceNodeID, SetReferenceActivePlaceNodeID
  const char* GetActivePlaceNodeID();
  /// Set the ID of the currently active placeNode.
  /// \sa GetActivePlaceNodeID, SetReferenceActivePlaceNodeID
  void SetActivePlaceNodeID(const char* id);
  /// Set the ID of the currently active placeNode and fire the
  /// ActivePlaceNodeIDChangedEvent event.
  /// \sa GetActivePlaceNodeID, SetActivePlaceNodeID
  void SetReferenceActivePlaceNodeID (const char *id)
  { this->SetActivePlaceNodeID(id);
    this->InvokeEvent(vtkMRMLSelectionNode::ActivePlaceNodeIDChangedEvent); };

  /// the ID of a MRMLROIList
  const char* GetActiveROIListID();
  void SetActiveROIListID(const char* id);
  /// \deprecated Use SetActiveROIListID instead
  void SetReferenceActiveROIListID (const char *id) { this->SetActiveROIListID(id); };

  /// the ID of a MRMLCameraNode
  const char* GetActiveCameraID();
  void SetActiveCameraID(const char* id);
  /// \deprecated Use SetActiveCameraID instead
  void SetReferenceActiveCameraID (const char *id) { this->SetActiveCameraID(id); };

  /// the ID of a MRMLTableNode
  const char* GetActiveTableID();
  void SetActiveTableID(const char* id);
  /// \deprecated Use SetActiveTableID instead
  void SetReferenceActiveTableID (char *id) { this->SetActiveTableID(id); };

  /// the ID of a MRMLViewNode
  const char* GetActiveViewID();
  void SetActiveViewID(const char* id );
  /// \deprecated Use SetActiveViewID instead
  void SetReferenceActiveViewID (const char *id) { this->SetActiveViewID(id); };

  /// the ID of a MRMLLayoutNode
  const char* GetActiveLayoutID();
  void SetActiveLayoutID(const char* id);
  /// \deprecated Use SetActiveLayoutID instead
  void SetReferenceActiveLayoutID (const char *id) { this->SetActiveLayoutID(id); };

  /// the ID of a MRMLPlotChartNode
  const char* GetActivePlotChartID();
  void SetActivePlotChartID(const char* id);
  /// \deprecated Use SetActivePlotChartID instead
  void SetReferenceActivePlotChartID (const char *id) { this->SetActivePlotChartID(id); };

  /// A list of events that this node can throw
  /// ActivePlaceNodeIDChangedEvent: is no longer observed by the Mouse mode
  /// tool bar, it only watches for the ActivePlaceNodeClassNameChangedEvent
  /// ActivePlaceNodeClassNameChangedEvent: is observed by the Mouse mode tool
  /// bar class to update that widget to the current place node
  /// PlaceNodeClassNameListModifiedEvent: this is fired when new place node
  /// class names are added, watched for by the Mouse mode tool bar so that it
  /// can offer the user all the valid types of nodes to place.
  /// UnitModifiedEvent: Fired every time a quantity unit node is changed
  /// or an active quantity unit node is modified. The calldata contains
  /// the node quantity
  /// \sa AddNewPlaceNodeClassNameToList
  enum
  {
    ActivePlaceNodeIDChangedEvent = 19001,
    ActivePlaceNodeClassNameChangedEvent,
    PlaceNodeClassNameListModifiedEvent,
    UnitModifiedEvent,
    ActivePlaceNodePlacementValidEvent
  };

  /// Add a new valid placeNode class name to the list, with optional qt resource
  /// reference string for updating GUI elements
  void AddNewPlaceNodeClassNameToList(const char *newID, const char *resource = nullptr, const char *iconName = "");

  // -- Units --

  /// Set/Get the current unit node associated with the given quantity.
  /// This is how the GUI or the logic can access the current node for
  /// a quantity. Changing the current node for a given quantity should only
  /// be done through the unit node settings panel.
  /// There can be no node (i.e. nullptr) associated to a quantity.
  /// To make sure to have the correct unit node, one should observe the
  /// selection node for UnitModifiedEvent.
  /// \sa GetUnitNode(), GetNodeReferenceID(), SetAndObserveNodeReferenceID()
  /// \sa UnitModifiedEvent
  const char* GetUnitNodeID(const char* quantity);
  void SetUnitNodeID(const char* quantity, const char* id);

  /// Return the unit node associated to the quantity.
  /// \sa GetUnitNodeID()
  vtkMRMLUnitNode* GetUnitNode(const char* quantity);

  /// Get all the unit node currently observed by the selection node.
  /// \sa GetReferenceNodes()
  /// \sa GetUnitNodeID(), SetUnitNodeID(), GetUnitNodeIDs()
  void GetUnitNodes(std::vector<vtkMRMLUnitNode*>& units);

  /// Get all the unit node IDs currently observed by the selection node.
  /// \sa GetUnitNodes()
  void GetUnitNodeIDs(std::vector<const char*>& quantities,
                      std::vector<const char*>& unitIDs);

  /// Method to propagate events generated in units nodes.
  /// \sa GetNodeReferenceID(), SetAndObserveNodeReferenceID()
  /// \sa UnitModifiedEvent
  void ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData) override;

  /// Remove a placeNode class name from the list
  /// \sa PlaceNodeClassNameInList
  void RemovePlaceNodeClassNameFromList(const char *className);
  /// Return nth placeNode class name string from the list,
  /// empty string if out of bounds
  std::string GetPlaceNodeClassNameByIndex(int n);
  /// Return nth placeNode resource string from the list,
  /// empty string if out of bounds
  std::string GetPlaceNodeResourceByIndex(int n);
  /// Return nth placeNode icon name string from the list,
  /// empty string if out of bounds
  std::string GetPlaceNodeIconNameByIndex(int n);

  /// Check for an classname in the list, returning it's index, -1 if not in list
  int PlaceNodeClassNameInList(std::string className);
  /// Return the placeNode resource associated with this classname, empty string if
  /// not found
  /// \sa vtkMRMLSelectionNode::PlaceNodeClassNameInList
  /// \sa vtkMRMLSelectionNode::GetPlaceNodeResourceByIndex
  std::string GetPlaceNodeResourceByClassName(std::string className);
  /// Get the number of class names in the list
  int GetNumberOfPlaceNodeClassNamesInList() { return static_cast<int>(this->PlaceNodeClassNameList.size()); };

  /// Set whether point placement is valid for the active placement node
  void SetActivePlaceNodePlacementValid(bool valid);
  /// Returns true if additional points can be placed in the current placement node, false if
  /// the node has a locked number of points.
  bool GetActivePlaceNodePlacementValid();

protected:
  vtkMRMLSelectionNode();
  ~vtkMRMLSelectionNode() override;
  vtkMRMLSelectionNode(const vtkMRMLSelectionNode&);
  void operator=(const vtkMRMLSelectionNode&);

  char *ActivePlaceNodeClassName;
  bool ActivePlaceNodePlacementValid;

  std::vector<std::string> PlaceNodeClassNameList;
  std::vector<std::string> PlaceNodeResourceList;
  std::vector<std::string> PlaceNodeIconNameList;
};

#endif
