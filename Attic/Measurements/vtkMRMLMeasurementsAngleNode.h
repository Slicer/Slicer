#ifndef __vtkMRMLMeasurementsAngleNode_h
#define __vtkMRMLMeasurementsAngleNode_h

#include "vtkMeasurementsWin32Header.h"

#include "vtkMRMLMeasurementsNode.h"
class VTK_MEASUREMENTS_EXPORT vtkMRMLMeasurementsAngleNode : public vtkMRMLMeasurementsNode
{
  public:
  static vtkMRMLMeasurementsAngleNode *New();
  vtkTypeRevisionMacro(vtkMRMLMeasurementsAngleNode, vtkMRMLMeasurementsNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// 
  /// get/set the first point position
  vtkGetVector3Macro(Position1, double);
  vtkSetVector3Macro(Position1, double);

  /// 
  /// get/set the second point position
  vtkGetVector3Macro(Position2, double);
  vtkSetVector3Macro(Position2, double);

  /// 
  /// get/set the center point position
  vtkGetVector3Macro(PositionCenter, double);
  vtkSetVector3Macro(PositionCenter, double);

  /// get/set the current angle between the two lines. GetAngle
  /// calculates it and sets the Angle class variable.
  double GetAngle();
  vtkSetMacro(Angle, double);
  /// 
  /// get/set the angle annotation format, it's in standard sprintf notation
  vtkGetStringMacro(LabelFormat);
  vtkSetStringMacro(LabelFormat);

  ///  get/set the current angle annotation (used by the 2d versions)
  vtkGetStringMacro(CurrentAngleAnnotation);
  vtkSetStringMacro(CurrentAngleAnnotation);
  
  /// 
  /// get/set the angle annotation scale
  vtkGetVector3Macro(LabelScale, double);
  vtkSetVector3Macro(LabelScale, double);
  
  /// 
  /// get/set the angle annotation visbility
  vtkGetMacro(LabelVisibility, int);
  vtkSetMacro(LabelVisibility, int);
  vtkBooleanMacro(LabelVisibility, int);

  /// 
  /// get/set the visibility of the rays and arc
  vtkGetMacro(Ray1Visibility, int);
  vtkSetMacro(Ray1Visibility, int);
  vtkBooleanMacro(Ray1Visibility, int);
  vtkGetMacro(Ray2Visibility, int);
  vtkSetMacro(Ray2Visibility, int);
  vtkBooleanMacro(Ray2Visibility, int);
  vtkGetMacro(ArcVisibility, int);
  vtkSetMacro(ArcVisibility, int);
  vtkBooleanMacro(ArcVisibility, int);
  
  /// 
  /// get/set the resolution (number of subdivisions) of the line.
  vtkGetMacro(Resolution, int);
  vtkSetMacro(Resolution, int);
  
  /// 
  /// get/set the point representation colours
  vtkGetVector3Macro(PointColour, double);
  vtkSetVector3Macro(PointColour, double);
  vtkGetVector3Macro(Point2Colour, double);
  vtkSetVector3Macro(Point2Colour, double);
  vtkGetVector3Macro(PointCentreColour, double);
  vtkSetVector3Macro(PointCentreColour, double);
  
  /// 
  /// get/set the line representation colour
  vtkGetVector3Macro(LineColour, double);
  vtkSetVector3Macro(LineColour, double);

  ///
  /// get/set the arc colour
  vtkGetVector3Macro(ArcColour, double);
  vtkSetVector3Macro(ArcColour, double);
  
  /// 
  /// get/set the angle annotation text colour
  vtkGetVector3Macro(LabelTextColour, double);
  vtkSetVector3Macro(LabelTextColour, double);

  /// 
  /// get/set the id of the model the ends of the widget are constrained upon
  vtkGetStringMacro(ModelID1);
  vtkSetStringMacro(ModelID1);
  vtkGetStringMacro(ModelID2);
  vtkSetStringMacro(ModelID2);
  vtkGetStringMacro(ModelIDCenter);
  vtkSetStringMacro(ModelIDCenter);
  
  
  /// 
  /// Create instance of a measurements angle node.
  virtual vtkMRMLNode* CreateNodeInstance();

  /// 
  /// Set node attributes from name/value pairs
  virtual void ReadXMLAttributes( const char** atts);

  /// 
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// 
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  
  
  /// 
  /// Get unique node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "MeasurementsAngle"; };

  /// 
  /// transform utility functions
  virtual bool CanApplyNonLinearTransforms() { return true; }
  virtual void ApplyTransform(vtkMatrix4x4* transformMatrix);
  virtual void ApplyTransform(vtkAbstractTransform* transform);

  /// update the current annotation from current angle and
  /// annotation format
  void UpdateCurrentAngleAnnotation();
protected:
  vtkMRMLMeasurementsAngleNode();
  ~vtkMRMLMeasurementsAngleNode();
  vtkMRMLMeasurementsAngleNode(const vtkMRMLMeasurementsAngleNode&);
  void operator=(const vtkMRMLMeasurementsAngleNode&);

  /// 
  /// first point location
  double Position1[3];

  /// 
  /// second point location
  double Position2[3];

  /// 
  /// center point location
  double PositionCenter[3];

  /// 
  /// the angle text properties
  char *LabelFormat;
  char *CurrentAngleAnnotation;
  double LabelScale[3];
  int LabelVisibility;

  /// Descritpion:
  /// visibility of sub components of the widget
  int Ray1Visibility;
  int Ray2Visibility;
  int ArcVisibility;

  /// 
  /// colours of the actors representing the end points and the line and the text
  double PointColour[3];
  double Point2Colour[3];
  double PointCentreColour[3];
  double LineColour[3];
  double ArcColour[3];
  double LabelTextColour[3];

  /// 
  /// number of subdivisions on the line
  int Resolution;

  /// 
  /// the model ids for the models that the ends of the widget are constrained
  /// to
  char *ModelID1;
  char *ModelID2;
  char *ModelIDCenter;

  /// current angle, updated externally
  double Angle;
};

#endif

