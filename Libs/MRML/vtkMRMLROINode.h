#ifndef __vtkMRMLROINode_h
#define __vtkMRMLROINode_h

#include <string> 

#include "vtkMRML.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLTransformableNode.h"

#include <string>

// .NAME vtkMRMLROINode - MRML node to represent a 3D ROI.
// .SECTION Description
// Model nodes describe ROI data.  They indicate where the ROI is 
// located and the size of the ROI

class VTK_MRML_EXPORT vtkMRMLROINode : public vtkMRMLNode
{
public:
  static vtkMRMLROINode *New();
  vtkTypeMacro(vtkMRMLROINode,vtkMRMLNode);
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
  // Get/Set for ROI Position in RAS cooridnates
  // Note: The ROI Postion is the center of the ROI 
  void SetXYZ(float X, float Y, float Z);
  void SetXYZ(float* XYZ);
  vtkGetVectorMacro(XYZ,float,3);

  // Description:
  // Get/Set for radius of the ROI in RAS cooridnates
  void SetRadiusXYZ(float RadiusX, float RadiusY, float RadiusZ);
  void SetRadiusXYZ(float* RadiusXYZ);
  vtkGetVectorMacro(RadiusXYZ,float,3);

  // Description:
  // Get/Set for ROI Position in IJK cooridnates
  void SetIJK(float I, float J, float K);
  void SetIJK(float* IJK);
  vtkGetVectorMacro(IJK,float,3);

  // Description:
  // Get/Set for radius of the ROI in IJK cooridnates
  void SetRadiusIJK(float RadiusI, float RadiusJ, float RadiusK);
  void SetRadiusIJK(float* RadiusIJK);
  vtkGetVectorMacro(RadiusIJK,float,3);

  // Description:
  // Get/Set for LabelText
  vtkSetStringMacro(LabelText);
  vtkGetStringMacro(LabelText);

  // Description:
  // Get/Set for ID
  vtkGetStringMacro(ID);
  vtkSetStringMacro(ID);

  // Description:
  // Get/Set for Selected
  vtkGetMacro(Selected, bool);
  vtkSetMacro(Selected, bool);

  void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  vtkGetStringMacro(VolumeNodeID);
  vtkSetStringMacro(VolumeNodeID);
protected:
  vtkMRMLROINode();
  ~vtkMRMLROINode();
  vtkMRMLROINode(const vtkMRMLROINode&);
  void operator=(const vtkMRMLROINode&);

  // Description:
  // The location of the ROI centroid in RAS space
  // Note: The ROI Postion is the center of the ROI 
  float XYZ[3];  
  // Description:
  // The raidus of  of the ROI box in RAS space
  float RadiusXYZ[3];
  // Description:
  // The location of the ROI centroid in IJK space
  // Note: The ROI Postion is the center of the ROI 
  float IJK[3];  
  // Description:
  // The radius of the ROI box in IJK space
  float RadiusIJK[3];

  char *ID;
  char *LabelText;
  bool Selected;

  // Description:
  // The ID of the volume associated with the ROI 
  char *VolumeNodeID;
};
#endif
