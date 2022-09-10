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
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLTransformableNode.h"
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

class VTK_MRML_EXPORT vtkMRMLDisplayableNode : public vtkMRMLTransformableNode
{
public:
  vtkTypeMacro(vtkMRMLDisplayableNode,vtkMRMLTransformableNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance() override = 0;

  const char* GetNodeTagName() override = 0;

  ///
  /// Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Write this node's information to a vector of strings for passing to a CLI.
  /// If the prefix is not an empty string, it gets pushed onto the vector
  /// of strings before the information.
  /// coordinateSystemFlag = vtkMRMLStorageNode::CoordinateSystemRAS or vtkMRMLStorageNode::CoordinateSystemLPS
  /// multipleFlag = 1 for the whole list, 0 for the first in the list
  virtual void WriteCLI(std::vector<std::string>& vtkNotUsed(commandLine),
                        std::string vtkNotUsed(prefix),
                        int vtkNotUsed(coordinateSystemFlag) = vtkMRMLStorageNode::CoordinateSystemRAS,
                        int vtkNotUsed(multipleFlag) = 1) {};

  ///
  /// Copy the node's attributes to this object
  void Copy(vtkMRMLNode *node) override;

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
  void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ ) override;

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

  /// Creates the most appropriate display node class for storing a sequence of these nodes.
  /// If the method is not overwritten by subclass then it creates display node by calling,
  /// CreateDefaultDisplayNodes method.
  virtual void CreateDefaultSequenceDisplayNodes();

  /// Utility to return the visibility of all the display nodes.
  /// Return 0 if they are all hidden, 1 if all are visible and 2 if some are
  /// visible and some are hidden.
  /// It ignores display nodes that have ShowMode other than vtkMRMLDisplayNode::ShowDefault.
  virtual int GetDisplayVisibility();
  virtual void SetDisplayVisibility(int visible);

  /// Get/Set visibility of display nodes of certain class
  /// if nodeClass is 0, get/set visibility of all display nodes
  /// It ignores display nodes that have ShowMode other than vtkMRMLDisplayNode::ShowDefault.
  virtual int GetDisplayClassVisibility(const char* nodeClass);
  virtual void SetDisplayClassVisibility(const char* nodeClass, int visible);

  /// Get bounding box in global RAS form (xmin,xmax, ymin,ymax, zmin,zmax).
  /// This method returns the bounds of the object with any transforms that may
  /// be applied to it.
  /// \sa GetBounds()
  virtual void GetRASBounds(double bounds[6]);

  /// Get bounding box in global RAS form (xmin,xmax, ymin,ymax, zmin,zmax).
  /// This method always returns the bounds of the untransformed object.
  /// \sa GetRASBounds()
  virtual void GetBounds(double bounds[6]);

  virtual const char* GetDisplayNodeReferenceRole();

  ///
  /// Override default selectable setting to notify display node
  /// about the change.
  void SetSelectable(int) override;

protected:
  vtkMRMLDisplayableNode();
  ~vtkMRMLDisplayableNode() override;
  vtkMRMLDisplayableNode(const vtkMRMLDisplayableNode&);
  void operator=(const vtkMRMLDisplayableNode&);

  static const char* DisplayNodeReferenceRole;
  static const char* DisplayNodeReferenceMRMLAttributeName;

  virtual const char* GetDisplayNodeReferenceMRMLAttributeName();

  ///
  /// Called when a node reference ID is added (list size increased).
  void OnNodeReferenceAdded(vtkMRMLNodeReference *reference) override;

  ///
  /// Called when a node reference ID is modified.
  void OnNodeReferenceModified(vtkMRMLNodeReference *reference) override;

  ///
  /// Called after a node reference ID is removed (list size decreased).
  void OnNodeReferenceRemoved(vtkMRMLNodeReference *reference) override;

private:
  /// Internally cached list of display nodes used ONLY to return the vector of node in GetDisplayNodes()
  /// DON'T USE this variable anywhere else
  std::vector<vtkMRMLDisplayNode *> DisplayNodes;
};

#endif
