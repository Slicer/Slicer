/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLStorableNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/
///  vtkMRMLStorableNode - MRML node to represent a 3D surface model.
/// 
/// Model nodes describe polygonal data.  Models 
/// are assumed to have been constructed with the orientation and voxel 
/// dimensions of the original segmented volume.

#ifndef __vtkMRMLStorableNode_h
#define __vtkMRMLStorableNode_h

// MRML includes
#include "vtkMRMLTransformableNode.h"
class vtkMRMLStorageNode;

// VTK includes
class vtkTagTable;

class VTK_MRML_EXPORT vtkMRMLStorableNode : public vtkMRMLTransformableNode
{
public:
  static vtkMRMLStorableNode *New(){return NULL;};
  vtkTypeMacro(vtkMRMLStorableNode,vtkMRMLTransformableNode);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  //--------------------------------------------------------------------------
  /// Methods for user-specified metadata
  //--------------------------------------------------------------------------
  vtkGetObjectMacro ( UserTagTable, vtkTagTable );
  
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

  /// 
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// 
  /// Updates this node if it depends on other nodes 
  /// when the node is deleted in the scene
  virtual void UpdateReferences();

  /// 
  /// Finds the storage node and read the data
  virtual void UpdateScene(vtkMRMLScene *scene);

  /// 
  /// Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  /// 
  /// alternative method to propagate events generated in Storage nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );
  /// 
  /// String ID of the storage MRML node
  /*
  vtkSetReferenceStringMacro(StorageNodeID);
  void SetReferenceStorageNodeID(const char *id) { this->SetStorageNodeID(id); }
  vtkGetStringMacro(StorageNodeID);

  /// 
  /// Get associated storage MRML node
  vtkMRMLStorageNode* GetStorageNode();
  */
  /// 
  /// String ID of the storage MRML node
  void SetAndObserveStorageNodeID(const char *StorageNodeID);
  void AddAndObserveStorageNodeID(const char *StorageNodeID);
  void SetAndObserveNthStorageNodeID(int n, const char *StorageNodeID);

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
  
  int GetNumberOfStorageNodes()
    {
      return (int)this->StorageNodeIDs.size();
    };

  const char *GetNthStorageNodeID(int n)
  {
      if (n < 0 || n >= (int)this->StorageNodeIDs.size())
      {
          return NULL;
      }
      return this->StorageNodeIDs[n].c_str();
  };

  const char *GetStorageNodeID()
    {
    return this->GetNthStorageNodeID(0);
    };

  /// 
  /// Get associated display MRML node
  vtkMRMLStorageNode* GetNthStorageNode(int n);

  vtkMRMLStorageNode* GetStorageNode()
    {
    return this->GetNthStorageNode(0);
    };

//BTX
  std::vector<vtkMRMLStorageNode*> GetStorageNodes()const
    {
    return this->StorageNodes;
    };
//ETX

  virtual vtkMRMLStorageNode* CreateDefaultStorageNode()
    {
    return NULL;
    };

 protected:
  vtkMRMLStorableNode();
  ~vtkMRMLStorableNode();
  vtkMRMLStorableNode(const vtkMRMLStorableNode&);
  void operator=(const vtkMRMLStorableNode&);

  void SetStorageNodeID(const char* id) ;
  void SetNthStorageNodeID(int n, const char* id);
  void AddStorageNodeID(const char* id);
  void AddAndObserveStorageNode(vtkMRMLStorageNode *dnode);

  vtkTagTable *UserTagTable;

//BTX
  std::vector<std::string> StorageNodeIDs;
  /// 
  /// SlicerDataType records the kind of storage node that
  /// holds the data. Set in each subclass.
  std::string SlicerDataType;
  std::vector<vtkMRMLStorageNode *> StorageNodes;
//ETX

  //char *StorageNodeID;
};

#endif
