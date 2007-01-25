#ifndef __vtkMRMLEMSTreeNode_h
#define __vtkMRMLEMSTreeNode_h

#include <vector>

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkEMSegment.h"
#include "vtkMRMLEMSTreeParametersNode.h"

class VTK_EMSEGMENT_EXPORT vtkMRMLEMSTreeNode : 
  public vtkMRMLNode
{
public:
  static vtkMRMLEMSTreeNode *New();
  vtkTypeMacro(vtkMRMLEMSTreeNode,vtkMRMLNode);
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
  virtual const char* GetNodeTagName() {return "EMSTree";}

  // Description:
  // Updates this node if it depends on other nodes
  // when the node is deleted in the scene
  virtual void UpdateReferences();

  // Description:
  // Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  // associated parameters nodes
  vtkGetStringMacro(ParentNodeID);
  vtkSetReferenceStringMacro(ParentNodeID);
  virtual vtkMRMLEMSTreeNode* GetParentNode();

  vtkGetStringMacro(TreeParametersNodeID);
  vtkSetReferenceStringMacro(TreeParametersNodeID);
  virtual vtkMRMLEMSTreeParametersNode* GetParametersNode();

  // the label is from a controlled vocabulary
  vtkGetStringMacro(Label);
  vtkSetStringMacro(Label);

  // convinience methods for accessing child nodes
  // NB: this searches through the MRML scene 
  virtual int GetNumberOfChildNodes();
  virtual char* GetNthChildNodeID(int n);
  virtual vtkMRMLEMSTreeNode* GetNthChildNode(int n);

protected:
  vtkMRMLEMSTreeNode();
  ~vtkMRMLEMSTreeNode();
  vtkMRMLEMSTreeNode(const vtkMRMLEMSTreeNode&);
  void operator=(const vtkMRMLEMSTreeNode&);

  // parent of this node
  char*                               ParentNodeID;

  // associated algorithm parameters
  char*                               TreeParametersNodeID;

  // structure label from controlled vocabulary
  char*                               Label;
};

#endif
