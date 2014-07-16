/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLDisplayableNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/

#ifndef __vtkMRMLDisplayableNode_h
#define __vtkMRMLDisplayableNode_h

// MRML includes
#include "vtkMRMLStorableNode.h"
class vtkMRMLDisplayNode;

// STD includes
#include <vector>

/// \brief MRML node to represent an item that can be rendered in a view.
///
/// It is the base class for models, volumes etc.
/// A displayable node points to a list of display nodes that control graphical
/// properties to render the displayable node. For example, if a displayable
/// node is a 3D surface/mesh, a first display node of the mesh can have a
/// red color attribute, while another display node for the mesh has a blue
/// color attribute. Both red and blue display nodes will be rendered using
/// the same 3D mesh data.
/// In a Model-View-Controller design pattern, the displayable node is the
/// model. The display nodes are the views of the model.
///
/// \tbd Check support for a display node to be simultaneously referenced by
/// different displayable nodes.
/// \sa vtkMRMLDisplayNode

class vtkMRMLDisplayNode;

class VTK_MRML_EXPORT vtkMRMLDisplayableNode : public vtkMRMLStorableNode
{
public:
  vtkTypeMacro(vtkMRMLDisplayableNode,vtkMRMLStorableNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance() = 0;

  virtual const char* GetNodeTagName() = 0;

  ///
  /// Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  ///
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// Write this node's information to a vector of strings for passing to a CLI.
  /// If the prefix is not an empty string, it gets pushed onto the vector
  /// of strings before the information.
  /// coordinateSystemFlag = 0 for RAS, 1 for LPS, 2 for IJK
  /// multipleFlag = 1 for the whole list, 0 for the first in the list
  virtual void WriteCLI(std::vector<std::string>& vtkNotUsed(commandLine),
                        std::string vtkNotUsed(prefix),
                        int vtkNotUsed(coordinateSystemFlag) = 0,
                        int vtkNotUsed(multipleFlag) = 1) {};

  ///
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  ///
  /// Convenience method that sets the first display node ID.
  /// \sa SetAndObserverNthDisplayNodeID(int, const char*)
  void SetAndObserveDisplayNodeID(const char *displayNodeID);

  ///
  /// Convenience method that adds a display node ID at the end of the list.
  /// \sa SetAndObserverNthDisplayNodeID(int, const char*)
  void AddAndObserveDisplayNodeID(const char *displayNodeID);

  ///
  /// Convenience method that removes the Nth display node ID from the list
  /// \sa SetAndObserverNthDisplayNodeID(int, const char*)
  void RemoveNthDisplayNodeID(int n);

  ///
  /// Remove all display node IDs and associated display nodes.
  void RemoveAllDisplayNodeIDs();

  ///
  /// Set and observe the Nth display node ID in the list.
  /// If n is larger than the number of display nodes, the display node ID
  /// is added at the end of the list. If DisplayNodeID is 0, the node ID is
  /// removed from the list.
  /// When a node ID is set (added or changed), its corresponding node is
  /// searched (slow) into the scene and cached for fast future access.
  /// It is possible however that the node is not yet into the scene (due to
  /// some temporary state (at loading time for example). UpdateScene() can
  /// later be called to retrieve the display nodes from the scene
  /// (automatically done when loading a scene). Get(Nth)DisplayNode() also
  /// scan the scene if the node was not yet cached.
  /// \sa SetAndObserveDisplayNodeID(const char*),
  /// AddAndObserveDisplayNodeID(const char *), RemoveNthDisplayNodeID(int)
  void SetAndObserveNthDisplayNodeID(int n, const char *displayNodeID);

  ///
  /// Return true if displayNodeID is in the display node ID list.
  bool HasDisplayNodeID(const char* displayNodeID);

  ///
  /// Return the number of display node IDs (and display nodes as they always
  /// have the same size).
  int GetNumberOfDisplayNodes();

  ///
  /// Return the string of the Nth display node ID. Or 0 if no such
  /// node exist.
  /// Warning, a temporary char generated from a std::string::c_str()
  /// is returned.
  const char *GetNthDisplayNodeID(int n);

  ///
  /// Utility function that returns the first display node id.
  /// \sa GetNthDisplayNodeID(int), GetDisplayNode()
  const char *GetDisplayNodeID();

  ///
  /// Get associated display MRML node. Can be 0 in temporary states; e.g. if
  /// the displayable node has no scene, or if the associated display is not
  /// yet into the scene.
  /// If not cached, it tnternally scans (slow) the scene to search for the
  /// associated display node ID.
  /// If the displayable node is no longer in the scene (GetScene() == 0), it
  /// happens after the node is removed from the scene (scene->RemoveNode(dn),
  /// the returned display node is 0.
  /// \sa GetNthDisplayNodeByClass()
  vtkMRMLDisplayNode* GetNthDisplayNode(int n);

  ///
  /// Utility function that returns the first display node.
  /// \sa GetNthDisplayNode(int), GetDisplayNodeID()
  vtkMRMLDisplayNode* GetDisplayNode();

  /// \deprecated GetDisplayNodes
  /// Obsolete utility function that provides an unsafe API.
  /// Please use GetNumberOfDisplayNodes() and
  /// GetNthDisplayNode() instead
  /// const std::vector<vtkMRMLDisplayNode*>& GetDisplayNodes();
  /// \sa GetNumberOfDisplayNodes, GetNthDisplayNode

  ///
  /// alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ );

  /// DisplayModifiedEvent is fired when:
  ///  - a new display node is observed
  ///  - a display node is not longer observed
  ///  - an associated display node is modified
  /// Note that when SetAndObserve(Nth)NodeID() is called with an ID that
  /// has not yet any associated display node in the scene, then
  /// DisplayModifiedEvent is not fired until found for the first time in
  /// the scene, e.g. Get(Nth)DisplayNode(), UpdateScene()...
  enum
    {
    DisplayModifiedEvent = 17000,
    };

  /// Create and observe default display node(s)
  /// Does nothing by default, must be reimplemented by subclasses that have
  /// display nodes.
  virtual void CreateDefaultDisplayNodes();

  /// TODO: Change it to Get/SetVisibility() for consistency with
  /// vtkMRMLDisplayNode.
  /// Utility to return the visibility of all the display nodes.
  /// Return 0 if they are all hidden, 1 if all are visible and 2 if some are
  /// visible and some are hidden.
  virtual int GetDisplayVisibility();
  virtual void SetDisplayVisibility(int visible);

  /// Get bounding box in global RAS the form (xmin,xmax, ymin,ymax, zmin,zmax).
  virtual void GetRASBounds(double bounds[6]);

 protected:
  vtkMRMLDisplayableNode();
  ~vtkMRMLDisplayableNode();
  vtkMRMLDisplayableNode(const vtkMRMLDisplayableNode&);
  void operator=(const vtkMRMLDisplayableNode&);

  static const char* DisplayNodeReferenceRole;
  static const char* DisplayNodeReferenceMRMLAttributeName;

  virtual const char* GetDisplayNodeReferenceRole();
  virtual const char* GetDisplayNodeReferenceMRMLAttributeName();

  ///
  /// Called when a node reference ID is added (list size increased).
  virtual void OnNodeReferenceAdded(vtkMRMLNodeReference *reference);

  ///
  /// Called when a node reference ID is modified.
  virtual void OnNodeReferenceModified(vtkMRMLNodeReference *reference);

  ///
  /// Called after a node reference ID is removed (list size decreased).
  virtual void OnNodeReferenceRemoved(vtkMRMLNodeReference *reference);

 private:

  /// Internally cached list of display nodes used ONLY to return the vector of node in GetDisplayNodes()
  /// DON'T USE this variable anywhere else
  std::vector<vtkMRMLDisplayNode *> DisplayNodes;
};


#endif
