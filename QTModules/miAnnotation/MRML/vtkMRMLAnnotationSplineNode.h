#ifndef __vtkMRMLAnnotationSplineNode_h
#define __vtkMRMLAnnotationSplineNode_h

#include "qSlicermiAnnotationModuleExport.h"
#include "vtkMRMLAnnotationLinesNode.h" 

class vtkMatrix4x4;
class vtkAbstractTransform;
class vtkMRMLScene;

class  Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkMRMLAnnotationSplineNode : public vtkMRMLAnnotationLinesNode
{
public:
  static vtkMRMLAnnotationSplineNode *New();
  vtkTypeMacro(vtkMRMLAnnotationSplineNode, vtkMRMLAnnotationLinesNode);
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
  virtual bool CanApplyNonLinearTransforms() { return true; }
  virtual void ApplyTransform(vtkMatrix4x4* transformMatrix);
  virtual void ApplyTransform(vtkAbstractTransform* transform);

  void Initialize(vtkMRMLScene* mrmlScene);

  int GetVisibility() {return this->GetVisible();}
  void SetVisibility(int flag) {this->SetVisible(flag);}

  enum
  {
      SplineNodeAddedEvent = 0,
      ValueModifiedEvent,
  };


protected:
  vtkMRMLAnnotationSplineNode();
  ~vtkMRMLAnnotationSplineNode();
  vtkMRMLAnnotationSplineNode(const vtkMRMLAnnotationSplineNode&);
  void operator=(const vtkMRMLAnnotationSplineNode&);

  // Description:
  // number of subdivisions on the line
  int Resolution;
  char* DistanceAnnotationFormat;
 
  int SetControlPoint(int id, double newControl[3]);

  int AddControlPoint(double newControl[3],int selectedFlag, int visibleFlag);

};

#endif
