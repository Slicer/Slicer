#ifndef __vtkMRMLEMSVolumeCollectionNode_h
#define __vtkMRMLEMSVolumeCollectionNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkEMSegment.h"
#include "vtkMRMLVolumeNode.h"
#include <list>
#include <map>

class VTK_EMSEGMENT_EXPORT vtkMRMLEMSVolumeCollectionNode : 
  public vtkMRMLNode
{
public:
  static vtkMRMLEMSVolumeCollectionNode *New();
  vtkTypeMacro(vtkMRMLEMSVolumeCollectionNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Set node attributes
  virtual void ReadXMLAttributes(const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // clone the volumes of rhs
  virtual void CloneVolumes(const vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "EMSVolumeCollection";}

  // Description:
  // Updates this node if it depends on other nodes
  // when the node is deleted in the scene
  virtual void UpdateReferences();

  // Description:
  // Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  virtual void   AddVolume(const char* key, const char* volumeNodeID);
  virtual int    GetNumberOfVolumes() const;

  virtual void   RemoveAllVolumes();
  virtual void   RemoveVolumeByKey(const char* key);
  virtual void   RemoveVolumeByNodeID(const char* key);
  virtual void   RemoveNthVolume(int n);

  virtual const char*        GetVolumeNodeIDByKey(const char* key) const;
  virtual const char*        GetKeyByVolumeNodeID(const char* nodeID) const;

  virtual int                GetIndexByKey(const char* key) const;
  virtual int                GetIndexByVolumeNodeID(const char* nodeID) const;

  virtual const char*        GetNthVolumeNodeID(int n) const;
  virtual void               SetNthVolumeNodeID(int n, const char* nodeID);

  virtual const char*        GetNthKey(int n) const;
  virtual vtkMRMLVolumeNode* GetNthVolumeNode(int n) const;

  virtual void  MoveNthVolume(int n, int toIndex);

protected:
  vtkMRMLEMSVolumeCollectionNode();
  ~vtkMRMLEMSVolumeCollectionNode();
  vtkMRMLEMSVolumeCollectionNode(const vtkMRMLEMSVolumeCollectionNode&);
  void operator=(const vtkMRMLEMSVolumeCollectionNode&);

  //BTX
  // provide mapping key->value and value->key
  typedef vtkstd::string                    KeyType;
  typedef vtkstd::string                    ValueType;
  typedef vtkstd::map<KeyType, ValueType>   MapType;
  mutable MapType                           KeyToVolumeNodeIDMap;
  mutable MapType                           VolumeNodeIDToKeyMap;

  // maintain order of keys
  typedef vtkstd::list<KeyType>             KeyListType;
  typedef KeyListType::iterator             KeyIterator;
  typedef KeyListType::const_iterator       KeyConstIterator;  
  mutable KeyListType                       KeyList;
  //ETX
};

#endif
