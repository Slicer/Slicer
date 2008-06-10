/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLVolumeRenderingSelectionNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
// .NAME vtkMRMLVolumeRenderingSelectionNode - MRML node for storing a slice through RAS space
// .SECTION Description
// This node stores the information about the currently selected volume
// 
//

#ifndef __vtkMRMLVolumeRenderingSelectionNode_h
#define __vtkMRMLVolumeRenderingSelectionNode_h

#include "vtkMRML.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkVolumeRendering.h"

#include "vtkMatrix4x4.h"

class VTK_SLICERVOLUMERENDERING_EXPORT vtkMRMLVolumeRenderingSelectionNode : public vtkMRMLNode
{
  public:
  static vtkMRMLVolumeRenderingSelectionNode *New();
  vtkTypeMacro(vtkMRMLVolumeRenderingSelectionNode,vtkMRMLNode);
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
  virtual const char* GetNodeTagName() {return "VolumeRenderingSelection";};

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
  // the ID of a parameter MRMLVolumeRenderingNode
  vtkGetStringMacro (ActiveVolumeRenderingID);
  vtkSetReferenceStringMacro (ActiveVolumeRenderingID);
  void SetReferenceActiveVolumeRenderingID (char *id) { this->SetActiveVolumeRenderingID(id); };

protected:
  vtkMRMLVolumeRenderingSelectionNode();
  ~vtkMRMLVolumeRenderingSelectionNode();
  vtkMRMLVolumeRenderingSelectionNode(const vtkMRMLVolumeRenderingSelectionNode&);
  void operator=(const vtkMRMLVolumeRenderingSelectionNode&);

  char *ActiveVolumeID;
  char *ActiveVolumeRenderingID;
};

#endif

