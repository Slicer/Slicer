#ifndef __vtkMRMLEMSCollectionNode_h
#define __vtkMRMLEMSCollectionNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkEMSegment.h"
#include <list>
#include <map>

class VTK_EMSEGMENT_EXPORT vtkMRMLEMSCollectionNode : 
  public vtkMRMLNode
{
public:
  static vtkMRMLEMSCollectionNode *New();
  vtkTypeMacro(vtkMRMLEMSCollectionNode,vtkMRMLNode);
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
  // Get node XML tag name (like , Model)
  virtual const char* GetNodeTagName() {return "EMSCollection";}

  // Description:
  // Updates this node if it depends on other nodes
  // when the node is deleted in the scene
  virtual void UpdateReferences();

  // Description:
  // Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  // Returns 1 if volume was added or 0 if the mapping between key and volumeNodeID existed 
  virtual int   AddNode(const char* key, const char* volumeNodeID);
  virtual int    GetNumberOfNodes() const;

  virtual void   RemoveAllNodes();
  virtual void   RemoveNodeByKey(const char* key);
  virtual void   RemoveNodeByNodeID(const char* key);
  virtual void   RemoveNthNode(int n);

  virtual const char*        GetNodeIDByKey(const char* key) const;
  virtual const char*        GetKeyByNodeID(const char* nodeID) const;

  virtual int                GetIndexByKey(const char* key) const;
  virtual int                GetIndexByNodeID(const char* nodeID) const;

  virtual const char*        GetNthNodeID(int n) const;
  virtual void               SetNthNodeID(int n, const char* nodeID);

  virtual const char*        GetNthKey(int n) const;
 
  virtual void  MoveNthNode(int n, int toIndex);

  virtual vtkMRMLNode*       GetNthNode(int n) const;

protected:
  vtkMRMLEMSCollectionNode();
  ~vtkMRMLEMSCollectionNode();
  vtkMRMLEMSCollectionNode(const vtkMRMLEMSCollectionNode&);
  void operator=(const vtkMRMLEMSCollectionNode&);

  //BTX
  // provide mapping key->value and value->key
  typedef vtkstd::string                    KeyType;
  typedef vtkstd::string                    ValueType;
  typedef vtkstd::map<KeyType, ValueType>   MapType;
  mutable MapType                           KeyToNodeIDMap;
  mutable MapType                           NodeIDToKeyMap;

  // maintain order of keys
  typedef vtkstd::list<KeyType>             KeyListType;
  typedef KeyListType::iterator             KeyIterator;
  typedef KeyListType::const_iterator       KeyConstIterator;  
  mutable KeyListType                       KeyList;
  //ETX

};

#endif
