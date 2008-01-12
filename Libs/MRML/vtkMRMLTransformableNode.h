/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLTransformableNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/
// .NAME vtkMRMLTransformableNode - MRML node for representing a node with a tranform
// .SECTION Description
// A supercalss for other nodes that can have a transform to parent node 
// like volume, model and transformation nodes

#ifndef __vtkMRMLTransformableNode_h
#define __vtkMRMLTransformableNode_h


#include "vtkMRML.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"

class vtkMRMLTransformNode;
class vtkCallbackCommand;
class vtkMatrix4x4;
class vtkAbstractTransform;

class VTK_MRML_EXPORT vtkMRMLTransformableNode : public vtkMRMLNode
{
  public:
  static vtkMRMLTransformableNode *New(){return NULL;};
  vtkTypeMacro(vtkMRMLTransformableNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance() = 0;

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
  virtual const char* GetNodeTagName() = 0;

  // Description:
  // Updates this node if it depends on other nodes 
  // when the node is deleted in the scene
  virtual void UpdateReferences();

  // Description:
  // Observe the reference transform node
  virtual void UpdateScene(vtkMRMLScene *scene);

  // Description:
  // Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  // Description:
  // String ID of the transform MRML node
  vtkGetStringMacro(TransformNodeID);
  void SetAndObserveTransformNodeID(const char *transformNodeID);

  // Description:
  // Associated transform MRML node
  vtkMRMLTransformNode* GetParentTransformNode();

  // Description:
  // alternative method to propagate events generated in Transform nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                  unsigned long /*event*/, 
                                  void * /*callData*/ );

  //BTX
  // Description:
  // TransformModifiedEvent is send when the parent transform is modidied
  enum
    {
      TransformModifiedEvent = 15000,
    };
 //ETX
 
  virtual bool CanApplyNonLinearTransforms() = 0;
  virtual void ApplyTransform(vtkMatrix4x4* transformMatrix); 
  virtual void ApplyTransform(vtkAbstractTransform* transform) = 0; 

protected:
  vtkMRMLTransformableNode();
  ~vtkMRMLTransformableNode();
  vtkMRMLTransformableNode(const vtkMRMLTransformableNode&);
  void operator=(const vtkMRMLTransformableNode&);

  vtkSetReferenceStringMacro(TransformNodeID);
  char *TransformNodeID;

  vtkMRMLTransformNode* TransformNode;

};

#endif


 

