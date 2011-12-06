/// vtkMRMLVolumePropertyNode - MRML node to represent volume rendering
/// information.
/// This node is especially used to store visualization parameter sets for
/// volume rendering

#ifndef __vtkMRMLVolumePropertyNode_h
#define __vtkMRMLVolumePropertyNode_h

// MRML includes
#include "vtkMRMLStorableNode.h"

// VTK includes
class vtkColorTransferFunction;
class vtkIntArray;
class vtkPiecewiseFunction;
class vtkVolumeProperty;

// STD includes
#include <string>
#include <vtksys/stl/vector>

#define COUNT_CROPPING_REGION_PLANES 6

class VTK_MRML_EXPORT vtkMRMLVolumePropertyNode : public vtkMRMLStorableNode
{
public:
  //--------------------------------------------------------------------------
  /// OWN methods
  //--------------------------------------------------------------------------


  /// Get a string representation of all points in the piecewise function.
  /// format: <numberOfPoints> <XValue1> <OpacityValue1>
  /// ...<XValueN> <OpacityValueN> 
  std::string GetPiecewiseFunctionString(vtkPiecewiseFunction* function);

  /// Get a string representation of all points in the color transfer function.
  //format: <numberOfPoints> <XValue1> <RValue1> <GValue1><BValue1> 
  //...<XValueN> <RValueN> <GValueN><BValueN>
  std::string GetColorTransferFunctionString(vtkColorTransferFunction* function);

  /// Put parameters described in a String into an existing
  /// vtkPiecewiseFunction, use together with GetPiecewiseFunctionString
  void GetPiecewiseFunctionFromString(const std::string& str,vtkPiecewiseFunction* result);

  /// Put parameters described in a String into an existing
  /// vtkColorTransferFunction, use together with getColorTransferFunctionString
  void GetColorTransferFunctionFromString(const std::string& str, vtkColorTransferFunction* result);


  /// Create a new vtkMRMLVolumePropertyNode
  static vtkMRMLVolumePropertyNode *New();
  vtkTypeMacro(vtkMRMLVolumePropertyNode,vtkMRMLStorableNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Don't change its scalarOpacity, gradientOpacity or color on the volumeproperty
  /// but use the methods below. It wouldn't observe them.
  vtkGetObjectMacro(VolumeProperty,vtkVolumeProperty);

  /// Set the scalar opacity to the volume property
  void SetScalarOpacity(vtkPiecewiseFunction* newScalarOpacity, int component = 0);

  /// Set the gradient opacity to the volume property
  void SetGradientOpacity(vtkPiecewiseFunction* newGradientOpacity, int component = 0);

  /// Set the color function to the volume property
  void SetColor(vtkColorTransferFunction* newColorFunction, int component = 0);


  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------
  virtual vtkMRMLNode* CreateNodeInstance();

  /// Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// Copy only the parameterset (like Volume Propertys, Piecewiesefunctions
  /// etc. as deep copy,but no references etc.)
  void CopyParameterSet(vtkMRMLNode *node);

  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "VolumeProperty";};

  /// Reimplemented for internal reasons.
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData);

  /// Create default storage node or NULL if does not have one
  virtual vtkMRMLStorageNode* CreateDefaultStorageNode();

  /// Put parameters described in a String into an existing vtkPiecewiseFunction, use together with GetPiecewiseFunctionString
  void GetPiecewiseFunctionFromString(char *str,vtkPiecewiseFunction* result) 
  {
    this->GetPiecewiseFunctionFromString(std::string (str), result);
  };

  /// Put parameters described in a String into an existing vtkColorTransferFunction, use together with getColorTransferFunctionString
  void GetColorTransferFunctionFromString(char *str, vtkColorTransferFunction* result)
  {
    GetColorTransferFunctionFromString(std::string (str), result);
  };

protected:
  /// Use ::New() to get a new instance.
  vtkMRMLVolumePropertyNode(void);

  /// Use ->Delete() to delete object
  ~vtkMRMLVolumePropertyNode(void);

  static int dataFromString(const std::string& dataString, double* &data);
  static std::string dataToString(double* data, int size);

  /// Events observed on the transfer functions
  vtkIntArray* ObservedEvents;

  /// Main parameters for visualization
  vtkVolumeProperty* VolumeProperty;

private:
  /// Caution: Not implemented
  vtkMRMLVolumePropertyNode(const vtkMRMLVolumePropertyNode&);//Not implemented
  void operator=(const vtkMRMLVolumePropertyNode&);/// Not implmented

};

#endif
