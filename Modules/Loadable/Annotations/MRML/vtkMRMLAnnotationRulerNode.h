#ifndef __vtkMRMLAnnotationRulerNode_h
#define __vtkMRMLAnnotationRulerNode_h

#include "vtkSlicerAnnotationsModuleMRMLExport.h"
#include "vtkMRMLAnnotationLinesNode.h" 

class vtkMatrix4x4;
class vtkAbstractTransform;
class vtkMRMLScene;

/// \ingroup Slicer_QtModules_Annotation
class  VTK_SLICER_ANNOTATIONS_MODULE_MRML_EXPORT vtkMRMLAnnotationRulerNode : public vtkMRMLAnnotationLinesNode
{
public:
  static vtkMRMLAnnotationRulerNode *New();
  vtkTypeMacro(vtkMRMLAnnotationRulerNode,vtkMRMLAnnotationLinesNode);
  // Description:
  // Just prints short summary 
  void PrintAnnotationInfo(ostream& os, vtkIndent indent, int titleFlag = 1);

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();
  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "AnnotationRuler";};

  virtual const char* GetIcon() {return ":/Icons/AnnotationDistance.png";};

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



  // Get position of the first point
  void GetPosition1(double position[3]);

  // Get position of the first point in world coordinates, including parnts transfroms
  void GetPositionWorldCoordinates1(double position[4])
    {
    double p[4]={0,0,0,1};
    this->GetPosition1(p);
    this->TransformPointToWorld(p,position);
    }

  // Legacy code
  // Description:
  // get/set the first point position
  double* GetPosition1() 
    {
    return this->GetControlPointCoordinates(0);
    }

  int SetPosition1(double newControl[3]) 
    { 
    return this->SetControlPoint(0, newControl) ; 
    }

  int SetPositionWorldCoordinates1(double newControl[4]) 
    { 
    double locPoint[4]={0,0,0,1};
    this->TransformPointFromWorld(newControl, locPoint);
    return this->SetPosition1(locPoint) ; 
    }

  int SetPosition1(double nC1, double nC2, double nC3) 
    { 
    double newControl[3] = {nC1,nC2,nC3};
    return this->SetPosition1(newControl) ; 
    }


  // Get position of the second point
  void GetPosition2(double position[3]);

  // Get position of the second point in world coordinates, including parnts transfroms
  void GetPositionWorldCoordinates2(double position[4])
    {
    double p[4]={0,0,0,1};
    this->GetPosition2(p);
    this->TransformPointToWorld(p,position);
    }

  double* GetPosition2() 
    {
    return this->GetControlPointCoordinates(1);
    }

  int SetPosition2(double newControl[3]) 
    { 
    return this->SetControlPoint(1, newControl);
    }

  int SetPositionWorldCoordinates2(double newControl[4]) 
    { 
    double locPoint[4]={0,0,0,1};
    this->TransformPointFromWorld(newControl, locPoint);
    return this->SetPosition2(locPoint) ; 
    }

  int SetPosition2(double nC1, double nC2, double nC3) 
    { 
    double newControl[3] = {nC1,nC2,nC3};
    return this->SetPosition2(newControl) ; 
    }

  // Description:
  // get/set the distance annotation format, it's in standard sprintf notation
  vtkGetStringMacro(DistanceAnnotationFormat);
  vtkSetStringMacro(DistanceAnnotationFormat);

  // Description:
  // KP Define - should be part of AnnotationRulerDisplayNode 
  double GetDistanceAnnotationScale();
  void SetDistanceAnnotationScale(double init);

  // Description:
  // get/set the distance annotation visbility
  int GetDistanceAnnotationVisibility();
  void SetDistanceAnnotationVisibility(int flag);

  int SetRuler(vtkIdType line1Id, int sel, int vis);  

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
  double *GetDistanceAnnotationTextColour();
  void SetDistanceAnnotationTextColour(double initColor[3]);

 // Description:
  // transform utility functions
  virtual void ApplyTransformMatrix(vtkMatrix4x4* transformMatrix);
  virtual void ApplyTransform(vtkAbstractTransform* transform);

  // Description:
  // Create default storage node or NULL if does not have one
  virtual vtkMRMLStorageNode* CreateDefaultStorageNode();  

//  void Initialize(vtkMRMLScene* mrmlScene);

  /// Calculate the distance between world coordinates of the two end points
  double GetDistanceMeasurement();


  /// get/set the id of the model the first and second end of the ruler is constrained upon
  vtkGetStringMacro(ModelID1);
  vtkSetStringMacro(ModelID1);
  vtkGetStringMacro(ModelID2);
  vtkSetStringMacro(ModelID2);

  enum
  {
      RulerNodeAddedEvent = 20010,
      ValueModifiedEvent,
  };


protected:
  vtkMRMLAnnotationRulerNode();
  ~vtkMRMLAnnotationRulerNode();
  vtkMRMLAnnotationRulerNode(const vtkMRMLAnnotationRulerNode&);
  void operator=(const vtkMRMLAnnotationRulerNode&);

  // Description:
  // number of subdivisions on the line
  int Resolution;
  char* DistanceAnnotationFormat;
 
  int SetControlPoint(int id, double newControl[3]);

  int AddControlPoint(double newControl[3],int selectedFlag, int visibleFlag);

  /// the model ids for the models that the ends of the ruler are constrained
  /// to
  char *ModelID1;
  char *ModelID2;
};

#endif
