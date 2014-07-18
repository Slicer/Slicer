/// vtkMRMLVolumePropertyNode - MRML node to represent volume rendering
/// information.
/// This node is especially used to store visualization parameter sets for
/// volume rendering

#ifndef __vtkMRMLVolumePropertyNode_h
#define __vtkMRMLVolumePropertyNode_h

// VolumeRendering includes
#include "vtkSlicerVolumeRenderingModuleMRMLExport.h"

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

/// \brief vtkMRMLVolumePropertyNode contains the transfer functions (scalar
/// opacity, color and gradient opacity) for the volume rendering.
class VTK_SLICER_VOLUMERENDERING_MODULE_MRML_EXPORT vtkMRMLVolumePropertyNode
  : public vtkMRMLStorableNode
{
public:
  //--------------------------------------------------------------------------
  /// OWN methods
  //--------------------------------------------------------------------------

  /// Create a new vtkMRMLVolumePropertyNode
  static vtkMRMLVolumePropertyNode *New();
  vtkTypeMacro(vtkMRMLVolumePropertyNode,vtkMRMLStorableNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Don't change its scalarOpacity, gradientOpacity or color on the volumeproperty
  /// but use the methods below. It wouldn't observe them.
  vtkGetObjectMacro(VolumeProperty,vtkVolumeProperty);

  /// Set the scalar opacity to the volume property.
  /// \sa GetScalarOpacity, GetPiecewiseFunctionString(), SetGradientOpacity(),
  /// SetColor()
  void SetScalarOpacity(vtkPiecewiseFunction* newScalarOpacity, int component = 0);\
  /// Return the scalar opacity function for a given component or 0 if no
  /// function exists.
  /// \sa SetScalarOpacity()
  vtkPiecewiseFunction* GetScalarOpacity(int component = 0);

  /// Set the gradient opacity to the volume property.
  /// \sa GetGradientOpacity(), GetPiecewiseFunctionString(),
  /// SetScalarOpacity(), SetColor()
  void SetGradientOpacity(vtkPiecewiseFunction* newGradientOpacity, int component = 0);
  /// Return the gradient opacity function for a given component or 0 if no
  /// function exists.
  /// \sa SetGradientOpacity()
  vtkPiecewiseFunction* GetGradientOpacity(int component = 0);

  /// Set the color function to the volume property.
  /// \sa GetColor(), GetPiecewiseFunctionString(),
  /// SetScalarOpacity(), SetGradientOpacity()
  void SetColor(vtkColorTransferFunction* newColorFunction, int component = 0);
  /// Return the color transfer function for a given component or 0 if no
  /// function exists.
  /// \sa SetColor()
  vtkColorTransferFunction* GetColor(int component = 0);

  /// Utility function that transforms a piecewise function into a string.
  /// Format:
  /// \verbatim
  /// <numberOfPoints> <XValue1> <OpacityValue1> ... <XValueN> <OpacityValueN>
  /// \endverbatim
  /// \sa GetPiecewiseFunctionFromString(), GetColorTransferFunctionString()
  static std::string GetPiecewiseFunctionString(vtkPiecewiseFunction* function);

  /// Utility function that transforms a color transfer function into a string.
  /// Format:
  /// \verbatim
  /// <numberOfPoints> <XValue1> <RValue1> <GValue1><BValue1> ... <XValueN> <RValueN> <GValueN><BValueN>
  /// \endverbatim
  /// \sa GetColorTransferFunctionFromString(), GetPiecewiseFunctionString()
  static std::string GetColorTransferFunctionString(vtkColorTransferFunction* function);

  /// Utility function:
  /// Put parameters described in a string into an existing
  /// vtkPiecewiseFunction, use together with GetPiecewiseFunctionString
  /// \sa GetPiecewiseFunctionString(), GetColorTransferFunctionFromString()
  static void GetPiecewiseFunctionFromString(const std::string& str,
                                             vtkPiecewiseFunction* result);

  /// Utility function:
  /// Put parameters described in a string into an existing
  /// vtkColorTransferFunction, use together with getColorTransferFunctionString
  /// \sa GetColorTransferFunctionString(), GetPiecewiseFunctionFromString()
  static void GetColorTransferFunctionFromString(const std::string& str,
                                                 vtkColorTransferFunction* result);

  /// Utility function:
  /// Put parameters described in a string into an existing vtkPiecewiseFunction.
  /// To be used with GetPiecewiseFunctionString()
  /// \sa GetPiecewiseFunctionString(), GetPiecewiseFunctionFromString()
  static inline void GetPiecewiseFunctionFromString(const char *str,
                                                    vtkPiecewiseFunction* result);

  /// Utility function:
  /// Put parameters described in a string into an existing
  /// vtkColorTransferFunction.
  /// To be used with GetColorTransferFunctionString()
  /// \sa GetColorTransferFunctionFromString()
  static inline void GetColorTransferFunctionFromString(const char *str,
                                                        vtkColorTransferFunction* result);

  /// Utility function:
  /// Return the nearest higher value.
  /// \sa HigherAndUnique()
  static double NextHigher(double value);
  /// Utility function:
  /// Return the value or the nearest higher value if the value is equal
  /// to previousValue. Update previousValue with the new higher value.
  /// \sa NextHigher()
  static double HigherAndUnique(double value, double & previousValue);

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

  /// \sa vtkMRMLStorableNode::GetModifiedSinceRead()
  virtual bool GetModifiedSinceRead();

protected:
  vtkMRMLVolumePropertyNode(void);
  ~vtkMRMLVolumePropertyNode(void);

  static int NodesFromString(const std::string& dataString, double* &data, int nodeSize);
  static int DataFromString(const std::string& dataString, double* &data);
  static std::string DataToString(double* data, int size);

  /// Events observed on the transfer functions
  vtkIntArray* ObservedEvents;

  /// Main parameters for visualization
  vtkVolumeProperty* VolumeProperty;

private:
  /// Caution: Not implemented
  vtkMRMLVolumePropertyNode(const vtkMRMLVolumePropertyNode&);//Not implemented
  void operator=(const vtkMRMLVolumePropertyNode&);/// Not implmented

};

//---------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode
::GetPiecewiseFunctionFromString(const char *str,
                                 vtkPiecewiseFunction* result)
{
  vtkMRMLVolumePropertyNode::GetPiecewiseFunctionFromString(
    std::string(str), result);
}

//---------------------------------------------------------------------------
void vtkMRMLVolumePropertyNode
::GetColorTransferFunctionFromString(const char *str,
                                     vtkColorTransferFunction* result)
{
  vtkMRMLVolumePropertyNode::GetColorTransferFunctionFromString(
    std::string (str), result);
}

#endif
