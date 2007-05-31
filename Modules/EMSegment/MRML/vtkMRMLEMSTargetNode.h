#ifndef __vtkMRMLEMSTargetNode_h
#define __vtkMRMLEMSTargetNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkEMSegment.h"
#include "vtkMRMLEMSVolumeCollectionNode.h"

class vtkMRMLEMSIntensityNormalizationParametersNode;

class VTK_EMSEGMENT_EXPORT vtkMRMLEMSTargetNode : 
  public vtkMRMLEMSVolumeCollectionNode
{
public:
  static vtkMRMLEMSTargetNode *New();
  vtkTypeMacro(vtkMRMLEMSTargetNode,vtkMRMLEMSVolumeCollectionNode);
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
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "EMSTarget";}

  // Description:
  // Updates this node if it depends on other nodes
  // when the node is deleted in the scene
  virtual void UpdateReferences();

  // Description:
  // Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  virtual void   AddVolume(const char* key, const char* volumeNodeID);

  virtual void   RemoveAllVolumes();
  virtual void   RemoveVolumeByKey(const char* key);
  virtual void   RemoveVolumeByNodeID(const char* key);
  virtual void   RemoveNthVolume(int n);

  virtual void   MoveNthVolume(int n, int toIndex);
  
  // set/get parameter node for intensity normalization
  virtual vtkMRMLEMSIntensityNormalizationParametersNode*
    GetNthIntensityNormalizationParametersNode(int n);
  virtual const char* GetNthIntensityNormalizationParametersNodeID(int n);
  virtual void SetNthIntensityNormalizationParametersNodeID(int n, 
                                                            const char* nodeID);

protected:
  vtkMRMLEMSTargetNode();
  ~vtkMRMLEMSTargetNode();
  vtkMRMLEMSTargetNode(const vtkMRMLEMSTargetNode&);
  void operator=(const vtkMRMLEMSTargetNode&);

  //BTX
  typedef vtkstd::vector<std::string>  IntensityNormalizationParameterListType;
  typedef IntensityNormalizationParameterListType::iterator 
  IntensityNormalizationParameterListIterator;
  IntensityNormalizationParameterListType IntensityNormalizationParameterList;
  //ETX
};

#endif
