/*=auto=========================================================================

Portions (c) Copyright 2017 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#ifndef __vtkMRMLMeasurement_h
#define __vtkMRMLMeasurement_h

// MRML includes
#include <vtkCodedEntry.h>
#include "vtkMRML.h"

// VTK includes
#include <vtkObject.h>
#include <vtkSmartPointer.h>

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

  static vtkMRMLMeasurement *New();
  vtkTypeMacro(vtkMRMLMeasurement, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Reset state of object
  virtual void Initialize();

  /// Copy one type into another (deep copy)
  virtual void Copy(vtkMRMLMeasurement* aEntry);

  /// Measurement name
  vtkGetStringMacro(Name);
  vtkSetStringMacro(Name);

  /// Measured quantity value
  vtkGetMacro(Value, double);
  vtkSetMacro(Value, double);

  /// Measurement unit
  vtkGetStringMacro(Units);
  vtkSetStringMacro(Units);

  /// Informal description of the measurement
  vtkGetStringMacro(Description);
  vtkSetStringMacro(Description);

  /// Formatting string for displaying measurement value and units
  vtkGetStringMacro(PrintFormat);
  vtkSetStringMacro(PrintFormat);

  // Copies content of coded entry
  void SetQuantityCode(vtkCodedEntry* entry);
  vtkGetObjectMacro(QuantityCode, vtkCodedEntry);

  // Copies content of coded entry
  void SetDerivationCode(vtkCodedEntry* entry);
  vtkGetObjectMacro(DerivationCode, vtkCodedEntry);

  // Copies content of coded entry
  void SetUnitsCode(vtkCodedEntry* entry);
  vtkGetObjectMacro(UnitsCode, vtkCodedEntry);

  // Copies content of coded entry
  void SetMethodCode(vtkCodedEntry* entry);
  vtkGetObjectMacro(MethodCode, vtkCodedEntry);

  ///
  /// Get measurement value and units as a single human-readable string.
  std::string GetValueWithUnitsAsPrintableString();

  ///
  /// Get content of the object as a single machine-readable string.
  std::string GetAsString();

  ///
  /// Set content of the object from a single machine-readable string.
  /// \return true on success
  bool SetFromString(const std::string& content);

protected:
  vtkMRMLMeasurement();
  ~vtkMRMLMeasurement() override;
  vtkMRMLMeasurement(const vtkMRMLMeasurement&);
  void operator=(const vtkMRMLMeasurement&);

protected:
  char* Name{nullptr};
  double Value{0.0};
  char* Units{nullptr};
  char* Description{nullptr};
  char* PrintFormat{nullptr}; // for value (double), unit (string)
  vtkCodedEntry* QuantityCode{nullptr};   // volume
  vtkCodedEntry* DerivationCode{nullptr}; // min/max/mean
  vtkCodedEntry* UnitsCode{nullptr};      // cubic millimeter
  vtkCodedEntry* MethodCode{nullptr};     // Sum of segmented voxel volumes
};

#endif
