/*=auto=========================================================================

Portions (c) Copyright 2017 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#include "vtkMRMLMeasurement.h"

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
  if (!this->PrintFormat)
    {
    return "";
    }
  if (!this->ValueDefined)
    {
    return "(undefined)";
    }
  char buf[80] = { 0 };
  snprintf(buf, sizeof(buf) - 1, this->PrintFormat, this->Value, this->Units);
  return buf;
}

//----------------------------------------------------------------------------
void vtkMRMLMeasurement::Initialize()
{
  this->SetEnabled(true);
  this->SetName(nullptr);
  this->SetValue(0.0);
  this->ValueDefined = false;
  this->SetUnits(nullptr);
  this->SetPrintFormat("%5.3f %s");
  this->SetDescription(nullptr);
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
void vtkMRMLMeasurement::ClearValue()
{
  this->SetValue(0.0);
  this->ValueDefined = false;

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
  os << indent << "Name: " << (this->Name ? this->Name : "(none)") << "\n";
  os << indent << "PrintableValue: " << this->GetValueWithUnitsAsPrintableString();
  os << indent << "Value: " << this->Value << "\n";
  os << indent << "ValueDefined: " << (this->ValueDefined ? "true" : "false") << "\n";
  os << indent << "Units: " << (this->Units ? this->Units : "(none)") << "\n";
  os << indent << "PrintFormat: " << (this->PrintFormat ? this->PrintFormat : "(none)") << "\n";
  os << indent << "Description: " << (this->Description ? this->Description : "(none)") << "\n";
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
std::string vtkMRMLMeasurement::GetAsString()
{
  std::string str;
  /* TODO: implement
  if (this->CodeValue)
    {
    str += "CodeValue:";
    str += this->CodeValue;
    }
  if (this->CodingSchemeDesignator)
    {
    if (!str.empty())
      {
      str += "|";
      }
    str += "CodingSchemeDesignator:";
    str += this->CodingSchemeDesignator;
    }
  if (this->CodeMeaning)
    {
    if (!str.empty())
      {
      str += "|";
      }
    str += "CodeMeaning:";
    str += this->CodeMeaning;
    }
    */
  return str;
}

//----------------------------------------------------------------------------
bool vtkMRMLMeasurement::SetFromString(const std::string& vtkNotUsed(content))
{
  this->Initialize();
  bool success = true;
  /* TODO: implement
  std::stringstream attributes(content);
  std::string attribute;
  while (std::getline(attributes, attribute, '|'))
    {
    int colonIndex = attribute.find(':');
    std::string name = attribute.substr(0, colonIndex);
    std::string value = attribute.substr(colonIndex + 1);
    if (name == "CodeValue")
      {
      this->SetCodeValue(value.c_str());
      }
    else if (name == "CodingSchemeDesignator")
      {
      this->SetCodingSchemeDesignator(value.c_str());
      }
    else if (name == "CodeMeaning")
      {
      this->SetCodeMeaning(value.c_str());
      }
    else
      {
      vtkWarningMacro("Parsing coded entry string failed: unknown name " << name << " in " + content);
      success = false;
      }
    }
  if (this->GetCodeValue() == nullptr)
    {
    vtkWarningMacro("Parsing coded entry string failed: CodeValue is not specified in " + content);
    success = false;
    }
  if (this->GetCodingSchemeDesignator() == nullptr)
    {
    vtkWarningMacro("Parsing coded entry string failed: CodingSchemeDesignator is not specified in " + content);
    success = false;
    }
  // CodeMeaning is optional
  */
  return success;
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
  this->InputMRMLNode = node;
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
  if (this->Value != value)
  {
    this->Value = value;
    this->ValueDefined = true;
    this->Modified();
  }
}
