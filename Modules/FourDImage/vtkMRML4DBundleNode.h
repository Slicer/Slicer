/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRML4DBundleNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/
// .NAME vtkMRML4DBundleNode - MRML node for representing 
// a linear transformation to the parent node
// .SECTION Description
// MRML node for representing 
// a linear transformation to the parent node in the form vtkMatrix4x4
// MatrixTransformToParent

#ifndef __vtkMRML4DBundleNode_h
#define __vtkMRML4DBundleNode_h

#include "vtkFourDImageWin32Header.h"

#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLScalarVolumeNode.h"

class vtkMRMLStorageNode;

class VTK_FourDImage_EXPORT vtkMRML4DBundleNode : public vtkMRMLLinearTransformNode
{
  public:
  static vtkMRML4DBundleNode *New();
  vtkTypeMacro(vtkMRML4DBundleNode,vtkMRMLLinearTransformNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "FourDBundle";};

  // Description:
  // alternative method to propagate events generated in Transform nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );

  virtual bool CanApplyNonLinearTransform() { return true; } 
 
  // Description:
  // Create default storage node or NULL if does not have one
  virtual vtkMRMLStorageNode* CreateDefaultStorageNode()
    {
    return Superclass::CreateDefaultStorageNode();
    };

  int GetNumberOfFrames();
  int InsertFrame(int i, const char* nodeID);
  int AddFrame(const char* nodeID);
  int RemoveFrame(int i);              // Delete a frame by index number (not remove from the scene)
  int RemoveFrame(const char* nodeID); // Delete a frame by node ID (not remove from the scene)
  void RemoveAllFrames();

  vtkMRMLNode* GetFrameNode(int i);

  int          SetDisplayBufferNodeID(int bufferIndex, const char* nodeID);
  vtkMRMLNode* GetDisplayBufferNode(int bufferIndex);
  void         SwitchDisplayBuffer(int bufferIndex, int i);

protected:
  vtkMRML4DBundleNode();
  ~vtkMRML4DBundleNode();
  vtkMRML4DBundleNode(const vtkMRML4DBundleNode&);
  void operator=(const vtkMRML4DBundleNode&);

  //BTX
  typedef std::vector<std::string> NodeIDListType;

  NodeIDListType   FrameNodeIDList;
  NodeIDListType   TransformNodeIDList;
  NodeIDListType   DisplayBufferNodeIDList;
  //ETX

};

#endif

