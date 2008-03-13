/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLStorableNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/
// .NAME vtkMRMLStorableNode - MRML node to represent a 3D surface model.
// .SECTION Description
// Model nodes describe polygonal data.  Models 
// are assumed to have been constructed with the orientation and voxel 
// dimensions of the original segmented volume.

#ifndef __vtkMRMLStorableNode_h
#define __vtkMRMLStorableNode_h

#include <string>

#include "vtkMRML.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLTransformableNode.h"

class vtkCallbackCommand;
class vtkMRMLStorageNode;
class vtkMRMLScene;
class vtkMRMLNode;
class VTK_MRML_EXPORT vtkMRMLStorableNode : public vtkMRMLTransformableNode
{
public:
  static vtkMRMLStorableNode *New(){return NULL;};
  vtkTypeMacro(vtkMRMLStorableNode,vtkMRMLTransformableNode);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance() = 0;

  virtual const char* GetNodeTagName() = 0;

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
  // Updates this node if it depends on other nodes 
  // when the node is deleted in the scene
  virtual void UpdateReferences();

  // Description:
  // Finds the storage node and read the data
  virtual void UpdateScene(vtkMRMLScene *scene);

  // Description:
  // Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  // Description:
  // alternative method to propagate events generated in Storage nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );
  // Description:
  // String ID of the storage MRML node
  /*
  vtkSetReferenceStringMacro(StorageNodeID);
  void SetReferenceStorageNodeID(const char *id) { this->SetStorageNodeID(id); }
  vtkGetStringMacro(StorageNodeID);

  // Description:
  // Get associated storage MRML node
  vtkMRMLStorageNode* GetStorageNode();
  */
  // Description:
  // String ID of the storage MRML node
  void SetAndObserveStorageNodeID(const char *StorageNodeID);
  void AddAndObserveStorageNodeID(const char *StorageNodeID);
  void SetAndObserveNthStorageNodeID(int n, const char *StorageNodeID);

  int GetNumberOfStorageNodes()
    {
    return this->StorageNodeIDs.size();
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

  // Description:
  // Get associated display MRML node
  vtkMRMLStorageNode* GetNthStorageNode(int n);

  vtkMRMLStorageNode* GetStorageNode()
    {
    return this->GetNthStorageNode(0);
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

//BTX
  std::vector<std::string> StorageNodeIDs;
 
  std::vector<vtkMRMLStorageNode *> StorageNodes;
//ETX

  //char *StorageNodeID;
};

#endif
