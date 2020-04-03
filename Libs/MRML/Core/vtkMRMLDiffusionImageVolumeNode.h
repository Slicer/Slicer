/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLVolumeNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/

#ifndef __vtkMRMLDiffusionImageVolumeNode_h
#define __vtkMRMLDiffusionImageVolumeNode_h

#include "vtkMRMLTensorVolumeNode.h"
class vtkMRMLDiffusionWeightedVolumeNode;

/// \brief MRML node for representing diffusion weighted MRI volume
///
/// Diffusion Weigthed Volume nodes describe data sets that encode diffusion weigthed
/// images. These images are the basis for computing the diffusion tensor.
/// The node is a container for the necessary information to interpert DW images:
/// 1. Gradient information.
/// 2. B value for each gradient.
/// 3. Measurement frame that relates the coordinate system where the gradients are given
///  to RAS.
class VTK_MRML_EXPORT vtkMRMLDiffusionImageVolumeNode : public vtkMRMLTensorVolumeNode
{
  public:
  static vtkMRMLDiffusionImageVolumeNode *New();
  vtkTypeMacro(vtkMRMLDiffusionImageVolumeNode,vtkMRMLTensorVolumeNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Set node attributes
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentDefaultMacro(vtkMRMLLabelMapVolumeDisplayNode);

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "DiffusionImageVolume";}

 /// Description:
  /// String ID of the storage MRML node
  void SetBaselineNodeID(const char* id);
  vtkGetStringMacro(BaselineNodeID);

  ///
  /// String ID of the display MRML node
  void SetMaskNodeID(const char* id);
  vtkGetStringMacro(MaskNodeID);

 /// Description:
  /// String ID of the display MRML node
  void SetDiffusionWeightedNodeID(const char* id);
  vtkGetStringMacro(DiffusionWeightedNodeID);

  ///
  /// Associated volume MRML node
  vtkMRMLVolumeNode* GetBaselineNode();

  ///
  /// Associated volume MRML node
  vtkMRMLVolumeNode* GetMaskNode();

  ///
  /// Associated volume MRML node
  vtkMRMLDiffusionWeightedVolumeNode* GetDiffusionWeightedNode();

  ///
  /// Associated volume MRML node
  //vtkMRMLDiffusionImageVolumeDisplayNode* GetDisplayNode();

  ///
  /// Update the stored reference to another node in the scene
  void UpdateReferenceID(const char *oldID, const char *newID) override;

   ///
  /// Finds the storage node and read the data
  //void UpdateScene(vtkMRMLScene *scene);

  ///
  /// Updates this node if it depends on other nodes
  /// when the node is deleted in the scene
  void UpdateReferences() override;


  ///
  /// alternative method to propagate events generated in Display nodes
  void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ ) override;

  ///
  /// Create default storage node or nullptr if does not have one
  vtkMRMLStorageNode* CreateDefaultStorageNode() override
    {
    return Superclass::CreateDefaultStorageNode();
    }

protected:
  vtkMRMLDiffusionImageVolumeNode();
  ~vtkMRMLDiffusionImageVolumeNode() override;
  vtkMRMLDiffusionImageVolumeNode(const vtkMRMLDiffusionImageVolumeNode&);
  void operator=(const vtkMRMLDiffusionImageVolumeNode&);

  char *BaselineNodeID;
  char *MaskNodeID;
  char *DiffusionWeightedNodeID;

};

#endif
