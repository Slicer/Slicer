#ifndef __vtkMRMLEMSegmentNode_h
#define __vtkMRMLEMSegmentNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"

#include "vtkEMSegment.h"

// Description: 
// The scripted module node is simply a MRMLNode container for 
// an arbitrary keyword value pair map

class VTK_EMSEGMENT_EXPORT vtkMRMLEMSegmentNode : 
  public vtkMRMLNode
{
public:
  static vtkMRMLEMSegmentNode *New();
  vtkTypeMacro(vtkMRMLEMSegmentNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "EMSegment";}

  // Description: The name of the Module - this is used to 
  // customize the node selectors and other things
  vtkGetStringMacro (ModuleName);
  vtkSetStringMacro (ModuleName);

  // Description:
  // Get/Set a parameter for the module.
//BTX
  void SetParameter(const vtksys_stl::string& name, 
                    const vtksys_stl::string& value);
  const vtksys_stl::string* GetParameter(const vtksys_stl::string &name) const;
//ETX  

  void SetParameter(const char *name, const char *value);
  void RequestParameter(const char *name);
  const char *GetParameter(const char *name);
  vtkSetStringMacro(Value);
  vtkGetStringMacro(Value);

protected:

//BTX
  vtksys_stl::map<vtksys_stl::string, vtksys_stl::string> Parameters;
//ETX
  char *Value;

  char *ModuleName;
  
private:
  vtkMRMLEMSegmentNode();
  ~vtkMRMLEMSegmentNode();
  vtkMRMLEMSegmentNode(const vtkMRMLEMSegmentNode&);
  void operator=(const vtkMRMLEMSegmentNode&);

};

#endif
