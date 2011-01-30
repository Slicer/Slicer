/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLDisplayableHierarchyNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/
///  vtkMRMLDisplayableHierarchyNode - MRML node to represent a hierarchy of
///  displayable nodes

#ifndef __vtkMRMLDisplayableHierarchyNode_h
#define __vtkMRMLDisplayableHierarchyNode_h

#include "vtkMRML.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLHierarchyNode.h"
#include "vtkMRMLDisplayableNode.h"
#include "vtkMRMLDisplayNode.h"


class vtkCallbackCommand;

class VTK_MRML_EXPORT vtkMRMLDisplayableHierarchyNode : public vtkMRMLHierarchyNode
{
public:
  static vtkMRMLDisplayableHierarchyNode *New();
  vtkTypeMacro(vtkMRMLDisplayableHierarchyNode,vtkMRMLHierarchyNode);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();

  /// 
  /// Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  /// 
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);


  /// 
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);
  
  /// 
  /// Get node XML tag name (like Volume, DisplayableHierarchy)
  virtual const char* GetNodeTagName() {return "DisplayableHierarchy";};

   /// 
  /// Updates this node if it depends on other nodes 
  /// when the node is deleted in the scene
  virtual void UpdateReferences();

  /// 
  /// Finds the Displayable node and read the data
  virtual void UpdateScene(vtkMRMLScene *scene);

  /// 
  /// Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  /// 
  /// String ID of the displayable MRML node
  vtkSetReferenceStringMacro(DisplayableNodeID);
  vtkGetStringMacro(DisplayableNodeID);

  /// Need this for tcl wrapping to call ReferenceStringMacro methods
  void SetDisplayableNodeIDReference(const char* ref) {
    this->SetDisplayableNodeID(ref);
  };


  /// 
  /// String ID of the displayable MRML node
  void SetAndObserveDisplayNodeID(const char *DisplayNodeID);
  vtkGetStringMacro(DisplayNodeID);


  /// 
  /// Get associated displayable MRML node
  vtkMRMLDisplayableNode* GetDisplayableNode();

  /// 
  /// Get associated display MRML node
  vtkMRMLDisplayNode* GetDisplayNode();
    

  /// 
  /// Indicates if the node is expanded
  vtkBooleanMacro(Expanded, int);
  vtkGetMacro(Expanded, int);
  vtkSetMacro(Expanded, int);

  /// 
  /// Get the first parent node in hierarchy which is not expanded
  vtkMRMLDisplayableHierarchyNode* GetUnExpandedParentNode();
  
  /// 
  /// Find all child displayable nodes in the hierarchy
  void GetChildrenDisplayableNodes(vtkCollection *children);

  /// 
  /// Get Hierarchy node for a given displayable node
  static vtkMRMLDisplayableHierarchyNode* GetDisplayableHierarchyNode(vtkMRMLScene *scene,
                                                                      const char *displayableNodeID);

  /// 
  /// alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );

  /// DisplayModifiedEvent is generated when display node parameters is changed
  enum
    {
      DisplayModifiedEvent = 17000
    };

  /// Mark hierarchy as modified when you
  static void DispalyableHierarchyIsModified(vtkMRMLScene *scene);

protected:
  vtkMRMLDisplayableHierarchyNode();
  ~vtkMRMLDisplayableHierarchyNode();
  vtkMRMLDisplayableHierarchyNode(const vtkMRMLDisplayableHierarchyNode&);
  void operator=(const vtkMRMLDisplayableHierarchyNode&);

  vtkSetReferenceStringMacro(DisplayNodeID);


  /// Data
  char *DisplayableNodeID;
  char *DisplayNodeID;

  vtkMRMLDisplayNode *DisplayNode;

  int Expanded;

  /// 
  /// Create Displayable to hierarchy map, 
  /// return number of Displayable hierarchy nodes
  static int UpdateDisplayableToHierarchyMap(vtkMRMLScene *scene);
  
  //BTX
  typedef std::map<std::string, vtkMRMLDisplayableHierarchyNode *> DisplayableHierarchyNodesType;

  static std::map< vtkMRMLScene*, DisplayableHierarchyNodesType> SceneDisplayableHierarchyNodes;

  static std::map< vtkMRMLScene*, unsigned long> SceneDisplayableHierarchyNodesMTime;
  //ETX

};

#endif
