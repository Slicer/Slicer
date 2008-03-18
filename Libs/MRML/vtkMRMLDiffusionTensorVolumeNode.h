/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLVolumeNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/
// .NAME vtkMRMLDiffusionTensorVolumeNode - MRML node for representing diffusion weighted MRI volume 
// .SECTION Description
// Diffusion Weigthed Volume nodes describe data sets that encode diffusion weigthed
// images. These images are the basis for computing the diffusion tensor.
// The node is a container for the neccesary information to interpert DW images:
// 1. Gradient information.
// 2. B value for each gradient.
// 3. Measurement frame that relates the coordinate system where the gradients are given 
//  to RAS.

#ifndef __vtkMRMLDiffusionTensorVolumeNode_h
#define __vtkMRMLDiffusionTensorVolumeNode_h


#include "vtkMRMLDiffusionImageVolumeNode.h"
#include "vtkMRMLDiffusionTensorVolumeDisplayNode.h"
#include "vtkMRMLDiffusionTensorVolumeSliceDisplayNode.h"

class vtkDoubleArray;

class VTK_MRML_EXPORT vtkMRMLDiffusionTensorVolumeNode : public vtkMRMLDiffusionImageVolumeNode
{
  public:
  static vtkMRMLDiffusionTensorVolumeNode *New();
  vtkTypeMacro(vtkMRMLDiffusionTensorVolumeNode,vtkMRMLTensorVolumeNode);
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
  virtual const char* GetNodeTagName() {return "DiffusionTensorVolume";};

  // Description:
  // Associated volume display MRML node
  virtual void SetAndObserveDisplayNodeID(const char *DisplayNodeID);

  // Description:
  // Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

   // Description:
  // Finds the storage node and read the data
  //void UpdateScene(vtkMRMLScene *scene);

  // Description:
  // Updates this node if it depends on other nodes 
  // when the node is deleted in the scene
  void UpdateReferences();

  
  // Description:
  // alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );
 
  // Description:
  // Associated display MRML node
  virtual vtkMRMLDiffusionTensorVolumeDisplayNode* GetDiffusionTensorVolumeDisplayNode()
  {
    return vtkMRMLDiffusionTensorVolumeDisplayNode::SafeDownCast(this->GetDisplayNode());
  }
//BTX
  // Description:
  // get associated slice glyph display node or NULL if not set
  std::vector< vtkMRMLDiffusionTensorVolumeSliceDisplayNode*> GetSliceGlyphDisplayNodes();


  // Description:
  // add slice glyph display nodes if not already present and return it
  std::vector< vtkMRMLDiffusionTensorVolumeSliceDisplayNode*>  AddSliceGlyphDisplayNodes();
//ETX

protected:
  vtkMRMLDiffusionTensorVolumeNode();
  ~vtkMRMLDiffusionTensorVolumeNode();
  vtkMRMLDiffusionTensorVolumeNode(const vtkMRMLDiffusionTensorVolumeNode&);
  void operator=(const vtkMRMLDiffusionTensorVolumeNode&);

};

#endif
