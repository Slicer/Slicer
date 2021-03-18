/*=auto=========================================================================

Portions (c) Copyright 2017 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#include "vtkMRMLMeasurement.h"

// MRML include
#include "vtkMRMLUnitNode.h"

// VTK include
#include <vtkObjectFactory.h>

// STD include
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLMeasurement::vtkMRMLMeasurement()
{
  this->SetPrintFormat("%5.3f %s");
}

//----------------------------------------------------------------------------
vtkMRMLMeasurement::~vtkMRMLMeasurement()
{
  this->Initialize();
}

//----------------------------------------------------------------------------
std::string vtkMRMLMeasurement::GetValueWithUnitsAsPrintableString()
{
  if (this->PrintFormat.empty())
    {
    return "";
    }
  if (!this->ValueDefined)
    {
    return "(undefined)";
    }
  char buf[80] = { 0 };
  snprintf(buf, sizeof(buf) - 1, this->PrintFormat.c_str(), this->Value, this->Units.c_str());
  return buf;
}

//----------------------------------------------------------------------------
void vtkMRMLMeasurement::Initialize()
{
  this->SetEnabled(true);
  this->SetValue(0.0);
  this->ValueDefined = false;
  this->SetPrintFormat("%5.3f %s");
  this->SetQuantityCode(nullptr);
  this->SetDerivationCode(nullptr);
  this->SetUnitsCode(nullptr);
  this->SetMethodCode(nullptr);
  this->SetControlPointValues(nullptr);
#ifdef USE_POLYDATA_MEASUREMENTS
  this->SetPolyDataValues(nullptr);
#endif
  this->SetInputMRMLNode(nullptr);
}

//----------------------------------------------------------------------------
void vtkMRMLMeasurement::ClearValue(ComputationResult computationResult/*=NoChange*/)
{
  bool modified = false;
  if (computationResult != NoChange)
    {
    if (this->LastComputationResult != computationResult)
      {
      this->LastComputationResult = computationResult;
      modified = true;
      }
    }
  if (this->Value != 0.0)
    {
    this->Value = 0.0;
    modified = true;
    }
  if (this->ValueDefined)
    {
    this->ValueDefined = false;
    modified = true;
    }
  if (modified)
    {
    this->Modified();
    }

  // Note: this->SetControlPointValues(nullptr); is not called here, because if we clear it here
  // then every time something in the markups node changes that calls curveGenerator->Modified()
  // that is supposed to use just these control point values, the UpdateMeasurementsInternal call
  // clears the value, thus deleting the control point data.
}

