#ifndef __vtkMRMLAnnotationAngleNode_h
#define __vtkMRMLAnnotationAngleNode_h

#include "vtkSlicerAnnotationsModuleMRMLExport.h"
#include "vtkMRMLAnnotationLinesNode.h" 

class vtkMatrix4x4;
class vtkAbstractTransform;
class vtkMRMLScene;

/// \ingroup Slicer_QtModules_Annotation
class  VTK_SLICER_ANNOTATIONS_MODULE_MRML_EXPORT vtkMRMLAnnotationAngleNode : public vtkMRMLAnnotationLinesNode
{
public:
  static vtkMRMLAnnotationAngleNode *New();
  vtkTypeMacro(vtkMRMLAnnotationAngleNode,vtkMRMLAnnotationLinesNode);
  // Description:
  // Just prints short summary 
  void PrintAnnotationInfo(ostream& os, vtkIndent indent, int titleFlag = 1);

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();
  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "AnnotationAngle";};

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

  double* GetPositionCenter() { return this->GetControlPointCoordinates(2);}
  int SetPositionCenter(double newControl[3]) { return this->SetControlPoint(2, newControl);}

  int SetPositionCenter(double nC1, double nC2, double nC3) { 
   double newControl[3] = {nC1,nC2,nC3};
    return this->SetPositionCenter(newControl) ; 
  }

  // Description:
  // get/set the distance annotation format, it's in standard sprintf notation
  vtkGetStringMacro(LabelFormat);
  vtkSetStringMacro(LabelFormat);

  // Description:
  // KP Define - should be part of AnnotationAngleDisplayNode 
  double GetLabelScale();
  void SetLabelScale(double init);

  // Description:
  // get/set the distance annotation visbility
  int GetLabelVisibility();
  void SetLabelVisibility(int flag);

  int SetAngle(vtkIdType line1Id, vtkIdType line2Id, int sel, int vis);  

  // Description:
  int GetRay1Visibility() {return this->GetAnnotationAttribute(0,vtkMRMLAnnotationLinesNode::LINE_VISIBLE);} 
  void SetRay1Visibility(int flag) { this->SetAnnotationAttribute(0,vtkMRMLAnnotationLinesNode::LINE_VISIBLE,flag);} 
  int GetRay2Visibility() {return this->GetAnnotationAttribute(1,vtkMRMLAnnotationLinesNode::LINE_VISIBLE);} 
  void SetRay2Visibility(int flag) { this->SetAnnotationAttribute(1,vtkMRMLAnnotationLinesNode::LINE_VISIBLE,flag);} 

  int GetArcVisibility() { return this->GetLabelVisibility();}
  void SetArcVisibility(int flag) { this->SetLabelVisibility(flag);}

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
  double *GetLabelTextColour();
  void SetLabelTextColour(double initColor[3]);

 // Description:
  // transform utility functions
  virtual bool CanApplyNonLinearTransforms() { return true; }
  virtual void ApplyTransform(vtkMatrix4x4* transformMatrix);
  virtual void ApplyTransform(vtkAbstractTransform* transform);

  // Description:
  // Create default storage node or NULL if does not have one
  virtual vtkMRMLStorageNode* CreateDefaultStorageNode();  

  //  void Initialize(vtkMRMLScene* mrmlScene);

 // Description:
  // get/set the id of the model the ends of the widget are constrained upon
  vtkGetStringMacro(ModelID1);
  vtkSetStringMacro(ModelID1);
  vtkGetStringMacro(ModelID2);
  vtkSetStringMacro(ModelID2);
  vtkGetStringMacro(ModelIDCenter);
  vtkSetStringMacro(ModelIDCenter);

  // Just here for historic reasons to comply with vtkMRMLMeasurementsAngleNode
  int GetVisibility() {return this->GetVisible();}
  void SetVisibility(int flag) {this->SetVisible(flag);}

  void SetAngleMeasurement(double val);
  double GetAngleMeasurement();

  enum
  {
      AngleNodeAddedEvent = 0,
      ValueModifiedEvent,
  };

protected:
  vtkMRMLAnnotationAngleNode();
  ~vtkMRMLAnnotationAngleNode();
  vtkMRMLAnnotationAngleNode(const vtkMRMLAnnotationAngleNode&);
  void operator=(const vtkMRMLAnnotationAngleNode&);

  // Description:
  // number of subdivisions on the line
  int Resolution;
  char* LabelFormat;
 
  int SetControlPoint(int id, double newControl[3]);

  int AddControlPoint(double newControl[3],int selectedFlag, int visibleFlag);

  // Description:
  // the model ids for the models that the ends of the widget are constrained
  // to
  char *ModelID1;
  char *ModelID2;
  char *ModelIDCenter;
  double angleMeasurement;

};

#endif
