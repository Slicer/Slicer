/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
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

// VolumeRendering includes
#include "vtkSlicerVolumeRenderingModuleMRMLExport.h"

// MRML includes
#include "vtkMRMLNode.h"

/// \ingroup Slicer_QtModules_VolumeRendering
class VTK_SLICER_VOLUMERENDERING_MODULE_MRML_EXPORT vtkMRMLVolumeRenderingScenarioNode : public vtkMRMLNode
{
  public:
  static vtkMRMLVolumeRenderingScenarioNode *New();
  vtkTypeMacro(vtkMRMLVolumeRenderingScenarioNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

  virtual vtkMRMLNode* CreateNodeInstance() VTK_OVERRIDE;

  // Description:
  // Set node attributes
  virtual void ReadXMLAttributes( const char** atts) VTK_OVERRIDE;

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent) VTK_OVERRIDE;

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node) VTK_OVERRIDE;

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() VTK_OVERRIDE {return "VolumeRenderingScenario";}

  // Description:
  // Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID) VTK_OVERRIDE;

  // Description:
  // Updates this node if it depends on other nodes
  // when the node is deleted in the scene
  virtual void UpdateReferences() VTK_OVERRIDE;

  // Description:
  // Observe the reference transform node
  virtual void UpdateScene(vtkMRMLScene *scene) VTK_OVERRIDE;

  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData) VTK_OVERRIDE;

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

