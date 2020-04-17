/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLStorableNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/

#ifndef __vtkMRMLStorableNode_h
#define __vtkMRMLStorableNode_h

// MRML includes
#include "vtkMRMLNode.h"
class vtkMRMLStorageNode;

// VTK includes
class vtkTagTable;

// STD includes
#include <vector>

/// \brief MRML node to represent a 3D surface model.
///
/// Model nodes describe polygonal data.  Models
/// are assumed to have been constructed with the orientation and voxel
/// dimensions of the original segmented volume.
class VTK_MRML_EXPORT vtkMRMLStorableNode : public vtkMRMLNode
{
public:
  vtkTypeMacro(vtkMRMLStorableNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //--------------------------------------------------------------------------
  /// Methods for user-specified metadata
  //--------------------------------------------------------------------------
  vtkGetObjectMacro ( UserTagTable, vtkTagTable );

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

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLStorableNode);

  ///
  /// Finds the storage node and read the data
  void UpdateScene(vtkMRMLScene *scene) override;

  ///
  /// alternative method to propagate events generated in Storage nodes
  void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ ) override;

  ///
  /// String ID of the storage MRML node
  void SetAndObserveStorageNodeID(const char *storageNodeID);
  void AddAndObserveStorageNodeID(const char *storageNodeID);
  void SetAndObserveNthStorageNodeID(int n, const char *storageNodeID);

  ///
  /// Return true if storageNodeID is in the storage node ID list.
  bool HasStorageNodeID(const char* storageNodeID);

  ///
  /// This is describes the type of data stored in the nodes storage node(s).
  /// It's an informatics metadata mechanism so that Slicer knows what kinds
  /// of nodes to create to receive downloaded datasets, and works around
  /// potential ambiguity of file extensions, etc. Method is called when storage
  /// nodes are created. The method gets applied to any storable data that
  /// should be saved with, and loaded with the scene, including nodes that
  /// are hidden from editors like scalar overlays.
  void SetSlicerDataType ( const char *type );
  const char *GetSlicerDataType ();

  int GetNumberOfStorageNodes();
  const char *GetNthStorageNodeID(int n);
  const char *GetStorageNodeID();

  ///
  /// Get associated display MRML node
  vtkMRMLStorageNode* GetNthStorageNode(int n);
  vtkMRMLStorageNode* GetStorageNode();

  /// Create a storage node for this node type.
  /// If it returns nullptr then it means the node can be stored
  /// in the scene (in XML), without using a storage node.
  /// Null by default.
  /// This must be overwritten by subclasses that use storage nodes.
  virtual vtkMRMLStorageNode* CreateDefaultStorageNode();

  /// Determines the most appropriate storage node class for the
  /// provided file name and node content.
  /// If the method is not overwritten by subclass then it uses
  /// CreateDefaultStorageNode to determine storage node class name.
  virtual std::string GetDefaultStorageNodeClassName(const char* filename = nullptr);

  /// Returns true on success. If storage node is not needed then
  /// storage node is not created and the method returns with true.
  virtual bool AddDefaultStorageNode(const char* filename = nullptr);

  /// Creates the most appropriate storage node class for storing a sequence of these nodes.
  /// The caller owns the returned object and responsible for deleting it.
  /// If the method is not overwritten by subclass then it creates vtkMRMLSequenceStorageNode,
  /// which saves the sequence in a MRML scene bundle file.
  virtual vtkMRMLStorageNode* CreateDefaultSequenceStorageNode();

  /// Returns true if the node is more recent than the file on disk.
  /// This information can be used by the application to know which node
  /// has been modified since it has been last read or written.
  /// Only storable properties are considered:
  /// even if a "non storable" property (e.g. color of a mesh) is modified after
  /// the node is being loaded, GetModifiedSinceRead() should
  /// return false; the new property value won't be stored on file (only in the
  /// MRML scene).
  /// By default, calling Modified() on the node doesn't make the node "modified
  /// since read", only calling Modified() on StorableModifiedTime does.
  /// GetModifiedSinceRead() can be overwritten to handle special storable
  /// property modification time.
  /// \sa GetStoredTime() StorableModifiedTime Modified() StorableModified()
  virtual bool GetModifiedSinceRead();

  /// Allows external code to mark that the storable has been modified
  /// and should therefore be selected for saving by default.
  /// \sa GetStoredTime() StorableModifiedTime Modified() GetModifiedSinceRead()
  virtual void StorableModified();

 protected:
  vtkMRMLStorableNode();
  ~vtkMRMLStorableNode() override;
  vtkMRMLStorableNode(const vtkMRMLStorableNode&);
  void operator=(const vtkMRMLStorableNode&);

  static const char* StorageNodeReferenceRole;
  static const char* StorageNodeReferenceMRMLAttributeName;

  virtual const char* GetStorageNodeReferenceRole();
  virtual const char* GetStorageNodeReferenceMRMLAttributeName();

  vtkTagTable *UserTagTable;

  ///
  /// SlicerDataType records the kind of storage node that
  /// holds the data. Set in each subclass.
  std::string SlicerDataType;

  /// Compute when the storable node was read/written for the last time.
  /// This information is used by GetModifiedSinceRead() to know if the node
  /// has been modified since the last time it was read or written
  /// By default, it retrieves the information from the associated storage
  /// nodes.
  /// \sa GetModifiedSinceRead(), StorableModifiedTime,
  /// vtkMRMLStorageNode::GetStoredTime()
  virtual vtkTimeStamp GetStoredTime();

  /// Last time when a storable property was modified. This is used to know
  /// if the node has been modified since the last time it was read or written
  /// on disk.
  /// The time stamp must be updated (Modified()) - in the derived classes - any
  /// time a "storable" property is changed. A storable property is a property
  /// that is stored on disk, not in the MRML scene: e.g. points and cells for a
  /// Model, voxel intensity or origin for a Volume...
  /// \sa GetModifiedSinceRead(), GetStoredTime()
  vtkTimeStamp StorableModifiedTime;
};

#endif
