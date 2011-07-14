/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLSelectionNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
///  vtkMRMLSelectionNode - MRML node for storing a slice through RAS space
/// 
/// This node stores the information about the currently selected volume
/// 
//

#ifndef __vtkMRMLSelectionNode_h
#define __vtkMRMLSelectionNode_h

#include "vtkMRML.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"

#include "vtkMatrix4x4.h"

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
  vtkSetReferenceStringMacro (ActiveVolumeID);
  void SetReferenceActiveVolumeID (char *id) { this->SetActiveVolumeID(id); };

  /// 
  /// the ID of a MRMLVolumeNode (typically foreground)
  vtkGetStringMacro (SecondaryVolumeID);
  vtkSetReferenceStringMacro (SecondaryVolumeID);
  void SetReferenceSecondaryVolumeID (char *id) { this->SetSecondaryVolumeID(id); };

  /// 
  /// the ID of a MRMLVolumeNode
  vtkGetStringMacro (ActiveLabelVolumeID);
  vtkSetReferenceStringMacro (ActiveLabelVolumeID);
  void SetReferenceActiveLabelVolumeID (char *id) { this->SetActiveLabelVolumeID(id); };

  /// 
  /// the ID of a MRMLFiducialList
  vtkGetStringMacro (ActiveFiducialListID);
  vtkSetReferenceStringMacro (ActiveFiducialListID);
  void SetReferenceActiveFiducialListID (char *id) { this->SetActiveFiducialListID(id); };

  /// 
  /// the ID of a MRMLAnnotationNode
  vtkGetStringMacro (ActiveAnnotationID);
  vtkSetReferenceStringMacro (ActiveAnnotationID);
  void SetReferenceActiveAnnotationID (char *id) { this->SetActiveAnnotationID(id); };

  ///
  /// the ID of a MRMLROIList
  vtkGetStringMacro (ActiveROIListID);
  vtkSetReferenceStringMacro (ActiveROIListID);
  void SetReferenceActiveROIListID (char *id) { this->SetActiveROIListID(id); };

  /// 
  /// the ID of a MRMLCameraNode
  vtkGetStringMacro (ActiveCameraID );
  vtkSetReferenceStringMacro ( ActiveCameraID );
  void SetReferenceActiveCameraID (char *id) { this->SetActiveCameraID(id); };
  
  /// Description
  /// the ID of a MRMLViewNode
  vtkGetStringMacro (ActiveViewID );
  vtkSetReferenceStringMacro ( ActiveViewID );
  void SetReferenceActiveViewID (char *id) { this->SetActiveViewID(id); };
  
  /// Description
  /// the ID of a MRMLLayoutNode
  vtkGetStringMacro (ActiveLayoutID );
  vtkSetReferenceStringMacro ( ActiveLayoutID );
  void SetReferenceActiveLayoutID (char *id) { this->SetActiveLayoutID(id); this->InvokeEvent(vtkMRMLSelectionNode::ActiveAnnotationIDChangedEvent); };

  /// Description
  /// a list of events that this node can throw
  enum
  {
    ActiveAnnotationIDChangedEvent = 19001,
    AnnotationIDListModifiedEvent,
  };

  /// Description:
  /// Add a new valid annotation id to the list, with optional qt resource
  /// reference string for updating GUI elements
  void AddNewAnnotationIDToList(const char *newID, const char *resource = NULL);
  /// Return nth annotation id/resource string from the list, empty string if
  /// out of bounds
//BTX
  std::string GetAnnotationIDByIndex(int n);
  std::string GetAnnotationResourceByIndex(int n);
  /// Check for an id in the list, returning it's index, -1 if not in list
  int AnnotationIDInList(std::string id);
  /// Return the annotation resource associated with this id, empty string if
  /// not found
  /// \sa vtkMRMLSelectionNode::AnnotationIDInList
  /// \sa vtkMRMLSelectionNode::GetAnnotationResourceFromList
  std::string GetAnnotationResourceByID(std::string id);
//ETX
  /// Get the number of ids in the list
  int GetNumberOfAnnotationIDsInList() { return this->AnnotationIDList.size(); };
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

