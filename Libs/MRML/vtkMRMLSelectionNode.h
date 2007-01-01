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
  // the ID of a MRMLVolumeNode
  vtkGetStringMacro (ActiveVolumeID);
  vtkSetReferenceStringMacro (ActiveVolumeID);

  // Description:
  // the ID of a MRMLVolumeNode
  vtkGetStringMacro (ActiveLabelVolumeID);
  vtkSetReferenceStringMacro (ActiveLabelVolumeID);

  // Description:
  // the ID of a MRMLFiducialList
  vtkGetStringMacro (ActiveFiducialListID);
  vtkSetReferenceStringMacro (ActiveFiducialListID);

  // Description:
  // the ID of a MRMLCameraNode
  vtkGetStringMacro (ActiveCameraID );
  vtkSetReferenceStringMacro ( ActiveCameraID );
  
  // Description
  // the ID of a MRMLViewNode
  vtkGetStringMacro (ActiveViewID );
  vtkSetReferenceStringMacro ( ActiveViewID );
  
  // Description:
  // Allows the mouse mode to be set and queried.
  vtkGetMacro (MouseInteractionMode, int );
  char *GetMouseInteractionModeAsString();
  char *GetMouseInteractionModeAsString(int mode);
  
  //vtkSetMacro (MouseInteractionMode, int );
  // Description:
  // Set the mouse mode and update the interactor
  void SetMouseInteractionMode(int mode);

  
  //BTX
  // Modes for mouse control in the Main Viewer (and Slice Viewers?)
  enum
  {
    MouseSelect = 0,
    MouseTransform,
    MousePut
  };
  //ETX
  
protected:
  vtkMRMLSelectionNode();
  ~vtkMRMLSelectionNode();
  vtkMRMLSelectionNode(const vtkMRMLSelectionNode&);
  void operator=(const vtkMRMLSelectionNode&);

  char *ActiveVolumeID;
  char *ActiveLabelVolumeID;
  char *ActiveFiducialListID;
  char *ActiveCameraID;
  char *ActiveViewID;

  // Description:
  // Allows the mouse to select and to deposit
  // things in the 3D viewer. 
  int MouseInteractionMode;
};

#endif

