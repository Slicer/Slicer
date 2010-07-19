#ifndef __vtkMRMLEMSTargetNode_h
#define __vtkMRMLEMSTargetNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLEMS.h"
#include "vtkMRMLEMSVolumeCollectionNode.h"

class vtkMRMLEMSIntensityNormalizationParametersNode;

class VTK_MRMLEMS_EXPORT vtkMRMLEMSTargetNode : 
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

  virtual const char* GetNthInputChannelName(vtkIdType n);
  virtual void SetNthInputChannelName(vtkIdType n, const char* targetName);
  virtual int  GetNumberOfInputChannelNames();
  virtual void SetNumberOfInputChannelName(vtkIdType newSize);
  virtual int AddInputChannelName(const char* name);


protected:
  vtkMRMLEMSTargetNode();
  ~vtkMRMLEMSTargetNode();
  vtkMRMLEMSTargetNode(const vtkMRMLEMSTargetNode&);
  void operator=(const vtkMRMLEMSTargetNode&);
  //BTX
  std::vector<std::string>   InputChannelNames; 
  //ETX
};

#endif
