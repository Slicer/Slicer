#ifndef __vtkMRMLAnnotationBidimensionalNode_h
#define __vtkMRMLAnnotationBidimensionalNode_h

#include "vtkSlicerAnnotationsModuleMRMLExport.h"
#include "vtkMRMLAnnotationLinesNode.h"

class vtkMatrix4x4;
class vtkAbstractTransform;
class vtkMRMLScene;

/// \ingroup Slicer_QtModules_Annotation
class  VTK_SLICER_ANNOTATIONS_MODULE_MRML_EXPORT vtkMRMLAnnotationBidimensionalNode : public vtkMRMLAnnotationLinesNode
{
public:
  static vtkMRMLAnnotationBidimensionalNode *New();
  vtkTypeMacro(vtkMRMLAnnotationBidimensionalNode, vtkMRMLAnnotationLinesNode);
  // Description:
  // Just prints short summary
  void PrintAnnotationInfo(ostream& os, vtkIndent indent, int titleFlag = 1) override;

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance() override;
  // Description:
  // Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "AnnotationBidimensional";}

  const char* GetIcon() override {return ":/Icons/AnnotationBidimensional.png";}

  // Description:
  // Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  // Description:
  // Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;


  // Description:
  // Copy the node's attributes to this object
  void Copy(vtkMRMLNode *node) override;

  void UpdateScene(vtkMRMLScene *scene) override;

  // Description:
  // alternative method to propagate events generated in Display nodes
  void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ ) override;


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
  void Initialize(vtkMRMLScene* mrmlScene) override;

  std::vector<double> GetBidimensionalMeasurement();
  void SetBidimensionalMeasurement(double val1, double val2);

  int SetControlPoint(double newControl[3], int id);

  int SetControlPointWorldCoordinates(double newControl[3], int id)
    {
    double localPoint[4]={0,0,0,1};
    this->TransformPointFromWorld(newControl, localPoint);
    return this->SetControlPoint(localPoint, id);
    }

  enum
  {
      BidimensionalNodeAddedEvent = 0,
      ValueModifiedEvent,
  };


protected:
  vtkMRMLAnnotationBidimensionalNode();
  ~vtkMRMLAnnotationBidimensionalNode() override;
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
