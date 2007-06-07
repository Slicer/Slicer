#ifndef __vtkMRMLROINode_h
#define __vtkMRMLROINode_h

#include <string> 

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLTransformableNode.h"

#include <string>

// .NAME vtkMRMLROINode - MRML node to represent a 3D ROI.
// .SECTION Description
// Model nodes describe ROI data.  They indicate where the ROI is 
// located and the size of the ROI

class VTK_MRML_EXPORT vtkMRMLROINode : public vtkMRMLTransformableNode
{
public:
  static vtkMRMLROINode *New();
  vtkTypeMacro(vtkMRMLROINode,vtkMRMLTransformableNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  //--------------------------------------------------------------------------
  // MRML methods
  //--------------------------------------------------------------------------
  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  virtual void ReadXMLString( const char *keyValuePairs);


  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "MRMLROINode";};

  // Description:
  // 
  virtual void UpdateScene(vtkMRMLScene *scene);

  // Description:
  // update display node ids
  void UpdateReferences();

  // Description:
  // Get/Set for Point
  vtkSetVector3Macro(XYZ,float);
  vtkGetVectorMacro(XYZ,float,3);

  // Get/Set for orientation 
  vtkSetVector3Macro(DeltaXYZ,float);
  vtkGetVectorMacro(DeltaXYZ,float,3);

  // Get/Set for LabelText
  vtkSetStringMacro(LabelText);
  vtkGetStringMacro(LabelText);

  // Get/Set for ID
  vtkGetStringMacro(ID);
  vtkSetStringMacro(ID);

  // Get/Set for Selected
  vtkGetMacro(Selected, bool);
  vtkSetMacro(Selected, bool);

  void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );


protected:
  vtkMRMLROINode();
  ~vtkMRMLROINode();
  vtkMRMLROINode(const vtkMRMLROINode&);
  void operator=(const vtkMRMLROINode&);

  // Description:
  // The location of the ROI centroid
  float XYZ[3];  
  // Description:
  // The size of  of the ROI box
  float DeltaXYZ[3];
  char *ID;
  char *LabelText;
  bool Selected;
};
#endif
