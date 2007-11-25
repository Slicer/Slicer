#ifndef __vtkMRMLEMSTreeParametersNode_h
#define __vtkMRMLEMSTreeParametersNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkEMSegment.h"
#include "vtkMRMLEMSTreeParametersLeafNode.h"
#include "vtkMRMLEMSTreeParametersParentNode.h"
#include <vector>

class VTK_EMSEGMENT_EXPORT vtkMRMLEMSTreeParametersNode : 
  public vtkMRMLNode
{
public:
  static vtkMRMLEMSTreeParametersNode *New();
  vtkTypeMacro(vtkMRMLEMSTreeParametersNode,vtkMRMLNode);
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
  virtual const char* GetNodeTagName() {return "EMSTreeParameters";}

  // Description:
  // Updates this node if it depends on other nodes
  // when the node is deleted in the scene
  virtual void UpdateReferences();

  // Description:
  // Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);  

  // manipulate tree structure
  virtual void AddChildNode(const char* childNodeID);
  virtual void RemoveNthChildNode(int n);
  virtual void MoveNthChildNode(int fromIndex, int toIndex);

  // manipulate target input channels
  vtkGetMacro(NumberOfTargetInputChannels, unsigned int);
  virtual void SetNumberOfTargetInputChannels(unsigned int n);
  virtual void AddTargetInputChannel();
  virtual void RemoveNthTargetInputChannel(int index);
  virtual void MoveNthTargetInputChannel(int fromIndex, int toIndex);

  // additional parameters valid for leaf nodes
  vtkSetReferenceStringMacro(LeafParametersNodeID);
  vtkGetStringMacro(LeafParametersNodeID);
  virtual vtkMRMLEMSTreeParametersLeafNode* GetLeafParametersNode();
  
  // additional parameters valid for parent nodes
  vtkSetReferenceStringMacro(ParentParametersNodeID);
  vtkGetStringMacro(ParentParametersNodeID);
  virtual vtkMRMLEMSTreeParametersParentNode* GetParentParametersNode();

  // input channel weights; length=NumberOfTargetInputChannels
  virtual double GetInputChannelWeight(int index) const;
  virtual void SetInputChannelWeight(int index, double value);
  
  // name of the spatial prior volume in the atlas
  vtkGetStringMacro(SpatialPriorVolumeName);
  vtkSetStringMacro(SpatialPriorVolumeName);

  // regulates the influence of the spatial prior
  // 0 => no influence, 1 => maximum influence
  vtkGetMacro(SpatialPriorWeight, double);
  vtkSetMacro(SpatialPriorWeight, double);
  
  // the relative probability of this class verses other classes at
  // the same level
  vtkGetMacro(ClassProbability, double);
  vtkSetMacro(ClassProbability, double);

  // Kilian: fill this in
  vtkGetMacro(ExcludeFromIncompleteEStep, int);
  vtkSetMacro(ExcludeFromIncompleteEStep, int);
  
  // Kilian: fill this in
  vtkGetMacro(PrintWeights, int);
  vtkSetMacro(PrintWeights, int);

  vtkSetVectorMacro(ColorRGB, double, 3);
  vtkGetVectorMacro(ColorRGB, double, 3);

protected:
  vtkMRMLEMSTreeParametersNode();
  ~vtkMRMLEMSTreeParametersNode();
  vtkMRMLEMSTreeParametersNode(const vtkMRMLEMSTreeParametersNode&);
  void operator=(const vtkMRMLEMSTreeParametersNode&);

  // references to other nodes
  char*                               LeafParametersNodeID;
  char*                               ParentParametersNodeID;

  double                              ColorRGB[3];

  //BTX
  typedef vtkstd::vector<double>      ChannelWeightListType;
  ChannelWeightListType               InputChannelWeights;
  //ETX

  char*                               SpatialPriorVolumeName;
  double                              SpatialPriorWeight;

  double                              ClassProbability;
  int                                 ExcludeFromIncompleteEStep;
  int                                 PrintWeights;

  unsigned int                        NumberOfTargetInputChannels;
};

#endif
