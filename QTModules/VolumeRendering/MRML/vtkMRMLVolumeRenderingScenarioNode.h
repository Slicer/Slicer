/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLVolumeRenderingScenarioNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
// .NAME vtkMRMLVolumeRenderingScenarioNode - MRML node for storing selected parameter node id
// .SECTION Description
// This node stores the information about the currently selected parameter node
//
//

#ifndef __vtkMRMLVolumeRenderingScenarioNode_h
#define __vtkMRMLVolumeRenderingScenarioNode_h

#include "vtkMRML.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLROINode.h"
#include "qSlicerVolumeRenderingModuleExport.h"
#include "vtkMRMLVolumePropertyNode.h"

#include "qSlicerVolumeRenderingModuleExport.h"


#include "vtkMatrix4x4.h"

class Q_SLICER_QTMODULES_VOLUMERENDERING_EXPORT vtkMRMLVolumeRenderingScenarioNode : public vtkMRMLNode
{
  public:
  static vtkMRMLVolumeRenderingScenarioNode *New();
  vtkTypeMacro(vtkMRMLVolumeRenderingScenarioNode,vtkMRMLNode);
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
  virtual const char* GetNodeTagName() {return "VolumeRenderingScenario";};

  // Description:
  // Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  // Description:
  // Updates this node if it depends on other nodes
  // when the node is deleted in the scene
  virtual void UpdateReferences();

  // Description:
  // Observe the reference transform node
  virtual void UpdateScene(vtkMRMLScene *scene);

  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData);

  // Description:
  // the ID of a MRMLVolumeRenderingParametersNode
  vtkGetStringMacro (ParametersNodeID);
  vtkSetStringMacro (ParametersNodeID);

protected:
  vtkMRMLVolumeRenderingScenarioNode();
  ~vtkMRMLVolumeRenderingScenarioNode();
  vtkMRMLVolumeRenderingScenarioNode(const vtkMRMLVolumeRenderingScenarioNode&);
  void operator=(const vtkMRMLVolumeRenderingScenarioNode&);

  char *ParametersNodeID;
};

#endif

