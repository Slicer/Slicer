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

/// \brief  MRML node for storing a slice through RAS space.
///
/// This node stores the information about the currently selected volume.
class VTK_MRML_EXPORT vtkMRMLSelectionNode : public vtkMRMLNode
{
  public:
  static vtkMRMLSelectionNode *New();
  vtkTypeMacro(vtkMRMLSelectionNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  /// 
  /// Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  /// 
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// 
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// 
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "Selection";};

  /// Set the nodes as references to the current scene.
  virtual void SetSceneReferences();

  /// 
  /// Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  /// 
  /// Updates this node if it depends on other nodes 
  /// when the node is deleted in the scene
  virtual void UpdateReferences();

  /// Note: the SetReferenceActive* routines are added because
  /// the vtkSetReferenceStringMacro is not wrapped (vtkSetStringMacro
  /// on which it is based is a special case in vtk's parser).

  /// 
  /// the ID of a MRMLVolumeNode (typically background)
  vtkGetStringMacro (ActiveVolumeID);
  void SetActiveVolumeID(const char* id);
  void SetReferenceActiveVolumeID (const char *id) { this->SetActiveVolumeID(id); };

  /// 
  /// the ID of a MRMLVolumeNode (typically foreground)
  vtkGetStringMacro (SecondaryVolumeID);
  void SetSecondaryVolumeID(const char* id);
  void SetReferenceSecondaryVolumeID (char *id) { this->SetSecondaryVolumeID(id); };

  /// 
  /// the ID of a MRMLVolumeNode
  vtkGetStringMacro (ActiveLabelVolumeID);
  void SetActiveLabelVolumeID(const char* id);
  void SetReferenceActiveLabelVolumeID (const char *id) { this->SetActiveLabelVolumeID(id); };

  /// 
  /// the ID of a MRMLFiducialList
  vtkGetStringMacro (ActiveFiducialListID);
  void SetActiveFiducialListID(const char* id);
  void SetReferenceActiveFiducialListID (const char *id) { this->SetActiveFiducialListID(id); };

  /// Set/Get the classname of the active annotation type.
  /// The active annotation is used to control what annotation is being
  /// dropped by the user.
  vtkGetStringMacro (ActiveAnnotationID);
  void SetActiveAnnotationID(const char* id);
  /// Set the active annotation id and fire the event
  /// ActiveAnnotationIDChangedEvent.
  void SetReferenceActiveAnnotationID (const char *id);


  /// the ID of a MRMLROIList
  vtkGetStringMacro (ActiveROIListID);
  void SetActiveROIListID(const char* id);
  void SetReferenceActiveROIListID (const char *id) { this->SetActiveROIListID(id); };

  /// 
  /// the ID of a MRMLCameraNode
  vtkGetStringMacro (ActiveCameraID );
  void SetActiveCameraID(const char* id);
  void SetReferenceActiveCameraID (const char *id) { this->SetActiveCameraID(id); };
  
  /// Description
  /// the ID of a MRMLViewNode
  vtkGetStringMacro (ActiveViewID );
  void SetActiveViewID(const char* id );
  void SetReferenceActiveViewID (const char *id) { this->SetActiveViewID(id); };
  
  /// Description
  /// the ID of a MRMLLayoutNode
  vtkGetStringMacro (ActiveLayoutID );
  void SetActiveLayoutID(const char* id);
  void SetReferenceActiveLayoutID (const char *id) { this->SetActiveLayoutID(id); };

  /// Description
  /// a list of events that this node can throw
  /// UnitModifiedEvent: Fired everytime a quantity unit node is changed
  /// or an active quantity unit node is modified. The calldata contains
  /// the node quantity
  enum
  {
    ActiveAnnotationIDChangedEvent = 19001,
    AnnotationIDListModifiedEvent,
    UnitModifiedEvent,
  };

  /// Description:
  /// Add a new valid annotation id to the list, with optional qt resource
  /// reference string for updating GUI elements
  void AddNewAnnotationIDToList(const char *newID, const char *resource = NULL);
  /// Description:
  /// remove an annotation from the list
  void RemoveAnnotationIDFromList(const char *id);
  /// Return nth annotation id/resource string from the list, empty string if
  /// out of bounds
  std::string GetAnnotationIDByIndex(int n);
  std::string GetAnnotationResourceByIndex(int n);
  /// Check for an id in the list, returning it's index, -1 if not in list
  int AnnotationIDInList(std::string id);
  /// Return the annotation resource associated with this id, empty string if
  /// not found
  /// \sa vtkMRMLSelectionNode::AnnotationIDInList
  /// \sa vtkMRMLSelectionNode::GetAnnotationResourceFromList
  std::string GetAnnotationResourceByID(std::string id);
  /// Get the number of ids in the list
  int GetNumberOfAnnotationIDsInList() { return static_cast<int>(this->AnnotationIDList.size()); };

  /// -- Units --

  /// Description:
  /// Set/Get the current unit node associated with the given quantity.
  /// This is how the GUI or the logic can access the current node for
  /// a quantity. Changing the current node for a given quantity should only
  /// be done through the unit node settings panel.
  /// There can be no node (i.e. NULL) associated to a quantity.
  /// To make sure to have the correct unit node, one should observe the
  /// selection node for UnitModifiedEvent.
  /// \sa GetNodeReferenceID(), SetAndObserveNodeReferenceID()
  /// \sa UnitModifiedEvent
  const char* GetUnitNodeID(const char* quantity);
  void SetUnitNodeID(const char* quantity, const char* id);

  /// Description:
  /// Get all the unit node currently observed by the selection node.
  /// \sa GetReferenceNodes()
  /// \sa GetUnitNodeID(), SetUnitNodeID()
  void GetUnitNodes(std::vector<vtkMRMLUnitNode*>& units);

  /// Description:
  /// Method to propagate events generated in units nodes.
  /// \sa GetNodeReferenceID(), SetAndObserveNodeReferenceID()
  /// \sa UnitModifiedEvent
  void ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData);

protected:
  vtkMRMLSelectionNode();
  ~vtkMRMLSelectionNode();
  vtkMRMLSelectionNode(const vtkMRMLSelectionNode&);
  void operator=(const vtkMRMLSelectionNode&);

  char *ActiveVolumeID;
  char *SecondaryVolumeID;
  char *ActiveLabelVolumeID;
  char *ActiveFiducialListID;
  char *ActiveAnnotationID;
  char *ActiveROIListID;
  char *ActiveCameraID;
  char *ActiveViewID;
  char *ActiveLayoutID;

  std::vector<std::string> AnnotationIDList;
  std::vector<std::string> AnnotationResourceList;
};

#endif