//----------------------------------------------------------------------------
void vtkMRMLMeasurement::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  os << indent << "Enabled: " << (this->Enabled ? "true" : "false") << "\n";
  os << indent << "Name: " << this->Name << "\n";
  os << indent << "PrintableValue: " << this->GetValueWithUnitsAsPrintableString();
  os << indent << "Value: " << this->Value << "\n";
  os << indent << "ValueDefined: " << (this->ValueDefined ? "true" : "false") << "\n";
  os << indent << "Units: " << this->Units << "\n";
  os << indent << "PrintFormat: " << this->PrintFormat << "\n";
  os << indent << "Description: " << this->Description << "\n";
  if (this->QuantityCode)
    {
    os << indent << "Quantity: " << this->QuantityCode->GetAsPrintableString() << std::endl;
    }
  if (this->DerivationCode)
    {
    os << indent << "Derivation: " << this->DerivationCode->GetAsPrintableString() << std::endl;
    }
  if (this->UnitsCode)
    {
    os << indent << "Units: " << this->UnitsCode->GetAsPrintableString() << std::endl;
    }
  if (this->MethodCode)
    {
    os << indent << "Method: " << this->MethodCode->GetAsPrintableString() << std::endl;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMeasurement::Copy(vtkMRMLMeasurement* src)
{
  if (!src)
    {
    return;
    }
  this->SetEnabled(src->Enabled);
  this->SetName(src->GetName());
  this->SetValue(src->GetValue());
  this->ValueDefined = src->GetValueDefined();
  this->SetUnits(src->GetUnits());
  this->SetPrintFormat(src->GetPrintFormat());
  this->SetDescription(src->GetDescription());
  this->SetQuantityCode(src->QuantityCode);
  this->SetDerivationCode(src->DerivationCode);
  this->SetUnitsCode(src->UnitsCode);
  this->SetMethodCode(src->MethodCode);
  this->SetControlPointValues(src->ControlPointValues);
  this->SetInputMRMLNode(src->InputMRMLNode);
#ifdef USE_POLYDATA_MEASUREMENTS
  this->SetPolyDataValues(src->PolyDataValues);
#endif
}

//----------------------------------------------------------------------------
void vtkMRMLMeasurement::SetEnabled(bool enabled)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting Enabled to " << enabled);
  if (this->Enabled != enabled)
    {
    this->Enabled = enabled;
    this->Modified();
    this->InvokeEvent(InputDataModifiedEvent);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMeasurement::EnabledOn()
{
  this->SetEnabled(true);
}

//----------------------------------------------------------------------------
void vtkMRMLMeasurement::EnabledOff()
{
  this->SetEnabled(false);
}

//----------------------------------------------------------------------------
void vtkMRMLMeasurement::SetUnits(std::string units)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting Units to " << units);
  if (this->Units != units)
    {
    this->Units = units;
    this->Modified();
    this->InvokeEvent(InputDataModifiedEvent);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMeasurement::SetPrintFormat(std::string format)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting PrintFormat to " << format);
  if (this->PrintFormat != format)
    {
    this->PrintFormat = format;
    this->Modified();
    this->InvokeEvent(InputDataModifiedEvent);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMeasurement::SetQuantityCode(vtkCodedEntry* entry)
{
  if (!entry)
    {
    if (this->QuantityCode)
      {
      this->QuantityCode->Delete();
      this->QuantityCode = nullptr;
      }
    return;
    }
  if (!this->QuantityCode)
    {
    this->QuantityCode = vtkCodedEntry::New();
    }
  this->QuantityCode->Copy(entry);
}

//----------------------------------------------------------------------------
void vtkMRMLMeasurement::SetDerivationCode(vtkCodedEntry* entry)
{
  if (!entry)
    {
    if (this->DerivationCode)
      {
      this->DerivationCode->Delete();
      this->DerivationCode = nullptr;
      }
    return;
    }
  if (!this->DerivationCode)
    {
    this->DerivationCode = vtkCodedEntry::New();
    }
  this->DerivationCode->Copy(entry);
}

//----------------------------------------------------------------------------
void vtkMRMLMeasurement::SetUnitsCode(vtkCodedEntry* entry)
{
  if (!entry)
    {
    if (this->UnitsCode)
      {
      this->UnitsCode->Delete();
      this->UnitsCode = nullptr;
      }
    return;
    }
  if (!this->UnitsCode)
    {
    this->UnitsCode = vtkCodedEntry::New();
    }
  this->UnitsCode->Copy(entry);
}

//----------------------------------------------------------------------------
void vtkMRMLMeasurement::SetMethodCode(vtkCodedEntry* entry)
{
  if (!entry)
    {
    if (this->MethodCode)
      {
      this->MethodCode->Delete();
      this->MethodCode = nullptr;
      }
    return;
    }
  if (!this->MethodCode)
    {
    this->MethodCode = vtkCodedEntry::New();
    }
  this->MethodCode->Copy(entry);
}

//----------------------------------------------------------------------------
void vtkMRMLMeasurement::SetControlPointValues(vtkDoubleArray* inputValues)
{
  if (!inputValues)
    {
    if (this->ControlPointValues)
      {
      this->ControlPointValues->Delete();
      this->ControlPointValues = nullptr;
      }
    return;
    }
  if (!this->ControlPointValues)
    {
    this->ControlPointValues = vtkDoubleArray::New();
    }
  this->ControlPointValues->DeepCopy(inputValues);
}

#ifdef USE_POLYDATA_MEASUREMENTS
//----------------------------------------------------------------------------
void vtkMRMLMeasurement::SetPolyDataValues(vtkPolyData* inputValues)
{
  if (!inputValues)
    {
    if (this->PolyDataValues)
      {
      this->PolyDataValues->Delete();
      this->PolyDataValues = nullptr;
      }
    return;
    }
  if (!this->PolyDataValues)
    {
    this->PolyDataValues = vtkPolyData::New();
    }
  this->PolyDataValues->DeepCopy(inputValues);
}
#endif

//----------------------------------------------------------------------------
void vtkMRMLMeasurement::SetInputMRMLNode(vtkMRMLNode* node)
{
  if (this->InputMRMLNode != node)
    {
    this->InputMRMLNode = node;
    this->Modified();
    this->InvokeEvent(InputDataModifiedEvent);
    }
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLMeasurement::GetInputMRMLNode()
{
  if (this->InputMRMLNode.GetPointer())
    {
    return this->InputMRMLNode.GetPointer();
    }

  return nullptr;
}

//----------------------------------------------------------------------------
const char* vtkMRMLMeasurement::GetLastComputationResultAsPrintableString()
{
  switch (this->LastComputationResult)
    {
    case OK: return "OK";
    case InsufficientInput: return "Insufficient input";
    case InternalError: return "Internal error";
    default:
      // invalid id
      return "";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMeasurement::SetValue(double value)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting Value to " << value);
  bool modified = false;
  if (this->Value != value)
    {
    this->Value = value;
    modified = true;
    }
  if (this->ValueDefined != true)
    {
    this->ValueDefined = true;
    modified = true;
    }
  if (modified)
    {
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMeasurement::SetValue(double value, vtkMRMLUnitNode* unitNode, int lastComputationResult,
  const std::string& defaultUnits, double defaultDisplayCoefficient, const std::string& defaultPrintFormat)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting Value to " << value);

  std::string printFormat;
  std::string units;
  if (unitNode)
    {
    if (unitNode->GetSuffix())
      {
      units = unitNode->GetSuffix();
      }
    value = unitNode->GetDisplayValueFromValue(value);
    printFormat = unitNode->GetDisplayStringFormat();
    }
  else
    {
    units = defaultUnits;
    value *= defaultDisplayCoefficient;
    printFormat = defaultPrintFormat;
    }

  bool modified = false;
  if (this->Value != value)
    {
    this->Value = value;
    modified = true;
    }
  if (this->ValueDefined != true)
    {
    this->ValueDefined = true;
    modified = true;
    }
  if (this->Units != units)
    {
    this->Units = units;
    modified = true;
    }
  if (this->PrintFormat != printFormat)
    {
    this->PrintFormat = printFormat;
    modified = true;
    }
  if (lastComputationResult != ComputationResult::NoChange)
    {
    if (this->LastComputationResult != static_cast<ComputationResult>(lastComputationResult))
      {
      this->LastComputationResult = static_cast<ComputationResult>(lastComputationResult);
      modified = true;
      }
    }
  if (modified)
    {
    this->Modified();
    }
}
