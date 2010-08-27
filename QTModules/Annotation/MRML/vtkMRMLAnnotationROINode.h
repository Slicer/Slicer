#ifndef __vtkMRMLAnnotationROINode_h
#define __vtkMRMLAnnotationROINode_h

#include "qSlicerAnnotationModuleExport.h"
#include "vtkMRMLAnnotationLinesNode.h" 

class vtkMatrix4x4;
class vtkAbstractTransform;
class vtkMRMLScene;

class  Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkMRMLAnnotationROINode : public vtkMRMLAnnotationLinesNode
{
public:
  static vtkMRMLAnnotationROINode *New();
  vtkTypeMacro(vtkMRMLAnnotationROINode,vtkMRMLAnnotationLinesNode);
  // Description:
  // Just prints short summary 
  void PrintAnnotationInfo(ostream& os, vtkIndent indent, int titleFlag = 1);

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();
  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "AnnotationROI";};

  // Description:
  // Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);
  
  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);


  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  void UpdateScene(vtkMRMLScene *scene);

  // Description:
  // alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );


  // Legacy code
  // Description:
  // get/set the first point position
  double* GetPosition1() {return this->GetControlPointCoordinates(0);}

  int SetPosition1(double newControl[3]) { return this->SetControlPoint(0, newControl) ; }
  int SetPosition1(double nC1, double nC2, double nC3) { 
    double newControl[3] = {nC1,nC2,nC3};
    return this->SetPosition1(newControl) ; 
  }


  double* GetPosition2() {return this->GetControlPointCoordinates(1);}
  int SetPosition2(double newControl[3]) { return this->SetControlPoint(1, newControl);}
  int SetPosition2(double nC1, double nC2, double nC3) { 
    double newControl[3] = {nC1,nC2,nC3};
    return this->SetPosition2(newControl) ; 
  }

  // Description:
  // get/set the distance annotation format, it's in standard sprintf notation
  vtkGetStringMacro(ROIAnnotationFormat);
  vtkSetStringMacro(ROIAnnotationFormat);

  /// 
  /// Indicates if the ROI is updated interactively
  vtkBooleanMacro(InteractiveMode, int);
  vtkGetMacro(InteractiveMode, int);
  vtkSetMacro(InteractiveMode, int);

  // Description:
  // KP Define - should be part of AnnotationRulerDisplayNode 
  double GetROIAnnotationScale();
  void SetROIAnnotationScale(double init);

  // Description:
  // get/set the distance annotation visbility
  int GetROIAnnotationVisibility();
  void SetROIAnnotationVisibility(int flag);

  int SetROI(vtkIdType line1Id, int sel, int vis);  

  // Description:
  // get/set the resolution (number of subdivisions) of the line.
  vtkGetMacro(Resolution, int);
  vtkSetMacro(Resolution, int);

  // Description:
  // get/set the point representation colour
  double *GetPointColour();
  void SetPointColour( double initColor[3]);

  // Description:
  // get/set the line representation colour
  double *GetLineColour();
  void SetLineColour(double newColor[3]);

  // Description:
  // get/set the distance annotation text colour
  double *GetROIAnnotationTextColour();
  void SetROIAnnotationTextColour(double initColor[3]);

 // Description:
  // transform utility functions
  virtual bool CanApplyNonLinearTransforms() { return true; }
  virtual void ApplyTransform(vtkMatrix4x4* transformMatrix);
  virtual void ApplyTransform(vtkAbstractTransform* transform);

  void Initialize(vtkMRMLScene* mrmlScene);

  int GetVisibility() {return this->GetVisible();}
  void SetVisibility(int flag) {this->SetVisible(flag);}

  /// 
  /// Get/Set for ROI Position in RAS cooridinates
  /// Note: The ROI Position is the center of the ROI 
  void SetXYZ(double X, double Y, double Z);
  void SetXYZ(double* XYZ);
  vtkGetVectorMacro(XYZ,double,3);

  /// 
  /// Get/Set for radius of the ROI in RAS cooridnates
  void SetRadiusXYZ(double RadiusX, double RadiusY, double RadiusZ);
  void SetRadiusXYZ(double* RadiusXYZ);
  vtkGetVectorMacro(RadiusXYZ,double,3);

  ///
  /// Get/Set for the bounds in RAS coordinates
  void SetBounds(double boundsX1, double boundsX2, double boundsY1, double boundsY2, double boundsZ1, double boundsZ2);
  void SetBounds(double * bounds);
  vtkGetVectorMacro(Bounds, double, 6);

  std::vector<double> GetROIMeasurement();
  void SetROIMeasurement(double val1, double val2, double val3);

  enum
  {
      ROINodeAddedEvent = 0,
      ValueModifiedEvent,
  };


protected:
  vtkMRMLAnnotationROINode();
  ~vtkMRMLAnnotationROINode();
  vtkMRMLAnnotationROINode(const vtkMRMLAnnotationROINode&);
  void operator=(const vtkMRMLAnnotationROINode&);

  // Description:
  // number of subdivisions on the line
  int Resolution;
  char* ROIAnnotationFormat;
  int InteractiveMode;
 
  int SetControlPoint(int id, double newControl[3]);

  int AddControlPoint(double newControl[3],int selectedFlag, int visibleFlag);

  /// the model ids for the models that the ends of the ruler are constrained
  /// to
  std::vector<double> ROIMeasurement;
  int Visibility;

  /// 
  /// The location of the ROI centroid in RAS space
  /// Note: The ROI Postion is the center of the ROI 
  double XYZ[3];  

  /// 
  /// The raidus of  of the ROI box in RAS space
  double RadiusXYZ[3];

  /// The location of the ROI centroid in IJK space
  /// Note: The ROI Postion is the center of the ROI 
  double IJK[3];  
  /// 
  /// The radius of the ROI box in IJK space
  double RadiusIJK[3];

  /// The bounds of the box in RAS coordinates
  double Bounds[6];

  /// Control the orientation of the normals
  int InsideOut;

  char *LabelText;

  /// 
  /// The ID of the volume associated with the ROI 
  char *VolumeNodeID;

};

#endif
