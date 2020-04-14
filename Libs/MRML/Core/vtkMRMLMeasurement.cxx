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

vtkStandardNewMacro(vtkMRMLMeasurement);

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
    return  "";
    }
  char buf[80] = { 0 };
  snprintf(buf, sizeof(buf) - 1, this->PrintFormat, this->Value, this->Units);
  return buf;
}

//----------------------------------------------------------------------------
void vtkMRMLMeasurement::Initialize()
{
  this->SetName(nullptr);
  this->SetValue(0.0);
  this->SetUnits(nullptr);
  this->SetPrintFormat("%5.3f %s");
  this->SetDescription(nullptr);
  this->SetQuantityCode(nullptr);
  this->SetDerivationCode(nullptr);
  this->SetUnitsCode(nullptr);
  this->SetMethodCode(nullptr);
}

//----------------------------------------------------------------------------
void vtkMRMLMeasurement::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  os << indent << "Name: " << (this->Name ? this->Name : "(none)") << "\n";
  os << indent << "PrintableValue: " << this->GetValueWithUnitsAsPrintableString();
  os << indent << "Value: " << this->Value << "\n";
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
  this->SetName(src->GetName());
  this->SetValue(src->GetValue());
  this->SetUnits(src->GetUnits());
  this->SetPrintFormat(src->GetPrintFormat());
  this->SetDescription(src->GetDescription());
  this->SetQuantityCode(src->QuantityCode);
  this->SetDerivationCode(src->DerivationCode);
  this->SetUnitsCode(src->UnitsCode);
  this->SetMethodCode(src->MethodCode);
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
