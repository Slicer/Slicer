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

#ifndef __vtkMRMLTransformableNode_h
#define __vtkMRMLTransformableNode_h


#include "vtkMRML.h"
#include "vtkMRMLNode.h"

class vtkMRMLTransformNode;
class vtkCallbackCommand;

class VTK_MRML_EXPORT vtkMRMLTransformableNode : public vtkMRMLNode
{
  public:
  static vtkMRMLTransformableNode *New(){return NULL;};
  vtkTypeMacro(vtkMRMLTransformableNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance() = 0;

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
  virtual const char* GetNodeTagName() = 0;

  // Description:
  // Observe the reference transform node
  virtual void UpdateScene(vtkMRMLScene *scene);

  // Description:
  // String ID of the transform MRML node
  vtkGetStringMacro(TransformNodeID);
  void SetAndObserveTransformNode(const char *transformNodeID);

  // Description:
  // Associated transform MRML node
  vtkMRMLTransformNode* GetParentTransformNode();

  // Description:
  // alternative method to propagate events generated in Transform nodes
  virtual void ProcessEvents ( vtkObject * /*caller*/, 
                               unsigned long /*event*/, 
                               void * /*callData*/ );

  //BTX
  enum
    {
      TransformModifiedEvent = 15000,
    };
//ETX
  

protected:
  vtkMRMLTransformableNode();
  ~vtkMRMLTransformableNode();
  vtkMRMLTransformableNode(const vtkMRMLTransformableNode&);
  void operator=(const vtkMRMLTransformableNode&);

  //BTX
  static void TransformNodeCallback( vtkObject *__caller,
                                     unsigned long eid, 
                                     void *__clientData,
                                     void *callData );
  //ETX

  vtkCallbackCommand* TransformNodeCallbackCommand;

  vtkSetStringMacro(TransformNodeID);
  char *TransformNodeID;

};

#endif


 

