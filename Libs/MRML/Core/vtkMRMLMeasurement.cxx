/*=auto=========================================================================

Portions (c) Copyright 2017 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#include "vtkMRMLMeasurement.h"

// MRML include
#include "vtkMRMLScene.h"
#include "vtkMRMLSelectionNode.h"
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
  this->Clear();
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
  snprintf(buf, sizeof(buf) - 1, this->PrintFormat.c_str(), this->GetDisplayValue(), this->Units.c_str());
  return buf;
}

//----------------------------------------------------------------------------
void vtkMRMLMeasurement::Clear()
{
  this->SetEnabled(true);
  this->ClearValue(InsufficientInput);
  this->ValueDefined = false;
  this->SetPrintFormat("%5.3f %s");
  this->SetDisplayCoefficient(1.0);
  this->SetQuantityCode(nullptr);
  this->SetDerivationCode(nullptr);
  this->SetUnitsCode(nullptr);
  this->SetMethodCode(nullptr);
  this->SetControlPointValues(nullptr);
  this->SetMeshValue(nullptr);
  this->SetInputMRMLNode(nullptr);
}

//----------------------------------------------------------------------------
void vtkMRMLMeasurement::ClearValue(ComputationResult computationResult /*=NoChange*/)
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
  Superclass::PrintSelf(os, indent);
  os << indent << "Enabled: " << (this->Enabled ? "true" : "false") << "\n";
  os << indent << "Name: " << this->Name << "\n";
  os << indent << "PrintableValue: " << this->GetValueWithUnitsAsPrintableString();
  os << indent << "Value: " << this->Value << "\n";
  os << indent << "DisplayCoefficient: " << this->DisplayCoefficient << "\n";
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
  this->SetDisplayCoefficient(src->GetDisplayCoefficient());
  this->ValueDefined = src->GetValueDefined();
  this->SetUnits(src->GetUnits());
  this->SetPrintFormat(src->GetPrintFormat());
  this->SetDescription(src->GetDescription());
  if (src->QuantityCode)
  {
    if (!this->QuantityCode)
    {
      this->QuantityCode = vtkSmartPointer<vtkCodedEntry>::New();
    }
    this->QuantityCode->Copy(src->QuantityCode);
  }
  else
  {
    this->QuantityCode = nullptr;
  }
  if (src->DerivationCode)
  {
    if (!this->DerivationCode)
    {
      this->DerivationCode = vtkSmartPointer<vtkCodedEntry>::New();
    }
    this->DerivationCode->Copy(src->DerivationCode);
  }
  else
  {
    this->DerivationCode = nullptr;
  }
  if (src->UnitsCode)
  {
    if (!this->UnitsCode)
    {
      this->UnitsCode = vtkSmartPointer<vtkCodedEntry>::New();
    }
    this->UnitsCode->Copy(src->UnitsCode);
  }
  else
  {
    this->UnitsCode = nullptr;
  }
  if (src->MethodCode)
  {
    if (!this->MethodCode)
    {
      this->MethodCode = vtkSmartPointer<vtkCodedEntry>::New();
    }
    this->MethodCode->Copy(src->MethodCode);
  }
  else
  {
    this->MethodCode = nullptr;
  }
  if (src->ControlPointValues)
  {
    if (!this->ControlPointValues)
    {
      this->ControlPointValues = vtkSmartPointer<vtkDoubleArray>::New();
    }
    this->ControlPointValues->DeepCopy(src->ControlPointValues);
  }
  else
  {
    this->ControlPointValues = nullptr;
  }
  if (src->MeshValue)
  {
    if (!this->MeshValue)
    {
      this->MeshValue = vtkSmartPointer<vtkPolyData>::New();
    }
    this->MeshValue->DeepCopy(src->MeshValue);
  }
  else
  {
    this->MeshValue = nullptr;
  }

  this->SetInputMRMLNode(src->InputMRMLNode);
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
    this->ControlPointValues = nullptr;
    return;
  }
  if (!this->ControlPointValues)
  {
    this->ControlPointValues = vtkSmartPointer<vtkDoubleArray>::New();
  }
  this->ControlPointValues->DeepCopy(inputValues);
}

//----------------------------------------------------------------------------
void vtkMRMLMeasurement::SetMeshValue(vtkPolyData* meshValue)
{
  this->MeshValue = meshValue;
}

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
    case OK:
      return "OK";
    case InsufficientInput:
      return "Insufficient input";
    case InternalError:
      return "Internal error";
    default:
      // invalid id
      return "";
  }
}

