/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLVolumeRenderingParametersNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
// .NAME vtkMRMLVolumeRenderingParametersNode - MRML node for storing a slice through RAS space
// .SECTION Description
// This node stores the information about the currently selected volume
//
//

#ifndef __vtkMRMLVectorImageContainerNode_h
#define __vtkMRMLVectorImageContainerNode_h

// TODO: sort the header elements, separate MRML and VTK
// MRML includes
#include "vtkMRML.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkSlicerVectorImageExplorerModuleMRMLExport.h"
#include "vtkDoubleArray.h"

class vtkMRMLAnnotationROINode;
class vtkMRMLVolumeNode;
class vtkMRMLDiffusionWeightedVolumeNode;

/*
  This class is a wrapper around a full-fledged, but too specialized, vector volume represented by DWI node. 
  We will use that node for its display and storage functionality, but will add additional features to store 
  extra metadata and provide an extra interface for exploring this data.
*/

/// \ingroup Slicer_QtModules_VectorImageContainerNode
class VTK_SLICER_VECTORIMAGEEXPLORER_MODULE_MRML_EXPORT vtkMRMLVectorImageContainerNode : public vtkMRMLNode
{
  public:   

  static vtkMRMLVectorImageContainerNode *New();
  vtkTypeMacro(vtkMRMLVectorImageContainerNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  /// Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "MRMLVectorImageContainer";};

  /// Update the stored reference to another node in the scene
  //virtual void UpdateReferenceID(const char *oldID, const char *newID);

  /// Updates this node if it depends on other nodes
  /// when the node is deleted in the scene
  //virtual void UpdateReferences();

  // Description:
  //virtual void UpdateScene(vtkMRMLScene *scene);

  //virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData);

  /*
  vtkMRMLDiffusionWeightedVolumeNode* GetDWVNode();
  void SetDWVNode(vtkMRMLDiffusionWeightedVolumeNode* dwv);
  */

  const std::string GetDWVNodeID();
  void SetDWVNodeID(std::string);

  void SetVectorLabelArray(vtkDoubleArray*);
  const vtkDoubleArray* GetVectorLabelArray();

  const std::string GetVectorLabelName(){ return VectorLabelName;}
  void SetVectorLabelName(std::string n){ VectorLabelName = n;}

protected:
  vtkMRMLVectorImageContainerNode();
  ~vtkMRMLVectorImageContainerNode();
  vtkMRMLVectorImageContainerNode(const vtkMRMLVectorImageContainerNode&);
  void operator=(const vtkMRMLVectorImageContainerNode&);


  vtkDoubleArray *VectorLabelArray;
  std::string VectorLabelName;
  std::string DWVNodeID;
};

#endif

