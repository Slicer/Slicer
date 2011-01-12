#ifndef __vtkMRMLAnnotationBidimensionalNode_h
#define __vtkMRMLAnnotationBidimensionalNode_h

#include "qSlicerAnnotationModuleExport.h"
#include "vtkMRMLAnnotationLinesNode.h" 

class vtkMatrix4x4;
class vtkAbstractTransform;
class vtkMRMLScene;

class  Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkMRMLAnnotationBidimensionalNode : public vtkMRMLAnnotationLinesNode
{
public:
  static vtkMRMLAnnotationBidimensionalNode *New();
  vtkTypeMacro(vtkMRMLAnnotationBidimensionalNode, vtkMRMLAnnotationLinesNode);
  // Description:
  // Just prints short summary 
  void PrintAnnotationInfo(ostream& os, vtkIndent indent, int titleFlag = 1);

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();
  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "AnnotationBidimensional";};

  virtual const char* GetIcon() {return ":/Icons/AnnotationBidimensional.png";};

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


  // Description:
  // get/set the distance annotation format, it's in standard sprintf notation
  vtkGetStringMacro(AnnotationFormat);
  vtkSetStringMacro(AnnotationFormat);

  // Description:
  // get/set the resolution (number of subdivisions) of the line.
  vtkGetMacro(Resolution, int);
  vtkSetMacro(Resolution, int);

 // Description:
  // transform utility functions
  virtual bool CanApplyNonLinearTransforms() { return true; }
  void Initialize(vtkMRMLScene* mrmlScene);

  int GetVisibility() {return this->GetVisible();}
  void SetVisibility(int flag) {this->SetVisible(flag);}

  std::vector<double> GetBidimensionalMeasurement();
  void SetBidimensionalMeasurement(double val1, double val2);

  int SetControlPoint(double newControl[3], int id);

  enum
  {
      BidimensionalNodeAddedEvent = 0,
      ValueModifiedEvent,
  };


protected:
  vtkMRMLAnnotationBidimensionalNode();
  ~vtkMRMLAnnotationBidimensionalNode();
  vtkMRMLAnnotationBidimensionalNode(const vtkMRMLAnnotationBidimensionalNode&);
  void operator=(const vtkMRMLAnnotationBidimensionalNode&);

  // Description:
  // number of subdivisions on the line
  int Resolution;
  char* AnnotationFormat;

  double measurement1;
  double measurement2;
 
};

#endif
