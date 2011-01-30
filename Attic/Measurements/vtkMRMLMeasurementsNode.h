#ifndef __vtkMRMLMeasurementsNode_h
#define __vtkMRMLMeasurementsNode_h

#include "vtkStringArray.h"

#include "vtkMRML.h"
#include "vtkMRMLTransformableNode.h"

#include "vtkMeasurementsWin32Header.h"
#include <string>
#include <vector>


class VTK_MEASUREMENTS_EXPORT vtkMRMLMeasurementsNode : public vtkMRMLTransformableNode
{
  public:
  static vtkMRMLMeasurementsNode *New();
  vtkTypeRevisionMacro(vtkMRMLMeasurementsNode, vtkMRMLTransformableNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// 
  /// get/set if the widget is visible
  vtkBooleanMacro(Visibility, int);
  vtkGetMacro(Visibility, int);
  vtkSetMacro(Visibility, int);
  
  /// Description:
  /// Create instance of a Measurements node.
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
  /// transform utility functions, need to be defined by subclasses
  virtual bool CanApplyNonLinearTransforms() { return true; }
  virtual void ApplyTransform(vtkMatrix4x4* vtkNotUsed(transformMatrix)) {};
  virtual void ApplyTransform(vtkAbstractTransform* vtkNotUsed(transform)) {};
  
  
  /// 
  /// Get unique node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "Measurements"; };

 protected:
  vtkMRMLMeasurementsNode();
  ~vtkMRMLMeasurementsNode();
  vtkMRMLMeasurementsNode(const vtkMRMLMeasurementsNode&);
  void operator=(const vtkMRMLMeasurementsNode&);

  /// 
  /// is the widget visible?
  int Visibility;
};

#endif

