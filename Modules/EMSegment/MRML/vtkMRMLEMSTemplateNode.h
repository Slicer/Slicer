#ifndef __vtkMRMLEMSTemplateNode_h
#define __vtkMRMLEMSTemplateNode_h

#include <vector>

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkEMSegment.h"
#include "vtkMRMLEMSTreeNode.h"
#include "vtkMRMLEMSGlobalParametersNode.h"

class VTK_EMSEGMENT_EXPORT vtkMRMLEMSTemplateNode : 
  public vtkMRMLNode
{
public:
  static vtkMRMLEMSTemplateNode *New();
  vtkTypeMacro(vtkMRMLEMSTemplateNode,vtkMRMLNode);
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
  virtual const char* GetNodeTagName() {return "EMSTemplate";}

  // Description:
  // Updates this node if it depends on other nodes
  // when the node is deleted in the scene
  virtual void UpdateReferences();

  // Description:
  // Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  // associated parameters nodes
  vtkGetStringMacro(TreeNodeID);
  vtkSetReferenceStringMacro(TreeNodeID);
  vtkMRMLEMSTreeNode* GetTreeNode();

  vtkGetStringMacro(GlobalParametersNodeID);
  vtkSetReferenceStringMacro(GlobalParametersNodeID);
  vtkMRMLEMSGlobalParametersNode* GetGlobalParametersNode();

protected:
  vtkMRMLEMSTemplateNode();
  ~vtkMRMLEMSTemplateNode();
  vtkMRMLEMSTemplateNode(const vtkMRMLEMSTemplateNode&);
  void operator=(const vtkMRMLEMSTemplateNode&);

  char*                               TreeNodeID;
  char*                               GlobalParametersNodeID;
};

#endif