//----------------------------------------------------------------------------
void vtkMRMLMeasurement::SetDisplayValue(double displayValue,
                                         const char* units /*=nullptr*/,
                                         double displayCoefficient /*=0.0*/)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting Value to " << displayValue);
  bool modified = false;

  // Update units and displayCoefficient if they are specified
  if (units)
  {
    if (this->Units != units)
    {
      this->Units = units;
      modified = true;
    }
  }
  if (displayCoefficient != 0.0)
  {
    // caller specified a new displayCoefficient
    if (this->DisplayCoefficient != displayCoefficient)
    {
      this->DisplayCoefficient = displayCoefficient;
      modified = true;
    }
  }
  else
  {
    // caller specified a new displayCoefficient
    displayCoefficient = this->DisplayCoefficient;
  }
  // Compute stored value
  if (displayCoefficient == 0.0)
  {
    vtkErrorMacro("vtkMRMLMeasurement::SetDisplayValue: invalid display coefficient == 0.0, using 1.0 instead");
    displayCoefficient = 1.0;
  }
  double value = displayValue / displayCoefficient;

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
void vtkMRMLMeasurement::SetValue(double value, const char* quantityName)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting Value to " << value);
  bool modified = false;

  vtkMRMLUnitNode* unitNode = this->GetUnitNode(quantityName);
  if (unitNode)
  {
    std::string units;
    if (unitNode->GetSuffix())
    {
      units = unitNode->GetSuffix();
    }
    if (this->Units != units)
    {
      this->Units = units;
      modified = true;
    }

    std::string printFormat;
    if (unitNode->GetDisplayStringFormat())
    {
      printFormat = unitNode->GetDisplayStringFormat();
    }
    if (this->PrintFormat != printFormat)
    {
      this->PrintFormat = printFormat;
      modified = true;
    }

    double displayCoefficient = unitNode->GetDisplayCoefficient();
    if (this->DisplayCoefficient != displayCoefficient)
    {
      this->DisplayCoefficient = displayCoefficient;
      modified = true;
    }
  }

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
  ComputationResult computationResult = vtkMRMLMeasurement::OK;
  if (this->LastComputationResult != computationResult)
  {
    this->LastComputationResult = computationResult;
    modified = true;
  }
  if (modified)
  {
    this->Modified();
  }
}

//---------------------------------------------------------------------------
double vtkMRMLMeasurement::GetDisplayValue()
{
  return this->Value * this->DisplayCoefficient;
};

//---------------------------------------------------------------------------
vtkMRMLUnitNode* vtkMRMLMeasurement::GetUnitNode(const char* quantityName)
{
  if (!quantityName || strlen(quantityName) == 0)
  {
    return nullptr;
  }
  if (!this->InputMRMLNode || !this->InputMRMLNode->GetScene())
  {
    vtkWarningMacro(
      "vtkMRMLMeasurement::GetUnitNode failed: InputMRMLNode is required to get the unit node from the scene");
    return nullptr;
  }
  vtkMRMLScene* scene = this->InputMRMLNode->GetScene();
  if (!scene)
  {
    return nullptr;
  }
  vtkMRMLSelectionNode* selectionNode =
    vtkMRMLSelectionNode::SafeDownCast(scene->GetNodeByID("vtkMRMLSelectionNodeSingleton"));
  if (!selectionNode)
  {
    vtkWarningMacro("vtkMRMLMeasurement::GetUnitNode failed: selection node not found");
    return nullptr;
  }
  vtkMRMLUnitNode* unitNode =
    vtkMRMLUnitNode::SafeDownCast(scene->GetNodeByID(selectionNode->GetUnitNodeID(quantityName)));

  // Do not log warning if null, because for example there is no 'angle' unit node, and in
  // that case hundreds of warnings would be thrown in a non erroneous situation.
  return unitNode;
}

//----------------------------------------------------------------------------
vtkCodedEntry* vtkMRMLMeasurement::GetQuantityCode()
{
  return this->QuantityCode;
}

//----------------------------------------------------------------------------
vtkCodedEntry* vtkMRMLMeasurement::GetDerivationCode()
{
  return this->DerivationCode;
}

//----------------------------------------------------------------------------
vtkCodedEntry* vtkMRMLMeasurement::GetUnitsCode()
{
  return this->UnitsCode;
}

//----------------------------------------------------------------------------
vtkCodedEntry* vtkMRMLMeasurement::GetMethodCode()
{
  return this->MethodCode;
}

//----------------------------------------------------------------------------
vtkPolyData* vtkMRMLMeasurement::GetMeshValue()
{
  return this->MeshValue;
}
