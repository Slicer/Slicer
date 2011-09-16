#ifndef __vtkMRMLEMSVolumeCollectionNode_h
#define __vtkMRMLEMSVolumeCollectionNode_h

#include "vtkMRML.h"
#include "vtkMRMLEMSCollectionNode.h"
#include "vtkEMSegment.h"
#include "vtkMRMLVolumeNode.h"
#include <list>
#include <map>

// Most things are kept for legacy 
class VTK_EMSEGMENT_EXPORT vtkMRMLEMSVolumeCollectionNode : 
  public vtkMRMLEMSCollectionNode
{
public:
  static vtkMRMLEMSVolumeCollectionNode *New();
  vtkTypeMacro(vtkMRMLEMSVolumeCollectionNode,vtkMRMLNode);
  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Set node attributes - for legacy purposes 
  virtual void ReadXMLAttributes(const char** atts);

  // Description:
  // clone the volumes of rhs
  virtual void CloneVolumes(const vtkMRMLNode *node, const char* addPostFix);

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "EMSVolumeCollection";}

  // Returns 1 if volume was added or 0 if the mapping between key and volumeNodeID existed 
  virtual int   AddVolume(const char* key, const char* volumeNodeID)
  {
    return this->AddNode(key,volumeNodeID);
  }
  virtual int    GetNumberOfVolumes() const  
  {
    return this->GetNumberOfNodes();
  }

  virtual void   RemoveAllVolumes()
  {
    this->RemoveAllNodes();
  }
  virtual void   RemoveVolumeByKey(const char* key) 
  {
    this->RemoveNodeByKey(key);
  }
  virtual void   RemoveVolumeByNodeID(const char* key)
  {
    this->RemoveNodeByNodeID(key);
  }
  virtual void   RemoveNthVolume(int n)
  {
    this->RemoveNthNode(n);
  }

  virtual const char*        GetVolumeNodeIDByKey(const char* key) const
  {
    return this->GetNodeIDByKey(key);
  }
  virtual const char*        GetKeyByVolumeNodeID(const char* nodeID) const
  {
    return this->GetKeyByVolumeNodeID(nodeID);
  }

  virtual int                GetIndexByVolumeNodeID(const char* nodeID) const
  {
    return this->GetIndexByNodeID(nodeID);
  }

  virtual const char*        GetNthVolumeNodeID(int n) const
  {
    return this->GetNthNodeID(n);
  }

  virtual void               SetNthVolumeNodeID(int n, const char* nodeID)
  {
    this->SetNthNodeID(n, nodeID);
  }

  virtual vtkMRMLVolumeNode* GetNthVolumeNode(int n) const;

  virtual void  MoveNthVolume(int n, int toIndex)
  {
    this->MoveNthNode(n, toIndex);
  }

protected:
  vtkMRMLEMSVolumeCollectionNode();
  ~vtkMRMLEMSVolumeCollectionNode();
  vtkMRMLEMSVolumeCollectionNode(const vtkMRMLEMSVolumeCollectionNode&);
  void operator=(const vtkMRMLEMSVolumeCollectionNode&);

};

#endif
