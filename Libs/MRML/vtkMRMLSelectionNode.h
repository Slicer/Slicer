/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLSelectionNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
// .NAME vtkMRMLSelectionNode - MRML node for storing a slice through RAS space
// .SECTION Description
// This node stores the information about the currently selected volume
// 
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

  // Description:
  // Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "Selection";};

  // Description:
  // Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  // Description:
  // Updates this node if it depends on other nodes 
  // when the node is deleted in the scene
  virtual void UpdateReferences();

  // Note: the SetReferenceActive* routines are added because
  // the vtkSetReferenceStringMacro is not wrapped (vtkSetStringMacro
  // on which it is based is a special case in vtk's parser).

  // Description:
  // the ID of a MRMLVolumeNode
  vtkGetStringMacro (ActiveVolumeID);
  vtkSetReferenceStringMacro (ActiveVolumeID);
  void SetReferenceActiveVolumeID (char *id) { this->SetActiveVolumeID(id); };

  // Description:
  // the ID of a MRMLVolumeNode
  vtkGetStringMacro (ActiveLabelVolumeID);
  vtkSetReferenceStringMacro (ActiveLabelVolumeID);
  void SetReferenceActiveLabelVolumeID (char *id) { this->SetActiveLabelVolumeID(id); };

  // Description:
  // the ID of a MRMLFiducialList
  vtkGetStringMacro (ActiveFiducialListID);
  vtkSetReferenceStringMacro (ActiveFiducialListID);
  void SetReferenceActiveFiducialListID (char *id) { this->SetActiveFiducialListID(id); };

  // Description:
  // the ID of a MRMLROIList
  vtkGetStringMacro (ActiveROIListID);
  vtkSetReferenceStringMacro (ActiveROIListID);
  void SetReferenceActiveROIListID (char *id) { this->SetActiveROIListID(id); };

  // Description:
  // the ID of a MRMLCameraNode
  vtkGetStringMacro (ActiveCameraID );
  vtkSetReferenceStringMacro ( ActiveCameraID );
  void SetReferenceActiveCameraID (char *id) { this->SetActiveCameraID(id); };
  
  // Description
  // the ID of a MRMLViewNode
  vtkGetStringMacro (ActiveViewID );
  vtkSetReferenceStringMacro ( ActiveViewID );
  void SetReferenceActiveViewID (char *id) { this->SetActiveViewID(id); };
  
  // Description
  // the ID of a MRMLLayoutNode
  vtkGetStringMacro (ActiveLayoutID );
  vtkSetReferenceStringMacro ( ActiveLayoutID );
  void SetReferenceActiveLayoutID (char *id) { this->SetActiveLayoutID(id); };

protected:
  vtkMRMLSelectionNode();
  ~vtkMRMLSelectionNode();
  vtkMRMLSelectionNode(const vtkMRMLSelectionNode&);
  void operator=(const vtkMRMLSelectionNode&);

  char *ActiveVolumeID;
  char *ActiveLabelVolumeID;
  char *ActiveFiducialListID;
  char *ActiveROIListID;
  char *ActiveCameraID;
  char *ActiveViewID;
  char *ActiveLayoutID;
  
};

#endif

