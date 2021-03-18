/*=auto=========================================================================

Portions (c) Copyright 2017 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#ifndef __vtkMRMLMeasurement_h
#define __vtkMRMLMeasurement_h

//#define USE_POLYDATA_MEASUREMENTS

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
    NoChange=0, ///< can be used to indicate to keep the current value
    OK, ///< success
    InsufficientInput,
    InternalError
    };

  enum Events
  {
    /// The node stores both inputs (e.g., input node, enabled, unit, etc.) and computed measurement.
    /// InputDataModifiedEvent is only invoked when input parameters are changed.
    /// In contrast, ModifiedEvent event is called if either an input or output parameter is changed.
    // vtkCommand::UserEvent + 555 is just a random value that is very unlikely to be used for anything else in this class
    InputDataModifiedEvent = vtkCommand::UserEvent + 555
  };

  vtkTypeMacro(vtkMRMLMeasurement, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Reset state of object
  virtual void Initialize();

  /// Clear measured value
  /// Note: per-control-point values are not cleared
  virtual void ClearValue(ComputationResult computationResult=NoChange);

  /// Copy one type into another (deep copy)
  virtual void Copy(vtkMRMLMeasurement* aEntry);

  /// Perform calculation on \sa InputMRMLNode and store the result internally.
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

  /// Measured quantity value
  vtkGetMacro(Value, double);
  void SetValue(double value);

  /// Set quantity value and units with a single modified event.
  /// If unitNode is nullptr then defaultUnits, defaultDisplayCoefficient, defaultPrintFormat is used
  /// lastComputationResult type is ComputationResult (int type is used for compatibility with Python wrapper)
  void SetValue(double value, vtkMRMLUnitNode* unitNode, int lastComputationResult,
    const std::string& defaultUnits, double defaultDisplayCoefficient, const std::string& defaultPrintFormat);

  /// Value defined flag (whether a computed value has been set or not)
  vtkGetMacro(ValueDefined, bool);

  /// Measurement unit
  vtkGetMacro(Units, std::string);
  virtual void SetUnits(std::string units);

  /// Informal description of the measurement
  vtkGetMacro(Description, std::string);
  vtkSetMacro(Description, std::string);

  /// Formatting string for displaying measurement value and units
  vtkGetMacro(PrintFormat, std::string);
  virtual void SetPrintFormat(std::string format);

  /// Copy content of coded entry
  void SetQuantityCode(vtkCodedEntry* entry);
  vtkGetObjectMacro(QuantityCode, vtkCodedEntry);

  /// Copy content of coded entry
  void SetDerivationCode(vtkCodedEntry* entry);
  vtkGetObjectMacro(DerivationCode, vtkCodedEntry);

  /// Copy content of coded entry
  void SetUnitsCode(vtkCodedEntry* entry);
  vtkGetObjectMacro(UnitsCode, vtkCodedEntry);

  /// Copy content of coded entry
  void SetMethodCode(vtkCodedEntry* entry);
  vtkGetObjectMacro(MethodCode, vtkCodedEntry);

  /// Get last computation result
  vtkGetMacro(LastComputationResult, int);
  /// Get last computation result as human-readable string
  const char* GetLastComputationResultAsPrintableString();

  /// Get measurement value and units as a single human-readable string.
  std::string GetValueWithUnitsAsPrintableString();

  /// Set the per-control point measurement values
  void SetControlPointValues(vtkDoubleArray* inputValues);
  vtkGetObjectMacro(ControlPointValues, vtkDoubleArray);

#ifdef USE_POLYDATA_MEASUREMENTS
  /// Set the per-polydata point measurement values
  void SetPolyDataValues(vtkPolyData* inputValues);
  vtkGetObjectMacro(PolyDataValues, vtkPolyData);
#endif

  /// Set input MRML node used for calculating the measurement \sa Execute
  void SetInputMRMLNode(vtkMRMLNode* node);
  /// Get input MRML node used for calculating the measurement \sa Execute
  vtkMRMLNode* GetInputMRMLNode();

protected:
  vtkMRMLMeasurement();
  ~vtkMRMLMeasurement() override;
  vtkMRMLMeasurement(const vtkMRMLMeasurement&);
  void operator=(const vtkMRMLMeasurement&);

protected:
  bool Enabled{true};
  std::string Name;
  double Value{0.0};
  bool ValueDefined{false};
  std::string Units;
  std::string Description;
  std::string PrintFormat; // for value (double), unit (string)
  vtkCodedEntry* QuantityCode{nullptr};   // volume
  vtkCodedEntry* DerivationCode{nullptr}; // min/max/mean
  vtkCodedEntry* UnitsCode{nullptr};      // cubic millimeter
  vtkCodedEntry* MethodCode{nullptr};     // Sum of segmented voxel volumes
  ComputationResult LastComputationResult{InsufficientInput};

  /// Per-control point measurements
  vtkDoubleArray* ControlPointValues{nullptr};
#ifdef USE_POLYDATA_MEASUREMENTS
  /// Measurement or displayed surface element stored in poly data
  vtkPolyData* PolyDataValues{nullptr};
#endif
  /// MRML node used to calculate the measurement \sa Execute
  vtkWeakPointer<vtkMRMLNode> InputMRMLNode;
};

#endif
