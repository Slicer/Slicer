#ifndef __vtkMRMLROINode_h
#define __vtkMRMLROINode_h

#include "vtkMRMLTransformableNode.h"

class vtkPlanes;

/// \brief MRML node to represent a 3D ROI.
///
/// Model nodes describe ROI data.  They indicate where the ROI is
/// located and the size of the ROI.
class VTK_MRML_EXPORT vtkMRMLROINode : public vtkMRMLTransformableNode
{
public:
  static vtkMRMLROINode *New();
  vtkTypeMacro(vtkMRMLROINode,vtkMRMLTransformableNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  //--------------------------------------------------------------------------
  /// MRML methods
  //--------------------------------------------------------------------------
  virtual vtkMRMLNode* CreateNodeInstance();

  ///
  /// Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  virtual void ReadXMLString( const char *keyValuePairs);

  ///
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  ///
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  ///
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "MRMLROINode";};

  ///
  ///
  virtual void UpdateScene(vtkMRMLScene *scene);

  ///
  /// update display node ids
  void UpdateReferences();

  ///
  /// Indicates if the ROI is visible
  vtkBooleanMacro(Visibility, int);
  vtkGetMacro(Visibility, int);
  vtkSetMacro(Visibility, int);

  ///
  /// Indicates if the ROI is updated interactively
  vtkBooleanMacro(InteractiveMode, int);
  vtkGetMacro(InteractiveMode, int);
  vtkSetMacro(InteractiveMode, int);

  ///
  /// Get/Set for ROI Position in RAS cooridnates
  /// Note: The ROI Postion is the center of the ROI
  void SetXYZ(double X, double Y, double Z);
  void SetXYZ(double* XYZ);
  vtkGetVectorMacro(XYZ,double,3);

  ///
  /// Get/Set for radius of the ROI in RAS cooridnates
  void SetRadiusXYZ(double RadiusX, double RadiusY, double RadiusZ);
  void SetRadiusXYZ(double* RadiusXYZ);
  vtkGetVectorMacro(RadiusXYZ,double,3);

  ///
  /// Get/Set for ROI Position in IJK cooridnates
  void SetIJK(double I, double J, double K);
  void SetIJK(double* IJK);
  vtkGetVectorMacro(IJK,double,3);

  ///
  /// Get/Set for radius of the ROI in IJK cooridnates
  void SetRadiusIJK(double RadiusI, double RadiusJ, double RadiusK);
  void SetRadiusIJK(double* RadiusIJK);
  vtkGetVectorMacro(RadiusIJK,double,3);

  ///
  /// Set/Get the InsideOut flag. This data memeber is used in conjunction
  /// with the GetPlanes() method. When off, the normals point out of the
  /// box. When on, the normals point into the hexahedron.  InsideOut is off
  /// by default.
  vtkSetMacro(InsideOut,int);
  vtkGetMacro(InsideOut,int);
  vtkBooleanMacro(InsideOut,int);

  ///
  /// Get/Set for LabelText
  vtkSetStringMacro(LabelText);
  vtkGetStringMacro(LabelText);

  void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  vtkGetStringMacro(VolumeNodeID);
  vtkSetStringMacro(VolumeNodeID);

  ///
  /// transform utility functions
  virtual bool CanApplyNonLinearTransforms()const;
  virtual void ApplyTransform(vtkAbstractTransform* transform);

  /// Description
  /// get transformed planes for the ROI region
  void GetTransformedPlanes(vtkPlanes *planes);

protected:
  vtkMRMLROINode();
  ~vtkMRMLROINode();
  vtkMRMLROINode(const vtkMRMLROINode&);
  void operator=(const vtkMRMLROINode&);
  int Visibility;
  int InteractiveMode;

  ///
  /// The location of the ROI centroid in RAS space
  /// Note: The ROI Postion is the center of the ROI
  double XYZ[3];
  ///
  /// The raidus of  of the ROI box in RAS space
  double RadiusXYZ[3];
  ///
  /// The location of the ROI centroid in IJK space
  /// Note: The ROI Postion is the center of the ROI
  double IJK[3];
  ///
  /// The radius of the ROI box in IJK space
  double RadiusIJK[3];

  /// Control the orientation of the normals
  int InsideOut;

  char *LabelText;

  ///
  /// The ID of the volume associated with the ROI
  char *VolumeNodeID;
};

#endif
