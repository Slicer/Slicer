/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLModelHierarchyNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/
// .NAME vtkMRMLModelHierarchyNode - MRML node to represent a hierarchyu of models

#ifndef __vtkMRMLModelHierarchyNode_h
#define __vtkMRMLModelHierarchyNode_h

#include <string>

#include "vtkMRML.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLHierarchyNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelDisplayNode.h"


class vtkCallbackCommand;

class VTK_MRML_EXPORT vtkMRMLModelHierarchyNode : public vtkMRMLHierarchyNode
{
public:
  static vtkMRMLModelHierarchyNode *New();
  vtkTypeMacro(vtkMRMLModelHierarchyNode,vtkMRMLHierarchyNode);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

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
  // Get node XML tag name (like Volume, ModelHierarchy)
  virtual const char* GetNodeTagName() {return "ModelHierarchy";};

   // Description:
  // Updates this node if it depends on other nodes 
  // when the node is deleted in the scene
  virtual void UpdateReferences();

  // Description:
  // Finds the model node and read the data
  virtual void UpdateScene(vtkMRMLScene *scene);

  // Description:
  // Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  // Description:
  // String ID of the model MRML node
  vtkSetReferenceStringMacro(ModelNodeID);
  vtkGetStringMacro(ModelNodeID);

  // Need this for tcl wrapping to call ReferenceStringMacro methods
  void SetModelNodeIDReference(const char* ref) {
    this->SetModelNodeID(ref);
  };


  // Description:
  // String ID of the display MRML node
  void SetAndObserveDisplayNodeID(const char *DisplayNodeID);
  vtkGetStringMacro(DisplayNodeID);


  // Description:
  // Get associated model MRML node
  vtkMRMLModelNode* GetModelNode();

  // Description:
  // Get associated display MRML node
  vtkMRMLModelDisplayNode* GetDisplayNode();
    

  // Description:
  // Indicates if the node is expanded
  vtkBooleanMacro(Expanded, int);
  vtkGetMacro(Expanded, int);
  vtkSetMacro(Expanded, int);

  // Description:
  // Get the first parent node in hierarchy which is not expanded
  vtkMRMLModelHierarchyNode* GetUnExpandedParentNode();
  
  // Description:
  // Get the top parent node in the hierarchy
  vtkMRMLModelHierarchyNode* GetTopParentNode();

  // Description:
  // Find all child model nodes in the hierarchy
  void GetChildrenModelNodes(vtkCollection *models);

  // Description:
  // Get Hierarchy node for a given Model node
  static vtkMRMLModelHierarchyNode* GetModelHierarchyNode(vtkMRMLScene *scene,
                                                          const char *modelNodeID);

  // Description:
  // alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );

//BTX
  // Description:
  // DisplayModifiedEvent is generated when display node parameters is changed
  enum
    {
      DisplayModifiedEvent = 17000,
    };
//ETX

protected:
  vtkMRMLModelHierarchyNode();
  ~vtkMRMLModelHierarchyNode();
  vtkMRMLModelHierarchyNode(const vtkMRMLModelHierarchyNode&);
  void operator=(const vtkMRMLModelHierarchyNode&);

  vtkSetReferenceStringMacro(DisplayNodeID);


  // Data

  char *ModelNodeID;
  char *DisplayNodeID;

  vtkMRMLModelDisplayNode *ModelDisplayNode;

  int Expanded;
};

#endif
