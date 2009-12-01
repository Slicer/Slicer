#ifndef __vtkMRMLMeasurementsRulerNode_h
#define __vtkMRMLMeasurementsRulerNode_h

#include "vtkMeasurementsWin32Header.h"

#include "vtkMRMLMeasurementsNode.h"
class VTK_MEASUREMENTS_EXPORT vtkMRMLMeasurementsRulerNode : public vtkMRMLMeasurementsNode
{
  public:
  static vtkMRMLMeasurementsRulerNode *New();
  vtkTypeRevisionMacro(vtkMRMLMeasurementsRulerNode, vtkMRMLMeasurementsNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // get/set the first point position
  vtkGetVector3Macro(Position1, double);
  vtkSetVector3Macro(Position1, double);

  // Description:
  // get/set the second point position
  vtkGetVector3Macro(Position2, double);
  vtkSetVector3Macro(Position2, double);

  // Description:
  // get/set the distance annotation format, it's in standard sprintf notation
  vtkGetStringMacro(DistanceAnnotationFormat);
  vtkSetStringMacro(DistanceAnnotationFormat);
  
  // Description:
  // get/set the distance annotation scale
  vtkGetVector3Macro(DistanceAnnotationScale, double);
  vtkSetVector3Macro(DistanceAnnotationScale, double);
  
  // Description:
  // get/set the distance annotation visbility
  vtkGetMacro(DistanceAnnotationVisibility, int);
  vtkSetMacro(DistanceAnnotationVisibility, int);
  vtkBooleanMacro(DistanceAnnotationVisibility, int);

  // Description:
  // get/set the resolution (number of subdivisions) of the line.
  vtkGetMacro(Resolution, int);
  vtkSetMacro(Resolution, int);
  
  // Description:
  // get/set the point representation colour
  vtkGetVector3Macro(PointColour, double);
  vtkSetVector3Macro(PointColour, double);

  // Description:
  // get/set the second point representation colour 
  vtkGetVector3Macro(Point2Colour, double);
  vtkSetVector3Macro(Point2Colour, double);
  
  // Description:
  // get/set the line representation colour
  vtkGetVector3Macro(LineColour, double);
  vtkSetVector3Macro(LineColour, double);

  // Description:
  // get/set the distance annotation text colour
  vtkGetVector3Macro(DistanceAnnotationTextColour, double);
  vtkSetVector3Macro(DistanceAnnotationTextColour, double);

  // Description:
  // get/set the id of the model the first and second end of the ruler is constrained upon
  vtkGetStringMacro(ModelID1);
  vtkSetStringMacro(ModelID1);
  vtkGetStringMacro(ModelID2);
  vtkSetStringMacro(ModelID2);
  
  // Description:
  // Create instance of a measurements ruler node.
  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Set node attributes from name/value pairs
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  
  
  // Description:
  // Get unique node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "MeasurementsRuler"; };

  // Description:
  // transform utility functions
  virtual bool CanApplyNonLinearTransforms() { return true; }
  virtual void ApplyTransform(vtkMatrix4x4* transformMatrix);
  virtual void ApplyTransform(vtkAbstractTransform* transform);

protected:
  vtkMRMLMeasurementsRulerNode();
  ~vtkMRMLMeasurementsRulerNode();
  vtkMRMLMeasurementsRulerNode(const vtkMRMLMeasurementsRulerNode&);
  void operator=(const vtkMRMLMeasurementsRulerNode&);

  // Description:
  // first point location
  double Position1[3];

  // Description:
  // second point location
  double Position2[3];

  // Description:
  // the distance text properties
  char *DistanceAnnotationFormat;
  double DistanceAnnotationScale[3];
  int DistanceAnnotationVisibility;

  // Description:
  // colours of the actors representing the end points and the line and the text
  double PointColour[3];
  double Point2Colour[3];
  double LineColour[3];
  double DistanceAnnotationTextColour[3];

  // Description:
  // number of subdivisions on the line
  int Resolution;

  // Description:
  // the model ids for the models that the ends of the ruler are constrained
  // to
  char *ModelID1;
  char *ModelID2;
};

#endif

