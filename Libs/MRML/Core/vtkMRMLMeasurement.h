/*=auto=========================================================================

Portions (c) Copyright 2017 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#ifndef __vtkMRMLMeasurement_h
#define __vtkMRMLMeasurement_h

// #define USE_POLYDATA_MEASUREMENTS

// MRML includes
#include <vtkCodedEntry.h>
#include "vtkMRMLNode.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkDoubleArray.h>
#include <vtkObject.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

class vtkMRMLUnitNode;

/// \brief Class for storing well-defined measurement results, using coded entries.
///
/// This stores all important information about a measurement using standard coded entries.
/// Measurement method, derivation, quantity value, units, etc, can be specified.
/// This is a commonly used concept in DICOM structured reports.
///
/// The measurement stores value, displayed value, and unit consistently with unit nodes.
///
/// DisplayValue is defined in the unit specified in Unit property of the measurement.
/// Value is defined in the base unit specified in the unit node in the scene.
/// DisplayCoefficient specifies scaling between Value and DisplayValue:
///
/// DisplayValue = Value * DisplayCoefficient.
///
/// Example:
///
/// In medical image computing, mm is commonly used as a length unit, but volume is most often
/// specified in cc (cm3). To allow performing all computations without unit conversions (e.g.,
/// compute volume = width * height * depth) it is useful to specify volume unit as cm3
/// with volume display coefficient of 0.001:
/// - Quantity=length, Unit=mm, DisplayCoefficient=1.0
/// - Quantity=volume, Unit=cm3, DisplayCoefficient=0.001
///
/// Measurements can then computed and stored like this:
///
/// <code>
/// // box size in mm: width * height * depth
/// diameterMeasurement->SetValue(sqrt(width*width+height*height+depth*depth), "length");
/// volumeMeasurement->SetValue(width*height*depth, "volume");
/// </code>
///
/// \sa vtkCodedEntry
///
class VTK_MRML_EXPORT vtkMRMLMeasurement : public vtkObject
{
public:
  /// Measurement computation status
  /// \sa LastComputationResult, GetLastComputationResult(),
  /// GetLastComputationResultAsString()
  enum ComputationResult
  {
    NoChange = 0, ///< can be used to indicate to keep the current value
    OK,           ///< success
    InsufficientInput,
    InternalError
  };

  enum Events
  {
    /// The node stores both inputs (e.g., input node, enabled, unit, etc.) and computed measurement.
    /// InputDataModifiedEvent is only invoked when input parameters are changed.
    /// In contrast, ModifiedEvent event is called if either an input or output parameter is changed.
    // vtkCommand::UserEvent + 555 is just a random value that is very unlikely to be used for anything else in this
    // class
    InputDataModifiedEvent = vtkCommand::UserEvent + 555
  };

  vtkTypeMacro(vtkMRMLMeasurement, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Create a new instance of this measurement type.
  /// Only in C++: The caller must take ownership of the returned object.
  VTK_NEWINSTANCE
  virtual vtkMRMLMeasurement* CreateInstance() const = 0;

  /// Reset state of object. Removes all content.
  virtual void Clear();

  /// Clear measured value
  /// Note: per-control-point values are not cleared
  virtual void ClearValue(ComputationResult computationResult = NoChange);

  /// Copy one type into another (deep copy)
  virtual void Copy(vtkMRMLMeasurement* source);

  /// Perform calculation on InputMRMLNode and store the result internally.
  /// The subclasses need to implement this function
  virtual void Compute() = 0;

  /// Enabled
  vtkGetMacro(Enabled, bool);
  virtual void SetEnabled(bool enabled);
  virtual void EnabledOn();
  virtual void EnabledOff();

  /// Measurement name
  vtkGetMacro(Name, std::string);
  vtkSetMacro(Name, std::string);

  /// Set value and units with a single modified event.
  /// If a value is set for a quantity that has a corresponding unit node in the scene then the value
  /// must be consistent with that definition.
  /// If quantityName is specified then Units, DisplayCoefficient, PrintFormat properties
  /// is updated from the corresponding unit node's Suffix, DisplayCoefficient, and DisplayStringFormat
  /// If QuantityName is specified then a valid InputMRMLNode must be set, too, because the scene is accessed
  /// via the InputMRMLNode.
  void SetValue(double value, const char* quantityName = nullptr);

  /// Get quantity value.
  vtkGetMacro(Value, double);

  /// Set display value and units with a single modified event.
  /// This method is useful if there is no unit node corresponding to this quantity in the scene.
  /// If a unit node is available for the measurement's quantity then it is important to set the correct
  /// displayCoefficient value for the chosen units.
  /// If units and/or displayCoefficient is not specified then the current Units and/or DisplayCoefficient values are
  /// used. The stored value is computed as displayValue / DisplayCoefficient.
  void SetDisplayValue(double value, const char* units = nullptr, double displayCoefficient = 0.0);

  /// Get display value.
  /// It is computed using this formula: DisplayValue = Value * DisplayCoefficient.
  double GetDisplayValue();

  /// Value defined flag (whether a computed value has been set or not)
  vtkGetMacro(ValueDefined, bool);

  /// Measurement unit
  vtkGetMacro(Units, std::string);
  virtual void SetUnits(std::string units);

  /// This multiplier will be applied to the value to compute display value:
  /// It is useful when the unit in the measurement differs from the base unit (e.g., application's length unit is mm
  /// but volume is displayed as cm3 instead of mm3).
  vtkGetMacro(DisplayCoefficient, double);
  vtkSetMacro(DisplayCoefficient, double);

  /// Informal description of the measurement
  vtkGetMacro(Description, std::string);
  vtkSetMacro(Description, std::string);

  /// Formatting string for displaying measurement value and units
  vtkGetMacro(PrintFormat, std::string);
  virtual void SetPrintFormat(std::string format);

  /// Copy content of coded entry
  void SetQuantityCode(vtkCodedEntry* entry);
  virtual vtkCodedEntry* GetQuantityCode();

  /// Copy content of coded entry
  void SetDerivationCode(vtkCodedEntry* entry);
  virtual vtkCodedEntry* GetDerivationCode();

  /// Copy content of coded entry
  void SetUnitsCode(vtkCodedEntry* entry);
  virtual vtkCodedEntry* GetUnitsCode();

  /// Copy content of coded entry
  void SetMethodCode(vtkCodedEntry* entry);
  virtual vtkCodedEntry* GetMethodCode();

  /// Get last computation result
  vtkGetMacro(LastComputationResult, int);
  /// Get last computation result as human-readable string
  const char* GetLastComputationResultAsPrintableString();

  /// Get measurement value and units as a single human-readable string.
  std::string GetValueWithUnitsAsPrintableString();

  /// Set the per-control point measurement values
  void SetControlPointValues(vtkDoubleArray* inputValues);
  vtkGetObjectMacro(ControlPointValues, vtkDoubleArray);

  /// Set mesh that can be used to visualize to computed value.
  /// For example, mesh for a calculated area value is the mesh that was generated
  /// to compute the surface area.
  void SetMeshValue(vtkPolyData* meshValue);
  virtual vtkPolyData* GetMeshValue();

  /// Set input MRML node used for calculating the measurement \sa Execute
  void SetInputMRMLNode(vtkMRMLNode* node);
  /// Get input MRML node used for calculating the measurement \sa Execute
  vtkMRMLNode* GetInputMRMLNode();

protected:
  vtkMRMLMeasurement();
  ~vtkMRMLMeasurement() override;
  vtkMRMLMeasurement(const vtkMRMLMeasurement&);
  void operator=(const vtkMRMLMeasurement&);

  /// Helper function to get unit node from the scene based on quantity name.
  vtkMRMLUnitNode* GetUnitNode(const char* quantityName);

protected:
  bool Enabled{ true };
  std::string Name;
  double Value{ 0.0 };
  bool ValueDefined{ false };
  double DisplayCoefficient{ 1.0 };
  std::string Units;
  std::string Description;
  std::string PrintFormat;                       // for value (double), unit (string)
  vtkSmartPointer<vtkCodedEntry> QuantityCode;   // volume
  vtkSmartPointer<vtkCodedEntry> DerivationCode; // min/max/mean
  vtkSmartPointer<vtkCodedEntry> UnitsCode;      // cubic millimeter
  vtkSmartPointer<vtkCodedEntry> MethodCode;     // Sum of segmented voxel volumes
  ComputationResult LastComputationResult{ InsufficientInput };

  /// Per-control point measurements.
  vtkSmartPointer<vtkDoubleArray> ControlPointValues;
  /// Surface mesh for displaying computed value.
  vtkSmartPointer<vtkPolyData> MeshValue;
  /// MRML node used to calculate the measurement \sa Execute
  vtkWeakPointer<vtkMRMLNode> InputMRMLNode;
};

#endif
